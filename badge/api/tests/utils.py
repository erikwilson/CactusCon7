import json
from cactuscoinapi import crypto

def post_message(client, key, url, message):
    """ Helper function to simplify posting JSON to the cactuscoin rest API, and create
    a signature with the supplied test badge key. """ 
    message_json = json.dumps(message)
    sig = crypto.sign(message_json.encode('utf8'), key)
    data = {'msg':message_json, 'sig':sig}
    return client.post(url, data=json.dumps(data), content_type='application/json')

def get_message(client, key, url):
    """ Helper function to simplify pulling JSON from a specified URL/endpoint and validating
    the conference key signature. """
    response = client.get(url).get_json()
    crypto.verify(response['msg'].encode('utf8'), response['sig'], key)
    message = json.loads(response['msg'])
    return message
