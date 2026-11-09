/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace css
  {
    struct Filter;
    struct FilterData;
  }

  namespace style
  {
    // Any <filter-function> or a reference to filter via <url>.
    // https://drafts.fxtf.org/filter-effects/#typedef-filter-function
    struct FilterValue
    {
      Ref<FilterOperation> Value;

      FilterValue(Ref<FilterOperation> value) noexcept : Value {krys::move(value)}
      {
      }

      KRYS_NODISCARD const FilterOperation &Get() const noexcept
      {
        return *Value;
      }
      KRYS_NODISCARD const FilterOperation &Platform() const noexcept
      {
        return *Value;
      }
      KRYS_NODISCARD const FilterOperation *operator->() const noexcept
      {
        return Value.get();
      }

      KRYS_NODISCARD bool operator==(const FilterValue &other) const noexcept
      {
        return Value.get() == other.Value.get() || *Value == *other.Value;
      }
    };

    // <filter-value-list> = [ <filter-function> | <url> ]+
    // https://drafts.fxtf.org/filter-effects/#typedef-filter-value-list
    using FilterValueList = css::SpaceSeparatedFixedList<FilterValue>;

    // <'filter'> = none | <filter-value-list>
    // https://drafts.fxtf.org/filter-effects/#propdef-filter
    struct Filter : css::ListOrNone<FilterValueList>
    {
      friend struct Blending<Filter>;

      using css::ListOrNone<FilterValueList>::ListOrNone;

      KRYS_NODISCARD bool HasFilterThatRequiresRepaintForCurrentColorChange() const noexcept;

      KRYS_NODISCARD bool HasFilterThatAffectsOpacity() const noexcept;
      KRYS_NODISCARD bool HasFilterThatMovesPixels() const noexcept;
      KRYS_NODISCARD bool HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept;

      template <FilterOperation::Type Type>
      KRYS_NODISCARD bool HasFilterOfType() const noexcept;

      KRYS_NODISCARD bool HasReferenceFilter() const noexcept;
      KRYS_NODISCARD bool IsReferenceFilter() const noexcept;

      KRYS_NODISCARD IntOutsets Outsets() const noexcept;
    };

    template <FilterOperation::Type type>
    bool Filter::HasFilterOfType() const noexcept
    {
      return std::ranges::any_of(*this, [](const auto &op) { return op->GetType() == type; });
    }

    // MARK: - Conversion

    template <>
    struct ToCSS<FilterValue>
    {
      KRYS_NODISCARD css::FilterData operator()(const FilterValue &, const RenderStyle &) noexcept;
    };
    template <>
    struct ToStyle<css::FilterData>
    {
      KRYS_NODISCARD FilterValue operator()(const css::FilterData &, const BuilderState &) noexcept;
    };

    template <>
    struct ToCSS<Filter>
    {
      KRYS_NODISCARD css::Filter operator()(const Filter &, const RenderStyle &) noexcept;
    };
    template <>
    struct ToStyle<css::Filter>
    {
      KRYS_NODISCARD Filter operator()(const css::Filter &, const BuilderState &) noexcept;
    };
    template <>
    struct CSSValueConversion<Filter>
    {
      KRYS_NODISCARD Filter operator()(BuilderState &, const css::Value &) noexcept;
    };
    template <>
    struct CSSValueCreation<Filter>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const Filter &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<Filter>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const Filter &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<Filter>
    {
      KRYS_NODISCARD bool CanBlend(const Filter &, const Filter &, CompositeOperation) noexcept;
      KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const Filter &,
                                                                                  const Filter &) noexcept
      {
        return true;
      }
      KRYS_NODISCARD Filter Blend(const Filter &, const Filter &, const BlendingContext &) noexcept;
    };

    // MARK: - Platform

    template <>
    struct ToPlatform<FilterValue>
    {
      KRYS_NODISCARD Ref<FilterOperation> operator()(const FilterValue &) noexcept;
    };
    template <>
    struct ToPlatform<Filter>
    {
      KRYS_NODISCARD FilterOperations operator()(const Filter &) noexcept;
    };
  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Filter)
