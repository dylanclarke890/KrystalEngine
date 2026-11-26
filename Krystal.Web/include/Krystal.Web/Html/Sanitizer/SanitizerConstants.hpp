#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Web/Html/Sanitizer/SanitizerConfig.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitization-constants
  class SanitizerConstants
  {
  public:
    /// @brief The configuration of `setHTML()` and `parseHTML()` when none is given, and of the `"default"`
    /// preset.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#built-in-safe-default-configuration
    KRYS_NODISCARD static const SanitizerConfig &BuiltInSafeDefaultConfiguration() noexcept;

    /// @brief The elements and attributes that removing unsafe content removes (along with every event
    /// handler content attribute).
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#built-in-safe-baseline-configuration
    KRYS_NODISCARD static const SanitizerConfig &BuiltInSafeBaselineConfiguration() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#built-in-non-replaceable-elements-list
    KRYS_NODISCARD static Span<const SanitizerElementNamespace> BuiltInNonReplaceableElements() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#built-in-navigating-url-attributes-list
    KRYS_NODISCARD static bool
      IsNavigatingUrlAttribute(const SanitizerElementNamespace &element,
                               const SanitizerAttributeNamespace &attribute) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#built-in-animating-url-attributes-list
    KRYS_NODISCARD static bool IsAnimatingUrlAttribute(const SanitizerElementNamespace &element,
                                                       const SanitizerAttributeNamespace &attribute) noexcept;

    /// @brief The names of the event handler content attributes, from every specification that defines one.
    /// @see https://html.spec.whatwg.org/multipage/webappapis.html#event-handler-content-attributes
    KRYS_NODISCARD static Span<const utf8_stringview> EventHandlerContentAttributes() noexcept;
  };
}
