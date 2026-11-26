#include "Krystal.Booey/Html/Elements/HTMLBDIElement.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"

namespace krys::boo
{
  HTMLBDIElement::HTMLBDIElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::BDI)
  {
    SetSelfOrPrecedingNodesAffectDirAuto(true);
    document.SetIsDirAttributeDirty();
  }
}
