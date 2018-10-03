from flask import current_app as app
from flask_redis import FlaskRedis

def init_db():
    redis_store = FlaskRedis() # TODO: probably need to handle some config settings here
    redis_store.init_app(app)
    app.redis_store = redis_store
