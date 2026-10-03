import pytest

import api

SUBMISSION = {
    'xml': '<Elster/>',
    'cert_base64': 'Y2VydGlmaWNhdGU=',
    'password': 'secret-password',
}

DATENABHOLUNG = {
    'idnr': '04452397687',
    'year': 2025,
    'cert_base64': 'Y2VydGlmaWNhdGU=',
    'password': 'secret-password',
    'hersteller_id': '97263',
}


@pytest.fixture
def client():
    return api.app.test_client()


@pytest.mark.parametrize('route, payload, field, secret', [
    ('/validate', {'xml': {'Nutzdaten': 'REAL_TAXPAYER_DATA'}}, 'xml', 'REAL_TAXPAYER_DATA'),
    ('/submit', {**SUBMISSION, 'password': {'secret': 'SuperSecret123'}}, 'password', 'SuperSecret123'),
    ('/submit', {**SUBMISSION, 'cert_base64': ['AAAA_SECRET_CERT_BYTES']}, 'cert_base64', 'AAAA_SECRET_CERT_BYTES'),
    ('/submit', {**SUBMISSION, 'xml': {'Nutzdaten': 'REAL_TAXPAYER_DATA'}}, 'xml', 'REAL_TAXPAYER_DATA'),
    ('/datenabholung', {**DATENABHOLUNG, 'password': {'secret': 'SuperSecret123'}}, 'password', 'SuperSecret123'),
    ('/datenabholung', {**DATENABHOLUNG, 'cert_base64': ['AAAA_SECRET_CERT_BYTES']}, 'cert_base64', 'AAAA_SECRET_CERT_BYTES'),
])
def test_bad_request_names_the_field_without_its_value(client, route, payload, field, secret):
    response = client.post(route, json=payload)

    assert response.status_code == 400
    assert field in response.json['error']
    assert secret not in response.get_data(as_text=True)


@pytest.mark.parametrize('route, payload, method', [
    ('/validate', {'xml': '<Elster/>'}, 'validate_xml'),
    ('/submit', SUBMISSION, 'submit_xml'),
    ('/datenabholung', DATENABHOLUNG, 'datenabholung'),
])
def test_unexpected_error_keeps_the_traceback_server_side(client, monkeypatch, capsys, route, payload, method):
    def explode(*args, **kwargs):
        raise RuntimeError('secret-password')

    monkeypatch.setattr(api.eric_client, method, explode)

    response = client.post(route, json=payload)

    assert response.status_code == 500
    assert response.json == {'error': 'Unexpected error'}
    assert 'Traceback' in capsys.readouterr().out
