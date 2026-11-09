/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Calc/TreeCopy.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Core/Utils/Apply.hpp"

namespace krys::boo::style::calc
{
  KRYS_NODISCARD static auto Copy(double) noexcept -> double;
  KRYS_NODISCARD static auto Copy(const Maybe<Child> &root) noexcept -> Maybe<Child>;
  KRYS_NODISCARD static auto Copy(const Random::Fixed &) noexcept -> Random::Fixed;
  KRYS_NODISCARD static auto Copy(const css::keywords::None &) noexcept -> css::keywords::None;
  KRYS_NODISCARD static auto Copy(const ChildOrNone &) noexcept -> ChildOrNone;
  KRYS_NODISCARD static auto Copy(const Children &) noexcept -> Children;
  KRYS_NODISCARD static auto Copy(const Child &) noexcept -> Child;
  template <Leaf Op>
  static Child Copy(const Op &) noexcept;
  template <typename Op>
  static auto Copy(const IndirectNode<Op> &) noexcept -> Child;

  // MARK: Copying

  Tree Copy(const Tree &tree) noexcept
  {
    return Tree {.Root = Copy(tree.Root)};
  }

  double Copy(double value) noexcept
  {
    return value;
  }

  Maybe<Child> Copy(const Maybe<Child> &root) noexcept
  {
    if (root)
    {
      return Copy(*root);
    }
    return null;
  }

  Random::Fixed Copy(const Random::Fixed &root) noexcept
  {
    return root;
  }

  css::keywords::None Copy(const css::keywords::None &none) noexcept
  {
    return none;
  }

  ChildOrNone Copy(const ChildOrNone &root) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return ChildOrNone {Copy(root)}; });
  }

  Children Copy(const Children &children) noexcept
  {
    List<Child> copyChildren;
    copyChildren.reserve(children.size());
    for (const auto &child : children)
    {
      copyChildren.push_back(Copy(child));
    }

    return copyChildren;
  }

  Child Copy(const Child &root) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return Copy(root); });
  }

  template <Leaf Op>
  Child Copy(const Op &root) noexcept
  {
    return {root};
  }

  template <typename Op>
  Child Copy(const IndirectNode<Op> &root) noexcept
  {
    return CreateChild(krys::apply([](const auto &...x) { return Op {Copy(x)...}; }, *root));
  }
}
