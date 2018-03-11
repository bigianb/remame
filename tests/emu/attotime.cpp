#define BOOST_TEST_MODULE boost_test_macro_overview
#include <boost/test/included/unit_test.hpp>

#include "../../source/core/attotime.h"

BOOST_AUTO_TEST_CASE(test_macro_overview)
{
   attotime value = attotime::from_seconds(1);
   BOOST_CHECK(value.as_attoseconds() == 1000000000000000000);
}