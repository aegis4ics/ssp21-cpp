

#ifndef RESPONDER_FIXTURE_H
#define RESPONDER_FIXTURE_H

#include "ssp21/crypto/BufferTypes.h"
#include "ssp21/crypto/Responder.h"

#include "testlib/MockExecutor.h"

#include "../mocks/MockLogHandler.h"
#include "../mocks/MockLowerLayer.h"
#include "../mocks/MockUpperLayer.h"
#include "../mocks/MockCryptoBackend.h"
#include "../mocks/HexMessageBuilders.h"
#include "../mocks/MockFrameWriter.h"

#include "ssp21/link/LinkConstants.h"

#include <memory>

namespace ssp21
{

    class ResponderFixture
    {
    private:

        struct Keys
        {
            Keys(BufferType key_type) :
                local_kp(std::make_unique<KeyPair>()),
                remote_static_key(std::make_unique<PublicKey>())
            {
                init_key(local_kp->private_key, key_type);
                init_key(local_kp->public_key, key_type);
                init_key(*remote_static_key, key_type);
            }

            static void init_key(BufferBase& buffer, BufferType key_type)
            {
                buffer.as_wseq().set_all_to(0xFF);
                buffer.set_type(key_type);
            }

            std::unique_ptr<KeyPair> local_kp;
            std::unique_ptr<PublicKey> remote_static_key;
        };


    public:

        ResponderFixture(
            const Session::Config& session_config = Session::Config(),
            const Responder::Config& config = Responder::Config(),
            uint16_t max_message_size = consts::link::max_config_payload_size
        ) :
            keys(BufferType::x25519_key),
            log("responder"),
            exe(openpal::MockExecutor::Create()),
            responder(
                config,
                session_config,
                log.logger,
                get_frame_writer(log.logger, max_message_size),
                exe,
                std::move(keys.local_kp), std::move(keys.remote_static_key)
            ),
            lower(responder),
            upper(responder)
        {
            MockCryptoBackend::instance.clear_actions();

            this->responder.bind_layers(lower, upper);
        }

        void set_tx_ready()
        {
            lower.set_tx_ready(true);
            responder.on_tx_ready();
        }

        static std::shared_ptr<IFrameWriter> get_frame_writer(const openpal::Logger& logger, uint16_t max_message_size)
        {
            return std::make_shared<MockFrameWriter>(logger, max_message_size);
        }

    private:

        Keys keys;

    public:

        ssp21::MockLogHandler log;
        const std::shared_ptr<openpal::MockExecutor> exe;
        Responder responder;
        MockLowerLayer lower;
        MockUpperLayer upper;

    };

}

#endif

