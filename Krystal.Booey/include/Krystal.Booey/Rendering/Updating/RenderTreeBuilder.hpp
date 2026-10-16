#pragma once

#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBox;
  class RenderBoxModelObject;
  class RenderElement;
  class RenderGrid;
  class RenderObject;
  class RenderStyle;
  class RenderView;

  /// @brief Builds and tears down the render tree: attaches renderers to their parents, wrapping them in the
  /// anonymous boxes the CSS box tree needs, and detaches and destroys them, cleaning the wrappers up.
  // FIXME: The multi-column, ruby, form control, SVG and MathML builders come with their renderers. There are
  // no continuations: blocks in inlines are laid out in inline layout
  // (`blocksInInlineLayoutEnabled`, on by default upstream).
  class RenderTreeBuilder
  {
    KRYS_NON_COPY_MOVABLE(RenderTreeBuilder);

  public:
    enum class IsInternalMove : bool
    {
      No,
      Yes
    };
    enum class WillBeDestroyed : bool
    {
      No,
      Yes
    };
    enum class CanCollapseAnonymousBlock : bool
    {
      No,
      Yes
    };
    enum class TearDownType : uint8
    {
      Root,                          // destroy root renderer
      SubtreeWithRootStillAttached,  // subtree teardown when renderers are still attached to the tree (common
                                     // case)
      SubtreeWithRootAlreadyDetached // subtree teardown when destroy root gets detached first followed by
                                     // destroying renderers (e.g. pseudo subtree)
    };
    // NormalizeAfterInsertion::Yes ensures that the destination subtree is consistent after the insertion
    // (anonymous wrappers etc).
    enum class NormalizeAfterInsertion : bool
    {
      No,
      Yes
    };

    class Block;
    class BlockFlow;
    class FirstLetter;
    class Inline;
    class List;
    class Table;

  private:
    static RenderTreeBuilder *s_current;

    RenderView &_view;
    RenderTreeBuilder *_previous {nullptr};
    UniquePtr<Table> _tableBuilder;
    UniquePtr<Block> _blockBuilder;
    UniquePtr<BlockFlow> _blockFlowBuilder;
    UniquePtr<Inline> _inlineBuilder;
    UniquePtr<FirstLetter> _firstLetterBuilder;
    UniquePtr<List> _listBuilder;
    IsInternalMove _internalMovesType {IsInternalMove::No};
    TearDownType _tearDownType {TearDownType::Root};
    const RenderElement *_subtreeDestroyRoot {nullptr};
    // Note that _anonymousDestroyRoot is not necessarily the root of the subtree being destroyed (it's the
    // root of the anonymous subtree, if any).
    const RenderObject *_anonymousDestroyRoot {nullptr};

  public:
    explicit RenderTreeBuilder(RenderView &view) noexcept;
    ~RenderTreeBuilder() noexcept;

    // This avoids having to convert all sites that need RenderTreeBuilder in one go.
    // FIXME: Remove.
    KRYS_NODISCARD static RenderTreeBuilder *Current() noexcept
    {
      return s_current;
    }

    KRYS_NODISCARD const RenderView &View() const noexcept
    {
      return _view;
    }

    KRYS_NODISCARD static bool IsRebuildRootForChildren(const RenderElement &renderer) noexcept;

    void Attach(RenderElement &parent, RenderPtr<RenderObject> child,
                RenderObject *beforeChild = nullptr) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject>
      Detach(RenderElement &parent, RenderObject &child, WillBeDestroyed willBeDestroyed,
             CanCollapseAnonymousBlock canCollapseAnonymousBlock = CanCollapseAnonymousBlock::Yes) noexcept;

    void
      Destroy(RenderObject &renderer,
              CanCollapseAnonymousBlock canCollapseAnonymousBlock = CanCollapseAnonymousBlock::Yes) noexcept;

    void Move(RenderBoxModelObject &from, RenderBoxModelObject &to, RenderObject &child,
              NormalizeAfterInsertion normalizeAfterInsertion) noexcept;

    void UpdateAfterDescendants(RenderElement &renderer) noexcept;
    void DestroyAndCleanUpAnonymousWrappers(RenderObject &rendererToDestroy,
                                            const RenderElement *subtreeDestroyRoot) noexcept;
    void NormalizeTreeAfterStyleChange(RenderElement &renderer, RenderStyle &oldStyle) noexcept;

  private:
    static void MarkBoxForRelayoutAfterSplit(RenderBoxModelObject &box) noexcept;

    void AttachInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                        RenderObject *beforeChild) noexcept;

    void ChildFlowStateChangesAndAffectsParentBlock(RenderElement &child) noexcept;
    void ChildFlowStateChangesAndNoLongerAffectsParentBlock(RenderElement &child) noexcept;
    void AttachIgnoringContinuation(RenderElement &parent, RenderPtr<RenderObject> child,
                                    RenderObject *beforeChild = nullptr) noexcept;
    void AttachToRenderGrid(RenderGrid &parent, RenderPtr<RenderObject> child,
                            RenderObject *beforeChild = nullptr) noexcept;
    void AttachToRenderElement(RenderElement &parent, RenderPtr<RenderObject> child,
                               RenderObject *beforeChild = nullptr) noexcept;
    void AttachToRenderElementInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                                       RenderObject *beforeChild = nullptr) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject> DetachFromRenderElement(RenderElement &parent, RenderObject &child,
                                                                   WillBeDestroyed willBeDestroyed) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> DetachFromRenderGrid(RenderGrid &parent, RenderObject &child,
                                                                WillBeDestroyed willBeDestroyed) noexcept;

    void Move(RenderBoxModelObject &from, RenderBoxModelObject &to, RenderObject &child,
              RenderObject *beforeChild, NormalizeAfterInsertion normalizeAfterInsertion) noexcept;
    // Move all of the kids from |startChild| up to but excluding |endChild|. 0 can be passed as the
    // |endChild| to denote that all the kids from |startChild| onwards should be moved.
    void MoveChildren(RenderBoxModelObject &from, RenderBoxModelObject &to, RenderObject *startChild,
                      RenderObject *endChild, NormalizeAfterInsertion normalizeAfterInsertion) noexcept;
    void MoveChildren(RenderBoxModelObject &from, RenderBoxModelObject &to, RenderObject *startChild,
                      RenderObject *endChild, RenderObject *beforeChild,
                      NormalizeAfterInsertion normalizeAfterInsertion) noexcept;
    void MoveAllChildrenIncludingFloats(RenderBlock &from, RenderBlock &toBlock,
                                        NormalizeAfterInsertion normalizeAfterInsertion) noexcept;
    void MoveAllChildren(RenderBoxModelObject &from, RenderBoxModelObject &to,
                         NormalizeAfterInsertion normalizeAfterInsertion) noexcept;
    void MoveAllChildren(RenderBoxModelObject &from, RenderBoxModelObject &to, RenderObject *beforeChild,
                         NormalizeAfterInsertion normalizeAfterInsertion) noexcept;

    void RemoveFloatingObjects(RenderBlock &renderer) noexcept;

    KRYS_NODISCARD RenderObject *SplitAnonymousBoxesAroundChild(RenderBoxModelObject &parent,
                                                                RenderObject &originalBeforeChild) noexcept;
    void CreateAnonymousWrappersForInlineContent(RenderBlock &parent,
                                                 RenderObject *insertionPoint = nullptr) noexcept;
    void RemoveAnonymousWrappersForInlineChildrenIfNeeded(RenderElement &parent) noexcept;

    KRYS_NODISCARD static RenderPtr<RenderBox>
      CreateAnonymousBoxWithSameTypeAndWithStyle(const RenderBox &renderer,
                                                 const RenderStyle &style) noexcept;

    KRYS_NODISCARD Table &TableBuilder() noexcept
    {
      return *_tableBuilder;
    }
    KRYS_NODISCARD Block &BlockBuilder() noexcept
    {
      return *_blockBuilder;
    }
    KRYS_NODISCARD BlockFlow &BlockFlowBuilder() noexcept
    {
      return *_blockFlowBuilder;
    }
    KRYS_NODISCARD Inline &InlineBuilder() noexcept
    {
      return *_inlineBuilder;
    }
    KRYS_NODISCARD FirstLetter &FirstLetterBuilder() noexcept
    {
      return *_firstLetterBuilder;
    }
    KRYS_NODISCARD List &ListBuilder() noexcept
    {
      return *_listBuilder;
    }
  };
}
