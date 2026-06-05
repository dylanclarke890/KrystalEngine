#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/DOM/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"
#include "Krystal.HTML/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.HTML/DOM/Internals/HTMLCollection/StaticHTMLCollection.hpp"
#include "Krystal.HTML/DOM/ProcessingInstruction.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/DOM/XHTMLDocument.hpp"
#include "Krystal.HTML/DOM/XMLDocument.hpp"
#include "Krystal.HTML/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/SVG/SVGDocument.hpp"
#include <ranges>

namespace Krys::HTML
{

  ExceptionOr<void> NodeAlgorithms::StringReplaceAll(DOMString &&string, ContainerNode &parent) noexcept
  {
    if (string.empty())
    {
      return MutationAlgorithms::ReplaceAll(nullptr, parent);
    }
    else
    {
      Ref<Text> textNode = CreateRef<Text>(parent.NodeDocument(), Krys::Move(string));
      return MutationAlgorithms::ReplaceAll(textNode.get(), parent);
    }
  }

  ExceptionOr<Ref<Node>> NodeAlgorithms::CloneNode(Node &node, RawPtr<Document> document, bool subtree,
                                                   RawPtr<ContainerNode> parent,
                                                   RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    if (document == nullptr)
    {
      document = &node.NodeDocument();
    }

    assert(!Is<Document>(node) || &node == document);

    auto copy = CloneSingleNode(node, *document, fallbackRegistry);
    ExtensibilityHooks::NodeCloned(node, *copy, subtree);

    if (parent != nullptr)
    {
      if (auto append = MutationAlgorithms::Append(*copy, *parent); append.HasException())
      {
        return append.ReleaseException();
      }
    }

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

    if (auto *element = DynamicDowncast<Element>(node))
    {
      if (auto shadowRoot = element->ShadowRoot(); shadowRoot != nullptr && shadowRoot->Clonable())
      {
        auto *elementCopy = Downcast<Element>(copy.get());
        assert(elementCopy->ShadowRoot() == nullptr);

        auto shadowRootRegistry = shadowRoot->CustomElementRegistry();
        if (CustomElementAlgorithms::IsGlobalCustomElementRegistry(shadowRootRegistry.get()))
        {
          shadowRootRegistry =
            ShareRefPtr(CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(*document));
        }

        if (auto attach = ElementAlgorithms::AttachShadowRoot(
              *elementCopy, shadowRoot->Mode(), Clonable(true), Serializable(shadowRoot->Serializable()),
              DelegatesFocus(shadowRoot->DelegatesFocus()), shadowRoot->SlotAssignment(),
              shadowRootRegistry.get());
            attach.HasException())
        {
          return attach.ReleaseException();
        }

        auto shadowRootCopy = elementCopy->ShadowRoot();
        shadowRootCopy->_declarative = shadowRoot->_declarative;
        shadowRootCopy->_keepCustomElementRegistryNull = shadowRoot->_keepCustomElementRegistryNull;

        for (auto &child : ChildNodeRange(*shadowRoot))
        {
          if (auto clone = CloneNode(child, document, subtree, shadowRootCopy.get()); clone.HasException())
          {
            return clone.ReleaseException();
          }
        }
      }
    }

    return copy;
  }

  Ref<Node> NodeAlgorithms::CloneSingleNode(const Node &node, Document &document,
                                            RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    RefPtr<Node> copy = nullptr;

    if (auto *element = DynamicDowncast<Element>(node))
    {
      auto registry = element->CustomElementRegistry();
      if (registry == nullptr)
      {
        registry = ShareRefPtr(fallbackRegistry);
      }

      if (CustomElementAlgorithms::IsGlobalCustomElementRegistry(registry.get()))
      {
        registry = ShareRefPtr(CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(document));
      }

      copy = ElementFactory::Create(document, element->_qualifiedName, element->_is, false, registry);

      for (auto &attr : element->_attributes)
      {
        auto attrCopy = AdoptRef<Attr>(*new Attr(document, attr->_name, DOMString(attr->Value())));
        ElementAlgorithms::AppendAttribute(*attr, Downcast<Element>(*copy));
      }
    }
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
      else if (Is<SVGDocument>(doc))
      {
        copy = CreateRef<SVGDocument>();
      }
      else if (Is<XMLDocument>(doc))
      {
        copy = CreateRef<XMLDocument>();
      }
      else
      {
        copy = CreateRef<Document>();
      }

      auto &docCopy = Downcast<Document>(*copy);
      docCopy._flags = doc._flags;
      docCopy._contentType = doc._contentType;
      docCopy._quirksMode = doc._quirksMode;
      docCopy._allowDeclarativeShadowRoots = doc._allowDeclarativeShadowRoots;

      if (doc._customElementRegistry->IsScoped())
      {
        docCopy._customElementRegistry = doc._customElementRegistry;
      }

      // TODO(feat): DOM - Set copy's encoding
      // SPEC-VIOLATION(URL): Set the origin on the cloned document to the origin of the original document.
      // SPEC-VIOLATION(URL): Set the url on the cloned document to the url of the original document.
    }
    else if (Is<DocumentType>(node))
    {
      auto &docType = Downcast<const DocumentType>(node);
      copy = AdoptRef(*new DocumentType(document, docType.Name(), docType.PublicId(), docType.SystemId()));
    }
    else if (Is<Attr>(node))
    {
      auto &attr = Downcast<const Attr>(node);
      copy = AdoptRef<Attr>(*new Attr(document, attr._name, DOMString(attr.Value())));
    }
    else if (Is<Text>(node))
    {
      auto &text = Downcast<const Text>(node);
      copy = CreateRef<Text>(document, DOMString(text.Data()));
    }
    else if (Is<Comment>(node))
    {
      auto &comment = Downcast<const Comment>(node);
      copy = CreateRef<Comment>(document, DOMString(comment.Data()));
    }
    else if (Is<ProcessingInstruction>(node))
    {
      auto &instruction = Downcast<const ProcessingInstruction>(node);
      copy = AdoptRef<ProcessingInstruction>(
        *new ProcessingInstruction(document, DOMString(instruction.Target()), DOMString(instruction.Data())));
    }

    assert(copy);
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
      case HTML::NodeType::DOCUMENT_TYPE_NODE:
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
      case HTML::NodeType::ELEMENT_NODE:
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

        for (size_t i = 0uz; i < thisElement._attributes.size(); i++)
        {
          if (i >= otherElement._attributes.size())
          {
            return false;
          }

          auto &thisAttribute = thisElement._attributes[i];
          auto &otherAttribute = otherElement._attributes[i];

          if (thisAttribute->LocalName() != otherAttribute->LocalName())
          {
            return false;
          }

          if (thisAttribute->NamespaceURI() != otherAttribute->NamespaceURI())
          {
            return false;
          }

          if (thisAttribute->Value() != otherAttribute->Value())
          {
            return false;
          }
        }

        break;
      }
      case HTML::NodeType::ATTRIBUTE_NODE:
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
      case HTML::NodeType::PROCESSING_INSTRUCTION_NODE:
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
      case HTML::NodeType::CDATA_SECTION_NODE:
      case HTML::NodeType::TEXT_NODE:
      case HTML::NodeType::COMMENT_NODE:
      {
        auto &thisCharacterData = Downcast<CharacterData>(A);
        auto &otherCharacterData = Downcast<CharacterData>(B);

        if (thisCharacterData.Data() != otherCharacterData.Data())
        {
          return false;
        }

        break;
      }
      case HTML::NodeType::DOCUMENT_NODE:
      case HTML::NodeType::DOCUMENT_FRAGMENT_NODE: break;
    }

    if (Is<ContainerNode>(A))
    {
      RawPtr<Node> child = A.FirstChild();
      RawPtr<Node> otherChild = B.FirstChild();

      while (child != nullptr && otherChild != nullptr)
      {
        if (!Equals(*child, *otherChild))
        {
          return false;
        }

        child = child->NextSibling();
        otherChild = otherChild->NextSibling();
      }

      if (otherChild != nullptr)
      {
        return false;
      }
    }

    return true;
  }

  DOMStringAtom NodeAlgorithms::LocateNamespacePrefix(const Element &element,
                                                      DOMStringAtom namespaceURI) noexcept
  {
    if (element.NamespaceURI() == namespaceURI && element.Prefix() != DOMStringAtom::Null())
    {
      return element.Prefix();
    }

    for (auto &attribute : element._attributes)
    {
      if (attribute->Prefix() == NamespacePrefix::XMLNS && attribute->Value() == namespaceURI)
      {
        return attribute->LocalName();
      }
    }

    RawPtr<const Element> parentElement = element.ParentElement();
    if (parentElement != nullptr)
    {
      return LocateNamespacePrefix(*parentElement, namespaceURI);
    }

    return DOMStringAtom::Null();
  }

  DOMStringAtom NodeAlgorithms::LocateNamespace(const Node &node, DOMStringAtom prefix) noexcept
  {
    switch (node.NodeType())
    {
      case NodeType::ELEMENT_NODE:
      {
        if (prefix == NamespacePrefix::XML)
        {
          return Namespace::XML;
        }

        if (prefix == NamespacePrefix::XMLNS)
        {
          return Namespace::XMLNS;
        }

        auto &element = Downcast<Element>(node);
        if (element.NamespaceURI() != DOMStringAtom::Null() && element.Prefix() == prefix)
        {
          return element.NamespaceURI();
        }

        if (prefix != DOMStringAtom::Null())
        {
          for (auto &attribute : element._attributes)
          {
            if (attribute->NamespaceURI() == Namespace::XMLNS && attribute->Prefix() == NamespacePrefix::XMLNS
                && attribute->LocalName() == prefix)
            {
              return attribute->Value().empty() ? DOMStringAtom::Null() : attribute->Value();
            }

            if (prefix == DOMStringAtom::Null() && attribute->NamespaceURI() == Namespace::XMLNS
                && attribute->Prefix() == DOMStringAtom::Null()
                && attribute->LocalName() == NamespacePrefix::XMLNS)
            {
              return attribute->Value().empty() ? DOMStringAtom::Null() : attribute->Value();
            }
          }
        }

        RawPtr<const Element> parentElement = element.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*parentElement, prefix);
      }
      case NodeType::DOCUMENT_NODE:
      {
        RefPtr<const Element> documentElement = Downcast<Document>(node).DocumentElement();
        if (documentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*documentElement, prefix);
      }
      case NodeType::DOCUMENT_TYPE_NODE:
      case NodeType::DOCUMENT_FRAGMENT_NODE:
      {
        return DOMStringAtom::Null();
      }
      case NodeType::ATTRIBUTE_NODE:
      {
        RawPtr<const Element> ownerElement = Downcast<Attr>(node).OwnerElement();
        if (ownerElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*ownerElement, prefix);
      }
      default:
      {
        RawPtr<const Element> parentElement = node.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*parentElement, prefix);
      }
    }
  }

  Ref<HTMLCollection> NodeAlgorithms::GetElementsByTagName(ContainerNode &root,
                                                           DOMStringAtom qualifiedName) noexcept
  {
    if (qualifiedName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [](const Element &) { return true; });
    }

    if (Is<HTMLDocument>(root.NodeDocument()))
    {
      DOMStringAtom qualifiedNameLowercase = ::Krys::Text::ToASCIILowercase(qualifiedName.View());
      return CreateRef<LiveHTMLCollection>(root,
                                           [qualifiedName, qualifiedNameLowercase](const Element &element)
                                           {
                                             if (element.NamespaceURI() == Namespace::HTML)
                                             {
                                               return element._qualifiedName.Name() == qualifiedNameLowercase;
                                             }
                                             else
                                             {
                                               return element._qualifiedName.Name() == qualifiedName;
                                             }
                                           });
    }

    return CreateRef<LiveHTMLCollection>(root, [qualifiedName](const Element &element)
                                         { return element._qualifiedName.Name() == qualifiedName; });
  }

  Ref<HTMLCollection> NodeAlgorithms::GetElementsByTagNameNS(ContainerNode &root, DOMStringAtom namespaceUri,
                                                             DOMStringAtom localName) noexcept
  {
    if (namespaceUri == DOMStringAtom::Empty())
    {
      namespaceUri = DOMStringAtom::Null();
    }

    if (namespaceUri == u8"*" && localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [](const Element &node) { return true; });
    }

    if (namespaceUri == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [localName](const Element &element)
                                           { return element.LocalName() == localName; });
    }

    if (localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(root, [namespaceUri](const Element &element)
                                           { return element.NamespaceURI() == namespaceUri; });
    }

    return CreateRef<LiveHTMLCollection>(
      root, [namespaceUri, localName](const Element &element)
      { return element.NamespaceURI() == namespaceUri && element.LocalName() == localName; });
  }

  Ref<HTMLCollection> NodeAlgorithms::GetElementsByClassName(ContainerNode &root,
                                                             DOMStringAtom classNames) noexcept
  {
    bool isQuirksMode = root.NodeDocument()._quirksMode == QuirksMode::Quirks;

    List<DOMString> classes;
    if (isQuirksMode)
    {
      classes = OrderedSet::Parser(Krys::Text::ToASCIILowercase(classNames.View()));
    }
    else
    {
      classes = OrderedSet::Parser(classNames.View());
    }

    if (classes.empty()) // TODO(perf): MINOR - return an empty collection instead.
    {
      return CreateRef<StaticHTMLCollection>(SmallElementList {});
    }

    if (isQuirksMode)
    {
      return CreateRef<LiveHTMLCollection>(
        root,
        [classesQuery = Krys::Move(classes)](const Element &element)
        {
          if (!element.HasAttribute(u8"class"))
          {
            return false;
          }

          // TODO(perf): MINOR - this is terrible for performance but eh, it's quirks mode.
          auto elementClasses =
            OrderedSet::Parser(Krys::Text::ToASCIILowercase(*element.GetAttribute(u8"class")));
          return std::ranges::all_of(classesQuery,
                                     [elementClasses = Krys::Move(elementClasses)](const DOMString &className)
                                     { return std::ranges::contains(elementClasses, className); });
        });
    }

    return CreateRef<LiveHTMLCollection>(
      root,
      [classes = Krys::Move(classes)](const Element &element)
      {
        if (!element.HasAttribute(u8"class"))
        {
          return false;
        }

        auto &classList = *element._domTokenList;
        return std::ranges::all_of(classes,
                                   [&](const DOMString &className) { return classList.Contains(className); });
      });
  }
}