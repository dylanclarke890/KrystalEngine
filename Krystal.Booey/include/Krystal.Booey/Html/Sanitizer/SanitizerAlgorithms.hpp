#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Html/Sanitizer/SanitizerConfig.hpp"
#include "Krystal.Booey/Html/Sanitizer/SetHTMLOptions.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @brief What sanitizing does with an element.
  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitizer-action
  enum class SanitizerAction : uint8
  {
    Keep,
    Remove,
    ReplaceWithChildren,
  };

  /// @brief The algorithms on sanitizer configurations.
  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitization-algorithms
  class SanitizerAlgorithms
  {
  public:
    /// @brief The configuration a sanitizer configured with @p configuration has: a canonical copy, or a
    /// `TypeError` if it isn't valid.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#configure-a-sanitizer
    KRYS_NODISCARD static ExceptionOr<SanitizerConfig> Configure(const SanitizerConfig &configuration,
                                                                 bool permissiveDefaults) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#canonicalize-the-configuration
    static void Canonicalize(SanitizerConfig &configuration, bool permissiveDefaults) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#canonicalize-a-sanitizer-element-with-attributes
    KRYS_NODISCARD static SanitizerElementNamespaceWithAttributes
      Canonicalize(SanitizerElementNamespaceWithAttributes element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#canonicalize-a-sanitizer-element
    KRYS_NODISCARD static SanitizerElementNamespace Canonicalize(SanitizerElementNamespace element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#canonicalize-a-sanitizer-name
    KRYS_NODISCARD static SanitizerAttributeNamespace
      Canonicalize(SanitizerAttributeNamespace attribute) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizerconfig-valid
    KRYS_NODISCARD static bool IsValid(const SanitizerConfig &config) noexcept;

    /// @return Whether @p configuration changed.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#remove-an-element-from-sanitizer-config
    static bool RemoveElement(SanitizerConfig &configuration, SanitizerElementNamespace element) noexcept;

    /// @return Whether @p configuration changed.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#remove-an-attribute-from-sanitizer-config
    static bool RemoveAttribute(SanitizerConfig &configuration,
                                SanitizerAttributeNamespace attribute) noexcept;

    /// @return Whether @p configuration changed.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#remove-unsafe
    static bool RemoveUnsafe(SanitizerConfig &configuration) noexcept;

    /// @brief The configuration markup is sanitized with, given the options' sanitizer member (null if it's
    /// missing); null for no sanitizing, or a `TypeError` if the configuration isn't valid.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#get-a-sanitizer-config-from-options
    KRYS_NODISCARD static ExceptionOr<Maybe<SanitizerConfig>>
      GetSanitizerConfigFromOptions(const SanitizerInput *sanitizer, bool safe) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#contains-a-javascript:-url
    KRYS_NODISCARD static bool ContainsJavaScriptUrl(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitizer-config-allows-comments
    KRYS_NODISCARD static bool AllowsComments(const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitizer-config-allows-processing-instruction-target
    KRYS_NODISCARD static bool
      AllowsProcessingInstructionTarget(DOMStringView piTarget,
                                        const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#get-the-sanitizer-action-for-an-element-name
    KRYS_NODISCARD static SanitizerAction ActionForElementName(DOMStringView localName,
                                                               DOMStringAtom namespaceURI,
                                                               const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitizer-config-allows-an-attribute
    KRYS_NODISCARD static bool AllowsAttribute(DOMStringView localName, const DOMStringAtom &namespaceURI,
                                               DOMStringView value, DOMStringView elementLocalName,
                                               const DOMStringAtom &elementNamespace,
                                               const SanitizerConfig &configuration) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#custom-data-attribute
    KRYS_NODISCARD static bool IsCustomDataAttribute(const SanitizerAttributeNamespace &attribute) noexcept;

    /// @brief Whether @p a sorts before @p b: by namespace (null first), then by name.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#compare-sanitizer-items
    KRYS_NODISCARD static bool CompareItems(DOMStringView nameA, const DOMStringAtom &namespaceA,
                                            DOMStringView nameB, const DOMStringAtom &namespaceB) noexcept;
  };
}
