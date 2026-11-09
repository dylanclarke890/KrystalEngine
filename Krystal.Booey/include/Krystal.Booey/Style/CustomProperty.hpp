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

#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/ValueSeparator.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Booey/Style/Values/Transforms/TransformFunction.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  namespace css
  {
    class Value;
    class ValuePool;
  }

  class RenderStyle;

  namespace style
  {
    /// @brief The computed value of a custom property: its tokens, or a typed value when it's registered.
    class CustomProperty final : public RefCounted<CustomProperty>
    {
    public:
      // https://drafts.csswg.org/css-variables-2/#guaranteed-invalid
      struct GuaranteedInvalid
      {
        KRYS_NODISCARD constexpr bool operator==(const GuaranteedInvalid &) const noexcept = default;
      };

      using Value =
        Variant<LengthPercentage<>, Length<>, Number<>, Percentage<>, Angle<>, Time<>, Resolution<>,
                ImageWrapper, Color, URL, css::CustomIdentifier, css::CSSOMString, TransformFunction>;

      struct ValueList
      {
        List<Value> Values;
        css::ValueSeparator Separator;

        KRYS_NODISCARD bool operator==(const ValueList &) const noexcept = default;
      };

      using Kind = Variant<GuaranteedInvalid, Ref<css::VariableData>, Value, ValueList>;

    private:
      const css::CSSOMStringAtom _name;
      const Kind _value;
      mutable RefPtr<css::VariableData> _cachedTokens;

    public:
      KRYS_NODISCARD static Ref<const CustomProperty>
        CreateForGuaranteedInvalid(const css::CSSOMStringAtom &name) noexcept;
      KRYS_NODISCARD static Ref<const CustomProperty>
        CreateForVariableData(const css::CSSOMStringAtom &name, Ref<css::VariableData> &&) noexcept;
      KRYS_NODISCARD static Ref<const CustomProperty> CreateForValue(const css::CSSOMStringAtom &name,
                                                                     Value &&) noexcept;
      KRYS_NODISCARD static Ref<const CustomProperty> CreateForValueList(const css::CSSOMStringAtom &name,
                                                                         ValueList &&) noexcept;

      KRYS_NODISCARD const css::CSSOMStringAtom &GetName() const noexcept
      {
        return _name;
      }
      KRYS_NODISCARD const Kind &GetValue() const noexcept
      {
        return _value;
      }

      KRYS_NODISCARD const List<css::Token> &Tokens() const noexcept;

      KRYS_NODISCARD bool IsGuaranteedInvalid() const noexcept;
      KRYS_NODISCARD bool IsVariableData() const noexcept;
      KRYS_NODISCARD bool IsValue() const noexcept;
      KRYS_NODISCARD bool IsValueList() const noexcept;

      KRYS_NODISCARD bool IsAnimatable() const noexcept;

      KRYS_NODISCARD bool operator==(const CustomProperty &) const noexcept;

      KRYS_NODISCARD Ref<css::Value> PropertyValue(css::ValuePool &, const RenderStyle &) const noexcept;
      KRYS_NODISCARD css::CSSOMString PropertyValueSerialisation(const css::SerialisationContext &,
                                                                 const RenderStyle &) const noexcept;
      void PropertyValueSerialisation(StringBuilder &, const css::SerialisationContext &,
                                      const RenderStyle &) const noexcept;

    private:
      CustomProperty(const css::CSSOMStringAtom &name, Kind &&value) noexcept
          : _name(name), _value(krys::move(value))
      {
      }

      KRYS_NODISCARD css::CSSOMString
        PropertyValueSerialisationForTokenization(const css::SerialisationContext &,
                                                  const RenderStyle &) const noexcept;
      void PropertyValueSerialisationForTokenization(StringBuilder &, const css::SerialisationContext &,
                                                     const RenderStyle &) const noexcept;
    };

    inline Ref<const CustomProperty>
      CustomProperty::CreateForGuaranteedInvalid(const css::CSSOMStringAtom &name) noexcept
    {
      return AdoptRef(*new CustomProperty(name, Kind {GuaranteedInvalid {}}));
    }

    inline Ref<const CustomProperty>
      CustomProperty::CreateForVariableData(const css::CSSOMStringAtom &name,
                                            Ref<css::VariableData> &&value) noexcept
    {
      return AdoptRef(
        *new CustomProperty(name, Kind {std::in_place_type<Ref<css::VariableData>>, krys::move(value)}));
    }

    inline Ref<const CustomProperty> CustomProperty::CreateForValue(const css::CSSOMStringAtom &name,
                                                                    Value &&value) noexcept
    {
      return AdoptRef(*new CustomProperty(name, Kind {std::in_place_type<Value>, krys::move(value)}));
    }

    inline Ref<const CustomProperty> CustomProperty::CreateForValueList(const css::CSSOMStringAtom &name,
                                                                        ValueList &&valueList) noexcept
    {
      return AdoptRef(*new CustomProperty(name, Kind {std::in_place_type<ValueList>, krys::move(valueList)}));
    }

    inline bool CustomProperty::IsGuaranteedInvalid() const noexcept
    {
      return std::holds_alternative<GuaranteedInvalid>(_value);
    }

    inline bool CustomProperty::IsVariableData() const noexcept
    {
      return std::holds_alternative<Ref<css::VariableData>>(_value);
    }

    inline bool CustomProperty::IsValue() const noexcept
    {
      return std::holds_alternative<Value>(_value);
    }

    inline bool CustomProperty::IsValueList() const noexcept
    {
      return std::holds_alternative<ValueList>(_value);
    }

    inline bool CustomProperty::IsAnimatable() const noexcept
    {
      return std::holds_alternative<Value>(_value) || std::holds_alternative<ValueList>(_value);
    }
  }
}
