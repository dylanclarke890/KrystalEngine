#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/Mixins/Slottable.hpp"
#include "Krystal.Booey/DOM/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <ranges>

namespace krys::boo::dom
{
  Element::Element(Document &document, const QualifiedName &name, NodeFlags nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags), _qualifiedName(name)
  {
  }

#pragma region Element

  DOMString Element::TagName() const noexcept
  {
    auto qualifiedName = _qualifiedName.Name();

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIIUpper(qualifiedName);
    }

    return qualifiedName;
  }

  void Element::Id(DOMString &&id) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"id", krys::move(id));
  }

  DOMString Element::Id() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"id");
  }

  bool Element::HasClassName(DOMStringView className) const noexcept
  {
    auto classes = GetAttribute(u8"class");
    if (!classes || className.empty())
    {
      return false;
    }
    bool foldCase = NodeDocument().InQuirksMode();
    DOMStringView remaining = *classes;
    while (!remaining.empty())
    {
      auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
      auto end =
        std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
      DOMStringView token(start, end);
      if (token.size() == className.size()
          && (foldCase
                ? std::ranges::equal(token, className, {}, [](char8 c) { return text::ToASCIILower(c); })
                : token == className))
      {
        return true;
      }
      remaining = DOMStringView(end, remaining.end());
    }
    return false;
  }

  List<DOMStringAtom> Element::ClassNames() const noexcept
  {
    List<DOMStringAtom> classNames;
    auto classes = GetAttribute(u8"class");
    if (!classes)
    {
      return classNames;
    }
    bool foldCase = NodeDocument().InQuirksMode();
    DOMStringView remaining = *classes;
    while (!remaining.empty())
    {
      auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
      auto end =
        std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
      if (start != end)
      {
        DOMStringAtom className = foldCase ? DOMStringAtom(text::ToASCIILower(DOMStringView(start, end)))
                                           : DOMStringAtom(DOMStringView(start, end));
        if (std::ranges::find(classNames, className) == classNames.end())
        {
          classNames.push_back(className);
        }
      }
      remaining = DOMStringView(end, remaining.end());
    }
    return classNames;
  }

  DOMString Element::IdForStyleResolution() const noexcept
  {
    auto id = GetAttribute(u8"id").value_or(DOMString());
    if (NodeDocument().InQuirksMode())
    {
      return text::ToASCIILower(DOMStringView(id));
    }
    return id;
  }

  void Element::ClassName(DOMString &&className) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"class", krys::move(className));
  }

  DOMString Element::ClassName() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"class");
  }

  DOMTokenList &Element::ClassList() noexcept
  {
    if (_domTokenList == nullptr)
    {
      _domTokenList = ElementAlgorithms::CreateDOMTokenList(*this, u8"class");
    }

    return *_domTokenList;
  }

  void Element::Slot(DOMString &&slot) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"slot", krys::move(slot));
  }

  DOMString Element::Slot() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"slot");
  }

  bool Element::HasAttributes() const noexcept
  {
    SynchronizeAllAttributes();
    return !_attributes.empty();
  }

  NamedNodeMap &Element::Attributes() noexcept
  {
    if (_namedNodeMap == nullptr)
    {
      _namedNodeMap = UniquePtr<NamedNodeMap>(new NamedNodeMap(*this));
    }

    return *_namedNodeMap;
  }

  List<DOMString> Element::GetAttributeNames() const noexcept
  {
    SynchronizeAllAttributes();
    return std::ranges::views::transform(_attributes, [](const Ref<Attr> &attr) { return attr->Name(); })
           | std::ranges::to<List<DOMString>>();
  }

  Maybe<DOMString> Element::GetAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    Attr *attr = ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
    if (attr == nullptr)
    {
      return null;
    }

    return attr->Value();
  }

  Maybe<DOMString> Element::GetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    Attr *attr = ElementAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
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

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with qualifiedName, null, this, and value.

    SynchronizeAllAttributes();
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    if (it != _attributes.end())
    {
      ElementAlgorithms::ChangeAttribute(**it, krys::move(value));
    }
    else
    {
      auto attr = AdoptRef<Attr>(
        *new Attr(NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), qualifiedName},
                  krys::move(value)));
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
    ElementAlgorithms::SetAttributeValue(*this, qName.LocalName(), std::move(value), qName.NamespacePrefix(),
                                         qName.NamespaceURI());

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

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    SynchronizeAllAttributes();
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
    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a->Name() == qualifiedName; });
  }

  bool Element::HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a)
                               { return a->NamespaceURI() == namespaceURI && a->LocalName() == localName; });
  }

  Attr *Element::GetAttributeNode(DOMStringAtom qualifiedName) const noexcept
  {
    return ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
  }

  Attr *Element::GetAttributeNodeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
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
          ElementAlgorithms::AttachShadowRoot(*this, init.Mode, init.Clonable, init.Serialisable,
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

  ExceptionOr<RefPtr<Element>> Element::Closest(DOMStringView selectors) noexcept
  {
    auto query = NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return ShareRefPtr(query.Value().Closest(*this));
  }

  ExceptionOr<bool> Element::Matches(DOMStringView selectors) const noexcept
  {
    auto query = NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return query.Value().Matches(*this);
  }

  Ref<HTMLCollection> Element::GetElementsByTagName(DOMStringAtom qualifiedName) noexcept
  {
    return NodeAlgorithms::GetElementsByTagName(*this, qualifiedName);
  }

  Ref<HTMLCollection> Element::GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                      DOMStringAtom localName) noexcept
  {
    return NodeAlgorithms::GetElementsByTagNameNS(*this, namespaceURI, localName);
  }

  Ref<HTMLCollection> Element::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    return NodeAlgorithms::GetElementsByClassName(*this, classNames);
  }

  ExceptionOr<Element *> Element::InsertAdjacentElement(InsertAdjacentWhere where, Element &element) noexcept
  {
    if (auto result = ElementAlgorithms::InsertAdjacent(*this, where, element); result.HasException())
    {
      return result.ReleaseException();
    }

    return &element;
  }

  ExceptionOr<void> Element::InsertAdjacentText(InsertAdjacentWhere where, DOMString &&data) noexcept
  {
    auto textNode = CreateRef<Text>(NodeDocument(), krys::move(data));
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
    return TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::TextContent(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> Element::Before(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> Element::After(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    return mixins::ChildNode::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

  RefPtr<Element> Element::NextElementSibling() noexcept
  {
    return mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return _elementRareData->Children(*this);
  }

  RefPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> Element::FirstElementChild() noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> Element::LastElementChild() const noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> Element::LastElementChild() noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Element::Append(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Element::MoveBefore(Node &node, Node *refChild) noexcept
  {
    return mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Element::QuerySelector(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region Slottable

  RefPtr<html::HTMLSlotElement> Element::AssignedSlot() noexcept
  {
    return mixins::Slottable::AssignedSlot(*this);
  }

#pragma endregion

#pragma region Lazy Attributes

  void Element::SetSynchronizedLazyAttribute(DOMStringAtom localName, DOMString &&value) noexcept
  {
    auto it = std::ranges::find_if(
      _attributes, [&](const Ref<Attr> &attr)
      { return attr->LocalName() == localName && attr->NamespaceURI() == DOMStringAtom::Null(); });
    if (it != _attributes.end())
    {
      (*it)->_value = krys::move(value);
      return;
    }

    auto attr = AdoptRef<Attr>(
      *new Attr(NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), localName},
                krys::move(value)));
    attr->_ownerElement = CreateWeakPtr(this);
    _attributes.push_back(krys::move(attr));
  }

  void Element::SynchronizeStyleAttribute() const noexcept
  {
    const_cast<StyledElement &>(Downcast<StyledElement>(*this)).SynchronizeStyleAttributeInternal();
  }

#pragma endregion

#pragma region Extension Hooks

  void Element::OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                                   Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept
  {
    if (namespaceURI != DOMStringAtom::Null())
    {
      return;
    }

    // NOTE: we currently don't need to handle `id` as we just use the value stored in the attribute. Later,
    // when we optimise we can use this hook to update a member variable to skip needing to look up the
    // attribute. This will be particularly important for a faster implementation of GetElementById.

    if (localName == Attributes::Class)
    {
      auto &domTokenList = ClassList();
      domTokenList._tokens.clear();
      if (value.has_value())
      {
        auto tokens = OrderedSet::Parser(*value);
        domTokenList._tokens.append(tokens.begin(), tokens.end());
      }
    }
    else if (localName == Attributes::Slot)
    {
      if (value == oldValue)
      {
        return;
      }

      if (!value.has_value() && oldValue.has_value() && oldValue->empty())
      {
        return;
      }

      if (value.has_value() && value->empty() && !oldValue.has_value())
      {
        return;
      }

      if (!value.has_value() || value->empty())
      {
        _slottableName = u8"";
      }
      else
      {
        _slottableName = DOMString(*value);
      }

      if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(*this))
      {
        SlotAlgorithms::AssignSlottables(*assignedSlot);
      }

      SlotAlgorithms::AssignSlot(*this);
    }
  }

#pragma endregion
}
