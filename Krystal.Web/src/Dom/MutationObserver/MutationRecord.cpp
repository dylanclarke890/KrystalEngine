#include "Krystal.Web/Dom/MutationObserver/MutationRecord.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"

namespace krys::web::dom
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
