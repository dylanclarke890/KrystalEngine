/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Motorola Mobility Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Motorola Mobility Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

#include "Krystal.Booey/Css/CSSOM/DOMCSSNamespace.hpp"
#include "Krystal.Booey/Css/CSSOM/Markup.hpp"
#include "Krystal.Booey/Css/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/Css/Parser/InputStream.hpp"
#include "Krystal.Booey/Css/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/Css/Parser/Parser.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Parser/SupportsParser.hpp"
#include "Krystal.Booey/Css/Parser/Tokenizer.hpp"
#include "Krystal.Booey/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/Css/Properties/Property.hpp"
#include "Krystal.Booey/Css/Properties/PropertyQueries.hpp"
#include "Krystal.Booey/Css/Properties/RegisteredCustomProperty.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"

namespace krys::boo
{
  CSSOMString DOMCSSNamespace::Escape(const CSSOMString &ident) noexcept
  {
    StringBuilder builder;
    css::SerialiseIdentifier(ident, builder);
    return builder.ToString();
  }

  bool DOMCSSNamespace::Supports(const Document &document, CSSOMStringView property,
                                 CSSOMStringView value) noexcept
  {
    // 1. If property is an ASCII case-insensitive match for any defined CSS property that the UA supports, or
    //    is a custom property name string, and value successfully parses according to that property's
    //    grammar, return true.
    // 2. Otherwise, return false.
    // (In standards mode, as the specification has no quirks for it.)
    css::ParserContext context(document);
    context.Mode = css::ParserMode::HTMLStandard;

    auto propertyId = css::PropertyIdFromName(property);
    if (propertyId == css::PropertyId::Invalid && css::IsCustomPropertyName(property))
    {
      auto declaration = css::MutableStyleProperties::Create();
      return css::Parser::ParseCustomPropertyValue(*declaration, CSSOMStringAtom(property), value,
                                                   css::IsImportant::No, context)
             != css::Parser::ParseResult::Error;
    }

    if (propertyId == css::PropertyId::Invalid || !IsExposed(propertyId, context.PropertySettings)
        || css::IsDescriptorOnly(propertyId) || value.empty())
    {
      return false;
    }

    auto declaration = css::MutableStyleProperties::Create();
    return css::Parser::ParseValue(*declaration, propertyId, value, css::IsImportant::No, context)
           != css::Parser::ParseResult::Error;
  }

  bool DOMCSSNamespace::Supports(const Document &document, CSSOMStringView conditionText) noexcept
  {
    // 1. If conditionText, parsed and evaluated as a <supports-condition>, would return true, return true.
    // 2. Otherwise, If conditionText, wrapped in parentheses and then parsed and evaluated as a
    //    <supports-condition>, would return true, return true.
    // 3. Otherwise, return false.
    css::ParserContext context(document);
    context.Mode = css::ParserMode::HTMLStandard;
    return css::SupportsParser::SupportsCondition(
             conditionText, context, css::SupportsParser::ParsingMode::AllowBareDeclarationAndGeneralEnclosed)
           == css::SupportsParser::Supported;
  }

  ExceptionOr<void> DOMCSSNamespace::RegisterProperty(Document &document,
                                                      const PropertyDefinition &definition) noexcept
  {
    // 1. Let property set be the value of this's relevant global object's associated Document's
    //    [[registeredPropertySet]] slot.
    // (The document's style scope's registry.)
    auto &registry = document.StyleScope().GetCustomPropertyRegistry();

    // 2. If name is not a custom property name string, throw a SyntaxError and exit this algorithm.
    if (!css::IsCustomPropertyName(definition.Name))
    {
      return Exception {ExceptionCode::SyntaxError,
                        u8"The name of this property is not a custom property name."};
    }
    // If property set already contains an entry with name as its property name (compared codepoint-wise),
    // throw an InvalidModificationError and exit this algorithm.
    // (Checked as it's registered, in step 6.)

    // 3. Attempt to consume a syntax definition from syntax. If it returns failure, throw a SyntaxError.
    //    Otherwise, let syntax definition be the returned syntax definition.
    auto syntax = css::CustomPropertySyntax::Parse(definition.Syntax);
    if (!syntax)
    {
      return Exception {ExceptionCode::SyntaxError, u8"Invalid property syntax definition."};
    }

    auto name = CSSOMStringAtom(definition.Name);
    RefPtr<const style::CustomProperty> initialValue;
    RefPtr<const css::VariableData> initialValueTokensForViewportUnits;

    // 4. If initialValue is not present, let parsed initial value be the guaranteed-invalid value.
    // (Only for the universal syntax, as in Chrome and WebKit, and as `@property` requires; see
    // DIFFERENCES.md.)
    if (!definition.InitialValue)
    {
      if (!syntax->IsUniversal())
      {
        return Exception {ExceptionCode::SyntaxError,
                          u8"An initial value is mandatory except for the '*' syntax."};
      }
      initialValue = style::CustomProperty::CreateForGuaranteedInvalid(name);
    }
    // Otherwise:
    else
    {
      // - parse initialValue according to <declaration-value>? if syntax definition is the universal syntax
      //   definition, and according to syntax definition otherwise.
      // - If parsing returns failure, throw a SyntaxError and exit this algorithm. Otherwise, let parsed
      //   initial value be the parsed result.
      // - If parsed initial value is not computationally independent, throw a SyntaxError and exit this
      //   algorithm.
      auto inputStream = css::InputStream(CSSOMString(*definition.InitialValue));
      auto tokenizer = css::Tokenizer(inputStream);
      std::ignore = tokenizer.PumpTokenizer();

      auto parsedInitialValue =
        style::CustomPropertyRegistry::ParseInitialValue(document, name, *syntax, tokenizer.Tokens());
      if (!parsedInitialValue)
      {
        if (parsedInitialValue.error()
            == style::CustomPropertyRegistry::ParseInitialValueError::NotComputationallyIndependent)
        {
          return Exception {ExceptionCode::SyntaxError,
                            u8"The given initial value must be computationally independent."};
        }
        return Exception {ExceptionCode::SyntaxError,
                          u8"The given initial value does not parse for the given syntax."};
      }

      initialValue = parsedInitialValue->first;
      if (parsedInitialValue->second == style::CustomPropertyRegistry::ViewportUnitDependency::Yes)
      {
        initialValueTokensForViewportUnits = css::VariableData::Create(tokenizer.Tokens());
        document.SetHasStyleWithViewportUnits();
      }
    }

    // 5. Set inherit flag to the value of inherits.
    // 6. Let registered property be a struct with a property name of name, a syntax of syntax definition, an
    //    initial value of parsed initial value, and an inherit flag of inherit flag. Append registered
    //    property to property set.
    auto property = css::RegisteredCustomProperty {
      .Name = name,
      .Syntax = krys::move(*syntax),
      .Inherits = definition.Inherits,
      .InitialValue = krys::move(initialValue),
      .InitialValueTokensForViewportUnits = krys::move(initialValueTokensForViewportUnits),
    };
    if (!registry.RegisterFromAPI(krys::move(property)))
    {
      return Exception {ExceptionCode::InvalidModificationError,
                        u8"This property has already been registered."};
    }
    return {};
  }
}
