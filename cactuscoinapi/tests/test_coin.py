import json
from utils import get_message,post_message
from cactuscoinapi import crypto

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
