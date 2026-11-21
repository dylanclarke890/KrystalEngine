/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class Document;
}

namespace krys::boo::css
{
  class Value;
}

namespace krys::boo
{
  /// @brief The kinds of Typed OM values.
  enum class CSSStyleValueType : uint8
  {
    CSSStyleValue,
    CSSImageValue,
    CSSTransformValue,
    CSSMathClamp,
    CSSMathInvert,
    CSSMathMin,
    CSSMathMax,
    CSSMathNegate,
    CSSMathProduct,
    CSSMathSum,
    CSSColor,
    CSSHSL,
    CSSHWB,
    CSSLab,
    CSSLCH,
    CSSOKLCH,
    CSSOKLab,
    CSSRGB,
    CSSUnitValue,
    CSSUnparsedValue,
    CSSKeywordValue,
  };
}

namespace krys::boo::css
{
  KRYS_NODISCARD constexpr bool IsCSSNumericValue(CSSStyleValueType type) noexcept
  {
    switch (type)
    {
      case CSSStyleValueType::CSSMathClamp:
      case CSSStyleValueType::CSSMathInvert:
      case CSSStyleValueType::CSSMathMin:
      case CSSStyleValueType::CSSMathMax:
      case CSSStyleValueType::CSSMathNegate:
      case CSSStyleValueType::CSSMathProduct:
      case CSSStyleValueType::CSSMathSum:
      case CSSStyleValueType::CSSUnitValue:
      {
        return true;
      }
      case CSSStyleValueType::CSSStyleValue:
      case CSSStyleValueType::CSSImageValue:
      case CSSStyleValueType::CSSTransformValue:
      case CSSStyleValueType::CSSUnparsedValue:
      case CSSStyleValueType::CSSKeywordValue:
      case CSSStyleValueType::CSSColor:
      case CSSStyleValueType::CSSHSL:
      case CSSStyleValueType::CSSHWB:
      case CSSStyleValueType::CSSLab:
      case CSSStyleValueType::CSSLCH:
      case CSSStyleValueType::CSSOKLab:
      case CSSStyleValueType::CSSOKLCH:
      case CSSStyleValueType::CSSRGB:
      {
        break;
      }
    }
    return false;
  }

  KRYS_NODISCARD constexpr bool IsCSSMathValue(CSSStyleValueType type) noexcept
  {
    switch (type)
    {
      case CSSStyleValueType::CSSMathClamp:
      case CSSStyleValueType::CSSMathInvert:
      case CSSStyleValueType::CSSMathMin:
      case CSSStyleValueType::CSSMathMax:
      case CSSStyleValueType::CSSMathNegate:
      case CSSStyleValueType::CSSMathProduct:
      case CSSStyleValueType::CSSMathSum:
      {
        return true;
      }
      case CSSStyleValueType::CSSUnitValue:
      case CSSStyleValueType::CSSStyleValue:
      case CSSStyleValueType::CSSImageValue:
      case CSSStyleValueType::CSSTransformValue:
      case CSSStyleValueType::CSSUnparsedValue:
      case CSSStyleValueType::CSSKeywordValue:
      case CSSStyleValueType::CSSColor:
      case CSSStyleValueType::CSSHSL:
      case CSSStyleValueType::CSSHWB:
      case CSSStyleValueType::CSSLab:
      case CSSStyleValueType::CSSLCH:
      case CSSStyleValueType::CSSOKLab:
      case CSSStyleValueType::CSSOKLCH:
      case CSSStyleValueType::CSSRGB:
      {
        break;
      }
    }
    return false;
  }

  KRYS_NODISCARD constexpr bool IsCSSColorValue(CSSStyleValueType type) noexcept
  {
    switch (type)
    {
      case CSSStyleValueType::CSSColor:
      case CSSStyleValueType::CSSHSL:
      case CSSStyleValueType::CSSHWB:
      case CSSStyleValueType::CSSLab:
      case CSSStyleValueType::CSSLCH:
      case CSSStyleValueType::CSSOKLab:
      case CSSStyleValueType::CSSOKLCH:
      case CSSStyleValueType::CSSRGB:
      {
        return true;
      }
      case CSSStyleValueType::CSSMathClamp:
      case CSSStyleValueType::CSSMathInvert:
      case CSSStyleValueType::CSSMathMin:
      case CSSStyleValueType::CSSMathMax:
      case CSSStyleValueType::CSSMathNegate:
      case CSSStyleValueType::CSSMathProduct:
      case CSSStyleValueType::CSSMathSum:
      case CSSStyleValueType::CSSUnitValue:
      case CSSStyleValueType::CSSStyleValue:
      case CSSStyleValueType::CSSImageValue:
      case CSSStyleValueType::CSSTransformValue:
      case CSSStyleValueType::CSSUnparsedValue:
      case CSSStyleValueType::CSSKeywordValue:
      {
        break;
      }
    }
    return false;
  }
}

namespace krys::boo
{
  /// @brief How a value nested in a math value is serialised.
  enum class SerialisationArguments : uint8
  {
    Nested,
    WithoutParentheses,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::SerialisationArguments, 2uz);

namespace krys::boo
{
  /// @brief A CSS value as the Typed OM represents it.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssstylevalue
  class CSSStyleValue : public RefCounted<CSSStyleValue>
  {
    KRYS_NON_COPY_MOVABLE(CSSStyleValue);

  protected:
    CSSOMString _customPropertyName;
    RefPtr<css::Value> _propertyValue;

  public:
    virtual ~CSSStyleValue() noexcept;

    KRYS_NODISCARD static Ref<CSSStyleValue> Create(RefPtr<css::Value> &&cssValue,
                                                    CSSOMString &&property = {}) noexcept;

    KRYS_NODISCARD static Ref<CSSStyleValue> Create() noexcept;

    /// @brief Parses @p cssText as a value of @p property, reified.
    /// @return A `TypeError` if @p property isn't a property or @p cssText doesn't parse as its value.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssstylevalue-parse
    KRYS_NODISCARD static ExceptionOr<Ref<CSSStyleValue>> Parse(Document &document, CSSOMStringView property,
                                                                CSSOMStringView cssText) noexcept;

    /// @brief Parses @p cssText as a value of @p property, each of its iterations reified.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssstylevalue-parseall
    KRYS_NODISCARD static ExceptionOr<List<Ref<CSSStyleValue>>>
      ParseAll(Document &document, CSSOMStringView property, CSSOMStringView cssText) noexcept;

    /// @see https://drafts.css-houdini.org/css-typed-om/#stylevalue-serialization
    KRYS_NODISCARD CSSOMString ToString() const noexcept;

    virtual void Serialise(StringBuilder &builder,
                           EnumSet<SerialisationArguments> arguments = {}) const noexcept;

    KRYS_NODISCARD virtual CSSStyleValueType StyleValueType() const noexcept
    {
      return CSSStyleValueType::CSSStyleValue;
    }

    KRYS_NODISCARD virtual RefPtr<css::Value> ToCSSValue() const noexcept;

    KRYS_NODISCARD virtual RefPtr<css::Value> ToCSSValueWithProperty(css::PropertyId) const noexcept
    {
      return ToCSSValue();
    }

  protected:
    explicit CSSStyleValue(RefPtr<css::Value> &&cssValue, CSSOMString &&property = {}) noexcept;

    CSSStyleValue() noexcept;
  };
}
