#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys
{
  /// @brief A red-black tree, a balanced binary tree supporting O(lg n) insertion, deletion and queries of
  /// comparable elements. The same element may be inserted multiple times.
  /// @tparam T The element type: default constructible, movable, and comparable with `<` and `==`.
  /// @tparam TNodeUpdater Augments the tree: `TNodeUpdater::update(node)` recomputes a node's summary
  /// information from its children's and returns whether it changed. It's called in an order that only needs
  /// the children to be up to date (an interval tree uses it to keep the maximum endpoint of each subtree).
  /// @note The design of this red-black tree comes from Cormen, Leiserson, and Rivest, _Introduction to
  /// Algorithms_, MIT Press, 1990.
  template <typename T, typename TNodeUpdater>
  class RedBlackTree
  {
    KRYS_NON_COPYABLE(RedBlackTree);

  protected:
    enum class Color : bool
    {
      Red,
      Black
    };

    class Node
    {
      KRYS_NON_COPY_MOVABLE(Node);

    private:
      Node *_left {nullptr};
      Node *_right {nullptr};
      Node *_parent {nullptr};
      Color _color {Color::Red};
      T _data;

    public:
      explicit Node(T &&data) noexcept : _data(krys::move(data))
      {
      }

      KRYS_NODISCARD Color GetColor() const noexcept
      {
        return _color;
      }
      void SetColor(Color color) noexcept
      {
        _color = color;
      }

      KRYS_NODISCARD T &Data() noexcept
      {
        return _data;
      }
      KRYS_NODISCARD const T &Data() const noexcept
      {
        return _data;
      }

      void MoveDataFrom(Node &src) noexcept
      {
        _data = krys::move(src._data);
      }

      KRYS_NODISCARD Node *Left() const noexcept
      {
        return _left;
      }
      void SetLeft(Node *node) noexcept
      {
        _left = node;
      }

      KRYS_NODISCARD Node *Right() const noexcept
      {
        return _right;
      }
      void SetRight(Node *node) noexcept
      {
        _right = node;
      }

      KRYS_NODISCARD Node *Parent() const noexcept
      {
        return _parent;
      }
      void SetParent(Node *node) noexcept
      {
        _parent = node;
      }
    };

  private:
    Node *_root {nullptr};

  public:
    RedBlackTree() noexcept = default;

    ~RedBlackTree() noexcept
    {
      clear();
    }

    void clear() noexcept
    {
      if (!_root)
      {
        return;
      }
      Node *next;
      for (Node *node = TreeMinimum(_root); node; node = next)
      {
        next = TreeSuccessorInPostOrder(node);
        delete node;
      }
      _root = nullptr;
    }

    void add(const T &data) noexcept
    {
      add(T {data});
    }

    void add(T &&data) noexcept
    {
      InsertNode(new Node(krys::move(data)));
    }

    /// @brief Removes one copy of @p data.
    /// @return Whether @p data was found in the tree.
    bool remove(const T &data) noexcept
    {
      Node *node = TreeSearch(data);
      if (node)
      {
        DeleteNode(node);
        return true;
      }
      return false;
    }

    KRYS_NODISCARD bool contains(const T &data) const noexcept
    {
      return TreeSearch(data);
    }

    KRYS_NODISCARD bool isEmpty() const noexcept
    {
      return !_root;
    }

    /// @brief Whether the tree is a valid red-black tree: red nodes have black children, and every path to a
    /// leaf has the same number of black nodes.
    KRYS_NODISCARD bool checkInvariants() const noexcept
    {
      int32 blackCount;
      return CheckInvariantsFromNode(_root, blackCount);
    }

  protected:
    // Returns the root of the tree, which is needed by some subclasses.
    KRYS_NODISCARD Node *Root() const noexcept
    {
      return _root;
    }

  private:
    static bool UpdateNode(Node &node) noexcept
    {
      return TNodeUpdater::Update(node);
    }

    KRYS_NODISCARD Node *TreeSearch(const T &data) const noexcept
    {
      for (auto *current = _root; current;)
      {
        if (current->Data() == data)
        {
          return current;
        }
        if (data < current->Data())
        {
          current = current->Left();
        }
        else
        {
          current = current->Right();
        }
      }
      return nullptr;
    }

    void TreeInsert(Node *z) noexcept
    {
      Node *y = nullptr;
      Node *x = _root;
      while (x)
      {
        y = x;
        if (z->Data() < x->Data())
        {
          x = x->Left();
        }
        else
        {
          x = x->Right();
        }
      }
      z->SetParent(y);
      if (!y)
      {
        _root = z;
      }
      else
      {
        if (z->Data() < y->Data())
        {
          y->SetLeft(z);
        }
        else
        {
          y->SetRight(z);
        }
      }
    }

    // Finds the node following the given one in sequential ordering of
    // their data, or null if none exists.
    KRYS_NODISCARD static Node *TreeSuccessor(Node *x) noexcept
    {
      if (x->Right())
      {
        return TreeMinimum(x->Right());
      }
      Node *y = x->Parent();
      while (y && x == y->Right())
      {
        x = y;
        y = y->Parent();
      }
      return y;
    }

    // Finds the minimum element in the sub-tree rooted at the given node.
    KRYS_NODISCARD static Node *TreeMinimum(Node *x) noexcept
    {
      while (x->Left())
      {
        x = x->Left();
      }
      return x;
    }

    KRYS_NODISCARD static Node *TreeSuccessorInPostOrder(Node *x) noexcept
    {
      Node *y = x->Parent();
      if (y && x == y->Left() && y->Right())
      {
        return TreeMinimum(y->Right());
      }
      return y;
    }

    // Left-rotates the subtree rooted at x.
    // Returns the new root of the subtree (x's right child).
    Node *LeftRotate(Node *x) noexcept
    {
      // Set y.
      Node *y = x->Right();

      // Turn y's left subtree into x's right subtree.
      x->SetRight(y->Left());
      if (y->Left())
      {
        y->Left()->SetParent(x);
      }

      // Link x's parent to y.
      y->SetParent(x->Parent());
      if (!x->Parent())
      {
        _root = y;
      }
      else
      {
        if (x == x->Parent()->Left())
        {
          x->Parent()->SetLeft(y);
        }
        else
        {
          x->Parent()->SetRight(y);
        }
      }

      // Put x on y's left.
      y->SetLeft(x);
      x->SetParent(y);

      // Update nodes lowest to highest.
      UpdateNode(*x);
      UpdateNode(*y);
      return y;
    }

    static void PropagateUpdates(Node *start) noexcept
    {
      while (start && UpdateNode(*start))
      {
        start = start->Parent();
      }
    }

    // Right-rotates the subtree rooted at y.
    // Returns the new root of the subtree (y's left child).
    Node *RightRotate(Node *y) noexcept
    {
      // Set x.
      Node *x = y->Left();

      // Turn x's right subtree into y's left subtree.
      y->SetLeft(x->Right());
      if (x->Right())
      {
        x->Right()->SetParent(y);
      }

      // Link y's parent to x.
      x->SetParent(y->Parent());
      if (!y->Parent())
      {
        _root = x;
      }
      else
      {
        if (y == y->Parent()->Left())
        {
          y->Parent()->SetLeft(x);
        }
        else
        {
          y->Parent()->SetRight(x);
        }
      }

      // Put y on x's right.
      x->SetRight(y);
      y->SetParent(x);

      // Update nodes lowest to highest.
      UpdateNode(*y);
      UpdateNode(*x);
      return x;
    }

    // Inserts the given node into the tree.
    void InsertNode(Node *x) noexcept
    {
      TreeInsert(x);
      x->SetColor(Color::Red);
      UpdateNode(*x);

      // The node from which to start propagating updates upwards.
      Node *updateStart = x->Parent();

      while (x != _root && x->Parent()->GetColor() == Color::Red)
      {
        if (x->Parent() == x->Parent()->Parent()->Left())
        {
          Node *y = x->Parent()->Parent()->Right();
          if (y && y->GetColor() == Color::Red)
          {
            // Case 1
            x->Parent()->SetColor(Color::Black);
            y->SetColor(Color::Black);
            x->Parent()->Parent()->SetColor(Color::Red);
            UpdateNode(*x->Parent());
            x = x->Parent()->Parent();
            UpdateNode(*x);
            updateStart = x->Parent();
          }
          else
          {
            if (x == x->Parent()->Right())
            {
              // Case 2
              x = x->Parent();
              LeftRotate(x);
            }
            // Case 3
            x->Parent()->SetColor(Color::Black);
            x->Parent()->Parent()->SetColor(Color::Red);
            Node *newSubTreeRoot = RightRotate(x->Parent()->Parent());
            updateStart = newSubTreeRoot->Parent();
          }
        }
        else
        {
          // Same as "then" clause with "right" and "left" exchanged.
          Node *y = x->Parent()->Parent()->Left();
          if (y && y->GetColor() == Color::Red)
          {
            // Case 1
            x->Parent()->SetColor(Color::Black);
            y->SetColor(Color::Black);
            x->Parent()->Parent()->SetColor(Color::Red);
            UpdateNode(*x->Parent());
            x = x->Parent()->Parent();
            UpdateNode(*x);
            updateStart = x->Parent();
          }
          else
          {
            if (x == x->Parent()->Left())
            {
              // Case 2
              x = x->Parent();
              RightRotate(x);
            }
            // Case 3
            x->Parent()->SetColor(Color::Black);
            x->Parent()->Parent()->SetColor(Color::Red);
            Node *newSubTreeRoot = LeftRotate(x->Parent()->Parent());
            updateStart = newSubTreeRoot->Parent();
          }
        }
      }

      PropagateUpdates(updateStart);

      _root->SetColor(Color::Black);
    }

    // Restores the red-black property to the tree after splicing out
    // a node. Note that x may be null, which is why xParent must be supplied.
    void DeleteFixup(Node *x, Node *xParent) noexcept
    {
      while (x != _root && (!x || x->GetColor() == Color::Black))
      {
        if (x == xParent->Left())
        {
          // Note: the text points out that w can not be null.
          // The reason is not obvious from simply looking at
          // the code; it comes about from the properties of the
          // red-black tree.
          Node *w = xParent->Right();
          krys_debug_assert(w); // x's sibling should not be null.
          if (w->GetColor() == Color::Red)
          {
            // Case 1
            w->SetColor(Color::Black);
            xParent->SetColor(Color::Red);
            LeftRotate(xParent);
            w = xParent->Right();
          }
          if ((!w->Left() || w->Left()->GetColor() == Color::Black)
              && (!w->Right() || w->Right()->GetColor() == Color::Black))
          {
            // Case 2
            w->SetColor(Color::Red);
            x = xParent;
            xParent = x->Parent();
          }
          else
          {
            if (!w->Right() || w->Right()->GetColor() == Color::Black)
            {
              // Case 3
              w->Left()->SetColor(Color::Black);
              w->SetColor(Color::Red);
              RightRotate(w);
              w = xParent->Right();
            }
            // Case 4
            w->SetColor(xParent->GetColor());
            xParent->SetColor(Color::Black);
            if (w->Right())
            {
              w->Right()->SetColor(Color::Black);
            }
            LeftRotate(xParent);
            x = _root;
            xParent = x->Parent();
          }
        }
        else
        {
          // Same as "then" clause with "right" and "left"
          // exchanged.

          // Note: the text points out that w can not be null.
          // The reason is not obvious from simply looking at
          // the code; it comes about from the properties of the
          // red-black tree.
          Node *w = xParent->Left();
          krys_debug_assert(w); // x's sibling should not be null.
          if (w->GetColor() == Color::Red)
          {
            // Case 1
            w->SetColor(Color::Black);
            xParent->SetColor(Color::Red);
            RightRotate(xParent);
            w = xParent->Left();
          }
          if ((!w->Right() || w->Right()->GetColor() == Color::Black)
              && (!w->Left() || w->Left()->GetColor() == Color::Black))
          {
            // Case 2
            w->SetColor(Color::Red);
            x = xParent;
            xParent = x->Parent();
          }
          else
          {
            if (!w->Left() || w->Left()->GetColor() == Color::Black)
            {
              // Case 3
              w->Right()->SetColor(Color::Black);
              w->SetColor(Color::Red);
              LeftRotate(w);
              w = xParent->Left();
            }
            // Case 4
            w->SetColor(xParent->GetColor());
            xParent->SetColor(Color::Black);
            if (w->Left())
            {
              w->Left()->SetColor(Color::Black);
            }
            RightRotate(xParent);
            x = _root;
            xParent = x->Parent();
          }
        }
      }
      if (x)
      {
        x->SetColor(Color::Black);
      }
    }

    // Deletes the given node from the tree. Note that this
    // particular node may not actually be removed from the tree;
    // instead, another node might be removed and its contents
    // copied into z.
    void DeleteNode(Node *z) noexcept
    {
      // Y is the node to be unlinked from the tree.
      Node *y;
      if (!z->Left() || !z->Right())
      {
        y = z;
      }
      else
      {
        y = TreeSuccessor(z);
      }

      // Y is guaranteed to be non-null at this point.
      Node *x;
      if (y->Left())
      {
        x = y->Left();
      }
      else
      {
        x = y->Right();
      }

      // X is the child of y which might potentially replace y in
      // the tree. X might be null at this point.
      Node *xParent;
      if (x)
      {
        x->SetParent(y->Parent());
        xParent = x->Parent();
      }
      else
      {
        xParent = y->Parent();
      }
      if (!y->Parent())
      {
        _root = x;
      }
      else
      {
        if (y == y->Parent()->Left())
        {
          y->Parent()->SetLeft(x);
        }
        else
        {
          y->Parent()->SetRight(x);
        }
      }
      if (y != z)
      {
        z->MoveDataFrom(*y);
        // This node has changed location in the tree and must be updated.
        UpdateNode(*z);
        // The parent and its parents may now be out of date.
        PropagateUpdates(z->Parent());
      }

      // If we haven't already updated starting from xParent, do so now.
      if (xParent && xParent != y && xParent != z)
      {
        PropagateUpdates(xParent);
      }
      if (y->GetColor() == Color::Black)
      {
        DeleteFixup(x, xParent);
      }

      delete y;
    }

    // Returns in the "blackCount" parameter the number of black
    // children along all paths to all leaves of the given node.
    bool CheckInvariantsFromNode(Node *node, int32 &blackCount) const noexcept
    {
      // Base case is a leaf node.
      if (!node)
      {
        blackCount = 1;
        return true;
      }

      // Every leaf (or null) is black.

      if (node->GetColor() == Color::Red)
      {
        // Both of its children are black.
        if (!((!node->Left() || node->Left()->GetColor() == Color::Black)))
        {
          return false;
        }
        if (!((!node->Right() || node->Right()->GetColor() == Color::Black)))
        {
          return false;
        }
      }

      // Every simple path to a leaf node contains the same number of black nodes.
      int32 leftCount = 0, rightCount = 0;
      bool leftValid = CheckInvariantsFromNode(node->Left(), leftCount);
      bool rightValid = CheckInvariantsFromNode(node->Right(), rightCount);
      if (!leftValid || !rightValid)
      {
        return false;
      }
      blackCount = leftCount + (node->GetColor() == Color::Black ? 1 : 0);
      return leftCount == rightCount;
    }
  };
}
