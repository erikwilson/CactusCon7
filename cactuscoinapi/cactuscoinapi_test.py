import json
import pytest

import crypto

from cactuscoinapi import create_app

@pytest.fixture
def client(request):
    app = create_app()
    test_client = app.test_client()

    def teardown():
        pass # nothing to tear down yet

    request.addfinalizer(teardown)
    return test_client

@pytest.fixture
def keys():
    keys = {}
    keys['conference'] = crypto.load_public_key_from_file('test_conference_public.pem')
    keys['badge'] = crypto.load_private_key_from_file('test_badge_keypair.pem')
    return keys

def post_message(client, keys, url, message):
    """ Helper function to simplify posting JSON to the cactuscoin rest API, and create
    a signature with the test badge key. """ 
    message_json = json.dumps(message).encode('utf8')
    data = {'msg':message_json, 'sig':crypto.sign(message_json, keys['badge'])}
    return client.post(url, data=json.dumps(data).encode('utf8'), content_type='application/json')

def get_message(client, keys, url):
    """ Helper function to simplify pulling JSON from a specified URL/endpoint and validating
    the conference key signature. """
    response = client.get(url).get_json()
    crypto.verify(response['msg'].encode('utf8'), response['sig'], keys['conference'])
    message = json.loads(response['msg'])
    return message

def test_register_badge(client, keys):
    """ Test to successfully register badge, then re-register which should always fail. """
    badge = {'name':'cybaix'}
    response = post_message(client, keys, '/badge/1', badge)
    assert response.status_code == 200

    saved_badge = get_message(client, keys, '/badge/1')
    assert saved_badge == badge

    # Attempt to re-register, this should fail.
    response = post_message(client, keys, '/badge/1', badge)
    assert response.status_code == 403

def test_register_badge_bad_data(client, keys):
    """ Attempts to register a badge with a bunch of bogus data. """
    badge = {'name':'cybaix'}
    response = post_message(client, keys, '/badge/a2_+9U*Eji2*Y*AOINHF', badge)
    assert response.status_code == 404

    badge = {'name':'cybaix'}
    response = post_message(client, keys, '/badge/a_\/', badge)
    assert response.status_code == 404

    badge = {'name':'`^&%'}
    response = post_message(client, keys, '/badge/1', badge)
    assert response.status_code == 400

def test_submit_coin(client):
    pass
    # parings = set(key1,key2)
    # wallets['id'] = current coin count
    # attendees['id'] = json data about attendee
    # coins_'id' = list of coins

def test_get_balance(client):
    pass

def test_get_leaderboard(client):
    pass

#User pops in batteries, prompted by display to type a name or use default.
#Registration associates this key with this name.
#Look up on ID number, grab public key, if signature is valid, register and activate and give 1 point.
#each user has a set of associated keys

#1 beacons, 2 sees it, 2 signs ID1 with key of 2 send to 1 along with it's ID
#1 gets response, signs 1st message with key 1 and sends to 2.
#both submit CSR

#flash:
#    new keypair, push down private, save public
#    push unique number down

