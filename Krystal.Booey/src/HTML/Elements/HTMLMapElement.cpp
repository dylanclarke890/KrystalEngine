#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"

namespace krys::boo
{
  HTMLMapElement::HTMLMapElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Map)
  {
  }

  bool HTMLMapElement::MapMouseEvent(LayoutPoint location, const LayoutSize &size,
                                     HitTestResult &result) noexcept
  {
    HTMLAreaElement *defaultArea = nullptr;

    for (auto &node : dom::DescendantRange(*this))
    {
      auto *area = DynamicDowncast<HTMLAreaElement>(node);
      if (!area)
      {
        continue;
      }
      if (area->IsDefault())
      {
        if (!defaultArea)
        {
          defaultArea = area;
        }
      }
      else if (area->MapMouseEvent(location, size, result))
      {
        return true;
      }
    }

    if (defaultArea)
    {
      result.SetInnerNode(defaultArea);
      result.SetUrlElement(defaultArea);
    }
    return defaultArea;
  }

  RefPtr<HTMLImageElement> HTMLMapElement::ImageElement() const noexcept
  {
    auto &root = dom::TreeQueries::Root(const_cast<HTMLMapElement &>(*this));
    for (auto &node : dom::DescendantRange(root))
    {
      auto *image = DynamicDowncast<HTMLImageElement>(node);
      if (image && image->AssociatedMapElement() == this)
      {
        return ShareRefPtr(image);
      }
    }
    return nullptr;
  }

#pragma region HTMLMapElement

  DOMString HTMLMapElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMapElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  Ref<HTMLCollection> HTMLMapElement::Areas() noexcept
  {
    return CreateRef<LiveHTMLCollection>(*this,
                                         [](const Element &element) { return Is<HTMLAreaElement>(element); });
  }

#pragma endregion

  void HTMLMapElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                          Maybe<DOMStringView> value,
                                          const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    if (namespaceURI == null && (localName == u8"id" || localName == u8"name") && oldValue != value
        && (IsConnected() || IsInShadowTree()))
    {
      auto &scope = TreeScope();
      if (oldValue && !oldValue->empty())
      {
        scope.RemoveImageMap(DOMStringAtom(*oldValue), *this);
      }
      if (value && !value->empty())
      {
        scope.AddImageMap(DOMStringAtom(*value), *this);
      }
    }
  }
}
