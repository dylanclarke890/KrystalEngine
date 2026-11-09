/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleCustomPropertyData.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::style
{
  static constexpr auto MaximumAncestorCount = 4;

  CustomPropertyData::CustomPropertyData(const CustomPropertyData &other) noexcept
      : _size(other._size), _mayHaveAnimatableProperties(other._mayHaveAnimatableProperties)
  {
    auto shouldReferenceAsParentValues = [&]
    {
      // Always reference the root style since it likely gets shared a lot.
      if (!other._parentValues && !other._ownValues.empty())
      {
        return true;
      }

      // Limit the list length.
      if (other._ancestorCount >= MaximumAncestorCount)
      {
        return false;
      }

      // If the number of properties is small we just copy them to avoid creating unnecessarily long linked
      // lists.
      static constexpr auto parentReferencePropertyCountThreshold = 8;
      return other._ownValues.size() > parentReferencePropertyCountThreshold;
    }();

    // If there are mutations on multiple levels this constructs a linked list of property data objects.
    if (shouldReferenceAsParentValues)
    {
      _parentValues = ShareRefPtr(&other);
    }
    else
    {
      _parentValues = other._parentValues;
      _ownValues = other._ownValues;
    }

    if (_parentValues)
    {
      _ancestorCount = _parentValues->_ancestorCount + 1;
    }

#if KRYS_DEBUG
    if (_parentValues)
    {
      _parentValues->_hasChildren = true;
    }
#endif
  }

  const CustomProperty *CustomPropertyData::Get(const css::CSSOMStringAtom &name) const noexcept
  {
    for (const auto *propertyData = this; propertyData; propertyData = propertyData->_parentValues.get())
    {
      if (auto it = propertyData->_ownValues.find(name); it != propertyData->_ownValues.end())
      {
        return it->second.get();
      }
    }
    return nullptr;
  }

  void CustomPropertyData::Set(const css::CSSOMStringAtom &name, Ref<const CustomProperty> &&value) noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(!_hasChildren);
#endif
    krys_debug_assert(
      [&]
      {
        const auto *existing = Get(name);
        return !existing || *existing != *value;
      }());

    _mayHaveAnimatableProperties = _mayHaveAnimatableProperties || value->IsAnimatable();

    auto [it, isNewEntry] = _ownValues.insert_or_assign(name, krys::move(value));

    bool isNewProperty = isNewEntry && (!_parentValues || !_parentValues->Get(name));
    if (isNewProperty)
    {
      ++_size;
    }
  }

  bool CustomPropertyData::operator==(const CustomPropertyData &other) const noexcept
  {
    if (_size != other._size)
    {
      return false;
    }

    if (_parentValues == other._parentValues)
    {
      // This relies on the values in _ownValues never being equal to those in _parentValues.
      if (_ownValues.size() != other._ownValues.size())
      {
        return false;
      }

      for (const auto &[key, value] : _ownValues)
      {
        auto it = other._ownValues.find(key);
        if (it == other._ownValues.end() || *value != *it->second)
        {
          return false;
        }
      }
      return true;
    }

    bool isEqual = true;
    ForEachInternal(
      [&](const Entry &entry)
      {
        const auto *otherValue = other.Get(entry.first);
        if (!otherValue || *entry.second != *otherValue)
        {
          isEqual = false;
          return IterationStatus::Done;
        }
        return IterationStatus::Continue;
      });

    return isEqual;
  }

  template <typename TCallback>
  void CustomPropertyData::ForEachInternal(TCallback &&callback) const noexcept
  {
    SmallList<const CustomPropertyData *, MaximumAncestorCount> descendants;

    auto isOverriddenByDescendants = [&](const css::CSSOMStringAtom &key)
    {
      for (const auto *descendant : descendants)
      {
        if (descendant->_ownValues.contains(key))
        {
          return true;
        }
      }
      return false;
    };

    const auto *propertyData = this;
    while (true)
    {
      for (const auto &entry : propertyData->_ownValues)
      {
        if (isOverriddenByDescendants(entry.first))
        {
          continue;
        }
        auto status = callback(entry);
        if (status == IterationStatus::Done)
        {
          return;
        }
      }
      if (!propertyData->_parentValues)
      {
        return;
      }
      descendants.push_back(propertyData);
      propertyData = propertyData->_parentValues.get();
    }
  }

  void CustomPropertyData::ForEach(const Func<IterationStatus(const Entry &)> &callback) const noexcept
  {
    ForEachInternal(callback);
  }

  css::CSSOMStringAtom CustomPropertyData::FindKeyAtIndex(uint32 index) const noexcept
  {
    uint32 currentIndex = 0;
    auto key = css::CSSOMStringAtom::Null();
    ForEachInternal(
      [&](const Entry &entry)
      {
        if (currentIndex == index)
        {
          key = entry.first;
          return IterationStatus::Done;
        }
        ++currentIndex;
        return IterationStatus::Continue;
      });
    return key;
  }
}
