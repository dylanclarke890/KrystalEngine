#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class RenderBlockFlow;

  /// @brief Attaches children to blocks, wrapping inline children of blocks with block children in anonymous
  /// blocks, and detaches them, merging and dropping the anonymous blocks left behind.
  class RenderTreeBuilder::Block
  {
    KRYS_NON_COPY_MOVABLE(Block);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit Block(RenderTreeBuilder &builder) noexcept;

    void Attach(RenderBlock &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void AttachIgnoringContinuation(RenderBlock &parent, RenderPtr<RenderObject> child,
                                    RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject>
      Detach(RenderBlock &parent, RenderObject &child, WillBeDestroyed willBeDestroyed,
             CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject>
      Detach(RenderBlockFlow &parent, RenderObject &child, WillBeDestroyed willBeDestroyed,
             CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept;

    void DropAnonymousBoxChild(RenderBlock &parent, RenderBlock &child) noexcept;
    void ChildBecameNonInline(RenderBlock &parent, RenderElement &child) noexcept;

    KRYS_NODISCARD static RenderPtr<RenderBlockFlow>
      CreateAnonymousBlockWithStyle(dom::Document &document, const RenderStyle &style) noexcept;

  private:
    static void MoveAllChildrenToInternal(RenderBoxModelObject &from, RenderElement &newParent) noexcept;

    void RemoveLeftoverAnonymousBlock(RenderBlock &anonymousBlock) noexcept;
  };
}
