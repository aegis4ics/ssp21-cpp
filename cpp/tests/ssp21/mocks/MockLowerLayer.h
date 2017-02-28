#ifndef SSP21_MOCKLOWERLAYER_H
#define SSP21_MOCKLOWERLAYER_H

#include "openpal/util/Uncopyable.h"

#include "ssp21/LayerInterfaces.h"

#include "testlib/Hex.h"
#include "testlib/HexConversions.h"

#include "ssp21/MakeUnique.h"

#include <deque>

namespace ssp21
{
    class MockLowerLayer : public ILowerLayer, private openpal::Uncopyable
    {

        typedef openpal::Buffer message_t;

    public:

        void bind_upper(IUpperLayer& upper)
        {
            this->upper = &upper;
        }

        virtual bool start_tx(const seq32_t& message) override
        {
            if (!this->is_tx_ready())
            {
                throw std::logic_error("start_tx called when not tx ready");
            }

            this->tx_messages.push_back(std::make_unique<message_t>(message));
            return true;
        }

        virtual bool is_tx_ready() const override
        {
            return this->is_tx_ready_flag;
        }

        virtual void on_rx_ready() override
        {
            if (!this->rx_messages.empty())
            {
                auto& front = this->rx_messages.front();

                if (upper->start_rx(front->as_rslice()))
                {
                    this->rx_messages.pop_front();

                    if (this->rx_messages.empty())
                    {
                        // TODO
                        // this->is_rx_ready = false;
                    }
                }
            }
        }

        void enqueue_message(const std::string& hex)
        {
            openpal::Hex hexdata(hex);

            /* TODO
            if (!upper->is_rx_ready())
            {
                this->rx_messages.push_back(std::make_unique<message_t>(hexdata.as_rslice()));
                this->is_rx_ready = true;
            }
            */
        }

        size_t num_rx_messages() const
        {
            return this->rx_messages.size();
        }

        std::string pop_tx_message()
        {
            if (this->tx_messages.empty())
            {
                throw std::logic_error("No messages to pop()");
            }

            auto hex = openpal::to_hex(this->tx_messages.front()->as_rslice());
            this->tx_messages.pop_front();
            return hex;
        }

        size_t num_tx_messages() const
        {
            return tx_messages.size();
        }

        void set_tx_ready(bool value)
        {
            this->is_tx_ready_flag = value;
        }

    private:

        bool is_tx_ready_flag = true;

        typedef std::deque<std::unique_ptr<message_t>> message_queue_t;

        IUpperLayer* upper = nullptr;

        message_queue_t tx_messages;

        message_queue_t rx_messages;
    };

}

#endif
