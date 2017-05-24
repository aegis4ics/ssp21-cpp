

#include "ssp21/crypto/HandshakeAuthentication.h"

namespace ssp21
{

    bool HandshakeAuthentication::auth_handshake_with_mac(mac_func_t mac_func, const SymmetricKey& key, HandshakeMode mode, const seq32_t& mac_value)
    {
        auto byte = get_auth_byte(mode);
        const seq32_t pattern(&byte, 1);
        HashOutput mac_buffer;
        mac_func(key.as_seq(), { pattern }, mac_buffer);
        return Crypto::secure_equals(mac_buffer.as_seq(), mac_value);
    }

    void HandshakeAuthentication::calc_handshake_mac_with_macfunc(mac_func_t mac_func, const SymmetricKey& key, HandshakeMode mode, HashOutput& output)
    {
        auto byte = get_calc_byte(mode);
        const seq32_t pattern(&byte, 1);
        mac_func(key.as_seq(), { pattern }, output);
    }
}


