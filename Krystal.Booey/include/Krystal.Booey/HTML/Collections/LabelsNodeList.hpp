/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2020 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html
{
  class HTMLElement;

  /// @brief The labels of a labelable element: the `label` elements in its tree whose labeled control it is,
  /// in tree order.
  /// @see https://html.spec.whatwg.org/#dom-lfe-labels
  class LabelsNodeList final : public dom::LiveNodeList
  {
  private:
    Ref<HTMLElement> _ownerNode;

  public:
    explicit LabelsNodeList(HTMLElement &ownerNode) noexcept;

    KRYS_NODISCARD bool ElementMatches(const dom::Element &testNode) const noexcept;
  };
}
