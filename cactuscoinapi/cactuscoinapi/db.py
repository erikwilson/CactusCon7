from flask import current_app as app
from flask_redis import FlaskRedis

def init_db():
    if app.testing:
        import mockredis
        redis_store = FlaskRedis.from_custom_provider(mockredis.mock_redis_client())
    else:
        redis_store = FlaskRedis() # TODO: probably need to handle some config settings here

    redis_store.init_app(app)
    app.redis_store = redis_store
