#define BOOST_TEST_MODULE boost_test_attotime
#include <boost/test/included/unit_test.hpp>

#include "../../source/core/attotime.h"

BOOST_AUTO_TEST_CASE(test_string_formatting)
{
   attotime value = attotime::from_seconds(1);
   BOOST_CHECK(value.as_attoseconds() == 1000000000000000000);
   BOOST_CHECK_EQUAL(value.as_string(0), "1");
   BOOST_CHECK_EQUAL(value.as_string(4), "1.0000");
   BOOST_CHECK_EQUAL(value.as_string(10), "1.0000000000");
   BOOST_CHECK_EQUAL(value.as_string(15), "1.000000000000000");
   BOOST_CHECK_EQUAL(attotime::never.as_string(5), "(never)");
}

BOOST_AUTO_TEST_CASE(test_never)
{
	attotime value = attotime::from_seconds(1);
	BOOST_CHECK(!value.is_never());
}

BOOST_AUTO_TEST_CASE(test_mul)
{
	attotime twoSecs = attotime::from_seconds(2);
    attotime threeSecs = attotime::from_seconds(3);
    attotime val = twoSecs * 3;
	BOOST_CHECK(val == attotime::from_seconds(6));

    val = attotime::never * 3;
	BOOST_CHECK(val == attotime::never);

    val = twoSecs * 0;
	BOOST_CHECK(val == attotime::zero);
}

BOOST_AUTO_TEST_CASE(test_div)
{
	attotime twoSecs = attotime::from_seconds(2);
    attotime sixSecs = attotime::from_seconds(6);
    attotime val = twoSecs / 3;
	BOOST_CHECK(val == twoSecs);

    val = attotime::never / 3;
	BOOST_CHECK(val == attotime::never);

    // divide by zero is ignored
    val = twoSecs / 0;
	BOOST_CHECK(val == twoSecs);
}