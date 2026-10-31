#pragma once

#include "Krystal.Booey/HTML/Sanitizer/SanitizerConfig.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#sanitization-constants
  class SanitizerConstants
  {
  public:
    /// @brief The configuration of `setHTML()` and `parseHTML()` when none is given, and of the `"default"`
    /// preset.
    /// @see https://html.spec.whatwg.org/#built-in-safe-default-configuration
    KRYS_NODISCARD static const SanitizerConfig &BuiltInSafeDefaultConfiguration() noexcept;

    /// @brief The elements and attributes that removing unsafe content removes (along with every event
    /// handler content attribute).
    /// @see https://html.spec.whatwg.org/#built-in-safe-baseline-configuration
    KRYS_NODISCARD static const SanitizerConfig &BuiltInSafeBaselineConfiguration() noexcept;

    /// @see https://html.spec.whatwg.org/#built-in-non-replaceable-elements-list
    KRYS_NODISCARD static Span<const SanitizerElementNamespace> BuiltInNonReplaceableElements() noexcept;

    /// @see https://html.spec.whatwg.org/#built-in-navigating-url-attributes-list
    KRYS_NODISCARD static bool
      IsNavigatingURLAttribute(const SanitizerElementNamespace &element,
                               const SanitizerAttributeNamespace &attribute) noexcept;

    /// @see https://html.spec.whatwg.org/#built-in-animating-url-attributes-list
    KRYS_NODISCARD static bool IsAnimatingURLAttribute(const SanitizerElementNamespace &element,
                                                       const SanitizerAttributeNamespace &attribute) noexcept;

    /// @brief The names of the event handler content attributes, from every specification that defines one.
    /// @see https://html.spec.whatwg.org/#event-handler-content-attributes
    KRYS_NODISCARD static Span<const utf8_stringview> EventHandlerContentAttributes() noexcept;
  };
}
