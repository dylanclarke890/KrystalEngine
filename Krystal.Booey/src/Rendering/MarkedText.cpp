#include "Krystal.Booey/Rendering/MarkedText.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/HighlightRegistry.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/RenderHighlight.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoElementRequest.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo
{
  List<MarkedText> MarkedText::Subdivide(const List<MarkedText> &markedTexts, OverlapStrategy overlapStrategy)
  {
    if (markedTexts.empty())
    {
      return {};
    }

    struct Offset
    {
      enum Kind
      {
        Begin,
        End
      };
      Kind OffsetKind;
      uint32 Value; // Copy of markedText.startOffset/endOffset to avoid the need to branch based on kind.
      const MarkedText *Text;
    };

    // 1. Build table of all offsets.
    List<Offset> offsets;
    krys_debug_assert(markedTexts.size() < std::numeric_limits<uint32>::max() / 2);
    uint32 numberOfMarkedTexts = static_cast<uint32>(markedTexts.size());
    uint32 numberOfOffsets = 2 * numberOfMarkedTexts;
    offsets.reserve(numberOfOffsets);
    for (auto &markedText : markedTexts)
    {
      offsets.push_back({Offset::Begin, markedText.StartOffset, &markedText});
      offsets.push_back({Offset::End, markedText.EndOffset, &markedText});
    }

    // 2. Sort offsets such that begin offsets are in paint order and end offsets are in reverse paint order.
    std::ranges::sort(offsets,
                      [](const Offset &a, const Offset &b)
                      {
                        return a.Value < b.Value
                               || (a.Value == b.Value && a.OffsetKind == b.OffsetKind
                                   && a.OffsetKind == Offset::Begin
                                   && a.Text->MarkedType < b.Text->MarkedType)
                               || (a.Value == b.Value && a.OffsetKind == b.OffsetKind
                                   && a.OffsetKind == Offset::End && a.Text->MarkedType > b.Text->MarkedType);
                      });

    // 3. Compute intersection.
    List<MarkedText> result;
    result.reserve(numberOfMarkedTexts);
    HashSet<const MarkedText *> processedMarkedTexts;
    uint32 offsetSoFar = offsets[0].Value;
    for (uint32 i = 1; i < numberOfOffsets; ++i)
    {
      if (offsets[i].Value > offsets[i - 1].Value)
      {
        if (overlapStrategy == OverlapStrategy::Frontmost)
        {
          Maybe<uint32> frontmost;
          for (uint32 j = 0; j < i; ++j)
          {
            if (!processedMarkedTexts.contains(offsets[j].Text)
                && (!frontmost || offsets[j].Text->MarkedType > offsets[*frontmost].Text->MarkedType))
            {
              frontmost = j;
            }
          }
          if (frontmost)
          {
            result.push_back({offsetSoFar, offsets[i].Value, offsets[*frontmost].Text->MarkedType,
                              offsets[*frontmost].Text->HighlightName});
          }
        }
        else
        {
          // The appended marked texts may not be in paint order. We will fix this up at the end of this
          // function.
          for (uint32 j = 0; j < i; ++j)
          {
            if (!processedMarkedTexts.contains(offsets[j].Text))
            {
              result.push_back({offsetSoFar, offsets[i].Value, offsets[j].Text->MarkedType,
                                offsets[j].Text->HighlightName, offsets[j].Text->Priority});
            }
          }
        }
        offsetSoFar = offsets[i].Value;
      }
      if (offsets[i].OffsetKind == Offset::End)
      {
        processedMarkedTexts.insert(offsets[i].Text);
      }
    }
    // Fix up; sort the marked texts so that they are in paint order.
    if (overlapStrategy == OverlapStrategy::None)
    {
      std::ranges::sort(result,
                        [](const MarkedText &a, const MarkedText &b)
                        {
                          return a.StartOffset < b.StartOffset
                                 || (a.StartOffset == b.StartOffset && a.MarkedType < b.MarkedType);
                        });
    }
    return result;
  }

  List<MarkedText> MarkedText::CollectForHighlights(const RenderText &renderer,
                                                    const TextBoxSelectableRange &selectableRange,
                                                    PaintPhase phase) noexcept
  {
    List<MarkedText> markedTexts;
    RenderHighlight renderHighlight;
    auto &parentRenderer = *renderer.Parent();
    auto &parentStyle = parentRenderer.Style();
    auto &document = renderer.GetDocument();
    if (auto *highlightRegistry = document.HighlightRegistryIfExists())
    {
      for (auto &highlightName : highlightRegistry->HighlightNames())
      {
        auto renderStyle = parentRenderer.GetUncachedPseudoStyle(
          style::PseudoElementRequest(PseudoElementType::Highlight, highlightName), &parentStyle);
        if (!renderStyle)
        {
          continue;
        }
        if (renderStyle->TextDecorationLineInEffect().IsNone() && phase == PaintPhase::Decoration)
        {
          continue;
        }
        auto *highlight = highlightRegistry->Get(highlightName);
        for (auto &highlightRange : highlight->HighlightRanges())
        {
          if (!renderHighlight.SetRenderRange(*highlightRange))
          {
            continue;
          }
          // A collapsed range highlights nothing, live or static.
          if (highlightRange->Range().Collapsed())
          {
            continue;
          }
          if (auto *staticRange = DynamicDowncast<dom::StaticRange>(highlightRange->Range());
              staticRange && !staticRange->IsValid())
          {
            continue;
          }
          // FIXME: Potentially move this check elsewhere, to where we collect this range information.
          auto hasRenderer = [&]
          {
            for (auto &node : dom::IntersectingNodes(dom::MakeSimpleRange(highlightRange->Range())))
            {
              if (node.Renderer())
              {
                return true;
              }
            }
            return false;
          }();
          if (!hasRenderer)
          {
            continue;
          }

          auto [highlightStart, highlightEnd] = renderHighlight.RangeForTextBox(renderer, selectableRange);

          if (highlightStart < highlightEnd)
          {
            int32 currentPriority = highlight->Priority();
            // If we can just append it to the end, do that instead.
            if (markedTexts.empty() || markedTexts.back().Priority <= currentPriority)
            {
              markedTexts.emplace_back(highlightStart, highlightEnd, Type::Highlight, highlightName,
                                       currentPriority);
            }
            else
            {
              // Gets the first place such that it > currentPriority.
              auto it = std::upper_bound(markedTexts.begin(), markedTexts.end(), currentPriority,
                                         [](int32 targetMarkedTextPriority, const MarkedText &markedText)
                                         { return targetMarkedTextPriority > markedText.Priority; });

              auto insertIndex = it == markedTexts.end() ? 0 : std::distance(markedTexts.begin(), it) - 1;

              markedTexts.insert(
                markedTexts.begin() + insertIndex,
                MarkedText(highlightStart, highlightEnd, Type::Highlight, highlightName, currentPriority));
            }
          }
        }
      }
    }

    auto appendMarkedTextHighlights = [&](const dom::HighlightRegistry &registry, Type markedTextType)
    {
      for (auto &[name, highlight] : registry.Map())
      {
        for (auto &highlightRange : highlight->HighlightRanges())
        {
          if (!renderHighlight.SetRenderRange(*highlightRange))
          {
            continue;
          }

          auto [highlightStart, highlightEnd] = renderHighlight.RangeForTextBox(renderer, selectableRange);
          if (highlightStart < highlightEnd)
          {
            markedTexts.emplace_back(highlightStart, highlightEnd, markedTextType);
          }
        }
      }
    };

    if (document.GetSettings().ScrollToTextFragmentEnabled)
    {
      if (auto *registry = document.FragmentHighlightRegistryIfExists())
      {
        appendMarkedTextHighlights(*registry, Type::FragmentHighlight);
      }
    }

    // FIXME: The text extraction and app highlights' registries, once there are any.
    return markedTexts;
  }
}
