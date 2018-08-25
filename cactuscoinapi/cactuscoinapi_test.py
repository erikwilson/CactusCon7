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
def conference_key():
    return crypto.load_public_key_from_file('test_conference_public.pem')

@pytest.fixture
def badge_keys():
    keys = {}
    keys[1] = crypto.load_private_key_from_file('test_badge1_keypair.pem')
    keys[2] = crypto.load_private_key_from_file('test_badge2_keypair.pem')
    return keys

@pytest.fixture
def badge_pub_keys():
    keys = {}
    keys[1] = crypto.load_public_key_from_file('test_badge1_public.pem')
    keys[2] = crypto.load_public_key_from_file('test_badge2_public.pem')
    return keys

def post_message(client, key, url, message):
    """ Helper function to simplify posting JSON to the cactuscoin rest API, and create
    a signature with the supplied test badge key. """ 
    message_json = json.dumps(message)
    sig = crypto.sign(message_json.encode('utf8'), key)
    data = {'msg':message_json, 'sig':sig}
    return client.post(url, data=json.dumps(data), content_type='application/json')

def get_message(client, key, url):
    """ Helper function to simplify pulling JSON from a specified URL/endpoint and validating
    the conference key signature. """
    response = client.get(url).get_json()
    crypto.verify(response['msg'].encode('utf8'), response['sig'], key)
    message = json.loads(response['msg'])
    return message

def test_register_and_update_badge(client, badge_keys, conference_key):
    """ Test to successfully register badge, and also update badge information. """
    badge = {'name':'cybaix'}
    response = post_message(client, badge_keys[1], '/badge/1', badge)
    assert response.status_code == 200
    saved_badge = get_message(client, conference_key, '/badge/1')
    assert saved_badge == badge

    # Test 'doh' scenario and ensure attendees can update information
    badge = {'name':'mark'}
    response = post_message(client, badge_keys[1], '/badge/1', badge)
    assert response.status_code == 200
    saved_badge = get_message(client, conference_key, '/badge/1')
    assert saved_badge == badge

def test_register_someone_elses_badge(client, badge_keys, conference_key):
    """ Ensures a badge cannot register with an ID not associated with it. """
    badge = {'name':'cybaix'}
    response = post_message(client, badge_keys[2], '/badge/1', badge)
    assert response.status_code == 403

    # make sure the real badge owner can still claim the badge
    badge = {'name':'cybaix'}
    response = post_message(client, badge_keys[1], '/badge/1', badge)
    assert response.status_code == 200
    saved_badge = get_message(client, conference_key, '/badge/1')
    assert saved_badge == badge

def test_register_badge_bad_data(client, badge_keys, conference_key):
    """ Attempts to register a badge with a bunch of bogus data. """
    badge = {'name':'cybaix'}
    response = post_message(client, badge_keys[1], '/badge/a2_+9U*Eji2*Y*AOINHF', badge)
    assert response.status_code == 404

    badge = {'name':'cybaix'}
    response = post_message(client, badge_keys[1], '/badge/a_\/', badge)
    assert response.status_code == 404

    badge = {'name':'`^&%'}
    response = post_message(client, badge_keys[1], '/badge/1', badge)
    assert response.status_code == 400

def test_submit_coin(client, badge_pub_keys, badge_keys):
    badge1_msg = json.dumps({'beacon_id': 1, 'seer_id':2}) # badge 1 beaconed, badge 2 did the CSR
    badge1_csr = {'csr':badge1_msg, 'seer_sig':crypto.sign(badge1_msg, badge_keys[2])}
    badge2_sig = crypto.sign(json.dumps(badge1_csr), badge_keys[1])
    coin = badge1_csr
    coin['beacon_sig'] = badge2_sig

    response = post_message(client, badge_keys[1], '/coin/1', coin)
    assert response.status_code == 200

    response = post_message(client, badge_keys[2], '/coin/2', coin)
    assert response.status_code == 409
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

