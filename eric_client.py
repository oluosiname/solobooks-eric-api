"""ERIC client - encapsulates all ERIC interaction logic"""
import os
import sys
import base64
import tempfile
import xml.etree.ElementTree as ET
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
            
            # Find year - check multiple possible locations
            jahr = None
            
            # First, try to find Jahr element directly
            for path in [
                './/elster:Jahr',
                './/Jahr',
                './/elster:Umsatzsteuervoranmeldung/elster:Jahr',
                './/Umsatzsteuervoranmeldung/Jahr',
                './/elster:Steuerfall/elster:Umsatzsteuervoranmeldung/elster:Jahr',
                './/Steuerfall/Umsatzsteuervoranmeldung/Jahr'
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
                return f"{daten_art}_{jahr}"
            else:
                # Fallback: use DatenArt as-is
                return daten_art
                
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
    ) -> Tuple[bool, Optional[int], Optional[int], Optional[str], Optional[bytes], Optional[str], Optional[str]]:
        """
        Submit XML with certificate authentication
        
        Returns:
            (success, error_code, transfer_handle, error_message, pdf_data, server_response_xml, validation_result_xml)
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
                        return False, rc, None, error_message, None, server_response_text, validation_result_xml
                    
                    # Get PDF from callback if available
                    result_pdf = None
                    if pdf_capture and pdf_capture.pdf_data:
                        result_pdf = pdf_capture.pdf_data
                    
                    return True, None, th, None, result_pdf, server_response_text, None
        finally:
            # Cleanup certificate file
            try:
                if os.path.exists(cert_file_path):
                    os.unlink(cert_file_path)
            except:
                pass

