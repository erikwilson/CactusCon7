import json
from utils import post_message,get_message

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
