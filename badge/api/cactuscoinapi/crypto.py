import base64
import os
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import padding, utils
from cryptography.exceptions import InvalidSignature

class InvalidMessageSignature(Exception):
    pass

def load_public_keys_from_dir(path):
    keys = {}
    for k in os.listdir(path):
        try:
            badge_id = int(k.strip('.pem'))
        except ValueError:
            raise ValueError('{} is not in the form of BADGE_ID.pem'.format(k))

        keys[badge_id] = load_public_key(k)

    return keys

def load_private_key(key_bytes):
    return serialization.load_pem_private_key(
        key_bytes,
        password=None,
        backend=default_backend()
    )

def load_public_key(key_bytes):
    return serialization.load_pem_public_key(
        key_bytes,
        backend=default_backend()
    )

def load_private_key_from_file(path):
    with open(path, "rb") as key_file:
        return load_private_key(key_file.read())

def load_public_key_from_file(path):
    with open(path, "rb") as key_file:
        return load_public_key(key_file.read())

def sign(msg, private_key):
    if isinstance(msg, str):
        msg = msg.encode('utf8')

    chosen_hash = hashes.SHA256()
    hasher = hashes.Hash(chosen_hash, default_backend())
    hasher.update(msg)
    digest = hasher.finalize()
    sig = private_key.sign(
        digest,
        padding.PKCS1v15(),
        utils.Prehashed(chosen_hash)
    )
    return base64.b64encode(sig).decode('utf8')    

def verify(msg, sig64, public_key):
    if isinstance(msg, str):
        msg = msg.encode('utf8')
    
    sig = base64.b64decode(sig64)
    chosen_hash = hashes.SHA256()
    hasher = hashes.Hash(chosen_hash, default_backend())
    hasher.update(msg)
    digest = hasher.finalize()

    try:
        public_key.verify(
            sig,
            digest,
            padding.PKCS1v15(),
            utils.Prehashed(chosen_hash)
        )
    except InvalidSignature as ex:
        raise InvalidMessageSignature from ex
