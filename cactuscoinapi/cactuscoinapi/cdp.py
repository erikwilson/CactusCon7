from . import crypto
import struct

SIGNATURE_LENGTH=256

class AbstractBaseMessage():
    def to_bytes(self):
        raise NotImplemented()

    def from_bytes(self, raw_bytes):
        raise NotImplemented()

    def __repr__(self):
        v = ','.join('{}={}'.format(k,v) for k,v in self.__dict__.items())
        return "{}({})".format(type(self).__name__, v)

class RegistrationUnderBadge(AbstractBaseMessage):
    def __init__(self, name):
        self.name = name

    @staticmethod
    def from_bytes(raw_bytes):
        length = struct.unpack('!B', raw_bytes[:1])[0]
        name = raw_bytes[1:length+1].decode('utf8')
        return RegistrationUnderBadge(name)

    def to_bytes(self):
        return struct.pack('!B', len(self.name)) + self.name.encode('utf8')

class GlobalBroadcastPayload(AbstractBaseMessage):
    @staticmethod
    def from_bytes(raw_bytes):
        return GlobalBroadcastPayload()

    def to_bytes(self):
        return b''

class CactuscoinSigningRequest(AbstractBaseMessage):
    def __init__(self, csr_id, broadcasted_id, signing_key=None, csr_signature=None):
        self.csr_id = csr_id
        self.broadcasted_id = broadcasted_id
        self.signing_key = signing_key
        self.csr_signature = csr_signature

    @staticmethod
    def from_bytes(raw_bytes):
        csr_id, broadcasted_id = struct.unpack('!HH', raw_bytes[:4])
        csr_signature = raw_bytes[4:SIGNATURE_LENGTH + 4]
        return CactuscoinSigningRequest(csr_id, broadcasted_id, csr_signature=csr_signature)

    def to_bytes(self):
        raw_bytes = struct.pack('!HH', self.csr_id, self.broadcasted_id)
        try:
            self.csr_signature = crypto.sign(raw_bytes, self.signing_key)
        except AttributeError as ex:
            raise AttributeError('Signing key not set, cannot create a CSR without it!') from ex
        return raw_bytes + self.csr_signature

class CactuscoinAuthenticationData(AbstractBaseMessage):
    def __init__(self, csr_id, broadcasted_id, csr_signature, signing_key=None, 
            broadcast_signature=None):
        self.csr_id = csr_id
        self.broadcasted_id = broadcasted_id
        self.csr_signature = csr_signature
        self.signing_key = signing_key
        self.broadcast_signature = broadcast_signature

    @staticmethod
    def from_bytes(raw_bytes):
        csr_id, broadcasted_id = struct.unpack('!HH', raw_bytes[:4])
        csr_signature = raw_bytes[4:SIGNATURE_LENGTH + 4]
        broadcast_sig_start = SIGNATURE_LENGTH + 4
        broadcast_sig_end = broadcast_sig_start + SIGNATURE_LENGTH + 4
        broadcast_signature = raw_bytes[broadcast_sig_start:broadcast_sig_end]
        return CactuscoinAuthenticationData(csr_id, broadcasted_id, csr_signature, 
                broadcast_signature=broadcast_signature)

    def to_bytes(self):
        raw_bytes = struct.pack('!HH', self.csr_id, self.broadcasted_id) + self.csr_signature
        try:
            self.broadcast_signature = crypto.sign(raw_bytes, self.signing_key)
        except AttributeError as ex:
            raise AttributeError('Signing key not set, cannot create a CAD without it!') from ex
        return raw_bytes + self.broadcast_signature

    def validate(self, public_keys):
        signed_bytes = struct.pack('!HH', self.csr_id, self.broadcasted_id)
        crypto.verify(signed_bytes, self.csr_signature, public_keys[self.csr_id])
        crypto.verify(signed_bytes + self.csr_signature, self.broadcast_signature, 
                public_keys[self.broadcasted_id])

class AutomaticTunnelMessage():
    def __init__(self, tunnelled_message):
        self.tunnelled_message = tunnelled_message
        self.length = len(tunnelled_message)

    @staticmethod
    def from_bytes(raw_bytes):
        length = struct.unpack('!H', raw_bytes[:2])[0]
        tunnelled_message = raw_bytes[2:length + 2]
        return AutomaticTunnelMessage(tunnelled_message)

    def to_bytes(self):
        return struct.pack('!H', len(self.tunnelled_message)) + self.tunnelled_message

class CactuscoinDataPacket():
    type_mapping = {1:RegistrationUnderBadge,
                    2:GlobalBroadcastPayload,
                    4:CactuscoinSigningRequest,
                    5:CactuscoinAuthenticationData,
                    6:AutomaticTunnelMessage,
                   }

    def __init__(self, badge_id, message, version=1, signature=None):
        self.message = message
        self.badge_id = badge_id
        self.version = version
        self.type = self.type_from_object(message)
        self.signature = signature

    @staticmethod
    def from_bytes(raw_bytes, public_keys):
        version, badge_id, msg_type, msg_len = struct.unpack('!BHBH', raw_bytes[:6])
        signature = raw_bytes[6:SIGNATURE_LENGTH + 6]
        message_bytes = raw_bytes[SIGNATURE_LENGTH + 6:SIGNATURE_LENGTH + 6 + msg_len]
        validate_bytes = raw_bytes[:6] + message_bytes
        crypto.verify(validate_bytes, signature, public_keys[badge_id])

        message = CactuscoinDataPacket.type_mapping[msg_type].from_bytes(message_bytes)
        cdp = CactuscoinDataPacket(badge_id, message, version)
        cdp.signature = signature # this sucks, clean up later
        return cdp

    def to_bytes(self, private_key):
        message_bytes = self.message.to_bytes()
        message_len = len(message_bytes)
        raw_bytes = struct.pack('!BHBH', self.version, self.badge_id, self.type, message_len)
        self.signature = crypto.sign(raw_bytes + message_bytes, private_key) # this sucks, clean up later
        return raw_bytes + self.signature + message_bytes

    def __repr__(self):
        msg = "CactuscoinDataPacket"
        msg += "(version={},badge_id={},type={},message={})".format(self.version,
                self.badge_id, self.type, repr(self.message))
        return msg

    def type_from_object(self, obj):
        return [x for x in self.type_mapping if isinstance(obj, self.type_mapping[x])][0]
