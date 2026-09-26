#pragma once

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

  KRYS_NODISCARD bool IsCustomPropertyName(CSSOMStringView id) noexcept;
}
