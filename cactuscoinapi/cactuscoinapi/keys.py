from flask import current_app as app
from . import crypto

def init_keys():
    conference_key = crypto.load_private_key_from_file(app.config['CONFERENCE_PRIVATE_KEY'])

    if app.testing:
        with open(app.config['TEST_BADGE_PUBLIC_KEYS'][1], 'rb') as key_file:
            app.redis_store.hset('badge_keys', 1, key_file.read())
        with open(app.config['TEST_BADGE_PUBLIC_KEYS'][2], 'rb') as key_file:
            app.redis_store.hset('badge_keys', 2, key_file.read())
    
    key_files = app.redis_store.hgetall('badge_keys')
    badge_keys = {}
    for badge_id in key_files:
        badge_keys[int(badge_id)] = crypto.load_public_key(key_files[badge_id])

    app.badge_keys = badge_keys
    app.conference_key = conference_key
