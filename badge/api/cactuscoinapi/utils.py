import json
from flask import request, jsonify
from flask import current_app as app
from flask_restful import abort
from . import crypto

def signed_jsonify(data):
    message = json.dumps(data)
    sig = crypto.sign(message, app.conference_key)
    response = {'msg':message, 'sig':crypto.sign(message, app.conference_key)}
    return jsonify(response)

def get_signed_json(badge_id):
    json_data = request.get_json()
    try:
        crypto.verify(json_data['msg'], json_data['sig'], app.badge_keys[badge_id])
    except crypto.InvalidMessageSignature:
        msg = 'Invalid signature supplied for badge #{}'.format(badge_id)
        abort(403, message=signed_jsonify({'status':403, 'message':msg}))
    return json.loads(json_data['msg'])
