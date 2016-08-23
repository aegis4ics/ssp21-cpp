

#include "catch.hpp"

#include "ssp21/crypto/Responder.h"

#include "openpal/container/StaticBuffer.h"

#include "testlib/BufferHelpers.h"

#include "mocks/MockLogger.h"
#include "mocks/MockLowerLayer.h"

#define SUITE(name) "ResponderTestSuite - " name

using namespace ssp21;
using namespace openpal;

TEST_CASE(SUITE("can be constructed"))
{		
	MockLogger log("responder");	
	MockLowerLayer lower;

	Responder resp(log.root.logger, lower);	
}