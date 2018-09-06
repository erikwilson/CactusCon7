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
    def __init__(self, csr_id, broadcasted_id):
        self.csr_id = csr_id
        self.broadcasted_id = broadcasted_id

    @staticmethod
    def from_bytes(raw_bytes):
        csr_id, broadcasted_id = struct.unpack('!HH', raw_bytes[:4])
        return CactuscoinSigningRequest(csr_id, broadcasted_id)

    def to_bytes(self):
        return struct.pack('!HH', self.csr_id, self.broadcasted_id)

class CactuscoinAuthenticationData(AbstractBaseMessage):
    def __init__(self, csr_id, broadcasted_id, csr_signature):
        self.csr_id = csr_id
        self.broadcasted_id = broadcasted_id
        self.csr_signature = csr_signature

    @staticmethod
    def from_bytes(raw_bytes):
        csr_id, broadcasted_id = struct.unpack('!HH', raw_bytes[:4])
        csr_signature = data[4:SIGNATURE_LENGTH + 4]
        return CactuscoinAuthenticationData(csr_id, broadcasted_id, csr_signature)

    def to_bytes(self):
        raw_bytes = struct.pack('!HH', self.csr_id, self.broadcasted_id)
        return raw_bytes + self.csr_signature

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
        print (self.tunnelled_message, len(self.tunnelled_message))
        return struct.pack('!h', len(self.tunnelled_message)) + self.tunnelled_message

class CactuscoinDataPacket():
    type_mapping = {1:RegistrationUnderBadge,
                    2:GlobalBroadcastPayload,
                    4:CactuscoinSigningRequest,
                    5:CactuscoinAuthenticationData,
                    6:AutomaticTunnelMessage,
                   }

    def __init__(self, badge_id, message, version=1):
        self.message = message
        self.badge_id = badge_id
        self.version = version
        self.type = self.type_from_object(message)

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
        msg += "(version={},type={},message={})".format(self.version,
                self.type, repr(self.message))
        return msg

    def type_from_object(self, obj):
        return [x for x in self.type_mapping if isinstance(obj, self.type_mapping[x])][0]
