#pragma once

#include "Krystal.Booey/Style/Values/Transforms/Functions/RotateTransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionWrapper.hpp"

namespace krys::boo::style
{
  // <'rotate'> = none | <angle> | [ x | y | z | <number>{3} ] && <angle>
  // https://drafts.csswg.org/css-transforms-2/#propdef-rotate
  struct Rotate
  {
    struct Function : TransformFunctionWrapper<RotateTransformFunction>
    {
      using TransformFunctionWrapper<RotateTransformFunction>::TransformFunctionWrapper;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...) const;
    };

    Rotate(css::keywords::None) noexcept : _value {nullptr}
    {
    }
    Rotate(Function &&value) noexcept : _value {krys::move(value.Value)}
    {
    }
    Rotate(Ref<const RotateTransformFunction> &&value) noexcept : _value {krys::move(value)}
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

    KRYS_NODISCARD bool operator==(const Rotate &other) const noexcept
    {
      return _value.get() == other._value.get() || (_value && other._value && *_value == *other._value);
    }

  private:
    friend struct Blending<Rotate>;
    friend struct ToPlatform<Rotate>;

    RefPtr<const RotateTransformFunction> _value;
  };

  // MARK: Rotate Function

  template <typename... F>
  decltype(auto) Rotate::Function::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    const auto &protectedValue = Value;
    if (!protectedValue->Is3DOperation()
        || (protectedValue->X().IsZero() && protectedValue->Y().IsZero() && !protectedValue->Z().IsZero()))
    {
      return visitor(protectedValue->GetAngle());
    }
    if (!protectedValue->X().IsZero() && protectedValue->Y().IsZero() && protectedValue->Z().IsZero())
    {
      return visitor(css::SpaceSeparatedTuple {css::keywords::X {}, protectedValue->GetAngle()});
    }
    if (protectedValue->X().IsZero() && !protectedValue->Y().IsZero() && protectedValue->Z().IsZero())
    {
      return visitor(css::SpaceSeparatedTuple {css::keywords::Y {}, protectedValue->GetAngle()});
    }
    return visitor(css::SpaceSeparatedTuple {
      protectedValue->X(),
      protectedValue->Y(),
      protectedValue->Z(),
      protectedValue->GetAngle(),
    });
  }

  // MARK: Rotate

  template <typename T>
  bool Rotate::HoldsAlternative() const noexcept
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
  decltype(auto) Rotate::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    if (!_value)
    {
      return visitor(css::keywords::None {});
    }
    return visitor(Function {Ref<const RotateTransformFunction> {_value}});
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Rotate>
  {
    KRYS_NODISCARD Rotate operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Rotate>
  {
    KRYS_NODISCARD Rotate Blend(const Rotate &, const Rotate &, const BlendingContext &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<Rotate>
  {
    KRYS_NODISCARD RefPtr<TransformOperation> operator()(const Rotate &, const FloatSize &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Rotate::Function)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Rotate)
