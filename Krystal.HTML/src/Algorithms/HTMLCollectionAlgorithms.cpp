#include "Krystal.HTML/Algorithms/HTMLCollectionAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML
{
  Ref<HTMLCollection> HTMLCollectionAlgorithms::ElementsByTagName(ContainerNode &root,
                                                                  DOMStringAtom qualifiedName) noexcept
  {
    if (qualifiedName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(root), [](const Element &) { return true; });
    }

    if (Is<HTMLDocument>(root.NodeDocument()))
    {
      DOMStringAtom qualifiedNameLowercase = ::Krys::Text::ToASCIILowercase(qualifiedName.View());
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(root),
                                           [qualifiedName, qualifiedNameLowercase](const Element &element)
                                           {
                                             if (element.NamespaceURI() == Namespaces::HTML)
                                             {
                                               return element._qualifiedName.Name() == qualifiedNameLowercase;
                                             }
                                             else
                                             {
                                               return element._qualifiedName.Name() == qualifiedName;
                                             }
                                           });
    }

    return CreateRef<LiveHTMLCollection>(CreateWeakRef(root), [qualifiedName](const Element &element)
                                         { return element._qualifiedName.Name() == qualifiedName; });
  }

  Ref<HTMLCollection> HTMLCollectionAlgorithms::ElementsByTagNameNS(ContainerNode &root,
                                                                    DOMStringAtom namespaceUri,
                                                                    DOMStringAtom localName) noexcept
  {
    if (namespaceUri == DOMStringAtom::Empty())
    {
      namespaceUri = DOMStringAtom::Null();
    }

    if (namespaceUri == u8"*" && localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(root), [](const Element &node) { return true; });
    }

    if (namespaceUri == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(root), [localName](const Element &element)
                                           { return element.LocalName() == localName; });
    }

    if (localName == u8"*")
    {
      return CreateRef<LiveHTMLCollection>(CreateWeakRef(root), [namespaceUri](const Element &element)
                                           { return element.NamespaceURI() == namespaceUri; });
    }

    return CreateRef<LiveHTMLCollection>(
      CreateWeakRef(root), [namespaceUri, localName](const Element &element)
      { return element.NamespaceURI() == namespaceUri && element.LocalName() == localName; });
  }

  Ref<HTMLCollection> HTMLCollectionAlgorithms::ElementsByClassName(ContainerNode &root,
                                                                    DOMStringAtom classNames) noexcept
  {
    auto classes = OrderedSet::Parser(classNames.View());
    if (classes.empty())
    {
      // TODO(perf): MINOR - return an empty collection instead.
      return CreateRef<StaticHTMLCollection>(SmallElementList {});
    }

    if (root.NodeDocument()._quirksMode == QuirksMode::Quirks)
    {
      // TODO(impl): REQUIRED - case insensitive matching in quirks mode.
    }

    // TODO(impl): REQUIRED - case sensitive matching
    return CreateRef<StaticHTMLCollection>(SmallElementList {});
  }
}