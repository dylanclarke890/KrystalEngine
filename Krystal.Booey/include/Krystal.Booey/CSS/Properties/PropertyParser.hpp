/*
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004 - 2021 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 - 2010  Torch Mobile (Beijing) Co. Ltd. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::style
{
  class BuilderState;
  class CustomProperty;
}

namespace krys::boo::css
{
  class Property;
  class TokenRange;
  class Value;

  struct PropertyParserState;
  struct ComputedStyleDependencies;
  struct CustomPropertySyntax;
  struct ParserContext;

  enum class GlobalKeyword : uint8;
  enum class RuleType : uint8;

  class PropertyParser
  {
  public:
    // Parses any CSS property or descriptor. If successful, the result will be appended to the `result`
    // SmallList and the function will return true, otherwise, the function will return false and the `result`
    // SmallList will be unmodified.
    KRYS_NODISCARD static bool ParseValue(PropertyId id, IsImportant isImportant, TokenRange tokens,
                                          const ParserContext &context, ParsedPropertyList &result,
                                          RuleType ruleType) noexcept;

    // Parses a longhand style property.
    KRYS_NODISCARD static RefPtr<Value> ParseStylePropertyLonghand(PropertyId id, const CSSOMString &str,
                                                                   const ParserContext &context) noexcept;

    KRYS_NODISCARD static RefPtr<Value> ParseStylePropertyLonghand(PropertyId id, TokenRange tokens,
                                                                   const ParserContext &context) noexcept;

    // Parses a @counter-style descriptor.
    KRYS_NODISCARD static RefPtr<Value> ParseCounterStyleDescriptor(PropertyId id, const CSSOMString &str,
                                                                    const ParserContext &context) noexcept;

    KRYS_NODISCARD static RefPtr<const style::CustomProperty> ParseTypedCustomPropertyInitialValue(
      const CSSOMStringAtom &str, const CustomPropertySyntax &customPropertySyntax, TokenRange tokens,
      style::BuilderState &state, const ParserContext &context) noexcept;

    KRYS_NODISCARD static Maybe<Variant<Ref<const style::CustomProperty>, GlobalKeyword>>
      ParseTypedCustomPropertyValue(const CSSOMStringAtom &name,
                                    const CustomPropertySyntax &customPropertySyntax, TokenRange tokens,
                                    style::BuilderState &state, const ParserContext &context) noexcept;

    KRYS_NODISCARD static ComputedStyleDependencies
      CollectParsedCustomPropertyValueDependencies(const CustomPropertySyntax &customPropertySyntax,
                                                   TokenRange tokens, const ParserContext &context) noexcept;

    KRYS_NODISCARD static bool
      IsValidCustomPropertyValueForSyntax(const CustomPropertySyntax &customPropertySyntax, TokenRange tokens,
                                          const ParserContext &context) noexcept;

    KRYS_NODISCARD static Maybe<GlobalKeyword> ParseGlobalKeyword(TokenRange tokens) noexcept;
  };
}
