#pragma once

#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys::boo
{
  /// @brief Similar to Is<>() but without the static_assert() making sure the check is necessary.
  template <typename T, typename U>
  KRYS_NODISCARD inline bool IsRendererOfType(const U &renderer) noexcept
  {
    return Is<remove_const_t<T>>(static_cast<const RenderObject &>(renderer));
  }

  // Traversal helpers

  struct RenderObjectTraversal
  {
    template <typename U>
    KRYS_NODISCARD static RenderObject *FirstChild(const U &object) noexcept
    {
      if constexpr (DerivedFrom<U, RenderElement>)
      {
        return object.FirstChild();
      }
      else
      {
        return object.FirstChildSlow();
      }
    }

    KRYS_NODISCARD static RenderObject *NextAncestorSibling(const RenderObject &current,
                                                            const RenderObject *stayWithin) noexcept
    {
      for (auto *ancestor = current.Parent(); ancestor; ancestor = ancestor->Parent())
      {
        if (ancestor == stayWithin)
        {
          return nullptr;
        }
        if (auto *sibling = ancestor->NextSibling())
        {
          return sibling;
        }
      }
      return nullptr;
    }

    template <typename U>
    KRYS_NODISCARD static RenderObject *Next(const U &current, const RenderObject *stayWithin) noexcept
    {
      if (auto *child = FirstChild(current))
      {
        return child;
      }

      if (&current == stayWithin)
      {
        return nullptr;
      }

      if (auto *sibling = current.NextSibling())
      {
        return sibling;
      }

      return NextAncestorSibling(current, stayWithin);
    }

    KRYS_NODISCARD static RenderObject *NextSkippingChildren(const RenderObject &current,
                                                             const RenderObject *stayWithin) noexcept
    {
      if (&current == stayWithin)
      {
        return nullptr;
      }

      if (auto *sibling = current.NextSibling())
      {
        return sibling;
      }

      return NextAncestorSibling(current, stayWithin);
    }
  };

  struct RenderObjectPostOrderTraversal
  {
    KRYS_NODISCARD static RenderObject *Next(const RenderObject &current,
                                             const RenderObject *stayWithin) noexcept
    {
      if (auto *sibling = current.NextSibling())
      {
        if (auto *firstLeafChild = sibling->FirstLeafChild())
        {
          return firstLeafChild;
        }
        return sibling;
      }

      auto *parent = current.Parent();
      if (parent == stayWithin)
      {
        return nullptr;
      }
      return parent;
    }
  };

  struct RenderTraversal
  {
    template <typename T, typename U>
    KRYS_NODISCARD static T *FirstChild(const U &current) noexcept
    {
      RenderObject *object = RenderObjectTraversal::FirstChild(current);
      while (object && !IsRendererOfType<T>(*object))
      {
        object = object->NextSibling();
      }
      return static_cast<T *>(object);
    }

    template <typename T, typename U>
    KRYS_NODISCARD static T *LastChild(const U &current) noexcept
    {
      RenderObject *object = current.LastChild();
      while (object && !IsRendererOfType<T>(*object))
      {
        object = object->PreviousSibling();
      }
      return static_cast<T *>(object);
    }

    template <typename T, typename U>
    KRYS_NODISCARD static T *NextSibling(const U &current) noexcept
    {
      RenderObject *object = current.NextSibling();
      while (object && !IsRendererOfType<T>(*object))
      {
        object = object->NextSibling();
      }
      return static_cast<T *>(object);
    }

    template <typename T, typename U>
    KRYS_NODISCARD static T *PreviousSibling(const U &current) noexcept
    {
      RenderObject *object = current.PreviousSibling();
      while (object && !IsRendererOfType<T>(*object))
      {
        object = object->PreviousSibling();
      }
      return static_cast<T *>(object);
    }

    template <typename T>
    KRYS_NODISCARD static T *FindAncestorOfType(const RenderObject &current) noexcept
    {
      for (auto *ancestor = current.Parent(); ancestor; ancestor = ancestor->Parent())
      {
        if (IsRendererOfType<T>(*ancestor))
        {
          return static_cast<T *>(ancestor);
        }
      }
      return nullptr;
    }

    template <typename T, typename U>
    KRYS_NODISCARD static T *FirstWithin(const U &current) noexcept
    {
      auto *descendant = RenderObjectTraversal::FirstChild(current);
      while (descendant && !IsRendererOfType<T>(*descendant))
      {
        descendant = RenderObjectTraversal::Next(*descendant, &current);
      }
      return static_cast<T *>(descendant);
    }

    template <typename T, typename U>
    KRYS_NODISCARD static T *Next(const U &current, const RenderObject *stayWithin) noexcept
    {
      auto *descendant = RenderObjectTraversal::Next(current, stayWithin);
      while (descendant && !IsRendererOfType<T>(*descendant))
      {
        descendant = RenderObjectTraversal::Next(*descendant, stayWithin);
      }
      return static_cast<T *>(descendant);
    }
  };

  struct RenderPostOrderTraversal
  {
    template <typename T>
    KRYS_NODISCARD static T *FirstWithin(const RenderObject &current) noexcept
    {
      auto *descendant = current.FirstLeafChild();
      while (descendant && !IsRendererOfType<T>(*descendant))
      {
        descendant = RenderObjectPostOrderTraversal::Next(*descendant, &current);
      }
      return static_cast<T *>(descendant);
    }

    template <typename T>
    KRYS_NODISCARD static T *Next(const RenderObject &current, const RenderObject *stayWithin) noexcept
    {
      auto *descendant = RenderObjectPostOrderTraversal::Next(current, stayWithin);
      while (descendant && !IsRendererOfType<T>(*descendant))
      {
        descendant = RenderObjectPostOrderTraversal::Next(*descendant, stayWithin);
      }
      return static_cast<T *>(descendant);
    }
  };

  /// @brief Iterates over renderers of type @p T (which may be const) within a root.
  template <typename T>
  class RenderIterator
  {
  private:
    const RenderElement *_root;
    T *_current;

  public:
    explicit RenderIterator(const RenderElement *root) noexcept : _root(root), _current(nullptr)
    {
    }

    RenderIterator(const RenderElement *root, T *current) noexcept : _root(root), _current(current)
    {
    }

    KRYS_NODISCARD T &operator*() const noexcept
    {
      krys_debug_assert(_current);
      return *_current;
    }

    KRYS_NODISCARD T *operator->() const noexcept
    {
      krys_debug_assert(_current);
      return _current;
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _current;
    }

    KRYS_NODISCARD bool operator==(const RenderIterator &other) const noexcept
    {
      krys_debug_assert(_root == other._root);
      return _current == other._current;
    }

    RenderIterator &TraverseNext() noexcept
    {
      krys_debug_assert(_current);
      _current = RenderTraversal::Next<T>(*_current, _root);
      return *this;
    }

    RenderIterator &TraverseNextSibling() noexcept
    {
      krys_debug_assert(_current);
      _current = RenderTraversal::NextSibling<T>(*_current);
      return *this;
    }

    RenderIterator &TraverseNextSkippingChildren() noexcept
    {
      krys_debug_assert(_current);
      _current = RenderObjectTraversal::NextSkippingChildren(*_current, _root);
      return *this;
    }

    RenderIterator &TraversePreviousSibling() noexcept
    {
      krys_debug_assert(_current);
      _current = RenderTraversal::PreviousSibling<T>(*_current);
      return *this;
    }

    RenderIterator &TraverseAncestor() noexcept
    {
      krys_debug_assert(_current);
      krys_debug_assert(_current != _root);
      _current = RenderTraversal::FindAncestorOfType<T>(*_current);
      return *this;
    }
  };

  template <typename T>
  using RenderConstIterator = RenderIterator<const T>;

  /// @brief Iterates over renderers of type @p T (which may be const) within a root, children first.
  template <typename T>
  class RenderPostOrderIterator
  {
  private:
    const RenderElement *_root;
    T *_current;

  public:
    explicit RenderPostOrderIterator(const RenderElement *root) noexcept : _root(root), _current(nullptr)
    {
    }

    RenderPostOrderIterator(const RenderElement *root, T *current) noexcept : _root(root), _current(current)
    {
    }

    KRYS_NODISCARD T &operator*() const noexcept
    {
      krys_debug_assert(_current);
      return *_current;
    }

    KRYS_NODISCARD T *operator->() const noexcept
    {
      krys_debug_assert(_current);
      return _current;
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _current;
    }

    KRYS_NODISCARD bool operator==(const RenderPostOrderIterator &other) const noexcept
    {
      krys_debug_assert(_root == other._root);
      return _current == other._current;
    }

    RenderPostOrderIterator &TraverseNext() noexcept
    {
      krys_debug_assert(_current);
      _current = RenderPostOrderTraversal::Next<T>(*_current, _root);
      return *this;
    }
  };

  template <typename T>
  using RenderPostOrderConstIterator = RenderPostOrderIterator<const T>;

  /// @brief `RenderElement`, const if @p T is.
  template <typename T>
  using RenderElementFor = conditional_t<IsConst<T>, const RenderElement, RenderElement>;

  /// @brief `RenderObject`, const if @p T is.
  template <typename T>
  using RenderObjectFor = conditional_t<IsConst<T>, const RenderObject, RenderObject>;
}
