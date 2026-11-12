/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2014 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class CachedImage;
  class SharedBuffer;
}

namespace krys::boo::html
{
  class HTMLImageElement;

  /// @brief The document a load of an image shows: an `img` in an otherwise empty document, given the loaded
  /// bytes rather than fetching them. The image is shrunk to fit the view, and a click on it toggles between
  /// that and its natural size.
  /// @see https://html.spec.whatwg.org/#read-media
  class ImageDocument final : public dom::HTMLDocument
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ImageDocument);

  private:
    WeakPtr<HTMLImageElement> _imageElement;
    dom::EventListenerHandle _clickListener;
    bool _imageSizeIsKnown {false};
    bool _didShrinkImage {false};
    bool _shouldShrinkImage {true};

  public:
    ImageDocument() noexcept;
    ~ImageDocument() noexcept override;

    /// @brief The document's `img`, once the first data has made it.
    KRYS_NODISCARD HTMLImageElement *ImageElement() const noexcept;

    /// @brief Gives the image the bytes loaded so far, making the document's elements on the first call.
    void UpdateDuringParsing(const SharedBuffer &data) noexcept;
    /// @brief Gives the image all its bytes, titles the document after the image and its size, and
    /// finishes parsing.
    void FinishLoading(const SharedBuffer *data) noexcept;

    /// @brief Shrinks the image to the view's new size, or restores it once it fits.
    void DidChangeViewSize() noexcept;
    /// @brief Toggles between the shrunk image and its natural size, scrolling to the point clicked (@p x
    /// and @p y in the shrunk image) when it grows.
    void ImageClicked(int32 x, int32 y) noexcept;

  private:
    void CreateDocumentStructure() noexcept;
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept;
    KRYS_NODISCARD LayoutSize ImageSize() noexcept;
    void ImageUpdated() noexcept;
    KRYS_NODISCARD float Scale() noexcept;
    void ResizeImageToFit() noexcept;
    void RestoreImageSize() noexcept;
    KRYS_NODISCARD bool ImageFitsInWindow() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::ImageDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Document &document) noexcept
  {
    return document.IsImageDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
