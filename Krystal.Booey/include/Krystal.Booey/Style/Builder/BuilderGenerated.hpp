#pragma once

#include "Krystal.Booey/Css/Properties/PropertyId.hpp"

namespace krys::boo::css
{
  class Value;
}

namespace krys::boo::style
{
  enum class ApplyValueType : uint8;
  class BuilderState;

  /// @brief Applies a longhand's initial, inherited or specified value to the style being built.
  class BuilderGenerated
  {
  public:
    static void ApplyProperty(css::PropertyId, BuilderState &, css::Value &, ApplyValueType) noexcept;
  };
}
