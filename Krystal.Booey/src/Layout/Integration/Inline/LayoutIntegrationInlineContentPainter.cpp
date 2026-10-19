#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContentPainter.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/InlineBoxPainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/TextBoxPainter.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout_integration
{
  InlineContentPainter::InlineContentPainter(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                             const RenderInline *inlineBoxWithLayer,
                                             const InlineContent &inlineContent,
                                             const RenderBlockFlow &root) noexcept
      : _paintInfo(paintInfo), _paintOffset(paintOffset), _inlineBoxWithLayer(inlineBoxWithLayer),
        _inlineContent(inlineContent), _root(root)
  {
    _damageRect = _paintInfo.Rect;
    _damageRect.MoveBy(-_paintOffset);
  }

  void InlineContentPainter::PaintDisplayBox(const inline_display::Box &box) noexcept
  {
    auto hasDamage = [&](auto &box)
    {
      auto rect = EnclosingLayoutRect(box.InkOverflow());
      Root().FlipForWritingMode(rect);
      // FIXME: This should test for intersection but horizontal ink overflow is miscomputed in a few cases
      // (like with negative letter-spacing).
      return _damageRect.MaxY() > rect.Y() && _damageRect.Y() < rect.MaxY();
    };

    if (box.IsFullyTruncated())
    {
      // Fully truncated boxes are visually empty and they don't show their descendants either (unlike
      // visibility property).
      return;
    }

    if (box.IsLineBreak())
    {
      // FIXME: Accessibility bounds come with accessibility.
      return;
    }

    if (box.IsInlineBox())
    {
      if (!box.IsVisible() || !hasDamage(box))
      {
        return;
      }
      // Don't paint inline boxes wrapping block-in-inline.
      if (_inlineContent.IsInlineBoxWrapperForBlockLevelBox(box))
      {
        return;
      }

      auto canSkipInlineBoxPainting = [&]()
      {
        if (_paintInfo.Phase != PaintPhase::Foreground)
        {
          return false;
        }

        // The root inline box only has to paint a background for ::first-line style.
        bool isFirstLineBox = !box.LineIndex();
        if (box.IsRootInlineBox() && (!isFirstLineBox || &box.Style() == &box.LayoutBox().Style()))
        {
          return true;
        }

        return false;
      }();

      if (canSkipInlineBoxPainting)
      {
        return;
      }

      auto inlineBoxPaintInfo = PaintInfo {_paintInfo};
      inlineBoxPaintInfo.Phase =
        _paintInfo.Phase == PaintPhase::ChildOutlines ? PaintPhase::Outline : _paintInfo.Phase;
      // FIXME: The outline objects the inline boxes gather come with inline outlines.

      InlineBoxPainter {_inlineContent, box, inlineBoxPaintInfo, _paintOffset}.Paint();
      return;
    }

    if (box.IsText())
    {
      auto hasVisibleDamage = box.GetText().Length() && box.IsVisible() && hasDamage(box);
      if (!hasVisibleDamage)
      {
        return;
      }

      TextBoxPainter {_inlineContent, box, box.Style(), _paintInfo, _paintOffset}.Paint();
      return;
    }

    if (auto *renderer = DynamicDowncast<RenderBox>(box.LayoutBox().RendererForIntegration()); renderer)
    {
      if (_paintInfo.ShouldPaintWithinRoot(*renderer))
      {
        // FIXME: Painting should not require a non-const renderer.
        auto &paintRenderer = const_cast<RenderBox &>(*renderer);
        auto flippedOffset = FlippedContentOffsetIfNeeded(*renderer);
        if (box.IsBlockLevelBox())
        {
          // Blocks-in-inline.
          auto paintInfoForChild = _root.PaintInfoForBlockChildren(_paintInfo);
          paintRenderer.Paint(paintInfoForChild, flippedOffset);
        }
        else
        {
          paintRenderer.PaintAsInlineBlock(_paintInfo, flippedOffset);
        }
      }
    }
  }

  void InlineContentPainter::Paint() noexcept
  {
    auto layerPaintScope = LayerPaintScope {_inlineBoxWithLayer};

    // FIXME: Line-ending ellipses (paintLineEndingEllipsisIfApplicable, painted after each line's last box)
    // come with ellipses.

    for (auto &box : _inlineContent.BoxesForRect(_damageRect))
    {
      if (!box.LayoutBox().RendererForIntegration())
      {
        // No renderer means damaged content, and we should have bailed out earlier at LineLayout::paint.
        krys_debug_assert(false);
        return;
      }

      auto shouldPaintBoxForPhase = [&]
      {
        switch (_paintInfo.Phase)
        {
          case PaintPhase::ChildOutlines:         return box.IsNonRootInlineBox() || box.IsBlockLevelBox();
          case PaintPhase::SelfOutline:           return box.IsRootInlineBox();
          case PaintPhase::Outline:               return box.IsInlineBox() || box.IsBlockLevelBox();
          case PaintPhase::Mask:                  return box.IsInlineBox();
          case PaintPhase::Float:
          case PaintPhase::ChildBlockBackground:
          case PaintPhase::ChildBlockBackgrounds: return box.IsBlockLevelBox();
          default:                                return true;
        }
      };

      bool includedInPaintScope = layerPaintScope.TestIsIncludesAndUpdate(box);

      if (includedInPaintScope && shouldPaintBoxForPhase())
      {
        PaintDisplayBox(box);
      }
    }

    // FIXME: The outlines of the inline boxes gathered above (OutlinePainter::paintOutline for each
    // RenderInline) come with inline outlines.
  }

  LayoutPoint
    InlineContentPainter::FlippedContentOffsetIfNeeded(const RenderBox &childRenderer) const noexcept
  {
    if (Root().GetWritingMode().IsBlockFlipped())
    {
      return Root().FlipForWritingModeForChild(childRenderer, _paintOffset);
    }
    return _paintOffset;
  }

  const RenderBlock &InlineContentPainter::Root() const noexcept
  {
    return _root;
  }

  LayerPaintScope::LayerPaintScope(const RenderInline *inlineBoxWithLayer) noexcept
      : _inlineBoxWithLayer(inlineBoxWithLayer ? inlineBoxWithLayer->LayoutBox() : nullptr)
  {
  }

  bool LayerPaintScope::TestIsIncludesAndUpdate(const inline_display::Box &box) noexcept
  {
    auto isInside = [](auto &displayBox, auto &inlineBox)
    {
      krys_debug_assert(inlineBox.IsInlineBox());

      if (displayBox.IsRootInlineBox())
      {
        return false;
      }

      for (auto *box = &displayBox.LayoutBox().Parent(); box->IsInlineBox(); box = &box->Parent())
      {
        if (box == &inlineBox)
        {
          return true;
        }
      }
      return false;
    };

    if (_inlineBoxWithLayer == &box.LayoutBox())
    {
      return true;
    }
    if (_inlineBoxWithLayer && !isInside(box, *_inlineBoxWithLayer))
    {
      return false;
    }
    if (_currentExcludedInlineBox && isInside(box, *_currentExcludedInlineBox))
    {
      return false;
    }

    if (box.IsRootInlineBox())
    {
      return true;
    }

    _currentExcludedInlineBox = nullptr;

    if (box.IsText() || box.IsLineBreak())
    {
      return true;
    }

    auto *renderer = DynamicDowncast<RenderLayerModelObject>(box.LayoutBox().RendererForIntegration());
    bool hasSelfPaintingLayer = renderer && renderer->HasSelfPaintingLayer();

    if (hasSelfPaintingLayer && box.IsNonRootInlineBox())
    {
      _currentExcludedInlineBox = &Downcast<layout::ElementBox>(box.LayoutBox());
    }

    return !hasSelfPaintingLayer;
  }
}
