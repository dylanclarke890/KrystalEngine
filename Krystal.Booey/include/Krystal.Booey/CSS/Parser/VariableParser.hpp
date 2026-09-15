#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  class CustomProperty;
}

namespace krys::boo::css
{
  class Property;
  class Token;
  class TokenRange;
  class Value;

  struct PropertyParserState;
  struct ComputedStyleDependencies;
  struct CustomPropertySyntax;
  struct ParserContext;

  class VariableParser
  {
  public:
    KRYS_NODISCARD static bool ContainsValidVariableReferences(TokenRange tokens,
                                                               const ParserContext &context) noexcept;

    KRYS_NODISCARD static RefPtr<Value> ParseDeclarationValue(const CSSOMStringAtom &atom, TokenRange tokens,
                                                              const ParserContext &context) noexcept;

    KRYS_NODISCARD static RefPtr<const style::CustomProperty>
      ParseInitialValueForUniversalSyntax(const CSSOMStringAtom &atom, TokenRange tokens) noexcept;

    KRYS_NODISCARD static bool IsValidVariableName(const Token &token) noexcept;
  };
}