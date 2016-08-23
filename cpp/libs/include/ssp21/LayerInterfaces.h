
#ifndef SSP21_LAYER_INTERFACES_H
#define SSP21_LAYER_INTERFACES_H

#include "ssp21/link/Addresses.h"

#include "openpal/container/RSlice.h"

namespace ssp21
{	

	class ILowerLayer
	{

	public:

		virtual void begin_transmit(const Addresses& addr, const openpal::RSlice& message) = 0;

		virtual bool read_message(Addresses& addr, openpal::RSlice& message) = 0;

		bool is_transmitting() const 
		{
			return is_transmitting_;
		}

	protected:

		bool is_transmitting_ = false;

	};

	class IUpperLayer
	{

	public:

		void on_open()
		{
			if (!is_open_)
			{
				is_open_ = true;
				this->on_open_impl();
			}
		}

		void on_close()
		{
			if (is_open_)
			{
				is_open_ = false;
				this->on_close_impl();
			}
		}

		void on_tx_ready()
		{
			if (is_open_)
			{
				this->on_tx_ready();				
			}
		}

		void on_rx_ready()
		{
			if (is_open_)
			{
				this->on_rx_ready();
			}
		}

	protected: 

		virtual void on_open_impl() = 0;

		virtual void on_close_impl() = 0;

		virtual void on_tx_ready_impl() = 0;

		virtual void on_rx_ready_impl() = 0;

		bool is_open() const { return is_open_; }

	private:

		bool is_open_ = false;

	};

}

#endif