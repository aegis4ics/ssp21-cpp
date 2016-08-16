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

#ifndef SSP21_NONCEMODE_H
#define SSP21_NONCEMODE_H

#include "openpal/util/Uncopyable.h"
#include <cstdint>

namespace ssp21 {

/**
  Determines how nonces are verified during the session
*/
enum class NonceMode : uint8_t
{
  /// new nonce must strictly be equal to last nonce plus one
  increment_last_rx = 0x0,
  /// new nonce must be greater than last nonce
  greater_than_last_rx = 0x1,
  /// undefined mode
  undefined = 0xFF
};

struct NonceModeSpec : private openpal::StaticOnly
{
  typedef NonceMode enum_type_t;

  static uint8_t to_type(NonceMode arg);
  static NonceMode from_type(uint8_t arg);
  static char const* to_string(NonceMode arg);
};

}

#endif
