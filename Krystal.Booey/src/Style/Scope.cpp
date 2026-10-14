#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
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

    KRYS_NODISCARD bool ContainsAlternate(const Maybe<dom::DOMString> &rel) noexcept
    {
      return rel && rel->find(u8"alternate") != dom::DOMString::npos;
    }

    void FilterEnabledNonemptyCSSStyleSheets(List<RefPtr<css::CSSStyleSheet>> &result,
                                             const List<RefPtr<css::StyleSheet>> &sheets) noexcept
    {
      for (auto &sheet : sheets)
      {
        auto *styleSheet = DynamicDowncast<css::CSSStyleSheet>(*sheet);
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

  Scope::Scope(dom::Document &document) noexcept
      : _document(&document), _counterStyleRegistry(CreateUnique<css::CounterStyleRegistry>())
  {
  }

  Scope::Scope(dom::ShadowRoot &shadowRoot) noexcept
      : _document(&shadowRoot.NodeDocument()), _shadowRoot(&shadowRoot),
        _counterStyleRegistry(CreateUnique<css::CounterStyleRegistry>())
  {
  }

  Scope::~Scope() noexcept
  {
    krys_debug_assert(!HasPendingSheets());
  }

  size_t Scope::ResolverSharingKeyHash::operator()(const ResolverSharingKey &key) const noexcept
  {
    size_t seed = Hash::Combine(std::get<1>(key), std::get<2>(key));
    for (auto &contents : std::get<0>(key))
    {
      seed = Hash::Combine(seed, contents.get());
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

    if (!_dynamicViewTransitionsStyle)
    {
      _dynamicViewTransitionsStyle = RuleSet::Create();
    }

    _resolver->RuleSets().SetDynamicViewTransitionsStyle(_dynamicViewTransitionsStyle.get());

    // FIXME: Tell the document's font selector that a build started and completed, and add the SVG font face
    // rules, once there are fonts.

    _resolver->RuleSets().InitializeUserStyle();
    _resolver->AppendAuthorStyleSheets(_activeStyleSheets);

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

    static constexpr auto maximimumSharedResolverCount = 256;
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
    // FIXME: User agent shadow roots, once there are any.
    return false;
  }

  void Scope::ClearResolver() noexcept
  {
    krys_assert(!_isUpdatingStyleResolver);
    // FIXME: Also assert that the document isn't resolving its tree style, once there is a tree resolver.

    _resolver = nullptr;
    // FIXME: Clear the custom properties registered from style sheets, once the custom property registry is
    // ported.
    GetCounterStyleRegistry().ClearAuthorCounterStyles();
  }

  void Scope::ClearViewTransitionStyles() noexcept
  {
    ClearResolver();
    _dynamicViewTransitionsStyle = nullptr;
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
    // FIXME: Also clear the match result cache, once it is ported.
  }

  Scope &Scope::ForNode(dom::Node &node) noexcept
  {
    if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(dom::TreeQueries::Root(node)))
    {
      return shadowRoot->StyleScope();
    }
    return node.NodeDocument().StyleScope();
  }

  const Scope &Scope::ForNode(const dom::Node &node) noexcept
  {
    return ForNode(const_cast<dom::Node &>(node));
  }

  void Scope::SetPreferredStylesheetSetName(const dom::DOMString &name) noexcept
  {
    if (_preferredStylesheetSetName == name)
    {
      return;
    }
    _preferredStylesheetSetName = name;
    DidChangeActiveStyleSheetCandidates();
  }

  void Scope::AddPendingSheet(const dom::Element &element) noexcept
  {
    krys_debug_assert(!HasPendingSheet(element));

    bool isInHead = false;
    for (auto *ancestor = element.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (Is<html::HTMLHeadElement>(*ancestor))
      {
        isInHead = true;
        break;
      }
    }

    if (isInHead)
    {
      _elementsInHeadWithPendingSheets.push_back(CreateWeakPtr(&element));
    }
    else
    {
      _elementsInBodyWithPendingSheets.push_back(CreateWeakPtr(&element));
    }
  }

  // This method is called whenever a top-level stylesheet has finished loading.
  void Scope::RemovePendingSheet(const dom::Element &element) noexcept
  {
    krys_debug_assert(HasPendingSheet(element));

    if (!Remove(_elementsInHeadWithPendingSheets, element))
    {
      Remove(_elementsInBodyWithPendingSheets, element);
    }

    DidRemovePendingStylesheet();
  }

  bool Scope::HasPendingSheets() const noexcept
  {
    return HasPendingSheetsBeforeBody() || !IsEmptyIgnoringNullReferences(_elementsInBodyWithPendingSheets);
  }

  bool Scope::HasPendingSheetsBeforeBody() const noexcept
  {
    // FIXME: Also processing instructions with pending sheets, once xml-stylesheet loads.
    return !IsEmptyIgnoringNullReferences(_elementsInHeadWithPendingSheets);
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

    // FIXME: Tell the document that every pending style sheet has loaded (didRemoveAllPendingStylesheet),
    // once scripts and rendering wait for style sheets.
  }

  bool Scope::HasPendingSheet(const dom::Element &element) const noexcept
  {
    return Contains(_elementsInHeadWithPendingSheets, element) || HasPendingSheetInBody(element);
  }

  bool Scope::HasPendingSheetInBody(const dom::Element &element) const noexcept
  {
    return Contains(_elementsInBodyWithPendingSheets, element);
  }

  void Scope::AddStyleSheetCandidateNode(dom::Node &node, bool createdByParser) noexcept
  {
    if (!node.IsConnected())
    {
      return;
    }

    std::erase_if(_styleSheetCandidateNodes, [](const WeakPtr<dom::Node> &candidate) { return !candidate; });

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

  void Scope::RemoveStyleSheetCandidateNode(dom::Node &node) noexcept
  {
    if (Remove(_styleSheetCandidateNodes, node))
    {
      DidChangeActiveStyleSheetCandidates();
    }
  }

  auto Scope::CollectActiveStyleSheets() noexcept -> ActiveStyleSheetCollection
  {
    List<RefPtr<css::StyleSheet>> sheets;
    List<RefPtr<css::StyleSheet>> styleSheetsForStyleSheetsList;

    for (auto &candidate : _styleSheetCandidateNodes)
    {
      if (!candidate)
      {
        continue;
      }
      auto &node = *candidate;

      RefPtr<css::StyleSheet> sheet;
      if (Is<dom::ProcessingInstruction>(node))
      {
        // FIXME: xml-stylesheet processing instructions, once they load style sheets.
        continue;
      }

      auto *element = DynamicDowncast<dom::Element>(node);
      if (!element)
      {
        continue;
      }

      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element);
      auto *linkElement = htmlElement ? DynamicDowncast<html::HTMLLinkElement>(*htmlElement) : nullptr;
      auto *styleElement = htmlElement ? DynamicDowncast<html::HTMLStyleElement>(*htmlElement) : nullptr;
      if (!linkElement && !styleElement)
      {
        // FIXME: SVGStyleElement, once it owns a style sheet.
        continue;
      }

      auto title = dom::TreeQueries::IsInShadowTree(*element)
                     ? dom::DOMString()
                     : element->GetAttribute(u8"title").value_or(u8"");
      bool enabledViaScript = false;
      if (linkElement)
      {
        // <LINK> element
        if (linkElement->Disabled())
        {
          continue;
        }
        // FIXME: enabledViaScript (a script setting the sheet's disabled to false) and the loading branch
        // come with the resource loader.
        if (!linkElement->Sheet())
        {
          title = {};
        }
      }
      // Get the current preferred styleset. This is the
      // set of sheets that will be enabled.
      if (linkElement)
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

    auto canActivateAdoptedStyleSheet = [&](const css::CSSStyleSheet &sheet)
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
      styleSheetsForStyleSheetsList.push_back(adoptedStyleSheet);
      sheets.push_back(adoptedStyleSheet);
    }

    return {krys::move(sheets), krys::move(styleSheetsForStyleSheetsList)};
  }

  void Scope::UpdateActiveStyleSheets(UpdateType updateType) noexcept
  {
    krys_debug_assert(!_pendingUpdate);

    // FIXME: Wait for a living render tree, and defer the update while style is being resolved, once there is
    // a render tree and style resolution.

    auto collection = CollectActiveStyleSheets();

    List<RefPtr<css::CSSStyleSheet>> activeCSSStyleSheets;

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

    // FIXME: Invalidate style after the style sheet change (all style when the resolver is reconstructed, or
    // with the added sheets), once there is style invalidation.
  }

  Scope::StyleSheetChange
    Scope::AnalyzeStyleSheetChange(const List<RefPtr<css::CSSStyleSheet>> &newStylesheets) noexcept
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

  void Scope::UpdateResolver(Span<const RefPtr<css::CSSStyleSheet>> activeStyleSheets,
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
      // FIXME: Clear the custom properties registered from style sheets, once the custom property registry is
      // ported.
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

  bool Scope::ActiveStyleSheetsContains(const css::CSSStyleSheet &sheet) const noexcept
  {
    return std::ranges::any_of(_activeStyleSheets, [&](const RefPtr<css::CSSStyleSheet> &active)
                               { return active.get() == &sheet; });
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
    for (auto &descendantShadowRoot : _document->InDocumentShadowRoots())
    {
      if (descendantShadowRoot)
      {
        descendantShadowRoot->StyleScope().FlushPendingUpdate();
      }
    }
    _hasDescendantWithPendingUpdate = false;
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
        // FIXME: Invalidate :host and ::slotted style, once there is style invalidation.
        UnshareShadowTreeResolverBeforeMutation();
      }

      ClearResolver();

      // FIXME: Also clear the match result cache, once it is ported.
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
    html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation,
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

  const List<RefPtr<css::StyleSheet>> &Scope::StyleSheetsForStyleSheetList() noexcept
  {
    // FIXME: StyleSheetList content should be updated separately from style resolver updates.
    FlushPendingUpdate();
    return _styleSheetsForStyleSheetList;
  }

  Scope &Scope::DocumentScope() noexcept
  {
    return _document->StyleScope();
  }

  dom::DocumentOrShadowRoot &Scope::TreeScope() noexcept
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

  html::HTMLSlotElement *AssignedSlotForScopeOrdinal(const dom::Element &element,
                                                     ScopeOrdinal scopeOrdinal) noexcept
  {
    krys_debug_assert(scopeOrdinal >= ScopeOrdinal::FirstSlot);
    auto *slot = const_cast<dom::Element &>(element).AssignedSlot().get();
    for (auto scopeDepth = ScopeOrdinal::FirstSlot; slot && scopeDepth != scopeOrdinal; ++scopeDepth)
    {
      slot = slot->AssignedSlot().get();
    }
    return slot;
  }

  dom::Element *HostForScopeOrdinal(const dom::Element &element, ScopeOrdinal scopeOrdinal) noexcept
  {
    krys_debug_assert(scopeOrdinal <= ScopeOrdinal::ContainingHost);
    auto *host = dom::TreeQueries::ShadowHost(element);
    for (auto scopeDepth = ScopeOrdinal::ContainingHost; host && scopeDepth != scopeOrdinal; --scopeDepth)
    {
      host = dom::TreeQueries::ShadowHost(*host);
    }
    return host;
  }
}
