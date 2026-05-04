#pragma once

#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ParentNodeAlgorithms
  {
  public:
    KRYS_NODISCARD static ExceptionOr<void> Prepend(ContainerNode &parent,
                                                    const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> Append(ContainerNode &parent,
                                                   const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> ReplaceChildren(ContainerNode &parent,
                                                            const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> MoveBefore(ContainerNode &parent, Node &node,
                                                       RawPtr<Node> refChild) noexcept;

    KRYS_NODISCARD static ExceptionOr<RefPtr<Element>> QuerySelector(ContainerNode &parent,
                                                                     DOMStringView selectors) noexcept;

    KRYS_NODISCARD static ExceptionOr<Ref<NodeList>> QuerySelectorAll(ContainerNode &parent,
                                                                      DOMStringView selectors) noexcept;
  };
}