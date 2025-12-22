"""Flask API - thin route handlers that delegate to EricClient"""
import os
import base64
import tempfile
from flask import Flask, request, send_file, jsonify
from models import ValidationRequest, ValidationResult, SubmissionRequest, SubmissionResult, HealthStatus
from eric_client import EricClient

app = Flask(__name__)

# Initialize ERIC client
eric_client = EricClient()


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
            return jsonify({'error': f'Invalid request: {str(e)}'}), 400
        
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
    except Exception as err:
        import traceback
        return jsonify({
            'error': f'Unexpected error: {str(err)}',
            'traceback': traceback.format_exc()
        }), 500


@app.route('/submit', methods=['POST'])
def submit_vat_return():
    """Submit advanced VAT return (UStVA) and return PDF + submission response"""
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
            return jsonify({'error': f'Invalid request: {str(e)}'}), 400
        
        # Submit XML using ERIC client
        success, error_code, transfer_handle, error_message, pdf_data, server_response = eric_client.submit_xml(
            req.xml,
            req.cert_base64,
            req.password,
            req.datenartversion,
            req.return_pdf
        )
        
        if not success:
            return jsonify({
                'error': 'ERIC submission failed',
                'error_code': error_code,
                'error_message': error_message,
                'server_response': server_response
            }), 500
        
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
            
            # Return PDF file
            response = send_file(
                pdf_file_path,
                mimetype='application/pdf',
                as_attachment=True,
                download_name=f'UStVA_2025_submission_{transfer_handle if transfer_handle else "unknown"}.pdf'
            )
            
            response.headers['X-Transfer-Handle'] = str(transfer_handle) if transfer_handle else 'N/A'
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
                pdf_base64=pdf_base64,
                server_response=server_response,
                message='VAT return submitted successfully'
            )
            
            return jsonify(result.model_dump())
    
    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception as err:
        import traceback
        return jsonify({
            'error': f'Unexpected error: {str(err)}',
            'traceback': traceback.format_exc()
        }), 500
    
    finally:
        # Cleanup temporary PDF file
        for temp_file in temp_files:
            try:
                if os.path.exists(temp_file):
                    os.unlink(temp_file)
            except:
                pass


if __name__ == '__main__':
    os.makedirs('logs', exist_ok=True)
    app.run(host='0.0.0.0', port=5000, debug=True)
