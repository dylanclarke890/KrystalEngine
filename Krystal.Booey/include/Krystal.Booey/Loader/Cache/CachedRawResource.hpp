/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
    Copyright (C) 2004-2025 Apple Inc. All rights reserved.

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

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"

namespace krys::boo
{
  /// @brief A resource kept as its bytes (`ResourceBuffer`), for its client to make what it likes of: an
  /// icon.
  class CachedRawResource final : public CachedResource
  {
  public:
    CachedRawResource(CachedResourceRequest &&request, Type type) noexcept;

  private:
    void FinishLoading(const SharedBuffer *data) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CachedRawResource)
  KRYS_NODISCARD static bool IsType(const krys::boo::CachedResource &resource) noexcept
  {
    return resource.GetType() == krys::boo::CachedResource::Type::RawResource
           || resource.GetType() == krys::boo::CachedResource::Type::Icon;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
