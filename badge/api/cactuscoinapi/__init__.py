import json
from flask import Flask, request, jsonify
from flask_restful import reqparse, abort, Resource, Api
from flask_redis import FlaskRedis
from flask_cors import CORS
from . import crypto, badge, coin, scoreboard, db, keys, coin_paginated

def create_app(test_config=None):
    global conference_key, badge_keys
    app = Flask(__name__)
    api = Api(app)
    CORS(app)

    #app.config.from_object('yourapplication.default_settings')

    if test_config is None:
        app.config.from_envvar('CACTUSCONAPI_SETTINGS_FILE')
        app.testing = False
    else:
        app.testing = True
        app.config.from_mapping(test_config)

    with app.app_context():
        db.init_db()
        keys.init_keys()

    api.add_resource(badge.Badge, '/badge/<int:badge_id>')
    api.add_resource(coin.Coin, '/coin/<int:badge_id>')
    api.add_resource(scoreboard.Scoreboard, '/scoreboard')
    api.add_resource(coin_paginated.CoinList, '/coin_list/<int:badge_id>')

    return app

if __name__ == '__main__':
    app = create_app()
    app.run(debug=True)
