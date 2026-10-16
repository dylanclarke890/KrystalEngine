#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
}

namespace krys::boo
{
  enum class RenderElementConstructBlockLevelRendererFor : uint8
  {
    Inline,
    ListItem,
    TableOrTablePart
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderElementConstructBlockLevelRendererFor, 3uz);

namespace krys::boo
{
  class RenderBlock;

  /// @brief A renderer with its own style: an element's, or an anonymous box's.
  class RenderElement : public RenderObject
  {
    friend class RenderTreeBuilder;
    KRYS_NON_COPY_MOVABLE(RenderElement);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderElement);

  private:
    RenderStyle _style;
    RenderObject *_firstChild {nullptr};
    RenderObject *_lastChild {nullptr};
    bool _hasInitializedStyle {false};

  public:
    /// @brief Displays that get a block flow instead of their usual renderer.
    using ConstructBlockLevelRendererFor = RenderElementConstructBlockLevelRendererFor;

    ~RenderElement() noexcept override;

    /// @brief The renderer @p element gets with @p style, or null if @p style's display doesn't generate a
    /// box.
    /// @param rendererTypeOverride The displays that get a block flow instead.
    KRYS_NODISCARD static RenderPtr<RenderElement>
      CreateFor(dom::Element &element, RenderStyle &&style,
                EnumSet<ConstructBlockLevelRendererFor> rendererTypeOverride = {}) noexcept;

    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return _style;
    }

    KRYS_NODISCARD RenderStyle &MutableStyle() noexcept
    {
      return _style;
    }

    KRYS_NODISCARD bool HasInitializedStyle() const noexcept
    {
      return _hasInitializedStyle;
    }

    /// @brief Takes the style the renderer was created with as its first style.
    void InitializeStyle() noexcept;

    /// @brief Replaces the renderer's style, doing at least @p minimalStyleDifference of the work a style
    /// change needs.
    /// @note FIXME: The difference doesn't invalidate layout or painting yet (no layout, painting or layers).
    void SetStyle(RenderStyle &&style,
                  style::DifferenceResult minimalStyleDifference = style::DifferenceResult::Equal) noexcept;

    /// @brief The element rendered, or null for an anonymous box.
    KRYS_NODISCARD dom::Element *GetElement() const noexcept;

    KRYS_NODISCARD RenderObject *FirstChild() const noexcept
    {
      return _firstChild;
    }
    KRYS_NODISCARD RenderObject *LastChild() const noexcept
    {
      return _lastChild;
    }

    KRYS_NODISCARD RenderObject *FirstInFlowChild() const noexcept;
    KRYS_NODISCARD RenderObject *LastInFlowChild() const noexcept;

    KRYS_NODISCARD RenderObject *FirstChildSlow() const noexcept final
    {
      return FirstChild();
    }
    KRYS_NODISCARD RenderObject *LastChildSlow() const noexcept final
    {
      return LastChild();
    }

    /// @brief Whether @p child, with @p style, may be a child of this renderer.
    KRYS_NODISCARD virtual bool IsChildAllowed(const RenderObject &, const RenderStyle &) const noexcept
    {
      return true;
    }

    void DidAttachChild(RenderObject &child, RenderObject *beforeChild) noexcept;

    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    KRYS_NODISCARD bool IsFixedPositioned() const noexcept;
    KRYS_NODISCARD bool IsAbsolutelyPositioned() const noexcept;

    KRYS_NODISCARD bool IsFlexItemIncludingDeprecated() const noexcept
    {
      return !IsInline() && !IsFloatingOrOutOfFlowPositioned() && Parent()
             && Parent()->IsFlexibleBoxIncludingDeprecated();
    }

    // block-level boxes are boxes that participate in a block formatting context.
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept;
    KRYS_NODISCARD bool IsAnonymousBlock() const noexcept;
    KRYS_NODISCARD bool IsBlockContainer() const noexcept;
    // A block-level box that is also a block container.
    KRYS_NODISCARD bool IsBlockBox() const noexcept;
    KRYS_NODISCARD bool IsBeforeContent() const noexcept;
    KRYS_NODISCARD bool IsAfterContent() const noexcept;
    KRYS_NODISCARD bool IsBeforeOrAfterContent() const noexcept;
    KRYS_NODISCARD static bool IsBeforeContent(const RenderObject *renderer) noexcept;
    KRYS_NODISCARD static bool IsAfterContent(const RenderObject *renderer) noexcept;

    /// @brief The element whose content this renderer renders: the host of a pseudo-element.
    KRYS_NODISCARD dom::Element *GeneratingElement() const noexcept;

    KRYS_NODISCARD Overflow EffectiveOverflowX() const noexcept;
    KRYS_NODISCARD Overflow EffectiveOverflowY() const noexcept;
    KRYS_NODISCARD Overflow EffectiveOverflowBlockDirection() const noexcept;
    KRYS_NODISCARD Overflow EffectiveOverflowInlineDirection() const noexcept;
    KRYS_NODISCARD bool HasPotentiallyScrollableOverflow() const noexcept;

    KRYS_NODISCARD bool HasClip() const noexcept;
    KRYS_NODISCARD bool HasClipOrNonVisibleOverflow() const noexcept;

    KRYS_NODISCARD bool CanEstablishContainingBlockWithTransform() const noexcept;
    KRYS_NODISCARD bool
      MayContainOutOfFlowPositionedObjects(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool
      CanContainFixedPositionObjects(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool
      CanContainAbsolutelyPositionedObjects(const RenderStyle *styleToUse = nullptr) const noexcept;

    KRYS_NODISCARD bool ShouldApplyAnyContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplySizeOrInlineSizeContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyLayoutContainment(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool ShouldApplySizeContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyInlineSizeContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyStyleContainment() const noexcept;
    KRYS_NODISCARD bool ShouldApplyPaintContainment(const RenderStyle *styleToUse = nullptr) const noexcept;
    KRYS_NODISCARD bool HasEligibleContainmentForSizeQuery() const noexcept;

    // FIXME: Layout queries, reachable only through `RendererOf` and `RenderBoxOf`, which return null until
    // there's layout.
    KRYS_NODISCARD FloatRect TransformReferenceBoxRect(const RenderStyle &) const noexcept;

  protected:
    RenderElement(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                  TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderElement(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                  TypeSpecificFlags typeSpecificFlags) noexcept;

    void WillBeDestroyed() noexcept override;

    // FIXME: Most of the work upstream does here (repaint, layout invalidation, layers, images, counters,
    // outlines, scroll anchoring, line layout) comes with layout and painting.
    virtual void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept;
    virtual void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept;

  private:
    RenderElement(Type type, dom::ContainerNode &elementOrDocument, RenderStyle &&style,
                  EnumSet<TypeFlag> flags, TypeSpecificFlags typeSpecificFlags) noexcept;

    // Called by RenderTreeBuilder: they take and give up ownership of the child.
    RenderObject *AttachRendererInternal(RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> DetachRendererInternal(RenderObject &renderer) noexcept;

    KRYS_NODISCARD style::Difference AdjustStyleDifference(style::Difference diff) const noexcept;
    KRYS_NODISCARD bool IsHTMLMarquee() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
