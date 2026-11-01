#include "Krystal.Booey/HTML/HTMLSourceElement.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLPictureElement.hpp"

namespace krys::boo::html
{
  HTMLSourceElement::HTMLSourceElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Source)
  {
  }
#pragma region HTMLSourceElement

  dom::USVString HTMLSourceElement::Src() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLSourceElement::Src(dom::USVString &&value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::DOMString HTMLSourceElement::Type() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLSourceElement::Type(dom::DOMString &&value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::USVString HTMLSourceElement::SrcSet() const noexcept
  {
    return Reflect<dom::USVString>(*this, u8"srcset");
  }

  void HTMLSourceElement::SrcSet(dom::USVString &&value) noexcept
  {
    return Reflect<dom::USVString>(*this, u8"srcset", krys::move(value));
  }

  dom::DOMString HTMLSourceElement::Sizes() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLSourceElement::Sizes(dom::DOMString &&value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"sizes", krys::move(value));
  }

  dom::DOMString HTMLSourceElement::Media() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLSourceElement::Media(dom::DOMString &&value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  uint32 HTMLSourceElement::Width() const noexcept
  {
    return Reflect<uint32>(*this, u8"width");
  }

  void HTMLSourceElement::Width(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLSourceElement::Height() const noexcept
  {
    return Reflect<uint32>(*this, u8"height");
  }

  void HTMLSourceElement::Height(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"height", krys::move(value));
  }

#pragma endregion

  const css::mq::MediaQueryList &
    HTMLSourceElement::ParsedMediaAttribute(dom::Document &document) const noexcept
  {
    if (!_cachedParsedMediaAttribute)
    {
      _cachedParsedMediaAttribute =
        css::mq::MediaQueryParser::Parse(AttributeOrEmpty(u8"media"), css::ParserContext(document));
    }
    return *_cachedParsedMediaAttribute;
  }

  // https://html.spec.whatwg.org/#the-source-element:html-element-insertion-steps
  void HTMLSourceElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();

    // 1. Let parent be insertedNode's parent.
    // 2. If parent is a media element that has no src attribute and whose networkState has the value
    //    NETWORK_EMPTY, then invoke that media element's resource selection algorithm.
    // (Media elements don't select resources.)
    // 3. If parent is a picture element, then for each child of parent's children, if child is an img
    //    element, then count this as a relevant mutation for child.
    // Only an insertion into a <picture> is a relevant mutation, not the insertion of a subtree whose
    // <picture> already holds the source.
    auto *pictureElement = DynamicDowncast<HTMLPictureElement>(ParentElement());
    if (!pictureElement || pictureElement == _insertedIntoPicture)
    {
      return;
    }
    UpdateInsertedIntoPicture(*pictureElement);
    if (_shouldCallSourcesChanged)
    {
      pictureElement->SourcesChanged();
    }
  }

  // https://html.spec.whatwg.org/#the-source-element:html-element-removing-steps
  void HTMLSourceElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);

    // 1. If isSubtreeRoot is true and oldAncestor is a picture element, then for each child of oldAncestor's
    //    children: if child is an img element, then count this as a relevant mutation for child.
    if (!isSubtreeRoot)
    {
      return;
    }
    _insertedIntoPicture = nullptr;
    if (_shouldCallSourcesChanged)
    {
      if (auto *pictureElement = DynamicDowncast<HTMLPictureElement>(oldAncestor))
      {
        pictureElement->SourcesChanged();
      }
      _shouldCallSourcesChanged = false;
    }
  }

  // https://html.spec.whatwg.org/#the-source-element:html-element-moving-steps
  void HTMLSourceElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    // 1. If movedNode's parent is a media element that has no src attribute and whose networkState has the
    //    value NETWORK_EMPTY, then invoke that media element's resource selection algorithm.
    // (Media elements don't select resources.)

    // 2. If isSubtreeRoot is true and oldAncestor is a picture element, then for each child of oldAncestor's
    //    children: if child is an img element, then count this as a relevant mutation for child.
    bool wasSource = _shouldCallSourcesChanged;
    _insertedIntoPicture = nullptr;
    _shouldCallSourcesChanged = false;
    auto *newPicture = DynamicDowncast<HTMLPictureElement>(ParentElement());
    if (newPicture)
    {
      UpdateInsertedIntoPicture(*newPicture);
    }
    auto *oldPicture = isSubtreeRoot ? DynamicDowncast<HTMLPictureElement>(oldAncestor) : nullptr;
    if (oldPicture && (wasSource || _shouldCallSourcesChanged))
    {
      oldPicture->SourcesChanged();
    }

    // Moving into another picture counts for its images too, as inserting does.
    if (newPicture && newPicture != oldPicture && _shouldCallSourcesChanged)
    {
      newPicture->SourcesChanged();
    }
  }

  void HTMLSourceElement::UpdateInsertedIntoPicture(HTMLPictureElement &pictureElement) noexcept
  {
    _insertedIntoPicture = &pictureElement;

    // The new source element only is a relevant mutation if it precedes any img element.
    _shouldCallSourcesChanged = true;
    for (const auto *node = PreviousSibling(); node; node = node->PreviousSibling())
    {
      if (Is<HTMLImageElement>(*node))
      {
        _shouldCallSourcesChanged = false;
      }
    }
  }

  void HTMLSourceElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"type" || localName == u8"srcset" || localName == u8"sizes" || localName == u8"media")
    {
      if (localName == u8"media")
      {
        _cachedParsedMediaAttribute = null;
      }
      auto *pictureElement = DynamicDowncast<HTMLPictureElement>(ParentElement());
      if (_shouldCallSourcesChanged && pictureElement)
      {
        pictureElement->SourcesChanged();
      }
    }
    else if (localName == u8"width" || localName == u8"height")
    {
      if (auto *pictureElement = DynamicDowncast<HTMLPictureElement>(ParentElement()))
      {
        pictureElement->SourceDimensionAttributesChanged(*this);
      }
    }
  }
}
