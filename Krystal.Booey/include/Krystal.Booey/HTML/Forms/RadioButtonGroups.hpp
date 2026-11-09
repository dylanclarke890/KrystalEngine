/*
 * Copyright (C) 2007-2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;
  class RadioButtonGroup;

  /// @brief The radio button groups of a form, or of a tree's radio buttons without a form, by name.
  /// @see https://html.spec.whatwg.org/#radio-button-group
  class RadioButtonGroups
  {
    KRYS_NON_COPY_MOVABLE(RadioButtonGroups);

  private:
    using NameToGroupMap = HashMap<dom::DOMStringAtom, UniquePtr<RadioButtonGroup>>;

    NameToGroupMap _nameToGroupMap;

  public:
    RadioButtonGroups() noexcept;
    ~RadioButtonGroups() noexcept;

    void Clear() noexcept;
    void AddButton(HTMLInputElement &element) noexcept;
    void UpdateCheckedState(HTMLInputElement &element) noexcept;
    void RequiredStateChanged(HTMLInputElement &element) noexcept;
    void RemoveButton(HTMLInputElement &element) noexcept;
    KRYS_NODISCARD RefPtr<HTMLInputElement>
      CheckedButtonForGroup(dom::DOMStringView groupName) const noexcept;
    KRYS_NODISCARD bool HasCheckedButton(const HTMLInputElement &element) const noexcept;
    KRYS_NODISCARD bool IsInRequiredGroup(HTMLInputElement &element) const noexcept;

    /// @brief The buttons in @p element's group, in tree order.
    KRYS_NODISCARD List<Ref<HTMLInputElement>> GroupMembers(const HTMLInputElement &element) const noexcept;

  private:
    KRYS_NODISCARD RadioButtonGroup *Find(dom::DOMStringView name) const noexcept;
  };
}
