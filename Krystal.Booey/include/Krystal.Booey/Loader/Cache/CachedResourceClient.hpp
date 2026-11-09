/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.

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

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class CachedResource;

  /// @brief Something told when a cached resource finishes loading.
  /// @note A client removes itself from the resources it was added to before it's destroyed.
  class CachedResourceClient
  {
    KRYS_NON_COPYABLE(CachedResourceClient);

  public:
    enum CachedResourceClientType : uint8
    {
      BaseResourceType,
      ImageType,
      FontType,
      StyleSheetType,
      SVGDocumentType,
      RawResourceType
    };

    virtual ~CachedResourceClient() noexcept = default;

    virtual void NotifyFinished(CachedResource &) noexcept
    {
    }

    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return BaseResourceType;
    }
    KRYS_NODISCARD virtual CachedResourceClientType ResourceClientType() const noexcept
    {
      return ExpectedType();
    }
    KRYS_NODISCARD virtual bool ShouldMarkAsReferenced() const noexcept
    {
      return true;
    }

  protected:
    CachedResourceClient() noexcept = default;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(ToClassName, CachedResourceTypeValue)             \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::boo::CachedResourceClient &client) noexcept                \
    {                                                                                                        \
      return client.ResourceClientType() == krys::boo::CachedResourceClient::CachedResourceTypeValue;        \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
