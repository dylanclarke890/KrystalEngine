#include "Krystal.Booey/DOM/MutationObservers/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"

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
