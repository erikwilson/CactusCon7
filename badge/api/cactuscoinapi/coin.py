import json
import struct
import time
import base64

from flask import current_app as app
from flask import request
from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify
from .crypto import InvalidMessageSignature, verify

class Coin(Resource):
    """ The Cactuscoin REST endpoint is responsible for handling the submissions of cactuscoins, 
    as well as provides the ability to get details about the coins that have been submitted.

    A Cactuscoin is generated by proof of proximity.  The proof of proximity works as follows:
        * Badge 1 beacons, badge 2 sees it.
        * Badge 2 creates a coin signing request (CSR) and sends it to badge 1.
           (a CSR is the id of the beaconer and it's own id, signed by the badge 2 private key)
        * Badge 1 recieves the CSR and signs (including badge 2's signature) with it's own key
        * This completes the coin, badge 1 sends to badge 2 and both attempt to submit to a
            cactuscoin node.
    """

    def get(self, badge_id):
        """ Provides a list of all badges (full details) that coins have been created with
        for the supplied badge ID.  Open to the public and does not require authentication.

        Args:
            badge_id: A numeric number indicating which badge to pull the list for.
        Returns:
            JSON list containing the associated IDs, names, and timestamps of the coins
            this badge has generated.  Timestamp is when the coin was first submitted.
        """
        coins = []
        badges = app.redis_store.lrange('badge_coins_{}'.format(badge_id), 0, -1)

        for other_badge in badges:
            other_id = int(other_badge)
            other_name = app.redis_store.hget('badge_names', other_badge).decode('utf8')
            timestamp = int(app.redis_store.hget('coins', str(sorted((badge_id, other_id)))))

            coins.append({'other_id':other_id, 'other_name':other_name, 'timestamp':timestamp})

        return signed_jsonify(coins)

    def post(self, badge_id):
        """ Accepts a new coin submission by the supplied badge ID.  Submission requires a JSON 
        string be posted that contains the full coin.

        Args:
            badge_id: A numeric number indicating which badge to pull the list for.
        Returns:
            200 if coin is valid and accepted.
            403 on coin signature verification error.
            400 if a coin has already been submitted.
        """
        coin = get_signed_json(badge_id)

        # validate signatures
        try:
            self.validate(coin)
        except InvalidMessageSignature:
            msg = 'Invalid signatures on coin'
            return {'status':403, 'message':msg}

        # TODO: Could use some redis data structure clean up, there is better ways to do this
        # check if a coin already exists
        keys = sorted((coin['broadcasterID'], coin['CSRID']))

        if app.redis_store.hget('coins', keys):
            balance = int(app.redis_store.llen('badge_coins_{}'.format(badge_id)))
            return signed_jsonify({'status':409, 'message':'coin already submitted', 'balance':balance})

        redis_pipe = app.redis_store.pipeline() # complete the following redis operations atomically
        redis_pipe.hset('coins', keys, int(time.time()))
        redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
        redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
        redis_pipe.zincrby('scoreboard', coin['broadcasterID'])
        redis_pipe.zincrby('scoreboard', coin['CSRID'])

        if coin['broadcasterID'] > 3000:
            redis_pipe.zincrby('scoreboard', coin['CSRID'], 9)
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['CSRID']), coin['broadcasterID'])  

        if coin['CSRID'] > 3000:
            redis_pipe.zincrby('scoreboard', coin['broadcasterID'], 9)
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  
            redis_pipe.lpush('badge_coins_{}'.format(coin['broadcasterID']), coin['CSRID'])  

        redis_pipe.execute()

        balance = int(app.redis_store.llen('badge_coins_{}'.format(badge_id)))
        return signed_jsonify({'status':200, 'mesage':'go go gadget socialization', 'balance':balance})

        #scoreboard sortedset 
        #badge_N [name] [
        #coin_1,2 [generated timestamp] [submitted timestamp]
        #badge_values

    def validate(self, coin):
        """ Checks the signatures on the coin for the badges involved. Looks up appropriate badge
        keys based on the IDs supplied in the initial coin signing request (CSR).  Please note
        that it is required for the beacon signature to cover the csr and seer_sig.

        Args:
            coin: JSON message containing the full cactuscoin. Should look something like:
                  {"CSRID": 1, "broadcasterID": 2, "signatureCSR": "base64", signatureBroadcaster: "base64"}

        Raises:
            cactuscoin.crypto.InvalidMessageSignature: When an invalid signature is detected on the coin.
        """
        #csr = json.loads(coin['csr'])
        #beacon_sig = coin.pop('beacon_sig')
        # validate beacon signature (coin, beacon_sig)

        coinbits = struct.pack('HH', coin['CSRID'], coin['broadcasterID'])
        verify(coinbits, coin['signatureCSR'], app.badge_keys[coin['CSRID']])
        coinbits = coinbits + base64.b64decode(coin['signatureCSR'])
        verify(coinbits, coin['signatureBroadcaster'], app.badge_keys[coin['broadcasterID']])
