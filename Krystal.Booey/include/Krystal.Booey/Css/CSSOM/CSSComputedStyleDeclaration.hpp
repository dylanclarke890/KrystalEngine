#pragma once

#include "Krystal.Booey/Css/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/Style/Extractor/Extractor.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Containers/FixedList.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class MutableStyleProperties;
}

namespace krys::boo
{
  class Element;

  /// @brief What `getComputedStyle()` returns: a live, read-only declaration block of the resolved values of
  /// an element or one of its pseudo-elements. Its declarations are every supported longhand, in
  /// lexicographical order, then the custom properties, while the element is connected; otherwise none.
  /// @see https://drafts.csswg.org/cssom/#dom-window-getcomputedstyle
  class CSSComputedStyleDeclaration final : public CSSStyleProperties,
                                            public RefCounted<CSSComputedStyleDeclaration>
  {
  private:
    const Ref<Element> _element;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;
    // Whether there's no element or pseudo-element to resolve values for, which leaves no declarations.
    bool _hasNoTarget {false};

    CSSComputedStyleDeclaration(Element &element, const Maybe<style::PseudoElementIdentifier> &pseudoElement,
                                bool hasNoTarget) noexcept;

  public:
    ~CSSComputedStyleDeclaration() noexcept override;

    /// @brief The declarations of @p element, or of its pseudo-element @p pseudoElement.
    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration>
      Create(Element &element, const Maybe<style::PseudoElementIdentifier> &pseudoElement) noexcept;

    /// @brief Declarations without any, for a pseudo-element that isn't one.
    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration> CreateEmpty(Element &element) noexcept;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }

    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    /// @brief The serialised resolved value of @p property, whether or not the element is connected.
    KRYS_NODISCARD CSSOMString GetPropertyValue(css::PropertyId property) const noexcept;

  private:
    // The CSSOM's members, reached through CSSStyleDeclaration.
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    ExceptionOr<void> SetCssText(const CSSOMString &text) noexcept final;

    KRYS_NODISCARD uint32 Length() const noexcept final;

    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept final;

    ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                  const CSSOMString &priority) noexcept final;

    ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(css::PropertyId property) noexcept final;

    ExceptionOr<void> SetPropertyInternal(css::PropertyId property, const CSSOMString &value,
                                          css::IsImportant important) noexcept final;

    KRYS_NODISCARD Ref<css::MutableStyleProperties> CopyProperties() const noexcept final;

    /// @brief Whether there are declarations: whether there's an element or pseudo-element, and the element
    /// is connected.
    KRYS_NODISCARD bool HasDeclarations() const noexcept;

    KRYS_NODISCARD const FixedList<css::PropertyId> &ExposedLonghands() const noexcept;

    KRYS_NODISCARD style::Extractor GetExtractor() const noexcept;
  };
}
