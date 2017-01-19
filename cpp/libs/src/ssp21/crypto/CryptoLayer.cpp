
#include "ssp21/crypto/CryptoLayer.h"

#include "ssp21/crypto/LogMessagePrinter.h"
#include "ssp21/LogLevels.h"

#include "openpal/logging/LogMacros.h"


namespace ssp21
{

    CryptoLayer::CryptoLayer(
        HandshakeMode type,
        const Config& context_config,
        const Session::Config& session_config,
        const openpal::Logger& logger,
        const std::shared_ptr<IFrameWriter>& frame_writer,
        const std::shared_ptr<openpal::IExecutor>& executor,
        std::unique_ptr<KeyPair> local_static_key_pair,
        std::unique_ptr<PublicKey> remote_static_public_key
    ) :
        logger(logger),
        frame_writer(frame_writer),
        executor(executor),
        local_static_key_pair(std::move(local_static_key_pair)),
        remote_static_public_key(std::move(remote_static_public_key)),
        handshake(type),
        session(frame_writer, session_config),
        reassembler(context_config.max_reassembly_size)
    {}

    void CryptoLayer::receive()
    {
        if (this->is_rx_ready && this->upper->on_rx_ready(this->reassembler.get_data()))
        {
            this->is_rx_ready = false;
        }
    }

    bool CryptoLayer::transmit(const seq32_t& data)
    {
        if (!this->get_is_open())
        {
            return false;
        }

        if (!this->upper->get_is_open())
        {
            return false;
        }

        // already transmitting on behalf on the upper layer
        if (!tx_state.initialize(data))
        {
            return false;
        }

        this->check_transmit();

        return true;
    }

    template <class MsgType>
    bool CryptoLayer::handle_message(const seq32_t& message, const openpal::Timestamp& now)
    {
        MsgType msg;
        auto err = msg.read(message);
        if (any(err))
        {
            FORMAT_LOG_BLOCK(logger, levels::warn, "Error parsing message (%s): %s", FunctionSpec::to_string(MsgType::function), ParseErrorSpec::to_string(err));
            this->on_parse_error(MsgType::function, err);
            return false;
        }
        else
        {
            FORMAT_LOG_BLOCK(this->logger, levels::rx_crypto_msg, "%s (length = %u)", FunctionSpec::to_string(MsgType::function), message.length());

            if (this->logger.is_enabled(levels::rx_crypto_msg_fields))
            {
                LogMessagePrinter printer(this->logger, levels::rx_crypto_msg_fields);
                msg.print(printer);
            }

            this->on_message(msg, message, now);

            return true;
        }
    }

    void CryptoLayer::on_close_impl()
    {
        // let the super class reset
        this->reset_state_on_close();

        this->session.reset();
        this->reassembler.reset();
        this->upper->on_close();
        this->tx_state.reset();
        this->reset_lower_layer();
    }

    bool CryptoLayer::on_rx_ready_impl(const seq32_t& data)
    {
        if (this->can_receive())
        {
            this->process(data);

            return true;
        }
        else
        {
            return false;
        }
    }

    void CryptoLayer::on_tx_ready_impl()
    {
        // TODO - evaluate the appropriate priority of these checks

        this->on_pre_tx_ready();

        if (this->tx_state.on_tx_complete())
        {
            // ready to transmit more data
            this->is_tx_ready = true;
            this->upper->on_tx_ready();
        }

        this->check_receive();
        this->check_transmit();
    }

    void CryptoLayer::process(const seq32_t& message)
    {
        const auto now = this->executor->get_time();

        if (message.is_empty())
        {
            SIMPLE_LOG_BLOCK(logger, levels::warn, "Received zero length message");
            return;
        }

        const auto raw_function = message[0];
        const auto function = FunctionSpec::from_type(raw_function);

        if (!this->supports(function))
        {
            FORMAT_LOG_BLOCK(logger, levels::warn, "Received unsupported function: %s(%u)", FunctionSpec::to_string(function), raw_function);
            return;
        }

        switch (function)
        {
        case(Function::request_handshake_begin):
            handle_message<RequestHandshakeBegin>(message, now);
            break;
        case(Function::request_handshake_auth):
            handle_message<RequestHandshakeAuth>(message, now);
            break;
        case(Function::reply_handshake_begin):
            handle_message<ReplyHandshakeBegin>(message, now);
            break;
        case(Function::reply_handshake_auth):
            handle_message<ReplyHandshakeAuth>(message, now);
            break;
        case(Function::reply_handshake_error):
            handle_message<ReplyHandshakeError>(message, now);
            break;
        case(Function::session_data):
            handle_message<SessionData>(message, now);
            break;
        default:
            break;
        }
    }

    void CryptoLayer::check_receive()
    {
        if (this->can_receive())
        {
            this->lower->receive();
        }
    }

    void CryptoLayer::check_transmit()
    {
        if (!this->can_transmit_session_data())
        {
            return;
        }

        auto remainder = this->tx_state.get_remainder();
        const auto fir = this->tx_state.get_fir();
        const auto now = this->executor->get_time();

        std::error_code err;
        const auto data = this->session.format_session_message(fir, now, remainder, err);
        if (err)
        {
            FORMAT_LOG_BLOCK(this->logger, levels::warn, "Error formatting session message: %s", err.message().c_str());

            // if any error occurs with transmission, we reset the session and notify the upper layer
            this->session.reset();
            this->upper->on_close();

            return;
        }

        this->tx_state.begin_transmit(remainder);

        this->lower->transmit(data);

        this->on_session_nonce_change(this->session.get_rx_nonce(), this->session.get_tx_nonce());
    }

    void CryptoLayer::on_message(const SessionData& msg, const seq32_t& raw_data, const openpal::Timestamp& now)
    {
        std::error_code ec;
        const auto payload = this->session.validate_message(msg, now, ec);

        if (ec)
        {
            FORMAT_LOG_BLOCK(this->logger, levels::warn, "validation error: %s", ec.message().c_str());
            return;
        }

        this->on_session_nonce_change(this->session.get_rx_nonce(), this->session.get_tx_nonce());

        // process the message using the reassembler
        const auto result = this->reassembler.process(msg.metadata.flags.fir, msg.metadata.flags.fin, msg.metadata.nonce, payload);

        switch (result)
        {
        case(ReassemblyResult::partial):
            break; // do nothing

        case(ReassemblyResult::complete):
            this->is_rx_ready = !this->upper->on_rx_ready(this->reassembler.get_data());
            break;

        default: // error
            FORMAT_LOG_BLOCK(this->logger, levels::warn, "reassembly error: %s", ReassemblyResultSpec::to_string(result));
            break;
        }
    }


}


