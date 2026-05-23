#include "Krystal.HTML/Algorithms/HTMLCollectionAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  Ref<HTMLCollection> HTMLCollectionAlgorithms::ElementsByTagName(ContainerNode &root,
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

    return CreateRef<LiveHTMLCollection>(root, [qualifiedName](const Element &element)
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

  Ref<HTMLCollection> HTMLCollectionAlgorithms::ElementsByClassName(ContainerNode &root,
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