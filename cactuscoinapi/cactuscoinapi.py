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

        try:
            self.validate(message)
        except ValueError as ex:
            abort(400, message=signed_jsonify({'status':400, 'message':str(ex)}))
        redis_store.hset('badge_names', badge_id, message['name'])

    def validate(self, badge_dict):
        for key in badge_dict:
            try:
                badge_dict[key].strip()
            except AttributeError:
                continue
        try:
            if not str.isalnum(badge_dict['name']):
                raise ValueError('Name must be alpha numeric')
        except KeyError:
            raise ValueError('Name must be set')

class Coin(Resource):
    def post(self, badge_id):
        coin = get_signed_json(badge_id)

        # validate signatures
        self.validate(coin)

        # check if a coin already exists

        # save coin

    def validate(self, coin):
        csr = json.loads(coin['csr'])
        beacon_sig = coin.pop('beacon_sig')
        # validate beacon signature (coin, beacon_sig)
        # validate seer signature (csr, coin['seer_sig'])

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
    message = json.dumps(data)
    sig = crypto.sign(message, conference_key)
    response = {'msg':message, 'sig':crypto.sign(message, conference_key)}
    return jsonify(response)

def get_signed_json(badge_id):
    json_data = request.get_json()
    try:
        crypto.verify(json_data['msg'], json_data['sig'], badge_keys[badge_id])
    except crypto.InvalidMessageSignature:
        msg = 'Invalid signature supplied for badge #{}'.format(badge_id)
        abort(403, message=signed_jsonify({'status':403, 'message':msg}))
    return json.loads(json_data['msg'])

def load_badge_keys():
    key_files = redis_store.hgetall('badge_keys')
    for badge_id in key_files:
        badge_keys[int(badge_id)] = crypto.load_public_key(key_files[badge_id])

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
        with open('test_badge1_public.pem', 'rb') as key_file:
            redis_store.hset('badge_keys', 1, key_file.read())
        with open('test_badge2_public.pem', 'rb') as key_file:
            redis_store.hset('badge_keys', 2, key_file.read())

    conference_key = crypto.load_private_key_from_file('test_conference_keypair.pem')
    load_badge_keys()

    api.add_resource(Badge, '/badge/<int:badge_id>')
    api.add_resource(Coin, '/coin/<int:badge_id>')
    api.add_resource(CoinList, '/coinlist/<int:badge_id>')
    api.add_resource(Leaderboard, '/leaderboard')

    return app

if __name__ == '__main__':
    app = create_app()
    app.run(debug=True)
