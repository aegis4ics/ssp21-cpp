
#include "ssp21/crypto/InitiatorHandshakeStates.h"

namespace ssp21
{
    Initiator::IHandshakeState& InitiatorHandshakeStateIdle::initialize(Initiator& ctx, const openpal::Timestamp& now)
    {
        // TODO - kick off the handshake

        return *this;
    }
}


