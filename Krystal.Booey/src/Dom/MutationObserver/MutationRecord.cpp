#include "Krystal.Booey/Dom/MutationObserver/MutationRecord.hpp"
#include "Krystal.Booey/Dom/Nodes/Node.hpp"

namespace krys::boo::dom
{
  RefPtr<Node> MutationRecord::PreviousSibling() const noexcept
  {
    return nullptr;
  }

  RefPtr<Node> MutationRecord::NextSibling() const noexcept
  {
    return nullptr;
  }
}
