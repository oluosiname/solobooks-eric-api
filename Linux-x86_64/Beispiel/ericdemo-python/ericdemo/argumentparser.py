import os
import argparse
from pathlib import Path
import sys as _sys

from applicationerror import ApplicationError


class DemoArgs(argparse.Namespace):
    """Namespace, der einen ApplicationError statt eines AttributeError wirft,
     wenn das Attribut nicht existiert"""
    def __getattr__(self, attr):
        raise ApplicationError("'DemoArgs' object has no attribute {}".format(attr))


class _DemoArgumentParser(argparse.ArgumentParser):
    def print_help(self, file=None):
        """Drucke Hilfe und warte auf Eingabe"""
        super().print_help(file)
        try:
            input("\nBitte drücken Sie die Eingabetaste\n")
        except EOFError:
            pass

    def error(self, message):
        """Hilfe ausgeben, wenn ein Parse-Fehler auftritt"""
        _sys.stderr.write('Fehler: %s\n\n' % message)
        self.print_help()
        _sys.exit(2)

    @staticmethod
    def process_demo_args(args):
        try:
            args.cert = "" if args.cert == '_NULL' else args.cert
            args.pin = "" if args.pin == '_NULL' else args.pin
            args.home_dir = os.fspath(Path(args.home_dir).resolve())
            args.log_dir = os.fspath(Path(args.log_dir).resolve())

        except AttributeError as e:
            raise ApplicationError("Argument Parser falsch konfiguriert: " + str(e))

        # Copy parsed arguments to DemoArgs
        result = DemoArgs()
        for arg in vars(args):
            setattr(result, arg, getattr(args, arg))

        return result

    def parse_args(self, args=None, namespace=None):
        result = super().parse_args(args, namespace)
        return self.process_demo_args(result)


def get_ericdemo_argument_parser():
    pfx = '-'
    lib_folder_name = 'lib'
    if os.name == 'nt':
        pfx = '/'
        lib_folder_name = 'dll'

    parser = _DemoArgumentParser(
        description='ERiC API Python Demo',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        usage=argparse.SUPPRESS,
        prefix_chars=pfx,
        add_help=False
    )
    cwd = os.getcwd()
    default_lib_dir = os.path.join(os.fspath(Path(cwd).parent.parent.resolve()), lib_folder_name)
    default_log_dir = cwd

    # Neue Argument-Gruppe (Default group "options" wird nicht benutzt)
    grp = parser.add_argument_group('Optionen')
    # Untergruppe für Argumente, die sich gegenseitig ausschliessen
    mx_grp = grp.add_mutually_exclusive_group()

    grp.add_argument(pfx + 'h', action='help', default=argparse.SUPPRESS,
                     help='Diese Hilfe ausgeben')

    # Optionen
    mx_grp.add_argument(pfx+'v', type=str,
                     metavar='<datenartversion>',
                     dest='datenartversion',
                     help='Datenartversion',
                     default='ESt_2020'
                     )
    grp.add_argument(pfx + 'x', type=str,
                     metavar='<xml>',
                     dest='xml',
                     help='Pfad zur Datensatzdatei',
                     default='ESt_2020.xml'
                     )
    grp.add_argument(pfx + 'c', type=str,
                     metavar='<certificate>',
                     dest='cert',
                     help='Pfad zu einem Benutzerzertifikat, _NULL (Nullzeiger) für kein Benutzerzertifikat',
                     default='test-softidnr-pse.pfx'
                     )
    grp.add_argument(pfx + 'p', type=str,
                     metavar='<pin>',
                     dest='pin',
                     help=' PIN fuer das Benutzerzertifikat, _NULL (Nullzeiger) für keine PIN',
                     default='123456'
                     )
    grp.add_argument(pfx + 'd', type=str,
                     metavar='<dir>',
                     dest='home_dir',
                     help='Pfad zum Verzeichnis mit den ERiC-Bibliotheken',
                     default=default_lib_dir
                     )
    grp.add_argument(pfx + 'l', type=str,
                     metavar='<log>',
                     dest='log_dir',
                     help='Pfad zum Verzeichnis, in dem die ERiC-Protokolldateien geschrieben werden',
                     default=default_log_dir
                     )
    grp.add_argument(pfx + 's', type=str,
                     metavar='<dateipfad>',
                     dest='output_file',
                     help='Die Serverantwort bzw. die entschluesselten Daten werden in die angegebene Datei geschrieben',
                     default=""
                     )
    grp.add_argument(pfx + 't', type=int,
                     metavar='<transferhandle>',
                     dest='transfer_handle',
                     help='Transferhandle, das an die Server uebermittelt wird (nur bei Datenabholungen anzugeben)',
                     default=None
                     )
    grp.add_argument(pfx + 'n', action='store_false',
                     dest="senden",
                     help='Der Datensatz soll nicht versendet, sondern nur validiert werden'
                     )
    mx_grp.add_argument(pfx + 'e', action='store_true',
                     dest='entschluesseln',
                     help='Die Datensatzdatei wird entschluesselt und nicht validiert oder versendet'
                     )
    # Standardwerte ausgeben
    defaults = [
        ('<datenartversion>:', 'ESt_2020'),
        ('<xml>:', 'ESt_2020.xml'),
        ('<certificate>:', 'test-softidnr-pse.pfx'),
        ('<pin>:', '123456'),
        ('<dir>:', str(os.path.join('<Arbeitsverzeichnis>', '..', '..', lib_folder_name))),
        ('<log>:', '<Arbeitsverzeichnis>'),
    ]
    epilog = "Standardwerte:\n"
    for d in defaults:
        epilog += '  %-20s%-20s\n' % (d[0], d[1])

    # Beispiele ausgeben
    aufruf = '  ericdemo'
    epilog += '\nBeispiele:\n'
    epilog += f'{aufruf}\n'

    epilog += f'{aufruf} {pfx}v ESt_2020 {pfx}x ESt_2020.xml {pfx}c test-softidnr-pse.pfx {pfx}p 123456\n'
    epilog += f'{aufruf} {pfx}v ESt_2016 {pfx}x ESt_2016.xml {pfx}c _NULL {pfx}p _NULL\n'
    epilog += f'{aufruf} {pfx}v ESt_2020 {pfx}x ESt_2020.xml {pfx}n\n'
    epilog += f'{aufruf} {pfx}v ESt_2020 {pfx}x ESt_2020.xml\n'
    epilog += f'{aufruf} {pfx}v Kontoinformation {pfx}x kontoinformation.xml {pfx}c "http://127.0.0.1:24727/eID-Client?testmerker=520000000" {pfx}p _NULL\n'
    epilog += f'{aufruf} {pfx}v MitteilungAbholung {pfx}x MitteilungAbholungAnfrage.xml {pfx}c test-softidnr-pse.pfx {pfx}p 123456 {pfx}t 0\n'
    epilog += f'{aufruf} {pfx}e {pfx}x Abholdaten.b64 {pfx}s Abholdaten.xml\n'

    parser.epilog = epilog
    return parser
