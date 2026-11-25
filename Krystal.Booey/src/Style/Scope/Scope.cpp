/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2006 Alexey Proskuryakov (ap@webkit.org)
 * Copyright (C) 2004-2009, 2011-2012, 2015-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2008, 2009, 2011, 2012 Google Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) Research In Motion Limited 2010-2011. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/CSSOM/StyleSheet.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLStyleElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/Invalidation/Invalidator.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Rules/MatchResultCache.hpp"
#include "Krystal.Booey/Style/Rules/RuleSet.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/SVG/Elements/SVGStyleElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Move.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo::style
{
  namespace
  {
    template <typename T, typename U>
    KRYS_NODISCARD bool Contains(const List<WeakPtr<T>> &list, const U &value) noexcept
    {
      return std::ranges::any_of(list, [&](const WeakPtr<T> &item) { return item.get() == &value; });
    }

    template <typename T, typename U>
    bool Remove(List<WeakPtr<T>> &list, const U &value) noexcept
    {
      auto it = std::ranges::find_if(list, [&](const WeakPtr<T> &item) { return item.get() == &value; });
      if (it == list.end())
      {
        return false;
      }
      list.erase(it);
      return true;
    }

    template <typename T>
    KRYS_NODISCARD bool IsEmptyIgnoringNullReferences(const List<WeakPtr<T>> &list) noexcept
    {
      return std::ranges::none_of(list, [](const WeakPtr<T> &item) { return item.get() != nullptr; });
    }

    KRYS_NODISCARD bool ContainsAlternate(const Maybe<DOMString> &rel) noexcept
    {
      return rel && rel->contains(u8"alternate");
    }

    void FilterEnabledNonemptyCSSStyleSheets(List<RefPtr<CSSStyleSheet>> &result,
                                             const List<RefPtr<StyleSheet>> &sheets) noexcept
    {
      for (auto &sheet : sheets)
      {
        auto *styleSheet = DynamicDowncast<CSSStyleSheet>(*sheet);
        if (!styleSheet)
        {
          continue;
        }
        if (styleSheet->IsLoading())
        {
          continue;
        }
        if (styleSheet->Disabled())
        {
          continue;
        }
        if (!styleSheet->Length())
        {
          continue;
        }
        result.push_back(ShareRefPtr(styleSheet));
      }
    }
  }

  ScopeIdentifier ScopeIdentifier::Generate() noexcept
  {
    static uint64 next = 0;
    return ScopeIdentifier {++next};
  }

  Scope::Scope(boo::Document &document) noexcept
      : _document(&document), _customPropertyRegistry(CreateUnique<CustomPropertyRegistry>(*this)),
        _counterStyleRegistry(CreateUnique<css::CounterStyleRegistry>())
  {
  }

  Scope::Scope(boo::ShadowRoot &shadowRoot) noexcept
      : _document(&shadowRoot.NodeDocument()), _shadowRoot(&shadowRoot),
        _customPropertyRegistry(CreateUnique<CustomPropertyRegistry>(*this)),
        _counterStyleRegistry(CreateUnique<css::CounterStyleRegistry>())
  {
  }

  Scope::~Scope() noexcept
  {
    krys_debug_assert(!HasPendingSheets());
  }

  size_t Scope::ResolverSharingKeyHash::operator()(const ResolverSharingKey &key) const noexcept
  {
    size_t seed = CombineFastHashes(std::get<1>(key), std::get<2>(key));
    for (auto &contents : std::get<0>(key))
    {
      seed = CombineFastHashes(seed, contents.get());
    }
    return seed;
  }

  Resolver &Scope::GetResolver() noexcept
  {
    if (!_resolver)
    {
      if (_shadowRoot)
      {
        CreateOrFindSharedShadowTreeResolver();
      }
      else
      {
        CreateDocumentResolver();
      }

      if (_resolver->RuleSets().Features().UsesHasPseudoClass())
      {
        _usesHasPseudoClass = true;
      }
    }
    return *_resolver;
  }

  Resolver *Scope::ResolverIfExists() noexcept
  {
    return _resolver.get();
  }

  const Resolver *Scope::ResolverIfExists() const noexcept
  {
    return _resolver.get();
  }

  void Scope::CreateDocumentResolver() noexcept
  {
    krys_debug_assert(!_resolver);
    krys_debug_assert(!_shadowRoot);

    auto isUpdatingStyleResolver = std::exchange(_isUpdatingStyleResolver, true);

    _resolver = Resolver::Create(*_document, Resolver::ScopeType::Document);

    _document->GetFontSelector().BuildStarted();

    _resolver->RuleSets().InitializeUserStyle();
    _resolver->AppendAuthorStyleSheets(_activeStyleSheets);

    _document->GetFontSelector().BuildCompleted();

    _isUpdatingStyleResolver = isUpdatingStyleResolver;
  }

  void Scope::CreateOrFindSharedShadowTreeResolver() noexcept
  {
    krys_debug_assert(!_resolver);
    krys_debug_assert(_shadowRoot);

    krys_assert(!_isUpdatingStyleResolver);

    auto key = MakeResolverSharingKey();

    auto &sharedShadowTreeResolvers = DocumentScope()._sharedShadowTreeResolvers;
    if (auto it = sharedShadowTreeResolvers.find(key); it != sharedShadowTreeResolvers.end())
    {
      _resolver = it->second;
      _resolver->SetSharedBetweenShadowTrees();
      return;
    }

    {
      auto isUpdatingStyleResolver = std::exchange(_isUpdatingStyleResolver, true);

      _resolver = Resolver::Create(*_document, Resolver::ScopeType::ShadowTree);

      _resolver->RuleSets().SetUsesSharedUserStyle(!IsForUserAgentShadowTree());
      _resolver->AppendAuthorStyleSheets(_activeStyleSheets);

      _isUpdatingStyleResolver = isUpdatingStyleResolver;
    }

    Ref<Resolver> sharedResolver = _resolver;
    sharedShadowTreeResolvers.emplace(krys::move(key), krys::move(sharedResolver));

    constexpr static auto maximimumSharedResolverCount = 256;
    if (sharedShadowTreeResolvers.size() > maximimumSharedResolverCount)
    {
      sharedShadowTreeResolvers.erase(sharedShadowTreeResolvers.begin());
    }
  }

  void Scope::UnshareShadowTreeResolverBeforeMutation() noexcept
  {
    krys_debug_assert(_shadowRoot);

    DocumentScope()._sharedShadowTreeResolvers.erase(MakeResolverSharingKey());
  }

  Scope::ResolverSharingKey Scope::MakeResolverSharingKey() noexcept
  {
    constexpr bool isNonEmptyHashTableValue = true;
    List<RefPtr<css::StyleSheetContents>> contents;
    contents.reserve(_activeStyleSheets.size());
    for (auto &sheet : _activeStyleSheets)
    {
      contents.push_back(ShareRefPtr(&sheet->Contents()));
    }
    return {krys::move(contents), IsForUserAgentShadowTree(), isNonEmptyHashTableValue};
  }

  bool Scope::IsForUserAgentShadowTree() const noexcept
  {
    return _shadowRoot && _shadowRoot->Mode() == dom::ShadowRootMode::UserAgent;
  }

  void Scope::ClearResolver() noexcept
  {
    krys_assert(!_isUpdatingStyleResolver);
    krys_assert(!_document->IsResolvingTreeStyle());

    _resolver = nullptr;
    GetCustomPropertyRegistry().ClearRegisteredFromStylesheets();
    GetCounterStyleRegistry().ClearAuthorCounterStyles();
  }

  void Scope::InvalidateMatchedDeclarationsCache() noexcept
  {
    if (!_shadowRoot)
    {
      for (auto &descendantShadowRoot : _document->InDocumentShadowRoots())
      {
        if (descendantShadowRoot)
        {
          descendantShadowRoot->StyleScope().InvalidateMatchedDeclarationsCache();
        }
      }
    }

    if (auto *resolver = ResolverIfExists())
    {
      resolver->InvalidateMatchedDeclarationsCache();
    }
  }

  MatchResultCache &Scope::GetMatchResultCache() noexcept
  {
    if (!_matchResultCache)
    {
      _matchResultCache = CreateUnique<MatchResultCache>();
    }
    return *_matchResultCache;
  }

  void Scope::ReleaseMemory() noexcept
  {
    if (!_shadowRoot)
    {
      for (auto &descendantShadowRoot : _document->InDocumentShadowRoots())
      {
        if (descendantShadowRoot)
        {
          descendantShadowRoot->StyleScope().ReleaseMemory();
        }
      }
    }

    ClearResolver();

    _sharedShadowTreeResolvers.clear();
    _matchResultCache = nullptr;
  }

  Scope *Scope::ForOrdinal(Element &element, ScopeOrdinal ordinal) noexcept
  {
    if (auto *pseudoElement = DynamicDowncast<PseudoElement>(element))
    {
      return pseudoElement->HostElement() ? ForOrdinal(*pseudoElement->HostElement(), ordinal) : nullptr;
    }

    if (ordinal == ScopeOrdinal::Element)
    {
      return &ForNode(element);
    }
    if (ordinal == ScopeOrdinal::Shadow)
    {
      auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element);
      return shadowRoot ? &shadowRoot->StyleScope() : nullptr;
    }
    if (ordinal <= ScopeOrdinal::ContainingHost)
    {
      auto *host = HostForScopeOrdinal(element, ordinal);
      return host ? &ForNode(*host) : nullptr;
    }
    auto *slot = AssignedSlotForScopeOrdinal(element, ordinal);
    return slot ? &ForNode(*slot) : nullptr;
  }

  const Scope *Scope::ForOrdinal(const Element &element, ScopeOrdinal ordinal) noexcept
  {
    return ForOrdinal(const_cast<Element &>(element), ordinal);
  }

  Scope &Scope::ForNode(Node &node) noexcept
  {
    if (auto *shadowRoot = DynamicDowncast<boo::ShadowRoot>(dom::TreeQueries::Root(node)))
    {
      return shadowRoot->StyleScope();
    }
    return node.NodeDocument().StyleScope();
  }

  const Scope &Scope::ForNode(const Node &node) noexcept
  {
    return ForNode(const_cast<Node &>(node));
  }

  void Scope::SetPreferredStylesheetSetName(const DOMString &name) noexcept
  {
    if (_preferredStylesheetSetName == name)
    {
      return;
    }
    _preferredStylesheetSetName = name;
    DidChangeActiveStyleSheetCandidates();
  }

  void Scope::AddPendingSheet(const Element &element) noexcept
  {
    krys_debug_assert(!HasPendingSheet(element));

    bool isInHead = false;
    for (auto *ancestor = element.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (Is<HTMLHeadElement>(*ancestor))
      {
        isInHead = true;
        break;
      }
    }

    if (isInHead)
    {
      _elementsInHeadWithPendingSheets.emplace_back(CreateWeakPtr(&element));
    }
    else
    {
      _elementsInBodyWithPendingSheets.emplace_back(CreateWeakPtr(&element));
    }
  }

  // This method is called whenever a top-level stylesheet has finished loading.
  void Scope::RemovePendingSheet(const Element &element) noexcept
  {
    krys_debug_assert(HasPendingSheet(element));

    if (!Remove(_elementsInHeadWithPendingSheets, element))
    {
      Remove(_elementsInBodyWithPendingSheets, element);
    }

    DidRemovePendingStylesheet();
  }

  void Scope::AddPendingSheet(const ProcessingInstruction &processingInstruction) noexcept
  {
    krys_debug_assert(!HasPendingSheet(processingInstruction));

    _processingInstructionsWithPendingSheets.emplace_back(CreateWeakPtr(&processingInstruction));
  }

  void Scope::RemovePendingSheet(const ProcessingInstruction &processingInstruction) noexcept
  {
    krys_debug_assert(HasPendingSheet(processingInstruction));

    Remove(_processingInstructionsWithPendingSheets, processingInstruction);

    DidRemovePendingStylesheet();
  }

  bool Scope::HasPendingSheets() const noexcept
  {
    return HasPendingSheetsBeforeBody() || !IsEmptyIgnoringNullReferences(_elementsInBodyWithPendingSheets);
  }

  bool Scope::HasPendingSheetsBeforeBody() const noexcept
  {
    return !IsEmptyIgnoringNullReferences(_elementsInHeadWithPendingSheets)
           || !IsEmptyIgnoringNullReferences(_processingInstructionsWithPendingSheets);
  }

  bool Scope::HasPendingSheetsInBody() const noexcept
  {
    return !IsEmptyIgnoringNullReferences(_elementsInBodyWithPendingSheets);
  }

  void Scope::DidRemovePendingStylesheet() noexcept
  {
    if (HasPendingSheets())
    {
      return;
    }

    DidChangeActiveStyleSheetCandidates();

    if (!_shadowRoot)
    {
      _document->DidRemoveAllPendingStylesheet();
    }
  }

  bool Scope::HasPendingSheet(const Element &element) const noexcept
  {
    return Contains(_elementsInHeadWithPendingSheets, element) || HasPendingSheetInBody(element);
  }

  bool Scope::HasPendingSheetInBody(const Element &element) const noexcept
  {
    return Contains(_elementsInBodyWithPendingSheets, element);
  }

  bool Scope::HasPendingSheet(const ProcessingInstruction &processingInstruction) const noexcept
  {
    return Contains(_processingInstructionsWithPendingSheets, processingInstruction);
  }

  void Scope::AddStyleSheetCandidateNode(Node &node, bool createdByParser) noexcept
  {
    if (!node.IsConnected())
    {
      return;
    }

    std::erase_if(_styleSheetCandidateNodes, [](const WeakPtr<Node> &candidate) { return !candidate; });

    // Until the <body> exists, we have no choice but to compare document positions,
    // since styles outside of the body and head continue to be shunted into the head
    // (and thus can shift to end up before dynamically added DOM content that is also
    // outside the body).
    if ((createdByParser && _document->Body()) || _styleSheetCandidateNodes.empty())
    {
      _styleSheetCandidateNodes.push_back(CreateWeakPtr(&node));
      return;
    }

    // Determine an appropriate insertion point.
    for (auto it = _styleSheetCandidateNodes.end(); it != _styleSheetCandidateNodes.begin();)
    {
      --it;
      if ((*it)->CompareDocumentPosition(node) == dom::DocumentPosition::DOCUMENT_POSITION_FOLLOWING)
      {
        _styleSheetCandidateNodes.insert(it + 1, CreateWeakPtr(&node));
        return;
      }
    }

    _styleSheetCandidateNodes.insert(_styleSheetCandidateNodes.begin(), CreateWeakPtr(&node));
  }

  void Scope::RemoveStyleSheetCandidateNode(Node &node) noexcept
  {
    if (Remove(_styleSheetCandidateNodes, node))
    {
      DidChangeActiveStyleSheetCandidates();
    }
  }

  auto Scope::CollectActiveStyleSheets() noexcept -> ActiveStyleSheetCollection
  {
    List<RefPtr<StyleSheet>> sheets;
    List<RefPtr<StyleSheet>> styleSheetsForStyleSheetsList;

    for (auto &candidate : _styleSheetCandidateNodes)
    {
      if (!candidate)
      {
        continue;
      }
      auto &node = *candidate;

      RefPtr<StyleSheet> sheet;
      if (auto *processingInstruction = DynamicDowncast<ProcessingInstruction>(node))
      {
        if (!processingInstruction->IsCSS())
        {
          continue;
        }
        // We don't support linking to embedded CSS stylesheets, see
        // <https://bugs.webkit.org/show_bug.cgi?id=49281> for discussion.
        sheet = ShareRefPtr(processingInstruction->Sheet());
        if (sheet)
        {
          styleSheetsForStyleSheetsList.push_back(sheet);
          sheets.push_back(krys::move(sheet));
        }
        continue;
      }

      auto *element = DynamicDowncast<Element>(node);
      if (!element)
      {
        continue;
      }

      auto *htmlElement = DynamicDowncast<HTMLElement>(*element);
      auto *linkElement = htmlElement ? DynamicDowncast<HTMLLinkElement>(*htmlElement) : nullptr;
      auto *styleElement = htmlElement ? DynamicDowncast<HTMLStyleElement>(*htmlElement) : nullptr;
      auto *svgElement = DynamicDowncast<SVGElement>(*element);
      auto *svgStyleElement = svgElement ? DynamicDowncast<SVGStyleElement>(*svgElement) : nullptr;
      if (!linkElement && !styleElement && !svgStyleElement)
      {
        continue;
      }

      auto title = dom::TreeQueries::IsInShadowTree(*element)
                     ? DOMString()
                     : element->GetAttribute(u8"title").value_or(u8"");
      bool enabledViaScript = false;
      if (linkElement)
      {
        // <LINK> element
        if (linkElement->IsDisabled())
        {
          continue;
        }
        enabledViaScript = linkElement->IsEnabledViaScript();
        if (linkElement->StyleSheetIsLoading())
        {
          // it is loading but we should still decide which style sheet set to use
          if (!enabledViaScript && !title.empty() && _preferredStylesheetSetName.empty())
          {
            if (!ContainsAlternate(element->GetAttribute(u8"rel")))
            {
              _preferredStylesheetSetName = title;
            }
          }
          continue;
        }
        if (!linkElement->Sheet())
        {
          title = {};
        }
      }
      // Get the current preferred styleset. This is the
      // set of sheets that will be enabled.
      if (svgStyleElement)
      {
        sheet = ShareRefPtr(svgStyleElement->Sheet());
      }
      else if (linkElement)
      {
        sheet = ShareRefPtr(linkElement->Sheet());
      }
      else
      {
        sheet = ShareRefPtr(styleElement->Sheet());
      }

      if (sheet)
      {
        styleSheetsForStyleSheetsList.push_back(sheet);
      }

      // Check to see if this sheet belongs to a styleset
      // (thus making it PREFERRED or ALTERNATE rather than
      // PERSISTENT).
      auto rel = element->GetAttribute(u8"rel");
      if (!enabledViaScript && sheet && !title.empty())
      {
        // Yes, we have a title.
        if (_preferredStylesheetSetName.empty())
        {
          // No preferred set has been established. If
          // we are NOT an alternate sheet, then establish
          // us as the preferred set. Otherwise, just ignore
          // this sheet.
          if (styleElement || !ContainsAlternate(rel))
          {
            _preferredStylesheetSetName = title;
          }
        }
        if (title != _preferredStylesheetSetName)
        {
          sheet = nullptr;
        }
      }

      if (ContainsAlternate(rel) && title.empty())
      {
        sheet = nullptr;
      }

      if (sheet)
      {
        sheets.push_back(krys::move(sheet));
      }
    }

    auto canActivateAdoptedStyleSheet = [&](const CSSStyleSheet &sheet)
    {
      if (sheet.Disabled())
      {
        return false;
      }
      return sheet.Title().empty() || sheet.Title() == _preferredStylesheetSetName;
    };

    for (auto &adoptedStyleSheet : TreeScope().GetAdoptedStyleSheets())
    {
      if (!canActivateAdoptedStyleSheet(*adoptedStyleSheet))
      {
        continue;
      }
      // Adopted sheets apply, but aren't in the StyleSheetList, which holds the tree's own CSS style sheets.
      sheets.emplace_back(adoptedStyleSheet);
    }

    return {krys::move(sheets), krys::move(styleSheetsForStyleSheetsList)};
  }

  void Scope::UpdateActiveStyleSheets(UpdateType updateType) noexcept
  {
    krys_debug_assert(!_pendingUpdate);

    if (_document->InStyleRecalc() || _document->InRenderTreeUpdate())
    {
      // Protect against deleting the resolver in the middle of a style resolution.
      _pendingUpdate = UpdateType::ContentsOrInterpretation;
      if (_shadowRoot)
      {
        DocumentScope()._hasDescendantWithPendingUpdate = true;
      }
      _document->ScheduleFullStyleRebuild();
      return;
    }

    auto collection = CollectActiveStyleSheets();

    List<RefPtr<CSSStyleSheet>> activeCSSStyleSheets;

    // FIXME: The injected author style sheets, once there are extension style sheets.

    FilterEnabledNonemptyCSSStyleSheets(activeCSSStyleSheets, collection.ActiveStyleSheets);

    auto styleSheetChange = StyleSheetChange {ResolverUpdateType::Reconstruct};
    if (updateType == UpdateType::ActiveSet)
    {
      styleSheetChange = AnalyzeStyleSheetChange(activeCSSStyleSheets);
    }

    UpdateResolver(activeCSSStyleSheets, styleSheetChange.UpdateType);

    _activeStyleSheets.swap(activeCSSStyleSheets);
    _styleSheetsForStyleSheetList.swap(collection.StyleSheetsForStyleSheetList);

    if (_resolver && _resolver->RuleSets().Features().UsesHasPseudoClass())
    {
      _usesHasPseudoClass = true;
    }

    // Nothing is styled without a render tree. The sheets are still kept current above: they're listed by
    // StyleSheets, and a document parsed before it gets its frame styles with them later.
    if (_document->HasLivingRenderTree())
    {
      InvalidateStyleAfterStyleSheetChange(styleSheetChange);
    }
  }

  void Scope::InvalidateStyleAfterStyleSheetChange(const StyleSheetChange &styleSheetChange) noexcept
  {
    if (_shadowRoot && !_shadowRoot->IsConnected())
    {
      return;
    }

    // The first body or frameset child of the html element.
    auto hasBodyOrFrameset = [&]
    {
      auto isHTMLElement = [](const Element &element, html::TagName tagName)
      {
        return Is<HTMLElement>(element) && element.TagQualifiedName().TagName() == tagName;
      };
      auto *documentElement = dom::TreeQueries::DocumentElement(*_document);
      if (!documentElement || !isHTMLElement(*documentElement, html::TagName::html))
      {
        return false;
      }
      for (auto *child = dom::TreeQueries::FirstElementChild(*documentElement); child;
           child = dom::TreeQueries::NextElementSibling(*child))
      {
        if (isHTMLElement(*child, html::TagName::body) || isHTMLElement(*child, html::TagName::frameset))
        {
          return true;
        }
      }
      return false;
    };

    // If we are already parsing the body and so may have significant amount of elements, put some effort into
    // trying to avoid style recalcs.
    bool invalidateAll = !hasBodyOrFrameset() || _document->HasNodesWithMissingStyle();

    if (styleSheetChange.UpdateType == ResolverUpdateType::Reconstruct || invalidateAll)
    {
      Invalidator::InvalidateAllStyle(*this);
      return;
    }

    Invalidator invalidator(styleSheetChange.AddedSheets, _resolver->MediaQueryEvaluator());
    invalidator.InvalidateStyle(*this);
  }

  Scope::StyleSheetChange
    Scope::AnalyzeStyleSheetChange(const List<RefPtr<CSSStyleSheet>> &newStylesheets) noexcept
  {
    size_t newStylesheetCount = newStylesheets.size();

    auto *resolver = ResolverIfExists();
    if (!resolver)
    {
      return {ResolverUpdateType::Reconstruct};
    }

    if (resolver->IsSharedBetweenShadowTrees())
    {
      return {ResolverUpdateType::Reconstruct};
    }

    // Find out which stylesheets are new.
    size_t oldStylesheetCount = _activeStyleSheets.size();
    if (newStylesheetCount < oldStylesheetCount)
    {
      return {ResolverUpdateType::Reconstruct};
    }

    List<Ref<css::StyleSheetContents>> addedSheets;
    size_t newIndex = 0;
    for (size_t oldIndex = 0; oldIndex < oldStylesheetCount; ++oldIndex)
    {
      if (newIndex >= newStylesheetCount)
      {
        return {ResolverUpdateType::Reconstruct};
      }

      while (_activeStyleSheets[oldIndex] != newStylesheets[newIndex])
      {
        addedSheets.push_back(ShareRef(newStylesheets[newIndex]->Contents()));
        ++newIndex;
        if (newIndex == newStylesheetCount)
        {
          return {ResolverUpdateType::Reconstruct};
        }
      }
      ++newIndex;
    }
    bool hasInsertions = !addedSheets.empty();
    while (newIndex < newStylesheetCount)
    {
      addedSheets.push_back(ShareRef(newStylesheets[newIndex]->Contents()));
      ++newIndex;
    }

    // If all new sheets were added at the end of the list we can just add them to existing Resolver.
    // If there were insertions we need to re-add all the stylesheets so rules are ordered correctly.
    return {hasInsertions ? ResolverUpdateType::Reset : ResolverUpdateType::Additive,
            krys::move(addedSheets)};
  }

  void Scope::UpdateResolver(Span<const RefPtr<CSSStyleSheet>> activeStyleSheets,
                             ResolverUpdateType updateType) noexcept
  {
    if (updateType == ResolverUpdateType::Reconstruct)
    {
      ClearResolver();
      return;
    }

    if (_shadowRoot)
    {
      UnshareShadowTreeResolverBeforeMutation();
    }

    auto isUpdatingStyleResolver = std::exchange(_isUpdatingStyleResolver, true);

    if (updateType == ResolverUpdateType::Reset)
    {
      GetCustomPropertyRegistry().ClearRegisteredFromStylesheets();
      GetCounterStyleRegistry().ClearAuthorCounterStyles();
      _resolver->RuleSets().ResetAuthorStyle();
      _resolver->AppendAuthorStyleSheets(activeStyleSheets);
    }
    else
    {
      krys_debug_assert(updateType == ResolverUpdateType::Additive);
      krys_debug_assert(activeStyleSheets.size() >= _activeStyleSheets.size());

      auto firstNewIndex = _activeStyleSheets.size();
      _resolver->AppendAuthorStyleSheets(activeStyleSheets.subspan(firstNewIndex));
    }

    _isUpdatingStyleResolver = isUpdatingStyleResolver;
  }

  bool Scope::ActiveStyleSheetsContains(const CSSStyleSheet &sheet) const noexcept
  {
    return std::ranges::any_of(_activeStyleSheets,
                               [&](const RefPtr<CSSStyleSheet> &active) { return active.get() == &sheet; });
  }

  void Scope::FlushPendingUpdate() noexcept
  {
    if (_hasDescendantWithPendingUpdate)
    {
      FlushPendingDescendantUpdates();
    }
    if (_pendingUpdate)
    {
      FlushPendingSelfUpdate();
    }
  }

  void Scope::FlushPendingSelfUpdate() noexcept
  {
    krys_debug_assert(_pendingUpdate);

    auto updateType = *_pendingUpdate;

    ClearPendingUpdate();

    UpdateActiveStyleSheets(updateType);
  }

  void Scope::FlushPendingDescendantUpdates() noexcept
  {
    krys_debug_assert(_hasDescendantWithPendingUpdate);
    krys_debug_assert(!_shadowRoot);
    // Cleared first: an update deferred during style resolution sets it again.
    _hasDescendantWithPendingUpdate = false;
    for (auto &descendantShadowRoot : _document->InDocumentShadowRoots())
    {
      if (descendantShadowRoot)
      {
        descendantShadowRoot->StyleScope().FlushPendingUpdate();
      }
    }
  }

  void Scope::ClearPendingUpdate() noexcept
  {
    _pendingUpdate = null;
  }

  void Scope::ScheduleUpdate(UpdateType update) noexcept
  {
    if (update == UpdateType::ContentsOrInterpretation)
    {
      // :host and ::slotted rules might go away.
      if (_shadowRoot)
      {
        Invalidator::InvalidateHostAndSlottedStyleIfNeeded(*_shadowRoot);
        UnshareShadowTreeResolverBeforeMutation();
      }

      ClearResolver();

      _matchResultCache = nullptr;
    }

    if (!_pendingUpdate || *_pendingUpdate < update)
    {
      _pendingUpdate = update;
      if (_shadowRoot)
      {
        DocumentScope()._hasDescendantWithPendingUpdate = true;
      }
    }

    if (_isPendingUpdateTaskQueued)
    {
      return;
    }
    _isPendingUpdateTaskQueued = true;
    html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation, Document(),
                                         [scope = CreateWeakPtr(this)]
                                         {
                                           if (scope)
                                           {
                                             scope->_isPendingUpdateTaskQueued = false;
                                             scope->FlushPendingUpdate();
                                           }
                                         });
  }

  void Scope::DidChangeActiveStyleSheetCandidates() noexcept
  {
    ScheduleUpdate(UpdateType::ActiveSet);
  }

  void Scope::DidChangeStyleSheetContents() noexcept
  {
    ScheduleUpdate(UpdateType::ContentsOrInterpretation);
  }

  void Scope::DidChangeStyleSheetEnvironment() noexcept
  {
    if (!_shadowRoot)
    {
      for (auto &descendantShadowRoot : _document->InDocumentShadowRoots())
      {
        if (descendantShadowRoot)
        {
          descendantShadowRoot->StyleScope().ScheduleUpdate(UpdateType::ContentsOrInterpretation);
        }
      }

      _document->InvalidateCachedCSSParserContext();
    }

    ScheduleUpdate(UpdateType::ContentsOrInterpretation);
  }

  auto Scope::MediaQueryViewportStateForDocument(const boo::Document &document) noexcept
    -> MediaQueryViewportState
  {
    // These things affect evaluation of viewport dependent media queries.
    return {document.View()->GetLayoutSize(), document.Frame()->PageZoomFactor(), document.Printing()};
  }

  void Scope::EvaluateMediaQueriesForViewportChange() noexcept
  {
    auto viewportState = MediaQueryViewportStateForDocument(*_document);

    if (_viewportStateOnPreviousMediaQueryEvaluation
        && *_viewportStateOnPreviousMediaQueryEvaluation == viewportState)
    {
      return;
    }
    // This doesn't need to be invalidated as any changes to the rules will compute their media queries to
    // correct values.
    _viewportStateOnPreviousMediaQueryEvaluation = viewportState;

    EvaluateMediaQueries([](Resolver &resolver) { return resolver.EvaluateDynamicMediaQueries(); });
  }

  void Scope::EvaluateMediaQueriesForAccessibilitySettingsChange() noexcept
  {
    EvaluateMediaQueries([](Resolver &resolver) { return resolver.EvaluateDynamicMediaQueries(); });
  }

  void Scope::EvaluateMediaQueriesForAppearanceChange() noexcept
  {
    EvaluateMediaQueries([](Resolver &resolver) { return resolver.EvaluateDynamicMediaQueries(); });
  }

  auto Scope::CollectResolverScopes() noexcept -> ResolverScopes
  {
    krys_debug_assert(!_shadowRoot);

    ResolverScopes resolverScopes;

    auto add = [&](Resolver &resolver, Scope &scope)
    {
      auto it =
        std::ranges::find_if(resolverScopes, [&](auto &entry) { return entry.first.get() == &resolver; });
      if (it == resolverScopes.end())
      {
        resolverScopes.emplace_back(ShareRef(resolver), List<WeakPtr<Scope>> {});
        it = std::prev(resolverScopes.end());
      }
      it->second.push_back(CreateWeakPtr(&scope));
    };

    if (auto *resolver = ResolverIfExists())
    {
      add(*resolver, *this);
    }

    for (auto &shadowRoot : _document->InDocumentShadowRoots())
    {
      if (!shadowRoot)
      {
        continue;
      }
      auto &scope = shadowRoot->StyleScope();

      if (auto *resolver = scope.ResolverIfExists())
      {
        add(*resolver, scope);
      }
    }
    return resolverScopes;
  }

  template <typename TFunction>
  void Scope::EvaluateMediaQueries(TFunction testFunction) noexcept
  {
    auto resolverScopes = CollectResolverScopes();
    for (auto &[resolver, scopes] : resolverScopes)
    {
      auto evaluationChanges = testFunction(*resolver);
      if (!evaluationChanges)
      {
        continue;
      }

      for (auto &scope : scopes)
      {
        if (!scope)
        {
          continue;
        }
        switch (evaluationChanges->ChangeType)
        {
          case DynamicMediaQueryEvaluationChanges::Type::InvalidateStyle:
          {
            Invalidator invalidator(evaluationChanges->InvalidationRuleSets);
            invalidator.InvalidateStyle(*scope);
            break;
          }
          case DynamicMediaQueryEvaluationChanges::Type::ResetStyle:
            scope->ScheduleUpdate(UpdateType::ContentsOrInterpretation);
            break;
        }
      }
    }
  }

  void Scope::DidChangeViewportSize() noexcept
  {
    ContainerNode *rootNode = _document;
    if (_shadowRoot)
    {
      rootNode = _shadowRoot;
    }
    else
    {
      if (!_document->HasStyleWithViewportUnits())
      {
        return;
      }

      for (auto &descendantShadowRoot : _document->InDocumentShadowRoots())
      {
        if (descendantShadowRoot && descendantShadowRoot->Mode() != dom::ShadowRootMode::UserAgent)
        {
          descendantShadowRoot->StyleScope().DidChangeViewportSize();
        }
      }
    }

    auto *resolver = ResolverIfExists();
    if (!resolver)
    {
      return;
    }
    resolver->ClearCachedDeclarationsAffectedByViewportUnits();

    if (GetCustomPropertyRegistry().InvalidatePropertiesWithViewportUnits(*_document))
    {
      if (!_shadowRoot)
      {
        if (auto element = _document->DocumentElement())
        {
          element->InvalidateStyleForSubtree();
        }
      }
      return;
    }

    // An element is noted for its pseudo-elements' styles too (nextIncludingPseudo), as they aren't nodes.
    std::erase_if(_elementsWithViewportUnits,
                  [&](const auto &entry)
                  {
                    auto *element = entry.second.get();
                    auto *style = element ? element->GetRenderStyle() : nullptr;
                    // Those no longer using them, or no longer here, are noted again if they do.
                    if (!style || !UsesViewportUnits(*style) || &ForNode(*element) != this)
                    {
                      return true;
                    }
                    element->InvalidateStyle();
                    return false;
                  });
  }

  bool Scope::UsesViewportUnits(const RenderStyle &style) noexcept
  {
    if (style.UsesViewportUnits())
    {
      return true;
    }
    auto *pseudoStyles = style.CachedPseudoStyles();
    return pseudoStyles
           && std::any_of(pseudoStyles->Styles.begin(), pseudoStyles->Styles.end(),
                          [](const auto &entry) { return entry.second && UsesViewportUnits(*entry.second); });
  }

  void Scope::AddElementWithViewportUnits(Element &element) noexcept
  {
    _elementsWithViewportUnits.insert_or_assign(&element, CreateWeakPtr<Element>(&element));
    if (_elementsWithViewportUnits.size() >= _elementsWithViewportUnitsPruneSize)
    {
      std::erase_if(_elementsWithViewportUnits, [](const auto &entry) { return !entry.second.get(); });
      _elementsWithViewportUnitsPruneSize = std::max(64uz, _elementsWithViewportUnits.size() * 2);
    }
  }

  const List<RefPtr<StyleSheet>> &Scope::StyleSheetsForStyleSheetList() noexcept
  {
    // FIXME: StyleSheetList content should be updated separately from style resolver updates.
    FlushPendingUpdate();
    return _styleSheetsForStyleSheetList;
  }

  Scope &Scope::DocumentScope() noexcept
  {
    return _document->StyleScope();
  }

  DocumentOrShadowRoot &Scope::TreeScope() noexcept
  {
    if (_shadowRoot)
    {
      return *_shadowRoot;
    }
    return *_document;
  }

  const Scope *Scope::HostScope() const noexcept
  {
    if (!_shadowRoot || !_shadowRoot->Host())
    {
      return nullptr;
    }
    return &ForNode(*_shadowRoot->Host());
  }

  HTMLSlotElement *AssignedSlotForScopeOrdinal(const Element &element, ScopeOrdinal scopeOrdinal) noexcept
  {
    krys_debug_assert(scopeOrdinal >= ScopeOrdinal::FirstSlot);
    auto *slot = dom::SlotAlgorithms::GetAssignedSlot(element);
    for (auto scopeDepth = ScopeOrdinal::FirstSlot; slot && scopeDepth != scopeOrdinal; ++scopeDepth)
    {
      slot = dom::SlotAlgorithms::GetAssignedSlot(*slot);
    }
    return slot;
  }

  Element *HostForScopeOrdinal(const Element &element, ScopeOrdinal scopeOrdinal) noexcept
  {
    krys_debug_assert(scopeOrdinal <= ScopeOrdinal::ContainingHost);
    auto *host = dom::TreeQueries::ShadowHost(element);
    for (auto scopeDepth = ScopeOrdinal::ContainingHost; host && scopeDepth != scopeOrdinal; --scopeDepth)
    {
      host = dom::TreeQueries::ShadowHost(*host);
    }
    return host;
  }

  bool Scope::InvalidateForLayoutDependencies(LayoutDependencyUpdateContext &context) noexcept
  {
    auto didInvalidate = false;
    didInvalidate |= InvalidateForContainerDependencies(context);
    didInvalidate |= InvalidateForAnchorDependencies(context);
    didInvalidate |= InvalidateForPositionTryFallbacks(context);
    return didInvalidate;
  }

  bool Scope::InvalidateForContainerDependencies(LayoutDependencyUpdateContext &context) noexcept
  {
    krys_debug_assert(!_shadowRoot);

    auto *renderView = _document->GetRenderView();
    if (!renderView)
    {
      return false;
    }

    auto previousQueryContainerDimensions = krys::move(_queryContainerDimensionsOnLastUpdate);
    _queryContainerDimensionsOnLastUpdate.clear();

    List<Element *> containersToInvalidate;

    for (const auto *containerRenderer : renderView->ContainerQueryBoxes())
    {
      auto *containerElement = containerRenderer->GetElement();

      // Invalidation uses real elements, replace ::before/::after with its host.
      if (auto *pseudoElement = DynamicDowncast<PseudoElement>(containerElement))
      {
        containerElement = pseudoElement->HostElement();
      }

      if (!containerElement)
      {
        continue;
      }

      auto size = containerRenderer->LogicalSize();

      auto sizeChanged = [&](LayoutSize oldSize)
      {
        switch (containerRenderer->Style().ContainerType())
        {
          case ContainerType::InlineSize: return size.Width() != oldSize.Width();
          case ContainerType::Size:       return size != oldSize;
          case ContainerType::Normal:     break;
        }
        krys_unreachable();
      };

      auto it = previousQueryContainerDimensions.find(containerElement);
      bool changed =
        it == previousQueryContainerDimensions.end() || !it->second.first || sizeChanged(it->second.second);
      // Protect against unstable layout by invalidating only once per container.
      if (changed && context.InvalidatedContainers.insert(containerElement).second)
      {
        containersToInvalidate.push_back(containerElement);
      }
      _queryContainerDimensionsOnLastUpdate.insert_or_assign(
        containerElement,
        Pair<WeakPtr<const Element>, LayoutSize> {CreateWeakPtr<const Element>(containerElement), size});
    }

    for (auto *toInvalidate : containersToInvalidate)
    {
      toInvalidate->InvalidateForQueryContainerSizeChange();
    }

    return !containersToInvalidate.empty();
  }

  bool Scope::InvalidateForAnchorDependencies(LayoutDependencyUpdateContext &context) noexcept
  {
    krys_debug_assert(!_shadowRoot);

    auto *renderView = _document->GetRenderView();
    if (!renderView)
    {
      return false;
    }

    auto previousAnchorPositions = krys::move(_anchorPositionsOnLastUpdate);
    _anchorPositionsOnLastUpdate.clear();

    List<Ref<Element>> anchoredElementsToInvalidate;

    if (renderView->Anchors().empty())
    {
      return false;
    }

    auto anchorMap = AnchorPositionEvaluator::MakeAnchorPositionedForAnchorMap(_anchorPositionedToAnchorMap);

    auto makeAnchorPosition = [&](const RenderBoxModelObject &anchorRenderer)
    {
      AnchorPosition result;
      result.AbsoluteRect = LayoutRect(anchorRenderer.AbsoluteBoundingBoxRectIgnoringTransforms());
      // Include containing block sizes as anchor function insets may be computed against any side and if they
      // change we need to invalidate.
      for (auto *containingBlock = anchorRenderer.ContainingBlock(); containingBlock;
           containingBlock = containingBlock->ContainingBlock())
      {
        if (containingBlock->CanContainAbsolutelyPositionedObjects())
        {
          result.ContainingBlockSizes.push_back(containingBlock->ContentBoxSize());
        }
      }
      return result;
    };

    for (const auto *anchorRenderer : renderView->Anchors())
    {
      auto anchorPosition = makeAnchorPosition(*anchorRenderer);
      _anchorPositionsOnLastUpdate.insert_or_assign(
        anchorRenderer, Pair<WeakPtr<const RenderObject>, AnchorPosition> {
                          CreateWeakPtr<const RenderObject>(anchorRenderer), anchorPosition});

      auto it = previousAnchorPositions.find(anchorRenderer);
      bool changed =
        it == previousAnchorPositions.end() || !it->second.first || it->second.second != anchorPosition;
      if (!changed)
      {
        continue;
      }

      auto anchoredElements = anchorMap.find(anchorRenderer);
      if (anchoredElements == anchorMap.end())
      {
        continue;
      }

      for (auto &anchoredElement : anchoredElements->second)
      {
        if (!context.InvalidatedAnchorPositioned.insert(anchoredElement.get()).second)
        {
          continue;
        }
        anchoredElementsToInvalidate.push_back(anchoredElement);
      }
    }

    for (auto &toInvalidate : anchoredElementsToInvalidate)
    {
      auto *renderer = toInvalidate->Renderer();
      if (renderer && AnchorPositionEvaluator::IsLayoutTimeAnchorPositioned(renderer->Style()))
      {
        renderer->SetNeedsLayout();
      }
      toInvalidate->InvalidateForAnchorRectChange();
    }

    return !anchoredElementsToInvalidate.empty();
  }

  bool Scope::InvalidateForPositionTryFallbacks(LayoutDependencyUpdateContext &context) noexcept
  {
    krys_debug_assert(!_shadowRoot);

    auto *renderView = _document->GetRenderView();
    if (!renderView)
    {
      return false;
    }

    bool invalidated = false;

    for (const auto *box : renderView->PositionTryBoxes())
    {
      if (!AnchorPositionEvaluator::OverflowsInsetModifiedContainingBlock(*box))
      {
        continue;
      }

      auto *element = box->GetElement();
      if (auto *pseudoElement = DynamicDowncast<PseudoElement>(element))
      {
        element = pseudoElement->HostElement();
      }

      if (element)
      {
        if (!context.InvalidatedAnchorPositioned.insert(element).second)
        {
          continue;
        }
        element->InvalidateForAnchorRectChange();
        invalidated = true;
      }
    }

    return invalidated;
  }

  void Scope::UpdateAnchorPositioningStateAfterStyleResolution() noexcept
  {
    if (auto *renderView = _document->GetRenderView())
    {
      // Is this necessary? Or will the combination of layout and scroll invalidation handle it sufficiently?
      AnchorPositionEvaluator::UpdateScrollAdjustments(*renderView);
    }

    std::erase_if(_anchorPositionedToAnchorMap, [](const auto &elementAndState)
                  { return !elementAndState.second.Element || elementAndState.second.Anchors.empty(); });
  }

  Maybe<size_t> Scope::LastSuccessfulPositionOptionIndexFor(const Styleable &styleable) const noexcept
  {
    AnchorPositionedKey key {ShareRefPtr<const boo::Element>(&styleable.Element),
                             styleable.PseudoElementIdentifier};
    auto it = _lastSuccessfulPositionOptionIndexes.find(key);
    if (it == _lastSuccessfulPositionOptionIndexes.end())
    {
      return null;
    }
    return it->second;
  }

  void Scope::SetLastSuccessfulPositionOptionIndexMap(
    HashMap<AnchorPositionedKey, size_t, AnchorPositionedKeyHash> &&map) noexcept
  {
    _lastSuccessfulPositionOptionIndexes = krys::move(map);
  }

  void Scope::ForgetLastSuccessfulPositionOptionIndex(const Styleable &styleable) noexcept
  {
    AnchorPositionedKey key {ShareRefPtr<const boo::Element>(&styleable.Element),
                             styleable.PseudoElementIdentifier};
    _lastSuccessfulPositionOptionIndexes.erase(key);
  }
}
