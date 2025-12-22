"""ctypes-Typ-Definitionen für die vom ERiC definierten Typen"""
import ctypes as _ct
from ._utils import get_platform

if get_platform() == 'Windows':
    EricLogCallback = _ct.WINFUNCTYPE(None, _ct.c_char_p, _ct.c_int, _ct.c_char_p, _ct.c_void_p)
    EricFortschrittCallback = _ct.WINFUNCTYPE(None, _ct.c_uint32, _ct.c_uint32, _ct.c_uint32, _ct.c_void_p)
    EricPdfCallback = _ct.WINFUNCTYPE(_ct.c_int, _ct.c_char_p, _ct.c_void_p, _ct.c_uint32, _ct.c_void_p)
else:
    EricLogCallback = _ct.CFUNCTYPE(None, _ct.c_char_p, _ct.c_int, _ct.c_char_p, _ct.c_void_p)
    EricFortschrittCallback = _ct.CFUNCTYPE(None, _ct.c_uint32, _ct.c_uint32, _ct.c_uint32, _ct.c_void_p)
    EricPdfCallback = _ct.CFUNCTYPE(_ct.c_int, _ct.c_char_p, _ct.c_void_p, _ct.c_uint32, _ct.c_void_p)

EricZertifikatHandle = _ct.c_uint32
EricTransferHandle = _ct.c_uint32


class EricRueckgabepufferHandle(_ct.c_void_p):
    pass


class eric_druck_parameter_t(_ct.Structure):
    _fields_ = [('version', _ct.c_uint32),
                ('vorschau', _ct.c_uint32),
                ('duplexDruck', _ct.c_uint32),
                ('pdfName', _ct.c_char_p),
                ('fussText', _ct.c_char_p),
                ('pdfCallback', EricPdfCallback),
                ('pdfCallbackBenutzerdaten', _ct.c_void_p)]


class eric_verschluesselungs_parameter_t(_ct.Structure):
    _fields_ = [('version', _ct.c_uint32),
                ('zertifikatHandle', EricZertifikatHandle),
                ('pin', _ct.c_char_p)]


class eric_zertifikat_parameter_t(_ct.Structure):
    _fields_ = [('version', _ct.c_uint32),
                ('name', _ct.c_char_p),
                ('land', _ct.c_char_p),
                ('ort', _ct.c_char_p),
                ('adresse', _ct.c_char_p),
                ('email', _ct.c_char_p),
                ('organisation', _ct.c_char_p),
                ('abteilung', _ct.c_char_p),
                ('beschreibung', _ct.c_char_p)]
