#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/StaticHTMLCollection.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/XHTMLDocument.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"
#include <ranges>

namespace krys::boo::dom
{
  ExceptionOr<void> NodeAlgorithms::StringReplaceAll(DOMString &&string, ContainerNode &parent) noexcept
  {
    // 1. Let node be null.
    RefPtr<Text> node;

    // 2. If string is not the empty string, then set node to the result of creating a text node given
    // parent's
    //    node document and string.
    if (!string.empty())
    {
      node = CreateRef<Text>(parent.NodeDocument(), krys::move(string));
    }

    // 3. Replace all with node within parent.
    return MutationAlgorithms::ReplaceAll(node.get(), parent);
  }

  ExceptionOr<Ref<Node>> NodeAlgorithms::CloneNode(Node &node, Document *document, bool subtree,
                                                   ContainerNode *parent,
                                                   html::CustomElementRegistry *fallbackRegistry) noexcept
  {
    if (document == nullptr)
    {
      document = &node.NodeDocument();
    }

    // 1. Assert: node is not a document or node is document.
    krys_debug_assert(!Is<Document>(node) || &node == document);

    // 2. Let copy be the result of cloning a single node given node, document, and fallbackRegistry.
    auto copy = CloneSingleNode(node, *document, fallbackRegistry);

    // 3. Run any cloning steps defined for node in other applicable specifications and pass node, copy, and
    //    subtree as parameters.
    ExtensibilityHooks::NodeCloned(node, *copy, subtree);

    // 4. If parent is non-null, then append copy to parent.
    if (parent != nullptr)
    {
      if (auto append = MutationAlgorithms::Append(*copy, *parent); append.HasException())
      {
        return append.ReleaseException();
      }
    }

    // 5. If subtree is true, then for each child of node's children, in tree order: clone a node given child
    // with
    //    document set to document, subtree set to subtree, parent set to copy, and fallbackRegistry set to
    //    fallbackRegistry.
    if (subtree)
    {
      if (auto *containerNode = DynamicDowncast<ContainerNode>(node))
      {
        auto *containerCopy = Downcast<ContainerNode>(copy.get());
        for (auto &child : ChildNodeRange(*containerNode))
        {
          if (auto clone = CloneNode(child, document, subtree, containerCopy, fallbackRegistry);
              clone.HasException())
          {
            return clone.ReleaseException();
          }
        }
      }
    }

    // 6. If node is an element, node is a shadow host, and node's shadow root's clonable is true:
    if (auto *element = DynamicDowncast<Element>(node))
    {
      if (auto shadowRoot = ShareRefPtr(TreeQueries::GetShadowRoot(*element));
          shadowRoot != nullptr && shadowRoot->Clonable())
      {
        // 6a. Assert: copy is not a shadow host.
        auto *elementCopy = Downcast<Element>(copy.get());
        krys_debug_assert(TreeQueries::GetShadowRoot(*elementCopy) == nullptr);

        // 6b. Let shadowRootRegistry be node's shadow root's custom element registry.
        auto shadowRootRegistry = shadowRoot->CustomElementRegistry();

        // 6c. If shadowRootRegistry is a global custom element registry, then set shadowRootRegistry to
        //     document's custom element registry's effective global custom element registry.
        if (html::CustomElementAlgorithms::IsGlobalCustomElementRegistry(shadowRootRegistry.get()))
        {
          shadowRootRegistry =
            ShareRefPtr(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(*document));
        }

        // 6d. Attach a shadow root with copy, node's shadow root's mode, node's shadow root's delegates
        // focus,
        //     node's shadow root's serializable, node's shadow root's slot assignment, true, and
        //     shadowRootRegistry.
        if (auto attach = ElementAlgorithms::AttachShadowRoot(
              *elementCopy, shadowRoot->Mode(), Clonable(true), Serialisable(shadowRoot->Serialisable()),
              DelegatesFocus(shadowRoot->DelegatesFocus()), shadowRoot->SlotAssignment(),
              shadowRootRegistry.get());
            attach.HasException())
        {
          return attach.ReleaseException();
        }

        // 6e. Set copy's shadow root's declarative to node's shadow root's declarative.
        auto *shadowRootCopy = TreeQueries::GetShadowRoot(*elementCopy);
        shadowRootCopy->_declarative = shadowRoot->_declarative;

        // 6f. Set copy's shadow root's keep custom element registry null to node's shadow root's keep custom
        //     element registry null.
        shadowRootCopy->_keepCustomElementRegistryNull = shadowRoot->_keepCustomElementRegistryNull;

        // 6g. For each child of node's shadow root's children, in tree order: clone a node given child with
        //     document set to document, subtree set to true, and parent set to copy's shadow root.
        for (auto &child : ChildNodeRange(*shadowRoot))
        {
          if (auto clone = CloneNode(child, document, true, shadowRootCopy); clone.HasException())
          {
            return clone.ReleaseException();
          }
        }
      }
    }

    // 7. Return copy.
    return copy;
  }

  Ref<Node> NodeAlgorithms::CloneSingleNode(const Node &node, Document &document,
                                            html::CustomElementRegistry *fallbackRegistry) noexcept
  {
    // 1. Let copy be null.
    RefPtr<Node> copy = nullptr;

    // 2. If node is an element:
    if (auto *element = DynamicDowncast<Element>(node))
    {
      // 2a. Let registry be node's custom element registry.
      auto registry = element->CustomElementRegistry();

      // 2b. If registry is null, then set registry to fallbackRegistry.
      if (registry == nullptr)
      {
        registry = ShareRefPtr(fallbackRegistry);
      }

      // 2c. If registry is a global custom element registry, then set registry to document's custom element
      //     registry's effective global custom element registry.
      if (html::CustomElementAlgorithms::IsGlobalCustomElementRegistry(registry.get()))
      {
        registry = ShareRefPtr(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(document));
      }

      // 2d. Set copy to the result of creating an element, given document, node's local name, node's
      // namespace,
      //     node's namespace prefix, node's is value, false, and registry.
      copy = ElementFactory::Create(document, element->_qualifiedName, element->_is, false, registry);

      // 2e. For each attribute of node's attribute list:
      element->SynchronizeAllAttributes();
      for (auto &attr : element->_attributes)
      {
        // 2e.i. Let copyAttribute be the result of cloning a single node given attribute, document, and null.
        auto attrCopy = AdoptRef<Attr>(*new Attr(document, attr->_name, DOMString(attr->Value())));

        // 2e.ii. Append copyAttribute to copy.
        ElementAlgorithms::AppendAttribute(*attrCopy, Downcast<Element>(*copy));
      }
    }
    // 3. Otherwise, if node is a document, set copy to the result of creating a document that implements the
    // same
    //    interfaces as node, given document's relevant realm.
    else if (Is<Document>(node))
    {
      auto &doc = Downcast<Document>(node);
      if (Is<HTMLDocument>(doc))
      {
        copy = CreateRef<HTMLDocument>();
      }
      else if (Is<XHTMLDocument>(doc))
      {
        copy = CreateRef<XHTMLDocument>();
      }
      else if (Is<svg::SVGDocument>(doc))
      {
        copy = CreateRef<svg::SVGDocument>();
      }
      else if (Is<XMLDocument>(doc))
      {
        copy = CreateRef<XMLDocument>();
      }
      else
      {
        copy = CreateRef<Document>();
      }

      // 5. Fulfill these additional requirements, switching on the interface node implements:
      // Document:
      // 5a. Set copy's encoding, content type, URL, origin, type, mode, and allow declarative shadow roots,
      // to
      //     those of node. (Skipped origin: documents have no origin.)
      auto &docCopy = Downcast<Document>(*copy);
      docCopy._flags = doc._flags;
      docCopy._contentType = doc._contentType;
      docCopy._encoding = doc._encoding;
      docCopy.SetQuirksMode(doc._quirksMode);
      docCopy.SetURL(url::URL(doc._url));
      docCopy._allowDeclarativeShadowRoots = doc._allowDeclarativeShadowRoots;
      docCopy.SetSettings(ShareRef(doc.GetSettings()));

      // 5b. (Skipped: documents have no base URL override; the copy's base URL comes from its URL and its
      // base
      //     element.)

      // 5c. If node's custom element registry's is scoped is true, then set copy's custom element registry to
      //     node's custom element registry.
      if (doc._customElementRegistry != nullptr && doc._customElementRegistry->IsScoped())
      {
        docCopy._customElementRegistry = doc._customElementRegistry;
      }
    }
    // 4. Otherwise, set copy to the result of creating a node that implements the same interfaces as node,
    // given
    //    document.
    // 5. Fulfill these additional requirements, switching on the interface node implements:
    // DocumentType: Set copy's name, public ID, and system ID to those of node.
    else if (Is<DocumentType>(node))
    {
      auto &docType = Downcast<const DocumentType>(node);
      copy = AdoptRef(*new DocumentType(document, docType.Name(), docType.PublicId(), docType.SystemId()));
    }
    // Attr: Set copy's namespace, namespace prefix, local name, and value to those of node.
    else if (Is<Attr>(node))
    {
      auto &attr = Downcast<const Attr>(node);
      copy = AdoptRef<Attr>(*new Attr(document, attr._name, DOMString(attr.Value())));
    }
    // Text: Set copy's data to that of node.
    else if (auto *cdataSection = DynamicDowncast<CDATASection>(node))
    {
      copy = CreateRef<CDATASection>(document, DOMString(cdataSection->Data()));
    }
    else if (Is<Text>(node))
    {
      auto &text = Downcast<const Text>(node);
      copy = CreateRef<Text>(document, DOMString(text.Data()));
    }
    // Comment: Set copy's data to that of node.
    else if (Is<Comment>(node))
    {
      auto &comment = Downcast<const Comment>(node);
      copy = CreateRef<Comment>(document, DOMString(comment.Data()));
    }
    // ProcessingInstruction: Set copy's target and data to those of node.
    else if (Is<ProcessingInstruction>(node))
    {
      auto &instruction = Downcast<const ProcessingInstruction>(node);
      copy = AdoptRef<ProcessingInstruction>(
        *new ProcessingInstruction(document, DOMString(instruction.Target()), DOMString(instruction.Data())));
    }

    // DocumentFragment (otherwise): Do nothing.
    else if (Is<DocumentFragment>(node))
    {
      copy = CreateRef<DocumentFragment>(document);
    }

    // 6. Assert: copy is a node.
    krys_debug_assert(copy != nullptr);

    // 7. If node is a document, then set document to copy.
    // 8. Set copy's node document to document.
    // (Each node is created with its node document, and a document's is itself.)

    // 9. Return copy.
    return copy;
  }

  bool NodeAlgorithms::Equals(const Node &A, const Node &B) noexcept
  {
    if (A.NodeType() != B.NodeType())
    {
      return false;
    }

    switch (A.NodeType())
    {
      case dom::NodeType::DOCUMENT_TYPE_NODE:
      {
        auto &thisDocType = Downcast<DocumentType>(A);
        auto &otherDocType = Downcast<DocumentType>(B);

        if (thisDocType.Name() != otherDocType.Name())
        {
          return false;
        }
        if (thisDocType.PublicId() != otherDocType.PublicId())
        {
          return false;
        }
        if (thisDocType.SystemId() != otherDocType.SystemId())
        {
          return false;
        }

        break;
      }
      case dom::NodeType::ELEMENT_NODE:
      {
        auto &thisElement = Downcast<Element>(A);
        auto &otherElement = Downcast<Element>(B);

        if (thisElement.NamespaceURI() != otherElement.NamespaceURI())
        {
          return false;
        }

        if (thisElement.Prefix() != otherElement.Prefix())
        {
          return false;
        }

        if (thisElement.LocalName() != otherElement.LocalName())
        {
          return false;
        }

        thisElement.SynchronizeAllAttributes();
        otherElement.SynchronizeAllAttributes();
        // The attribute lists are sets: the same size, and each attribute of one is in the other.
        if (thisElement._attributes.size() != otherElement._attributes.size())
        {
          return false;
        }

        for (auto &thisAttribute : thisElement._attributes)
        {
          auto *otherAttribute = ElementAlgorithms::GetAttributeByNamespace(
            thisAttribute->NamespaceURI(), thisAttribute->LocalName(), otherElement);
          if (otherAttribute == nullptr || thisAttribute->Value() != otherAttribute->Value())
          {
            return false;
          }
        }

        break;
      }
      case dom::NodeType::ATTRIBUTE_NODE:
      {
        auto &thisAttribute = Downcast<Attr>(A);
        auto &otherAttribute = Downcast<Attr>(B);

        if (thisAttribute.LocalName() != otherAttribute.LocalName())
        {
          return false;
        }
        if (thisAttribute.NamespaceURI() != otherAttribute.NamespaceURI())
        {
          return false;
        }
        if (thisAttribute.Value() != otherAttribute.Value())
        {
          return false;
        }
        break;
      }
      case dom::NodeType::PROCESSING_INSTRUCTION_NODE:
      {
        auto &thisProcessingInstruction = Downcast<ProcessingInstruction>(A);
        auto &otherProcessingInstruction = Downcast<ProcessingInstruction>(B);

        if (thisProcessingInstruction.Target() != otherProcessingInstruction.Target())
        {
          return false;
        }
        if (thisProcessingInstruction.Data() != otherProcessingInstruction.Data())
        {
          return false;
        }

        break;
      }
      case dom::NodeType::CDATA_SECTION_NODE:
      case dom::NodeType::TEXT_NODE:
      case dom::NodeType::COMMENT_NODE:
      {
        auto &thisCharacterData = Downcast<CharacterData>(A);
        auto &otherCharacterData = Downcast<CharacterData>(B);

        if (thisCharacterData.Data() != otherCharacterData.Data())
        {
          return false;
        }

        break;
      }
      case dom::NodeType::DOCUMENT_NODE:
      case dom::NodeType::DOCUMENT_FRAGMENT_NODE: break;
    }

    if (Is<ContainerNode>(A))
    {
      Node *child = A.FirstChild();
      Node *otherChild = B.FirstChild();

      while (child != nullptr && otherChild != nullptr)
      {
        if (!Equals(*child, *otherChild))
        {
          return false;
        }

        child = child->NextSibling();
        otherChild = otherChild->NextSibling();
      }

      if (child != nullptr || otherChild != nullptr)
      {
        return false;
      }
    }

    return true;
  }

  DOMStringAtom NodeAlgorithms::LocateNamespacePrefix(const Element &element,
                                                      DOMStringAtom namespaceURI) noexcept
  {
    // 1. If element's namespace is namespace and its namespace prefix is non-null, then return its namespace
    //    prefix.
    if (element.NamespaceURI() == namespaceURI && element.Prefix() != DOMStringAtom::Null())
    {
      return element.Prefix();
    }

    // 2. If element has an attribute whose namespace prefix is "xmlns" and value is namespace, then return
    //    element's first such attribute's local name.
    for (auto &attribute : element._attributes)
    {
      if (attribute->Prefix() == infra::NamespacePrefixes::XMLNS && attribute->Value() == namespaceURI)
      {
        return attribute->LocalName();
      }
    }

    // 3. If element's parent element is non-null, then return the result of running locate a namespace prefix
    // on
    //    that element using namespace.
    const Element *parentElement = element.ParentElement();
    if (parentElement != nullptr)
    {
      return LocateNamespacePrefix(*parentElement, namespaceURI);
    }

    // 4. Return null.
    return DOMStringAtom::Null();
  }

  DOMStringAtom NodeAlgorithms::LocateNamespace(const Node &node, DOMStringAtom prefix) noexcept
  {
    switch (node.NodeType())
    {
      // Element:
      case NodeType::ELEMENT_NODE:
      {
        // 1. If prefix is "xml", then return the XML namespace.
        if (prefix == infra::NamespacePrefixes::XML)
        {
          return infra::Namespaces::XML;
        }

        // 2. If prefix is "xmlns", then return the XMLNS namespace.
        if (prefix == infra::NamespacePrefixes::XMLNS)
        {
          return infra::Namespaces::XMLNS;
        }

        // 3. If its namespace is non-null and its namespace prefix is prefix, then return namespace.
        auto &element = Downcast<Element>(node);
        if (element.NamespaceURI() != DOMStringAtom::Null() && element.Prefix() == prefix)
        {
          return element.NamespaceURI();
        }

        // 4. If it has an attribute whose namespace is the XMLNS namespace, namespace prefix is "xmlns", and
        // local
        //    name is prefix, or if prefix is null and it has an attribute whose namespace is the XMLNS
        //    namespace, namespace prefix is null, and local name is "xmlns", then return its value if it is
        //    not the empty string, and null otherwise.
        for (auto &attribute : element._attributes)
        {
          if (attribute->NamespaceURI() != infra::Namespaces::XMLNS)
          {
            continue;
          }

          if ((attribute->Prefix() == infra::NamespacePrefixes::XMLNS && attribute->LocalName() == prefix)
              || (prefix == DOMStringAtom::Null() && attribute->Prefix() == DOMStringAtom::Null()
                  && attribute->LocalName() == infra::NamespacePrefixes::XMLNS))
          {
            return attribute->Value().empty() ? DOMStringAtom::Null() : attribute->Value();
          }
        }

        // 5. If its parent element is null, then return null.
        const Element *parentElement = element.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        // 6. Return the result of running locate a namespace on its parent element using prefix.
        return LocateNamespace(*parentElement, prefix);
      }
      // Document:
      case NodeType::DOCUMENT_NODE:
      {
        // 1. If its document element is null, then return null.
        RefPtr<const Element> documentElement = Downcast<Document>(node).DocumentElement();
        if (documentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        // 2. Return the result of running locate a namespace on its document element using prefix.
        return LocateNamespace(*documentElement, prefix);
      }
      // DocumentType, DocumentFragment: Return null.
      case NodeType::DOCUMENT_TYPE_NODE:
      case NodeType::DOCUMENT_FRAGMENT_NODE:
      {
        return DOMStringAtom::Null();
      }
      // Attr:
      case NodeType::ATTRIBUTE_NODE:
      {
        // 1. If its element is null, then return null.
        const Element *ownerElement = Downcast<Attr>(node).OwnerElement();
        if (ownerElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        // 2. Return the result of running locate a namespace on its element using prefix.
        return LocateNamespace(*ownerElement, prefix);
      }
      // Otherwise:
      default:
      {
        // 1. If its parent element is null, then return null.
        const Element *parentElement = node.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        // 2. Return the result of running locate a namespace on its parent element using prefix.
        return LocateNamespace(*parentElement, prefix);
      }
    }
  }

  Ref<HTMLCollection> NodeAlgorithms::GetElementsByTagName(ContainerNode &root,
                                                           DOMStringAtom qualifiedName) noexcept
  {
    // 1. If qualifiedName is U+002A (*), then return an HTMLCollection rooted at root, whose filter matches
    // only
    //    descendant elements.
    if (qualifiedName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [](const Element &) { return true; });
    }

    // 2. Otherwise, if root's node document is an HTML document, return an HTMLCollection rooted at root,
    // whose
    //    filter matches the following descendant elements:
    //    - Whose namespace is the HTML namespace and whose qualified name is qualifiedName, in ASCII
    //    lowercase.
    //    - Whose namespace is not the HTML namespace and whose qualified name is qualifiedName.
    if (Is<HTMLDocument>(root.NodeDocument()))
    {
      DOMStringAtom qualifiedNameLowercase = ::krys::text::ToASCIILower(qualifiedName.View());
      return CreateRef<LiveHTMLCollection>(root,
                                           [qualifiedName, qualifiedNameLowercase](const Element &element)
                                           {
                                             if (element.NamespaceURI() == infra::Namespaces::HTML)
                                             {
                                               return element._qualifiedName.Name() == qualifiedNameLowercase;
                                             }
                                             else
                                             {
                                               return element._qualifiedName.Name() == qualifiedName;
                                             }
                                           });
    }

    // 3. Otherwise, return an HTMLCollection rooted at root, whose filter matches descendant elements whose
    //    qualified name is qualifiedName.
    return CreateRef<LiveHTMLCollection>(root, [qualifiedName](const Element &element)
                                         { return element._qualifiedName.Name() == qualifiedName; });
  }

  Ref<HTMLCollection> NodeAlgorithms::GetElementsByTagNameNS(ContainerNode &root, DOMStringAtom namespaceUri,
                                                             DOMStringAtom localName) noexcept
  {
    // 1. If namespace is the empty string, then set it to null.
    if (namespaceUri == DOMStringAtom::Empty())
    {
      namespaceUri = DOMStringAtom::Null();
    }

    // 2. If both namespace and localName are U+002A (*), then return an HTMLCollection rooted at root, whose
    //    filter matches descendant elements.
    if (namespaceUri == u8"*" && localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [](const Element &) { return true; });
    }

    // 3. If namespace is U+002A (*), then return an HTMLCollection rooted at root, whose filter matches
    //    descendant elements whose local name is localName.
    if (namespaceUri == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [localName](const Element &element)
                                           { return element.LocalName() == localName; });
    }

    // 4. If localName is U+002A (*), then return an HTMLCollection rooted at root, whose filter matches
    //    descendant elements whose namespace is namespace.
    if (localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [namespaceUri](const Element &element)
                                           { return element.NamespaceURI() == namespaceUri; });
    }

    // 5. Return an HTMLCollection rooted at root, whose filter matches descendant elements whose namespace is
    //    namespace and local name is localName.
    return CreateRef<LiveHTMLCollection>(
      root, [namespaceUri, localName](const Element &element)
      { return element.NamespaceURI() == namespaceUri && element.LocalName() == localName; });
  }

  Ref<HTMLCollection> NodeAlgorithms::GetElementsByClassName(ContainerNode &root,
                                                             DOMStringAtom classNames) noexcept
  {
    bool isQuirksMode = root.NodeDocument()._quirksMode == QuirksMode::Quirks;

    // 1. Let classes be the result of running the ordered set parser on classNames.
    // (In quirks mode they're lowercased first, as the comparisons are ASCII case-insensitive.)
    List<DOMString> classes;
    if (isQuirksMode)
    {
      classes = OrderedSet::Parser(krys::text::ToASCIILower(classNames.View()));
    }
    else
    {
      classes = OrderedSet::Parser(classNames.View());
    }

    // 2. If classes is the empty set, return an empty HTMLCollection.
    if (classes.empty())
    {
      return CreateRef<StaticHTMLCollection>(SmallElementList {});
    }

    // 3. Return an HTMLCollection rooted at root, whose filter matches descendant elements that have all
    // their
    //    classes in classes. The comparisons for the classes must be done in an ASCII case-insensitive manner
    //    if root's node document's mode is "quirks"; otherwise in an identical to manner.
    if (isQuirksMode)
    {
      return CreateRef<LiveHTMLCollection>(
        root,
        [classesQuery = krys::move(classes)](const Element &element)
        {
          if (!element.HasAttribute(u8"class"))
          {
            return false;
          }

          // FIXME: This is terrible for performance but eh, it's quirks mode.
          auto elementClasses =
            OrderedSet::Parser(krys::text::ToASCIILower(*element.GetAttribute(u8"class")));
          return std::ranges::all_of(classesQuery,
                                     [elementClasses = krys::move(elementClasses)](const DOMString &className)
                                     { return std::ranges::contains(elementClasses, className); });
        });
    }

    return CreateRef<LiveHTMLCollection>(
      root,
      [classes = krys::move(classes)](const Element &element)
      {
        if (!element.HasAttribute(u8"class"))
        {
          return false;
        }

        // The class list keeps the element's classes, parsed again only after the attribute changes.
        auto &classList = const_cast<Element &>(element).ClassList();
        return std::ranges::all_of(classes,
                                   [&](const DOMString &className) { return classList.Contains(className); });
      });
  }
}