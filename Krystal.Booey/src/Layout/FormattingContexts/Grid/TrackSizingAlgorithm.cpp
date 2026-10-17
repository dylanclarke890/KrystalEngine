#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridLayout.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::layout
{
  namespace
  {
    struct UnsizedTrack
    {
      LayoutUnit BaseSize;
      LayoutUnit GrowthLimit;
    };

    // https://www.w3.org/TR/css-grid-1/#algo-init
    KRYS_NODISCARD List<UnsizedTrack>
      InitializeTrackSizes(const TrackSizingFunctionsList &trackSizingFunctionsList) noexcept
    {
      List<UnsizedTrack> unsizedTracks;
      unsizedTracks.reserve(trackSizingFunctionsList.size());
      for (const auto &trackSizingFunctions : trackSizingFunctionsList)
      {
        // For each track, if the track’s min track sizing function is:
        auto baseSize = [&] -> LayoutUnit
        {
          const auto &minTrackSizingFunction = trackSizingFunctions.Min;

          // A fixed sizing function
          // Resolve to an absolute length and use that size as the track’s initial base size.
          if (minTrackSizingFunction.IsLength())
          {
            const auto &trackBreadthLength = minTrackSizingFunction.Length();
            if (auto fixedValue = trackBreadthLength.TryFixed())
            {
              return style::Evaluate<LayoutUnit>(*fixedValue, style::ZoomNeeded {});
            }

            if (trackBreadthLength.TryPercentage())
            {
              // FIXME: Not implemented yet.
              krys_debug_assert(false);
              return {};
            }
          }

          // An intrinsic sizing function
          // Use an initial base size of zero.
          if (minTrackSizingFunction.IsContentSized())
          {
            return {};
          }

          krys_debug_assert(false);
          return {};
        };

        // For each track, if the track’s max track sizing function is:
        auto growthLimit = [&] -> LayoutUnit
        {
          const auto &maxTrackSizingFunction = trackSizingFunctions.Max;

          // A fixed sizing function
          // Resolve to an absolute length and use that size as the track’s initial growth limit.
          if (maxTrackSizingFunction.IsLength())
          {
            if (auto fixedValue = maxTrackSizingFunction.Length().TryFixed())
            {
              return style::Evaluate<LayoutUnit>(*fixedValue, style::ZoomNeeded {});
            }
          }

          // An intrinsic sizing function
          // A flexible sizing function
          // Use an initial growth limit of infinity.
          if (maxTrackSizingFunction.IsContentSized() || maxTrackSizingFunction.IsFlex())
          {
            return LayoutUnit::Max();
          }

          krys_debug_assert(false);
          return {};
        };

        unsizedTracks.push_back({baseSize(), growthLimit()});
      }
      return unsizedTracks;
    }
  }

  // https://drafts.csswg.org/css-grid-1/#algo-track-sizing
  TrackSizes TrackSizingAlgorithm::SizeTracks(const PlacedGridItems &,
                                              const TrackSizingFunctionsList &trackSizingFunctions) noexcept
  {
    // 1. Initialize Track Sizes
    auto unsizedTracks = InitializeTrackSizes(trackSizingFunctions);

    // FIXME: 2. Resolve Intrinsic Track Sizes
    // FIXME: 3. Maximize Tracks
    // FIXME: 4. Expand Flexible Tracks
    // FIXME: 5. Expand Stretched auto Tracks

    // Each track has a base size, a <length> which grows throughout the algorithm and
    // which will eventually be the track’s final size...
    TrackSizes trackSizes;
    trackSizes.reserve(unsizedTracks.size());
    for (const auto &unsizedTrack : unsizedTracks)
    {
      trackSizes.push_back(unsizedTrack.BaseSize);
    }
    return trackSizes;
  }
}
