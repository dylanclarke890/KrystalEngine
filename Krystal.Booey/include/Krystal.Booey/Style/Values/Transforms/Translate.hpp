/*
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionWrapper.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TranslateTransformFunction.hpp"

namespace krys::boo::style
{
  // <'translate'> = none | <length-percentage> [ <length-percentage> <length>? ]?
  // https://drafts.csswg.org/css-transforms-2/#propdef-translate
  struct Translate
  {
    struct Function : TransformFunctionWrapper<TranslateTransformFunction>
    {
      using TransformFunctionWrapper<TranslateTransformFunction>::TransformFunctionWrapper;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...) const;
    };

    Translate(css::keywords::None) noexcept : _value {nullptr}
    {
    }
    Translate(Function &&value) noexcept : _value {krys::move(value.Value)}
    {
    }
    Translate(Ref<const TranslateTransformFunction> &&value) noexcept : _value {krys::move(value)}
    {
    }

    KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept
    {
      return !_value || _value->IsRepresentableIn2D();
    }
    KRYS_NODISCARD bool Is3DOperation() const noexcept
    {
      return _value && _value->Is3DOperation();
    }

    KRYS_NODISCARD TransformFunctionSizeDependencies ComputeSizeDependencies() const noexcept;

    void Apply(TransformationMatrix &, const FloatSize &) const noexcept;

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_value;
    }
    KRYS_NODISCARD bool IsFunction() const noexcept
    {
      return !!_value;
    }

    template <typename>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept;
    template <typename... F>
    decltype(auto) SwitchOn(F &&...) const;

    KRYS_NODISCARD bool operator==(const Translate &other) const noexcept
    {
      return _value.get() == other._value.get() || (_value && other._value && *_value == *other._value);
    }

  private:
    friend struct Blending<Translate>;
    friend struct ToPlatform<Translate>;

    RefPtr<const TranslateTransformFunction> _value;
  };

  // MARK: Translate Operation

  template <typename... F>
  decltype(auto) Translate::Function::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    const auto &protectedValue = Value;
    if (!protectedValue->Z().IsZero())
    {
      return visitor(
        css::SpaceSeparatedTuple {protectedValue->X(), protectedValue->Y(), protectedValue->Z()});
    }
    if (!protectedValue->Y().IsKnownZero() || protectedValue->Y().IsPercent())
    {
      return visitor(css::SpaceSeparatedTuple {protectedValue->X(), protectedValue->Y()});
    }
    return visitor(css::SpaceSeparatedTuple {protectedValue->X()});
  }

  // MARK: Translate

  template <typename T>
  bool Translate::HoldsAlternative() const noexcept
  {
    if constexpr (std::same_as<T, css::keywords::None>)
    {
      return IsNone();
    }
    else if constexpr (std::same_as<T, Function>)
    {
      return IsFunction();
    }
  }

  template <typename... F>
  decltype(auto) Translate::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    if (!_value)
    {
      return visitor(css::keywords::None {});
    }
    return visitor(Function {Ref<const TranslateTransformFunction> {_value}});
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Translate>
  {
    KRYS_NODISCARD Translate operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Translate>
  {
    KRYS_NODISCARD Translate Blend(const Translate &, const Translate &, const BlendingContext &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<Translate>
  {
    KRYS_NODISCARD RefPtr<TransformOperation> operator()(const Translate &, const FloatSize &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Translate::Function)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Translate)
