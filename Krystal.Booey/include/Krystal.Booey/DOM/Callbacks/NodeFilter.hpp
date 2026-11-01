#pragma once

#include "Krystal.Booey/DOM/Enums/FilterResult.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @brief Decides which nodes a NodeIterator or TreeWalker returns.
  /// @see https://dom.spec.whatwg.org/#interface-nodefilter
  class NodeFilter : public RefCounted<NodeFilter>
  {
  public:
    virtual ~NodeFilter() noexcept = default;

    /// @brief Whether to accept, reject or skip the node.
    KRYS_NODISCARD virtual ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept = 0;
  };
}