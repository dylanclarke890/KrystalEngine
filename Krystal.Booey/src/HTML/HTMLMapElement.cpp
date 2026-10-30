#include "Krystal.Booey/HTML/HTMLMapElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"

namespace krys::boo::html
{
  HTMLMapElement::HTMLMapElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Map)
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
      result.SetURLElement(defaultArea);
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

  dom::DOMString HTMLMapElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMapElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  Ref<dom::HTMLCollection> HTMLMapElement::Areas() noexcept
  {
    return CreateRef<dom::LiveHTMLCollection>(*this, [](const dom::Element &element)
                                              { return Is<HTMLAreaElement>(element); });
  }

#pragma endregion
}
