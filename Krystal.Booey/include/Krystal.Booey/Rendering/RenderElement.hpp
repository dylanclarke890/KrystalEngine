#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
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
    KRYS_NON_COPY_MOVABLE(RenderElement);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderElement);

  private:
    RenderStyle _style;
    bool _hasInitializedStyle {false};

  public:
    /// @brief Displays that get a block flow instead of their usual renderer.
    using ConstructBlockLevelRendererFor = RenderElementConstructBlockLevelRendererFor;

    ~RenderElement() noexcept override;

    /// @brief The renderer @p element gets with @p style, or null if @p style's display doesn't generate a
    /// box.
    /// @param rendererTypeOverride The displays that get a block flow instead.
    /// @note FIXME: Until the renderer classes are ported, the renderer is a plain `RenderElement` of the
    /// type its class would have, without that class's type flags.
    KRYS_NODISCARD static UniquePtr<RenderElement>
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

    // FIXME: Layout queries. They're only reachable through `RendererOf` and `RenderBoxOf`, which return null
    // until there's layout.
    KRYS_NODISCARD const RenderElement *Container() const noexcept;
    KRYS_NODISCARD const RenderBlock *ContainingBlock() const noexcept;
    KRYS_NODISCARD bool IsInline() const noexcept;
    KRYS_NODISCARD bool IsBlockLevelReplacedOrAtomicInline() const noexcept;
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept;
    KRYS_NODISCARD bool IsFlexItem() const noexcept;
    KRYS_NODISCARD bool IsGridItem() const noexcept;
    KRYS_NODISCARD bool IsHorizontalWritingMode() const noexcept;
    KRYS_NODISCARD FloatRect TransformReferenceBoxRect(const RenderStyle &) const noexcept;

  protected:
    RenderElement(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                  TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderElement(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                  TypeSpecificFlags typeSpecificFlags) noexcept;

  private:
    RenderElement(Type type, dom::ContainerNode &elementOrDocument, RenderStyle &&style,
                  EnumSet<TypeFlag> flags, TypeSpecificFlags typeSpecificFlags) noexcept;

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
