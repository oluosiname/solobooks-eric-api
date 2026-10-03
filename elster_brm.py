"""ElsterBRM Nutzdaten and answers for VaSt authorisation.

Pure functions with no ERiC dependency. Element order and versions follow
elsterbrm_antrag-v11.xsd and the SDK examples under
VaSt-Berechtigungsmanagement/ElsterBRM/Beispiele/.
"""
import xml.etree.ElementTree as ET
from datetime import date
from typing import Iterable, List, Optional
from xml.sax.saxutils import escape

NS = 'http://www.elster.de/elsterxml/schema/v11'
_NS = {'e': NS}


def antrag(idnr: str, date_of_birth: date, valid_until: date, datenabrufer_mail: str) -> str:
    # EOPBenachrichtigungDA off: approval is detected by polling SpezRechtListe.
    return (
        '<SpezRechtAntrag version="3">'
        f'<DateninhaberIdNr>{escape(idnr)}</DateninhaberIdNr>'
        f'<DateninhaberGeburtstag>{date_of_birth.isoformat()}</DateninhaberGeburtstag>'
        '<Recht>AbrufEBelege</Recht>'
        f'<GueltigBis>{valid_until.isoformat()}</GueltigBis>'
        f'<DatenabruferMail>{escape(datenabrufer_mail)}</DatenabruferMail>'
        '<EOPBenachrichtigungDA>false</EOPBenachrichtigungDA>'
        '<Veranlagungszeitraum><Unbeschraenkt>true</Unbeschraenkt></Veranlagungszeitraum>'
        '</SpezRechtAntrag>'
    )


def liste(idnrs: Iterable[str]) -> str:
    # Unfiltered, the list holds every Dateninhaber this certificate has asked about.
    holders = ''.join(f'<DateninhaberIdNr>{escape(i)}</DateninhaberIdNr>' for i in idnrs)
    return (
        '<SpezRechtListe version="7">'
        f'<Suchkriterien><Dateninhaber>{holders}</Dateninhaber></Suchkriterien>'
        '</SpezRechtListe>'
    )


def storno(antrags_id: str) -> str:
    return f'<SpezRechtStorno version="3"><AntragsID>{escape(antrags_id)}</AntragsID></SpezRechtStorno>'


def parse_antrag(answer_xml: str) -> dict:
    node = _find(answer_xml, './/e:SpezRechtAntrag/e:AntragAntwort')
    return {
        'antrags_id': _text(node, 'AntragsID'),
        'antrags_datum': _text(node, 'AntragsDatum'),
        'antrags_status': _text(node, 'AntragsStatus'),
        'genehmigen_bis': _text(node, 'GenehmigenBis'),
        'freischaltcode': _text(node, 'Freischaltcode'),
    }


def parse_liste(answer_xml: str) -> List[dict]:
    return [
        {
            'antrags_id': _text(node, 'AntragsID'),
            'antrags_datum': _text(node, 'AntragsDatum'),
            'antrags_status': _text(node, 'AntragsStatus'),
            'gueltig_bis': _text(node, 'GueltigBis'),
            'genehmigen_bis': _text(node, 'GenehmigenBis'),
            'dateninhaber_idnr': _text(node, 'DateninhaberIdNr'),
        }
        for node in ET.fromstring(answer_xml).iterfind('.//e:SpezRechtListe/e:Antrag', _NS)
    ]


def parse_storno(answer_xml: str) -> dict:
    node = _find(answer_xml, './/e:SpezRechtStorno')
    return {
        'antrags_id': _text(node, 'AntragsID'),
        'antrags_status': _text(node, 'StornoAntwort/e:AntragsStatus'),
    }


def _find(answer_xml: str, path: str) -> ET.Element:
    node = ET.fromstring(answer_xml).find(path, _NS)
    if node is None:
        raise ValueError(f'ELSTER answer has no {path}')
    return node


def _text(node: ET.Element, path: str) -> Optional[str]:
    return node.findtext(f'e:{path}', namespaces=_NS)
