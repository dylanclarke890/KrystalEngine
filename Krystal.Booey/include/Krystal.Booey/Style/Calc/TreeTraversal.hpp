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

#pragma once

#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Core/Utils/Apply.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::style::calc
{
  // MARK: - ForAllChildren

  // `ForAllChildren` will call the provided `functor` on all direct children of the provided node. This
  // will include values of type `Child`, `ChildOrNone` and `Maybe<Child>`.

  template <typename F, Leaf Op>
  void ForAllChildren(const Op &, const F &) noexcept
  {
    // No children.
  }

  template <typename F, typename Op>
  void ForAllChildren(const Op &root, const F &functor) noexcept
  {
    struct Caller
    {
      const F &functor;

      void operator()(const Children &children) noexcept
      {
        for (auto &child : children)
        {
          functor(child);
        }
      }

      void operator()(const Maybe<Child> &root) noexcept
      {
        functor(root);
      }

      void operator()(const ChildOrNone &root) noexcept
      {
        functor(root);
      }

      void operator()(const Child &root) noexcept
      {
        functor(root);
      }

      void operator()(const double &root) noexcept
      {
        functor(root);
      }

      void operator()(const Random::Fixed &root) noexcept
      {
        functor(root);
      }
    };

    auto caller = Caller {functor};
    krys::apply([&](const auto &...x) { (..., caller(x)); }, root);
  }

  template <typename F>
  void ForAllChildren(const Child &root, const F &functor) noexcept
  {
    krys::SwitchOn(root,
                   [&]<typename T>(const T &node)
                   {
                     if constexpr (!Leaf<T>)
                     {
                       ForAllChildren(*node, functor);
                     }
                   });
  }
}
