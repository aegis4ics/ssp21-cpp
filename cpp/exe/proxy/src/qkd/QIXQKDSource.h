#ifndef SSP21PROXY_QIXQKDSOURCE_H
#define SSP21PROXY_QIXQKDSOURCE_H

#include "IQKDSource.h"

#include <log4cpp/Logger.h>
#include <qix/QIXFrameReader.h>
#include <qix/QIXKeyStore.h>
#include <exe4cpp/asio/BasicExecutor.h>

#include <yaml-cpp/yaml.h>

#include <map>
#include <mutex>
#include <chrono>
#include <deque>

class QIXQKDSource : public IQKDSource
{    
	class FrameHandler : public IQIXFrameHandler
	{	
    public:
        FrameHandler(const YAML::Node& node, log4cpp::Logger& logger);

		void handle(const QIXFrame& frame) override;

        std::map<uint16_t, std::shared_ptr<QIXKeyStore>> subscribers;

    private:

        const uint16_t num_subscribers;
        const std::chrono::high_resolution_clock::duration key_metric_update_rate;
        const uint16_t key_metric_bin_size;

        log4cpp::Logger logger;

        std::chrono::high_resolution_clock::time_point last_metric_update_time;
        std::deque<std::chrono::high_resolution_clock::time_point> key_data_bin;

        double calc_mean_time_between_keys();
        double calc_std_dev_of_time_between_keys(double mean_time_between_keys);
	};

public:

	QIXQKDSource(const YAML::Node& node, const std::shared_ptr<exe4cpp::BasicExecutor>& executor, log4cpp::Logger& logger);
	
	std::shared_ptr<ssp21::IKeyLookup> bind_responder_key_lookup(uint16_t modulo, uint16_t max_cached_keys) override;
	
	std::shared_ptr<ssp21::IKeySource> bind_initiator_key_source(uint16_t modulo, uint16_t max_cached_keys) override;

private:

    std::shared_ptr<QIXKeyStore> bind_key_store(uint16_t modulo, uint16_t max_cached_keys);


	std::shared_ptr<FrameHandler> handler;    	
};

#endif
