#pragma once

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Core/Visitor.hpp"
#include "Krystal.Core/Utils/Apply.hpp"

namespace krys::boo::css::calc
{
  // MARK: Traversal

  // MARK: - ForAllChildren

  /// @brief `ForAllChildren` will call the provided `functor` on all direct children of the provided node.
  /// This will include values of type `Child`, `ChildOrNone` and `Maybe<Child>`.
  template <typename F, Leaf Op>
  void ForAllChildren(const auto &, const F &) noexcept
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

      void operator()(const CSSOMStringAtom &root) noexcept
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
    krys::SwitchOn(root, [&](const auto &root) { ForAllChildren(*root, functor); });
  }

  // MARK: - ForAllChildNodes

  // `ForAllChildNodes` will call the provided `functor` on all direct `Child` typed children of the
  // provided node. If a child is of type `ChildOrNone` or `Maybe<Child>`, the functor will be
  // called on the unwrapped `Child` if and only if that is what the type is holding.

  template <typename F, Leaf Op>
  void ForAllChildNodes(const Op &, const F &) noexcept
  {
    // No children.
  }

  template <typename F, typename Op>
  void ForAllChildNodes(const Op &root, const F &functor) noexcept
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
        if (root)
        {
          functor(*root);
        }
      }

      void operator()(const ChildOrNone &root) noexcept
      {
        krys::SwitchOn(root, [&](const Child &root) { functor(root); }, [&](const keywords::None &) {});
      }

      void operator()(const Child &root)
      {
        functor(root);
      }

      void operator()(const CSSOMStringAtom &) noexcept
      {
      }

      void operator()(const Random::SharingType &) noexcept
      {
      }
    };

    auto caller = Caller {functor};
    krys::apply([&](const auto &...x) { (..., caller(x)); }, root);
  }

  template <typename F>
  void ForAllChildNodes(const Child &root, const F &functor) noexcept
  {
    krys::SwitchOn(root, [&](const auto &root) { ForAllChildNodes(*root, functor); });
  }
}