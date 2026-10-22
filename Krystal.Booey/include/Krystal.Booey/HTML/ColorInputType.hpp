#pragma once

#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"

namespace krys::boo::html
{
  /// @brief The color input type.
  // FIXME: The color well (its shadow tree and renderer), the picker (the chrome client's color chooser), and
  // the alpha and colorspace attributes come with the other controls.
  class ColorInputType final : public BaseClickableWithKeyInputType
  {
  public:
    explicit ColorInputType(HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(Type::Color, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Color;
    }

    KRYS_NODISCARD bool SupportsRequired() const noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> FallbackValue() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;

    /// @brief The value as a colour.
    KRYS_NODISCARD SRGBA<uint8> ValueAsColor() const noexcept;
  };
}
