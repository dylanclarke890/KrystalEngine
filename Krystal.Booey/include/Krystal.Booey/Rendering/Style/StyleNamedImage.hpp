#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"

namespace krys::boo
{
  /// @brief A named image (`-webkit-named-image()`).
  /// FIXME: Drawing (`NamedImageGeneratedImage`) comes with the graphics backend.
  class StyleNamedImage final : public StyleGeneratedImage
  {
  private:
    css::CSSOMString _name;

  public:
    static constexpr bool IsFixedSize = false;

    KRYS_NODISCARD static Ref<StyleNamedImage> Create(css::CSSOMString name) noexcept
    {
      return AdoptRef(*new StyleNamedImage(krys::move(name)));
    }
    ~StyleNamedImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleNamedImage &) const noexcept;

  private:
    explicit StyleNamedImage(css::CSSOMString &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleNamedImage, IsNamedImage)
