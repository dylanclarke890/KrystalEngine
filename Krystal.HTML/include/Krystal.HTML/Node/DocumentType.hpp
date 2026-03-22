#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class DocumentType final : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DocumentType);

  private:
    DOMString _name;
    DOMString _publicId;
    DOMString _systemId;

  public:
    DocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                 const DOMString &systemId) noexcept;

#pragma region DocumentType

    KRYS_NODISCARD const DOMString &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const DOMString &PublicId() const noexcept
    {
      return _publicId;
    }

    KRYS_NODISCARD const DOMString &SystemId() const noexcept
    {
      return _systemId;
    }

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return _name;
    }

#pragma endregion

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    ExceptionOr<void> Before(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> After(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> ReplaceWith(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> Remove() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::DocumentType)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsDocumentTypeNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();