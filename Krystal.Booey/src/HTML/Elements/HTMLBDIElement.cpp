#include "Krystal.Booey/HTML/Elements/HTMLBDIElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo::html
{
  HTMLBDIElement::HTMLBDIElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::BDI)
  {
    SetSelfOrPrecedingNodesAffectDirAuto(true);
    document.SetIsDirAttributeDirty();
  }
}
