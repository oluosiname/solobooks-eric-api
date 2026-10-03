from datetime import date
from pathlib import Path

import pytest
from lxml import etree

import elster_brm

FIXTURES = Path(__file__).parent / 'fixtures' / 'elster_brm'


@pytest.fixture(scope='module')
def schema():
    return etree.XMLSchema(etree.parse(str(FIXTURES / 'xsd' / 'elsterbrm_antrag-v11.xsd')))


def element(nutzdaten):
    wrapper = etree.fromstring(f'<Nutzdaten xmlns="{elster_brm.NS}">{nutzdaten}</Nutzdaten>')
    return wrapper[0]


def fixture(name):
    return (FIXTURES / name).read_text(encoding='utf-8')


class TestAntrag:
    def build(self):
        return elster_brm.antrag(
            idnr='04452397687',
            date_of_birth=date(1985, 1, 1),
            valid_until=date(2031, 10, 3),
            datenabrufer_mail='elster@solobooks.de',
        )

    def test_validates_against_the_schema(self, schema):
        schema.assertValid(element(self.build()))

    def test_sends_dates_as_iso(self):
        antrag = element(self.build())

        assert antrag.findtext(f'{{{elster_brm.NS}}}DateninhaberGeburtstag') == '1985-01-01'
        assert antrag.findtext(f'{{{elster_brm.NS}}}GueltigBis') == '2031-10-03'

    def test_requests_every_tax_year(self):
        assert element(self.build()).findtext(f'.//{{{elster_brm.NS}}}Unbeschraenkt') == 'true'


class TestListe:
    def test_validates_against_the_schema(self, schema):
        schema.assertValid(element(elster_brm.liste(['04452397687'])))

    def test_uses_version_7(self):
        # The XSD accepts any version; ERiC refuses 4 (the one 00_Szenarien.txt
        # gives) with 610301006, and only a live call shows it.
        assert element(elster_brm.liste(['04452397687'])).get('version') == '7'

    def test_filters_by_every_idnr_given(self):
        liste = element(elster_brm.liste(['04452397687', '09952417688']))

        found = [e.text for e in liste.iter(f'{{{elster_brm.NS}}}DateninhaberIdNr')]
        assert found == ['04452397687', '09952417688']


class TestStorno:
    def test_validates_against_the_schema(self, schema):
        schema.assertValid(element(elster_brm.storno('br12701v299sh650fgwcn0c31z2k0xrb')))


class TestParseAntrag:
    def test_reads_the_antrag_answer(self):
        assert elster_brm.parse_antrag(fixture('02a_SpezRechtAntrag_Response.xml')) == {
            'antrags_id': 'br12701v299sh650fgwcn0c31z2k0xrb',
            'antrags_datum': '2013-05-07T21:25:21.543',
            'antrags_status': 'offen',
            'genehmigen_bis': '2013-08-05',
            'freischaltcode': None,
        }

    def test_reports_a_freischaltcode_when_elster_sends_one(self):
        answer = fixture('02a_SpezRechtAntrag_Response.xml').replace(
            '<AntragsStatus>offen</AntragsStatus>',
            '<AntragsStatus>offen</AntragsStatus><Freischaltcode>6FG5-R32P-JJ4S</Freischaltcode>',
        )

        assert elster_brm.parse_antrag(answer)['freischaltcode'] == '6FG5-R32P-JJ4S'


class TestParseListe:
    def test_reads_each_antrag(self):
        assert elster_brm.parse_liste(fixture('16_SpezRechtListe_Suchkriterien_Response.xml')) == [{
            'antrags_id': 'br1272xf3i59m2323ft9qtk7iqzxzke4',
            'antrags_datum': '2013-05-07T21:25:21.949',
            'antrags_status': 'offen',
            'gueltig_bis': '2020-05-07',
            'genehmigen_bis': '2013-08-05',
            'dateninhaber_idnr': '09952417688',
        }]


class TestParseStorno:
    def test_reads_the_resulting_status(self):
        assert elster_brm.parse_storno(fixture('08_SpezRechtStorno_Response.xml')) == {
            'antrags_id': 'br1271mrfht6w750vcjwkhfd4qvgv7hf',
            'antrags_status': 'abgelehnt',
        }
