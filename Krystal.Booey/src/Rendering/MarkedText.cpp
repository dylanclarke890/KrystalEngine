#include "Krystal.Booey/Rendering/MarkedText.hpp"
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
}
