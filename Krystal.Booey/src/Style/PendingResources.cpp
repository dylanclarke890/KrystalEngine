/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/PendingResources.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo::style
{
  namespace
  {
    // <https://html.spec.whatwg.org/multipage/urls-and-fetching.html#cors-settings-attributes>
    enum class LoadPolicy : uint8
    {
      CORS,
      NoCORS,
      Anonymous
    };

    void LoadPendingImage(dom::Document &document, const StyleImage *styleImage, const dom::Element *,
                          LoadPolicy = LoadPolicy::NoCORS) noexcept
    {
      if (!styleImage || !styleImage->IsPending())
      {
        return;
      }

      ResourceLoaderOptions options = CachedResourceLoader::DefaultCachedResourceOptions();

      const_cast<StyleImage &>(*styleImage).Load(document.GetCachedResourceLoader(), options);
    }
  }

  void LoadPendingResources(RenderStyle &style, dom::Document &document, const dom::Element *element) noexcept
  {
    for (auto &backgroundLayer : style.GetBackgroundLayers().UsedValues())
    {
      LoadPendingImage(document, backgroundLayer.GetImage().TryStyleImage().get(), element);
    }

    if (auto *contentData = style.GetContent().TryData())
    {
      for (auto &contentItem : contentData->List)
      {
        if (auto *image = std::get_if<Content::Image>(&contentItem))
        {
          LoadPendingImage(document, image->Value.Value.get(), element);
        }
      }
    }

    if (auto cursorImages = style.GetCursor().Images)
    {
      for (auto &cursorImage : *cursorImages)
      {
        LoadPendingImage(document, cursorImage.Image.get(), element);
      }
    }

    LoadPendingImage(document, style.ListStyleImage().TryStyleImage().get(), element);
    LoadPendingImage(document, style.GetBorderImageSource().TryStyleImage().get(), element);
    LoadPendingImage(document, style.GetMaskBorderSource().TryStyleImage().get(), element);

    // Masking operations may be sensitive to timing attacks that can be used to reveal the pixel data of
    // the image used as the mask. As a means to mitigate such attacks CSS mask images and shape-outside
    // images are retrieved in "Anonymous" mode, which uses a potentially CORS-enabled fetch.
    for (auto &maskLayer : style.GetMaskLayers().UsedValues())
    {
      LoadPendingImage(document, maskLayer.GetImage().TryStyleImage().get(), element, LoadPolicy::CORS);
    }

    if (RefPtr<StyleImage> shapeValueImage = style.GetShapeOutside().GetImage())
    {
      LoadPendingImage(document, shapeValueImage.get(), element, LoadPolicy::Anonymous);
    }

    // Are there other pseudo-elements that need resource loading?
    if (auto *firstLineStyle = style.GetCachedPseudoStyle({PseudoElementType::FirstLine}))
    {
      LoadPendingResources(*firstLineStyle, document, element);
    }
  }
}
