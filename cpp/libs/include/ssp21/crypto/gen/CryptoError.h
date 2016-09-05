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
// Licensed under the terms of the BSDv3 license
//

#ifndef SSP21_CRYPTOERROR_H
#define SSP21_CRYPTOERROR_H

#include <cstdint>
#include "openpal/util/Uncopyable.h"
#include "ssp21/ErrorCategory.h"

namespace ssp21 {

/**
    The result of a message parse operation
*/
enum class CryptoError : uint8_t
{
    /// attempted operation with bad key type
    bad_key_type = 0x0,
    /// X25519 DH operation failed. Bad public key?
    dh_x25519_fail = 0x1,
    /// a procedure cannot be executed b/c of an undefined algorithm
    undefined_algorithm = 0x2,
    /// A calculated MAC did not match the specified MAC
    mac_auth_fail = 0x3,
    /// A buffer had an insufficient or incorrect size
    bad_buffer_size = 0x4
};

struct CryptoErrorSpec : private openpal::StaticOnly
{
    typedef CryptoError enum_type_t;

    static const char* name;

    static const char* to_string(CryptoError arg);
};

typedef ErrorCategory<CryptoErrorSpec> CryptoErrorCategory;

inline std::error_code make_error_code(CryptoError err)
{
    return std::error_code(static_cast<int>(err), CryptoErrorCategory::get());
}

}

namespace std {

template <>
struct is_error_code_enum<ssp21::CryptoError> : public true_type {};

}

#endif
