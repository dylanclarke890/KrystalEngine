#pragma once

#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxModernPath.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::inline_iterator
{
  /// @brief A box of the legacy line layout (SVG text's).
  class BoxLegacyPath
  {
  private:
    const LegacyInlineBox *_inlineBox {nullptr};

  public:
    explicit BoxLegacyPath(const LegacyInlineBox *inlineBox) noexcept : _inlineBox(inlineBox)
    {
    }

    KRYS_NODISCARD bool IsText() const noexcept
    {
      return _inlineBox->IsInlineTextBox();
    }
    KRYS_NODISCARD bool IsInlineBox() const noexcept
    {
      return _inlineBox->IsInlineFlowBox();
    }
    KRYS_NODISCARD bool IsRootInlineBox() const noexcept
    {
      return _inlineBox->IsRootInlineBox();
    }
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
    {
      return Renderer().IsBlockLevelReplacedOrAtomicInline();
    }

    KRYS_NODISCARD FloatRect VisualRectIgnoringBlockDirection() const noexcept
    {
      return _inlineBox->FrameRect();
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _inlineBox->IsHorizontal();
    }
    KRYS_NODISCARD bool IsLineBreak() const noexcept
    {
      return _inlineBox->IsLineBreak();
    }

    KRYS_NODISCARD uint32 MinimumCaretOffset() const noexcept
    {
      return static_cast<uint32>(_inlineBox->CaretMinOffset());
    }
    KRYS_NODISCARD uint32 MaximumCaretOffset() const noexcept
    {
      return static_cast<uint32>(_inlineBox->CaretMaxOffset());
    }

    KRYS_NODISCARD UBiDiLevel BidiLevel() const noexcept
    {
      return _inlineBox->BidiLevel();
    }

    KRYS_NODISCARD bool HasHyphen() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD utf8_stringview OriginalText() const noexcept
    {
      return utf8_stringview(InlineTextBox()->Renderer().Text())
        .substr(InlineTextBox()->Start(), InlineTextBox()->Len());
    }
    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      size_t precedingLines = 0;
      for (auto *rootBox = RootInlineBox().PrevRootBox(); rootBox; rootBox = rootBox->PrevRootBox())
      {
        ++precedingLines;
      }
      return precedingLines;
    }
    KRYS_NODISCARD uint32 Start() const noexcept
    {
      return InlineTextBox()->Start();
    }
    KRYS_NODISCARD uint32 End() const noexcept
    {
      return InlineTextBox()->End();
    }
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return InlineTextBox()->Len();
    }

    KRYS_NODISCARD TextBoxSelectableRange SelectableRange() const noexcept
    {
      return InlineTextBox()->SelectableRange();
    }

    KRYS_NODISCARD TextRun GetTextRun(TextRunMode = TextRunMode::Painting) const noexcept
    {
      if (IsText())
      {
        return InlineTextBox()->CreateTextRun();
      }
      krys_debug_assert(false);
      return TextRun {utf8_stringview()};
    }

    KRYS_NODISCARD const RenderObject &Renderer() const noexcept
    {
      return _inlineBox->Renderer();
    }

    KRYS_NODISCARD bool HasRenderer() const noexcept
    {
      return true;
    }

    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return _inlineBox->Root().BlockFlow();
    }

    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return _inlineBox->LineStyle();
    }

    void TraverseNextTextBox() noexcept
    {
      _inlineBox = InlineTextBox()->NextTextBox();
    }

    void TraverseNextLeafOnLine() noexcept
    {
      _inlineBox = _inlineBox->NextLeafOnLine();
    }

    void TraversePreviousLeafOnLine() noexcept
    {
      _inlineBox = _inlineBox->PreviousLeafOnLine();
    }

    void TraverseNextInlineBox() noexcept
    {
      _inlineBox = InlineFlowBox()->NextLineBox();
    }

    void TraversePreviousInlineBox() noexcept
    {
      _inlineBox = InlineFlowBox()->PrevLineBox();
    }

    KRYS_NODISCARD BoxLegacyPath FirstLeafBoxForInlineBox() const noexcept
    {
      return BoxLegacyPath {InlineFlowBox()->FirstLeafDescendant()};
    }

    KRYS_NODISCARD BoxLegacyPath LastLeafBoxForInlineBox() const noexcept
    {
      return BoxLegacyPath {InlineFlowBox()->LastLeafDescendant()};
    }

    KRYS_NODISCARD BoxLegacyPath ParentInlineBox() const noexcept
    {
      return BoxLegacyPath {_inlineBox->Parent()};
    }

    KRYS_NODISCARD TextDirection Direction() const noexcept
    {
      return BidiLevel() % 2 ? TextDirection::RTL : TextDirection::LTR;
    }
    KRYS_NODISCARD bool IsFirstFormattedLine() const noexcept
    {
      return !RootInlineBox().PrevRootBox();
    }

    KRYS_NODISCARD friend bool operator==(const BoxLegacyPath &, const BoxLegacyPath &) noexcept = default;

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_inlineBox;
    }

    KRYS_NODISCARD LegacyInlineBox *GetLegacyInlineBox() const noexcept
    {
      return const_cast<LegacyInlineBox *>(_inlineBox);
    }
    KRYS_NODISCARD const LegacyRootInlineBox &RootInlineBox() const noexcept
    {
      return _inlineBox->Root();
    }

    void TraverseNextBoxOnLine() noexcept
    {
      if (auto *flowBox = DynamicDowncast<LegacyInlineFlowBox>(_inlineBox); flowBox && flowBox->FirstChild())
      {
        _inlineBox = flowBox->FirstChild();
        return;
      }

      TraverseNextBoxOnLineSkippingChildren();
    }

    void TraverseNextBoxOnLineSkippingChildren() noexcept
    {
      if (_inlineBox->NextOnLine())
      {
        _inlineBox = _inlineBox->NextOnLine();
        return;
      }

      auto *parent = _inlineBox->Parent();
      while (parent && !parent->NextOnLine())
      {
        parent = parent->Parent();
      }

      _inlineBox = parent ? parent->NextOnLine() : nullptr;
    }

    KRYS_NODISCARD const List<SVGTextFragment> &SvgTextFragments() const noexcept
    {
      return SvgInlineTextBox()->TextFragments();
    }

  private:
    KRYS_NODISCARD const LegacyInlineTextBox *InlineTextBox() const noexcept
    {
      return Downcast<LegacyInlineTextBox>(_inlineBox);
    }
    KRYS_NODISCARD const LegacyInlineFlowBox *InlineFlowBox() const noexcept
    {
      return Downcast<LegacyInlineFlowBox>(_inlineBox);
    }
    KRYS_NODISCARD const SVGInlineTextBox *SvgInlineTextBox() const noexcept
    {
      return Downcast<SVGInlineTextBox>(_inlineBox);
    }
  };
}
