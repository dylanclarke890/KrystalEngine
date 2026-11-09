/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Fonts/FontFeatureValues.hpp"

namespace krys::boo
{
  void FontFeatureValues::UpdateOrInsert(const FontFeatureValues &other) noexcept
  {
    if (this == &other)
    {
      return;
    }

    auto updateOrInsertTags = [](Tags &into, const Tags &tags)
    {
      for (const auto &[name, values] : tags)
      {
        into.insert_or_assign(name, values);
      }
    };
    updateOrInsertTags(_styleset, other.Styleset());
    updateOrInsertTags(_stylistic, other.Stylistic());
    updateOrInsertTags(_characterVariant, other.CharacterVariant());
    updateOrInsertTags(_swash, other.Swash());
    updateOrInsertTags(_ornaments, other.Ornaments());
    updateOrInsertTags(_annotation, other.Annotation());
  }

  void FontFeatureValues::UpdateOrInsertForType(FontFeatureValuesType type,
                                                const List<FontFeatureValuesTag> &tags) noexcept
  {
    auto updateOrInsertTags = [](Tags &into, const List<FontFeatureValuesTag> &tags)
    {
      for (const auto &[name, values] : tags)
      {
        into.insert_or_assign(name, values);
      }
    };
    switch (type)
    {
      case FontFeatureValuesType::Styleset:
      {
        updateOrInsertTags(_styleset, tags);
        break;
      }
      case FontFeatureValuesType::Stylistic:
      {
        updateOrInsertTags(_stylistic, tags);
        break;
      }
      case FontFeatureValuesType::CharacterVariant:
      {
        updateOrInsertTags(_characterVariant, tags);
        break;
      }
      case FontFeatureValuesType::Swash:
      {
        updateOrInsertTags(_swash, tags);
        break;
      }
      case FontFeatureValuesType::Ornaments:
      {
        updateOrInsertTags(_ornaments, tags);
        break;
      }
      case FontFeatureValuesType::Annotation:
      {
        updateOrInsertTags(_annotation, tags);
        break;
      }
    }
  }

  size_t FontFeatureValues::Hash() const noexcept
  {
    // The maps are unordered, so equal maps can iterate in different orders: combine the entries with an
    // order-independent sum.
    auto hashTags = [](const Tags &tags) -> size_t
    {
      size_t sum = 0;
      for (const auto &[name, values] : tags)
      {
        size_t entry = DefaultHash<css::CSSOMString>::Hash(name);
        for (auto value : values)
        {
          entry = krys::Hash::Combine(entry, value);
        }
        sum += entry;
      }
      return krys::Hash::Combine(tags.empty(), sum);
    };
    return krys::Hash::Combine(hashTags(_styleset), hashTags(_stylistic), hashTags(_characterVariant),
                               hashTags(_swash), hashTags(_ornaments), hashTags(_annotation));
  }
}
