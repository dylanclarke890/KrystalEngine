/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/IterationStatus.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  /// @brief The custom properties of a style, shared copy-on-write between styles.
  /// @note Copies reference their source as a parent instead of copying it when it holds many properties, so
  /// mutations on several levels form a short linked list of property data.
  class CustomPropertyData : public RefCounted<CustomPropertyData>
  {
    using CustomPropertyValueMap = HashMap<css::CSSOMStringAtom, RefPtr<const CustomProperty>>;

  private:
    RefPtr<const CustomPropertyData> _parentValues;
    CustomPropertyValueMap _ownValues;
    uint32 _size {0};
    uint32 _ancestorCount {0};
    bool _mayHaveAnimatableProperties {false};
#if KRYS_DEBUG
    mutable bool _hasChildren {false};
#endif

  public:
    using Entry = CustomPropertyValueMap::value_type;

    KRYS_NODISCARD static Ref<CustomPropertyData> Create() noexcept
    {
      return AdoptRef(*new CustomPropertyData);
    }
    KRYS_NODISCARD Ref<CustomPropertyData> Copy() const noexcept
    {
      return AdoptRef(*new CustomPropertyData(*this));
    }

    KRYS_NODISCARD bool operator==(const CustomPropertyData &) const noexcept;

    KRYS_NODISCARD const CustomProperty *Get(const css::CSSOMStringAtom &) const noexcept;
    void Set(const css::CSSOMStringAtom &, Ref<const CustomProperty> &&) noexcept;

    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD bool MayHaveAnimatableProperties() const noexcept
    {
      return _mayHaveAnimatableProperties;
    }

    void ForEach(const Func<IterationStatus(const Entry &)> &) const noexcept;
    KRYS_NODISCARD css::CSSOMStringAtom FindKeyAtIndex(uint32) const noexcept;

  private:
    CustomPropertyData() noexcept = default;
    CustomPropertyData(const CustomPropertyData &) noexcept;

    template <typename TCallback>
    void ForEachInternal(TCallback &&) const noexcept;
  };
}
