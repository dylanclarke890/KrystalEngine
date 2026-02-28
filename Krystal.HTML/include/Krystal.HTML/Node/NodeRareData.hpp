#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class Node;
  class EmptyNodeList;
  class ChildNodeList;
  class NodeList;

  class NodeRareData
  {
  private:
    WeakPtr<ChildNodeList> _childNodeList;

  public:
    KRYS_NODISCARD Ref<NodeList> ChildNodes(Node &node) noexcept;

    void InvalidateChildNodes() noexcept;
  };
}