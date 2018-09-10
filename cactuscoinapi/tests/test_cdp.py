import json
import pytest
import struct
from utils import get_message,post_message
from cactuscoinapi import cdp, crypto

def test_registration_under_badge():
    cdp_message = cdp.RegistrationUnderBadge(name='cybaix')
    assert cdp_message.to_bytes() == b'\x06cybaix'
    cdp_message = cdp.RegistrationUnderBadge.from_bytes(b'\x06cybaix')
    assert cdp_message.name == 'cybaix'

def test_global_broadcast_payload():
    cdp_message = cdp.GlobalBroadcastPayload()
    assert cdp_message.to_bytes() == b''

def test_cactuscoin_signing_request(badge_keys, badge_pub_keys):
    cdp_message = cdp.CactuscoinSigningRequest(csr_id=1, broadcasted_id=2,
            signing_key=badge_keys[1])
    assert cdp_message.to_bytes()[:4] == b'\x00\x01\x00\x02'
    the_bytes = struct.pack('!HH', cdp_message.csr_id, cdp_message.broadcasted_id)
    crypto.verify(the_bytes, cdp_message.csr_signature, badge_pub_keys[1])

    cdp_message = cdp.CactuscoinSigningRequest.from_bytes(b'\x00\x02\x00\x01')
    assert cdp_message.csr_id == 2
    assert cdp_message.broadcasted_id == 1

    # Endianness and everything else looks good, let's check that it's unsigned
    cdp_message = cdp.CactuscoinSigningRequest(csr_id=65534, broadcasted_id=2, 
            signing_key=badge_keys[1])
    assert cdp_message.to_bytes()[:4] == b'\xff\xfe\x00\x02'

    cdp_message = cdp.CactuscoinSigningRequest.from_bytes(b'\xff\xfe\x00\x02')
    assert cdp_message.csr_id == 65534
    assert cdp_message.broadcasted_id == 2


def test_cactuscoin_authentication_data(badge_keys, badge_pub_keys):
    csr = cdp.CactuscoinSigningRequest(csr_id=1, broadcasted_id=2, 
            signing_key=badge_keys[1])
    cdp_packet = cdp.CactuscoinDataPacket(1, csr)
    cdp_packet.to_bytes(badge_keys[1])

    cad = cdp.CactuscoinAuthenticationData(csr_id=1, broadcasted_id=2, 
            csr_signature=cdp_packet.message.csr_signature, signing_key=badge_keys[2])
    cad_bytes = cad.to_bytes()

    cad_reconstructed = cdp.CactuscoinAuthenticationData.from_bytes(cad_bytes)
    cad_reconstructed.validate(badge_pub_keys)

def test_cactuscoin_data_packet(badge_keys, badge_pub_keys):
    rup = cdp.RegistrationUnderBadge(name='cybaix')
    rup_cdp_packet = cdp.CactuscoinDataPacket(1, rup)
    rup_cdp_bytes = rup_cdp_packet.to_bytes(badge_keys[1])

    assert rup_cdp_packet.message.name == 'cybaix'

def test_automatic_tunnel_message(badge_keys, badge_pub_keys):
    rup = cdp.RegistrationUnderBadge(name='cybaix')
    rup_cdp_packet = cdp.CactuscoinDataPacket(1, rup)
    rup_cdp_bytes = rup_cdp_packet.to_bytes(badge_keys[1])

    atm = cdp.AutomaticTunnelMessage(tunnelled_message=rup_cdp_bytes)
    atm_cdp_packet = cdp.CactuscoinDataPacket(badge_id=2, message=atm)
    packet_bytes = atm_cdp_packet.to_bytes(badge_keys[2])

    unpacked_packet = cdp.CactuscoinDataPacket.from_bytes(packet_bytes, badge_pub_keys)
    tunnelled_cdp = cdp.CactuscoinDataPacket.from_bytes(unpacked_packet.message.tunnelled_message, badge_pub_keys)

    assert tunnelled_cdp.message.name == 'cybaix'
