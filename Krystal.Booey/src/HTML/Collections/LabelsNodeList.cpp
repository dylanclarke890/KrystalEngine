/**
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
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
 */

#include "Krystal.Booey/HTML/Collections/LabelsNodeList.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLabelElement.hpp"

namespace krys::boo::html
{
  LabelsNodeList::LabelsNodeList(HTMLElement &ownerNode) noexcept
      : LiveNodeList(
          ownerNode,
          [this](const dom::Node &node)
          {
            auto *element = DynamicDowncast<dom::Element>(node);
            return element && ElementMatches(*element);
          },
          dom::LiveCollectionInvalidation::Labels, dom::LiveCollectionTraversal::OwnerTree),
        _ownerNode(ShareRef(ownerNode))
  {
  }

  bool LabelsNodeList::ElementMatches(const dom::Element &testNode) const noexcept
  {
    auto *label = DynamicDowncast<HTMLLabelElement>(testNode);
    return label && label->Control().get() == _ownerNode.get();
  }
}
