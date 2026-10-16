#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <cursor-image> = [ <url> | <url-set> ] <number>{2}?
  // https://drafts.csswg.org/css-ui-4/#typedef-cursor-cursor-image
  struct CursorImage
  {
    Ref<StyleImage> Image;
    IntPoint HotSpot {-1, -1};

    KRYS_NODISCARD bool operator==(const CursorImage &) const noexcept = default;
  };

  using CursorImageList = css::CommaSeparatedRefCountedFixedVector<CursorImage>;

  // <'cursor'> = <cursor-image>#? <cursor-predefined>
  // https://drafts.csswg.org/css-ui-4/#propdef-cursor
  struct Cursor
  {
    using ImagesType = Maybe<CursorImageList>;

    ImagesType Images {};
    CursorType Predefined {CursorType::Auto};

    Cursor(ImagesType &&images, CursorType predefined) noexcept
        : Images {krys::move(images)}, Predefined {predefined}
    {
    }

    Cursor(const ImagesType &images, CursorType predefined) noexcept
        : Images {images}, Predefined {predefined}
    {
    }

    explicit Cursor(CursorType predefined) noexcept : Predefined {predefined}
    {
    }

    // Special constructor for use constructing initial 'auto' value.
    Cursor(css::keywords::Auto) noexcept : Predefined {CursorType::Auto}
    {
    }

    KRYS_NODISCARD bool operator==(const Cursor &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Cursor &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Images;
    }
    else if constexpr (I == 1)
    {
      return value.Predefined;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Cursor>
  {
    KRYS_NODISCARD Cursor operator()(BuilderState &, const css::Value &) noexcept;
  };

  template <>
  struct CSSValueCreation<CursorImage>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const CursorImage &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<CursorImage>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const CursorImage &) noexcept;
  };
}

DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Cursor, 2)
