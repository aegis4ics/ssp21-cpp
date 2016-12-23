
#include "ssp21/crypto/TruncatedMacSessionMode.h"

#include "ssp21/crypto/gen/CryptoError.h"
#include "ssp21/crypto/Crypto.h"

using namespace openpal;

namespace ssp21
{


openpal::RSlice TruncatedMacSessionMode::read(
    const SymmetricKey& key,
    const AuthMetadata& metadata,
    const openpal::RSlice& payload,
    openpal::WSlice dest,
    std::error_code& ec
)
{
	// payload must at least have the truncated HMAC
	if (payload.length() < trunc_length)
	{
		ec = CryptoError::bad_buffer_size;
		return RSlice::empty_slice();
	}

	const auto user_data_length = payload.length() - trunc_length;

	metadata_buffer_t buffer;
	auto ad_bytes = get_metadata_bytes(metadata, buffer);

	// split the payload into user data and MAC
	const auto user_data = payload.take(user_data_length);
	const auto read_mac = payload.skip(user_data_length);

	// Now calculate the expected MAC
	HashOutput calc_mac_buffer;
	mac_func(key.as_slice(), { ad_bytes, user_data }, calc_mac_buffer);
	const auto truncated_mac = calc_mac_buffer.as_slice().take(trunc_length);

	if (!Crypto::secure_equals(read_mac, truncated_mac)) // authentication failure
	{
		ec = CryptoError::mac_auth_fail;
		return openpal::RSlice::empty_slice();
	}

	// we're authenticated, so return the user_data slice
	return user_data;
}

openpal::RSlice TruncatedMacSessionMode::write(
    const SymmetricKey& key,
    const AuthMetadata& metadata,
    const openpal::RSlice& userdata,
    openpal::WSlice dest,
    std::error_code& ec
)
{
	// maximum userdata length
	const uint16_t max_userdata_length = UInt16::max_value - trunc_length;

	// payload must at least have the truncated HMAC
	if (userdata.length() > max_userdata_length)
	{
		ec = CryptoError::bad_buffer_size;
		return RSlice::empty_slice();
	}

	const uint16_t payload_length = userdata.length() + trunc_length;

	if (dest.length() < payload_length)
	{
		ec = CryptoError::bad_buffer_size;
		return RSlice::empty_slice();
	}

	metadata_buffer_t buffer;
	auto ad_bytes = get_metadata_bytes(metadata, buffer);

	// Now calculate the mac
	HashOutput calc_mac_buffer;
	mac_func(key.as_slice(), { ad_bytes, userdata }, calc_mac_buffer);
	const auto truncated = calc_mac_buffer.as_slice().take(trunc_length);

	const auto ret = dest.as_rslice().take(payload_length);

	// write the output buffer, advancing dest
	userdata.copy_to(dest);
	truncated.copy_to(dest);

	return ret;
}

uint16_t TruncatedMacSessionMode::max_writable_user_data_length(uint16_t max_payload_size)
{
    return (max_payload_size < trunc_length) ? 0 : max_payload_size - trunc_length;
}



}
