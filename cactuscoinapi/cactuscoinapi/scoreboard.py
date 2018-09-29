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

        redis_pipe = app.redis_store.pipeline()
        for s in scores:
            redis_pipe.hget('badge_names', s[0])

        results = redis_pipe.execute()
        scores = [(x[0], y.decode('utf8'), x[1]) for x,y in zip(scores, results)]

        return jsonify({'scoreboard':scores})
