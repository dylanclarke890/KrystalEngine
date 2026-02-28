#pragma once

#include "Krystal.HTML/NodeList/NodeList.hpp"
#include "Krystal.HTML/Utils/SmallNodeList.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Node;
  class NodeRareData;

  class ChildNodeList final : public NodeList
  {
    friend class NodeRareData;

  private:
    Ref<Node> _owner;
    mutable SmallNodeList _nodes;
    mutable bool _invalid;

  public:
    explicit ChildNodeList(Node &owner) noexcept;

    KRYS_NODISCARD RawPtr<Node> Item(size_t index) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;

  private:
    void Invalidate() const noexcept
    {
      _invalid = true;
    }

    void BuildCollection() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ChildNodeList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return target.IsChildNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();