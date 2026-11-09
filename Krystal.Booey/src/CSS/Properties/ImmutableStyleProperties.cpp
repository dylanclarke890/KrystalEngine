/**
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <new>

namespace krys::boo::css
{
  static_assert(sizeof(PropertyMetadata) == sizeof(uint32), "property metadata should stay small");

  ImmutableStyleProperties::ImmutableStyleProperties(Span<const Property> properties,
                                                     css::ParserMode mode) noexcept
      : StyleProperties(mode, static_cast<uint32>(properties.size()))
  {
    auto *metadata = const_cast<PropertyMetadata *>(MetadataSpan().data());
    auto *values = ValueSpan().data();
    for (auto i = 0uz; i < properties.size(); ++i)
    {
      auto &property = properties[i];
      new (metadata + i) PropertyMetadata(property.Metadata());
      auto &value = property.Value();
      new (values + i) PackedPtr<const css::Value>(&value);
      value.AddRef();
    }
  }

  ImmutableStyleProperties::~ImmutableStyleProperties() noexcept
  {
    for (auto &value : ValueSpan())
    {
      value->SubRef();
    }
  }

  Ref<ImmutableStyleProperties> ImmutableStyleProperties::Create(Span<const Property> properties,
                                                                 css::ParserMode mode) noexcept
  {
    void *slot = ::operator new(ObjectSize(static_cast<uint32>(properties.size())));
    return AdoptRef(*new (slot) ImmutableStyleProperties(properties, mode));
  }

  KRYS_NODISCARD static HashMap<size_t, Ref<ImmutableStyleProperties>> &DeduplicationMap() noexcept
  {
    static NeverDestroyed<HashMap<size_t, Ref<ImmutableStyleProperties>>> map;
    return map.get();
  }

  Ref<ImmutableStyleProperties> ImmutableStyleProperties::CreateDeduplicating(Span<const Property> properties,
                                                                              css::ParserMode mode) noexcept
  {
    constexpr static auto maximumDeduplicationMapSize = 1'024uz;
    auto &map = DeduplicationMap();
    if (map.size() >= maximumDeduplicationMapSize)
    {
      map.erase(map.begin());
    }

    auto computeHash = [&]() -> size_t
    {
      auto seed = Hash::Combine(mode);
      for (auto &property : properties)
      {
        if (!property.Value().AddHash(seed))
        {
          return 0uz;
        }

        seed = Hash::Combine(seed, property.Id(), static_cast<bool>(property.Important()));
      }

      return seed;
    };

    auto hash = computeHash();
    if (!hash)
    {
      return Create(properties, mode);
    }

    auto iterator = map.find(hash);
    if (iterator == map.end())
    {
      return map.emplace(hash, Create(properties, mode)).first->second;
    }

    auto isEqual = [&](const ImmutableStyleProperties &existingValue)
    {
      if (existingValue.PropertyCount() != properties.size())
      {
        return false;
      }

      if (existingValue.ParserMode() != mode)
      {
        return false;
      }

      for (auto i = 0u; i < properties.size(); ++i)
      {
        if (!(existingValue.PropertyAt(i).ToProperty() == properties[i]))
        {
          return false;
        }
      }

      return true;
    };

    if (!isEqual(*iterator->second))
    {
      return Create(properties, mode);
    }

    return iterator->second;
  }

  void ImmutableStyleProperties::ClearDeduplicationMap() noexcept
  {
    DeduplicationMap().clear();
  }

  Maybe<uint32> ImmutableStyleProperties::FindPropertyIndex(PropertyId property) const noexcept
  {
    auto metadataSpan = MetadataSpan();
    for (auto n = static_cast<uint32>(metadataSpan.size()); n--;)
    {
      if (metadataSpan[n].Id == property)
      {
        return n;
      }
    }

    return null;
  }

  Maybe<uint32> ImmutableStyleProperties::FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept
  {
    auto metadataSpan = MetadataSpan();
    auto valueSpan = ValueSpan();
    for (auto n = static_cast<uint32>(metadataSpan.size()); n--;)
    {
      if (metadataSpan[n].Id == PropertyId::Custom)
      {
        // We found a custom property. See if the name matches.
        auto *value = valueSpan[n].get();
        if (!value)
        {
          continue;
        }

        if (Downcast<CustomPropertyValue>(*value).Name().View() == propertyName)
        {
          return n;
        }
      }
    }

    return null;
  }
}
