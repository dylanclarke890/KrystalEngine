#include "Krystal.Booey/Rendering/TextBoxTrimmer.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD TextBoxTrim GetTextBoxTrim(const RenderBlockFlow &textBoxTrimRoot) noexcept
    {
      // FIXME: A multi-column flow takes its multi-column block's text-box-trim, once there are multi-column
      // flows.
      return textBoxTrimRoot.Style().GetTextBoxTrim();
    }

    KRYS_NODISCARD LocalFrameViewLayoutContext &LayoutContext(const RenderBox &renderer) noexcept
    {
      return renderer.View().FrameView().LayoutContext();
    }

    void RemoveTextBoxTrimStart(LocalFrameViewLayoutContext &layoutContext) noexcept
    {
      auto textBoxTrim = layoutContext.GetTextBoxTrim();
      if (!textBoxTrim || !textBoxTrim->TrimFirstFormattedLine)
      {
        krys_debug_assert(false);
        return;
      }
      layoutContext.SetTextBoxTrim(
        LocalFrameViewLayoutContext::TextBoxTrim {false, textBoxTrim->LastFormattedLineRoot});
    }

    KRYS_NODISCARD bool
      ShouldIgnoreAsFirstLastFormattedLineContainer(const RenderBlockFlow &container) noexcept
    {
      if (container.Style().Display() == DisplayType::RubyAnnotation
          || container.CreatesNewFormattingContext())
      {
        return true;
      }
      // Empty continuation pre/post blocks should be ignored as they are implementation detail.
      if (container.IsAnonymousBlock())
      {
        if (auto firstLineBox = inline_iterator::FirstLineBoxFor(container))
        {
          return !firstLineBox->LineLeftmostLeafBox();
        }
        return true;
      }
      return false;
    }

    KRYS_NODISCARD RenderBlockFlow *
      FirstFormattedLineRoot(const RenderBlockFlow &enclosingBlockContainer) noexcept
    {
      for (auto *child = enclosingBlockContainer.FirstChild(); child; child = child->NextSibling())
      {
        auto *blockContainer = DynamicDowncast<RenderBlockFlow>(*child);
        if (!blockContainer || blockContainer->CreatesNewFormattingContext()
            || blockContainer->IsFirstLetter())
        {
          continue;
        }
        if (blockContainer->HasContentfulInlineOrBlockLine())
        {
          return blockContainer;
        }
        if (auto *descendantRoot = FirstFormattedLineRoot(*blockContainer))
        {
          return descendantRoot;
        }
        if (!ShouldIgnoreAsFirstLastFormattedLineContainer(*blockContainer))
        {
          return nullptr;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD RenderBlockFlow *
      LastFormattedLineRoot(const RenderBlockFlow &enclosingBlockContainer) noexcept
    {
      if (enclosingBlockContainer.HasContentfulInlineOrBlockLine())
      {
        // With blocks-in-inline, the last formatted line may be a block sitting on the last line.
        auto firstBoxOnLastFormattedLineWithContent = [&]() -> inline_iterator::LeafBoxIterator
        {
          for (auto lineBox = inline_iterator::LastLineBoxFor(enclosingBlockContainer); lineBox; --lineBox)
          {
            if (auto box = lineBox->LogicalLeftmostLeafBox())
            {
              return box;
            }
          }
          return {};
        };
        if (auto box = firstBoxOnLastFormattedLineWithContent(); box && box->IsBlockLevelBox())
        {
          krys_debug_assert(Is<RenderBlockFlow>(box->Renderer()));
          if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(const_cast<RenderObject &>(box->Renderer())))
          {
            if (auto *candidate = LastFormattedLineRoot(*blockFlow))
            {
              return candidate;
            }
            // This block itself might be the enclosing block on the last formatted line.
            if (blockFlow->HasContentfulInlineLine())
            {
              return blockFlow;
            }
          }
        }
      }

      for (auto *child = enclosingBlockContainer.LastChild(); child; child = child->PreviousSibling())
      {
        auto *blockContainer = DynamicDowncast<RenderBlockFlow>(*child);
        if (!blockContainer || blockContainer->CreatesNewFormattingContext()
            || blockContainer->IsFirstLetter())
        {
          continue;
        }
        if (blockContainer->HasContentfulInlineOrBlockLine())
        {
          if (auto *candidate = LastFormattedLineRoot(*blockContainer))
          {
            return candidate;
          }
          return blockContainer;
        }
        if (auto *descendantRoot = LastFormattedLineRoot(*blockContainer))
        {
          return descendantRoot;
        }
        if (!ShouldIgnoreAsFirstLastFormattedLineContainer(*blockContainer))
        {
          return nullptr;
        }
      }
      return nullptr;
    }
  }

  TextBoxTrimmer::TextBoxTrimmer(const RenderBlockFlow &blockContainer) noexcept
      : _blockContainer(&blockContainer)
  {
    AdjustTextBoxTrimStatusBeforeLayout({});
  }

  TextBoxTrimmer::TextBoxTrimmer(const RenderBlockFlow &blockContainer,
                                 const RenderBlockFlow &lastFormattedLineRoot) noexcept
      : _blockContainer(&blockContainer)
  {
    AdjustTextBoxTrimStatusBeforeLayout(&lastFormattedLineRoot);
  }

  TextBoxTrimmer::~TextBoxTrimmer() noexcept
  {
    AdjustTextBoxTrimStatusAfterLayout();
  }

  RenderBlockFlow *
    TextBoxTrimmer::LastInlineFormattingContextRootForTrimEnd(const RenderBlockFlow &blockContainer) noexcept
  {
    auto textBoxTrimValue = GetTextBoxTrim(blockContainer);
    auto hasTextBoxTrimEnd =
      textBoxTrimValue == TextBoxTrim::TrimEnd || textBoxTrimValue == TextBoxTrim::TrimBoth;
    if (!hasTextBoxTrimEnd)
    {
      return nullptr;
    }
    auto *candidateForLastBlockContainer = LastFormattedLineRoot(blockContainer);
    if (!candidateForLastBlockContainer || candidateForLastBlockContainer == &blockContainer)
    {
      return nullptr;
    }
    // If the nested (last) block container has border/padding after, trimming should not happen.
    return !candidateForLastBlockContainer->BorderAndPaddingAfter() ? candidateForLastBlockContainer
                                                                    : nullptr;
  }

  void
    TextBoxTrimmer::AdjustTextBoxTrimStatusBeforeLayout(const RenderBlockFlow *lastFormattedLineRoot) noexcept
  {
    auto textBoxTrimValue = GetTextBoxTrim(*_blockContainer);
    if (textBoxTrimValue == TextBoxTrim::None)
    {
      return HandleTextBoxTrimNoneBeforeLayout();
    }

    auto &layoutContext = LayoutContext(*_blockContainer);
    // This block container starts setting up trimming for its subtree.
    // 1. Let's save the current trimming status, merge (and restore after layout).
    // 2. Figure out which side(s) of the content is going to get trimmed.
    _previousTextBoxTrimStatus = layoutContext.GetTextBoxTrim();
    _shouldRestoreTextBoxTrimStatus = true;

    auto shouldTrimFirstFormattedLineStart =
      (textBoxTrimValue == TextBoxTrim::TrimStart || textBoxTrimValue == TextBoxTrim::TrimBoth)
      || (_previousTextBoxTrimStatus && _previousTextBoxTrimStatus->TrimFirstFormattedLine);
    auto shouldTrimmingLastFormattedLineEnd =
      textBoxTrimValue == TextBoxTrim::TrimEnd || textBoxTrimValue == TextBoxTrim::TrimBoth;

    if (shouldTrimmingLastFormattedLineEnd)
    {
      if (!lastFormattedLineRoot && _blockContainer->ChildrenInline())
      {
        // Last line end trimming is explicitly set on this inline formatting context. Let's assume last line
        // is part of this block container.
        lastFormattedLineRoot = _blockContainer;
      }
      else if (lastFormattedLineRoot)
      {
        // This is the dedicated "last line" layout on the last inline formatting context, where we should not
        // trim the first line unless this IFC includes it too.
        if (shouldTrimFirstFormattedLineStart)
        {
          shouldTrimFirstFormattedLineStart =
            FirstFormattedLineRoot(*_blockContainer) == lastFormattedLineRoot;
        }
      }
    }
    if (!lastFormattedLineRoot && _previousTextBoxTrimStatus)
    {
      lastFormattedLineRoot = _previousTextBoxTrimStatus->LastFormattedLineRoot.get();
    }

    layoutContext.SetTextBoxTrim(LocalFrameViewLayoutContext::TextBoxTrim {
      shouldTrimFirstFormattedLineStart, lastFormattedLineRoot
                                           ? WeakPtr<const RenderBlockFlow>::WithRef(*lastFormattedLineRoot)
                                           : WeakPtr<const RenderBlockFlow> {}});
  }

  void TextBoxTrimmer::AdjustTextBoxTrimStatusAfterLayout() noexcept
  {
    auto &layoutContext = LayoutContext(*_blockContainer);
    if (_shouldRestoreTextBoxTrimStatus)
    {
      return layoutContext.SetTextBoxTrim(_previousTextBoxTrimStatus);
    }

    if (auto textBoxTrim = layoutContext.GetTextBoxTrim(); textBoxTrim && textBoxTrim->TrimFirstFormattedLine)
    {
      // Only the first formatted line is trimmed.
      if (!ShouldIgnoreAsFirstLastFormattedLineContainer(*_blockContainer))
      {
        RemoveTextBoxTrimStart(layoutContext);
      }
    }
  }

  void TextBoxTrimmer::HandleTextBoxTrimNoneBeforeLayout() noexcept
  {
    auto &layoutContext = LayoutContext(*_blockContainer);
    // This is when the block container does not have text-box-trim set.
    // 1. trimming from ancestors does not get propagated into formatting contexts e.g inside inline-block.
    // 2. border and padding (before) prevent trim start.
    if (_blockContainer->CreatesNewFormattingContext())
    {
      _previousTextBoxTrimStatus = layoutContext.GetTextBoxTrim();
      _shouldRestoreTextBoxTrimStatus = true;
      // Run layout on this subtree with no text-box-trim.
      layoutContext.SetTextBoxTrim({});
      return;
    }

    auto hasTextBoxTrimStart =
      layoutContext.GetTextBoxTrim() && layoutContext.GetTextBoxTrim()->TrimFirstFormattedLine;
    if (hasTextBoxTrimStart && _blockContainer->BorderAndPaddingBefore())
    {
      // We've got this far with no start trimming and now border/padding prevent trimming.
      RemoveTextBoxTrimStart(layoutContext);
    }
  }

  TextBoxTrimStartDisabler::TextBoxTrimStartDisabler(const RenderBox &renderBox) noexcept
      : _renderBox(&renderBox)
  {
    auto &layoutContext = LayoutContext(*_renderBox);
    _previousTextBoxTrimStatus = layoutContext.GetTextBoxTrim();
    if (_previousTextBoxTrimStatus)
    {
      layoutContext.SetTextBoxTrim(
        LocalFrameViewLayoutContext::TextBoxTrim {false, _previousTextBoxTrimStatus->LastFormattedLineRoot});
    }
  }

  TextBoxTrimStartDisabler::~TextBoxTrimStartDisabler() noexcept
  {
    LayoutContext(*_renderBox).SetTextBoxTrim(_previousTextBoxTrimStatus);
  }
}
