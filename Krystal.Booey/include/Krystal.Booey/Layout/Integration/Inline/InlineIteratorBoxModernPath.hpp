#pragma once

#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBlockFlow;
  class RenderObject;
  class RenderStyle;
  class RenderText;
}

namespace krys::boo::inline_iterator
{
  enum class TextRunMode : bool
  {
    Painting,
    Editing
  };

  /// @brief A position in a block container's display boxes, with the traversals the box iterators are built
  /// on.
  class BoxModernPath
  {
  private:
    WeakPtr<const layout_integration::InlineContent> _inlineContent;
    size_t _boxIndex {0};

  public:
    /// @brief An iterator at the end, with no content.
    BoxModernPath() noexcept = default;
    explicit BoxModernPath(const layout_integration::InlineContent &inlineContent) noexcept
        : _inlineContent(WeakPtr<const layout_integration::InlineContent>::WithRef(inlineContent))
    {
      SetAtEnd();
    }
    BoxModernPath(const layout_integration::InlineContent &inlineContent, size_t startIndex) noexcept
        : _inlineContent(WeakPtr<const layout_integration::InlineContent>::WithRef(inlineContent)),
          _boxIndex(startIndex)
    {
    }

    KRYS_NODISCARD bool IsText() const noexcept
    {
      return GetBox().IsTextOrSoftLineBreak();
    }
    KRYS_NODISCARD bool IsInlineBox() const noexcept
    {
      return GetBox().IsInlineBox();
    }
    KRYS_NODISCARD bool IsRootInlineBox() const noexcept
    {
      return GetBox().IsRootInlineBox();
    }
    // Blocks-in-inline.
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept
    {
      return GetBox().IsBlockLevelBox();
    }
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
    {
      return GetBox().IsAtomicInlineBox();
    }

    KRYS_NODISCARD FloatRect VisualRectIgnoringBlockDirection() const noexcept
    {
      return GetBox().VisualRectIgnoringBlockDirection();
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return GetBox().IsHorizontal();
    }
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept
    {
      return GetBox().GetWritingMode();
    }
    KRYS_NODISCARD bool IsLineBreak() const noexcept
    {
      return GetBox().IsLineBreak();
    }

    KRYS_NODISCARD uint32 MinimumCaretOffset() const noexcept
    {
      return IsText() ? Start() : 0;
    }
    KRYS_NODISCARD uint32 MaximumCaretOffset() const noexcept
    {
      return IsText() ? End() : 1;
    }

    KRYS_NODISCARD UBiDiLevel BidiLevel() const noexcept
    {
      return GetBox().BidiLevel();
    }

    KRYS_NODISCARD bool HasHyphen() const noexcept
    {
      return GetBox().GetText().HasHyphen();
    }
    KRYS_NODISCARD utf8_stringview OriginalText() const noexcept
    {
      return GetBox().GetText().OriginalContent();
    }
    KRYS_NODISCARD uint32 Start() const noexcept
    {
      return static_cast<uint32>(GetBox().GetText().Start());
    }
    KRYS_NODISCARD uint32 End() const noexcept
    {
      return static_cast<uint32>(GetBox().GetText().End());
    }
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(GetBox().GetText().Length());
    }
    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return GetBox().LineIndex();
    }

    KRYS_NODISCARD TextRun GetTextRun(TextRunMode mode = TextRunMode::Painting) const noexcept;
    /// @brief The part of the renderer's text the text box shows, for clamping offsets to it.
    KRYS_NODISCARD TextBoxSelectableRange SelectableRange() const noexcept;

    KRYS_NODISCARD const RenderObject &Renderer() const noexcept
    {
      return *GetBox().LayoutBox().RendererForIntegration();
    }
    KRYS_NODISCARD bool HasRenderer() const noexcept
    {
      return GetBox().LayoutBox().RendererForIntegration();
    }

    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return _inlineContent->FormattingContextRoot();
    }

    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return GetBox().Style();
    }

    void TraverseNextTextBox() noexcept
    {
      krys_debug_assert(!AtEnd());
      krys_debug_assert(GetBox().IsTextOrSoftLineBreak());

      if (GetBox().IsLastForLayoutBox())
      {
        SetAtEnd();
        return;
      }

      TraverseNextWithSameLayoutBox();
      krys_debug_assert(GetBox().IsTextOrSoftLineBreak());
    }

    void TraverseNextLeafOnLine() noexcept
    {
      krys_debug_assert(!AtEnd());
      auto oldLineIndex = GetBox().LineIndex();
      TraverseNextLeaf();
      if (!AtEnd() && oldLineIndex != GetBox().LineIndex())
      {
        SetAtEnd();
      }
    }

    void TraversePreviousLeafOnLine() noexcept
    {
      krys_debug_assert(!AtEnd());
      auto oldLineIndex = GetBox().LineIndex();
      TraversePreviousLeaf();
      if (!AtEnd() && oldLineIndex != GetBox().LineIndex())
      {
        SetAtEnd();
      }
    }

    void TraverseNextInlineBox() noexcept
    {
      krys_debug_assert(!AtEnd());
      krys_debug_assert(GetBox().IsInlineBox());

      if (GetBox().IsLastForLayoutBox())
      {
        SetAtEnd();
        return;
      }

      TraverseNextWithSameLayoutBox();
      krys_debug_assert(GetBox().IsInlineBox());
    }

    void TraversePreviousInlineBox() noexcept
    {
      krys_debug_assert(!AtEnd());
      krys_debug_assert(GetBox().IsInlineBox());

      if (GetBox().IsFirstForLayoutBox())
      {
        SetAtEnd();
        return;
      }

      TraversePreviousWithSameLayoutBox();
      krys_debug_assert(GetBox().IsInlineBox());
    }

    void TraverseNextBoxOnLine() noexcept
    {
      auto lineIndex = GetBox().LineIndex();
      TraverseNextBox();
      if (!AtEnd() && lineIndex != GetBox().LineIndex())
      {
        SetAtEnd();
      }
    }

    void TraverseNextBoxOnLineSkippingChildren() noexcept
    {
      auto lineIndex = GetBox().LineIndex();
      bool wasInlineBox = GetBox().IsInlineBox();
      auto &startBox = GetBox().LayoutBox();

      TraverseNextBox();

      if (wasInlineBox)
      {
        while (!AtEnd() && IsWithinInlineBox(startBox))
        {
          TraverseNextBox();
        }
      }

      if (!AtEnd() && lineIndex != GetBox().LineIndex())
      {
        SetAtEnd();
      }
    }

    KRYS_NODISCARD BoxModernPath FirstLeafBoxForInlineBox() const noexcept
    {
      krys_debug_assert(GetBox().IsInlineBox());

      auto &inlineBox = GetBox().LayoutBox();

      // The next box is the first descendant of this box;
      auto first = *this;
      first.TraverseNextLeafOnLine();

      if (!first.AtEnd() && !first.IsWithinInlineBox(inlineBox))
      {
        first.SetAtEnd();
      }

      return first;
    }

    KRYS_NODISCARD BoxModernPath LastLeafBoxForInlineBox() const noexcept
    {
      krys_debug_assert(GetBox().IsInlineBox());

      auto &inlineBox = GetBox().LayoutBox();

      // FIXME: Get the last box index directly from the display box.
      auto last = FirstLeafBoxForInlineBox();
      for (auto box = last; !box.AtEnd() && box.IsWithinInlineBox(inlineBox); box.TraverseNextLeafOnLine())
      {
        last = box;
      }

      return last;
    }

    KRYS_NODISCARD BoxModernPath ParentInlineBox() const noexcept
    {
      krys_debug_assert(!AtEnd());

      auto candidate = *this;

      if (IsRootInlineBox())
      {
        candidate.SetAtEnd();
        return candidate;
      }

      auto &parentLayoutBox = GetBox().LayoutBox().Parent();
      do
      {
        candidate.TraversePreviousBox();
      } while (!candidate.AtEnd() && &candidate.GetBox().LayoutBox() != &parentLayoutBox);

      krys_debug_assert(candidate.AtEnd() || candidate.GetBox().IsInlineBox());

      return candidate;
    }

    KRYS_NODISCARD TextDirection Direction() const noexcept
    {
      return BidiLevel() % 2 ? TextDirection::RTL : TextDirection::LTR;
    }
    KRYS_NODISCARD bool IsFirstFormattedLine() const noexcept
    {
      return GetBox().IsFirstFormattedLine();
    }

    KRYS_NODISCARD friend bool operator==(const BoxModernPath &a, const BoxModernPath &b) noexcept
    {
      return a._inlineContent.get() == b._inlineContent.get() && a._boxIndex == b._boxIndex;
    }

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_inlineContent || _boxIndex == Boxes().size();
    }
    KRYS_NODISCARD const inline_display::Box &GetBox() const noexcept
    {
      return Boxes()[_boxIndex];
    }
    KRYS_NODISCARD const layout_integration::InlineContent &GetInlineContent() const noexcept
    {
      return *_inlineContent;
    }
    KRYS_NODISCARD size_t BoxIndex() const noexcept
    {
      return _boxIndex;
    }

  private:
    KRYS_NODISCARD bool IsWithinInlineBox(const layout::Box &inlineBox) const noexcept
    {
      for (auto *layoutBox = &GetBox().LayoutBox().Parent();; layoutBox = &layoutBox->Parent())
      {
        if (layoutBox == &inlineBox)
        {
          return true;
        }
        if (!layoutBox->IsInlineBox())
        {
          return false;
        }
      }
    }

    void TraverseNextBox() noexcept
    {
      krys_debug_assert(!AtEnd());
      ++_boxIndex;
    }

    void TraversePreviousBox() noexcept
    {
      krys_debug_assert(!AtEnd());
      _boxIndex = _boxIndex ? _boxIndex - 1 : Boxes().size();
    }

    void TraverseNextLeaf() noexcept
    {
      do
      {
        TraverseNextBox();
      } while (!AtEnd() && GetBox().IsInlineBox());
    }

    void TraversePreviousLeaf() noexcept
    {
      do
      {
        TraversePreviousBox();
      } while (!AtEnd() && GetBox().IsInlineBox());
    }

    void TraverseNextWithSameLayoutBox() noexcept
    {
      auto &layoutBox = GetBox().LayoutBox();
      do
      {
        TraverseNextBox();
      } while (!AtEnd() && &GetBox().LayoutBox() != &layoutBox);
    }

    void TraversePreviousWithSameLayoutBox() noexcept
    {
      auto &layoutBox = GetBox().LayoutBox();
      do
      {
        TraversePreviousBox();
      } while (!AtEnd() && &GetBox().LayoutBox() != &layoutBox);
    }

    void SetAtEnd() noexcept
    {
      _boxIndex = Boxes().size();
    }

    KRYS_NODISCARD const inline_display::Boxes &Boxes() const noexcept
    {
      return _inlineContent->DisplayContent().BoxList;
    }
    KRYS_NODISCARD const inline_display::Line &GetLine() const noexcept
    {
      return _inlineContent->LineForBox(GetBox());
    }
    KRYS_NODISCARD const RenderText &GetRenderText() const noexcept;
  };
}
