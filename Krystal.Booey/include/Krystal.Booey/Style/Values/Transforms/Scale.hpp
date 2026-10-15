#pragma once

#include "Krystal.Booey/Style/Values/Transforms/Functions/ScaleTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionWrapper.hpp"

namespace krys::boo::style
{
  // <'scale'> = none | [ <number> | <percentage> ]{1,3}
  // https://drafts.csswg.org/css-transforms-2/#propdef-scale
  struct Scale
  {
    struct Function : TransformFunctionWrapper<ScaleTransformFunction>
    {
      using TransformFunctionWrapper<ScaleTransformFunction>::TransformFunctionWrapper;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...) const;
    };

    Scale(css::keywords::None) noexcept : _value {nullptr}
    {
    }
    Scale(Function &&value) noexcept : _value {krys::move(value.Value)}
    {
    }
    Scale(Ref<const ScaleTransformFunction> &&value) noexcept : _value {krys::move(value)}
    {
    }

    KRYS_NODISCARD bool AffectedByTransformOrigin() const noexcept
    {
      return _value && !_value->IsIdentity();
    }
    KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept
    {
      return !_value || _value->IsRepresentableIn2D();
    }
    KRYS_NODISCARD bool Is3DOperation() const noexcept
    {
      return _value && _value->Is3DOperation();
    }

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

    KRYS_NODISCARD bool operator==(const Scale &other) const noexcept
    {
      return _value.get() == other._value.get() || (_value && other._value && *_value == *other._value);
    }

  private:
    friend struct Blending<Scale>;
    friend struct ToPlatform<Scale>;

    RefPtr<const ScaleTransformFunction> _value;
  };

  // MARK: Scale Function

  template <typename... F>
  decltype(auto) Scale::Function::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    const auto &protectedValue = Value;
    if (protectedValue->Z() != 1)
    {
      return visitor(
        css::SpaceSeparatedTuple {protectedValue->X(), protectedValue->Y(), protectedValue->Z()});
    }
    if (protectedValue->X() != protectedValue->Y())
    {
      return visitor(css::SpaceSeparatedTuple {protectedValue->X(), protectedValue->Y()});
    }
    return visitor(protectedValue->X());
  }

  // MARK: Scale

  template <typename T>
  bool Scale::HoldsAlternative() const noexcept
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
  decltype(auto) Scale::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    if (!_value)
    {
      return visitor(css::keywords::None {});
    }
    return visitor(Function {Ref<const ScaleTransformFunction> {_value}});
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Scale>
  {
    KRYS_NODISCARD Scale operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Scale>
  {
    KRYS_NODISCARD Scale Blend(const Scale &, const Scale &, const BlendingContext &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<Scale>
  {
    KRYS_NODISCARD RefPtr<TransformOperation> operator()(const Scale &, const FloatSize &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Scale::Function)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Scale)
