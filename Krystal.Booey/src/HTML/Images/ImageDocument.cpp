/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Images/ImageDocument.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/ImageLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/URL/URLParser.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <algorithm>

namespace krys::boo::html
{
  ImageDocument::ImageDocument() noexcept : HTMLDocument(dom::DocumentFlags::IsImageDocument)
  {
    // https://html.spec.whatwg.org/#read-media
    // 2. Set document's mode to "no-quirks".
    SetQuirksMode(dom::QuirksMode::NoQuirks);
  }

  ImageDocument::~ImageDocument() noexcept = default;

  HTMLImageElement *ImageDocument::ImageElement() const noexcept
  {
    return _imageElement.get();
  }

  CachedImage *ImageDocument::GetCachedImage() const noexcept
  {
    auto *imageElement = ImageElement();
    return imageElement ? imageElement->GetCachedImage() : nullptr;
  }

  LayoutSize ImageDocument::ImageSize() noexcept
  {
    auto *imageElement = ImageElement();
    krys_debug_assert(imageElement);
    UpdateStyleIfNeeded();
    auto *cachedImage = GetCachedImage();
    if (!cachedImage)
    {
      return {};
    }
    auto *frame = Frame();
    return cachedImage->ImageSizeForRenderer(imageElement->Renderer(),
                                             frame ? frame->PageZoomFactor() : 1.0f);
  }

  void ImageDocument::UpdateDuringParsing(const SharedBuffer &data) noexcept
  {
    if (!_imageElement)
    {
      CreateDocumentStructure();
    }

    if (auto *cachedImage = GetCachedImage())
    {
      cachedImage->UpdateBuffer(data);
    }

    ImageUpdated();
  }

  void ImageDocument::FinishLoading(const SharedBuffer *data) noexcept
  {
    Ref<ImageDocument> protectedThis = ShareRef(*this);
    if (!_imageElement)
    {
      CreateDocumentStructure();
    }

    if (CachedResourceHandle<CachedImage> cachedImage = GetCachedImage())
    {
      cachedImage->FinishLoading(data);
      cachedImage->Finish();

      // The title gives the image's natural size, whatever the zoom.
      UpdateStyleIfNeeded();
      auto *imageElement = ImageElement();
      auto size = FlooredIntSize(
        cachedImage->ImageSizeForRenderer(imageElement ? imageElement->Renderer() : nullptr, 1.0f));
      if (size.Width())
      {
        // The decoded file name, or the host if the path has none.
        auto name = text::ReplaceInvalidUTF8(url::URLParser::PercentDecode(GetURL().LastPathComponent()));
        if (name.empty())
        {
          name = utf8_string(GetURL().Host());
        }
        StringBuilder title;
        title.Append(name, u8" (", size.Width(), u8"×", size.Height(), u8")");
        (void)Title(dom::DOMString(krys::move(title).ToString()));
      }

      ImageUpdated();
    }

    // https://html.spec.whatwg.org/#read-media
    // 10. Act as if the user agent had stopped parsing document.
    FinishedParsing();
  }

  void ImageDocument::CreateDocumentStructure() noexcept
  {
    // https://html.spec.whatwg.org/#read-media
    // 3. Append an html element to document.
    Ref rootElement = dom::ElementFactory::CreateHTMLElement<HTMLHtmlElement>(*this, u8"html");
    (void)AppendChild(*rootElement);
    (void)rootElement->SetInlineStyleProperty(css::PropertyId::Height, 100, css::UnitType::Percentage);

    // 4. Append a head element to the html element. (The title goes in it.)
    Ref head = dom::ElementFactory::CreateHTMLElement<HTMLHeadElement>(*this, u8"head");
    (void)rootElement->AppendChild(*head);

    // 5. Append a body element to the html element.
    Ref body = dom::ElementFactory::CreateHTMLElement<HTMLBodyElement>(*this, u8"body");
    (void)body->SetAttribute(u8"style", u8"margin: 0px; height: 100%");
    (void)rootElement->AppendChild(*body);

    // 6.-7. Append an img element to the body, whose src is the image's URL. Its image takes the loaded
    // bytes rather than requesting them.
    Ref imageElement = dom::ElementFactory::CreateHTMLElement<HTMLImageElement>(*this, u8"img");
    (void)imageElement->SetAttribute(u8"style", _shouldShrinkImage
                                                  ? u8"user-select: none; display: block; margin: auto"
                                                  : u8"user-select: none; display: block");
    auto &imageLoader = imageElement->GetImageLoader();
    imageLoader.SetLoadManually(true);
    (void)imageElement->SetAttribute(u8"src", dom::DOMString(GetURL().String()));
    (void)body->AppendChild(*imageElement);
    imageLoader.SetLoadManually(false);
    if (auto *cachedImage = imageElement->GetCachedImage())
    {
      cachedImage->SetResponse(ResourceResponse(GetURL(), utf8_string(ContentType()), -1));
    }

    if (_shouldShrinkImage)
    {
      _clickListener = imageElement->AddEventListener<dom::MouseEvent>(
        dom::DOMStringAtom(EventNames::Click),
        [document = CreateWeakPtr(this)](dom::MouseEvent &event)
        {
          if (document && dom::IsAnyClick(event))
          {
            document->ImageClicked(static_cast<int32>(event.OffsetX()), static_cast<int32>(event.OffsetY()));
          }
        });
    }

    _imageElement = CreateWeakPtr(imageElement.get());
  }

  void ImageDocument::ImageUpdated() noexcept
  {
    krys_debug_assert(_imageElement);

    if (_imageSizeIsKnown)
    {
      return;
    }

    auto imageSize = ImageSize();
    if (imageSize.IsEmpty())
    {
      return;
    }

    _imageSizeIsKnown = true;

    if (_shouldShrinkImage)
    {
      // Call DidChangeViewSize for its side effect of sizing the image.
      DidChangeViewSize();
    }
  }

  float ImageDocument::Scale() noexcept
  {
    if (!_imageElement)
    {
      return 1;
    }

    auto *view = View();
    if (!view)
    {
      return 1;
    }

    auto imageSize = ImageSize();
    auto viewportSize = view->VisibleSize();
    float widthScale = static_cast<float>(viewportSize.Width()) / imageSize.Width().ToFloat();
    float heightScale = static_cast<float>(viewportSize.Height()) / imageSize.Height().ToFloat();

    return std::min(widthScale, heightScale);
  }

  void ImageDocument::ResizeImageToFit() noexcept
  {
    RefPtr<HTMLImageElement> imageElement = ShareRefPtr(ImageElement());
    if (!imageElement)
    {
      return;
    }

    auto imageSize = ImageSize();

    float scale = Scale();
    imageElement->Width(static_cast<uint32>(imageSize.Width().ToFloat() * scale));
    imageElement->Height(static_cast<uint32>(imageSize.Height().ToFloat() * scale));

    (void)imageElement->SetInlineStyleProperty(css::PropertyId::Cursor, css::ValueId::ZoomIn);
  }

  void ImageDocument::RestoreImageSize() noexcept
  {
    if (!_imageSizeIsKnown)
    {
      return;
    }

    RefPtr<HTMLImageElement> imageElement = ShareRefPtr(ImageElement());
    if (!imageElement)
    {
      return;
    }

    auto imageSize = ImageSize();
    imageElement->Width(static_cast<uint32>(imageSize.Width().ToUnsigned()));
    imageElement->Height(static_cast<uint32>(imageSize.Height().ToUnsigned()));

    if (ImageFitsInWindow())
    {
      (void)imageElement->RemoveInlineStyleProperty(css::PropertyId::Cursor);
    }
    else
    {
      (void)imageElement->SetInlineStyleProperty(css::PropertyId::Cursor, css::ValueId::ZoomOut);
    }

    _didShrinkImage = false;
  }

  bool ImageDocument::ImageFitsInWindow() noexcept
  {
    if (!_imageElement)
    {
      return true;
    }

    auto *view = View();
    if (!view)
    {
      return true;
    }

    auto imageSize = ImageSize();
    auto viewportSize = view->VisibleSize();
    return imageSize.Width() <= viewportSize.Width() && imageSize.Height() <= viewportSize.Height();
  }

  void ImageDocument::DidChangeViewSize() noexcept
  {
    if (!_imageSizeIsKnown)
    {
      return;
    }

    RefPtr<HTMLImageElement> imageElement = ShareRefPtr(ImageElement());
    if (!imageElement)
    {
      return;
    }

    bool fitsInWindow = ImageFitsInWindow();

    // If the image has been explicitly zoomed in, restore the cursor if the image fits and set it to a zoom
    // out cursor if the image doesn't fit.
    if (!_shouldShrinkImage)
    {
      if (fitsInWindow)
      {
        (void)imageElement->RemoveInlineStyleProperty(css::PropertyId::Cursor);
      }
      else
      {
        (void)imageElement->SetInlineStyleProperty(css::PropertyId::Cursor, css::ValueId::ZoomOut);
      }
      return;
    }

    if (_didShrinkImage)
    {
      // If the window has been resized so that the image fits, restore the image size, otherwise update the
      // restored image size.
      if (fitsInWindow)
      {
        RestoreImageSize();
      }
      else
      {
        ResizeImageToFit();
      }
    }
    else if (!fitsInWindow)
    {
      // If the image isn't resized but needs to be, then resize it.
      ResizeImageToFit();
      _didShrinkImage = true;
    }
  }

  void ImageDocument::ImageClicked(int32 x, int32 y) noexcept
  {
    if (!_imageSizeIsKnown || ImageFitsInWindow())
    {
      return;
    }

    _shouldShrinkImage = !_shouldShrinkImage;

    if (_shouldShrinkImage)
    {
      // Call DidChangeViewSize for its side effect of sizing the image.
      DidChangeViewSize();
      return;
    }

    RestoreImageSize();
    (void)UpdateLayout();

    auto *view = View();
    if (!view)
    {
      return;
    }

    float scale = Scale();
    auto viewportSize = view->VisibleSize();
    auto scrollX =
      static_cast<int32>(static_cast<float>(x) / scale - static_cast<float>(viewportSize.Width()) / 2.0f);
    auto scrollY =
      static_cast<int32>(static_cast<float>(y) / scale - static_cast<float>(viewportSize.Height()) / 2.0f);
    view->SetScrollPosition(IntPoint(scrollX, scrollY));
  }
}
