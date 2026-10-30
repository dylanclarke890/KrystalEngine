#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/ScopeIdentifier.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <tuple>

namespace krys::boo::css
{
  class CounterStyleRegistry;
  class CSSStyleSheet;
  class StyleSheet;
  class StyleSheetContents;
}

namespace krys::boo::dom
{
  class Document;
  class DocumentOrShadowRoot;
  class Element;
  class Node;
  class ProcessingInstruction;
  class ShadowRoot;
}

namespace krys::boo::html
{
  class HTMLSlotElement;
}

namespace krys::boo
{
  class RenderBoxModelObject;
  class RenderObject;
}

namespace krys::boo::style
{
  class CustomPropertyRegistry;
  class MatchResultCache;
  class Resolver;
  class RuleSet;
  struct Styleable;

  /// @brief The style sheets of a document or shadow root: the nodes that can provide a style sheet (in tree
  /// order), the sheets that are still loading, the preferred style sheet set, and the active style sheets
  /// that result. Changes are batched and applied by a task on the event loop, or when something needs the
  /// up-to-date sheets.
  class Scope final : public CanMakeWeakPtr<Scope>, public CanMakeCheckedPtr<Scope>
  {
    KRYS_NON_COPY_MOVABLE(Scope);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Scope);

    enum class UpdateType : uint8
    {
      ActiveSet,
      ContentsOrInterpretation
    };

    struct ActiveStyleSheetCollection
    {
      List<RefPtr<css::StyleSheet>> ActiveStyleSheets;
      List<RefPtr<css::StyleSheet>> StyleSheetsForStyleSheetList;
    };

    enum class ResolverUpdateType : uint8
    {
      Reconstruct,
      Reset,
      Additive
    };

    struct StyleSheetChange
    {
      ResolverUpdateType UpdateType;
      List<Ref<css::StyleSheetContents>> AddedSheets {};
    };

    using ResolverSharingKey = std::tuple<List<RefPtr<css::StyleSheetContents>>, bool, bool>;

    // These things affect evaluation of viewport dependent media queries.
    using MediaQueryViewportState = std::tuple<IntSize, float, bool>;

    using ResolverScopes = List<Pair<Ref<Resolver>, List<WeakPtr<Scope>>>>;

    struct ResolverSharingKeyHash
    {
      KRYS_NODISCARD size_t operator()(const ResolverSharingKey &key) const noexcept;
    };

  private:
    const ScopeIdentifier _identifier {ScopeIdentifier::Generate()};
    dom::Document *_document;
    dom::ShadowRoot *_shadowRoot {nullptr};

    RefPtr<Resolver> _resolver;
    mutable RefPtr<RuleSet> _dynamicViewTransitionsStyle;

    List<RefPtr<css::StyleSheet>> _styleSheetsForStyleSheetList;
    List<RefPtr<css::CSSStyleSheet>> _activeStyleSheets;

    // Track the currently loading top-level stylesheets needed for rendering.
    // Sheets loaded using the @import directive are not included in this count.
    // We use this count of pending sheets to detect when we can begin attaching
    // elements and when it is safe to execute scripts.
    List<WeakPtr<const dom::Element>> _elementsInHeadWithPendingSheets;
    List<WeakPtr<const dom::Element>> _elementsInBodyWithPendingSheets;
    List<WeakPtr<const dom::ProcessingInstruction>> _processingInstructionsWithPendingSheets;

    List<WeakPtr<dom::Node>> _styleSheetCandidateNodes;

    dom::DOMString _preferredStylesheetSetName;

    Maybe<UpdateType> _pendingUpdate;

    bool _hasDescendantWithPendingUpdate {false};
    bool _isPendingUpdateTaskQueued {false};
    bool _usesHasPseudoClass {false};
    bool _isUpdatingStyleResolver {false};

    const UniquePtr<CustomPropertyRegistry> _customPropertyRegistry;
    const UniquePtr<css::CounterStyleRegistry> _counterStyleRegistry;

    HashMap<ResolverSharingKey, Ref<Resolver>, ResolverSharingKeyHash> _sharedShadowTreeResolvers;

    Maybe<MediaQueryViewportState> _viewportStateOnPreviousMediaQueryEvaluation;

    UniquePtr<MatchResultCache> _matchResultCache;

    // Keyed by address; the weak pointer tells a destroyed container from a new one at the same address.
    HashMap<const dom::Element *, Pair<WeakPtr<const dom::Element>, LayoutSize>>
      _queryContainerDimensionsOnLastUpdate;

    style::AnchorPositionedToAnchorMap _anchorPositionedToAnchorMap;

    struct AnchorPosition
    {
      LayoutRect AbsoluteRect;
      List<LayoutSize> ContainingBlockSizes;

      KRYS_NODISCARD bool operator==(const AnchorPosition &) const noexcept = default;
    };
    // Keyed by address, like the query containers' dimensions.
    HashMap<const RenderBoxModelObject *, Pair<WeakPtr<const RenderObject>, AnchorPosition>>
      _anchorPositionsOnLastUpdate;

    HashMap<AnchorPositionedKey, size_t, AnchorPositionedKeyHash> _lastSuccessfulPositionOptionIndexes;

  public:
    /// @brief What the invalidations for layout dependencies have already invalidated in one layout.
    struct LayoutDependencyUpdateContext
    {
      HashSet<const dom::Element *> InvalidatedContainers;
      HashSet<const dom::Element *> InvalidatedAnchorPositioned;
    };

    explicit Scope(dom::Document &document) noexcept;
    explicit Scope(dom::ShadowRoot &shadowRoot) noexcept;
    ~Scope() noexcept;

    KRYS_NODISCARD const List<RefPtr<css::CSSStyleSheet>> &ActiveStyleSheets() const noexcept
    {
      return _activeStyleSheets;
    }

    KRYS_NODISCARD const List<RefPtr<css::StyleSheet>> &StyleSheetsForStyleSheetList() noexcept;

    void AddStyleSheetCandidateNode(dom::Node &node, bool createdByParser) noexcept;
    void RemoveStyleSheetCandidateNode(dom::Node &node) noexcept;

    void SetPreferredStylesheetSetName(const dom::DOMString &name) noexcept;

    void AddPendingSheet(const dom::Element &element) noexcept;
    void RemovePendingSheet(const dom::Element &element) noexcept;
    void AddPendingSheet(const dom::ProcessingInstruction &processingInstruction) noexcept;
    void RemovePendingSheet(const dom::ProcessingInstruction &processingInstruction) noexcept;
    KRYS_NODISCARD bool HasPendingSheets() const noexcept;
    KRYS_NODISCARD bool HasPendingSheetsBeforeBody() const noexcept;
    KRYS_NODISCARD bool HasPendingSheetsInBody() const noexcept;
    KRYS_NODISCARD bool HasPendingSheet(const dom::Element &element) const noexcept;
    KRYS_NODISCARD bool HasPendingSheetInBody(const dom::Element &element) const noexcept;
    KRYS_NODISCARD bool
      HasPendingSheet(const dom::ProcessingInstruction &processingInstruction) const noexcept;

    KRYS_NODISCARD bool ActiveStyleSheetsContains(const css::CSSStyleSheet &sheet) const noexcept;

    /// @brief Called when some stylesheet becomes newly enabled or disabled.
    void DidChangeActiveStyleSheetCandidates() noexcept;
    /// @brief Called when contents of a stylesheet is mutated.
    void DidChangeStyleSheetContents() noexcept;
    /// @brief Called when the environment where we interpret the stylesheets changes (for example switching
    /// to printing). The change is assumed to potentially affect all author and user stylesheets including
    /// shadow roots.
    void DidChangeStyleSheetEnvironment() noexcept;

    /// @brief Re-evaluates the media queries of the style sheets after the viewport changed, and invalidates
    /// the styles whose rules started or stopped applying.
    void EvaluateMediaQueriesForViewportChange() noexcept;
    void EvaluateMediaQueriesForAccessibilitySettingsChange() noexcept;
    void EvaluateMediaQueriesForAppearanceChange() noexcept;

    /// @brief Invalidates the styles that use viewport units, after the viewport changed size.
    void DidChangeViewportSize() noexcept;

    KRYS_NODISCARD bool HasPendingUpdate() const noexcept
    {
      return _pendingUpdate || _hasDescendantWithPendingUpdate;
    }

    void FlushPendingUpdate() noexcept;

    KRYS_NODISCARD bool UsesHasPseudoClass() const noexcept
    {
      return _usesHasPseudoClass;
    }

    /// @brief The style resolver, created if needed (shared with shadow trees that have the same style
    /// sheets).
    KRYS_NODISCARD Resolver &GetResolver() noexcept;

    /// @brief The style resolver, if one has been created.
    KRYS_NODISCARD Resolver *ResolverIfExists() noexcept;
    KRYS_NODISCARD const Resolver *ResolverIfExists() const noexcept;

    void ClearResolver() noexcept;
    void ClearViewTransitionStyles() noexcept;

    /// @brief Drops the cached styles of the resolvers of this scope (and, for the document, of its shadow
    /// trees), after a change that can change the styles built from the same declarations.
    void InvalidateMatchedDeclarationsCache() noexcept;

    KRYS_NODISCARD MatchResultCache &GetMatchResultCache() noexcept;

    void ReleaseMemory() noexcept;

    KRYS_NODISCARD const CustomPropertyRegistry &GetCustomPropertyRegistry() const noexcept
    {
      return *_customPropertyRegistry;
    }

    KRYS_NODISCARD CustomPropertyRegistry &GetCustomPropertyRegistry() noexcept
    {
      return *_customPropertyRegistry;
    }

    KRYS_NODISCARD const css::CounterStyleRegistry &GetCounterStyleRegistry() const noexcept
    {
      return *_counterStyleRegistry;
    }

    KRYS_NODISCARD css::CounterStyleRegistry &GetCounterStyleRegistry() noexcept
    {
      return *_counterStyleRegistry;
    }

    KRYS_NODISCARD const dom::Document &Document() const noexcept
    {
      return *_document;
    }

    KRYS_NODISCARD dom::Document &Document() noexcept
    {
      return *_document;
    }

    KRYS_NODISCARD const dom::ShadowRoot *ShadowRoot() const noexcept
    {
      return _shadowRoot;
    }

    KRYS_NODISCARD dom::ShadowRoot *ShadowRoot() noexcept
    {
      return _shadowRoot;
    }

    KRYS_NODISCARD ScopeIdentifier Identifier() const noexcept
    {
      return _identifier;
    }

    /// @brief The scope of the shadow root's host, or null for a document's scope.
    KRYS_NODISCARD const Scope *HostScope() const noexcept;

    /// @brief The scope of the shadow root `node` is in, or else of its document.
    KRYS_NODISCARD static Scope &ForNode(dom::Node &node) noexcept;
    KRYS_NODISCARD static const Scope &ForNode(const dom::Node &node) noexcept;
    /// @brief The scope @p ordinal names from @p element (its own, its shadow root's, a host's or a slot's),
    /// if any.
    KRYS_NODISCARD static Scope *ForOrdinal(dom::Element &element, ScopeOrdinal ordinal) noexcept;
    KRYS_NODISCARD static const Scope *ForOrdinal(const dom::Element &element, ScopeOrdinal ordinal) noexcept;

    /// @brief Dereferences the tree-scoped name @p reference: calls @p function with the scope it's from,
    /// then with each scope of the hosts above, until it returns a truthy result.
    /// @see https://drafts.csswg.org/css-scoping-1/#shadow-names
    template <typename TFunction>
    KRYS_NODISCARD static auto ResolveTreeScopedReference(const dom::Element &element,
                                                          const ScopedName &reference,
                                                          const TFunction &function) noexcept;

    /// @brief Invalidates the styles that depend on the last layout.
    /// @return Whether anything was invalidated, so the document needs another style update and layout.
    bool InvalidateForLayoutDependencies(LayoutDependencyUpdateContext &context) noexcept;

    /// @brief The anchors each anchor-positioned element was last resolved against.
    KRYS_NODISCARD style::AnchorPositionedToAnchorMap &AnchorPositionedToAnchorMap() noexcept
    {
      return _anchorPositionedToAnchorMap;
    }
    KRYS_NODISCARD const style::AnchorPositionedToAnchorMap &AnchorPositionedToAnchorMap() const noexcept
    {
      return _anchorPositionedToAnchorMap;
    }
    void UpdateAnchorPositioningStateAfterStyleResolution() noexcept;

    /// @brief The index of the position option @p styleable last chose, from the last rendering update.
    /// @see https://drafts.csswg.org/css-anchor-position-1/#last-successful-position-option
    KRYS_NODISCARD Maybe<size_t>
      LastSuccessfulPositionOptionIndexFor(const Styleable &styleable) const noexcept;
    void SetLastSuccessfulPositionOptionIndexMap(
      HashMap<AnchorPositionedKey, size_t, AnchorPositionedKeyHash> &&map) noexcept;
    void ForgetLastSuccessfulPositionOptionIndex(const Styleable &styleable) noexcept;

  private:
    bool InvalidateForContainerDependencies(LayoutDependencyUpdateContext &context) noexcept;
    bool InvalidateForAnchorDependencies(LayoutDependencyUpdateContext &context) noexcept;
    bool InvalidateForPositionTryFallbacks(LayoutDependencyUpdateContext &context) noexcept;

    KRYS_NODISCARD Scope &DocumentScope() noexcept;
    KRYS_NODISCARD dom::DocumentOrShadowRoot &TreeScope() noexcept;

    void DidRemovePendingStylesheet() noexcept;

    void UpdateActiveStyleSheets(UpdateType updateType) noexcept;
    KRYS_NODISCARD StyleSheetChange
      AnalyzeStyleSheetChange(const List<RefPtr<css::CSSStyleSheet>> &newStylesheets) noexcept;
    void InvalidateStyleAfterStyleSheetChange(const StyleSheetChange &styleSheetChange) noexcept;
    void UpdateResolver(Span<const RefPtr<css::CSSStyleSheet>> activeStyleSheets,
                        ResolverUpdateType updateType) noexcept;
    void CreateDocumentResolver() noexcept;
    void CreateOrFindSharedShadowTreeResolver() noexcept;
    void UnshareShadowTreeResolverBeforeMutation() noexcept;
    KRYS_NODISCARD ResolverSharingKey MakeResolverSharingKey() noexcept;
    KRYS_NODISCARD bool IsForUserAgentShadowTree() const noexcept;
    void ScheduleUpdate(UpdateType update) noexcept;

    void FlushPendingSelfUpdate() noexcept;
    void FlushPendingDescendantUpdates() noexcept;
    void ClearPendingUpdate() noexcept;

    KRYS_NODISCARD ActiveStyleSheetCollection CollectActiveStyleSheets() noexcept;

    KRYS_NODISCARD ResolverScopes CollectResolverScopes() noexcept;

    template <typename TFunction>
    void EvaluateMediaQueries(TFunction &&testFunction) noexcept;

    KRYS_NODISCARD static MediaQueryViewportState
      MediaQueryViewportStateForDocument(const dom::Document &document) noexcept;
  };

  template <typename TFunction>
  auto Scope::ResolveTreeScopedReference(const dom::Element &element, const ScopedName &reference,
                                         const TFunction &function) noexcept
  {
    using ReturnType = std::invoke_result_t<TFunction, const Scope &, const css::CSSOMStringAtom &>;

    // https://drafts.csswg.org/css-scoping-1/#shadow-names
    // "Whenever a tree-scoped reference is dereferenced to find the CSS construct it is referencing, first
    // search only the tree-scoped names associated with the same root as the tree-scoped reference must be
    // searched."
    const auto *firstScope = Scope::ForOrdinal(element, reference.ScopeOrdinal);
    if (!firstScope)
    {
      return ReturnType {};
    }

    if (auto result = function(*firstScope, reference.Name))
    {
      return result;
    }

    // "If no relevant tree-scoped name is found, and the root is a shadow root, then repeat this search in
    // the root’s host’s node tree."
    for (const auto *hostScope = firstScope->HostScope(); hostScope; hostScope = hostScope->HostScope())
    {
      if (auto result = function(*hostScope, reference.Name))
      {
        return result;
      }
    }

    return ReturnType {};
  }

  /// @return The slot @p scopeOrdinal slots up the assignment chain from @p element, if any.
  KRYS_NODISCARD html::HTMLSlotElement *AssignedSlotForScopeOrdinal(const dom::Element &element,
                                                                    ScopeOrdinal scopeOrdinal) noexcept;

  /// @return The shadow host @p scopeOrdinal scopes up from @p element, if any.
  KRYS_NODISCARD dom::Element *HostForScopeOrdinal(const dom::Element &element,
                                                   ScopeOrdinal scopeOrdinal) noexcept;
}
