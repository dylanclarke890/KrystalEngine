#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"

namespace krys::boo
{
  /// @brief A `-webkit-canvas()` image: the contents of a named CSS canvas.
  /// FIXME: Observing the document's CSS canvas element (`CanvasObserver`, `element`) and drawing need the
  /// document's CSS canvases (`getCSSCanvasContext`), which only scripts make.
  class StyleCanvasImage final : public StyleGeneratedImage
  {
  private:
    css::CSSOMString _name;

  public:
    static constexpr bool IsFixedSize = true;

    KRYS_NODISCARD static Ref<StyleCanvasImage> Create(css::CSSOMString name) noexcept
    {
      return AdoptRef(*new StyleCanvasImage(krys::move(name)));
    }
    ~StyleCanvasImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleCanvasImage &) const noexcept;

  private:
    explicit StyleCanvasImage(css::CSSOMString &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleCanvasImage, IsCanvasImage)
