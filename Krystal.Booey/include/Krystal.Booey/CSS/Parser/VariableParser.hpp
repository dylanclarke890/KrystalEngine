#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  class CustomProperty;
}

namespace krys::boo::css
{
  class CustomPropertyValue;
  class Token;

  struct ParserContext;

  /// @brief Validates and parses custom property values and values containing `var()`/`env()`.
  class VariableParser
  {
  public:
    KRYS_NODISCARD static bool ContainsValidVariableReferences(TokenRange tokens,
                                                               const ParserContext &context) noexcept;

    KRYS_NODISCARD static RefPtr<CustomPropertyValue>
      ParseDeclarationValue(const CSSOMStringAtom &variableName, TokenRange tokens,
                            const ParserContext &context) noexcept;

    // FIXME: Not defined until style::CustomProperty is ported.
    KRYS_NODISCARD static RefPtr<const style::CustomProperty>
      ParseInitialValueForUniversalSyntax(const CSSOMStringAtom &variableName, TokenRange tokens) noexcept;

    KRYS_NODISCARD static bool IsValidVariableName(const Token &token) noexcept;
  };
}
