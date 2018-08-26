import json
from utils import get_message,post_message
from cactuscoinapi import crypto

def test_submit_valid_coin(client, valid_cactuscoin, badge_keys):
    response = post_message(client, badge_keys[1], '/coin/1', valid_cactuscoin)
    assert response.status_code == 200

    response = post_message(client, badge_keys[2], '/coin/2', valid_cactuscoin)
    assert response.status_code == 409

def test_submit_invalid_coin(client, invalid_cactuscoin, badge_keys):
    response = post_message(client, badge_keys[1], '/coin/1', invalid_cactuscoin)
    assert response.status_code == 403

def test_get_coins(mocker, client, valid_cactuscoin, badge_keys, conference_key):
    #mocker.patch(
    post_message(client, badge_keys[2], '/badge/2', {'name':'cybaix'})

    response = post_message(client, badge_keys[1], '/coin/1', valid_cactuscoin)
    assert response.status_code == 200

    coins = get_message(client, conference_key, '/coin/1')
    assert coins[0]['other_id'] == 2
    assert coins[0]['other_name'] == 'cybaix'
