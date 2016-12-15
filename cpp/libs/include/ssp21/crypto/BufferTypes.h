
#ifndef SSP21_BUFFER_TYPES_H
#define SSP21_BUFFER_TYPES_H

#include "Constants.h"

#include <openpal/util/Uncopyable.h>
#include <openpal/container/StaticBuffer.h>

namespace ssp21
{
    // the BufferType also indirectly defines the length
    enum class BufferType
    {
        empty,
        x25519_key,
        sha256,
        symmetric_key
    };

    class BufferBase : private openpal::Uncopyable
    {
    public:

        virtual ~BufferBase() {}

        openpal::RSlice as_slice() const;

        BufferType get_type() const;

        openpal::WSlice get_write_slice();

        void set_type(BufferType key_type);

        void copy(const BufferBase& other);

    protected:

        BufferBase() = default;

    private:

        static uint32_t get_buffer_length(BufferType);

        uint32_t length = 0;
        BufferType buffer_type = BufferType::empty;

    protected:
        openpal::StaticBuffer<consts::crypto::max_primitive_buffer_length> buffer;
    };

    /**
    	A secure key zeros its buffer upon destruction
    	and provides a clear method
    */
    class SecureBuffer : public BufferBase
    {
    public:
        virtual ~SecureBuffer();

    protected:
        SecureBuffer() {}
    };

    // specialized types that actually get used

    class PublicKey final : public BufferBase {};

    class PrivateKey final : public SecureBuffer {};

    class DHOutput final : public SecureBuffer {};

    class HashOutput final : public SecureBuffer {};

    class SymmetricKey final : public SecureBuffer {};

    struct KeyPair final
    {
        PublicKey public_key;
        PrivateKey private_key;
    };

    struct SessionKeys final
    {
        SymmetricKey rx_key;
        SymmetricKey tx_key;

        inline void copy(const SessionKeys& other)
        {
            this->rx_key.copy(other.rx_key);
            this->rx_key.copy(other.tx_key);
        }

        bool valid() const
        {
            return (rx_key.get_type() == BufferType::symmetric_key) && (tx_key.get_type() == BufferType::symmetric_key);
        }
    };


}

#endif
