/*
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/SVG/Properties/SVGList.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @brief A list of SVG properties, which it owns: a change to an item is committed through the list.
  template <typename TProperty>
  class SVGPropertyList : public SVGList<Ref<TProperty>>, public SVGPropertyOwner
  {
  public:
    using BaseList = SVGList<Ref<TProperty>>;
    using BaseList::Append;
    using BaseList::IsEmpty;
    using BaseList::Size;

  protected:
    using BaseList::_access;
    using BaseList::_items;
    using BaseList::_owner;

    SVGPropertyList(SVGPropertyOwner *owner = nullptr,
                    SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : BaseList(owner, access)
    {
    }

    ~SVGPropertyList() noexcept override
    {
      // Detach the items from the list before it is deleted.
      DetachItems();
    }

    void DetachItems() noexcept override
    {
      for (auto &item : _items)
      {
        item->Detach();
      }
    }

    KRYS_NODISCARD SVGPropertyOwner *Owner() const noexcept override
    {
      return _owner;
    }

    void CommitPropertyChange(SVGProperty *) noexcept override
    {
      if (Owner())
      {
        Owner()->CommitPropertyChange(this);
      }
    }

    KRYS_NODISCARD Ref<TProperty> At(uint32 index) const noexcept override
    {
      krys_assert(index < Size());
      return _items.at(index);
    }

    Ref<TProperty> InsertAt(uint32 index, Ref<TProperty> &&newItem) noexcept override
    {
      krys_assert(index <= Size());

      // Spec: if newItem is not a detached object, then set newItem to be
      // a clone object of newItem.
      if (newItem->IsAttached())
      {
        newItem = newItem->Clone();
      }

      // Spec: Attach newItem to the list object.
      newItem->Attach(this, _access);
      _items.insert(_items.begin() + index, krys::move(newItem));
      return At(index);
    }

    Ref<TProperty> ReplaceAt(uint32 index, Ref<TProperty> &&newItem) noexcept override
    {
      krys_assert(index < Size());
      Ref<TProperty> &item = _items[index];

      // Spec: Detach item.
      item->Detach();

      // Spec: if newItem is not a detached object, then set newItem to be
      // a clone object of newItem.
      if (newItem->IsAttached())
      {
        item = newItem->Clone();
      }
      else
      {
        item = krys::move(newItem);
      }

      // Spec: Attach newItem to the list object.
      item->Attach(this, _access);
      return At(index);
    }

    Ref<TProperty> RemoveAt(uint32 index) noexcept override
    {
      krys_assert(index < Size());
      Ref<TProperty> item = At(index);

      // Spec: Detach item.
      item->Detach();
      _items.erase(_items.begin() + index);
      return item;
    }

    Ref<TProperty> Append(Ref<TProperty> &&newItem) noexcept override
    {
      // Spec: if newItem is not a detached object, then set newItem to be
      // a clone object of newItem.
      if (newItem->IsAttached())
      {
        newItem = newItem->Clone();
      }

      // Spec: Attach newItem to the list object.
      newItem->Attach(this, _access);
      _items.push_back(krys::move(newItem));
      return At(static_cast<uint32>(Size() - 1));
    }
  };
}
