#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerConfig.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @brief A sanitizer configuration that can be modified and given to the HTML insertion methods.
  /// @see https://html.spec.whatwg.org/#sanitizer
  class Sanitizer final : public RefCounted<Sanitizer>
  {
    friend class SanitizerAlgorithms;

  private:
    /// @see https://html.spec.whatwg.org/#sanitizer-configuration
    SanitizerConfig _configuration;

    Sanitizer() noexcept = default;

  public:
    /// @brief The sanitizer with the built-in safe default configuration.
    /// @see https://html.spec.whatwg.org/#dom-sanitizer-constructor
    KRYS_NODISCARD static Ref<Sanitizer>
      Create(SanitizerPresets configuration = SanitizerPresets::Default) noexcept;

    /// @brief A sanitizer with @p configuration; a `TypeError` if it isn't valid.
    /// @see https://html.spec.whatwg.org/#dom-sanitizer-constructor
    KRYS_NODISCARD static dom::ExceptionOr<Ref<Sanitizer>>
      Create(const SanitizerConfig &configuration) noexcept;

    /// @brief A copy of the configuration, with its lists sorted.
    /// @see https://html.spec.whatwg.org/#dom-sanitizer-get
    KRYS_NODISCARD SanitizerConfig Get() const noexcept;

    /// @brief The configuration itself, which is valid.
    KRYS_NODISCARD const SanitizerConfig &Configuration() const noexcept
    {
      return _configuration;
    }

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-allowelement
    bool AllowElement(SanitizerElementNamespaceWithAttributes element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-removeelement
    bool RemoveElement(SanitizerElementNamespace element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-replaceelementwithchildren
    bool ReplaceElementWithChildren(SanitizerElementNamespace element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-allowprocessinginstruction
    bool AllowProcessingInstruction(SanitizerProcessingInstruction pi) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-removeprocessinginstruction
    bool RemoveProcessingInstruction(SanitizerProcessingInstruction pi) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-allowattribute
    bool AllowAttribute(SanitizerAttributeNamespace attribute) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-removeattribute
    bool RemoveAttribute(SanitizerAttributeNamespace attribute) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-setcomments
    bool SetComments(bool allow) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-setdataattributes
    bool SetDataAttributes(bool allow) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-setjavascripturls
    bool SetJavascriptURLs(bool allow) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-sanitizer-removeunsafe
    bool RemoveUnsafe() noexcept;
  };
}
