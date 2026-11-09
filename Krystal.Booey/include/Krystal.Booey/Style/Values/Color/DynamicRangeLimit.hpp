/*
 * Copyright (C) 2025 Sam Weinig. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/CompactVariant.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  class PlatformDynamicRangeLimit;
}

namespace krys::boo::css
{
  struct DynamicRangeLimit;
}

namespace krys::boo::style
{
  /// @brief The computed `dynamic-range-limit`.
  struct DynamicRangeLimit
  {
  private:
    // The mix function is never null, except after a move.
    using Kind = CompactVariant<css::keywords::Standard, css::keywords::Constrained, css::keywords::NoLimit,
                                UniquePtr<DynamicRangeLimitMixFunction>>;

    Kind _value;

  public:
    DynamicRangeLimit(css::keywords::Standard keyword) noexcept;
    DynamicRangeLimit(css::keywords::Constrained keyword) noexcept;
    DynamicRangeLimit(css::keywords::NoLimit keyword) noexcept;
    DynamicRangeLimit(DynamicRangeLimitMixFunction &&mix) noexcept;

    DynamicRangeLimit(DynamicRangeLimit &&) noexcept = default;
    DynamicRangeLimit &operator=(DynamicRangeLimit &&) noexcept = default;
    DynamicRangeLimit(const DynamicRangeLimit &other) noexcept;
    DynamicRangeLimit &operator=(const DynamicRangeLimit &other) noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const;

    KRYS_NODISCARD PlatformDynamicRangeLimit ToPlatformDynamicRangeLimit() const noexcept;

    KRYS_NODISCARD bool operator==(const DynamicRangeLimit &) const noexcept = default;

  private:
    KRYS_NODISCARD static Kind CopyKind(const Kind &other) noexcept;
  };

  inline DynamicRangeLimit::DynamicRangeLimit(css::keywords::Standard keyword) noexcept : _value {keyword}
  {
  }

  inline DynamicRangeLimit::DynamicRangeLimit(css::keywords::Constrained keyword) noexcept : _value {keyword}
  {
  }

  inline DynamicRangeLimit::DynamicRangeLimit(css::keywords::NoLimit keyword) noexcept : _value {keyword}
  {
  }

  inline DynamicRangeLimit::DynamicRangeLimit(DynamicRangeLimitMixFunction &&mix) noexcept
      : _value {CreateUnique<DynamicRangeLimitMixFunction>(krys::move(mix))}
  {
  }

  inline DynamicRangeLimit::DynamicRangeLimit(const DynamicRangeLimit &other) noexcept
      : _value {CopyKind(other._value)}
  {
  }

  inline DynamicRangeLimit &DynamicRangeLimit::operator=(const DynamicRangeLimit &other) noexcept
  {
    if (this == &other)
    {
      return *this;
    }

    _value = CopyKind(other._value);
    return *this;
  }

  template <typename... F>
  decltype(auto) DynamicRangeLimit::SwitchOn(F &&...f) const
  {
    auto visitor = CreateVisitor(std::forward<F>(f)...);
    using ResultType = decltype(visitor(std::declval<css::keywords::Standard>()));

    return krys::SwitchOn(
      _value, [&]<css::ValueId Id>(const css::KeywordValueConstant<Id> &keyword) -> ResultType
      { return visitor(keyword); },
      [&](const UniquePtr<DynamicRangeLimitMixFunction> &mix) -> ResultType { return visitor(*mix); });
  }

  inline DynamicRangeLimit::Kind DynamicRangeLimit::CopyKind(const Kind &other) noexcept
  {
    return krys::SwitchOn(
      other, []<css::ValueId Id>(const css::KeywordValueConstant<Id> &keyword) { return Kind {keyword}; },
      [](const UniquePtr<DynamicRangeLimitMixFunction> &mix)
      { return Kind {CreateUnique<DynamicRangeLimitMixFunction>(*mix)}; });
  }

  // MARK: Conversion

  template <>
  struct ToCSS<DynamicRangeLimit>
  {
    KRYS_NODISCARD css::DynamicRangeLimit operator()(const DynamicRangeLimit &limit,
                                                     const RenderStyle &style) noexcept;
  };

  template <>
  struct ToStyle<css::DynamicRangeLimit>
  {
    KRYS_NODISCARD DynamicRangeLimit operator()(const css::DynamicRangeLimit &limit,
                                                const BuilderState &state) noexcept;
    KRYS_NODISCARD DynamicRangeLimit operator()(const css::DynamicRangeLimit &limit,
                                                const css::ToLengthConversionData &conversionData) noexcept;
  };

  template <>
  struct CSSValueCreation<DynamicRangeLimit>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const DynamicRangeLimit &) noexcept;
  };

  template <>
  struct CSSValueConversion<DynamicRangeLimit>
  {
    KRYS_NODISCARD DynamicRangeLimit operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: Serialisation

  template <>
  struct Serialiser<DynamicRangeLimit>
  {
    void operator()(StringBuilder &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const DynamicRangeLimit &value) noexcept;
  };

  // MARK: Blending

  template <>
  struct Blending<DynamicRangeLimit>
  {
    KRYS_NODISCARD DynamicRangeLimit Blend(const DynamicRangeLimit &from, const DynamicRangeLimit &to,
                                           const BlendingContext &context) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::DynamicRangeLimit)
