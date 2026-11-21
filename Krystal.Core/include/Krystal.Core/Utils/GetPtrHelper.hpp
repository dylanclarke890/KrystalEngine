/*
 *  Copyright (C) 2006-2019 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys
{
  template <typename T, bool IsSmartPtr>
  struct GetPtrHelperBase;

  template <typename T>
  struct GetPtrHelperBase<T, false /* IsSmartPtr */>
  {
    using PtrType = T *;
    using UnderlyingType = T;

    KRYS_NODISCARD constexpr static PtrType get(T &value) noexcept
    {
      return std::addressof(value);
    }
  };

  template <typename T>
  struct GetPtrHelperBase<T, true /* IsSmartPtr */>
  {
    using PtrType = decltype(std::declval<T>().get());
    using UnderlyingType = remove_ptr_t<PtrType>;

    KRYS_NODISCARD constexpr static PtrType get(const T &value) noexcept
    {
      return value.get();
    }
  };

  template <typename T>
  struct GetPtrHelper : public GetPtrHelperBase<T, IsSmartPtr<T>>
  {
  };

  template <typename T>
  KRYS_NODISCARD constexpr typename GetPtrHelper<T>::PtrType GetPtr(T &p) noexcept
  {
    return GetPtrHelper<T>::get(p);
  }

  template <typename T>
  KRYS_NODISCARD constexpr typename GetPtrHelper<T>::PtrType GetPtr(const T &p) noexcept
  {
    return GetPtrHelper<T>::get(p);
  }
}
