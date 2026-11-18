/*
 * Copyright (C) 2007-2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Elements/HTMLSourceElement.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLPictureElement.hpp"

namespace krys::boo
{
  HTMLSourceElement::HTMLSourceElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Source)
  {
  }

#pragma region HTMLSourceElement

  dom::USVString HTMLSourceElement::Src() const noexcept
  {
    return html::ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLSourceElement::Src(dom::USVString value) noexcept
  {
    return html::ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  DOMString HTMLSourceElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLSourceElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

  dom::USVString HTMLSourceElement::SrcSet() const noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"srcset");
  }

  void HTMLSourceElement::SrcSet(dom::USVString value) noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"srcset", krys::move(value));
  }

  DOMString HTMLSourceElement::Sizes() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLSourceElement::Sizes(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"sizes", krys::move(value));
  }

  DOMString HTMLSourceElement::Media() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLSourceElement::Media(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"media", krys::move(value));
  }

  uint32 HTMLSourceElement::Width() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"width");
  }

  void HTMLSourceElement::Width(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLSourceElement::Height() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"height");
  }

  void HTMLSourceElement::Height(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

#pragma endregion

  const css::mq::MediaQueryList &HTMLSourceElement::ParsedMediaAttribute(Document &document) const noexcept
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
  void HTMLSourceElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
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
  void HTMLSourceElement::OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
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

  void HTMLSourceElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                             Maybe<DOMStringView> value,
                                             const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
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
