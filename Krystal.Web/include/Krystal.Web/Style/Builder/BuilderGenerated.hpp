#pragma once

#include "Krystal.Web/Css/Properties/PropertyId.hpp"

namespace krys::web::css
{
  class Value;
}

namespace krys::web::style
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
