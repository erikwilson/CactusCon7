import json
import pytest

from cactuscoinapi import crypto
from cactuscoinapi import create_app

@pytest.fixture
def client(request, shared_datadir):
    config = {'CONFERENCE_PRIVATE_KEY': str(shared_datadir / 'test_conference_keypair.pem'),
              'TEST_BADGE_PUBLIC_KEYS': {1:str(shared_datadir / 'test_badge1_public.pem'),
                                         2:str(shared_datadir / 'test_badge2_public.pem')
                                        }
             }
    app = create_app(config)
    test_client = app.test_client()

    def teardown():
        pass # nothing to tear down yet

    request.addfinalizer(teardown)
    return test_client

@pytest.fixture
def conference_key(shared_datadir):
    return crypto.load_public_key_from_file(str(shared_datadir / 'test_conference_public.pem'))

@pytest.fixture
def badge_keys(shared_datadir):
    keys = {}
    keys[1] = crypto.load_private_key_from_file(str(shared_datadir / 'test_badge1_keypair.pem'))
    keys[2] = crypto.load_private_key_from_file(str(shared_datadir / 'test_badge2_keypair.pem'))
    return keys

@pytest.fixture
def badge_pub_keys(shared_datadir):
    keys = {}
    keys[1] = crypto.load_public_key_from_file(str(shared_datadir / 'test_badge1_public.pem'))
    keys[2] = crypto.load_public_key_from_file(str(shared_datadir / 'test_badge2_public.pem'))
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

