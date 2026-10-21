import typing
from contextlib import nullcontext

import ericapi

from applicationerror import ApplicationError

from argumentparser import get_ericdemo_argument_parser, DemoArgs
from ericapi.fehlercodes import ERIC_OK

from erictasks import EricProcess, EricDecode
from ericutilities import EricBuffer, EricCertificate, print_titelzeile

from fileio import write_file_bytes


def main():
    parser = get_ericdemo_argument_parser()
    args = parser.parse_args()

    try:
        # Initialisiere PyEric
        eric = ericapi.PyEric(args.home_dir, args.log_dir)

        antwort = bytes()

        # Zertifikat einlesen, falls vorhanden
        cert: typing.Union[EricCertificate, nullcontext[None]] = nullcontext()
        if args.cert and (args.senden or args.entschluesseln):
            cert = EricCertificate(eric, args.cert, args.pin)
        with cert as certificate:
            if certificate is not None:
                print_titelzeile(f'Zertifikatseigenschaften von "{args.cert}"')
                print(certificate.get_properties())

            if args.entschluesseln:
                # Daten dekodieren
                dekodierung = EricDecode(eric)
                dekodierung.read_dataset(args.xml)
                rc = dekodierung.execute(certificate)
                ergebnis = dekodierung.get_result()

            else:
                # Daten verarbeiten
                vorgang = EricProcess(eric)
                vorgang.read_dataset(args.xml, args.datenartversion)

                rc, ergebnis, antwort = vorgang.execute(
                    certificate=certificate,
                    pdf_path=args.pdf_path,
                    send=args.senden)

            _protokolliere(args, rc, ergebnis, antwort, eric)


    except ericapi.PyEricError as err:
        print('\nPyEricError: ' + str(err))

    except ApplicationError as err:
        print('\nApplicationError: ' + str(err))


def _protokolliere(
        args: DemoArgs,
        fehlercode: int,
        ergebnis: bytes,
        antwort: bytes,
        eric: ericapi.PyEric
):
    """Gibt den Antwort-Text einer ERiC-Funktion aus und schreibt die Serverantwort oder das Ergebnis in eine Datei"""
    if fehlercode != ERIC_OK:
        with EricBuffer(eric) as textbuffer:
            if ERIC_OK == eric.PyEricHoleFehlerText(fehlercode, textbuffer.handle()):
                fehlertext = textbuffer.content().decode(encoding="utf-8", errors="replace")
            else:
                fehlertext = '<Kein fehlertext verfügbar>'
    else:
        fehlertext = 'Verarbeitung fehlerfrei.'

    p = '\n'
    status_text = 'Validierungsstatus'
    if args.entschluesseln:
        status_text = 'Entschlüsselungsstatus'
    elif args.senden:
        status_text = 'Sendestatus'
    p += f'{status_text}: {fehlertext}\n'

    if ergebnis:
        p += '\nRückgabe:\n'
        if args.entschluesseln:
            p += _lesbare_rueckgabe(ergebnis) + '\n'
        else:
            p += ergebnis.decode(encoding="utf-8", errors="replace") + '\n'
    if antwort:
        p += '\nServerantwort:\n'
        p += antwort.decode(encoding="utf-8", errors="replace") + '\n'

    # Schreibe in Datei
    if args.output_file and (antwort or ergebnis):
        write_file_bytes(args.output_file, antwort if antwort else ergebnis)
        p += f'\n{"Die Serverantwort" if antwort else "Das Ergebnis"} wurde auch in die Datei \"{args.output_file}\" geschrieben.\n'

    print(p)


def _lesbare_rueckgabe(data: bytes) -> str:
    """Gibt eine menschenlesbare Repräsentation der Daten zurück, d.h. bei Texten den Text
    und bei binären Daten eine Angabe zum Binärformat"""
    if len(data) > 3:
        if data[0:4] == b'\x25\x50\x44\x46':
            return '[PDF-Dokument]'
        if data[0:4] == b'\x50\x4B\x03\x04':
            return '[Office-Dokument / ZIP-Archiv]'
        if data[0:2] == b'\xFF\xD8':
            return '[Jpeg-Bild]'
    # Datenanfang auf Null-Bytes durchsuchen
    if data.find(b'\x00', 0, 3000) > 0:
        return '[Binäre Daten]'
    return data.decode(encoding='utf-8', errors='replace')


if __name__ == '__main__':
    main()
    try:
        input("Bitte drücken Sie die Eingabetaste\n")
    except EOFError:
        pass
