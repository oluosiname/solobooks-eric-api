import os
from applicationerror import ApplicationError


def read_file_bytes(file: str) -> bytes:
    """Lese Datei ein"""
    try:
        with open(file, 'rb') as f:
            contents = f.read()
    except EnvironmentError:
        raise ApplicationError(f'Datei {file} konnte nicht gelesen werden.')

    return contents


def write_file_bytes(file: str, content: bytes):
    """Schreibt in Datei"""
    try:
        with open(file, 'bw') as f:
            f.write(content)
    except EnvironmentError:
        raise ApplicationError(f'Fehler beim Schreiben in die Datei {file}.')
