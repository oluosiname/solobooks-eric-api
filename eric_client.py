"""ERIC client - encapsulates all ERIC interaction logic"""
import os
import sys
import base64
import tempfile
import xml.etree.ElementTree as ET
from xml.sax import saxutils
import locale
import threading
from pathlib import Path
from contextlib import contextmanager
from typing import Tuple, Optional

# Add the ericdemo path to sys.path to access ericapi
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'Linux-x86_64/Beispiel/ericdemo-python'))

from ericdemo import ericapi
from ericdemo.ericapi.fehlercodes import ERIC_OK
from ericdemo.ericapi.bearbeitungsflags import ERIC_VALIDIERE, ERIC_SENDE, ERIC_DRUCKE


class UnknownTaxYearError(ValueError):
    """The tax year could not be determined and guessing it would be unsafe."""


class PDFCapture:
    """Class to capture PDF data from ERIC callback"""
    def __init__(self):
        self.pdf_data = None
        self.pdf_name = None
        self.lock = threading.Lock()
    
    def callback(self, pdf_name: str, pdf_data: bytes) -> int:
        """Callback function to receive PDF from ERIC"""
        with self.lock:
            self.pdf_name = pdf_name
            self.pdf_data = pdf_data
        return 0  # Return 0 for success


class EricClient:
    """Client for interacting with ERIC API"""
    
    def __init__(self, eric_home_dir: Optional[str] = None, eric_log_dir: Optional[str] = None):
        """Initialize ERIC client"""
        self.eric_home_dir = eric_home_dir or os.path.join(os.path.dirname(__file__), 'Linux-x86_64/lib')
        self.eric_log_dir = eric_log_dir or os.path.join(os.path.dirname(__file__), 'logs')
        self._eric_instance = None
    
    def _get_eric_instance(self):
        """Get or create the ERIC instance"""
        if self._eric_instance is None:
            os.makedirs(self.eric_log_dir, exist_ok=True)
            self._eric_instance = ericapi.PyEric(self.eric_home_dir, self.eric_log_dir)
        return self._eric_instance
    
    @contextmanager
    def _eric_buffer(self, eric):
        """Context manager for ERIC return buffer"""
        handle = eric.PyEricRueckgabepufferErzeugen()
        if handle is None:
            raise Exception("Failed to create return buffer")
        try:
            yield handle
        finally:
            eric.PyEricRueckgabepufferFreigeben(handle)
    
    @contextmanager
    def _eric_certificate(self, eric, cert_path: str, pin: str):
        """Context manager for ERIC certificate"""
        # Encode path for Windows if needed
        path = cert_path.encode(locale.getpreferredencoding()) if os.name == 'nt' else cert_path
        
        rc, htoken, info = eric.PyEricGetHandleToCertificate(path)
        if rc != 0:
            raise Exception(f"Failed to load certificate: error code {rc}")
        
        try:
            # Create encryption parameters
            crypto_params = ericapi.eric_verschluesselungs_parameter_t()
            crypto_params.version = 3
            crypto_params.zertifikatHandle = htoken
            crypto_params.pin = pin
            
            yield crypto_params
        finally:
            eric.PyEricCloseHandleToCertificate(htoken)
    
    def _create_print_parameters(self, pdf_capture: Optional[PDFCapture] = None):
        """Create default print parameters for PDF generation"""
        params = ericapi.eric_druck_parameter_t()
        params.version = 4
        params.vorschau = 0
        params.duplexDruck = 0
        params.pdfName = 'ericprint.pdf'
        params.fussText = None
        
        # Set callback if provided
        if pdf_capture:
            params.set_callback(pdf_capture.callback)
        else:
            params.set_callback(None)
        
        return params
    
    def _get_error_message(self, eric, error_code: int) -> str:
        """Get human-readable error message from ERIC"""
        try:
            with self._eric_buffer(eric) as buffer:
                rc = eric.PyEricHoleFehlerText(error_code, buffer)
                if rc == ERIC_OK:
                    return eric.PyEricRueckgabepufferInhalt(buffer).decode('utf-8', errors='replace')
        except:
            pass
        return f'Error code: {error_code}'
    
    @staticmethod
    def _extract_est_year(root) -> Optional[str]:
        """Find the tax year of an Einkommensteuererklaerung from the E10 namespace.

        E10 carries no Jahr element; the year is only in its namespace URI
        (.../est/e10/v2025). Returns None when the document is not an E10.
        """
        for elem in root.iter():
            tag = elem.tag
            if not tag.startswith('{'):
                continue

            namespace, _, local_name = tag[1:].partition('}')
            if local_name != 'E10':
                continue

            segment = namespace.rstrip('/').rsplit('/', 1)[-1]
            year = segment[1:] if segment.startswith('v') else segment
            if year.isdigit() and len(year) == 4:
                return year

        return None

    def extract_transferticket(self, eric, server_response: Optional[str]) -> Optional[str]:
        """Extract the Transferticket from the Finanzamt server response XML.

        The Transferticket is the receipt the filer quotes to the Finanzamt. It is
        distinct from the transfer handle returned by EricBearbeiteVorgang, which is
        a Datenabholung bundling parameter and is NULL for a submission.

        Returns None rather than raising: a submission that succeeded must not be
        reported as failed just because the receipt could not be parsed.
        """
        if not server_response:
            return None

        try:
            with self._eric_buffer(eric) as ticket_buffer, \
                    self._eric_buffer(eric) as rc_buffer, \
                    self._eric_buffer(eric) as error_buffer, \
                    self._eric_buffer(eric) as ndh_buffer:
                rc = eric.PyEricGetErrormessagesFromXMLAnswer(
                    server_response,
                    ticket_buffer,
                    rc_buffer,
                    error_buffer,
                    ndh_buffer)

                if rc != ERIC_OK:
                    print(f'Warning: could not extract Transferticket: error code {rc}')
                    return None

                ticket = eric.PyEricRueckgabepufferInhalt(ticket_buffer)
                if not ticket:
                    return None

                return ticket.decode('utf-8', errors='replace').strip() or None
        except Exception as e:
            print(f'Warning: could not extract Transferticket: {e}')
            return None

    def extract_datenart_version(self, xml_content: str) -> Optional[str]:
        """Extract DatenArt and version from XML"""
        try:
            root = ET.fromstring(xml_content)
            ns = {'elster': 'http://www.elster.de/elsterxml/schema/v11'}
            
            # Check if root element is 'zm' (ZM format - no TransferHeader)
            root_tag = root.tag
            if '}' in root_tag:
                root_tag = root_tag.split('}')[1]
            
            if root_tag == 'zm':
                # ZM format: extract year from zm structure
                jahr = None
                for path in [
                    './/elster:zm/elster:unternehmer/elster:zm-zeilen/elster:mzr/elster:jahr',
                    './/zm/unternehmer/zm-zeilen/mzr/jahr',
                    './/elster:mzr/elster:jahr',
                    './/mzr/jahr'
                ]:
                    jahr_elem = root.find(path, ns if 'elster:' in path else {})
                    if jahr_elem is not None and jahr_elem.text:
                        jahr = jahr_elem.text
                        break
                
                if jahr:
                    return f"ZM_{jahr}"
                else:
                    # Fallback: try to get version from zm element's version attribute
                    version_attr = root.get('version')
                    if version_attr:
                        # Version is like "000005", but we need the year
                        # For now, return ZM without year if we can't find it
                        return "ZM"
                    raise ValueError("Could not find year in ZM XML")
            
            # Standard format: Get DatenArt from TransferHeader
            daten_art_elem = root.find('.//elster:TransferHeader/elster:DatenArt', ns)
            if daten_art_elem is None:
                daten_art_elem = root.find('.//TransferHeader/DatenArt')
            
            if daten_art_elem is None or daten_art_elem.text is None:
                raise ValueError("Could not find DatenArt in XML")
            
            daten_art = daten_art_elem.text

            # ESt (Einkommensteuererklaerung): the year lives in the E10 element's
            # namespace, e.g. http://finkonsens.de/elster/elstererklaerung/est/e10/v2025.
            # Resolved explicitly rather than through the path guessing below, which
            # has no E10 case and would fall through to a bare "ESt".
            est_jahr = self._extract_est_year(root)
            if est_jahr:
                return f"ESt_{est_jahr}"

            # Find year - check multiple possible locations
            jahr = None
            
            # First, try to find Jahr element directly
            for path in [
                './/elster:Jahr',
                './/Jahr',
                './/elster:Umsatzsteuervoranmeldung/elster:Jahr',
                './/Umsatzsteuervoranmeldung/Jahr',
                './/elster:Steuerfall/elster:Umsatzsteuervoranmeldung/elster:Jahr',
                './/Steuerfall/Umsatzsteuervoranmeldung/Jahr',
                './/elster:Einnahmenueberschussrechnung/elster:Jahr',
                './/Einnahmenueberschussrechnung/Jahr',
                './/elster:E77/elster:Jahr',
                './/E77/Jahr'
            ]:
                jahr_elem = root.find(path, ns if 'elster:' in path else {})
                if jahr_elem is not None and jahr_elem.text:
                    jahr = jahr_elem.text
                    break
            
            # If not found, try to extract from element names (UStVA_2025, Anmeldungssteuern with version attribute, etc.)
            if jahr is None:
                for elem in root.iter():
                    tag = elem.tag
                    # Remove namespace if present
                    if '}' in tag:
                        tag = tag.split('}')[1]
                    
                    # Check for UStVA_YYYY pattern
                    if '_' in tag and tag.startswith('UStVA'):
                        parts = tag.split('_')
                        if len(parts) == 2 and parts[1].isdigit():
                            jahr = parts[1]
                            break
                    
                    # For EUER, check E77 element version attribute specifically
                    if daten_art == 'EUER' and tag == 'E77' and 'version' in elem.attrib:
                        version_attr = elem.attrib['version']
                        # Version might be in format like "2025" or "v2025"
                        if version_attr.isdigit() and len(version_attr) == 4:
                            jahr = version_attr
                            break
                        elif version_attr.startswith('v') and version_attr[1:].isdigit() and len(version_attr) == 5:
                            jahr = version_attr[1:]
                            break
                    
                    # Check for version attribute (e.g., Anmeldungssteuern version="2025")
                    if 'version' in elem.attrib:
                        version_attr = elem.attrib['version']
                        if version_attr.isdigit() and len(version_attr) == 4:
                            jahr = version_attr
                            break
            
            # Construct datenartversion
            if jahr:
                if daten_art == 'UStVA':
                    return f"UStVA_{jahr}"
                elif daten_art == 'EUER':
                    return f"EUER_{jahr}"
                return f"{daten_art}_{jahr}"
            elif daten_art == 'ESt':
                # A bare "ESt" resolves to no plugin, or the wrong year's. Fail loudly
                # rather than letting ERiC pick.
                raise UnknownTaxYearError(
                    'Could not determine the tax year for an ESt submission. '
                    'Pass datenartversion explicitly, e.g. ESt_2025.')
            else:
                # Fallback: use DatenArt as-is
                return daten_art

        except UnknownTaxYearError:
            raise
        except Exception as e:
            print(f"Warning: Could not extract datenartversion from XML: {e}")
            return None
    
    def validate_xml(self, xml_content: str, datenart_version: Optional[str] = None) -> Tuple[bool, Optional[int], Optional[str], Optional[str]]:
        """
        Validate XML without submitting
        
        Returns:
            (is_valid, error_code, error_message, validation_result_xml)
        """
        eric = self._get_eric_instance()
        
        # Extract datenartversion if not provided
        if not datenart_version:
            datenart_version = self.extract_datenart_version(xml_content)
            if not datenart_version:
                raise ValueError('Could not determine datenartversion from XML. Please provide it explicitly.')
        
        # Convert XML to bytes
        if isinstance(xml_content, str):
            xml_bytes = xml_content.encode('utf-8')
        else:
            xml_bytes = xml_content
        
        # Only validate, no send or print
        processing_flags = ERIC_VALIDIERE
        
        with self._eric_buffer(eric) as response_buffer, self._eric_buffer(eric) as server_buffer:
            rc, th = eric.PyEricBearbeiteVorgang(
                datenpuffer=xml_bytes,
                datenartVersion=datenart_version,
                bearbeitungsFlags=processing_flags,
                druckParameter=None,
                cryptoParameter=None,
                transferHandle=None,
                rueckgabeXmlPuffer=response_buffer,
                serverantwortXmlPuffer=server_buffer
            )
            
            result = eric.PyEricRueckgabepufferInhalt(response_buffer)
            validation_result = result.decode('utf-8', errors='replace') if result else None
            
            if rc != ERIC_OK:
                error_message = self._get_error_message(eric, rc)
                return False, rc, error_message, validation_result
            
            return True, None, None, validation_result
    
    def submit_xml(
        self, 
        xml_content: str, 
        cert_base64: str, 
        password: str, 
        datenart_version: Optional[str] = None,
        return_pdf: bool = True
    ) -> Tuple[bool, Optional[int], Optional[int], Optional[str], Optional[bytes], Optional[str], Optional[str], Optional[str]]:
        """
        Submit XML with certificate authentication

        Returns:
            (success, error_code, transfer_handle, error_message, pdf_data,
             server_response_xml, validation_result_xml, transferticket)
        """
        eric = self._get_eric_instance()
        
        # Extract datenart_version if not provided
        if not datenart_version:
            datenart_version = self.extract_datenart_version(xml_content)
            if not datenart_version:
                raise ValueError('Could not determine datenartversion from XML. Please provide it explicitly.')
        
        # Allow any valid datenartversion - ERIC library will validate if it's supported
        # This enables support for UStVA, ZM, and other data types
        
        # Decode and save certificate
        try:
            cert_data = base64.b64decode(cert_base64)
            with tempfile.NamedTemporaryFile(mode='wb', suffix='.pfx', delete=False) as cert_file:
                cert_file.write(cert_data)
                cert_file_path = cert_file.name
        except Exception as e:
            raise ValueError(f'Failed to decode certificate: {str(e)}')
        
        try:
            # Convert XML to bytes
            if isinstance(xml_content, str):
                xml_bytes = xml_content.encode('utf-8')
            else:
                xml_bytes = xml_content
            
            # Process with ERIC
            processing_flags = ERIC_VALIDIERE | ERIC_SENDE | ERIC_DRUCKE
            
            # Always capture PDF (ERIC generates it when ERIC_DRUCKE is set)
            pdf_capture = PDFCapture()
            print_params = self._create_print_parameters(pdf_capture)
            
            with self._eric_certificate(eric, cert_file_path, password) as crypto_params:
                with self._eric_buffer(eric) as response_buffer, self._eric_buffer(eric) as server_buffer:
                    rc, th = eric.PyEricBearbeiteVorgang(
                        datenpuffer=xml_bytes,
                        datenartVersion=datenart_version,
                        bearbeitungsFlags=processing_flags,
                        druckParameter=print_params,
                        cryptoParameter=crypto_params,
                        transferHandle=None,
                        rueckgabeXmlPuffer=response_buffer,
                        serverantwortXmlPuffer=server_buffer
                    )
                    
                    # Get results - response buffer contains XML, not PDF
                    result_xml = eric.PyEricRueckgabepufferInhalt(response_buffer)
                    server_response = eric.PyEricRueckgabepufferInhalt(server_buffer)
                    server_response_text = server_response.decode('utf-8', errors='replace') if server_response else None
                    validation_result_xml = result_xml.decode('utf-8', errors='replace') if result_xml else None
                    
                    if rc != ERIC_OK:
                        error_message = self._get_error_message(eric, rc)
                        return False, rc, None, error_message, None, server_response_text, validation_result_xml, None

                    # Get PDF from callback if available
                    result_pdf = None
                    if pdf_capture and pdf_capture.pdf_data:
                        result_pdf = pdf_capture.pdf_data

                    transferticket = self.extract_transferticket(eric, server_response_text)

                    return True, None, th, None, result_pdf, server_response_text, None, transferticket
        finally:
            # Cleanup certificate file
            try:
                if os.path.exists(cert_file_path):
                    os.unlink(cert_file_path)
            except:
                pass


    VAST_ELSTER_NS = 'http://www.elster.de/elsterxml/schema/v11'
    # The twelve Belegarten this transport serves (Entwicklerhandbuch
    # Tab. 9-24); only the belegart attribute and the caller's parser differ.
    # Not validated here -- the caller owns input validation.
    VAST_BELEGARTEN = frozenset([
        'VaSt_LStB', 'VaSt_KRV', 'VaSt_RIE', 'VaSt_RUE', 'VaSt_LErsL',
        'VaSt_Pers1', 'VaSt_Pers2', 'VaSt_RBM', 'VaSt_VWL', 'VaSt_FSA',
        'VaSt_ZUS', 'VaSt_GDB',
    ])
    VAST_ABHOLUNG_NS = 'http://finkonsens.de/elster/elsterdatenabholung/v3'

    def datenabholung(
        self,
        idnr: str,
        year: int,
        cert_base64: str,
        password: str,
        hersteller_id: str,
        datenlieferant: str = 'Solobooks',
        product_name: str = 'Solobooks',
        product_version: str = 'spike',
        belegart: Optional[str] = 'VaSt_LStB',
        testmerker: Optional[str] = None,
        datenart_version: str = 'ElsterVaStDaten_31'
    ) -> Tuple[bool, Optional[int], Optional[str], list]:
        """
        Run a complete VaSt Belegabruf (ERiC Entwicklerhandbuch 9.2.4.2).

        Both phases run here rather than in the caller because they must share
        one EricTransferHandle: it is initialised to 0 for the Anfrage and the
        value ERIC returns has to be passed back unchanged on the Abholung,
        which is what bundles them into a single retrieval. Passing NULL
        instead yields ERIC_GLOBAL_TRANSFERHANDLE (610001227).

        Returns:
            (success, error_code, error_message, belege)
        """
        eric = self._get_eric_instance()

        # validate=True: b64decode otherwise drops junk characters and yields
        # empty bytes, which would write a 0-byte .pfx and fail obscurely later.
        try:
            cert_data = base64.b64decode(cert_base64, validate=True)
        except Exception as e:
            raise ValueError(f'Failed to decode certificate: {str(e)}')

        if not cert_data:
            raise ValueError('Certificate is empty after base64 decoding')

        with tempfile.NamedTemporaryFile(mode='wb', suffix='.pfx', delete=False) as cert_file:
            cert_file.write(cert_data)
            cert_file_path = cert_file.name

        try:
            with self._eric_certificate(eric, cert_file_path, password) as crypto_params:
                header = {
                    'hersteller_id': hersteller_id,
                    'datenlieferant': datenlieferant,
                    'product_name': product_name,
                    'product_version': product_version,
                    'testmerker': testmerker,
                }

                # Phase 1: which Belege are on offer. Handle starts at 0.
                anfrage = self._build_anfrage(idnr, year, belegart, header)
                rc, transfer_handle, answer = self._send_abholung(
                    eric, anfrage, datenart_version, crypto_params, 0
                )
                failure = self._failure(eric, rc, answer)
                if failure:
                    return (False, *failure, [])

                ids = self._beleg_ids(answer, belegart)
                if not ids:
                    return True, None, None, []

                # Phase 2: fetch them, reusing the handle phase 1 returned.
                abholung = self._build_abholung(idnr, year, ids, header)
                rc, _th, answer = self._send_abholung(
                    eric, abholung, datenart_version, crypto_params, transfer_handle
                )
                failure = self._failure(eric, rc, answer)
                if failure:
                    return (False, *failure, [])

                return True, None, None, self._decode_belege(eric, answer, crypto_params)
        finally:
            try:
                if os.path.exists(cert_file_path):
                    os.unlink(cert_file_path)
            except OSError:
                pass

    def _failure(self, eric, rc, answer_xml):
        """
        (error_code, error_message) if the retrieval failed, else None.

        Two independent failure channels: rc is ERIC's own code (bad handle,
        plugin missing, transport), while ELSTER refuses a permitted-looking
        request by returning ERIC_OK with a non-zero <Rueckgabe><Code> inside
        the server answer -- e.g. 371015220 when the Belegabruf has never been
        switched on. Checking only rc reports such a refusal as "nothing
        available", which is the one answer a caller must not confuse it with.
        """
        if rc != ERIC_OK:
            return rc, self._get_error_message(eric, rc)

        for code, text in self._server_return_codes(answer_xml):
            if code and code != '0':
                return int(code) if code.isdigit() else None, text

        return None

    def _server_return_codes(self, answer_xml):
        """Every <RC><Rueckgabe> in the server answer, as (code, text)."""
        if not answer_xml:
            return []

        ns = {'e': self.VAST_ELSTER_NS}
        try:
            root = ET.fromstring(answer_xml)
        except ET.ParseError:
            return []

        codes = []
        for rueckgabe in root.iterfind('.//e:RC/e:Rueckgabe', ns):
            code = rueckgabe.find('e:Code', ns)
            text = rueckgabe.find('e:Text', ns)
            codes.append((
                (code.text or '').strip() if code is not None else None,
                (text.text or '').strip() if text is not None else None,
            ))
        return codes

    def _send_abholung(self, eric, xml: str, datenart_version: str, crypto_params, transfer_handle: int):
        """One EricBearbeiteVorgang round trip. Returns (rc, handle, answer_xml)."""
        # No ERIC_DRUCKE: ElsterVaStDaten does not support PDF print (9.2.3).
        flags = ERIC_VALIDIERE | ERIC_SENDE

        with self._eric_buffer(eric) as response_buffer, self._eric_buffer(eric) as server_buffer:
            rc, th = eric.PyEricBearbeiteVorgang(
                datenpuffer=xml.encode('utf-8'),
                datenartVersion=datenart_version,
                bearbeitungsFlags=flags,
                druckParameter=None,
                cryptoParameter=crypto_params,
                transferHandle=transfer_handle,
                rueckgabeXmlPuffer=response_buffer,
                serverantwortXmlPuffer=server_buffer
            )
            server_response = eric.PyEricRueckgabepufferInhalt(server_buffer)
            answer = server_response.decode('utf-8', errors='replace') if server_response else None

            return rc, th, answer

    @staticmethod
    def _xml_escape(value) -> str:
        """Caller-supplied header text can carry & or <, which would otherwise
        produce malformed XML that ERIC rejects with an opaque parse error."""
        return saxutils.escape(str(value)) if value is not None else ''

    def _transfer_header(self, header: dict) -> str:
        e = self._xml_escape
        testmerker = f"<Testmerker>{e(header['testmerker'])}</Testmerker>" if header.get('testmerker') else ''
        return (
            '<TransferHeader version="11">'
            '<Verfahren>ElsterDatenabholung</Verfahren>'
            '<DatenArt>ElsterVaStDaten</DatenArt>'
            '<Vorgang>send-Auth</Vorgang>'
            f'{testmerker}'
            f"<HerstellerID>{e(header['hersteller_id'])}</HerstellerID>"
            f"<DatenLieferant>{e(header['datenlieferant'])}</DatenLieferant>"
            '<Datei><Verschluesselung>CMSEncryptedData</Verschluesselung>'
            '<Kompression>GZIP</Kompression><TransportSchluessel/></Datei>'
            f"<VersionClient>{e(header['product_version'])}</VersionClient>"
            '</TransferHeader>'
        )

    def _nutzdatenblock(self, ticket: int, header: dict, inner: str) -> str:
        e = self._xml_escape
        return (
            '<Nutzdatenblock>'
            '<NutzdatenHeader version="11">'
            f'<NutzdatenTicket>{ticket}</NutzdatenTicket>'
            '<Empfaenger id="L">CS</Empfaenger>'
            f"<Hersteller><ProduktName>{e(header['product_name'])}</ProduktName>"
            f"<ProduktVersion>{e(header['product_version'])}</ProduktVersion></Hersteller>"
            f"<DatenLieferant>{e(header['datenlieferant'])}</DatenLieferant>"
            '</NutzdatenHeader>'
            '<Nutzdaten>'
            f'<Datenabholung xmlns="{self.VAST_ABHOLUNG_NS}" version="31">{inner}</Datenabholung>'
            '</Nutzdaten>'
            '</Nutzdatenblock>'
        )

    def _envelope(self, header: dict, blocks: str) -> str:
        return (
            '<?xml version="1.0" encoding="UTF-8"?>'
            f'<Elster xmlns="{self.VAST_ELSTER_NS}">'
            f'{self._transfer_header(header)}'
            f'<DatenTeil>{blocks}</DatenTeil>'
            '</Elster>'
        )

    def _build_anfrage(self, idnr: str, year: int, belegart: Optional[str], header: dict) -> str:
        attrs = f'idnr="{idnr}" veranlagungsjahr="{year}"'
        if belegart:
            attrs += f' belegart="{belegart}"'
        return self._envelope(header, self._nutzdatenblock(1, header, f'<Anfrage {attrs}/>'))

    def _build_abholung(self, idnr: str, year: int, ids: list, header: dict) -> str:
        blocks = ''.join(
            self._nutzdatenblock(
                index + 1, header,
                f'<Abholung id="{beleg_id}" idnr="{idnr}" veranlagungsjahr="{year}"/>'
            )
            for index, beleg_id in enumerate(ids)
        )
        return self._envelope(header, blocks)

    def _beleg_ids(self, answer_xml: Optional[str], belegart: Optional[str]) -> list:
        if not answer_xml:
            return []

        ns = {'d': self.VAST_ABHOLUNG_NS}
        try:
            root = ET.fromstring(answer_xml)
        except ET.ParseError:
            return []

        ids = []
        for node in root.iterfind('.//d:Anfrage/d:Id', ns):
            if belegart and node.get('belegart') != belegart:
                continue
            if node.get('id'):
                ids.append(node.get('id'))
        return ids

    def _decode_belege(self, eric, answer_xml: Optional[str], crypto_params) -> list:
        """Decrypt each <Datenpaket>; the packets are CMS-encrypted to the cert."""
        if not answer_xml:
            return []

        ns = {'d': self.VAST_ABHOLUNG_NS}
        try:
            root = ET.fromstring(answer_xml)
        except ET.ParseError:
            return []

        belege = []
        for packet in root.iterfind('.//d:Abholung/d:Datenpaket', ns):
            encoded = ''.join((packet.text or '').split())
            if not encoded:
                continue

            with self._eric_buffer(eric) as buffer:
                rc = eric.PyEricDekodiereDaten(
                    zertifikatHandle=crypto_params.zertifikatHandle,
                    pin=crypto_params.pin,
                    base64Eingabe=encoded,
                    rueckgabePuffer=buffer
                )
                if rc != ERIC_OK:
                    belege.append(f'DECODE_ERROR {rc}: {self._get_error_message(eric, rc)}')
                    continue

                decoded = eric.PyEricRueckgabepufferInhalt(buffer)
                belege.append(decoded.decode('utf-8', errors='replace') if decoded else '')

        return belege
