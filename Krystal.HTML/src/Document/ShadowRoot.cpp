#include "Krystal.HTML/Document/ShadowRoot.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  ShadowRoot::ShadowRoot(Document &document, NodeFlags flags) noexcept
      : DocumentFragment(document, flags | NodeFlags::IsShadowRoot)
  {
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ShadowRoot)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsShadowRootNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
