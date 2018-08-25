from flask import current_app as app
from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify

class Badge(Resource):
    def get(self, badge_id):
        name = app.redis_store.hget('badge_names', badge_id).decode('utf8')
        return signed_jsonify({'name': name})

    def post(self, badge_id):
        message = get_signed_json(badge_id)

        try:
            self.validate(message)
        except ValueError as ex:
            abort(400, message=signed_jsonify({'status':400, 'message':str(ex)}))
        app.redis_store.hset('badge_names', badge_id, message['name'])

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
