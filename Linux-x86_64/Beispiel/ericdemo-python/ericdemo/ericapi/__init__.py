"""
Dieses Paket bietet Zugriff auf die Funktionen der ERiC API und stellt die Konstanten und notwendige Strukturen zur Verfügung.
Die ERiC-Funktionen werden über die Wrapper-Klasse PyEric bereitgestellt.
Zur Verwendung dieses Pakets kann in den meisten Fällen die ERiC-API-Referenz konsultiert werden.
Abweichungen:
- ERiC-Funktionen, die Werte über übergebene Zeiger zurückgeben, geben diese Werte hier stattdessen als zusätzlichen
Rückgabewert zurück. Das betrifft die Funktionen: PyEricBearbeiteVorgang, PyEricGetHandleToCertificate, PyEricGetPinStatus
- Beim Registrieren der Callbackfunktionen wird kein Userdata übergeben

Die möglichen Argumente und Rückgabewerte sind in den Type Annotations und Docstrings der PyEric-Funktionen dokumentiert.

Hinweis zur String-Kodierung:
ERiC API Funktionen, die Strings von den Typen 'char*' oder 'byteChar*' erwarten, akzeptieren
über die PyEric-Wrapperfunktionen jeweils beide Python-Typen 'str' und 'bytes'. Daten vom Typ 'str' werden dabei
UTF-8-kodiert und Daten vom Typ 'bytes' werden unverändert an die ERiC API weitergereicht.
"""
from ._ericapi import *
