#define BOOST_TEST_MODULE boost_test_attotime
#include <boost/test/included/unit_test.hpp>

#include "../../source/core/attotime.h"

BOOST_AUTO_TEST_CASE(test_string_formatting)
{
   attotime value = attotime::from_seconds(1);
   BOOST_CHECK(value.as_attoseconds() == 1000000000000000000);
   BOOST_CHECK(value.as_string(0) == "1");
   BOOST_CHECK(value.as_string(4) == "1.0000");
   BOOST_CHECK(value.as_string(10) == "1.0000000000");
   BOOST_CHECK(value.as_string(15) == "1.000000000000000");
}

BOOST_AUTO_TEST_CASE(test_never)
{
	attotime value = attotime::from_seconds(1);
	BOOST_CHECK(!value.is_never());
}
