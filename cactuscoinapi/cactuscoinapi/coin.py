import json
from flask import current_app as app
from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify
from .crypto import InvalidMessageSignature, verify

class Coin(Resource):
    def post(self, badge_id):
        coin = get_signed_json(badge_id)

        # validate signatures
        try:
            csr = self.validate(coin)
        except InvalidMessageSignature:
            msg = 'Invalid signatures on coin'
            abort(403, message=signed_jsonify({'status':403, 'message':msg}))

        # check if a coin already exists
        keys = sorted((csr['beacon_id'], csr['seer_id']))

        if app.redis_store.sismember('coins', keys):
            abort(409, message=signed_jsonify({'status':208, 'message':'coin already submitted'}))

        app.redis_store.sadd('coins', keys)

        # save coin

    def validate(self, coin):
        csr = json.loads(coin['csr'])
        beacon_sig = coin.pop('beacon_sig')
        # validate beacon signature (coin, beacon_sig)
        verify(json.dumps(coin), beacon_sig, app.badge_keys[csr['beacon_id']])
        # validate seer signature (csr, coin['seer_sig'])
        verify(coin['csr'], coin['seer_sig'], app.badge_keys[csr['seer_id']])
        return csr
