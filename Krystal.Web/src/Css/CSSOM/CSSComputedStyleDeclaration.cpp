#include "Krystal.Web/Css/CSSOM/CSSComputedStyleDeclaration.hpp"
#include "Krystal.Web/Css/Parser/KeywordLookup.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/Style/StyleCustomPropertyData.hpp"

namespace krys::web
{
  CSSComputedStyleDeclaration::CSSComputedStyleDeclaration(
    Element &element, const Maybe<style::PseudoElementIdentifier> &pseudoElement, bool hasNoTarget) noexcept
      : _element(ShareRef(element)), _pseudoElementIdentifier(pseudoElement), _hasNoTarget(hasNoTarget)
  {
  }

  CSSComputedStyleDeclaration::~CSSComputedStyleDeclaration() noexcept = default;

  Ref<CSSComputedStyleDeclaration>
    CSSComputedStyleDeclaration::Create(Element &element,
                                        const Maybe<style::PseudoElementIdentifier> &pseudoElement) noexcept
  {
    return AdoptRef(*new CSSComputedStyleDeclaration(element, pseudoElement, false));
  }

  Ref<CSSComputedStyleDeclaration> CSSComputedStyleDeclaration::CreateEmpty(Element &element) noexcept
  {
    return AdoptRef(*new CSSComputedStyleDeclaration(element, null, true));
  }

  bool CSSComputedStyleDeclaration::HasDeclarations() const noexcept
  {
    // 5. If obj is not null, and elt is connected, [...] set decls to a list of all longhand properties that
    //    are supported CSS properties, in lexicographical order, with the value being the resolved value
    //    computed for obj [...].
    return !_hasNoTarget && _element->IsConnected();
  }

  const FixedList<css::PropertyId> &CSSComputedStyleDeclaration::ExposedLonghands() const noexcept
  {
    return _element->NodeDocument().ExposedComputedCSSPropertyIds();
  }

  style::Extractor CSSComputedStyleDeclaration::GetExtractor() const noexcept
  {
    return style::Extractor(_element.get(), _pseudoElementIdentifier);
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyValue(css::PropertyId property) const noexcept
  {
    if (_hasNoTarget)
    {
      return {};
    }
    return GetExtractor().PropertyValueSerialisation(property, css::DefaultSerialisationContext());
  }

  CSSOMString CSSComputedStyleDeclaration::CssText() const noexcept
  {
    // A computed declaration block serialises as the empty string.
    return {};
  }

  ExceptionOr<void> CSSComputedStyleDeclaration::SetCssText(const CSSOMString &) noexcept
  {
    return ExceptionCode::NoModificationAllowedError;
  }

  uint32 CSSComputedStyleDeclaration::Length() const noexcept
  {
    if (!HasDeclarations())
    {
      return 0;
    }
    style::Extractor::UpdateStyleIfNeededForProperty(*_element, css::PropertyId::Custom);
    const auto *style = _element->ComputedStyle(_pseudoElementIdentifier);
    if (!style)
    {
      return 0;
    }
    return static_cast<uint32>(ExposedLonghands().size()) + style->InheritedCustomProperties().Size()
           + style->NonInheritedCustomProperties().Size();
  }

  CSSOMString CSSComputedStyleDeclaration::Item(uint32 index) const noexcept
  {
    if (index >= Length())
    {
      return {};
    }
    const auto &longhands = ExposedLonghands();
    if (index < longhands.size())
    {
      return CSSOMString(ToString(longhands[index]));
    }

    // The custom properties follow, the inherited ones first.
    const auto *style = _element->ComputedStyle(_pseudoElementIdentifier);
    if (!style)
    {
      return {};
    }
    // FIXME: FindKeyAtIndex searches the properties in turn, so reading every item takes quadratic time.
    auto customIndex = index - static_cast<uint32>(longhands.size());
    const auto &inherited = style->InheritedCustomProperties();
    if (customIndex < inherited.Size())
    {
      return inherited.FindKeyAtIndex(customIndex).ToString();
    }
    return style->NonInheritedCustomProperties().FindKeyAtIndex(customIndex - inherited.Size()).ToString();
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyValue(const CSSOMString &propertyName) noexcept
  {
    if (!HasDeclarations())
    {
      return {};
    }
    if (css::IsCustomPropertyName(propertyName))
    {
      return GetExtractor().CustomPropertyValueSerialisation(CSSOMStringAtom {propertyName},
                                                             css::DefaultSerialisationContext());
    }
    // A shorthand's value is serialised from its longhands.
    auto property = css::PropertyIdFromName(propertyName);
    if (property == css::PropertyId::Invalid)
    {
      return {};
    }
    return GetExtractor().PropertyValueSerialisation(property, css::DefaultSerialisationContext());
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyPriority(const CSSOMString &) noexcept
  {
    // No resolved value is important.
    return {};
  }

  ExceptionOr<void> CSSComputedStyleDeclaration::SetProperty(const CSSOMString &, const CSSOMString &,
                                                             const CSSOMString &) noexcept
  {
    return ExceptionCode::NoModificationAllowedError;
  }

  ExceptionOr<CSSOMString> CSSComputedStyleDeclaration::RemoveProperty(const CSSOMString &) noexcept
  {
    return ExceptionCode::NoModificationAllowedError;
  }

  CSSRule *CSSComputedStyleDeclaration::ParentRule() const noexcept
  {
    return nullptr;
  }

  CSSOMString CSSComputedStyleDeclaration::GetPropertyValueInternal(css::PropertyId property) noexcept
  {
    return HasDeclarations() ? GetPropertyValue(property) : CSSOMString {};
  }

  ExceptionOr<void> CSSComputedStyleDeclaration::SetPropertyInternal(css::PropertyId, const CSSOMString &,
                                                                     css::IsImportant) noexcept
  {
    return ExceptionCode::NoModificationAllowedError;
  }

  Ref<css::MutableStyleProperties> CSSComputedStyleDeclaration::CopyProperties() const noexcept
  {
    if (_hasNoTarget)
    {
      return css::MutableStyleProperties::Create();
    }
    return GetExtractor().CopyProperties();
  }
}
