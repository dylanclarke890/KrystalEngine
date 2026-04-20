#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ChildNodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ElementAttributeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/Algorithms/SlotAssignmentAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/ElementRareData.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/HTMLCollection.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"

namespace Krys::HTML
{
  Element::Element(Document &document, NodeFlag nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags | NodeFlag::IsElement)
  {
  }

#pragma region Element

  void Element::Id(DOMString &&id) noexcept
  {
    ElementAttributeAlgorithms::SetAttributeValue(*this, u8"id", Krys::Move(id));
  }

  DOMString Element::Id() const noexcept
  {
    return ElementAttributeAlgorithms::GetAttributeValue(*this, u8"id");
  }

  void Element::ClassName(DOMString &&className) noexcept
  {
    ElementAttributeAlgorithms::SetAttributeValue(*this, u8"class", Krys::Move(className));
  }

  DOMString Element::ClassName() const noexcept
  {
    return ElementAttributeAlgorithms::GetAttributeValue(*this, u8"class");
  }

  KRYS_NODISCARD DOMTokenList Element::ClassList() noexcept
  {
    return DOMTokenList(*this, u8"class");
  }

  void Element::Slot(DOMString &&slot) noexcept
  {
    ElementAttributeAlgorithms::SetAttributeValue(*this, u8"slot", Krys::Move(slot));
  }

  DOMString Element::Slot() const noexcept
  {
    return ElementAttributeAlgorithms::GetAttributeValue(*this, u8"slot");
  }

  bool Element::HasAttributes() const noexcept
  {
    return !_attributes.empty();
  }

  // NamedNodeMap &Element::Attributes() const noexcept
  //{
  // }

  List<DOMString> Element::GetAttributeNames() const noexcept
  {
    return std::ranges::views::transform(_attributes, [](const Ref<Attr> &attr) { return attr->Name(); })
           | std::ranges::to<List<DOMString>>();
  }

  Maybe<DOMString> Element::GetAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    RawPtr<Attr> attr = ElementAttributeAlgorithms::GetAttributeByName(qualifiedName, *this);
    if (attr == nullptr)
    {
      return std::nullopt;
    }

    return attr->Value();
  }

  Maybe<DOMString> Element::GetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    RawPtr<Attr> attr = ElementAttributeAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
    if (attr == nullptr)
    {
      return std::nullopt;
    }

    return attr->Value();
  }

  ExceptionOr<void> Element::SetAttribute(DOMStringAtom qualifiedName, DOMString &&value) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    // TODO(impl): If this is in the HTML namespace and its node document is an HTML document, then set
    // qualifiedName to qualifiedName in ASCII lowercase.

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant attribute value with
    // qualifiedName, null, this, and value.

    // TODO(impl): Implement this method

    return {};
  }

  ExceptionOr<void> Element::SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                            DOMString &&value) noexcept
  {
    auto validateAndExtractResult =
      NameValidation::ValidateAndExtract(namespaceURI, qualifiedName, ValidateAndExtractContext::Attribute);

    if (validateAndExtractResult.HasException())
    {
      return validateAndExtractResult.ReleaseException();
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with localName, namespace, this, and value.

    const auto &qName = validateAndExtractResult.Value();
    ElementAttributeAlgorithms::SetAttributeValue(*this, qName.LocalName, std::move(value), qName.Prefix,
                                                  qName.NamespaceURI);

    return {};
  }

  void Element::RemoveAttribute(DOMStringAtom qualifiedName) noexcept
  {
    ElementAttributeAlgorithms::RemoveAttributeByName(qualifiedName, *this);
  }

  void Element::RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept
  {
    ElementAttributeAlgorithms::RemoveAttributeByNamespace(namespaceURI, localName, *this);
  }

  bool Element::ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept
  {
    // TODO(impl): Implement this method
    return false;
  }

  bool Element::HasAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    // TODO(impl): If this is in the HTML namespace and its node document is an HTML document, then set
    // qualifiedName to qualifiedName in ASCII lowercase.

    return std::ranges::any_of(_attributes,
                               [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
  }

  bool Element::HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    return std::ranges::any_of(
      _attributes, [&](const Ref<Attr> &attr)
      { return attr->NamespaceURI() == namespaceURI && attr->LocalName() == localName; });
  }

  ExceptionOr<Ref<Attr>> Element::RemoveAttributeNode(Attr &attribute) noexcept
  {
    // TODO(impl): Implement this method
    return Exception {ExceptionCode::NotSupportedError};
  }

#pragma endregion

#pragma region Node

  DOMString Element::NodeName() const noexcept
  {
    // TODO(impl): Return the qualified name
    return u8"element";
  }

  DOMString Element::TextContent() const noexcept
  {
    return TreeQueries::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::SetTextContent(DOMString &&value) noexcept
  {
    return ExceptionOr<void>();
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> Element::Before(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::Before(*this, nodes);
  }

  ExceptionOr<void> Element::After(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::After(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    return ChildNodeAlgorithms::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(*this));
  }

  RefPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(*this));
  }

  RefPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(*this));
  }

  RefPtr<Element> Element::NextElementSibling() noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(*this));
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return _elementRareData->Children(*this);
  }

  RefPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<Element> Element::FirstElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<const Element> Element::LastElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  RefPtr<Element> Element::LastElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::PreInsert(*node.Value(), *this, FirstChild());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::Append(const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::Append(*node.Value(), *this); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::ReplaceAll(node.Value().get(), *this); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    if (&node == refChild)
    {
      refChild = node.NextSibling();
    }

    return MutationAlgorithms::Move(node, *this, refChild);
  }

  ExceptionOr<RefPtr<Element>> Element::QuerySelector(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

#pragma endregion

#pragma region Slottable

  RawPtr<HTMLSlotElement> Element::AssignedSlot() noexcept
  {
    return SlotAssignmentAlgorithms::FindSlot(*this, true);
  }

#pragma endregion
}