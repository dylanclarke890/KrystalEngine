#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/SelectorMatchingState.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/TreeResolutionState.hpp"
#include "Krystal.Booey/Style/Update.hpp"
#include "Krystal.Booey/Style/Validity.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
  class ShadowRoot;
}

namespace krys::boo::style
{
  enum class IsInDisplayNoneTree : bool
  {
    No,
    Yes
  };

  /// @brief Resolves the styles of a document's elements that need resolving, walking the composed tree.
  // FIXME: Animations (the animated updates, @starting-style, after-change styles), position options and
  // position visibility, accessibility updates and elements with custom style resolve callbacks.
  class TreeResolver
  {
    KRYS_NON_COPY_MOVABLE(TreeResolver);

    enum class ResolutionType : uint8
    {
      RebuildUsingExisting,
      AnimationOnly,
      FastPathInherit,
      FullWithMatchResultCache,
      Full
    };

    enum class LayoutInterleavingAction : uint8
    {
      None,
      SkipDescendants
    };

    enum class DescendantsToResolve : uint8
    {
      None,
      RebuildAllUsingExisting,
      ChildrenWithExplicitInherit,
      Children,
      All
    };

    struct Scope : RefCounted<Scope>
    {
      Ref<style::Resolver> Resolver;
      style::SelectorMatchingState SelectorMatchingState;
      RefPtr<dom::ShadowRoot> ShadowRoot;
      RefPtr<Scope> EnclosingScope;

      Scope(dom::Document &document, Update &update) noexcept;
      Scope(dom::ShadowRoot &shadowRoot, Scope &enclosingScope) noexcept;
      ~Scope() noexcept;
    };

    struct Parent
    {
      dom::Element *Element;
      const RenderStyle &Style;
      EnumSet<Change> Changes;
      TreeResolver::DescendantsToResolve DescendantsToResolve {TreeResolver::DescendantsToResolve::None};
      bool DidPushScope {false};
      bool ResolvedFirstLineAndLetterChild {false};
      bool NeedsUpdateQueryContainerDependentStyle {false};
      style::IsInDisplayNoneTree IsInDisplayNoneTree {style::IsInDisplayNoneTree::No};

      explicit Parent(dom::Document &document) noexcept;
      Parent(dom::Element &element, const RenderStyle &style, EnumSet<Change> changes,
             TreeResolver::DescendantsToResolve descendantsToResolve,
             style::IsInDisplayNoneTree isInDisplayNoneTree) noexcept;
    };

    // States relevant to deferring and resuming descendant resolution.
    // Also see deferDescendantResolution and resumeDescendantResolutionIfNeeded.
    struct DeferredDescendantResolutionState
    {
      EnumSet<Change> Changes;
      TreeResolver::DescendantsToResolve DescendantsToResolve {TreeResolver::DescendantsToResolve::None};
    };

    struct QueryContainerState
    {
      bool Invalidated {false};
    };

  private:
    Ref<dom::Document> _document;
    UniquePtr<RenderStyle> _computedDocumentElementStyle;

    List<Ref<Scope>> _scopeStack;
    List<Parent> _parentStack;
    bool _didSeePendingStylesheet {false};

    HashMap<Ref<dom::Element>, DeferredDescendantResolutionState> _deferredDescendantResolutionStates;
    bool _needsInterleavedLayout {false};
    bool _didFirstInterleavedLayout {false};

    HashMap<Ref<dom::Element>, QueryContainerState> _queryContainerStates;

    // This state gets passes to the style builder and holds state for a single tree resolution, including
    // over any interleaving.
    TreeResolutionState _treeResolutionState;
    HashMap<Ref<const dom::Element>, UniquePtr<RenderStyle>> _savedBeforeResolutionStylesForInterleaving;

    HashSet<css::CSSOMStringAtom> _changedAnchorNames;
    bool _allAnchorNamesInvalid {false};

    UniquePtr<Update> _update;

  public:
    explicit TreeResolver(dom::Document &document, UniquePtr<Update> update = {}) noexcept;
    ~TreeResolver() noexcept;

    /// @brief Resolves the styles that need resolving.
    /// @return The update to apply to the render tree, or null if nothing changed.
    KRYS_NODISCARD UniquePtr<Update> Resolve() noexcept;

    KRYS_NODISCARD bool NeedsInterleavedLayout() const noexcept
    {
      return _needsInterleavedLayout;
    }

  private:
    KRYS_NODISCARD ResolvedStyle StyleForStyleable(const Styleable &styleable, ResolutionType resolutionType,
                                                   const ResolutionContext &resolutionContext,
                                                   const RenderStyle *existingStyle) noexcept;

    void ResolveComposedTree() noexcept;

    KRYS_NODISCARD const RenderStyle *ExistingStyle(const dom::Element &element) noexcept;

    KRYS_NODISCARD LayoutInterleavingAction UpdateStateForQueryContainer(
      dom::Element &element, const RenderStyle *style, DescendantsToResolve &descendantsToResolve) noexcept;
    KRYS_NODISCARD LayoutInterleavingAction UpdateAnchorPositioningState(dom::Element &element,
                                                                         const RenderStyle *style) noexcept;

    // For elements requiring style/layout interleaving (anchor-positioned and query
    // containers), descendant resolution is deferred on the first style resolution
    // pass and resumed in subsequent passes.
    // When deferral is needed, deferDescendantResolution saves the internal state
    // relevant to which descendants should be resolved.
    void DeferDescendantResolution(dom::Element &element, EnumSet<Change> changes,
                                   DescendantsToResolve descendantsToResolve) noexcept;
    // When descendant resolution can be resumed, resumeDescendantResolutionIfNeeded
    // restores the previously saved state (if there is)
    void ResumeDescendantResolutionIfNeeded(dom::Element &element, EnumSet<Change> &changes,
                                            DescendantsToResolve &descendantsToResolve) noexcept;

    KRYS_NODISCARD Pair<ElementUpdate, DescendantsToResolve>
      ResolveElement(dom::Element &element, const RenderStyle *existingStyle,
                     ResolutionType resolutionType) noexcept;

    KRYS_NODISCARD ElementUpdate CreateAnimatedElementUpdate(
      ResolvedStyle &&resolvedStyle, const Styleable &styleable, EnumSet<Change> parentChanges,
      const ResolutionContext &resolutionContext,
      style::IsInDisplayNoneTree isInDisplayNoneTree = style::IsInDisplayNoneTree::No) noexcept;

    KRYS_NODISCARD UniquePtr<RenderStyle>
      ResolveStartingStyle(const ResolvedStyle &resolvedStyle, const Styleable &styleable,
                           const ResolutionContext &resolutionContext) noexcept;
    KRYS_NODISCARD UniquePtr<RenderStyle>
      ResolveAfterChangeStyleForNonAnimated(const ResolvedStyle &resolvedStyle, const Styleable &styleable,
                                            const ResolutionContext &resolutionContext) noexcept;
    KRYS_NODISCARD UniquePtr<RenderStyle> ResolveAgainInDifferentContext(
      const ResolvedStyle &resolvedStyle, const Styleable &styleable, const RenderStyle &parentStyle,
      EnumSet<PropertyCascade::PropertyType> properties, const ResolutionContext &resolutionContext) noexcept;
    KRYS_NODISCARD const RenderStyle &
      ParentAfterChangeStyle(const Styleable &styleable,
                             const ResolutionContext &resolutionContext) const noexcept;
    KRYS_NODISCARD HashSet<AnimatableCSSProperty>
      ApplyCascadeAfterAnimation(RenderStyle &animatedStyle,
                                 const AnimatableCSSPropertySet &animatedProperties, bool isTransition,
                                 const MatchResult &matchResult, const dom::Element &element,
                                 const ResolutionContext &resolutionContext) noexcept;

    KRYS_NODISCARD Maybe<ElementUpdate>
      ResolvePseudoElement(dom::Element &element, const PseudoElementIdentifier &pseudoElementIdentifier,
                           const ElementUpdate &elementUpdate, style::IsInDisplayNoneTree isInDisplayNoneTree,
                           const RenderStyle *existingStyle) noexcept;
    KRYS_NODISCARD Maybe<ElementUpdate>
      ResolveAncestorPseudoElement(dom::Element &element,
                                   const PseudoElementIdentifier &pseudoElementIdentifier,
                                   const ElementUpdate &elementUpdate) noexcept;
    KRYS_NODISCARD Maybe<ResolvedStyle>
      ResolveAncestorFirstLinePseudoElement(dom::Element &element,
                                            const ElementUpdate &elementUpdate) noexcept;
    KRYS_NODISCARD Maybe<ResolvedStyle>
      ResolveAncestorFirstLetterPseudoElement(dom::Element &element, const ElementUpdate &elementUpdate,
                                              ResolutionContext &resolutionContext) noexcept;

    void ResetStyleForNonRenderedDescendants(dom::Element &subtreeRoot) noexcept;

    KRYS_NODISCARD Scope &GetScope() noexcept
    {
      return *_scopeStack.back();
    }

    KRYS_NODISCARD const Scope &GetScope() const noexcept
    {
      return *_scopeStack.back();
    }

    KRYS_NODISCARD Parent &GetParent() noexcept
    {
      return _parentStack.back();
    }

    KRYS_NODISCARD const Parent &GetParent() const noexcept
    {
      return _parentStack.back();
    }

    void PushScope(dom::ShadowRoot &shadowRoot) noexcept;
    void PushEnclosingScope() noexcept;
    void PopScope() noexcept;

    void PushParent(dom::Element &element, const RenderStyle &style, EnumSet<Change> changes,
                    DescendantsToResolve descendantsToResolve,
                    style::IsInDisplayNoneTree isInDisplayNoneTree) noexcept;
    void PopParent() noexcept;
    void PopParentsToDepth(uint32 depth) noexcept;

    KRYS_NODISCARD DescendantsToResolve ComputeDescendantsToResolve(const ElementUpdate &update,
                                                                    const RenderStyle *existingStyle,
                                                                    Validity validity) const noexcept;
    KRYS_NODISCARD static Maybe<ResolutionType>
      DetermineResolutionType(const dom::Element &element, const RenderStyle *existingStyle,
                              DescendantsToResolve parentDescendantsToResolve,
                              EnumSet<Change> parentChanges) noexcept;
    static void ResetDescendantStyleRelations(dom::Element &element,
                                              DescendantsToResolve descendantsToResolve) noexcept;

    KRYS_NODISCARD ResolutionContext MakeResolutionContext() noexcept;
    KRYS_NODISCARD ResolutionContext MakeResolutionContextForPseudoElement(
      const ElementUpdate &elementUpdate, const PseudoElementIdentifier &pseudoElementIdentifier) noexcept;
    KRYS_NODISCARD Maybe<ResolutionContext>
      MakeResolutionContextForInheritedFirstLine(const ElementUpdate &elementUpdate,
                                                 const RenderStyle &inheritStyle) noexcept;
    KRYS_NODISCARD const Parent *BoxGeneratingParent() const noexcept;
    KRYS_NODISCARD const RenderStyle *ParentBoxStyle() const noexcept;
    KRYS_NODISCARD const RenderStyle *
      ParentBoxStyleForPseudoElement(const ElementUpdate &elementUpdate) const noexcept;
    KRYS_NODISCARD const RenderStyle *DocumentElementStyle() const noexcept;

    // This returns the style that was in effect (applied to the render tree) before we started the style
    // resolution. Layout interleaving may cause different styles to be applied during the style resolution.
    KRYS_NODISCARD const RenderStyle *
      BeforeResolutionStyle(const dom::Element &element,
                            const Maybe<PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;
    void SaveBeforeResolutionStyleForInterleaving(const dom::Element &element) noexcept;

    KRYS_NODISCARD bool HasUnresolvedAnchorPosition(const Styleable &styleable) const noexcept;
    KRYS_NODISCARD bool HasResolvedAnchorPosition(const Styleable &styleable) const noexcept;

    void CollectChangedAnchorNames(const RenderStyle &newStyle, const RenderStyle *currentStyle) noexcept;
  };

  // Integrate with the HTML5 event loop instead, see EventLoop.cpp and consumers.
  void DeprecatedQueuePostResolutionCallback(Func<void()> &&callback) noexcept;
  KRYS_NODISCARD bool PostResolutionCallbacksAreSuspended() noexcept;

  KRYS_NODISCARD inline bool SupportsFirstLineAndLetterPseudoElement(const RenderStyle &style) noexcept
  {
    auto display = style.Display();
    return display == DisplayType::Block || display == DisplayType::ListItem
           || display == DisplayType::InlineBlock || display == DisplayType::TableCell
           || display == DisplayType::TableCaption || display == DisplayType::FlowRoot;
  }

  /// @brief Suspends the callbacks queued with `DeprecatedQueuePostResolutionCallback` until the outermost
  /// disabler goes away, which runs them.
  class PostResolutionCallbackDisabler
  {
    KRYS_NON_COPY_MOVABLE(PostResolutionCallbackDisabler);

  public:
    enum class DrainCallbacks : bool
    {
      No,
      Yes
    };

  private:
    DrainCallbacks _drainCallbacks;

  public:
    explicit PostResolutionCallbackDisabler(dom::Document &document,
                                            DrainCallbacks drainCallbacks = DrainCallbacks::Yes) noexcept;
    ~PostResolutionCallbackDisabler() noexcept;
  };
}
