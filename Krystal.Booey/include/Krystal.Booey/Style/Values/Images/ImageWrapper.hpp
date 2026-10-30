#pragma once

#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo::style
{
  // Utility type that wraps a StyleImage for use with the strong style type system.
  struct ImageWrapper
  {
    Ref<StyleImage> Value;

    KRYS_NODISCARD bool operator==(const ImageWrapper &other) const noexcept
    {
      return ArePointingToEqualData(Value, other.Value);
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueCreation<ImageWrapper>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const ImageWrapper &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<ImageWrapper>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const ImageWrapper &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ImageWrapper>
  {
    KRYS_NODISCARD ImageWrapper Blend(const ImageWrapper &, const ImageWrapper &,
                                      const BlendingContext &) noexcept;
  };
}
