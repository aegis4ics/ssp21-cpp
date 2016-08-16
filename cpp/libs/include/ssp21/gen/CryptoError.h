//
//  _   _         ______    _ _ _   _             _ _ _
// | \ | |       |  ____|  | (_) | (_)           | | | |
// |  \| | ___   | |__   __| |_| |_ _ _ __   __ _| | | |
// | . ` |/ _ \  |  __| / _` | | __| | '_ \ / _` | | | |
// | |\  | (_) | | |___| (_| | | |_| | | | | (_| |_|_|_|
// |_| \_|\___/  |______\__,_|_|\__|_|_| |_|\__, (_|_|_)
//                                           __/ |
//                                          |___/
// 
// This file is auto-generated. Do not edit manually
// 
// License TBD
//

#ifndef SSP21_CRYPTOERROR_H
#define SSP21_CRYPTOERROR_H

#include <cstdint>

namespace ssp21 {

/**
  The result of a message parse operation
*/
enum class CryptoError : uint8_t
{
  /// attempted operation with bad key type
  bad_key_type = 0x0,
  /// X25519 DH operation failed. Bad public key?
  dh_x25519_fail = 0x1
};

char const* crypto_error_to_string(CryptoError arg);

}

#endif
