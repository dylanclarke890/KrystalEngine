/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLPictureElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD List<Ref<HTMLImageElement>> ImageChildren(const HTMLPictureElement &picture) noexcept
    {
      List<Ref<HTMLImageElement>> images;
      for (auto *child = picture.FirstChild(); child; child = child->NextSibling())
      {
        if (auto *image = DynamicDowncast<HTMLImageElement>(*child))
        {
          images.push_back(ShareRef(*image));
        }
      }
      return images;
    }
  }

  HTMLPictureElement::HTMLPictureElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Picture)
  {
  }

  void HTMLPictureElement::SourcesChanged() noexcept
  {
    for (auto &element : ImageChildren(*this))
    {
      element->SelectImageSource(RelevantMutation::Yes);
    }
  }

  void HTMLPictureElement::SourceDimensionAttributesChanged(const HTMLSourceElement &sourceElement) noexcept
  {
    for (auto &element : ImageChildren(*this))
    {
      if (&sourceElement == element->SourceElement())
      {
        element->InvalidateAttributeMapping();
      }
    }
  }
}
