#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Values/Primitives/Url.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"

namespace krys::web
{
  class StyleCursorImage;
}

namespace krys::web::style
{
  class BuilderState;
}

namespace krys::web::css
{
  class ValuePair;

  /// @brief One image of the `cursor` property: a `url()` or `image-set()`, and the optional `<x> <y>` of
  /// its hot spot.
  /// @see https://www.w3.org/TR/css-ui-4/#cursor
  class CursorImageValue final : public Value
  {
  private:
    Ref<Value> _image;
    /// @brief Two numbers, the x and y of the point in the image that is the pointer's position.
    RefPtr<ValuePair> _hotSpot;
    /// @brief The image's URL, kept for the style image; none for an `image-set()`.
    css::Url _originalUrl;

  public:
    ~CursorImageValue() noexcept;

    /// @brief The cursor image @p image, whose URL is the original URL if it's a `url()`.
    KRYS_NODISCARD static Ref<CursorImageValue> Create(Ref<Value> image, RefPtr<ValuePair> hotSpot) noexcept;

    /// @brief The cursor image @p image with @p originalUrl, as a computed style gives back.
    KRYS_NODISCARD static Ref<CursorImageValue> Create(Ref<Value> image, RefPtr<ValuePair> hotSpot,
                                                       css::Url originalUrl) noexcept;

    /// @brief The style image for this value, with its hot spot truncated to whole pixels.
    /// @return Null if @p state can't make a style image of the image.
    KRYS_NODISCARD RefPtr<StyleCursorImage> CreateStyleImage(const style::BuilderState &state) const noexcept;

    // The Value operations.

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    /// @brief Whether both have equal images and hot spots; the original URLs aren't compared.
    KRYS_NODISCARD bool Equals(const CursorImageValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;

  private:
    CursorImageValue(Ref<Value> image, RefPtr<ValuePair> hotSpot, css::Url originalUrl) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::css::CursorImageValue)
  KRYS_NODISCARD static bool IsType(const krys::web::css::Value &value) noexcept
  {
    return value.Type() == krys::web::css::ValueType::CursorImage;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
