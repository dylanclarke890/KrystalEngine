/*
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterRenderingMode.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterStyle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class Filter;
  class FilterImage;
  class FilterResults;
  class GraphicsContext;

  enum class FilterRepresentation : uint8
  {
    TestOutput,
    Debugging
  };

  /// @brief A step of a filter: a whole filter (a chain of functions), or one filter effect.
  class FilterFunction : public RefCounted<FilterFunction>
  {
  public:
    enum class Type : uint8
    {
      CSSFilterRenderer,
      SVGFilterRenderer,

      // These are filter effects
      FEBlend,
      FEColorMatrix,
      FEComponentTransfer,
      FEComposite,
      FEConvolveMatrix,
      FEDiffuseLighting,
      FEDisplacementMap,
      FEDropShadow,
      FEFlood,
      FEGaussianBlur,
      FEImage,
      FEMerge,
      FEMorphology,
      FEOffset,
      FESpecularLighting,
      FETile,
      FETurbulence,
      SourceAlpha,
      SourceGraphic
    };

  private:
    Type _filterType;

  public:
    explicit FilterFunction(Type type) noexcept : _filterType(type)
    {
    }
    virtual ~FilterFunction() noexcept = default;

    KRYS_NODISCARD Type FilterType() const noexcept
    {
      return _filterType;
    }

    KRYS_NODISCARD bool IsCSSFilterRenderer() const noexcept
    {
      return _filterType == Type::CSSFilterRenderer;
    }
    KRYS_NODISCARD bool IsSVGFilterRenderer() const noexcept
    {
      return _filterType == Type::SVGFilterRenderer;
    }
    KRYS_NODISCARD bool IsFilter() const noexcept
    {
      return _filterType == Type::CSSFilterRenderer || _filterType == Type::SVGFilterRenderer;
    }
    KRYS_NODISCARD bool IsFilterEffect() const noexcept
    {
      return _filterType >= Type::FEBlend && _filterType <= Type::SourceGraphic;
    }

    KRYS_NODISCARD static utf8_stringview FilterName(Type type) noexcept;
    KRYS_NODISCARD static utf8_stringview SourceAlphaName() noexcept
    {
      return FilterName(Type::SourceAlpha);
    }
    KRYS_NODISCARD static utf8_stringview SourceGraphicName() noexcept
    {
      return FilterName(Type::SourceGraphic);
    }
    KRYS_NODISCARD utf8_stringview FilterName() const noexcept
    {
      return FilterName(_filterType);
    }

    /// @brief The rendering modes the function supports, of @p preferredFilterRenderingModes: by default,
    /// the accelerated backend, which applies every effect.
    KRYS_NODISCARD virtual EnumSet<FilterRenderingMode>
      SupportedFilterRenderingModes(EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept
    {
      return EnumSet<FilterRenderingMode> {FilterRenderingMode::Accelerated} & preferredFilterRenderingModes;
    }
    /// @brief The function applied to @p input; null when it can't be.
    KRYS_NODISCARD virtual RefPtr<FilterImage> Apply(const Filter &, FilterImage &input,
                                                     FilterResults &) noexcept;
    /// @brief The styles that apply the function in the graphics context, following @p input.
    KRYS_NODISCARD virtual FilterStyleVector CreateFilterStyles(GraphicsContext &, const Filter &,
                                                                const FilterStyle & /*input*/) const noexcept
    {
      return {};
    }

    virtual TextStream &
      ExternalRepresentation(TextStream &,
                             FilterRepresentation = FilterRepresentation::TestOutput) const noexcept = 0;
  };

  TextStream &operator<<(TextStream &ts, const FilterFunction &filterFunction) noexcept;
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(ClassName)                                               \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ClassName)                                               \
    KRYS_NODISCARD static bool IsType(const krys::boo::FilterFunction &filter) noexcept                      \
    {                                                                                                        \
      return filter.FilterType() == krys::boo::FilterFunction::Type::ClassName;                              \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
