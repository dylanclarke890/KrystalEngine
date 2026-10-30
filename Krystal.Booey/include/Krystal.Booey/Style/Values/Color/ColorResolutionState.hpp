#pragma once

#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/Style/ForVisitedLink.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  /// @brief Used to resolve a `css::Color` to a `style::Color` during style building.
  struct ColorResolutionState
  {
    Ref<const dom::Document> Document;
    const RenderStyle &Style;
    css::ToLengthConversionData ConversionData;
    style::ForVisitedLink ForVisitedLink;
    uint32 NestingLevel {0};
  };

  /// @brief RAII helper to increment/decrement nesting level.
  struct ColorResolutionStateNester
  {
    KRYS_NON_COPY_MOVABLE(ColorResolutionStateNester);

    ColorResolutionState &State;

    explicit ColorResolutionStateNester(ColorResolutionState &state) noexcept : State {state}
    {
      State.NestingLevel++;
    }

    ~ColorResolutionStateNester() noexcept
    {
      State.NestingLevel--;
    }
  };
}
