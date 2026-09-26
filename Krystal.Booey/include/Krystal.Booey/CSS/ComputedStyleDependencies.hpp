#pragma once

#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

// #include "StyleExtractor.h"
// #include <WebCore/PseudoElementIdentifier.h>
// #include <WebCore/RenderStyleConstants.h>

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  class MutableStyleProperties;

  class CSSComputedStyleDeclaration final : public CSSStyleProperties,
                                            public RefCounted<CSSComputedStyleDeclaration>
  {
  public:
    enum class AllowVisited : bool
    {
      No,
      Yes
    };

  private:
    const Ref<dom::Element> _element;
    Maybe<Style::PseudoElementIdentifier> _pseudoElementIdentifier {null};
    bool _isEmpty {false};
    bool _allowVisitedStyle {false};

  public:
    virtual ~CSSComputedStyleDeclaration() noexcept;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }

    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }

    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration> Create(dom::Element &element,
                                                                  AllowVisited allowVisited) noexcept;

    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration>
      Create(dom::Element &, const std::optional<Style::PseudoElementIdentifier> &) noexcept;

    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration> CreateEmpty(dom::Element &element) noexcept;

    KRYS_NODISCARD CSSOMString GetPropertyValue(PropertyId property) const noexcept;

  private:
    enum class IsEmpty : bool
    {
      No,
      Yes
    };

    CSSComputedStyleDeclaration(dom::Element &element, AllowVisited allowVisited) noexcept;
    CSSComputedStyleDeclaration(dom::Element &element, IsEmpty isEmpty) noexcept;
    CSSComputedStyleDeclaration(
      dom::Element &element, const Maybe<Style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    // CSSOM functions. Don't make these public.
    CSSRule *ParentRule() const noexcept final;
    CSSRule *cssRules() const final;
    unsigned length() const final;
    String item(unsigned index) const final;
    RefPtr<DeprecatedCSSOMValue> getPropertyCSSValue(const String &propertyName) final;
    String getPropertyValue(const String &propertyName) final;
    String getPropertyPriority(const String &propertyName) final;
    String getPropertyShorthand(const String &propertyName) final;
    bool isPropertyImplicit(const String &propertyName) final;
    ExceptionOr<void> setProperty(const String &propertyName, const String &value,
                                  const String &priority) final;
    ExceptionOr<String> removeProperty(const String &propertyName) final;
    String cssText() const final;
    ExceptionOr<void> setCssText(const String &) final;
    String getPropertyValueInternal(CSSPropertyID) final;
    ExceptionOr<void> setPropertyInternal(CSSPropertyID, const String &value, IsImportant) final;
    Ref<MutableStyleProperties> copyProperties() const final;

    Ref<Element> protectedElement() const
    {
      return _element;
    }

    KRYS_NODISCARD const SmallList<PropertyId> &ExposedComputedCSSPropertyIDs() const;

    Style::Extractor extractor() const;
  };

} // namespace krys::boo::css
