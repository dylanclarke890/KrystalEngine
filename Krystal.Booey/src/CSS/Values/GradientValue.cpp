/*
 * Copyright (C) 2008-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGradientImage.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::css
{
  GradientValue::~GradientValue() noexcept = default;

  GradientValue::GradientValue(boo::css::Gradient &&gradient) noexcept
      : Value(ValueType::Gradient), _gradient(krys::move(gradient))
  {
  }

  CSSOMString GradientValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _gradient);
  }

  bool GradientValue::Equals(const GradientValue &other) const noexcept
  {
    return _gradient == other._gradient;
  }

  IterationStatus GradientValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _gradient);
  }

  namespace
  {
    template <typename>
    struct StyleImageIsUncacheable;

    template <typename TCSSType>
    KRYS_NODISCARD bool IsStyleImageUncacheable(const TCSSType &value) noexcept
    {
      return StyleImageIsUncacheable<TCSSType> {}(value);
    }

    template <>
    struct StyleImageIsUncacheable<GradientColorInterpolationMethod>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &) noexcept
      {
        return false;
      }
    };

    template <>
    struct StyleImageIsUncacheable<Color>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return ContainsCurrentColor(value) || ContainsColorSchemeDependentColor(value);
      }
    };

    template <ValueId C>
    struct StyleImageIsUncacheable<KeywordValueConstant<C>>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &) noexcept
      {
        return false;
      }
    };

    template <UnitEnum TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &value) noexcept
      {
        return ConversionToCanonicalUnitRequiresConversionData(value);
      }
    };

    template <NumericRaw TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &value) noexcept
      {
        return IsStyleImageUncacheable(value.Unit);
      }
    };

    template <Calc TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return value.ProtectedCalc()->RequiresConversionData();
      }
    };

    template <OptionalLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return value && IsStyleImageUncacheable(*value);
      }
    };

    template <TupleLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return krys::apply([&](const auto &...x) { return (IsStyleImageUncacheable(x) || ...); }, value);
      }
    };

    template <RangeLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return std::ranges::any_of(value,
                                   [](const auto &element) { return IsStyleImageUncacheable(element); });
      }
    };

    template <VariantLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return krys::SwitchOn(value,
                              [](const auto &alternative) { return IsStyleImageUncacheable(alternative); });
      }
    };
  }

  RefPtr<StyleImage> GradientValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    if (_cachedStyleImage)
    {
      return _cachedStyleImage;
    }

    auto styleImage = StyleGradientImage::Create(style::ConvertToStyle(_gradient, state));
    if (!IsStyleImageUncacheable(_gradient))
    {
      _cachedStyleImage = styleImage;
    }

    return styleImage;
  }
}
