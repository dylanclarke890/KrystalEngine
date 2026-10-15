#pragma once

#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"

namespace krys::boo::style
{
  // <'mask-border-source'> = none | <image>
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-source
  struct MaskBorderSource
  {
    MaskBorderSource(css::keywords::None) noexcept
    {
    }

    MaskBorderSource(ImageWrapper &&image) noexcept : _image {krys::move(image.Value)}
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

    bool operator==(const MaskBorderSource &other) const noexcept
    {
      return ArePointingToEqualData(_image, other._image);
    }

  private:
    RefPtr<StyleImage> _image {};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MaskBorderSource>
  {
    KRYS_NODISCARD MaskBorderSource operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<MaskBorderSource>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderSource &, const MaskBorderSource &) noexcept;
    KRYS_NODISCARD MaskBorderSource Blend(const MaskBorderSource &, const MaskBorderSource &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaskBorderSource)
