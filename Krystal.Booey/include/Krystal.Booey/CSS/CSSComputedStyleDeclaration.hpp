#pragma once

#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  class MutableStyleProperties;

  /// @brief The read-only declaration block of an element's resolved values, as returned by
  /// `getComputedStyle()`.
  class CSSComputedStyleDeclaration final : public CSSStyleProperties,
                                            public RefCounted<CSSComputedStyleDeclaration>
  {
  private:
    enum class IsEmpty : bool
    {
      No,
      Yes
    };

    const Ref<dom::Element> _element;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier {null};
    bool _isEmpty {false};
    bool _allowVisitedStyle {false};

  public:
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

    enum class AllowVisited : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration> Create(dom::Element &, AllowVisited) noexcept;
    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration>
      Create(dom::Element &, const Maybe<style::PseudoElementIdentifier> &) noexcept;
    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration> CreateEmpty(dom::Element &) noexcept;

    ~CSSComputedStyleDeclaration() noexcept override;

    KRYS_NODISCARD CSSOMString GetPropertyValue(PropertyId) const noexcept;

  private:
    CSSComputedStyleDeclaration(dom::Element &, AllowVisited) noexcept;
    CSSComputedStyleDeclaration(dom::Element &, IsEmpty) noexcept;
    CSSComputedStyleDeclaration(dom::Element &, const Maybe<style::PseudoElementIdentifier> &) noexcept;

    // CSSOM functions. Don't make these public.
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;
    KRYS_NODISCARD CSSRule *CssRules() const noexcept final;
    KRYS_NODISCARD uint32 Length() const noexcept final;
    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyShorthand(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD bool IsPropertyImplicit(const CSSOMString &propertyName) noexcept final;
    dom::ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                       const CSSOMString &priority) noexcept final;
    dom::ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    dom::ExceptionOr<void> SetCssText(const CSSOMString &) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(PropertyId) noexcept final;
    dom::ExceptionOr<void> SetPropertyInternal(PropertyId, const CSSOMString &value,
                                               IsImportant) noexcept final;
    KRYS_NODISCARD Ref<MutableStyleProperties> CopyProperties() const noexcept final;

    KRYS_NODISCARD Ref<dom::Element> ProtectedElement() const noexcept
    {
      return _element;
    }

    KRYS_NODISCARD const FixedVector<PropertyId> &ExposedComputedCSSPropertyIds() const noexcept;

    KRYS_NODISCARD style::Extractor GetExtractor() const noexcept;
  };

  /// @brief The resolved values of `element`'s properties, or of its pseudo-element `pseudoElement` (such as
  /// `::before`).
  /// @see https://drafts.csswg.org/cssom/#dom-window-getcomputedstyle
  // FIXME: This is a member of the window; move it there once there is one.
  KRYS_NODISCARD Ref<CSSStyleDeclaration> GetComputedStyle(dom::Element &element,
                                                           const CSSOMString &pseudoElement = {}) noexcept;
}
