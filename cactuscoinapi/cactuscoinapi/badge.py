from flask import current_app as app
from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify

class Badge(Resource):
    def get(self, badge_id):
        name = app.redis_store.hget('badge_names', badge_id).decode('utf8')
        return signed_jsonify({'name': name})

    def post(self, badge_id):
        """ Registers the badge with the supplied name if it doesn't already exist.  If it does
        update the name with the one supplied.

        Arguments:
          name: Alpha numeric name to associate with this badge.
        """
        message = get_signed_json(badge_id)

        try:
            self.validate(message)
        except ValueError as ex:
            abort(400, message=signed_jsonify({'status':400, 'message':str(ex)}))

        redis_pipe = app.redis_store.pipeline()
        redis_pipe.hset('badge_names', badge_id, message['name'])

        if app.redis_store.hget('badge_names', badge_id) is None:
            redis_pipe.zincrby('scoreboard', badge_id)

        redis_pipe.execute()

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
