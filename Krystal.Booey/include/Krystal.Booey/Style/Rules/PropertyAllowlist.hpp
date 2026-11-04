#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::style
{
  /// @brief The properties a rule may set, restricted for rules that style some pseudo-elements.
  enum class PropertyAllowlist : uint8
  {
    None,
    Marker,
    Cue,
    CueSelector,
    CueBackground,
  };

  KRYS_NODISCARD PropertyAllowlist PropertyAllowlistForPseudoElement(PseudoElementType type) noexcept;

  KRYS_NODISCARD bool IsValidMarkerStyleProperty(css::PropertyId id) noexcept;
  KRYS_NODISCARD bool IsValidCueStyleProperty(css::PropertyId id) noexcept;
  KRYS_NODISCARD bool IsValidCueSelectorStyleProperty(css::PropertyId id) noexcept;
  KRYS_NODISCARD bool IsValidCueBackgroundStyleProperty(css::PropertyId id) noexcept;
}
