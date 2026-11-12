/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/TextBoxTrimmer.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD TextBoxTrim GetTextBoxTrim(const RenderBlockFlow &textBoxTrimRoot) noexcept
    {
      if (auto *multiColumnFlow = DynamicDowncast<RenderMultiColumnFlow>(textBoxTrimRoot))
      {
        return multiColumnFlow->MultiColumnBlockFlow()->Style().GetTextBoxTrim();
      }
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
      shouldTrimFirstFormattedLineStart, CreateWeakPtr(lastFormattedLineRoot)});
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
