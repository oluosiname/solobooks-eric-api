import ericapi
import os
import locale
from applicationerror import ApplicationError

from typing import Optional


def print_titelzeile(msg: str):
    print(f'\n*** {msg} ***\n')


class EricBuffer:
    """Verwalten von ERiC-Rückgabepuffern"""
    def __init__(self, eric: ericapi.PyEric):
        self._eric = eric
        self._handle = None

    def __enter__(self):
        """Erzeugt eine Instanz eines EricRueckgabePuffers und speichert das erhaltene Handle"""
        self._handle = self._eric.PyEricRueckgabepufferErzeugen()
        if self._handle is None:
            raise ApplicationError("Erzeugung des Rückgabepuffers fehlgeschlagen.")
        return self

    def __exit__(self, *args):
        """Gibt den EricRueckgabePuffer wieder frei"""
        if self._handle is not None:
            rc = self._eric.PyEricRueckgabepufferFreigeben(self._handle)
            if rc != 0:
                raise ApplicationError("Freigeben des Rückgabepuffers fehlgeschlagen.")
        self._handle = None

    def handle(self) -> ericapi.EricRueckgabepufferHandle:
        """Hole das Handle auf den von diesem Objekt verwalteten Rückgabepuffers"""
        if self._handle is None:
            raise ApplicationError("Kein Rückgabepuffer vorhanden.")
        return self._handle

    def content(self) -> bytes:
        """Hole den Inhalt des von diesem Objekt verwalteten Rückgabepuffers"""
        if self._handle is None:
            raise ApplicationError("Kein Rückgabepuffer vorhanden.")
        return self._eric.PyEricRueckgabepufferInhalt(self._handle)

    def length(self) -> int:
        """Hole Anzahl der in den von diesem Objekt verwalteten Rückgabepuffer geschriebenen Bytes"""
        if self._handle is None:
            raise ApplicationError("Kein Rückgabepuffer vorhanden.")
        return self._eric.PyEricRueckgabepufferLaenge(self._handle)


class EricCertificate:
    """Verwalten von einem ERiC-Zertifikat"""
    def __init__(self,
                 eric: ericapi.PyEric,
                 file_path: str,
                 pin: Optional[str]):
        self._eric = eric
        self._file_path = file_path
        self._properties = ""
        self._pin = pin
        self._encryption_parameters = None

    def __enter__(self):
        """Lädt das Zertifikat"""
        path = self._file_path.encode(locale.getpreferredencoding()) if os.name == 'nt' else self._file_path
        rc, htoken, info = self._eric.PyEricGetHandleToCertificate(path)
        if rc != 0:
            raise ApplicationError(f"Kann Zertifikat {self._file_path} nicht laden")

        self._encryption_parameters = ericapi.eric_verschluesselungs_parameter_t()
        self._encryption_parameters.version = 3
        self._encryption_parameters.zertifikatHandle = htoken
        self._encryption_parameters.pin = self._pin

        with EricBuffer(self._eric) as buffer:
            rc = self._eric.PyEricHoleZertifikatEigenschaften(
                self._encryption_parameters.zertifikatHandle,
                self._encryption_parameters.pin,
                buffer.handle()
            )
            if rc != 0:
                self._properties = f"Die Zertifikatseigenschaften konnten nicht ermittelt werden. Fehlercode {rc}"
            else:
                self._properties = buffer.content().decode(encoding='utf-8')

        return self

    def __exit__(self, *args):
        """Schließt das Zertifikat-Handle"""
        if self._encryption_parameters is not None:
            rc = self._eric.PyEricCloseHandleToCertificate(self._encryption_parameters.zertifikatHandle)
            if rc != 0:
                raise ApplicationError(f"Das Zertifikat konnte nicht geschlossen werden. Fehlercode {rc}")
        self._encryption_parameters = None

    def get_pin(self):
        return self._pin

    def get_path(self):
        return self._file_path

    def get_properties(self):
        if self._encryption_parameters is None:
            raise ApplicationError("Das Zertifikat ist nicht geladen.")
        return self._properties

    def get_handle(self):
        if self._encryption_parameters is None:
            raise ApplicationError("Das Zertifikat ist nicht geladen.")
        return self._encryption_parameters.zertifikatHandle

    def get_verschluesselungsparameter(self):
        if self._encryption_parameters is None:
            raise ApplicationError("Das Zertifikat ist nicht geladen.")
        return self._encryption_parameters
