import base64
import os
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import padding, utils

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
    chosen_hash = hashes.SHA256()
    hasher = hashes.Hash(chosen_hash, default_backend())
    hasher.update(msg)
    digest = hasher.finalize()
    sig = private_key.sign(
        digest,
        padding.PSS(
            mgf=padding.MGF1(hashes.SHA256()),
            salt_length=padding.PSS.MAX_LENGTH
        ),
        utils.Prehashed(chosen_hash)
    )
    return base64.b64encode(sig)

def verify(msg, sig64, public_key):
    sig = base64.b64decode(sig64)
    chosen_hash = hashes.SHA256()
    hasher = hashes.Hash(chosen_hash, default_backend())
    hasher.update(msg)
    digest = hasher.finalize()
    public_key.verify(
        sig,
        digest,
        padding.PSS(
            mgf=padding.MGF1(hashes.SHA256()),
            salt_length=padding.PSS.MAX_LENGTH
        ),
        utils.Prehashed(chosen_hash)
    )
