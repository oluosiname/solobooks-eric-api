"""Flask API - thin route handlers that delegate to EricClient"""
import os
import base64
import tempfile
import traceback
from flask import Flask, request, send_file, jsonify
from pydantic import ValidationError

from models import (
    ValidationRequest, ValidationResult, SubmissionRequest, SubmissionResult, HealthStatus,
    DatenabholungRequest, DatenabholungResult, ElsterBrmResult,
    SpezRechtAntragRequest, SpezRechtListeRequest, SpezRechtStornoRequest,
)
from eric_client import EricClient
import elster_brm

app = Flask(__name__)

# Initialize ERIC client
eric_client = EricClient()


def invalid_request(err):
    # Pydantic's message embeds the offending input, which on these routes can
    # be the certificate, its password or the taxpayer's XML, so only the field
    # names and reasons are returned.
    if isinstance(err, ValidationError):
        faults = '; '.join(
            f"{'.'.join(str(p) for p in e['loc'])}: {e['msg']}" for e in err.errors()
        )
        return jsonify({'error': f'Invalid request: {faults}'}), 400
    return jsonify({'error': 'Invalid request'}), 400


def unexpected_error():
    # The traceback's frames hold the request's secrets, so it stays in the
    # server log and the caller gets an opaque error.
    print(traceback.format_exc(), flush=True)
    return jsonify({'error': 'Unexpected error'}), 500


@app.route('/health', methods=['GET'])
def health():
    """Health check endpoint"""
    return jsonify(HealthStatus(status='ok').model_dump())


@app.route('/validate', methods=['POST'])
def validate_xml():
    """Validate XML without submitting"""
    try:
        # Parse and validate request
        data = request.get_json()
        if not data:
            return jsonify({'error': 'No JSON data provided'}), 400
        
        # Create request model (will validate)
        try:
            req = ValidationRequest(**data)
        except Exception as e:
            return invalid_request(e)
        
        # Validate XML using ERIC client
        is_valid, error_code, error_message, validation_result = eric_client.validate_xml(
            req.xml,
            req.datenartversion
        )
        
        if is_valid:
            result = ValidationResult(
                valid=True,
                message='Validation successful',
                validation_result=validation_result
            )
        else:
            result = ValidationResult(
                valid=False,
                error_code=error_code,
                error_message=error_message,
                validation_result=validation_result
            )
        
        return jsonify(result.model_dump())
    
    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception:
        return unexpected_error()


@app.route('/submit', methods=['POST'])
def submit_submission():
    """Submit tax return (UStVA, ZM, etc.) to ELSTER and return PDF + submission response"""
    pdf_file_path = None
    temp_files = []
    
    try:
        # Parse and validate request
        data = request.get_json()
        if not data:
            return jsonify({'error': 'No JSON data provided'}), 400
        
        # Create request model (will validate)
        try:
            req = SubmissionRequest(**data)
        except Exception as e:
            return invalid_request(e)
        
        # Submit XML using ERIC client
        success, error_code, transfer_handle, error_message, pdf_data, server_response, validation_result, transferticket = eric_client.submit_xml(
            req.xml,
            req.cert_base64,
            req.password,
            req.datenartversion,
            req.return_pdf
        )
        
        if not success:
            # A rejection is a problem with the submitted data, not a server fault.
            return jsonify({
                'error': 'ERIC submission failed',
                'error_code': error_code,
                'error_message': error_message,
                'server_response': server_response,
                'validation_result': validation_result
            }), 422
        
        # Handle PDF return based on return_pdf setting
        if req.return_pdf:
            if pdf_data is None:
                return jsonify({
                    'error': 'PDF generation failed - no PDF received from callback',
                    'server_response': server_response
                }), 500
            
            # Verify PDF
            if len(pdf_data) < 4 or pdf_data[0:4] != b'\x25\x50\x44\x46':
                return jsonify({
                    'error': 'PDF generation failed - invalid PDF format',
                    'server_response': server_response
                }), 500
            
            # Save PDF to temporary file
            with tempfile.NamedTemporaryFile(mode='wb', suffix='.pdf', delete=False) as pdf_file:
                pdf_file.write(pdf_data)
                pdf_file_path = pdf_file.name
                temp_files.append(pdf_file_path)
            
            # Extract datenartversion for filename
            datenartversion = req.datenartversion or eric_client.extract_datenart_version(req.xml) or 'submission'
            
            # Return PDF file
            response = send_file(
                pdf_file_path,
                mimetype='application/pdf',
                as_attachment=True,
                download_name=f'{datenartversion}_submission_{transfer_handle if transfer_handle else "unknown"}.pdf'
            )
            
            response.headers['X-Transfer-Handle'] = str(transfer_handle) if transfer_handle else 'N/A'
            response.headers['X-Transferticket'] = transferticket if transferticket else 'N/A'
            response.headers['X-Submission-Status'] = 'success'
            if server_response:
                # Remove newlines and limit length for header (headers can't contain newlines)
                header_value = server_response[:500].replace('\n', ' ').replace('\r', '')
                response.headers['X-Server-Response'] = header_value
            
            return response
        else:
            # Return JSON response with PDF as base64
            pdf_base64 = None
            if pdf_data:
                pdf_base64 = base64.b64encode(pdf_data).decode('utf-8')
            
            result = SubmissionResult(
                status='success',
                transfer_handle=transfer_handle,
                transferticket=transferticket,
                pdf_base64=pdf_base64,
                server_response=server_response,
                message='Tax return submitted successfully'
            )
            
            return jsonify(result.model_dump())
    
    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception:
        return unexpected_error()
    
    finally:
        # Cleanup temporary PDF file
        for temp_file in temp_files:
            try:
                if os.path.exists(temp_file):
                    os.unlink(temp_file)
            except:
                pass


@app.route('/datenabholung', methods=['POST'])
def datenabholung():
    """Run a full VaSt Belegabruf and return the decrypted Belege"""
    try:
        data = request.get_json(silent=True)
        if not data:
            return jsonify({'error': 'No JSON data provided'}), 400

        try:
            req = DatenabholungRequest(**data)
        except Exception as e:
            return invalid_request(e)

        success, error_code, error_message, belege = eric_client.datenabholung(
            idnr=req.idnr,
            year=req.year,
            cert_base64=req.cert_base64,
            password=req.password,
            hersteller_id=req.hersteller_id,
            datenlieferant=req.datenlieferant,
            product_name=req.product_name,
            product_version=req.product_version,
            belegart=req.belegart,
            testmerker=req.testmerker,
        )

        result = DatenabholungResult(
            belege=belege,
            error_code=None if success else error_code,
            error_message=None if success else error_message
        )

        return jsonify(result.model_dump()), (200 if success else 502)

    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception:
        return unexpected_error()


def _elster_brm(request_model, datenart, nutzdaten, parse):
    """Run one ElsterBRM round trip: validate, send, parse the answer."""
    try:
        data = request.get_json(silent=True)
        if not data:
            return jsonify({'error': 'No JSON data provided'}), 400

        try:
            req = request_model(**data)
        except Exception as e:
            return invalid_request(e)

        success, error_code, error_message, answer = eric_client.elster_brm(
            datenart=datenart,
            nutzdaten=nutzdaten(req),
            cert_base64=req.cert_base64,
            password=req.password,
            hersteller_id=req.hersteller_id,
            datenlieferant=req.datenlieferant,
        )

        if not success:
            return jsonify(ElsterBrmResult(error_code=error_code, error_message=error_message).model_dump()), 502

        return jsonify(ElsterBrmResult(data=parse(answer)).model_dump()), 200

    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception:
        return unexpected_error()


@app.route('/elster_brm/antrag', methods=['POST'])
def spez_recht_antrag():
    """Request an authorisation to retrieve a Dateninhaber's Belege"""
    return _elster_brm(
        SpezRechtAntragRequest, 'SpezRechtAntrag',
        lambda r: elster_brm.antrag(r.idnr, r.date_of_birth, r.valid_until, r.datenabrufer_mail),
        elster_brm.parse_antrag,
    )


@app.route('/elster_brm/liste', methods=['POST'])
def spez_recht_liste():
    """List authorisations for the given Dateninhaber"""
    return _elster_brm(
        SpezRechtListeRequest, 'SpezRechtListe',
        lambda r: elster_brm.liste(r.idnrs),
        elster_brm.parse_liste,
    )


@app.route('/elster_brm/storno', methods=['POST'])
def spez_recht_storno():
    """Withdraw an authorisation"""
    return _elster_brm(
        SpezRechtStornoRequest, 'SpezRechtStorno',
        lambda r: elster_brm.storno(r.antrags_id),
        elster_brm.parse_storno,
    )


if __name__ == '__main__':
    os.makedirs('logs', exist_ok=True)
    app.run(host='0.0.0.0', port=5000, debug=True)
