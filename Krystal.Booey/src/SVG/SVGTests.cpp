#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGStringList.hpp"

namespace krys::boo::svg
{
  SVGConditionalProcessingAttributes::SVGConditionalProcessingAttributes(SVGElement &contextElement) noexcept
      : _requiredExtensions(SVGStringList::Create(&contextElement)),
        _systemLanguage(SVGStringList::Create(&contextElement))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      SVGTests::PropertyRegistry::RegisterConditionalProcessingAttributeProperty<
        &SVGConditionalProcessingAttributes::_requiredExtensions>(SVGNames::RequiredExtensionsAttr());
      SVGTests::PropertyRegistry::RegisterConditionalProcessingAttributeProperty<
        &SVGConditionalProcessingAttributes::_systemLanguage>(SVGNames::SystemLanguageAttr());
    }
  }

  SVGTests::SVGTests(SVGElement *contextElement) noexcept : _contextElement(CreateWeakPtr(contextElement))
  {
  }

  bool SVGTests::HasExtension(dom::DOMStringView extension) noexcept
  {
    // We recognize XHTML and MathML, as implemented in Gecko and suggested in the SVG Tiny
    // recommendation (http://www.w3.org/TR/SVG11/struct.html#RequiredExtensionsAttribute).
    if (extension == infra::Namespaces::MathML)
    {
      return true;
    }
    return extension == infra::Namespaces::HTML;
  }

  bool SVGTests::IsValid() const noexcept
  {
    auto *attributes = ConditionalProcessingAttributesIfExists();
    if (!attributes)
    {
      return true;
    }

    auto defaultLanguage = DefaultLanguage();
    auto genericDefaultLanguage = dom::DOMStringView(defaultLanguage).substr(0, 2);
    for (auto &language : attributes->SystemLanguage().Items())
    {
      if (language != genericDefaultLanguage)
      {
        return false;
      }
    }
    for (auto &extension : attributes->RequiredExtensions().Items())
    {
      if (!HasExtension(extension))
      {
        return false;
      }
    }
    return true;
  }

  void SVGTests::ParseAttribute(const dom::QualifiedName &name, Maybe<dom::DOMStringView> value) noexcept
  {
    // NOTE: an attribute that's present but empty evaluates to false (its list holds an empty string); one
    // that's absent evaluates to true.
    auto reset = [&](SVGStringList &list)
    {
      if (value)
      {
        list.Reset(*value);
      }
      else
      {
        list.ClearItems();
      }
    };

    if (name == SVGNames::RequiredExtensionsAttr())
    {
      reset(RequiredExtensions());
    }
    else if (name == SVGNames::SystemLanguageAttr())
    {
      reset(SystemLanguage());
    }
  }

  void SVGTests::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (!PropertyRegistry::IsKnownAttribute(attributeName))
    {
      return;
    }

    auto &contextElement = ContextElement();
    if (!contextElement.IsConnected())
    {
      return;
    }
    contextElement.InvalidateStyleAndRenderersForSubtree();
  }

  SVGElement &SVGTests::ContextElement() const noexcept
  {
    krys_assert(_contextElement);
    return *_contextElement;
  }

  SVGConditionalProcessingAttributes &SVGTests::ConditionalProcessingAttributes() noexcept
  {
    return ContextElement().ConditionalProcessingAttributes();
  }

  SVGConditionalProcessingAttributes *SVGTests::ConditionalProcessingAttributesIfExists() const noexcept
  {
    return ContextElement().ConditionalProcessingAttributesIfExists();
  }
}
