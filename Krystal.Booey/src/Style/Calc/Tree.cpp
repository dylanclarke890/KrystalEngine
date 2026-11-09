/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/TreeTraversal.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>

namespace krys::boo::style::calc
{
  KRYS_NODISCARD static size_t ComputeDepth(const Child &root) noexcept
  {
    size_t maximumChildDepth = 0;
    ForAllChildren(
      root,
      krys::CreateVisitor(
        [&](const Maybe<Child> &child)
        {
          if (child)
          {
            maximumChildDepth = std::max(ComputeDepth(*child), maximumChildDepth);
          }
        },
        [&](const Child &child) { maximumChildDepth = std::max(ComputeDepth(child), maximumChildDepth); },
        [&](const ChildOrNone &childOrNone)
        {
          if (krys::HoldsAlternative<Child>(childOrNone))
          {
            maximumChildDepth = std::max(ComputeDepth(get<Child>(childOrNone)), maximumChildDepth);
          }
        },
        [&](const auto &) { maximumChildDepth = std::max<size_t>(1, maximumChildDepth); }));
    return maximumChildDepth + 1;
  }

  size_t ComputeDepth(const Tree &tree) noexcept
  {
    return ComputeDepth(tree.Root);
  }
}
