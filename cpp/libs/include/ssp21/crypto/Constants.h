#ifndef SSP21_CONSTANTS_H
#define SSP21_CONSTANTS_H

#include <cstdint>

#include "ssp21/link/LinkConstants.h"

namespace ssp21
{
    namespace consts
    {
        namespace crypto
        {
            const uint16_t protocol_version = 0x0000;

            // implementation constants
            const uint8_t max_certificate_chain = 3;
            const uint8_t max_seq_of_seq = max_certificate_chain;

            // cryptographic constants
            const uint8_t sha256_hash_output_length = 32;
            const uint8_t x25519_key_length = 32;
            const uint8_t symmetric_ley_length = 32;

            // HMAC truncation lengths
            const uint8_t trunc16 = 16;

            // maximum length_ required buffer_ length_ across algorithm types
            const uint8_t max_primitive_buffer_length = x25519_key_length;


			// defaults for the Session
            const uint32_t default_ttl_pad_ms = 10000;
            const uint16_t default_max_nonce = 32768;
			const uint32_t default_max_session_time_ms = 60 * 60 * 1000; // 1 hour in milliseconds

        }
    }
}

#endif
