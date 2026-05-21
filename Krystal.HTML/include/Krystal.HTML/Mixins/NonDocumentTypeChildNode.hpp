#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CharacterData;
  class Element;
}

namespace Krys::HTML::Mixins
{
  /// @see https://dom.spec.whatwg.org/#nondocumenttypechildnode
  class NonDocumentTypeChildNode
  {
  public:
    /// Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD static RefPtr<Element> PreviousElementSibling(CharacterData &node) noexcept;

    /// Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD static RefPtr<const Element> PreviousElementSibling(const CharacterData &node) noexcept;

    /// Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD static RefPtr<Element> PreviousElementSibling(Element &node) noexcept;

    /// Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD static RefPtr<const Element> PreviousElementSibling(const Element &node) noexcept;

    /// Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD static RefPtr<Element> NextElementSibling(CharacterData &node) noexcept;

    /// Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD static RefPtr<const Element> NextElementSibling(const CharacterData &node) noexcept;

    /// Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD static RefPtr<Element> NextElementSibling(Element &node) noexcept;

    /// Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD static RefPtr<const Element> NextElementSibling(const Element &node) noexcept;
  };
}