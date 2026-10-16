#pragma once

#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class RenderElement;
  class RenderObject;
  class RenderView;

  /// @brief Builds and tears down the render tree: attaches renderers to their parents, and detaches and
  /// destroys them.
  // FIXME: The per-type builders (block, block flow, inline, continuation, table, list, first letter, ...),
  // the anonymous box rules and tree normalization after style changes come in the next part of the render
  // tree.
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

  private:
    static RenderTreeBuilder *s_current;

    RenderView &_view;
    RenderTreeBuilder *_previous {nullptr};
    IsInternalMove _internalMovesType {IsInternalMove::No};
    TearDownType _tearDownType {TearDownType::Root};

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

    void DestroyAndCleanUpAnonymousWrappers(RenderObject &rendererToDestroy,
                                            const RenderElement *subtreeDestroyRoot) noexcept;

  private:
    void AttachInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                        RenderObject *beforeChild) noexcept;
    void AttachToRenderElement(RenderElement &parent, RenderPtr<RenderObject> child,
                               RenderObject *beforeChild = nullptr) noexcept;
    void AttachToRenderElementInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                                       RenderObject *beforeChild = nullptr) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject> DetachFromRenderElement(RenderElement &parent, RenderObject &child,
                                                                   WillBeDestroyed willBeDestroyed) noexcept;
  };
}
