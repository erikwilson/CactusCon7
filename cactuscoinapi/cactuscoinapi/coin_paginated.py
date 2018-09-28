import json
import struct
import time
import base64

from flask import current_app as app
from flask import request
from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify
from .crypto import InvalidMessageSignature, verify

class CoinList(Resource):

    def post(self, badge_id):
        request = get_signed_json(badge_id)

        # validate signatures
        try:
            self.validate(request)
        except ValueError as ex:
            abort(400, message=signed_jsonify({'status':400, 'message':str(ex)}))

        start = request['start']
        num_badges = request['numBadges'] - 1 # stupid redis lists are inclusive at the end

        badges = app.redis_store.lrange('badge_coins_{}'.format(badge_id), start, start + num_badges)

        try:
            badges = [int(i) for i in badges]
        except (ValueError, TypeError):
            msg = 'no coins yet.. stop being so anti-social'
            abort(404, message=signed_jsonify({'status':404, 'message':msg}))

        return signed_jsonify(badges)

    def validate(self, badge_dict):
        try:
            badge_dict['start'] = int(badge_dict['start'])
        except KeyError:
            raise ValueError('start must be set')
        except ValueError as ex:
                raise ValueError('start must be numeric') from ex

        try:
            badge_dict['numBadges'] = int(badge_dict['numBadges'])
        except KeyError:
            raise ValueError('numBadges must be set')
        except ValueError as ex:
                raise ValueError('numBadges must be numeric') from ex
