
#include "catch.hpp"

#include "fixtures/CryptoLayerFixture.h"

#define SUITE(name) "InitiatorTestSuite - " name

using namespace ssp21;
using namespace openpal;

void test_open(InitiatorFixture& fix);

// ---------- tests for initial handshake message -----------

TEST_CASE(SUITE("construction"))
{
    InitiatorFixture fix;
}

TEST_CASE(SUITE("transmits REQUEST_HANDSHAKE_BEGIN when opened"))
{
    InitiatorFixture fix;
    test_open(fix);
}

TEST_CASE(SUITE("stops timer when closed"))
{
    InitiatorFixture fix;
    test_open(fix);
    fix.initiator.on_close();
    REQUIRE(fix.exe->num_active() == 0);
}

TEST_CASE(SUITE("cancels response timer and starts retry timer when error message received"))
{
    InitiatorFixture fix;
    test_open(fix);

    fix.lower.enqueue_message(hex::reply_handshake_error(HandshakeError::bad_message_format));
    REQUIRE(fix.exe->num_timer_cancel() == 1);
    REQUIRE(fix.exe->num_pending_timers() == 1);
}

TEST_CASE(SUITE("starts retry timer when response timeout fires"))
{
    InitiatorFixture fix;
    test_open(fix);

    REQUIRE(fix.exe->advance_to_next_timer());
    REQUIRE(fix.exe->num_timer_cancel() == 0);
    REQUIRE(fix.exe->num_pending_timers() == 1);
}

// ---------- helper implementations -----------

void test_open(InitiatorFixture& fix)
{
    REQUIRE(fix.lower.num_tx_messages() == 0);
    fix.initiator.on_open();
    REQUIRE(fix.lower.num_tx_messages() == 1);

    const auto expected = hex::request_handshake_begin(
                              0,
                              NonceMode::increment_last_rx,
                              DHMode::x25519,
                              HandshakeHash::sha256,
                              HandshakeKDF::hkdf_sha256,
                              HandshakeMAC::hmac_sha256,
                              SessionMode::hmac_sha256_16,
                              CertificateMode::preshared_keys,
                              hex::repeat(0xFF, 32)
                          );

    REQUIRE(fix.lower.pop_tx_message() == expected);
    REQUIRE(fix.exe->num_pending_timers() == 1);
}

