#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.HTML/Types/SmallTextNodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CharacterData;
  class ContainerNode;
  class Node;
  class Text;

  /// @brief Implementations of the CharacterData/Text node interface algorithms.
  /// @see https://dom.spec.whatwg.org/#interface-characterdata
  /// @see https://dom.spec.whatwg.org/#interface-text
  class TextAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-cd-replace
    KRYS_NODISCARD static ExceptionOr<void> Replace(CharacterData &node, size_t offset, size_t count,
                                                    DOMString &&data) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-cd-substring
    KRYS_NODISCARD static ExceptionOr<DOMString> Substring(const CharacterData &node, size_t offset,
                                                           size_t count) noexcept;

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(RawPtr<const Node> node) noexcept;

    /// @see https://dom.spec.whatwg.org/#contiguous-text-nodes
    KRYS_NODISCARD static SmallTextNodeList ContiguousTextNodes(Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#contiguous-text-nodes
    KRYS_NODISCARD static SmallConstTextNodeList ContiguousTextNodes(const Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#contiguous-exclusive-text-nodes
    KRYS_NODISCARD static SmallTextNodeList ContiguousExclusiveTextNodes(Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#contiguous-exclusive-text-nodes
    KRYS_NODISCARD static SmallConstTextNodeList ContiguousExclusiveTextNodes(const Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-child-text-content
    KRYS_NODISCARD static DOMString ChildTextContent(const ContainerNode &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-descendant-text-content
    KRYS_NODISCARD static DOMString DescendantTextContent(const ContainerNode &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-text-split
    KRYS_NODISCARD static ExceptionOr<Ref<Text>> Split(Text &node, size_t offset) noexcept;
  };
}