import ericapi
from ericapi.bearbeitungsflags import *

from applicationerror import ApplicationError
from ericutilities import EricBuffer, EricCertificate, print_titelzeile

from fileio import read_file_bytes

from typing import Tuple, Optional
from contextlib import nullcontext


def get_default_print_parameters() -> ericapi.eric_druck_parameter_t:
    """Standard-Druckeinstellungen des Beispiels"""
    params = ericapi.eric_druck_parameter_t()
    params.version = 4
    params.vorschau = 0
    params.duplexDruck = 0
    params.pdfName = 'ericprint.pdf'
    params.fussText = None
    params.set_callback(None)
    return params


class EricProcess:
    """Klasse zum Aufrufen von EricBearbeiteVorgang"""
    def __init__(self, eric: ericapi.PyEric):
        self._eric = eric
        self._datensatz = ""
        self._datenart_version = ""
        self._result_response = bytes()
        self._result_server = bytes()
        self._result_th = None

    def get_results(self) -> Tuple[bytes, bytes]:
        """Holt den Inhalt des Rückgabepuffers und Serverantwort-Puffers des zuletzt ausgeführten Vorgangs
        :returns: Ergebnis, Serverantwort
        """
        return self._result_response, self._result_server

    def get_transfer_handle(self) -> int:
        """Holt den vom letzten ausgeführten Vorgang gesetzten Transfer Handle.
        :returns: TransferHandle
        """
        return self._result_th

    def read_dataset(self, datei: str, datenart_version: str) -> None:
        """Lese XML-Datensatz von einer Datei"""
        print_titelzeile(f'Lese die Datensatzdatei "{datei}" mit Datenartversion "{datenart_version}" ein')
        self._datensatz = read_file_bytes(datei)
        self._datenart_version = datenart_version

    def execute(self,
                certificate: Optional[EricCertificate] = None,
                send: bool = False,
                transfer_handle: Optional[int] = None
                ) -> int:
        """Führt den Vorgang aus
        :returns: EricFehlercode
        """
        self._result_server = bytes()
        self._result_response = bytes()
        self._result_th = None

        print_params = None
        processing_flags = ERIC_VALIDIERE

        if send:
            processing_flags |= ERIC_SENDE
            print_titelzeile('{} den Datensatz'.format("Sende" if transfer_handle is not None else "Sende und drucke"))
            if transfer_handle is None:
                processing_flags |= ERIC_DRUCKE
                print_params = get_default_print_parameters()

            if certificate is not None:
                print(f'Sende mit Zertifikat: {certificate.get_path()}')
            else:
                print('Sende ohne Zertifikat')
        else:
            print_titelzeile('Validiere den Datensatz')

        with certificate if certificate else nullcontext():
            crypto_params = certificate.get_verschluesselungsparameter() if certificate and send else None
            with EricBuffer(self._eric) as response_buffer, EricBuffer(self._eric) as server_buffer:
                rc, th = self._eric.PyEricBearbeiteVorgang(
                    datenpuffer=self._datensatz,
                    datenartVersion=self._datenart_version,
                    bearbeitungsFlags=processing_flags,
                    druckParameter=print_params,
                    cryptoParameter=crypto_params,
                    transferHandle=transfer_handle,
                    rueckgabeXmlPuffer=response_buffer.handle(),
                    serverantwortXmlPuffer=server_buffer.handle()
                    )

                self._result_th = th
                self._result_response = response_buffer.content()
                self._result_server = server_buffer.content()

        return rc


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

        with EricBuffer(self._eric) as buffer, certificate:
            rc = self._eric.PyEricDekodiereDaten(
                zertifikatHandle=certificate.get_handle(),
                pin=certificate.get_pin(),
                base64Eingabe=self._datensatz,
                rueckgabePuffer=buffer.handle()
            )
            self._result = buffer.content()
        return rc
