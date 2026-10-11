#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class Category : uint8
  {
    Integer,
    Number,
    Percentage,
    Length,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex,
    LengthPercentage,
    AnglePercentage,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::Category, 11uz);