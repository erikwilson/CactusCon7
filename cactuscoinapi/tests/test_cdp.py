import json
import pytest
from utils import get_message,post_message
from cactuscoinapi import cdp

def test_registration_under_badge():
    cdp_message = cdp.RegistrationUnderBadge(name='cybaix')
    assert cdp_message.to_bytes() == b'\x06cybaix'
    cdp_message = cdp.RegistrationUnderBadge.from_bytes(b'\x06cybaix')
    assert cdp_message.name == 'cybaix'

def test_global_broadcast_payload():
    cdp_message = cdp.GlobalBroadcastPayload()
    assert cdp_message.to_bytes() == b''

def test_cactuscoin_signing_request():
    cdp_message = cdp.CactuscoinSigningRequest(csr_id=1, broadcasted_id=2)
    assert cdp_message.to_bytes() == b'\x00\x01\x00\x02'

    cdp_message = cdp.CactuscoinSigningRequest.from_bytes(b'\x00\x02\x00\x01')
    assert cdp_message.csr_id == 2
    assert cdp_message.broadcasted_id == 1

    # Endianness and everything else looks good, let's check that it's unsigned
    cdp_message = cdp.CactuscoinSigningRequest(csr_id=65534, broadcasted_id=2)
    assert cdp_message.to_bytes() == b'\xff\xfe\x00\x02'

    cdp_message = cdp.CactuscoinSigningRequest.from_bytes(b'\xff\xfe\x00\x02')
    assert cdp_message.csr_id == 65534
    assert cdp_message.broadcasted_id == 2

def test_cactuscoin_authentication_data(badge_keys, badge_pub_keys):
    cdp_message = cdp.CactuscoinSigningRequest(csr_id=1, broadcasted_id=2)
    cdp_packet = cdp.CactuscoinDataPacket(1, cdp_message)
    cdp_packet.to_bytes(badge_keys[1])

    cdp_message = cdp.CactuscoinAuthenticationData(csr_id=1, broadcasted_id=2, 
            csr_signature=cdp_packet.signature)
    #cdp_packet = cdp.CactuscoinDataPacket(2, cdp_message)

    print (cdp_packet)
    assert False


def test_cactuscoin_data_packet(badge_keys, badge_pub_keys):
    cdp_message = cdp.RegistrationUnderBadge(name='cybaix')
    cdp_packet = cdp.CactuscoinDataPacket(1, cdp_message)
    packet_bytes = cdp_packet.to_bytes(badge_keys[1])
    cdp_packet1 = cdp.CactuscoinDataPacket.from_bytes(packet_bytes, badge_pub_keys)

    cdp_message = cdp.CactuscoinSigningRequest(csr_id=1, broadcasted_id=2)
    cdp_packet = cdp.CactuscoinDataPacket(1, cdp_message)
    packet_bytes = cdp_packet.to_bytes(badge_keys[1])
    cdp_packet1 = cdp.CactuscoinDataPacket.from_bytes(packet_bytes, badge_pub_keys)

    cdp_message = cdp.CactuscoinAuthenticationData(csr_id=1, broadcasted_id=2, 
            csr_signature=cdp_packet.signature)
    cdp_packet = cdp.CactuscoinDataPacket(2, cdp_message)
    packet_bytes = cdp_packet.to_bytes(badge_keys[2])

    cdp_packet = cdp.CactuscoinDataPacket.from_bytes(packet_bytes, badge_pub_keys)
    #crypto.validate cdp_packet.message.csr_signature = 

    cdp_message = cdp.AutomaticTunnelMessage(tunnelled_message=packet_bytes)
    cdp_packet = cdp.CactuscoinDataPacket(2, cdp_message)
    packet_bytes = cdp_packet.to_bytes(badge_keys[2])
    cdp_packet1 = cdp.CactuscoinDataPacket.from_bytes(packet_bytes, badge_pub_keys)

    
