#pragma once

#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderOverflow.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  class Font;
  class LegacyInlineTextBox;

  using GlyphOverflowAndFallbackFontsMap =
    HashMap<const LegacyInlineTextBox *, Pair<List<WeakPtr<const Font>>, GlyphOverflow>>;

  /// @brief A box on a line that holds the boxes of its renderer's children on that line.
  class LegacyInlineFlowBox : public LegacyInlineBox
  {
    KRYS_NON_COPY_MOVABLE(LegacyInlineFlowBox);

  private:
    bool _hasTextChildren : 1 {false};
    bool _hasTextDescendants : 1 {false};

  protected:
    // The following members are only used by RootInlineBox but moved here to keep the bits packed.

    // Whether or not this line uses alphabetic or ideographic baselines by default.
    FontBaseline _baselineType {FontBaseline::Alphabetic};

    UCharDirection _lineBreakBidiStatusEor {U_OTHER_NEUTRAL};
    UCharDirection _lineBreakBidiStatusLastStrong {U_OTHER_NEUTRAL};
    UCharDirection _lineBreakBidiStatusLast {U_OTHER_NEUTRAL};

    bool _hasSelfPaintInlineBox : 1 {false};

    // End of RootInlineBox-specific members.

    UniquePtr<RenderOverflow> _overflow;

    LegacyInlineBox *_firstChild {nullptr};
    LegacyInlineBox *_lastChild {nullptr};

    LegacyInlineFlowBox *_prevLineBox {nullptr}; // The previous box that also uses our RenderObject
    LegacyInlineFlowBox *_nextLineBox {nullptr}; // The next box that also uses our RenderObject

  public:
    explicit LegacyInlineFlowBox(RenderBoxModelObject &renderer) noexcept : LegacyInlineBox(renderer)
    {
    }

    KRYS_NODISCARD RenderBoxModelObject &Renderer() const noexcept
    {
      return Downcast<RenderBoxModelObject>(LegacyInlineBox::Renderer());
    }
    KRYS_NODISCARD const RenderStyle &LineStyle() const noexcept
    {
      return IsFirstLine() ? Renderer().FirstLineStyle() : Renderer().Style();
    }

    KRYS_NODISCARD LegacyInlineFlowBox *PrevLineBox() const noexcept
    {
      return _prevLineBox;
    }
    KRYS_NODISCARD LegacyInlineFlowBox *NextLineBox() const noexcept
    {
      return _nextLineBox;
    }
    void SetNextLineBox(LegacyInlineFlowBox *n) noexcept
    {
      _nextLineBox = n;
    }
    void SetPreviousLineBox(LegacyInlineFlowBox *p) noexcept
    {
      _prevLineBox = p;
    }

    KRYS_NODISCARD LegacyInlineBox *FirstChild() const noexcept
    {
      return _firstChild;
    }
    KRYS_NODISCARD LegacyInlineBox *LastChild() const noexcept
    {
      return _lastChild;
    }

    KRYS_NODISCARD bool IsLeaf() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD LegacyInlineBox *FirstLeafDescendant() const noexcept;
    KRYS_NODISCARD LegacyInlineBox *LastLeafDescendant() const noexcept;

    void SetConstructed() noexcept final
    {
      LegacyInlineBox::SetConstructed();
      for (auto *child = FirstChild(); child; child = child->NextOnLine())
      {
        child->SetConstructed();
      }
    }

    void AddToLine(LegacyInlineBox *child) noexcept;
    void DeleteLine() noexcept final;
    void AdjustPosition(float dx, float dy) noexcept override;

    virtual void RemoveLineBoxFromRenderObject() noexcept;

    void ComputeOverflow(LayoutUnit lineTop, LayoutUnit lineBottom,
                         GlyphOverflowAndFallbackFontsMap &) noexcept;

    void RemoveChild(LegacyInlineBox *child) noexcept;

    KRYS_NODISCARD RenderObject::HighlightState SelectionState() const noexcept override;

    KRYS_NODISCARD bool HasTextChildren() const noexcept
    {
      return _hasTextChildren;
    }
    KRYS_NODISCARD bool HasTextDescendants() const noexcept
    {
      return _hasTextDescendants;
    }
    void SetHasTextChildren() noexcept
    {
      _hasTextChildren = true;
      SetHasTextDescendants();
    }
    void SetHasTextDescendants() noexcept
    {
      _hasTextDescendants = true;
    }

    KRYS_NODISCARD LayoutRect VisualOverflowRect(LayoutUnit lineTop, LayoutUnit lineBottom) const noexcept
    {
      return _overflow ? _overflow->VisualOverflowRect()
                       : EnclosingLayoutRect(FrameRectIncludingLineHeight(lineTop, lineBottom));
    }
    KRYS_NODISCARD LayoutUnit LogicalLeftVisualOverflow() const noexcept
    {
      return _overflow
               ? (IsHorizontal() ? _overflow->VisualOverflowRect().X() : _overflow->VisualOverflowRect().Y())
               : LayoutUnit(LogicalLeft());
    }
    KRYS_NODISCARD LayoutUnit LogicalRightVisualOverflow() const noexcept
    {
      return _overflow ? (IsHorizontal() ? _overflow->VisualOverflowRect().MaxX()
                                         : _overflow->VisualOverflowRect().MaxY())
                       : LayoutUnit(std::ceil(LogicalRight()));
    }
    KRYS_NODISCARD LayoutUnit LogicalTopVisualOverflow(LayoutUnit lineTop) const noexcept
    {
      if (_overflow)
      {
        return IsHorizontal() ? _overflow->VisualOverflowRect().Y() : _overflow->VisualOverflowRect().X();
      }
      return lineTop;
    }
    KRYS_NODISCARD LayoutUnit LogicalBottomVisualOverflow(LayoutUnit lineBottom) const noexcept
    {
      if (_overflow)
      {
        return IsHorizontal() ? _overflow->VisualOverflowRect().MaxY()
                              : _overflow->VisualOverflowRect().MaxX();
      }
      return lineBottom;
    }
    KRYS_NODISCARD LayoutRect LogicalVisualOverflowRect(LayoutUnit lineTop,
                                                        LayoutUnit lineBottom) const noexcept
    {
      LayoutRect result = VisualOverflowRect(lineTop, lineBottom);
      if (!Renderer().IsHorizontalWritingMode())
      {
        result = result.TransposedRect();
      }
      return result;
    }

    void SetOverflowFromLogicalRects(const LayoutRect &logicalVisualOverflow, LayoutUnit lineTop,
                                     LayoutUnit lineBottom) noexcept;
    void SetVisualOverflow(const LayoutRect &, LayoutUnit lineTop, LayoutUnit lineBottom) noexcept;

    KRYS_NODISCARD FloatRect FrameRectIncludingLineHeight(LayoutUnit lineTop,
                                                          LayoutUnit lineBottom) const noexcept
    {
      if (IsHorizontal())
      {
        return FloatRect(X(), lineTop.ToFloat(), Width(), (lineBottom - lineTop).ToFloat());
      }
      return FloatRect(lineTop.ToFloat(), Y(), (lineBottom - lineTop).ToFloat(), Height());
    }

    KRYS_NODISCARD FloatRect LogicalFrameRectIncludingLineHeight(LayoutUnit lineTop,
                                                                 LayoutUnit lineBottom) const noexcept
    {
      return FloatRect(LogicalLeft(), lineTop.ToFloat(), LogicalWidth(), (lineBottom - lineTop).ToFloat());
    }

    KRYS_NODISCARD bool HasSelfPaintInlineBox() const noexcept
    {
      return _hasSelfPaintInlineBox;
    }

  private:
    KRYS_NODISCARD bool IsInlineFlowBox() const noexcept final
    {
      return true;
    }
    void BoxModelObject() const = delete;

    void AddTextBoxVisualOverflow(LegacyInlineTextBox &, GlyphOverflowAndFallbackFontsMap &,
                                  LayoutRect &logicalVisualOverflow) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyInlineFlowBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsInlineFlowBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
