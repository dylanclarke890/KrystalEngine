#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Web/Html/Sanitizer/SanitizerConfig.hpp"

namespace krys::web::html
{
  /// @brief A sanitizer configuration that can be modified and given to the HTML insertion methods.
  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitizer
  class Sanitizer final : public RefCounted<Sanitizer>
  {
    friend class SanitizerAlgorithms;

  private:
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sanitizer-configuration
    SanitizerConfig _configuration;

    Sanitizer() noexcept = default;

  public:
    /// @brief The sanitizer with the built-in safe default configuration.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-constructor
    KRYS_NODISCARD static Ref<Sanitizer>
      Create(SanitizerPresets configuration = SanitizerPresets::Default) noexcept;

    /// @brief A sanitizer with @p configuration; a `TypeError` if it isn't valid.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-constructor
    KRYS_NODISCARD static ExceptionOr<Ref<Sanitizer>> Create(const SanitizerConfig &configuration) noexcept;

    /// @brief A copy of the configuration, with its lists sorted.
    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-get
    KRYS_NODISCARD SanitizerConfig Get() const noexcept;

    /// @brief The configuration itself, which is valid.
    KRYS_NODISCARD const SanitizerConfig &Configuration() const noexcept
    {
      return _configuration;
    }

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-allowelement
    bool AllowElement(SanitizerElementNamespaceWithAttributes element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-removeelement
    bool RemoveElement(SanitizerElementNamespace element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-replaceelementwithchildren
    bool ReplaceElementWithChildren(SanitizerElementNamespace element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-allowprocessinginstruction
    bool AllowProcessingInstruction(SanitizerProcessingInstruction pi) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-removeprocessinginstruction
    bool RemoveProcessingInstruction(SanitizerProcessingInstruction pi) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-allowattribute
    bool AllowAttribute(SanitizerAttributeNamespace attribute) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-removeattribute
    bool RemoveAttribute(SanitizerAttributeNamespace attribute) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-setcomments
    bool SetComments(bool allow) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-setdataattributes
    bool SetDataAttributes(bool allow) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-setjavascripturls
    bool SetJavascriptUrls(bool allow) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-sanitizer-removeunsafe
    bool RemoveUnsafe() noexcept;
  };
}
