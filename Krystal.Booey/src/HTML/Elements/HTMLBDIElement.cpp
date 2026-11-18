#include "Krystal.Booey/HTML/Elements/HTMLBDIElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo
{
  HTMLBDIElement::HTMLBDIElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::BDI)
  {
    SetSelfOrPrecedingNodesAffectDirAuto(true);
    document.SetIsDirAttributeDirty();
  }
}
