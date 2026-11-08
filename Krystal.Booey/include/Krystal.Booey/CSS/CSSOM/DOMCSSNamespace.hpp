#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  /// @brief A custom property to register with `CSS.registerProperty()`.
  /// @see https://drafts.css-houdini.org/css-properties-values-api/#dictdef-propertydefinition
  struct PropertyDefinition
  {
    CSSOMString Name;
    CSSOMString Syntax {u8"*"};
    bool Inherits {false};
    Maybe<CSSOMString> InitialValue;
  };

  /// @brief The `CSS` namespace. Members that depend on a global take the document standing in for it.
  /// @see https://drafts.csswg.org/cssom/#namespacedef-css
  struct DOMCSSNamespace
  {
    /// @brief @p ident escaped for use as a CSS identifier.
    /// @see https://drafts.csswg.org/cssom/#dom-css-escape
    KRYS_NODISCARD static CSSOMString Escape(const CSSOMString &ident) noexcept;

    /// @brief Whether @p property is a supported property (or a custom property) and @p value parses for it.
    /// @see https://drafts.csswg.org/css-conditional-3/#dom-css-supports
    KRYS_NODISCARD static bool Supports(const dom::Document &document, CSSOMStringView property,
                                        CSSOMStringView value) noexcept;

    /// @brief Whether @p conditionText, a `<supports-condition>` (or, failing that, a declaration as if it
    /// were in parentheses), is supported.
    /// @see https://drafts.csswg.org/css-conditional-3/#dom-css-supports-conditiontext
    KRYS_NODISCARD static bool Supports(const dom::Document &document,
                                        CSSOMStringView conditionText) noexcept;

    /// @brief Registers the custom property @p definition describes in @p document.
    /// @return A `SyntaxError` if the name, syntax or initial value is invalid, and an
    /// `InvalidModificationError` if the property is already registered.
    /// @see https://drafts.css-houdini.org/css-properties-values-api/#dom-css-registerproperty
    static dom::ExceptionOr<void> RegisterProperty(dom::Document &document,
                                                   const PropertyDefinition &definition) noexcept;
  };
}
