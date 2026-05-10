#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/Factories/ElementFactory.hpp"
#include "Krystal.HTML/Algorithms/Mixins/NonElementParentNode.hpp"
#include "Krystal.HTML/Algorithms/Mixins/ParentNode.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CDATASection.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/DOMImplementation.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Ranges/Range.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <cassert>

namespace Krys::HTML
{
  Document::Document(Type documentType) noexcept : Document()
  {
    _documentType = documentType;
  }

#pragma region Document

  Document::Document() noexcept : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlag::IsContainerNode)
  {
    SetEventTargetFlag(EventTargetFlag::IsConnected);
  }

  DOMImplementation &Document::Implementation() noexcept
  {
    if (_implementation == nullptr)
    {
      _implementation = UniquePtr<DOMImplementation>(new DOMImplementation(*this));
    }

    return *_implementation;
  }

  DOMString Document::URL() const noexcept
  {
    // TODO(impl):
    return _baseURL.Href;
  }

  DOMString Document::DocumentURI() const noexcept
  {
    return URL();
  }

  DOMString Document::CompatMode() const noexcept
  {
    return _quirksMode == QuirksMode::Quirks ? u8"BackCompat" : u8"CSS1Compat";
  }

  DOMString Document::CharacterSet() const noexcept
  {
    // TODO(impl):
    return u8"UTF-8";
  }

  DOMString Document::Charset() const noexcept
  {
    return CharacterSet();
  }

  DOMString Document::InputEncoding() const noexcept
  {
    return CharacterSet();
  }

  DOMString Document::ContentType() const noexcept
  {
    // TODO(impl):
    return _contentType;
  }

  RefPtr<DocumentType> Document::DocType() noexcept
  {
    auto children = ChildNodeRange(*this);
    auto it = FirstOfType<DocumentType>(children);
    return ShareRefPtr(it == std::ranges::end(children) ? nullptr : Downcast<DocumentType>(&*it));
  }

  RefPtr<const DocumentType> Document::DocType() const noexcept
  {
    auto children = ConstChildNodeRange(*this);
    auto it = FirstOfType<DocumentType>(children);
    return ShareRefPtr(it == std::ranges::end(children) ? nullptr : Downcast<DocumentType>(&*it));
  }

  RefPtr<Element> Document::DocumentElement() noexcept
  {
    return FirstElementChild();
  }

  RefPtr<const Element> Document::DocumentElement() const noexcept
  {
    return FirstElementChild();
  }

  Ref<HTMLCollection> Document::GetElementsByTagName(DOMStringAtom qualifiedName) noexcept
  {
    if (qualifiedName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(*this), [](const Element &) { return true; });
    }

    if (Is<HTMLDocument>(this))
    {
      DOMStringAtom qualifiedNameLowercase = ::Krys::Text::ToASCIILowercase(qualifiedName.View());
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(*this),
                                           [qualifiedName, qualifiedNameLowercase](const Element &element)
                                           {
                                             if (element.NamespaceURI() == Namespaces::HTML)
                                             {
                                               return element.GetQualifiedName().Name()
                                                      == qualifiedNameLowercase;
                                             }
                                             else
                                             {
                                               return element.GetQualifiedName().Name() == qualifiedName;
                                             }
                                           });
    }

    return CreateRef<LiveHTMLCollection>(CreateWeakRef(*this), [qualifiedName](const Element &element)
                                         { return element.GetQualifiedName().Name() == qualifiedName; });
  }

  Ref<HTMLCollection> Document::GetElementsByTagNameNS(DOMStringAtom namespaceUri,
                                                       DOMStringAtom localName) noexcept
  {
    if (namespaceUri == DOMStringAtom::Empty())
    {
      namespaceUri = DOMStringAtom::Null();
    }

    if (namespaceUri == u8"*" && localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(*this), [](const Element &node) { return true; });
    }

    if (namespaceUri == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(*this), [localName](const Element &element)
                                           { return element.LocalName() == localName; });
    }

    if (localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(*this), [namespaceUri](const Element &element)
                                           { return element.NamespaceURI() == namespaceUri; });
    }

    return CreateRef<LiveHTMLCollection>(
      CreateWeakRef(*this), [namespaceUri, localName](const Element &element)
      { return element.NamespaceURI() == namespaceUri && element.LocalName() == localName; });
  }

  Ref<HTMLCollection> Document::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    auto classes = OrderedSet::Parser(classNames.View());
    if (classes.empty())
    {
      // TODO(perf): return an empty collection instead.
      return CreateRef<StaticHTMLCollection>(SmallElementList {});
    }

    if (_quirksMode == QuirksMode::Quirks)
    {
      // TODO(impl): case insensitive matching in quirks mode.
    }

    // TODO(impl): case sensitive matching
    return CreateRef<StaticHTMLCollection>(SmallElementList {});
  }

  ExceptionOr<Ref<Element>> Document::CreateElement(DOMStringAtom localName,
                                                    const ElementCreationOptionsOrString &options) noexcept
  {
    if (!NameValidation::IsValidElementLocalName(localName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (Is<HTMLDocument>(*this))
    {
      localName = ::Krys::Text::ToASCIILowercase(localName.View());
    }

    auto creationOptions = DocumentAlgorithms::FlattenElementCreationOptions(options, *this);
    if (creationOptions.HasException())
    {
      return creationOptions.ReleaseException();
    }

    DOMStringAtom namespaceURI = DOMStringAtom::Null();
    if (Is<HTMLDocument>(*this) || _contentType == u8"application/xhtml+xml")
    {
      namespaceURI = Namespaces::HTML;
    }

    return ElementFactory::CreateElement(*this, {namespaceURI, DOMStringAtom::Null(), localName},
                                         creationOptions.Value().Is, true,
                                         creationOptions.Value().CustomElementRegistry);
  }

  ExceptionOr<Ref<Element>> Document::CreateElementNS(DOMStringAtom namespaceUri, DOMStringAtom qualifiedName,
                                                      const ElementCreationOptionsOrString &options) noexcept
  {
    return DocumentAlgorithms::InternalCreateElementNS(*this, namespaceUri, qualifiedName, options);
  }

  Ref<DocumentFragment> Document::CreateDocumentFragment() noexcept
  {
    return CreateRef<DocumentFragment>(*this);
  }

  Ref<Text> Document::CreateTextNode(DOMString &&data) noexcept
  {
    return CreateRef<Text>(*this, Krys::Move(data));
  }

  ExceptionOr<Ref<CDATASection>> Document::CreateCDATASection(DOMString &&data) noexcept
  {
    if (Is<HTMLDocument>(*this))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (data.contains(u8"]]>"))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    return CreateRef<CDATASection>(*this, Krys::Move(data));
  }

  Ref<Comment> Document::CreateComment(DOMString &&data) noexcept
  {
    return CreateRef<Comment>(*this, Krys::Move(data));
  }

  ExceptionOr<Ref<ProcessingInstruction>> Document::CreateProcessingInstruction(DOMString &&target,
                                                                                DOMString &&data) noexcept
  {
    // TODO(impl): If target does not match the (xml 'NT-Name) Name production, then throw an
    // "InvalidCharacterError" DOMException.

    if (data.contains(u8"?>"))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    return CreateRef<ProcessingInstruction>(*this, Krys::Move(target), Krys::Move(data));
  }

  ExceptionOr<Ref<Node>> Document::ImportNode(Node &node, const BoolOrImportNodeOptions &options) noexcept
  {
    if (Is<Document>(node) || Is<ShadowRoot>(node))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    bool subtree = false;
    RefPtr<HTML::CustomElementRegistry> registry = nullptr;
    if (std::holds_alternative<bool>(options))
    {
      subtree = std::get<bool>(options);
    }
    else
    {
      auto &dict = std::get<ImportNodeOptions>(options);
      subtree = !dict.SelfOnly;
      registry = dict.CustomElementRegistry;

      if (registry != nullptr && !registry->IsScoped() && registry != CustomElementRegistry())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }
    }

    if (registry == nullptr)
    {
      registry = CustomElementRegistry();
    }

    return NodeAlgorithms::CloneNode(node, this, subtree, nullptr, registry.get());
  }

  ExceptionOr<Ref<Node>> Document::AdoptNode(Node &node) noexcept
  {
    if (Is<Document>(node))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (Is<ShadowRoot>(node)) // ShadowRoot cannot disconnect itself from the host node.
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (Is<DocumentFragment>(node))
    {
      auto &documentFragment = Downcast<DocumentFragment>(node);
      // If node is a DocumentFragment node whose host is non-null, then return.
      if (auto host = documentFragment._host.lock())
      {
        return ShareRef<Node>(node);
      }
    }

    if (auto adoptResult = DocumentAlgorithms::AdoptNode(node, *this); adoptResult.HasException())
    {
      return adoptResult.ReleaseException();
    }

    return ShareRef<Node>(node);
  }

  ExceptionOr<Ref<Attr>> Document::CreateAttribute(DOMStringAtom localName) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(localName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (Is<HTMLDocument>(*this))
    {
      localName = ::Krys::Text::ToASCIILowercase(localName.View());
    }

    QualifiedName qualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), localName};
    return AdoptRef<Attr>(*new Attr(*this, qualifiedName));
  }

  ExceptionOr<Ref<Attr>> Document::CreateAttributeNS(DOMStringAtom namespaceUri,
                                                     DOMStringAtom qualifiedName) noexcept
  {
    auto result =
      NameValidation::ValidateAndExtract(namespaceUri, qualifiedName, ValidateAndExtractContext::Attribute);
    if (result.HasException())
    {
      return result.ReleaseException();
    }

    QualifiedName name {result.Value().NamespaceURI, result.Value().Prefix, result.Value().LocalName};
    return AdoptRef<Attr>(*new Attr(*this, name));
  }

  Ref<Event> Document::CreateEvent(DOMStringAtom interface) noexcept
  {
    // TODO(impl): create correct event based on 'interface'
    return CreateRef<Event>(interface);
  }

  Ref<Range> Document::CreateRange() noexcept
  {
    return CreateRef<Range>(BoundaryPoint {ShareRef(*this), 0uz}, BoundaryPoint {ShareRef(*this), 0uz});
  }

  Ref<NodeIterator> Document::CreateNodeIterator(Node &root, WhatToShow whatToShow,
                                                 RefPtr<NodeFilter> &&filter) noexcept
  {
    return AdoptRef(*new NodeIterator(root, whatToShow, Krys::Move(filter)));
  }

  Ref<TreeWalker> Document::CreateTreeWalker(Node &root, WhatToShow whatToShow,
                                             RefPtr<NodeFilter> &&filter) noexcept
  {
    return AdoptRef(*new TreeWalker(root, whatToShow, Krys::Move(filter)));
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Document::Children() noexcept
  {
    return _documentRareData->Children(*this);
  }

  RefPtr<const Element> Document::FirstElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<Element> Document::FirstElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<const Element> Document::LastElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  RefPtr<Element> Document::LastElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  size_t Document::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

  ExceptionOr<void> Document::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Document::Append(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Document::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Document::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    return Mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Document::QuerySelector(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Document::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> Document::GetElementById(DOMStringView elementId) noexcept
  {
    return ShareRefPtr(Mixins::NonElementParentNode::GetElementById(*this, elementId));
  }

  RefPtr<const Element> Document::GetElementById(DOMStringView elementId) const noexcept
  {
    return ShareRefPtr(Mixins::NonElementParentNode::GetElementById(*this, elementId));
  }

#pragma endregion

#pragma region DocumentOrShadowRoot

  RefPtr<CustomElementRegistry> Document::CustomElementRegistry() const noexcept
  {
    if (_customElementRegistry != nullptr && !_customElementRegistry->IsScoped())
    {
      return _customElementRegistry;
    }

    return nullptr;
  }

#pragma endregion
}