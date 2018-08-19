import json
from flask import Flask, request, jsonify
from flask_restful import reqparse, abort, Resource, Api
from flask_redis import FlaskRedis
from mockredis import MockRedis
import crypto

redis_store = None
conference_key = None
badge_keys = {}

class Badge(Resource):
    def get(self, badge_id):
        return signed_jsonify({'name': redis_store.hget('badge_names', badge_id)})

    def post(self, badge_id):
        message = get_signed_json(badge_id)
        redis_store.hset('badge_names', badge_id, message['name'].strip())

class Coin(Resource):
    def post(self, badge_id):
        pass

class CoinList(Resource):
    def get(self, badge_id):
        pass
        
class Leaderboard(Resource):
    def get(self):
        pass

class MockRedisWrapper(MockRedis):
    '''A wrapper to add the `from_url` classmethod'''
    @classmethod
    def from_url(cls, *args, **kwargs):
        return cls()

def signed_jsonify(data):
    message = json.dumps(data).encode('utf8')
    response = {'msg':message, 'sig':crypto.sign(message, conference_key)}
    return jsonify(response)

def get_signed_json(badge_id):
    json_data = request.get_json()
    crypto.verify(badge_keys[badge_id], json_data['msg'], json_data['sig'])
    return json.loads(json_data['msg'])

def load_badge_keys():
    key_files = redis_store.hgetall('badge_keys')
    for badge_id in key_files:
        badge_keys[int(badge_id)] = crypto.load_public_key(key_files[badge_id].decode('utf8'))

def create_app():
    global redis_store, conference_key
    app = Flask(__name__)
    api = Api(app)
    app.testing = True

    if app.testing:
        redis_store = FlaskRedis.from_custom_provider(MockRedisWrapper)
    else:
        redis_store = FlaskRedis()
    redis_store.init_app(app)

    if app.testing:
        with open('test_badge_public.pem', 'rb') as key_file:
            redis_store.hset('badge_keys', 1, key_file.read())

    conference_key = crypto.load_private_key_from_file('test_conference_keypair.pem')
    load_badge_keys()

    api.add_resource(Badge, '/badge/<int:badge_id>')
    api.add_resource(Coin, '/csr/<int:badge_id>')
    api.add_resource(CoinList, '/coinlist/<int:badge_id>')
    api.add_resource(Leaderboard, '/leaderboard')

    return app

if __name__ == '__main__':
    app = create_app()
    app.run(debug=True)
