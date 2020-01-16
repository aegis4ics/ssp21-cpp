
#include "PublicKeyResponderHandshake.h"

#include "crypto/AlgorithmSet.h"
#include "crypto/HandshakeHasher.h"
#include "crypto/TripleDH.h"
#include "crypto/gen/ReplyHandshakeBegin.h"

#include "ssp21/stack/LogLevels.h"

#include "log4cpp/LogMacros.h"

namespace ssp21 {

PublicKeyResponderHandshake::PublicKeyResponderHandshake(const log4cpp::Logger& logger, const StaticKeys& static_keys, const std::shared_ptr<ICertificateHandler>& cert_handler)
    : logger(logger)
    , static_keys(static_keys)
    , cert_handler(cert_handler)
{
}

IResponderHandshake::Result PublicKeyResponderHandshake::process(const RequestHandshakeBegin& msg, const seq32_t& msg_bytes, const exe4cpp::steady_time_t& now, IFrameWriter& writer, Session& session)
{
    if (msg.spec.handshake_ephemeral != HandshakeEphemeral::x25519) {
        return Result::failure(HandshakeError::unsupported_handshake_ephemeral);
    }

    // verify that the public key length matches the DH mode
    if (msg.ephemeral_data.length() != consts::crypto::x25519_key_length) {
        return Result::failure(HandshakeError::bad_message_format);
    }

    seq32_t remote_public_static_key;

    {
        const auto err = this->cert_handler->validate(msg.handshake_mode, msg.mode_data, remote_public_static_key);
        if (any(err)) {
            return Result::failure(err);
        }
    }

    public_key_algorithms_t algorithms;

    {
        const auto err = algorithms.configure(msg.spec);
        if (any(err)) {
            return Result::failure(err);
        }
    }

    // generate an ephemeral key pair
    KeyPair ephemeralKeys;
    algorithms.handshake.gen_keypair(ephemeralKeys);

    // prepare the response
    const ReplyHandshakeBegin reply(
        ephemeralKeys.public_key.as_seq(),
        this->cert_handler->certificate_data());

    const auto result = writer.write(reply);
    if (any(result.err)) {
        FORMAT_LOG_BLOCK(this->logger, levels::error, "Error writing handshake reply: %s", FormatErrorSpec::to_string(result.err));
        return Result::failure(HandshakeError::unknown);
    }

    HandshakeHasher hasher;
    const auto handshake_hash = hasher.compute(algorithms.handshake.hash, msg_bytes, result.written);

    // compute the input_key_material
    TripleDH triple_dh;
    std::error_code ec;
    const auto ikm = triple_dh.compute(
        algorithms.handshake.dh,
        this->static_keys,
        ephemeralKeys,
        remote_public_static_key,
        msg.ephemeral_data,
        ec);

    if (ec) {
        FORMAT_LOG_BLOCK(this->logger, levels::error, "Error calculating input key material: %s", ec.message().c_str());
        return Result::failure(HandshakeError::unknown);
    }

    SessionKeys session_keys;

    algorithms.handshake.kdf(
        handshake_hash,
        { ikm.dh1, ikm.dh2, ikm.dh3 },
        session_keys.rx_key,
        session_keys.tx_key);

    session.initialize(
        algorithms.session,
        Session::Param(
            now,
            msg.constraints.max_nonce,
            std::chrono::milliseconds(msg.constraints.max_session_duration)),
        session_keys);

    return Result::success(result.frame);
}

}
