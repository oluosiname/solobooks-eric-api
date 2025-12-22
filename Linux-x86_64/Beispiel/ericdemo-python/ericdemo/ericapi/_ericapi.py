"""
Dieses Modul implementiert die Klasse PyEric, die den ERiC lädt und Wrapper-Funktionen zur Verfügung stellt.
Außerdem werden die Klassen eric_druck_parameter_t, eric_verschluesselungs_parameter_t und eric_zertifikat_parameter_t
definiert, die im Aufrufskript verwendet werden und an die ERiC-Funktionen übergeben werden können.
"""

import os as _os
from pathlib import Path as _Path
import typing as _typing

from ctypes import *
import _ctypes

from . import _erictypes as _et
from ._utils import *

__all__ = ['PyEric',
           'PyEricError',
           'eric_druck_parameter_t',
           'eric_verschluesselungs_parameter_t',
           'eric_zertifikat_parameter_t',
           'EricRueckgabepufferHandle']


class PyEricError(Exception):
    def __init__(self, message: str):
        self.message = message
        super().__init__(self.message)


EricRueckgabepufferHandle = _et.EricRueckgabepufferHandle


class eric_druck_parameter_t():
    def __init__(self):
        self._obj = _et.eric_druck_parameter_t()
        self._as_parameter_ = byref(self._obj)
        self._obj.pdfCallbackBenutzerdaten = c_void_p()
        self._py_pdf_callback = lambda pdf_name, pdf_daten: None 

    version = property(
        lambda self: self._obj.version,
        lambda self, value: setattr(self._obj, 'version', c_uint32(value)))

    vorschau = property(
        lambda self: self._obj.vorschau,
        lambda self, value: setattr(self._obj, 'vorschau', c_uint32(value)))

    duplexDruck = property(
        lambda self: self._obj.duplexDruck,
        lambda self, value: setattr(self._obj, 'duplexDruck', c_uint32(value)))

    pdfName = property(
        lambda self: decode(self._obj.pdfName),
        lambda self, value: setattr(self._obj, 'pdfName', None if value is None else c_char_p(encode_ifstr(value))))

    fussText = property(
        lambda self: decode(self._obj.fussText),
        lambda self, value: setattr(self._obj, 'fussText', None if value is None else c_char_p(encode_ifstr(value))))

    def set_callback(self, callback: _typing.Optional[_typing.Callable[[str, bytes], int]]):
        self._py_pdf_callback = callback
        if callback is None:
            self._obj.pdfCallback = cast(c_void_p(),_et.EricPdfCallback)
        else:
            self._obj.pdfCallback = _et.EricPdfCallback(self._c_pdf_callback)

    def _c_pdf_callback(self, pdf_name, pdf_daten, pdf_groesse, pdf_benutzerdaten) -> int:
        return self._py_pdf_callback(decode(pdf_name), string_at(pdf_daten, pdf_groesse))


class eric_verschluesselungs_parameter_t():
    def __init__(self):
        self._obj = _et.eric_verschluesselungs_parameter_t()
        self._as_parameter_ = byref(self._obj)

    version = property(
        lambda self: self._obj.version,
        lambda self, value: setattr(self._obj, 'version', c_uint32(value)))

    zertifikatHandle = property(
        lambda self: self._obj.zertifikatHandle,
        lambda self, value: setattr(self._obj, 'zertifikatHandle', _et.EricZertifikatHandle(value)))

    pin = property(
        lambda self: decode(self._obj.pin),
        lambda self, value: setattr(self._obj, 'pin', None if value is None else c_char_p(encode_ifstr(value))))


class eric_zertifikat_parameter_t:
    def __init__(self):
        self._obj = _et.eric_zertifikat_parameter_t()
        self._as_parameter_ = byref(self._obj)

    version = property(
        lambda self: self._obj.version,
        lambda self, value: setattr(self._obj, 'version', c_uint32(value)))

    name = property(
        lambda self: decode(self._obj.name),
        lambda self, value: setattr(self._obj, 'name', None if value is None else c_char_p(encode_ifstr(value))))

    land = property(
        lambda self: decode(self._obj.land),
        lambda self, value: setattr(self._obj, 'land', None if value is None else c_char_p(encode_ifstr(value))))

    ort = property(
        lambda self: decode(self._obj.ort),
        lambda self, value: setattr(self._obj, 'ort', None if value is None else c_char_p(encode_ifstr(value))))

    adresse = property(
        lambda self: decode(self._obj.adresse),
        lambda self, value: setattr(self._obj, 'adresse', None if value is None else c_char_p(encode_ifstr(value))))

    email = property(
        lambda self: decode(self._obj.email),
        lambda self, value: setattr(self._obj, 'email', None if value is None else c_char_p(encode_ifstr(value))))

    organisation = property(
        lambda self: decode(self._obj.organisation),
        lambda self, value: setattr(self._obj, 'organisation', None if value is None else c_char_p(encode_ifstr(value))))

    abteilung = property(
        lambda self: decode(self._obj.abteilung),
        lambda self, value: setattr(self._obj, 'abteilung', None if value is None else c_char_p(encode_ifstr(value))))


class EricString(c_char_p):
    @classmethod
    def from_param(cls, obj):
        return c_char_p(encode_ifstr(obj))


class _CallbackLog:
    def __init__(self):
        self._user_callback_log = lambda kategorie, logLevel, nachricht: None
        self._as_parameter_ = cast(c_void_p(), _et.EricLogCallback)

    def _callback_log(self, kategorie, loglevel, nachricht, user):
        """Diese Funktion ruft die vom Benutzer übergebene Callback-Funktion auf"""
        self._user_callback_log(kategorie, loglevel, nachricht)

    def set_callback_log(self, callback: _typing.Optional[_typing.Callable[[str, int, str], None]]):
        if callback is None:
            self._user_callback_log = lambda kategorie, logLevel, nachricht: None
            self._as_parameter_ = cast(c_void_p(), _et.EricLogCallback)
        else:
            self._user_callback_log = callback
            self._as_parameter_ = _et.EricLogCallback(self._callback_log)


class _CallbackProgress:
    def __init__(self):
        self._user_callback_progress = lambda id, pos, max: None
        self._as_parameter_ = cast(c_void_p(), _et.EricFortschrittCallback)

    def _callback_progress(self, id, pos, max, user):
        """Diese Funktion ruft die vom Benutzer übergebene Callback-Funktion auf"""
        self._user_callback_progress(id, pos, max)

    def set_callback_progress(self, callback: _typing.Optional[_typing.Callable[[int, int, int], None]]):
        if callback is None:
            self._user_callback_progress = lambda id, pos, max: None
            self._as_parameter_ = cast(c_void_p(), _et.EricFortschrittCallback)
        else:
            self._user_callback_progress = callback
            self._as_parameter_ = _et.EricFortschrittCallback(self._callback_progress)


class PyEric:
    def __init__(
            self,
            home_dir: str,
            log_dir: _typing.Optional[str]):
        """
        Lädt die ERiC-Bibliothek und ruft EricInitialisiere auf.\n
        Wird das Objekt zerstört, wird EricBeende aufgerufen und die Bibliothek entladen.\n

        :param home_dir: Pfad des Verzeichnisses aus dem die ericapi geladen werden soll
        :param log_dir: Pfad zum Verzeichnis, in dem die ERiC-Protokolldateien geschrieben werden
        :raises PyEricError: Fehler tritt auf, wenn die ERiC-Bibliothek nicht geladen werden kann oder EricInitialisiere fehlschlägt
        """
        self._lib_loaded = False
        self._eric_initialized = False

        arg_home_dir = _os.fspath(_Path(home_dir).resolve())
        arg_log_dir = _os.fspath(_Path(log_dir).resolve()) if log_dir is not None else None
        library_path = _os.path.join(arg_home_dir, get_lib_name('ericapi'))

        try:
            self._lib = self._lade(library_path)
            self._lib_loaded = True
        except OSError as err:
            raise PyEricError(str(err))

        rc = self.PyEricInitialisiere(arg_home_dir, arg_log_dir)
        if rc != 0:
            raise PyEricError(f'ERiC-Initialisierung fehlgeschlagen (Fehler {rc})')
        self._eric_initialized = True

        self._FortschrittsCallback = _CallbackProgress()
        self._GlobalerFortschrittsCallback = _CallbackProgress()
        self._LogCallback = _CallbackLog()

    def __del__(self):
        if self._eric_initialized:
            rc = self.PyEricBeende()
            if rc != 0:
                raise PyEricError(f'ERiC konnte nicht beendet werden (Fehler {rc})')
            self._eric_initialized = False
        if self._lib_loaded:
            if get_platform() == 'Windows':
                _ctypes.FreeLibrary(self._lib._handle)
            else:
                _ctypes.dlclose(self._lib._handle)
            del self._lib

    @staticmethod
    def _lade(lib_dir: str):
        if get_platform() == 'Windows':
            lib = windll.LoadLibrary(lib_dir)
        else:
            lib = cdll.LoadLibrary(lib_dir)

        # [ Function name, Return type, Argument types ]
        ERICAPI_FUNC_TYPES = [
            ['EricBearbeiteVorgang',
             c_int, [EricString,
                     EricString,
                     c_uint32,
                     POINTER(_et.eric_druck_parameter_t),
                     POINTER(_et.eric_verschluesselungs_parameter_t),
                     POINTER(_et.EricTransferHandle),
                     _et.EricRueckgabepufferHandle,
                     _et.EricRueckgabepufferHandle]],
            ['EricBeende',
             c_int, None],
            ['EricChangePassword',
             c_int, [EricString,
                     EricString,
                     EricString]],
            ['EricCheckXML',
             c_int, [EricString,
                     EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricCloseHandleToCertificate',
             c_int, [_et.EricZertifikatHandle]],
            ['EricCreateKey',
             c_int, [EricString,
                     EricString,
                     POINTER(_et.eric_zertifikat_parameter_t)]],
            ['EricCreateTH',
             c_int, [EricString,
                     EricString,
                     EricString,
                     EricString,
                     EricString,
                     EricString,
                     EricString,
                     EricString,
                     EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricDekodiereDaten',
             c_int, [_et.EricZertifikatHandle,
                     EricString,
                     EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricEinstellungAlleZuruecksetzen',
             c_int, None],
            ['EricEinstellungLesen',
             c_int, [EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricEinstellungSetzen',
             c_int, [EricString,
                     EricString]],
            ['EricEinstellungZuruecksetzen',
             c_int, [EricString]],
            ['EricEntladePlugins',
             c_int, None],
            ['EricFormatEWAz',
             c_int, [EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricFormatStNr',
             c_int, [EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricGetAuswahlListen',
             c_int, [EricString,
                     EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricGetErrormessagesFromXMLAnswer',
             c_int, [EricString,
                     _et.EricRueckgabepufferHandle,
                     _et.EricRueckgabepufferHandle,
                     _et.EricRueckgabepufferHandle,
                     _et.EricRueckgabepufferHandle]],
            ['EricGetHandleToCertificate',
             c_int, [POINTER(_et.EricZertifikatHandle),
                     POINTER(c_uint32),
                     EricString]],
            ['EricGetPinStatus',
             c_int, [_et.EricZertifikatHandle,
                     POINTER(c_uint32),
                     c_uint32]],
            ['EricGetPublicKey',
             c_int, [POINTER(_et.eric_verschluesselungs_parameter_t),
                     _et.EricRueckgabepufferHandle]],
            ['EricHoleFehlerText',
             c_int, [c_int, _et.EricRueckgabepufferHandle]],
            ['EricHoleFinanzaemter',
             c_int, [EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricHoleFinanzamtLandNummern',
             c_int, [_et.EricRueckgabepufferHandle]],
            ['EricHoleFinanzamtsdaten',
             c_int, [EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricHoleTestfinanzaemter',
             c_int, [_et.EricRueckgabepufferHandle]],
            ['EricHoleZertifikatEigenschaften',
             c_int, [_et.EricZertifikatHandle,
                     EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricHoleZertifikatFingerabdruck',
             c_int, [POINTER(_et.eric_verschluesselungs_parameter_t),
                     _et.EricRueckgabepufferHandle,
                     _et.EricRueckgabepufferHandle]],
            ['EricInitialisiere',
             c_int, [EricString,
                     EricString]],
            ['EricMakeElsterEWAz',
             c_int, [EricString,
                     EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricMakeElsterStnr',
             c_int, [EricString,
                     EricString,
                     EricString,
                     _et.EricRueckgabepufferHandle]],
            ['EricPruefeBIC',
             c_int, [EricString]],
            ['EricPruefeBuFaNummer',
             c_int, [EricString]],
            ['EricPruefeEWAz',
             c_int, [EricString]],
            ['EricPruefeIBAN',
             c_int, [EricString]],
            ['EricPruefeIdentifikationsMerkmal',
             c_int, [EricString]],
            ['EricPruefeSteuernummer',
             c_int, [EricString]],
            ['EricPruefeZertifikatPin',
             c_int, [EricString,
                     EricString,
                     c_uint32]],
            ['EricRegistriereFortschrittCallback',
             c_int, [_et.EricFortschrittCallback,
                     c_void_p]],
            ['EricRegistriereGlobalenFortschrittCallback',
             c_int, [_et.EricFortschrittCallback,
                     c_void_p]],
            ['EricRegistriereLogCallback',
             c_int, [_et.EricLogCallback,
                     c_uint32,
                     c_void_p]],
            ['EricRueckgabepufferErzeugen',
             _et.EricRueckgabepufferHandle, None],
            ['EricRueckgabepufferFreigeben',
             c_int, [_et.EricRueckgabepufferHandle]],
            ['EricRueckgabepufferInhalt',
             POINTER(c_char), [_et.EricRueckgabepufferHandle]],
            ['EricRueckgabepufferLaenge',
             c_uint32, [_et.EricRueckgabepufferHandle]],
            ['EricSystemCheck',
             c_int, None],
            ['EricVersion',
             c_int, [_et.EricRueckgabepufferHandle]]
        ]

        for fn in ERICAPI_FUNC_TYPES:
            # fn: [name, restype, argtypes]
            setattr(getattr(lib, fn[0]), 'restype', fn[1])
            setattr(getattr(lib, fn[0]), 'argtypes', fn[2])

        return lib

    def PyEricBearbeiteVorgang(
            self,
            datenpuffer: _typing.Union[str, bytes],
            datenartVersion: _typing.Union[str, bytes],
            bearbeitungsFlags: int,
            druckParameter: _typing.Optional[eric_druck_parameter_t],
            cryptoParameter: _typing.Optional[eric_verschluesselungs_parameter_t],
            transferHandle: _typing.Optional[int],
            rueckgabeXmlPuffer: EricRueckgabepufferHandle,
            serverantwortXmlPuffer: EricRueckgabepufferHandle) -> _typing.Tuple[int, _typing.Optional[int]]:
        """
        :returns: EricFehlerCode, EricTransferHandle
        """
        th = None
        th_ref = None
        if transferHandle is not None:
            th = _et.EricTransferHandle(transferHandle)
            th_ref = byref(th)

        rc = self._lib.EricBearbeiteVorgang(
            datenpuffer,
            datenartVersion,
            bearbeitungsFlags,
            druckParameter,
            cryptoParameter,
            th_ref,
            rueckgabeXmlPuffer,
            serverantwortXmlPuffer)

        return rc, None if transferHandle is None else th.value

    def PyEricBeende(self) -> int:
        """
        Wird aufgerufen, wenn PyEric zerstört wird.
        :returns: EricFehlerCode
        """
        return self._lib.EricBeende()

    def PyEricChangePassword(
            self,
            psePath: _typing.Union[bytes, str],
            oldPin: _typing.Union[bytes, str],
            newPin: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricChangePassword(psePath, oldPin, newPin)

    def PyEricCheckXML(
            self,
            xml: _typing.Union[str, bytes],
            datenartVersion: _typing.Union[str, bytes],
            fehlertextPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricCheckXML(xml, datenartVersion, fehlertextPuffer)

    def PyEricCloseHandleToCertificate(
            self,
            hToken: int) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricCloseHandleToCertificate(hToken)

    def PyEricCreateKey(
            self,
            pin: _typing.Union[bytes, str],
            pfad: _typing.Union[bytes, str],
            zertifikatInfo: eric_zertifikat_parameter_t):
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricCreateKey(pin, pfad, zertifikatInfo)

    def PyEricCreateTH(
            self,
            xml: _typing.Union[bytes, str],
            verfahren: _typing.Union[bytes, str],
            datenart: _typing.Union[bytes, str],
            vorgang: _typing.Union[bytes, str],
            testmerker: _typing.Union[bytes, str, None],
            herstellerId: _typing.Union[bytes, str],
            datenLieferant: _typing.Union[bytes, str],
            versionClient: _typing.Union[bytes, str, None],
            publicKey: _typing.Union[bytes, str, None],
            xmlRueckgabePuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricCreateTH(
            xml,
            verfahren,
            datenart,
            vorgang,
            testmerker,
            herstellerId,
            datenLieferant,
            versionClient,
            publicKey,
            xmlRueckgabePuffer)

    def PyEricDekodiereDaten(
            self,
            zertifikatHandle: int,
            pin: _typing.Union[bytes, str],
            base64Eingabe: _typing.Union[bytes, str],
            rueckgabePuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricDekodiereDaten(
            zertifikatHandle,
            pin,
            base64Eingabe,
            rueckgabePuffer)

    def PyEricEinstellungAlleZuruecksetzen(self) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricEinstellungAlleZuruecksetzen()

    def PyEricEinstellungLesen(
            self,
            name: _typing.Union[bytes, str],
            rueckgabePuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricEinstellungLesen(name, rueckgabePuffer)

    def PyEricEinstellungSetzen(
            self,
            name: _typing.Union[bytes, str],
            wert: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricEinstellungSetzen(name, wert)

    def PyEricEinstellungZuruecksetzen(
            self,
            name: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricEinstellungZuruecksetzen(name)

    def PyEricEntladePlugins(self) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricEntladePlugins()

    def PyEricFormatEWAz(
            self,
            ewAzElster: _typing.Union[bytes, str],
            ewAzBescheidPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricFormatEWAz(ewAzElster, ewAzBescheidPuffer)

    def PyEricFormatStNr(
            self,
            eingabeSteuernummer: _typing.Union[bytes, str],
            rueckgabePuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricFormatStNr(eingabeSteuernummer, rueckgabePuffer)

    def PyEricGetAuswahlListen(
            self,
            datenartVersion: _typing.Union[bytes, str],
            feldkennung: _typing.Union[bytes, str, None],
            rueckgabeXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricGetAuswahlListen(datenartVersion, feldkennung, rueckgabeXmlPuffer)

    def PyEricGetErrormessagesFromXMLAnswer(
            self,
            xml: _typing.Union[bytes, str],
            transferticketPuffer: EricRueckgabepufferHandle,
            returncodeTHPuffer: EricRueckgabepufferHandle,
            fehlertextTHPuffer: EricRueckgabepufferHandle,
            returncodesUndFehlertexteNDHXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricGetErrormessagesFromXMLAnswer(
            xml,
            transferticketPuffer,
            returncodeTHPuffer,
            fehlertextTHPuffer,
            returncodesUndFehlertexteNDHXmlPuffer)

    def PyEricGetHandleToCertificate(
            self,
            pathToKeystore: _typing.Union[bytes, str]) -> _typing.Tuple[int, int, int]:
        """
        :returns: EricFehlerCode, EricZertifikatHandle, iInfoPinSupport
        """
        hToken = _et.EricZertifikatHandle(1)
        iInfoPinSupport = c_uint32(1)
        rc = self._lib.EricGetHandleToCertificate(
            byref(hToken),
            byref(iInfoPinSupport),
            pathToKeystore
        )
        return rc, hToken.value, iInfoPinSupport.value

    def PyEricGetPinStatus(
            self,
            hToken: int,
            keyType: int) -> _typing.Tuple[int, int]:
        """
        :returns: EricFehlerCode, PinStatus
        """
        pinStatus = c_uint32(0)
        rc = self._lib.EricGetPinStatus(hToken, byref(pinStatus), keyType)
        return rc, pinStatus.value

    def PyEricGetPublicKey(
            self,
            cryptoParameter: eric_verschluesselungs_parameter_t,
            rueckgabePuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricGetPublicKey(cryptoParameter, rueckgabePuffer)

    def PyEricHoleFehlerText(
            self,
            fehlerkode: int,
            rueckgabePuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricHoleFehlerText(fehlerkode, rueckgabePuffer)

    def PyEricHoleFinanzaemter(
            self,
            finanzamtLandNummer: _typing.Union[bytes, str],
            rueckgabeXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricHoleFinanzaemter(finanzamtLandNummer, rueckgabeXmlPuffer)

    def PyEricHoleFinanzamtLandNummern(
            self,
            rueckgabeXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricHoleFinanzamtLandNummern(rueckgabeXmlPuffer)

    def PyEricHoleFinanzamtsdaten(
            self,
            bufaNr: _typing.Union[bytes, str],
            rueckgabeXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricHoleFinanzamtsdaten(bufaNr, rueckgabeXmlPuffer)

    def PyEricHoleTestfinanzaemter(
            self,
            rueckgabeXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricHoleTestfinanzaemter(rueckgabeXmlPuffer)

    def PyEricHoleZertifikatEigenschaften(
            self,
            hToken: int,
            pin: _typing.Union[bytes, str],
            rueckgabeXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricHoleZertifikatEigenschaften(
            hToken,
            pin,
            rueckgabeXmlPuffer)

    def PyEricHoleZertifikatFingerabdruck(
            self,
            cryptoParameter: eric_verschluesselungs_parameter_t,
            fingerabdruckPuffer: EricRueckgabepufferHandle,
            signaturPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricHoleZertifikatFingerabdruck(
            cryptoParameter,
            fingerabdruckPuffer,
            signaturPuffer)

    def PyEricInitialisiere(
            self,
            pluginPfad: _typing.Union[bytes, str, None],
            logPfad: _typing.Union[bytes, str, None]) -> int:
        """
        Wird bei der Initialisierung von PyEric aufgerufen.
        :returns: EricFehlerCode
        """
        return self._lib.EricInitialisiere(pluginPfad, logPfad)

    def PyEricMakeElsterEWAz(
            self,
            ewAzBescheid: _typing.Union[bytes, str],
            landeskuerzel: _typing.Union[bytes, str],
            ewAzElsterPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricMakeElsterEWAz(
            ewAzBescheid,
            landeskuerzel,
            ewAzElsterPuffer)

    def PyEricMakeElsterStnr(
            self,
            steuernrBescheid: _typing.Union[bytes, str],
            landesnr: _typing.Union[bytes, str, None],
            bundesfinanzamtsnr: _typing.Union[bytes, str, None],
            steuernrPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricMakeElsterStnr(
            steuernrBescheid,
            landesnr,
            bundesfinanzamtsnr,
            steuernrPuffer)

    def PyEricPruefeBIC(
            self,
            bic: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricPruefeBIC(bic)

    def PyEricPruefeBuFaNummer(
            self,
            steuernummer: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricPruefeBuFaNummer(steuernummer)

    def PyEricPruefeEWAz(
            self,
            einheitswertAz: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricPruefeEWAz(einheitswertAz)

    def PyEricPruefeIBAN(
            self,
            iban: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricPruefeIBAN(iban)

    def PyEricPruefeIdentifikationsMerkmal(
            self,
            steuerId: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricPruefeIdentifikationsMerkmal(steuerId)

    def PyEricPruefeSteuernummer(
            self,
            steuernummer: _typing.Union[bytes, str]) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricPruefeSteuernummer(steuernummer)

    def PyEricPruefeZertifikatPin(
            self,
            pathToKeystore: _typing.Union[bytes, str],
            pin: _typing.Union[bytes, str],
            keyType: int) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricPruefeZertifikatPin(pathToKeystore, pin, keyType)

    def PyEricRegistriereFortschrittCallback(
            self,
            callback: _typing.Optional[_typing.Callable[[int, int, int], None]]) -> int:
        """
        Es können Funktionen oder Methoden im ERiC als Callback registriert werden.\n
        Bei der Übergabe von Methoden wird das gebundene Objekt durch PyEric am Leben erhalten.
        Es wird freigegeben, wenn ein anderer Callback registriert, der Callback abgemeldet wird oder PyEric
        zerstört wird.\n
        Wird None übergeben, wird der Callback abgemeldet.\n
        :returns: EricFehlerCode
        """
        self._FortschrittsCallback.set_callback_progress(callback)
        return self._lib.EricRegistriereFortschrittCallback(self._FortschrittsCallback, None)

    def PyEricRegistriereGlobalenFortschrittCallback(
            self,
            callback: _typing.Optional[_typing.Callable[[int, int, int], None]]) -> int:
        """
        Es können Funktionen oder Methoden im ERiC als Callback registriert werden.\n
        Bei der Übergabe von Methoden wird das gebundene Objekt durch PyEric am Leben erhalten.
        Es wird freigegeben, wenn ein anderer Callback registriert, der Callback abgemeldet wird oder PyEric
        zerstört wird.\n
        Wird None übergeben, wird der Callback abgemeldet.\n
        :returns: EricFehlerCode
        """
        self._GlobalerFortschrittsCallback.set_callback_progress(callback)
        return self._lib.EricRegistriereGlobalenFortschrittCallback(self._GlobalerFortschrittsCallback, None)

    def PyEricRegistriereLogCallback(
            self,
            callback: _typing.Optional[_typing.Callable[[str, int, str], None]],
            schreibe_eric_log: int) -> int:
        """
        Es können Funktionen oder Methoden im ERiC als Callback registriert werden.\n
        Bei der Übergabe von Methoden wird das gebundene Objekt durch PyEric am Leben erhalten.
        Es wird freigegeben, wenn ein anderer Callback registriert, der Callback abgemeldet wird oder PyEric
        zerstört wird.\n
        Wird None übergeben, wird der Callback abgemeldet.\n
        :returns: EricFehlerCode
        """
        self._LogCallback.set_callback_log(callback)
        return self._lib.EricRegistriereLogCallback(self._LogCallback, schreibe_eric_log, None)

    def PyEricRueckgabepufferErzeugen(self) -> _typing.Optional[EricRueckgabepufferHandle]:
        """
        :returns: EricRueckgabepufferHandle
        """
        return self._lib.EricRueckgabepufferErzeugen()

    def PyEricRueckgabepufferFreigeben(
            self,
            handle: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricRueckgabepufferFreigeben(handle)

    def PyEricRueckgabepufferInhalt(
            self,
            handle: EricRueckgabepufferHandle) -> bytes:
        """
        :returns: Inhalt des Rückgabepuffers
        """
        data_length = self._lib.EricRueckgabepufferLaenge(handle)
        if data_length == 0:
            return b''
        else:
            data_pointer = self._lib.EricRueckgabepufferInhalt(handle)
            return string_at(data_pointer, data_length)

    def PyEricRueckgabepufferLaenge(
            self,
            handle: EricRueckgabepufferHandle) -> int:
        """
        :returns: Länge des Rückgabepuffers
        """
        return self._lib.EricRueckgabepufferLaenge(handle)

    def PyEricSystemCheck(self) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricSystemCheck()

    def PyEricVersion(
            self,
            rueckgabeXmlPuffer: EricRueckgabepufferHandle) -> int:
        """
        :returns: EricFehlerCode
        """
        return self._lib.EricVersion(rueckgabeXmlPuffer)
