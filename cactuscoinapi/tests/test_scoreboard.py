import json
from utils import post_message,get_message

def test_get_scoreboard(client, badge_keys, valid_cactuscoin):
    """ Test to successfully register badge, and also update badge information. """
    badge = {'name':'cybaix'}
    post_message(client, badge_keys[1], '/badge/1', badge)

    post_message(client, badge_keys[1], '/coin/1', valid_cactuscoin)
    response = client.get('/scoreboard').get_json()    

    assert response['scoreboard'][0][0] == '1'
    assert response['scoreboard'][0][1] == 2.0
    assert response['scoreboard'][1][0] == '2'
    assert response['scoreboard'][1][1] == 1.0
