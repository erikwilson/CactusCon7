import json
import time

from flask import current_app as app
from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify
from .crypto import InvalidMessageSignature, verify

class Coin(Resource):
    def get(self, badge_id):
        coins = []
        badges = app.redis_store.smembers('badge_coins_{}'.format(badge_id))

        for other_badge in badges:
            other_id = int(other_badge)
            other_name = app.redis_store.hget('badge_names', other_badge).decode('utf8')
            timestamp = int(app.redis_store.hget('coins', str(sorted((badge_id, other_id)))))
            coins.append({'other_id':other_id, 'other_name':other_name, 'timestamp':timestamp})

        return signed_jsonify(coins)

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

        if app.redis_store.hget('coins', keys):
            abort(409, message=signed_jsonify({'status':208, 'message':'coin already submitted'}))

        app.redis_store.hset('coins', keys, int(time.time()))

        # link badges... TODO: handle failure better, e.g. one badge links, other fails
        app.redis_store.sadd('badge_coins_{}'.format(csr['beacon_id']), csr['seer_id'])  
        app.redis_store.sadd('badge_coins_{}'.format(csr['seer_id']), csr['beacon_id'])  

    def validate(self, coin):
        """ Checks the signatures on the coin for the badges involved. Looks up appropriate badge
        keys based on the IDs supplied in the initial coin signing request (CSR).  Please note
        that it is required for the beacon signature to cover the csr and seer_sig.

        Args:
            coin: JSON message containing the full cactuscoin. Should look something like:
                  {"csr": {"beacon_id":1, "seer_id":2}, "seer_sig":"base64", "beacon_sig":"base64"}

        Raises:
            cactuscoin.crypto.InvalidMessageSignature: When an invalid signature is detected on the coin.
        """
        csr = json.loads(coin['csr'])
        beacon_sig = coin.pop('beacon_sig')
        # validate beacon signature (coin, beacon_sig)
        verify(json.dumps(coin), beacon_sig, app.badge_keys[csr['beacon_id']])
        # validate seer signature (csr, coin['seer_sig'])
        verify(coin['csr'], coin['seer_sig'], app.badge_keys[csr['seer_id']])
        return csr
