#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/Filters/SourceAlpha.hpp"
#include "Krystal.Booey/Graphics/Filters/SourceGraphic.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  /// @brief The graph of a filter's primitives: its nodes, named by their results, and each node's inputs.
  template <typename TNode>
  class SVGFilterGraph
  {
  public:
    using NodeVector = List<Ref<TNode>>;

  protected:
    HashMap<dom::DOMStringAtom, Ref<TNode>> _namedNodes;
    HashMap<Ref<TNode>, NodeVector> _nodeInputs;
    RefPtr<TNode> _lastNode;

  public:
    virtual ~SVGFilterGraph() noexcept = default;

    virtual void AddNamedNode(const dom::DOMStringAtom &id, Ref<TNode> &&) noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<TNode> GetNamedNode(const dom::DOMStringAtom &id) const noexcept = 0;

    /// @brief The nodes named @p names; none if one isn't a node or a source.
    KRYS_NODISCARD Maybe<NodeVector> GetNamedNodes(Span<const dom::DOMStringAtom> names) const noexcept
    {
      NodeVector nodes;

      nodes.reserve(names.size());

      for (const auto &name : names)
      {
        if (auto node = GetNamedNode(name))
        {
          nodes.push_back(node);
        }
        else if (!IsSourceName(name))
        {
          return null;
        }
      }

      return nodes;
    }

    void SetNodeInputs(TNode &node, NodeVector &&inputs) noexcept
    {
      _nodeInputs.insert_or_assign(ShareRef(node), krys::move(inputs));
    }

    KRYS_NODISCARD NodeVector GetNodeInputs(TNode &node) const noexcept
    {
      auto it = _nodeInputs.find(ShareRef(node));
      return it != _nodeInputs.end() ? it->second : NodeVector {};
    }

    KRYS_NODISCARD NodeVector Nodes() const noexcept
    {
      NodeVector nodes;
      nodes.reserve(_nodeInputs.size());
      for (const auto &[node, inputs] : _nodeInputs)
      {
        nodes.push_back(node);
      }
      return nodes;
    }

    KRYS_NODISCARD TNode *LastNode() const noexcept
    {
      return _lastNode.get();
    }

    /// @brief Visits the nodes from the last one, each before its inputs, with its depth.
    /// @return False when the graph is empty or has a cycle.
    template <typename TCallback>
    bool Visit(const TCallback &callback) noexcept
    {
      RefPtr<TNode> lastNode = _lastNode;
      if (!lastNode)
      {
        return false;
      }

      List<Ref<TNode>> stack;
      return Visit(*lastNode, stack, 0, callback);
    }

  protected:
    KRYS_NODISCARD static bool IsSourceName(const dom::DOMStringAtom &name) noexcept
    {
      return name.View() == SourceGraphic::EffectName() || name.View() == SourceAlpha::EffectName();
    }

    template <typename TCallback>
    bool Visit(TNode &node, List<Ref<TNode>> &stack, uint32 level, const TCallback &callback) noexcept
    {
      // A cycle is detected.
      for (const auto &item : stack)
      {
        if (item.get() == &node)
        {
          return false;
        }
      }

      stack.push_back(ShareRef(node));

      callback(node, level);

      for (auto &input : GetNodeInputs(node))
      {
        if (!Visit(*input, stack, level + 1, callback))
        {
          return false;
        }
      }

      krys_debug_assert(!stack.empty());
      krys_debug_assert(stack.back().get() == &node);

      stack.pop_back();
      return true;
    }
  };
}
