
#include "ssp21/crypto/PublicKeyInitiatorHandshake.h"

#include "openpal/logging/LogMacros.h"

#include "ssp21/crypto/TripleDH.h"

using namespace openpal;

namespace ssp21
{
    seq32_t PublicKeyInitiatorHandshake::generate_ephemeral_data()
    {
        this->algorithms.handshake.gen_keypair(this->local_ephemeral_keys);
        return this->local_ephemeral_keys.public_key.as_seq();
    }

    void PublicKeyInitiatorHandshake::begin_request_transmit(const seq32_t& data, const openpal::Timestamp& now)
    {
        this->time_request_tx = now;
        this->algorithms.handshake.hash({ data }, this->handshake_hash);
    }

    bool PublicKeyInitiatorHandshake::initialize_session(const ReplyHandshakeBegin& msg, const seq32_t& msg_bytes, const InitiatorConfig::Params& params, const Timestamp& now, Session& session)
    {
        // extract the remote public key
        seq32_t remote_public_key;
        const auto err = this->cert_handler->validate(msg.handshake_data, remote_public_key);
        if (any(err))
        {
            FORMAT_LOG_BLOCK(this->logger, levels::error, "error validating certificate data: %s", HandshakeErrorSpec::to_string(err));
            return false;
        }

        // mix the handshake hash
        const auto salt = this->mix_handshake_hash(msg_bytes);

        // perform a triple-dh
        TripleDH triple_dh;

        std::error_code ec;
        const auto ikm = triple_dh.compute(
                             this->algorithms.handshake.dh,
                             this->static_keys,
                             this->local_ephemeral_keys,
                             remote_public_key,
                             msg.ephemeral_data,
                             ec
                         );

        if (ec)
        {
            FORMAT_LOG_BLOCK(this->logger, levels::warn, "Error generating input key material: %s", ec.message().c_str());
            return false;
        }

        // perform session key derivation
        SessionKeys session_keys;

        this->algorithms.handshake.kdf(
            salt,
        { ikm.dh1, ikm.dh3, ikm.dh2 },
        session_keys.tx_key,
        session_keys.rx_key
        );

        if (now < this->time_request_tx)
        {
            SIMPLE_LOG_BLOCK(this->logger, levels::error, "clock rollback detected");
            return false;
        }

        // estimate the session initialization time
        const auto elapsed_ms = now.milliseconds - this->time_request_tx.milliseconds;
        const auto session_start_time = openpal::Timestamp(now.milliseconds - (elapsed_ms / 2)); // estimate

        return session.initialize(
                   this->algorithms.session,
                   Session::Param(session_start_time, params.max_nonce_value, params.max_session_time_ms),
                   session_keys
               );
    }

    seq32_t PublicKeyInitiatorHandshake::mix_handshake_hash(const seq32_t& input)
    {
        // h = hash(h || input)

        this->algorithms.handshake.hash(
        { this->handshake_hash.as_seq(), input },
        this->handshake_hash
        );

        return this->handshake_hash.as_seq();
    }
}


