#pragma once

#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"

namespace krys::boo
{
  class RenderLayer;
  class RenderInline;
  class RenderLayerModelObject;
  class RenderObject;

  /*
   * Paint the object and its children, clipped by (x|y|w|h).
   * (tx|ty) is the calculated position of the parent
   */
  /// FIXME: Overlap test requests (for plug-ins) and event and accessibility regions (`regionContext`) aren't
  /// ported.
  struct PaintInfo
  {
    LayoutRect Rect;
    PaintPhase Phase;
    EnumSet<boo::PaintBehavior> PaintBehavior;
    RenderObject *SubtreePaintRoot;               // used to draw just one element and its visual children
    const RenderLayerModelObject *PaintContainer; // the layer object that originates the current painting
    /// @brief The inlines whose outlines the inline content being painted paints after its boxes.
    ListHashSet<const RenderInline *> *OutlineObjects {nullptr};
    bool RequireSecurityOriginAccessForWidgets {false};

  private:
    const RenderLayer *_enclosingSelfPaintingLayer {nullptr};
    GraphicsContext *_context;

  public:
    PaintInfo(GraphicsContext &newContext, const LayoutRect &newRect, PaintPhase newPhase,
              EnumSet<boo::PaintBehavior> newPaintBehavior, RenderObject *newSubtreePaintRoot = nullptr,
              const RenderLayerModelObject *newPaintContainer = nullptr,
              const RenderLayer *enclosingSelfPaintingLayer = nullptr,
              bool newRequireSecurityOriginAccessForWidgets = false) noexcept
        : Rect(newRect), Phase(newPhase), PaintBehavior(newPaintBehavior),
          SubtreePaintRoot(newSubtreePaintRoot), PaintContainer(newPaintContainer),
          RequireSecurityOriginAccessForWidgets(newRequireSecurityOriginAccessForWidgets),
          _enclosingSelfPaintingLayer(enclosingSelfPaintingLayer), _context(&newContext)
    {
    }

    KRYS_NODISCARD GraphicsContext &Context() const noexcept
    {
      krys_debug_assert(_context);
      return *_context;
    }

    void SetContext(GraphicsContext &context) noexcept
    {
      _context = &context;
    }

    void UpdateSubtreePaintRootForChildren(const RenderObject *renderer) noexcept
    {
      if (!SubtreePaintRoot)
      {
        return;
      }

      // If we're the painting root, kids draw normally, and see root of nullptr.
      if (SubtreePaintRoot == renderer)
      {
        SubtreePaintRoot = nullptr;
        return;
      }
    }

    KRYS_NODISCARD bool ShouldPaintWithinRoot(const RenderObject &renderer) const noexcept
    {
      return !SubtreePaintRoot || SubtreePaintRoot == &renderer;
    }

    KRYS_NODISCARD bool ForceTextColor() const noexcept
    {
      return ForceBlackText() || ForceWhiteText();
    }
    KRYS_NODISCARD bool ForceBlackText() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::ForceBlackText);
    }
    KRYS_NODISCARD bool ForceWhiteText() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::ForceWhiteText);
    }
    KRYS_NODISCARD Color ForcedTextColor() const noexcept
    {
      return ForceBlackText() ? Color::black : Color::white;
    }

    KRYS_NODISCARD bool SkipRootBackground() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::SkipRootBackground);
    }
    KRYS_NODISCARD bool PaintRootBackgroundOnly() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::RootBackgroundOnly);
    }

    KRYS_NODISCARD const RenderLayer *EnclosingSelfPaintingLayer() const noexcept
    {
      return _enclosingSelfPaintingLayer;
    }

    void ApplyTransform(const AffineTransform &localToAncestorTransform) noexcept
    {
      if (localToAncestorTransform.IsIdentity())
      {
        return;
      }

      Context().ConcatCTM(localToAncestorTransform);

      if (Rect.IsInfinite())
      {
        return;
      }

      FloatRect transformedRect(
        localToAncestorTransform.Inverse().value_or(AffineTransform {}).MapRect(FloatRect(Rect)));
      Rect.SetLocation(LayoutPoint(transformedRect.Location()));
      Rect.SetSize(LayoutSize(transformedRect.Size()));
    }
  };
}
