from flask import current_app as app, jsonify
from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify

class Scoreboard(Resource):
    def get(self):
        """ Provides a sorted list of current scores for each registered badge in the game. 

        Returns:
          scoreboard: multi-dimensional array of scores and badge ids, sorted in descending order by score.
        """
        # TODO: This still needs to get badge names
        scores = app.redis_store.zrevrangebyscore('scoreboard', '+inf', '-inf', withscores=True)
        scores = [(x[0].decode('utf8'), x[1]) for x in scores]
        return jsonify({'scoreboard':scores})
