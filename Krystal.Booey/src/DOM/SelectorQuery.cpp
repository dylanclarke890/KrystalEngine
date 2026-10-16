#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/NodeList/StaticNodeList.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::dom
{
  namespace
  {
    template <typename TOutput>
    KRYS_ALWAYS_INLINE void AppendOutputForElement(TOutput &output, Element &element) noexcept
    {
      if constexpr (SameType<TOutput, Element *>)
      {
        krys_debug_assert(!output);
        output = &element;
      }
      else
      {
        output.push_back(ShareRef(element));
      }
    }

    template <typename TOutput>
    constexpr bool StopsAtFirstMatch = SameType<TOutput, Element *>;

    /// @brief Calls @p func with each element descendant of @p rootNode in tree order, until it returns true.
    template <typename TFunc>
    void ForEachDescendantElement(const ContainerNode &rootNode, TFunc &&func) noexcept
    {
      for (auto *node = TreeQueries::Next(rootNode, &rootNode); node;
           node = TreeQueries::Next(*node, &rootNode))
      {
        if (auto *element = DynamicDowncast<Element>(*node))
        {
          if (func(*element))
          {
            return;
          }
        }
      }
    }

    KRYS_NODISCARD bool CanBeUsedForIdFastPath(const css::Selector &selector) noexcept
    {
      return selector.Match() == css::SelectorMatch::Id
             || (selector.Match() == css::SelectorMatch::AttributeEquals
                 && selector.Attribute().LocalName().View() == u8"id"
                 && selector.Attribute().NamespaceURI() == DOMStringAtom::Null()
                 && !selector.IsAttributeValueMatchingCaseInsensitive());
    }

    KRYS_NODISCARD bool NameMatchesFilter(const QualifiedName &name, const DOMStringAtom &filterPrefix,
                                          const DOMStringAtom &filterLocalName,
                                          const DOMStringAtom &filterNamespaceURI) noexcept
    {
      if (filterLocalName != name.LocalName())
      {
        return false;
      }
      return filterPrefix == StarAtom() || filterNamespaceURI == name.NamespaceURI();
    }

    KRYS_NODISCARD bool CanOptimizeSingleAttributeExactMatch(const css::Selector &selector) noexcept
    {
      // Bailout if attribute name needs to be definitely case-insensitive.
      if (selector.IsAttributeValueMatchingCaseInsensitive())
      {
        return false;
      }

      auto &attribute = selector.Attribute();

      if (!HTMLDocument::IsCaseSensitiveAttribute(attribute))
      {
        return false;
      }

      // Bailout if we need to synchronize attributes.
      static const DOMStringAtom styleAttr(u8"style");
      static const QualifiedName styleName(DOMStringAtom::Null(), DOMStringAtom::Null(), styleAttr);
      if (NameMatchesFilter(styleName, attribute.NamespacePrefix(), attribute.LocalNameLower(),
                            attribute.NamespaceURI()))
      {
        return false;
      }

      return true;
    }

    KRYS_NODISCARD bool LocalNameMatches(const Element &element, const DOMStringAtom &localName,
                                         const DOMStringAtom &lowercaseLocalName) noexcept
    {
      if (Is<html::HTMLElement>(element) && Is<dom::HTMLDocument>(element.NodeDocument()))
      {
        return element.LocalName() == lowercaseLocalName;
      }
      return element.LocalName() == localName;
    }

    template <typename TOutput>
    void ElementsForLocalName(const ContainerNode &rootNode, const DOMStringAtom &localName,
                              const DOMStringAtom &lowercaseLocalName, TOutput &output) noexcept
    {
      ForEachDescendantElement(rootNode,
                               [&](Element &element)
                               {
                                 if (!LocalNameMatches(element, localName, lowercaseLocalName))
                                 {
                                   return false;
                                 }
                                 AppendOutputForElement(output, element);
                                 return StopsAtFirstMatch<TOutput>;
                               });
    }

    template <typename TOutput>
    void AnyElement(const ContainerNode &rootNode, TOutput &output) noexcept
    {
      ForEachDescendantElement(rootNode,
                               [&](Element &element)
                               {
                                 AppendOutputForElement(output, element);
                                 return StopsAtFirstMatch<TOutput>;
                               });
    }
  }

  SelectorDataList::SelectorDataList(const css::SelectorList &selectorList) noexcept
  {
    for (auto &selector : selectorList)
    {
      _selectors.push_back({&selector});
    }

    if (_selectors.size() == 1)
    {
      const css::Selector &selector = *_selectors.front().Selector;
      if (selector.IsFirstInComplexSelector())
      {
        switch (selector.Match())
        {
          case css::SelectorMatch::Type:  _matchType = MatchType::TagNameMatch; break;
          case css::SelectorMatch::Class: _matchType = MatchType::ClassNameMatch; break;
          case css::SelectorMatch::AttributeEquals:
            // FIXME: The [id="name"] fast path, once tree scopes map IDs to elements.
            if (!CanBeUsedForIdFastPath(selector) && CanOptimizeSingleAttributeExactMatch(selector))
            {
              _matchType = MatchType::AttributeExactMatch;
            }
            else
            {
              _matchType = MatchType::SingleSelector;
            }
            break;
          default: _matchType = MatchType::SingleSelector; break;
        }
      }
      else
      {
        // FIXME: The ID fast paths (rightmost ID lookup and filtering the root by an ancestor ID), once tree
        // scopes map IDs to elements.
        _matchType = MatchType::SingleSelector;
      }
    }
    else
    {
      _matchType = MatchType::MultipleSelectorMatch;
    }
  }

  bool SelectorDataList::SelectorMatches(const SelectorData &selectorData, const Element &element,
                                         const ContainerNode &rootNode) const noexcept
  {
    css::SelectorChecker selectorChecker(element.NodeDocument());
    css::SelectorChecker::CheckingContext selectorCheckingContext(css::SelectorChecker::Mode::QueryingRules);
    selectorCheckingContext.Scope = Is<Document>(rootNode) ? nullptr : ShareRefPtr(&rootNode);

    return selectorChecker.Match(*selectorData.Selector, element, selectorCheckingContext);
  }

  bool SelectorDataList::Matches(const Element &targetElement) const noexcept
  {
    for (auto &selector : _selectors)
    {
      if (SelectorMatches(selector, targetElement, targetElement))
      {
        return true;
      }
    }
    return false;
  }

  Element *SelectorDataList::Closest(Element &targetElement) const noexcept
  {
    for (auto *currentElement = &targetElement; currentElement;
         currentElement = currentElement->ParentElement())
    {
      for (auto &selector : _selectors)
      {
        if (SelectorMatches(selector, *currentElement, targetElement))
        {
          return currentElement;
        }
      }
    }
    return nullptr;
  }

  Ref<NodeList> SelectorDataList::QueryAll(ContainerNode &rootNode) const noexcept
  {
    List<Ref<Element>> result;
    Execute(rootNode, result);
    SmallNodeList nodes;
    for (auto &element : result)
    {
      nodes.push_back(Ref<Node>(element));
    }
    return CreateRef<StaticNodeList>(krys::move(nodes));
  }

  Element *SelectorDataList::QueryFirst(ContainerNode &rootNode) const noexcept
  {
    Element *result = nullptr;
    Execute(rootNode, result);
    return result;
  }

  template <typename TOutput>
  void SelectorDataList::ExecuteSingleTagNameSelectorData(const ContainerNode &rootNode,
                                                          const SelectorData &selectorData,
                                                          TOutput &output) const noexcept
  {
    krys_debug_assert(_selectors.size() == 1);

    auto &tagQualifiedName = selectorData.Selector->TagQualifiedName();
    auto &selectorLocalName = tagQualifiedName.LocalName();
    auto &selectorLowercaseLocalName = selectorData.Selector->TagLocalNameLower();
    auto &selectorNamespaceURI = tagQualifiedName.NamespaceURI();

    if (selectorNamespaceURI == StarAtom())
    {
      if (selectorLocalName != StarAtom())
      {
        // Common case: name defined, selectorNamespaceURI is a wildcard.
        ElementsForLocalName(rootNode, selectorLocalName, selectorLowercaseLocalName, output);
      }
      else
      {
        // Other fairly common case: both are wildcards.
        AnyElement(rootNode, output);
      }
    }
    else
    {
      // Fallback: NamespaceURI is set, selectorLocalName may be StarAtom().
      ForEachDescendantElement(
        rootNode,
        [&](Element &element)
        {
          if (element.NamespaceURI() != selectorNamespaceURI
              || (selectorLocalName != StarAtom()
                  && !LocalNameMatches(element, selectorLocalName, selectorLowercaseLocalName)))
          {
            return false;
          }
          AppendOutputForElement(output, element);
          return StopsAtFirstMatch<TOutput>;
        });
    }
  }

  template <typename TOutput>
  void SelectorDataList::ExecuteSingleClassNameSelectorData(const ContainerNode &rootNode,
                                                            const SelectorData &selectorData,
                                                            TOutput &output) const noexcept
  {
    krys_debug_assert(_selectors.size() == 1);

    auto className = selectorData.Selector->Value().View();
    ForEachDescendantElement(rootNode,
                             [&](Element &element)
                             {
                               if (!element.HasClassName(className))
                               {
                                 return false;
                               }
                               AppendOutputForElement(output, element);
                               return StopsAtFirstMatch<TOutput>;
                             });
  }

  template <typename TOutput>
  void SelectorDataList::ExecuteSingleAttributeExactSelectorData(const ContainerNode &rootNode,
                                                                 const SelectorData &selectorData,
                                                                 TOutput &output) const noexcept
  {
    krys_debug_assert(_selectors.size() == 1);
    krys_debug_assert(CanOptimizeSingleAttributeExactMatch(*selectorData.Selector));

    auto &selectorAttribute = selectorData.Selector->Attribute();
    auto selectorValue = selectorData.Selector->Value().View();
    auto &localNameLowercase = selectorAttribute.LocalNameLower();
    auto &localName = selectorAttribute.LocalName();
    auto &prefix = selectorAttribute.NamespacePrefix();
    auto &namespaceURI = selectorAttribute.NamespaceURI();

    bool documentIsHTML = Is<dom::HTMLDocument>(rootNode.NodeDocument());

    ForEachDescendantElement(
      rootNode,
      [&](Element &element)
      {
        bool isHTML = documentIsHTML && Is<html::HTMLElement>(element);
        auto &localNameToMatch = isHTML ? localNameLowercase : localName;
        for (auto &attribute : element.AttributeList())
        {
          if (!NameMatchesFilter(attribute->GetQualifiedName(), prefix, localNameToMatch, namespaceURI))
          {
            continue;
          }

          if (selectorValue == attribute->Value())
          {
            AppendOutputForElement(output, element);
            return StopsAtFirstMatch<TOutput>;
          }
        }
        return false;
      });
  }

  template <typename TOutput>
  void SelectorDataList::ExecuteSingleSelectorData(const ContainerNode &rootNode,
                                                   const ContainerNode &searchRootNode,
                                                   const SelectorData &selectorData,
                                                   TOutput &output) const noexcept
  {
    krys_debug_assert(_selectors.size() == 1);

    ForEachDescendantElement(searchRootNode,
                             [&](Element &element)
                             {
                               if (!SelectorMatches(selectorData, element, rootNode))
                               {
                                 return false;
                               }
                               AppendOutputForElement(output, element);
                               return StopsAtFirstMatch<TOutput>;
                             });
  }

  template <typename TOutput>
  void SelectorDataList::ExecuteSingleMultiSelectorData(const ContainerNode &rootNode,
                                                        TOutput &output) const noexcept
  {
    ForEachDescendantElement(rootNode,
                             [&](Element &element)
                             {
                               for (auto &selector : _selectors)
                               {
                                 if (SelectorMatches(selector, element, rootNode))
                                 {
                                   AppendOutputForElement(output, element);
                                   return StopsAtFirstMatch<TOutput>;
                                 }
                               }
                               return false;
                             });
  }

  template <typename TOutput>
  void SelectorDataList::Execute(ContainerNode &rootNode, TOutput &output) const noexcept
  {
    switch (_matchType)
    {
      case MatchType::SingleSelector:
        ExecuteSingleSelectorData(rootNode, rootNode, _selectors.front(), output);
        break;
      case MatchType::TagNameMatch:
        ExecuteSingleTagNameSelectorData(rootNode, _selectors.front(), output);
        break;
      case MatchType::ClassNameMatch:
        ExecuteSingleClassNameSelectorData(rootNode, _selectors.front(), output);
        break;
      case MatchType::AttributeExactMatch:
        ExecuteSingleAttributeExactSelectorData(rootNode, _selectors.front(), output);
        break;
      case MatchType::MultipleSelectorMatch: ExecuteSingleMultiSelectorData(rootNode, output); break;
    }
  }

  SelectorQuery::SelectorQuery(css::SelectorList &&selectorList) noexcept
      : _selectorList(krys::move(selectorList)), _selectors(_selectorList)
  {
  }

  size_t SelectorQueryCache::KeyHash::operator()(const Key &key) const noexcept
  {
    auto &context = key.second;
    return Hash::Combine(std::hash<DOMString> {}(key.first), context.Mode,
                         context.thumbAndTrackPseudoElementsEnabled,
                         context.webkitMediaTextTrackDisplayQuirkEnabled);
  }

  SelectorQueryCache &SelectorQueryCache::Singleton() noexcept
  {
    static MainThreadNeverDestroyed<SelectorQueryCache> cache;
    return cache;
  }

  SelectorQuery *SelectorQueryCache::Add(DOMStringView selectors, const Document &document) noexcept
  {
    krys_debug_assert(!selectors.empty());

    constexpr auto MaximumSelectorQueryCacheSize = 512uz;
    if (_entries.size() == MaximumSelectorQueryCacheSize)
    {
      _entries.erase(_entries.begin());
    }

    auto context = css::SelectorParserContext(document);
    auto key = Key {DOMString(selectors), context};

    auto it = _entries.find(key);
    if (it == _entries.end())
    {
      UniquePtr<SelectorQuery> query;
      if (auto selectorList = css::SelectorParser::ParseSelectorList(selectors, context))
      {
        if (selectorList->HasExplicitNestingParent())
        {
          selectorList = css::SelectorParser::ResolveNestingParent(*selectorList, nullptr);
        }
        query = CreateUnique<SelectorQuery>(krys::move(*selectorList));
      }
      it = _entries.emplace(krys::move(key), krys::move(query)).first;
    }
    return it->second.get();
  }

  void SelectorQueryCache::Clear() noexcept
  {
    _entries.clear();
  }
}
