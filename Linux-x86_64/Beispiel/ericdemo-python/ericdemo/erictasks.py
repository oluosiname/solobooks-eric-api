import locale
import os

import ericapi
from ericapi.bearbeitungsflags import ERIC_SENDE, ERIC_VALIDIERE, ERIC_DRUCKE

from applicationerror import ApplicationError
from progresscallbacks import ProgressCallbacks
from ericutilities import EricBuffer, EricCertificate, print_titelzeile, EricFortschrittsCallback

from fileio import read_file_bytes

from typing import Optional
from contextlib import nullcontext


def create_print_parameters(pdf_path: str, preview: bool) -> ericapi.eric_druck_parameter_t:
    """Druckeinstellungen erzeugen"""
    params = ericapi.eric_druck_parameter_t()
    params.version = 4
    params.vorschau = 1 if preview else 0
    params.duplexDruck = 0
    params.pdfName = pdf_path.encode(locale.getpreferredencoding()) if os.name == 'nt' else pdf_path
    params.fussText = None
    params.set_callback(None)
    return params


class EricProcess:
    """Klasse zum Aufrufen von EricBearbeiteVorgang"""

    def __init__(self, eric: ericapi.PyEric):
        self._eric = eric
        self._datensatz = ""
        self._datenart_version = ""

    def read_dataset(self, datei: str, datenart_version: str) -> None:
        """Lese XML-Datensatz von einer Datei"""
        print_titelzeile(f'Lese die Datensatzdatei "{datei}" mit Datenartversion "{datenart_version}" ein')
        self._datensatz = read_file_bytes(datei)
        self._datenart_version = datenart_version

    def execute(self,
                certificate: Optional[EricCertificate] = None,
                pdf_path: Optional[str] = None,
                send: bool = False) -> tuple[int, bytes, bytes]:
        """Führt den Vorgang aus
        :returns: EricFehlercode, Eric-Antwort, Serverantwort
        """
        result_server: bytes = bytes()
        result_response: bytes = bytes()

        # Registriere Callbacks
        callback: ProgressCallbacks = ProgressCallbacks()

        with EricFortschrittsCallback(self._eric, callback.global_progress, callback.progress):

            print_params: Optional[ericapi.eric_druck_parameter_t] = None
            processing_flags: int = ERIC_VALIDIERE

            titelzeile: str = "Validiere"

            if send:
                processing_flags |= ERIC_SENDE
                titelzeile = "Sende"
                if certificate is not None:
                    print(f'Sende mit Zertifikat: {certificate.get_path()}')
                else:
                    print('Sende ohne Zertifikat')

            if pdf_path is not None:
                processing_flags |= ERIC_DRUCKE
                print_params = create_print_parameters(pdf_path, not send)
                titelzeile += " und drucke den Datensatz in die Datei \"{}\"".format(pdf_path)
            else:
                titelzeile += " den Datensatz"

            print_titelzeile(titelzeile)

            crypto_params: Optional[
                ericapi.eric_verschluesselungs_parameter_t] = certificate.get_verschluesselungsparameter() if certificate and send else None
            rc = 0
            with EricBuffer(self._eric) as response_buffer:
                server_buffer_context = nullcontext()
                if send:
                    server_buffer_context = EricBuffer(self._eric)
                with server_buffer_context as server_buffer:
                    server_buffer_handle = server_buffer.handle() if server_buffer else None
                    rc = self._eric.PyEricBearbeiteVorgang(
                        datenpuffer=self._datensatz,
                        datenartVersion=self._datenart_version,
                        bearbeitungsFlags=processing_flags,
                        druckParameter=print_params,
                        cryptoParameter=crypto_params,
                        rueckgabeXmlPuffer=response_buffer.handle(),
                        serverantwortXmlPuffer=server_buffer_handle
                    )
                    result_server: bytes = server_buffer.content() if server_buffer else bytes()
                result_response: bytes = response_buffer.content()

        return rc, result_response, result_server


class EricDecode:
    """Klasse zum Aufrufen von EricDekodiereDaten"""

    def __init__(self, eric: ericapi.PyEric):
        self._eric = eric
        self._datei = ""
        self._datensatz = ""
        self._result = bytes()

    def read_dataset(self, datei: str) -> None:
        """Lese XML-Datensatz von einer Datei"""
        self._datei = datei
        self._datensatz = read_file_bytes(self._datei)

    def get_result(self) -> bytes:
        """Holt das Ergebnis der zuletzt ausgeführten Dekodierung"""
        return self._result

    def execute(self, certificate: EricCertificate) -> int:
        """Führt die Dekodierung mit dem übergebenen Zertifikat aus und gibt die
        entschlüsselten Daten zurück.
        :returns: EricFehlercode
        """
        print_titelzeile(f'Entschlüsselung der Daten aus der Datei "{self._datei}"')
        self._result = ""

        if certificate is None:
            raise ApplicationError("Zur Entschluesselung muss ein Zertifikat angegeben werden.")

        print(f'Entschlüssele mit Zertifikat: {certificate.get_path()}')

        with EricBuffer(self._eric) as buffer:
            rc = self._eric.PyEricDekodiereDaten(
                zertifikatHandle=certificate.get_handle(),
                pin=certificate.get_pin(),
                base64Eingabe=self._datensatz,
                rueckgabePuffer=buffer.handle()
            )
            self._result = buffer.content()
        return rc
