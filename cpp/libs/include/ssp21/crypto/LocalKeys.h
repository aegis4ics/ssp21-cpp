
#ifndef SSP21_LOCALKEYS_H
#define SSP21_LOCALKEYS_H

#include "ssp21/crypto/BufferTypes.h"

#include <memory>

namespace ssp21
{
    struct LocalKeys
    {
        LocalKeys(
            const std::shared_ptr<const PublicKey>& local_static_public_key,
            const std::shared_ptr<const PrivateKey>& local_static_private_key
        ) :
            local_static_public_key(local_static_public_key),
            local_static_private_key(local_static_private_key)
        {}

        const std::shared_ptr<const PublicKey> local_static_public_key;
        const std::shared_ptr<const PrivateKey> local_static_private_key;
    };
}

#endif