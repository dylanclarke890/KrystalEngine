/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Color/AbsoluteColorResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  template <typename D, size_t Index>
  using AbsoluteColorComponent = GetCSSColorParseTypeWithCalcComponentResult<D, Index>;

  template <typename D>
  struct AbsoluteColor
  {
    using Descriptor = D;

    CSSColorParseTypeWithCalc<Descriptor> Components;

    KRYS_NODISCARD bool operator==(const AbsoluteColor<Descriptor> &) const noexcept = default;
  };

  template <typename D>
  KRYS_NODISCARD boo::Color CreateColor(const AbsoluteColor<D> &unresolved,
                                        PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};

    auto resolver =
      AbsoluteColorResolver<D> {.Components = unresolved.Components, .NestingLevel = state.NestingLevel};

    if (state.ConversionData)
    {
      return Resolve(krys::move(resolver), *state.ConversionData);
    }

    if (!ComponentsRequireConversionData<D>(resolver.Components))
    {
      return ResolveNoConversionDataRequired(krys::move(resolver));
    }

    return {};
  }

  template <typename D>
  KRYS_NODISCARD constexpr bool ContainsColorSchemeDependentColor(const AbsoluteColor<D> &) noexcept
  {
    return false;
  }

  template <typename D>
  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const AbsoluteColor<D> &) noexcept
  {
    return false;
  }

  template <typename D>
  struct Serialiser<AbsoluteColor<D>>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const AbsoluteColor<D> &value) noexcept
    {
      if constexpr (D::UsesColorFunctionForSerialisation)
      {
        builder.Append(u8"color(");
        builder.Append(boo::Serialisation(ColourSpaceFor<typename D::ColorType>));
        builder.Append(u8' ');
      }
      else
      {
        builder.Append(D::SerialisationFunctionName);
        builder.Append(u8'(');
      }

      const auto &[c1, c2, c3, alpha] = value.Components;

      Serialise(builder, context, c1);
      builder.Append(u8' ');
      Serialise(builder, context, c2);
      builder.Append(u8' ');
      Serialise(builder, context, c3);

      if (alpha)
      {
        builder.Append(u8" / ");
        Serialise(builder, context, *alpha);
      }

      builder.Append(u8')');
    }
  };

  template <typename D>
  struct ComputedStyleDependenciesCollector<AbsoluteColor<D>>
  {
    void operator()(ComputedStyleDependencies &dependencies, const AbsoluteColor<D> &value) noexcept
    {
      CollectComputedStyleDependencies(dependencies, std::get<0>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<1>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<2>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<3>(value.Components));
    }
  };

  template <typename D>
  struct ValueChildrenVisitor<AbsoluteColor<D>>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const AbsoluteColor<D> &value) noexcept
    {
      if (VisitValueChildren(func, std::get<0>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (VisitValueChildren(func, std::get<1>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (VisitValueChildren(func, std::get<2>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (VisitValueChildren(func, std::get<3>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}
