#include "Krystal.Booey/HTML/HTMLBDIElement.hpp"
#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::html
{
  HTMLBDIElement::HTMLBDIElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::BDI)
  {
    SetSelfOrPrecedingNodesAffectDirAuto(true);
    document.SetIsDirAttributeDirty();
  }
}
