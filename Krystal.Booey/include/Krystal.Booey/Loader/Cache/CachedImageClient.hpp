/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
    Copyright (C) 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"

namespace krys::boo
{
  class CachedImage;
  class IntRect;

  namespace dom
  {
    class Document;
  }

  enum class VisibleInViewportState : uint8
  {
    Unknown,
    Yes,
    No
  };

  /// @brief Something told when a cached image's data or frames change.
  class CachedImageClient : public CachedResourceClient
  {
  public:
    ~CachedImageClient() noexcept override = default;

    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return ImageType;
    }
    KRYS_NODISCARD CachedResourceClientType ResourceClientType() const noexcept override
    {
      return ExpectedType();
    }

    // Called whenever a frame of an image changes because we got more data from the network.
    // If not null, the IntRect is the changed rect of the image.
    virtual void ImageChanged(CachedImage *, const IntRect * = nullptr) noexcept
    {
    }

    KRYS_NODISCARD virtual bool CanDestroyDecodedData() const noexcept
    {
      return true;
    }

    // Called when a new decoded frame for a large image is available or when an animated image is ready to
    // advance to the next frame.
    virtual VisibleInViewportState ImageFrameAvailable(CachedImage &, ImageAnimatingState,
                                                       const IntRect *) noexcept;
    KRYS_NODISCARD virtual VisibleInViewportState ImageVisibleInViewport(const dom::Document &) const noexcept
    {
      return VisibleInViewportState::No;
    }

    virtual void DidRemoveCachedImageClient(CachedImage &) noexcept
    {
    }

    virtual void ImageContentChanged(CachedImage &) noexcept
    {
    }
    virtual void ScheduleRenderingUpdateForImage(CachedImage &) noexcept
    {
    }

    KRYS_NODISCARD virtual bool AllowsAnimation() const noexcept
    {
      return true;
    }

    /// @brief Whether the client is drawn with the dark appearance.
    KRYS_NODISCARD virtual bool UseSystemDarkAppearance() const noexcept
    {
      return false;
    }

  protected:
    CachedImageClient() noexcept = default;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(CachedImageClient, ImageType)
