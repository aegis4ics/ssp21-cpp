
#ifndef SSP21_CRYPTOSUITE_H
#define SSP21_CRYPTOSUITE_H

#include "ssp21/crypto/gen/HandshakeEphemeral.h"
#include "ssp21/crypto/gen/HandshakeHash.h"
#include "ssp21/crypto/gen/HandshakeKDF.h"
#include "ssp21/crypto/gen/SessionNonceMode.h"
#include "ssp21/crypto/gen/SessionCryptoMode.h"

namespace ssp21
{
    // The cyrptographic modes that the initiator will request from the responder
    struct CryptoSuite
    {
        CryptoSuite() = default;

        CryptoSuite(            
            HandshakeEphemeral handshake_ephemeral,
            HandshakeHash handshake_hash,
            HandshakeKDF handshake_kdf,
			SessionNonceMode session_nonce_mode,
			SessionCryptoMode session_crypto_mode
        ) :			
            handshake_ephemeral(handshake_ephemeral),
            handshake_hash(handshake_hash),
            handshake_kdf(handshake_kdf),
			session_nonce_mode(session_nonce_mode),
			session_crypto_mode(session_crypto_mode)
        {}
        
        HandshakeEphemeral handshake_ephemeral = HandshakeEphemeral::x25519;
        HandshakeHash handshake_hash = HandshakeHash::sha256;
        HandshakeKDF handshake_kdf = HandshakeKDF::hkdf_sha256;
		SessionNonceMode session_nonce_mode = SessionNonceMode::increment_last_rx;
		SessionCryptoMode session_crypto_mode = SessionCryptoMode::hmac_sha256_16;
    };

}

#endif
