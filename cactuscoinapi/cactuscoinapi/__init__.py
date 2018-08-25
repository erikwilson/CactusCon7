import json
from flask import Flask, request, jsonify
from flask_restful import reqparse, abort, Resource, Api
from flask_redis import FlaskRedis
from . import crypto, badge, coin, leaderboard, db, keys

def create_app(test_config=None):
    global conference_key, badge_keys
    app = Flask(__name__)
    api = Api(app)

    if test_config is None:
        app.config.from_pyfile('config.py', silent=True)
        app.testing = False
    else:
        import mockredis
        app.testing = True
        app.config.from_mapping(test_config)

    with app.app_context():
        db.init_db()
        keys.init_keys()

    api.add_resource(badge.Badge, '/badge/<int:badge_id>')
    api.add_resource(coin.Coin, '/coin/<int:badge_id>')
    #api.add_resource(CoinList, '/coinlist/<int:badge_id>')
    api.add_resource(leaderboard.Leaderboard, '/leaderboard')

    return app

if __name__ == '__main__':
    app = create_app()
    app.run(debug=True)
