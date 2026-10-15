#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"

namespace krys::boo::css
{
  class Value;
}

namespace krys::boo::style
{
  class BuilderState;
  enum class ApplyValueType : uint8;

  /// @brief Applies a longhand's initial, inherited or specified value to the style being built.
  class BuilderGenerated
  {
  public:
    static void ApplyProperty(css::PropertyId, BuilderState &, css::Value &, ApplyValueType) noexcept;
  };
}
