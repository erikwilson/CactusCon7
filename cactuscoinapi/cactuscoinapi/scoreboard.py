from flask_restful import abort, Resource
from .utils import get_signed_json, signed_jsonify

class Leaderboard(Resource):
    def get(self):
        pass
