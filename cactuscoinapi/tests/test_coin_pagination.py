import json
import pytest
from utils import get_message,post_message
from cactuscoinapi import crypto

def test_get_coins(mocker, client, valid_cactuscoin, badge_keys, conference_key):
    pytest.redis.flushdb() # this is a hack for now, need to redo tests to work directly with db
    fake_badges = [i for i in range(3, 200)]
    responded_badges = []
    badge_batch_size = 10
    pos = 0

    for i in range(3, 200):
        pytest.redis.lpush('badge_coins_2', i)

    while True:
        signature = crypto.sign(json.dumps({'start':0, 'numBadges':badge_batch_size}), badge_keys[2])
        message = {'start':pos, 'numBadges':badge_batch_size}
        response = post_message(client, badge_keys[2], '/coin_list/2', message)
        assert response.status_code == 200
        response = json.loads(response.get_json()['msg'])

        for badge in response:
            responded_badges.append(badge)
            
        if len(response) < badge_batch_size:
            break

        pos += badge_batch_size

    responded_badges.reverse()
    assert responded_badges == fake_badges
