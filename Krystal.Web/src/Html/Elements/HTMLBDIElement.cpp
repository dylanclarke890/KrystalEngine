#include "Krystal.Web/Html/Elements/HTMLBDIElement.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"

namespace krys::web
{
  HTMLBDIElement::HTMLBDIElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::BDI)
  {
    SetSelfOrPrecedingNodesAffectDirAuto(true);
    document.SetIsDirAttributeDirty();
  }
}
