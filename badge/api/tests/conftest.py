import json
import struct
import base64
import pytest
import functools

import mockredis
from flask_redis import FlaskRedis
from cactuscoinapi import crypto
from cactuscoinapi import create_app

def pytest_namespace():
        return {'redis': None}

def pytest_addoption(parser):
        parser.addoption('--real-redis', action='store_true',
                help='run tests using a real instance running at localhost:6379')

@pytest.fixture
def real_redis_opt(request):
    return request.config.getoption("--real-redis")

@pytest.fixture
def client(request, shared_datadir, real_redis_opt, mocker):
    config = {'CONFERENCE_PRIVATE_KEY_FILE': str(shared_datadir / 'test_conference_keypair.pem'),
              'TEST_BADGE_PUBLIC_KEYS': {1:str(shared_datadir / 'test_badge1_public.pem'),
                                         2:str(shared_datadir / 'test_badge2_public.pem')
                                        }
             }

    if real_redis_opt:
        pytest.redis = FlaskRedis()
    else:
        pytest.redis = FlaskRedis.from_custom_provider(mockredis.mock_redis_client())

    def retredis():
        return pytest.redis

    mocker.patch('cactuscoinapi.db.FlaskRedis', retredis)

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

@pytest.fixture
def valid_cactuscoin(badge_pub_keys, badge_keys):
    coinbits = struct.pack('HH', 2, 1)
    coin = {'CSRID':2, 'broadcasterID':1, 'signatureCSR':crypto.sign(coinbits, badge_keys[2])}
    coinbits += base64.b64decode(coin['signatureCSR'])
    badge2_sig = crypto.sign(coinbits, badge_keys[1])
    coin['signatureBroadcaster'] = badge2_sig
    return coin

@pytest.fixture
def invalid_cactuscoin(badge_pub_keys, badge_keys):
    coinbits = struct.pack('HH', 1, 2) # swapped id so signatures won't match
    coin = {'CSRID':2, 'broadcasterID':1, 'signatureCSR':crypto.sign(coinbits, badge_keys[2])}
    coinbits += base64.b64decode(coin['signatureCSR'])
    badge2_sig = crypto.sign(coinbits, badge_keys[1])
    coin['signatureBroadcaster'] = badge2_sig
    return coin

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

