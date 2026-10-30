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
