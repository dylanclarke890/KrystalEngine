#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Css/Properties/ParsedPropertyList.hpp"
#include "Krystal.Web/Css/Properties/PropertyId.hpp"
#include "Krystal.Web/Css/Properties/Types/IsImportant.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"

namespace krys::web::style
{
  class BuilderState;
  class CustomProperty;
}

namespace krys::web::css
{
  class TokenRange;
  class Value;

  struct ComputedStyleDependencies;
  struct CustomPropertySyntax;
  struct ParserContext;

  enum class GlobalKeyword : uint8;
  enum class RuleType : uint8;

  /// @brief Parses the values of declarations: properties, descriptors and registered custom properties.
  struct PropertyParser
  {
    /// @brief Parses @p tokens as the value of property or descriptor @p id in a @p ruleType rule, appending
    /// the declarations it sets (a shorthand's longhands, for one) to @p result.
    /// @return Whether it parsed; if not, @p result is as it was.
    KRYS_NODISCARD static bool ParseValue(PropertyId id, IsImportant isImportant, TokenRange tokens,
                                          const ParserContext &context, ParsedPropertyList &result,
                                          RuleType ruleType) noexcept;

    /// @brief Parses @p str as the value of longhand style property @p id, or a CSS-wide keyword.
    /// @return The value, or null if @p str isn't one.
    KRYS_NODISCARD static RefPtr<Value> ParseStylePropertyLonghand(PropertyId id, const CSSOMString &str,
                                                                   const ParserContext &context) noexcept;

    /// @brief Parses @p tokens as the value of longhand style property @p id, or a CSS-wide keyword.
    /// @return The value, or null if @p tokens aren't one.
    KRYS_NODISCARD static RefPtr<Value> ParseStylePropertyLonghand(PropertyId id, TokenRange tokens,
                                                                   const ParserContext &context) noexcept;

    /// @brief Parses @p str as the value of `@counter-style` descriptor @p id.
    /// @return The value, or null if @p str isn't one.
    KRYS_NODISCARD static RefPtr<Value> ParseCounterStyleDescriptor(PropertyId id, const CSSOMString &str,
                                                                    const ParserContext &context) noexcept;

    /// @brief Parses @p tokens as the `initial-value` of registered custom property @p name.
    /// @return The computed initial value, or null if @p tokens don't match the syntax or are a CSS-wide
    /// keyword.
    /// @see https://drafts.css-houdini.org/css-properties-values-api-1/#initial-value-descriptor
    KRYS_NODISCARD static RefPtr<const style::CustomProperty> ParseTypedCustomPropertyInitialValue(
      const CSSOMStringAtom &name, const CustomPropertySyntax &customPropertySyntax, TokenRange tokens,
      style::BuilderState &state, const ParserContext &context) noexcept;

    /// @brief Parses @p tokens as a value of registered custom property @p name, at computed-value time.
    /// @return The computed value or a CSS-wide keyword, or null if @p tokens match neither.
    /// @see https://drafts.css-houdini.org/css-properties-values-api-1/#calculation-of-computed-values
    KRYS_NODISCARD static Maybe<Variant<Ref<const style::CustomProperty>, GlobalKeyword>>
      ParseTypedCustomPropertyValue(const CSSOMStringAtom &name,
                                    const CustomPropertySyntax &customPropertySyntax, TokenRange tokens,
                                    style::BuilderState &state, const ParserContext &context) noexcept;

    /// @brief What computing @p tokens, parsed with @p customPropertySyntax, depends on, such as `em` on the
    /// font size; nothing if they don't match it.
    KRYS_NODISCARD static ComputedStyleDependencies
      CollectParsedCustomPropertyValueDependencies(const CustomPropertySyntax &customPropertySyntax,
                                                   TokenRange tokens, const ParserContext &context) noexcept;

    /// @brief Whether @p tokens match @p customPropertySyntax.
    KRYS_NODISCARD static bool
      IsValidCustomPropertyValueForSyntax(const CustomPropertySyntax &customPropertySyntax, TokenRange tokens,
                                          const ParserContext &context) noexcept;

    /// @brief The CSS-wide keyword @p tokens consist of, if they do.
    /// @see https://drafts.csswg.org/css-values-4/#common-keywords
    KRYS_NODISCARD static Maybe<GlobalKeyword> ParseGlobalKeyword(TokenRange tokens) noexcept;
  };
}
