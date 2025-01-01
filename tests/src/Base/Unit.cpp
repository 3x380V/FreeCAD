#include "Base/Unit.h"
#include <gtest/gtest.h>
#include <Base/Exception.h>

// NOLINTBEGIN
using namespace Base;

TEST(Unit, string_simple_numerator_no_denominator)
{
    EXPECT_EQ(Unit::Length.getString(), "mm");
}

TEST(Unit, string_complex_numerator_no_denominator)
{
    EXPECT_EQ(Unit::Area.getString(), "mm^2");
}

TEST(Unit, string_complex_single_denominator)
{
    EXPECT_EQ(Unit::DissipationRate.getString(), "mm^2/s^3");
}

TEST(Unit, string_no_numerator)
{
    EXPECT_EQ(Unit::InverseArea.getString(), "1/mm^2");
}

TEST(Unit, string_complex_multi_denominator)
{
    EXPECT_EQ(Unit::MagneticFlux.getString(), "mm^2*kg/(s^2*A)");
}

TEST(Unit, type_string)
{
    EXPECT_EQ(Unit::MagneticFlux.getTypeString(), "MagneticFlux");
}

// NOLINTEND
