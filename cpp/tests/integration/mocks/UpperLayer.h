#ifndef SSP21_UPPERLAYER_H
#define SSP21_UPPERLAYER_H

#include "ssp21/LayerInterfaces.h"

#include <memory>
#include <vector>

namespace ssp21
{
    struct IReceiveValidator
    {
        virtual void validate(const seq32_t& data) = 0;
    };

    class UpperLayer final : public IUpperLayer
    {

    public:

        void configure(ILowerLayer& lower)
        {
            this->lower = &lower;
        }

        uint32_t num_bytes_rx = 0;

        void add_validator(const std::shared_ptr<IReceiveValidator>& validator)
        {
            this->validators.push_back(validator);
        }

    private:

        virtual void on_open_impl() override {}

        virtual void on_close_impl() override {}

        virtual void on_tx_ready_impl() override {}

        virtual void start_rx_impl(const seq32_t& data) override
        {
            num_bytes_rx += data.length();

            for (auto& v : this->validators)
            {
                v->validate(data);
            }

        }

        virtual bool is_rx_ready_impl() override
        {
            return true;
        }

        // set during configure step
        ILowerLayer* lower = nullptr;

        std::vector<std::shared_ptr<IReceiveValidator>> validators;
    };

}

#endif
