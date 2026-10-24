#include "Krystal.Booey/CSS/CSSComputedStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCustomPropertyData.hpp"

namespace krys::boo::css
{
  CSSComputedStyleDeclaration::CSSComputedStyleDeclaration(dom::Element &element,
                                                           AllowVisited allowVisited) noexcept
      : _element(ShareRef(element)), _allowVisitedStyle(allowVisited == AllowVisited::Yes)
  {
  }

  CSSComputedStyleDeclaration::CSSComputedStyleDeclaration(dom::Element &element, IsEmpty isEmpty) noexcept
      : _element(ShareRef(element)), _isEmpty(isEmpty == IsEmpty::Yes)
  {
  }

  CSSComputedStyleDeclaration::CSSComputedStyleDeclaration(
    dom::Element &element, const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
      : _element(ShareRef(element)), _pseudoElementIdentifier(pseudoElementIdentifier)
  {
  }

  CSSComputedStyleDeclaration::~CSSComputedStyleDeclaration() noexcept = default;

  Ref<CSSComputedStyleDeclaration> CSSComputedStyleDeclaration::Create(dom::Element &element,
                                                                       AllowVisited allowVisited) noexcept
  {
    return AdoptRef(*new CSSComputedStyleDeclaration(element, allowVisited));
  }

  Ref<CSSComputedStyleDeclaration> CSSComputedStyleDeclaration::Create(
    dom::Element &element, const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return AdoptRef(*new CSSComputedStyleDeclaration(element, pseudoElementIdentifier));
  }

  Ref<CSSComputedStyleDeclaration> CSSComputedStyleDeclaration::CreateEmpty(dom::Element &element) noexcept
  {
    return AdoptRef(*new CSSComputedStyleDeclaration(element, IsEmpty::Yes));
  }

  style::Extractor CSSComputedStyleDeclaration::GetExtractor() const noexcept
  {
    return style::Extractor(_element.get(), _allowVisitedStyle, _pseudoElementIdentifier);
  }

  CSSOMString CSSComputedStyleDeclaration::CssText() const noexcept
  {
    return {};
  }

  dom::ExceptionOr<void> CSSComputedStyleDeclaration::SetCssText(const CSSOMString &) noexcept
  {
    return dom::ExceptionCode::NoModificationAllowedError;
  }

  Ref<MutableStyleProperties> CSSComputedStyleDeclaration::CopyProperties() const noexcept
  {
    if (_isEmpty)
    {
      return MutableStyleProperties::Create();
    }
    return GetExtractor().CopyProperties();
  }

  const FixedVector<PropertyId> &CSSComputedStyleDeclaration::ExposedComputedCSSPropertyIds() const noexcept
  {
    return ProtectedElement()->NodeDocument().ExposedComputedCSSPropertyIds();
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyValue(PropertyId propertyID) const noexcept
  {
    if (_isEmpty)
    {
      return {}; // FIXME: Should this be null instead, as it is in StyleProperties::GetPropertyValue?
    }

    return GetExtractor().PropertyValueSerialisation(propertyID, DefaultSerialisationContext());
  }

  uint32 CSSComputedStyleDeclaration::Length() const noexcept
  {
    if (_isEmpty)
    {
      return 0;
    }

    style::Extractor::UpdateStyleIfNeededForProperty(*_element, PropertyId::Custom);

    const auto *style = ProtectedElement()->ComputedStyle(_pseudoElementIdentifier);
    if (!style)
    {
      return 0;
    }

    return static_cast<uint32>(ExposedComputedCSSPropertyIds().size())
           + style->InheritedCustomProperties().Size() + style->NonInheritedCustomProperties().Size();
  }

  CSSOMString CSSComputedStyleDeclaration::Item(uint32 i) const noexcept
  {
    if (_isEmpty)
    {
      return {};
    }

    if (i >= Length())
    {
      return {};
    }

    auto exposedCount = static_cast<uint32>(ExposedComputedCSSPropertyIds().size());
    if (i < exposedCount)
    {
      return CSSOMString(ToString(ExposedComputedCSSPropertyIds()[i]));
    }

    const auto *style = ProtectedElement()->ComputedStyle(_pseudoElementIdentifier);
    if (!style)
    {
      return {};
    }

    const auto &inheritedCustomProperties = style->InheritedCustomProperties();

    // FIXME: FindKeyAtIndex does a linear search for the property name, so if
    // we are called in a loop over all item indexes, we'll spend quadratic time
    // searching for keys.

    if (i < exposedCount + inheritedCustomProperties.Size())
    {
      return CSSOMString(inheritedCustomProperties.FindKeyAtIndex(i - exposedCount).View());
    }

    const auto &nonInheritedCustomProperties = style->NonInheritedCustomProperties();
    return CSSOMString(
      nonInheritedCustomProperties.FindKeyAtIndex(i - inheritedCustomProperties.Size() - exposedCount)
        .View());
  }

  CSSRule *CSSComputedStyleDeclaration::ParentRule() const noexcept
  {
    return nullptr;
  }

  CSSRule *CSSComputedStyleDeclaration::CssRules() const noexcept
  {
    return nullptr;
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyValue(const CSSOMString &propertyName) noexcept
  {
    if (_isEmpty)
    {
      return {};
    }

    if (IsCustomPropertyName(propertyName))
    {
      return GetExtractor().CustomPropertyValueSerialisation(CSSOMStringAtom {propertyName},
                                                             DefaultSerialisationContext());
    }

    auto propertyID = PropertyIdFromName(propertyName);
    if (propertyID == PropertyId::Invalid)
    {
      return {};
    }

    return GetExtractor().PropertyValueSerialisation(propertyID, DefaultSerialisationContext());
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyPriority(const CSSOMString &) noexcept
  {
    // All computed styles have a priority of not "important".
    return {};
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyShorthand(const CSSOMString &) noexcept
  {
    return {}; // FIXME: Should this sometimes be null instead of empty, to match a normal style declaration?
  }

  bool CSSComputedStyleDeclaration::IsPropertyImplicit(const CSSOMString &) noexcept
  {
    return false;
  }

  dom::ExceptionOr<void> CSSComputedStyleDeclaration::SetProperty(const CSSOMString &, const CSSOMString &,
                                                                  const CSSOMString &) noexcept
  {
    return dom::ExceptionCode::NoModificationAllowedError;
  }

  dom::ExceptionOr<CSSOMString> CSSComputedStyleDeclaration::RemoveProperty(const CSSOMString &) noexcept
  {
    return dom::ExceptionCode::NoModificationAllowedError;
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyValueInternal(PropertyId propertyID) noexcept
  {
    return GetPropertyValue(propertyID);
  }

  dom::ExceptionOr<void> CSSComputedStyleDeclaration::SetPropertyInternal(PropertyId, const CSSOMString &,
                                                                          IsImportant) noexcept
  {
    return dom::ExceptionCode::NoModificationAllowedError;
  }
}
