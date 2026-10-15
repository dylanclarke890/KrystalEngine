#pragma once

#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"

namespace krys::boo::style
{
  // <'border-image-source'> = none | <image>
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-source
  struct BorderImageSource
  {
    BorderImageSource(css::keywords::None) noexcept
    {
    }

    BorderImageSource(ImageWrapper &&image) noexcept : _image {krys::move(image.Value)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_image;
    }
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return !!_image;
    }

    KRYS_NODISCARD Maybe<ImageWrapper> TryImage() const noexcept
    {
      return _image ? Maybe<ImageWrapper> {ImageWrapper {ShareRef(*_image)}} : std::nullopt;
    }
    KRYS_NODISCARD RefPtr<StyleImage> TryStyleImage() const noexcept
    {
      return _image;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(ImageWrapper {ShareRef(*_image)});
    }

    bool operator==(const BorderImageSource &other) const noexcept
    {
      return ArePointingToEqualData(_image, other._image);
    }

  private:
    RefPtr<StyleImage> _image {};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BorderImageSource>
  {
    KRYS_NODISCARD BorderImageSource operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BorderImageSource>
  {
    KRYS_NODISCARD bool CanBlend(const BorderImageSource &, const BorderImageSource &) noexcept;
    KRYS_NODISCARD BorderImageSource Blend(const BorderImageSource &, const BorderImageSource &,
                                           const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BorderImageSource)
