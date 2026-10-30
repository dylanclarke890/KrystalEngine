#pragma once

#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"

namespace krys::boo::style
{
  struct ImageOrNone
  {
    ImageOrNone(css::keywords::None) noexcept
    {
    }

    ImageOrNone(ImageWrapper &&image) noexcept : _value {krys::move(image.Value)}
    {
    }

    ImageOrNone(RefPtr<StyleImage> &&image) noexcept : _value {krys::move(image)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_value;
    }
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return !!_value;
    }

    KRYS_NODISCARD Maybe<ImageWrapper> TryImage() const noexcept
    {
      return _value ? Maybe<ImageWrapper> {ImageWrapper {ShareRef(*_value)}} : std::nullopt;
    }
    KRYS_NODISCARD RefPtr<StyleImage> TryStyleImage() const noexcept
    {
      return _value;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(ImageWrapper {ShareRef(*_value)});
    }

    KRYS_NODISCARD bool operator==(const ImageOrNone &other) const noexcept
    {
      return ArePointingToEqualData(_value, other._value);
    }

  private:
    RefPtr<StyleImage> _value {};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ImageOrNone>
  {
    KRYS_NODISCARD ImageOrNone operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ImageOrNone>
  {
    KRYS_NODISCARD bool CanBlend(const ImageOrNone &, const ImageOrNone &) noexcept;
    KRYS_NODISCARD ImageOrNone Blend(const ImageOrNone &, const ImageOrNone &,
                                     const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ImageOrNone)
