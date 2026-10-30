#include "Krystal.Booey/HTML/HTMLSourceElement.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
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
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLSourceElement::Src(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::DOMString HTMLSourceElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLSourceElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::USVString HTMLSourceElement::SrcSet() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"srcset");
  }

  void HTMLSourceElement::SrcSet(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"srcset", krys::move(value));
  }

  dom::DOMString HTMLSourceElement::Sizes() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLSourceElement::Sizes(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"sizes", krys::move(value));
  }

  dom::DOMString HTMLSourceElement::Media() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLSourceElement::Media(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  uint32 HTMLSourceElement::Width() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLSourceElement::Width(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLSourceElement::Height() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLSourceElement::Height(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
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

  void HTMLSourceElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    // Only an insertion into a <picture> is a relevant mutation, not the insertion of a subtree whose
    // <picture> already holds the source.
    auto *pictureElement = DynamicDowncast<HTMLPictureElement>(ParentElement());
    if (!pictureElement || pictureElement == _insertedIntoPicture)
    {
      return;
    }
    _insertedIntoPicture = pictureElement;

    // The new source element only is a relevant mutation if it precedes any img element.
    _shouldCallSourcesChanged = true;
    for (const auto *node = PreviousSibling(); node; node = node->PreviousSibling())
    {
      if (Is<HTMLImageElement>(*node))
      {
        _shouldCallSourcesChanged = false;
      }
    }
    if (_shouldCallSourcesChanged)
    {
      pictureElement->SourcesChanged();
    }
  }

  void HTMLSourceElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
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
