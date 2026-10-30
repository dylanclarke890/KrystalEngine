#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMultiImage.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief An image in a `cursor` value, with its hot spot.
  /// FIXME: Tracking the `<cursor>` SVG elements the URL refers to (`cursorElementRemoved`,
  /// `cursorElementChanged`, `updateCursorElement`), once there's `SVGCursorElement`.
  class StyleCursorImage final : public StyleMultiImage
  {
  private:
    Ref<StyleImage> _image;
    Maybe<IntPoint> _hotSpot;
    style::URL _originalURL;

  public:
    KRYS_NODISCARD static Ref<StyleCursorImage> Create(const Ref<StyleImage> &, Maybe<IntPoint>,
                                                       const style::URL &) noexcept;
    KRYS_NODISCARD static Ref<StyleCursorImage> Create(Ref<StyleImage> &&, Maybe<IntPoint>,
                                                       style::URL &&) noexcept;
    ~StyleCursorImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleCursorImage &) const noexcept;
    KRYS_NODISCARD bool EqualInputImages(const StyleCursorImage &) const noexcept;

    KRYS_NODISCARD bool UsesDataProtocol() const noexcept final;

    KRYS_NODISCARD Maybe<IntPoint> HotSpot() const noexcept
    {
      return _hotSpot;
    }

  private:
    StyleCursorImage(const Ref<StyleImage> &, Maybe<IntPoint> hotSpot, const style::URL &) noexcept;
    StyleCursorImage(Ref<StyleImage> &&, Maybe<IntPoint> hotSpot, style::URL &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD ImageWithScale SelectBestFitImage(const dom::Document &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleCursorImage, IsCursorImage)
