#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class HTMLCollection;
  class ContainerNode;

  class HTMLCollectionAlgorithms
  {
  public:
    KRYS_NODISCARD static Ref<HTMLCollection> ElementsByTagName(ContainerNode &root,
                                                                DOMStringAtom qualifiedName) noexcept;

    KRYS_NODISCARD static Ref<HTMLCollection>
      ElementsByTagNameNS(ContainerNode &root, DOMStringAtom namespaceUri, DOMStringAtom localName) noexcept;

    KRYS_NODISCARD static Ref<HTMLCollection> ElementsByClassName(ContainerNode &root,
                                                                  DOMStringAtom classNames) noexcept;
  };
}