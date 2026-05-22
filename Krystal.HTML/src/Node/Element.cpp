#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/HTMLCollectionAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Mixins/ChildNode.hpp"
#include "Krystal.HTML/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.HTML/Mixins/ParentNode.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NamedNodeMap.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/RareData/ElementRareData.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  Element::Element(Document &document, NodeFlag nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags)
  {
  }

  Element::Element(Document &document, const QualifiedName &name, NodeFlag nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags), _qualifiedName(name)
  {
  }

#pragma region Element

  DOMString Element::TagName() const noexcept
  {
    auto qualifiedName = _qualifiedName.Name();

    if (NamespaceURI() == Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = Krys::Text::ToASCIIUppercase(qualifiedName);
    }

    return qualifiedName;
  }

  void Element::Id(DOMString &&id) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"id", Krys::Move(id));
  }

  DOMString Element::Id() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"id");
  }

  void Element::ClassName(DOMString &&className) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"class", Krys::Move(className));
  }

  DOMString Element::ClassName() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"class");
  }

  DOMTokenList &Element::ClassList() noexcept
  {
    if (_domTokenList == nullptr)
    {
      _domTokenList = CreateUnique<DOMTokenList>(*this, u8"class");
    }

    return *_domTokenList;
  }

  void Element::Slot(DOMString &&slot) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"slot", Krys::Move(slot));
  }

  DOMString Element::Slot() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"slot");
  }

  bool Element::HasAttributes() const noexcept
  {
    return !_attributes.empty();
  }

  NamedNodeMap &Element::Attributes() noexcept
  {
    if (_namedNodeMap == nullptr)
    {
      _namedNodeMap = CreateUnique<NamedNodeMap>(*this);
    }

    return *_namedNodeMap;
  }

  List<DOMString> Element::GetAttributeNames() const noexcept
  {
    return std::ranges::views::transform(_attributes, [](const Ref<Attr> &attr) { return attr->Name(); })
           | std::ranges::to<List<DOMString>>();
  }

  Maybe<DOMString> Element::GetAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    RawPtr<Attr> attr = ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
    if (attr == nullptr)
    {
      return Null;
    }

    return attr->Value();
  }

  Maybe<DOMString> Element::GetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    RawPtr<Attr> attr = ElementAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
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

    if (NamespaceURI() == Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = Krys::Text::ToASCIILowercase(qualifiedName.View());
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with qualifiedName, null, this, and value.

    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    if (it != _attributes.end())
    {
      ElementAlgorithms::ChangeAttribute(**it, Krys::Move(value));
    }
    else
    {
      auto attr = AdoptRef<Attr>(
        *new Attr(NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), qualifiedName},
                  Krys::Move(value)));
      ElementAlgorithms::AppendAttribute(*attr, *this);
    }

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
    ElementAlgorithms::SetAttributeValue(*this, qName.LocalName, std::move(value), qName.Prefix,
                                         qName.NamespaceURI);

    return {};
  }

  void Element::RemoveAttribute(DOMStringAtom qualifiedName) noexcept
  {
    ElementAlgorithms::RemoveAttributeByName(qualifiedName, *this);
  }

  void Element::RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept
  {
    ElementAlgorithms::RemoveAttributeByNamespace(namespaceURI, localName, *this);
  }

  ExceptionOr<bool> Element::ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (NamespaceURI() == Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = Krys::Text::ToASCIILowercase(qualifiedName.View());
    }

    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });

    if (it == _attributes.end())
    {
      if (!force.has_value() || force.value())
      {
        auto attr = AdoptRef<Attr>(*new Attr(
          NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), qualifiedName}, u8""));
        ElementAlgorithms::AppendAttribute(*attr, *this);
        return true;
      }

      return false;
    }

    if (!force.has_value() || !force.value())
    {
      ElementAlgorithms::RemoveAttribute(**it);
      return false;
    }

    return true;
  }

  bool Element::HasAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    if (NamespaceURI() == Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = Krys::Text::ToASCIILowercase(qualifiedName.View());
    }

    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a->Name() == qualifiedName; });
  }

  bool Element::HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a)
                               { return a->NamespaceURI() == namespaceURI && a->LocalName() == localName; });
  }

  RawPtr<Attr> Element::GetAttributeNode(DOMStringAtom qualifiedName) const noexcept
  {
    return ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
  }

  RawPtr<Attr> Element::GetAttributeNodeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    return ElementAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNode(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNodeNS(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<Ref<Attr>> Element::RemoveAttributeNode(Attr &attr) noexcept
  {
    if (!std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a.get() == &attr; }))
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    ElementAlgorithms::RemoveAttribute(attr);

    return ShareRef(attr);
  }

  ExceptionOr<Ref<ShadowRoot>> Element::AttachShadow(const ShadowRootInit &init) noexcept
  {
    auto registry =
      init.CustomElementRegistry ? init.CustomElementRegistry : NodeDocument().CustomElementRegistry().get();

    if (registry != nullptr && !registry->IsScoped() && registry != NodeDocument().CustomElementRegistry())
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (auto attachResult =
          ShadowRootAlgorithms::AttachShadowRoot(*this, init.Mode, init.Clonable, init.Serializable,
                                                 init.DelegatesFocus, init.SlotAssignment, registry);
        attachResult.HasException())
    {
      return attachResult.ReleaseException();
    }

    return ShareRef(*_shadowRoot);
  }

  RefPtr<ShadowRoot> Element::ShadowRoot() const noexcept
  {
    if (!_shadowRoot || _shadowRoot->Mode() == ShadowRootMode::Closed)
    {
      return nullptr;
    }

    return _shadowRoot;
  }

  RefPtr<CustomElementRegistry> Element::CustomElementRegistry() const noexcept
  {
    return _customElementRegistry;
  }

  RefPtr<Element> Element::Closest(DOMStringView selectors) noexcept
  {
    // TODO(impl): CSS-SELECTORS
    return nullptr;
  }

  bool Element::Matches(DOMStringView selectors) const noexcept
  {
    // TODO(impl): CSS-SELECTORS
    return false;
  }

  Ref<HTMLCollection> Element::GetElementsByTagName(DOMStringAtom qualifiedName) noexcept
  {
    return HTMLCollectionAlgorithms::ElementsByTagName(*this, qualifiedName);
  }

  Ref<HTMLCollection> Element::GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                      DOMStringAtom localName) noexcept
  {
    return HTMLCollectionAlgorithms::ElementsByTagNameNS(*this, namespaceURI, localName);
  }

  Ref<HTMLCollection> Element::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    return HTMLCollectionAlgorithms::ElementsByClassName(*this, classNames);
  }

  ExceptionOr<RawPtr<Element>> Element::InsertAdjacentElement(InsertAdjacentWhere where,
                                                              Element &element) noexcept
  {
    if (auto result = ElementAlgorithms::InsertAdjacent(*this, where, element); result.HasException())
    {
      return result.ReleaseException();
    }

    return &element;
  }

  ExceptionOr<void> Element::InsertAdjacentText(InsertAdjacentWhere where, DOMString &&data) noexcept
  {
    auto textNode = CreateRef<Text>(NodeDocument(), Krys::Move(data));
    if (auto result = ElementAlgorithms::InsertAdjacent(*this, where, *textNode); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

#pragma endregion

#pragma region Node

  DOMString Element::NodeName() const noexcept
  {
    return TagName();
  }

  Maybe<DOMString> Element::TextContent() const noexcept
  {
    return TreeQueries::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::TextContent(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(Krys::Move(value), *this);
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> Element::Before(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> Element::After(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    return Mixins::ChildNode::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return Mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return Mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return Mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

  RefPtr<Element> Element::NextElementSibling() noexcept
  {
    return Mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return _elementRareData->Children(*this);
  }

  RefPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return Mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> Element::FirstElementChild() noexcept
  {
    return Mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> Element::LastElementChild() const noexcept
  {
    return Mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> Element::LastElementChild() noexcept
  {
    return Mixins::ParentNode::LastElementChild(*this);
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return Mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Element::Append(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Element::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    return Mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Element::QuerySelector(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region Slottable

  RawPtr<HTMLSlotElement> Element::AssignedSlot() noexcept
  {
    return SlotAlgorithms::FindSlot(*this, true);
  }

#pragma endregion
}