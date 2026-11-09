/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/DOM/Internals/UserActionElementSet.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::dom
{
  void UserActionElementSet::Clear() noexcept
  {
    for (auto &[element, flags] : _elements)
    {
      const_cast<Element *>(element)->SetUserActionElement(false);
    }
    _elements.clear();
  }

  bool UserActionElementSet::HasFlag(const Element &element, Flag flag) const noexcept
  {
    // Caller has the responsibility to check IsUserActionElement before calling this.
    krys_debug_assert(element.IsUserActionElement());

    auto iterator = _elements.find(&element);
    return iterator != _elements.end() && iterator->second.Contains(flag);
  }

  void UserActionElementSet::ClearFlags(Element &element, EnumSet<Flag> flags) noexcept
  {
    krys_debug_assert(!flags.IsEmpty());

    if (!element.IsUserActionElement())
    {
      return;
    }

    auto iterator = _elements.find(&element);
    krys_debug_assert(iterator != _elements.end());
    auto updatedFlags = iterator->second - flags;
    if (updatedFlags.IsEmpty())
    {
      element.SetUserActionElement(false);
      _elements.erase(iterator);
    }
    else
    {
      iterator->second = updatedFlags;
    }
  }

  void UserActionElementSet::SetFlags(Element &element, EnumSet<Flag> flags) noexcept
  {
    krys_debug_assert(!flags.IsEmpty());

    _elements[&element].Add(flags);

    element.SetUserActionElement(true);
  }
}
