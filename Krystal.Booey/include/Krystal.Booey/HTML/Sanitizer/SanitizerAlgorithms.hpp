#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Dicts/SetHTMLOptions.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerConfig.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @brief What sanitizing does with an element.
  /// @see https://html.spec.whatwg.org/#sanitizer-action
  enum class SanitizerAction : uint8
  {
    Keep,
    Remove,
    ReplaceWithChildren,
  };

  /// @brief The algorithms on sanitizer configurations.
  /// @see https://html.spec.whatwg.org/#sanitization-algorithms
  class SanitizerAlgorithms
  {
  public:
    /// @brief The configuration a sanitizer configured with @p configuration has: a canonical copy, or a
    /// `TypeError` if it isn't valid.
    /// @see https://html.spec.whatwg.org/#sanitizer-configure
    KRYS_NODISCARD static dom::ExceptionOr<SanitizerConfig> Configure(const SanitizerConfig &configuration,
                                                                      bool permissiveDefaults) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizer-canonicalize-the-configuration
    static void Canonicalize(SanitizerConfig &configuration, bool permissiveDefaults) noexcept;

    /// @see https://html.spec.whatwg.org/#canonicalize-a-sanitizerelementwithattributes
    KRYS_NODISCARD static SanitizerElementNamespaceWithAttributes
      Canonicalize(SanitizerElementNamespaceWithAttributes element) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizer-canonicalize-a-sanitizer-element
    KRYS_NODISCARD static SanitizerElementNamespace Canonicalize(SanitizerElementNamespace element) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizer-canonicalize-a-sanitizer-name
    KRYS_NODISCARD static SanitizerAttributeNamespace
      Canonicalize(SanitizerAttributeNamespace attribute) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizerconfig-valid
    KRYS_NODISCARD static bool IsValid(const SanitizerConfig &config) noexcept;

    /// @return Whether @p configuration changed.
    /// @see https://html.spec.whatwg.org/#sanitizer-remove-an-element
    static bool RemoveElement(SanitizerConfig &configuration, SanitizerElementNamespace element) noexcept;

    /// @return Whether @p configuration changed.
    /// @see https://html.spec.whatwg.org/#sanitizer-remove-an-attribute
    static bool RemoveAttribute(SanitizerConfig &configuration,
                                SanitizerAttributeNamespace attribute) noexcept;

    /// @return Whether @p configuration changed.
    /// @see https://html.spec.whatwg.org/#sanitizer-remove-unsafe
    static bool RemoveUnsafe(SanitizerConfig &configuration) noexcept;

    /// @brief The configuration markup is sanitized with, given the options' sanitizer member (null if it's
    /// missing); null for no sanitizing, or a `TypeError` if the configuration isn't valid.
    /// @see https://html.spec.whatwg.org/#get-a-sanitizer-config-from-options
    KRYS_NODISCARD static dom::ExceptionOr<Maybe<SanitizerConfig>>
      GetSanitizerConfigFromOptions(const SanitizerInput *sanitizer, bool safe) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizer-contains-a-javascript-url
    KRYS_NODISCARD static bool ContainsJavaScriptURL(dom::DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizer-config-allows-comments
    KRYS_NODISCARD static bool AllowsComments(const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizer-config-allows-processing-instruction-target
    KRYS_NODISCARD static bool
      AllowsProcessingInstructionTarget(dom::DOMStringView piTarget,
                                        const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/#get-the-sanitizer-action-for-an-element-name
    KRYS_NODISCARD static SanitizerAction ActionForElementName(dom::DOMStringView localName,
                                                               dom::DOMStringAtom namespaceURI,
                                                               const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/#sanitizer-config-allows-an-attribute
    KRYS_NODISCARD static bool AllowsAttribute(dom::DOMStringView localName, dom::DOMStringAtom namespaceURI,
                                               dom::DOMStringView value, dom::DOMStringView elementLocalName,
                                               dom::DOMStringAtom elementNamespace,
                                               const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/#custom-data-attribute
    KRYS_NODISCARD static bool IsCustomDataAttribute(const SanitizerAttributeNamespace &attribute) noexcept;

    /// @brief Whether @p a sorts before @p b: by namespace (null first), then by name.
    /// @see https://html.spec.whatwg.org/#sanitizer-compare-sanitizer-items
    KRYS_NODISCARD static bool CompareItems(dom::DOMStringView nameA, dom::DOMStringAtom namespaceA,
                                            dom::DOMStringView nameB, dom::DOMStringAtom namespaceB) noexcept;
  };
}
