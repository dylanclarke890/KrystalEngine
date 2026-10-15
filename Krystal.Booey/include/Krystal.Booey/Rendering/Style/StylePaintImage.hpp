#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"

namespace krys::boo::css
{
  class VariableData;
}

namespace krys::boo
{
  /// @brief A `paint()` image, drawn by a paint worklet.
  /// FIXME: Drawing (`CustomPaintImage` and the paint worklet lookup) comes with worklets.
  class StylePaintImage final : public StyleGeneratedImage
  {
  private:
    css::CSSOMString _name;
    const Ref<css::VariableData> _arguments;

  public:
    static constexpr bool IsFixedSize = false;

    KRYS_NODISCARD static Ref<StylePaintImage> Create(css::CSSOMString name,
                                                      Ref<css::VariableData> arguments) noexcept
    {
      return AdoptRef(*new StylePaintImage(krys::move(name), krys::move(arguments)));
    }
    ~StylePaintImage() noexcept override;

  private:
    StylePaintImage(css::CSSOMString &&, Ref<css::VariableData> &&) noexcept;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StylePaintImage, IsPaintImage)
