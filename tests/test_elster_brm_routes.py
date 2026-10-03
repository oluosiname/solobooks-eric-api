from pathlib import Path

import pytest

import api

FIXTURES = Path(__file__).parent / 'fixtures' / 'elster_brm'

CREDENTIALS = {
    'cert_base64': 'Y2VydGlmaWNhdGU=',
    'password': 'secret-password',
    'hersteller_id': '97263',
}

ANTRAG = {
    **CREDENTIALS,
    'idnr': '04452397687',
    'date_of_birth': '1985-01-01',
    'valid_until': '2031-10-03',
    'datenabrufer_mail': 'elster@solobooks.de',
}


@pytest.fixture
def client():
    return api.app.test_client()


@pytest.fixture
def sent(monkeypatch):
    calls = []

    def answer_with(result):
        def fake(**kwargs):
            calls.append(kwargs)
            if isinstance(result, Exception):
                raise result
            return result
        monkeypatch.setattr(api.eric_client, 'elster_brm', fake)
        return calls

    return answer_with


def antrag_answer():
    return (True, None, None, (FIXTURES / '02a_SpezRechtAntrag_Response.xml').read_text(encoding='utf-8'))


def test_antrag_returns_the_parsed_answer(client, sent):
    sent(antrag_answer())

    response = client.post('/elster_brm/antrag', json=ANTRAG)

    assert response.status_code == 200
    assert response.get_json()['data']['antrags_id'] == 'br12701v299sh650fgwcn0c31z2k0xrb'


def test_antrag_sends_the_built_nutzdaten(client, sent):
    calls = sent(antrag_answer())

    client.post('/elster_brm/antrag', json=ANTRAG)

    assert calls[0]['datenart'] == 'SpezRechtAntrag'
    assert '<DateninhaberGeburtstag>1985-01-01</DateninhaberGeburtstag>' in calls[0]['nutzdaten']


def test_a_german_date_is_refused_before_elster(client, sent):
    calls = sent(antrag_answer())

    response = client.post('/elster_brm/antrag', json={**ANTRAG, 'date_of_birth': '01.01.1985'})

    assert response.status_code == 400
    assert calls == []


def test_a_bad_request_never_echoes_secrets(client, sent):
    sent(antrag_answer())

    response = client.post('/elster_brm/antrag', json={**ANTRAG, 'idnr': ['04452397687'], 'password': {'p': 'secret-password'}})

    assert response.status_code == 400
    assert 'secret-password' not in response.get_data(as_text=True)
    assert '04452397687' not in response.get_data(as_text=True)


def test_an_elster_refusal_is_a_502_with_its_code(client, sent):
    sent((False, 371015235, 'IdNr unbekannt', None))

    response = client.post('/elster_brm/liste', json={**CREDENTIALS, 'idnrs': ['04452397687']})

    assert response.status_code == 502
    assert response.get_json()['error_code'] == 371015235


def test_an_unexpected_error_hides_its_traceback(client, sent):
    sent(RuntimeError('boom secret-password'))

    response = client.post('/elster_brm/storno', json={**CREDENTIALS, 'antrags_id': 'br1'})

    assert response.status_code == 500
    assert response.get_json() == {'error': 'Unexpected error'}
