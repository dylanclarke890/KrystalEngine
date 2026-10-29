#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"
#include "Krystal.Booey/Platform/ColorChooser.hpp"
#include "Krystal.Booey/Platform/ColorChooserClient.hpp"

namespace krys::boo::html
{
  /// @brief The color input type: a swatch of its colour (the `::-webkit-color-swatch` part), which opens the
  /// host's colour picker when activated.
  // FIXME: The alpha and colorspace attributes, and the datalist's suggestions (suggestedColors).
  class ColorInputType final : public BaseClickableWithKeyInputType, private ColorChooserClient
  {
  private:
    RefPtr<ColorChooser> _chooser;

  public:
    explicit ColorInputType(HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(Type::Color, element)
    {
    }

    ~ColorInputType() noexcept override;

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Color;
    }

    KRYS_NODISCARD bool SupportsRequired() const noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> FallbackValue() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;

    /// @brief The value as a colour.
    KRYS_NODISCARD Color ValueAsColor() const noexcept;

    void Detach() noexcept final;

    // ColorChooserClient functions:
    KRYS_NODISCARD bool SupportsAlpha() const noexcept final;
    KRYS_NODISCARD bool IsElementRTL() const noexcept final;
    KRYS_NODISCARD List<Color> SuggestedColors() const noexcept final;

  private:
    void DidChooseColor(const Color &color) noexcept final;
    void DidEndChooser() noexcept final;
    KRYS_NODISCARD IntRect ElementRectRelativeToRootView() const noexcept final;

    // InputType functions:
    KRYS_NODISCARD bool IsPresentingAttachedView() const noexcept final;
    void CreateShadowSubtree() noexcept final;
    void SetValue(dom::DOMStringView value, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
    void AttributeChanged(dom::DOMStringAtom name) noexcept final;
    void HandleActivationBehavior(dom::Event &event) noexcept final;
    void ShowPicker() noexcept final;
    void ElementDidBlur() noexcept final;

    void EndColorChooser() noexcept;
    void UpdateColorSwatch() noexcept;
    KRYS_NODISCARD HTMLElement *ShadowColorSwatch() const noexcept;
  };
}
