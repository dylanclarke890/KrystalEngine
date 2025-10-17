#include "Krystal.UI.Layout/Algorithm/Cache.hpp"
#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.UI.Layout/Algorithm/PixelGrid.hpp"

namespace Krys::UI::Layout
{
  static inline bool SizeIsExactAndMatchesOldMeasuredSize(SizingMode sizeMode, float size,
                                                          float lastComputedSize)
  {
    return sizeMode == SizingMode::StretchFit && Krys::InexactEquals(size, lastComputedSize);
  }

  static inline bool OldSizeIsMaxContentAndStillFits(SizingMode sizeMode, float size, SizingMode lastSizeMode,
                                                     float lastComputedSize)
  {
    return sizeMode == SizingMode::FitContent && lastSizeMode == SizingMode::MaxContent
           && (size >= lastComputedSize || Krys::InexactEquals(size, lastComputedSize));
  }

  static inline bool NewSizeIsStricterAndStillValid(SizingMode sizeMode, float size, SizingMode lastSizeMode,
                                                    float lastSize, float lastComputedSize)
  {
    return lastSizeMode == SizingMode::FitContent && sizeMode == SizingMode::FitContent
           && Krys::IsDefined(lastSize) && Krys::IsDefined(size) && Krys::IsDefined(lastComputedSize)
           && lastSize > size && (lastComputedSize <= size || Krys::InexactEquals(size, lastComputedSize));
  }

  bool CanUseCachedMeasurement(const SizingMode widthMode, const float availableWidth,
                               const SizingMode heightMode, const float availableHeight,
                               const SizingMode lastWidthMode, const float lastAvailableWidth,
                               const SizingMode lastHeightMode, const float lastAvailableHeight,
                               const float lastComputedWidth, const float lastComputedHeight,
                               const float marginRow, const float marginColumn, const Config *const config)
  {
    if ((Krys::IsDefined(lastComputedHeight) && lastComputedHeight < 0)
        || ((Krys::IsDefined(lastComputedWidth)) && lastComputedWidth < 0))
    {
      return false;
    }

    const float pointScaleFactor = config->GetPointScaleFactor();

    bool useRoundedComparison = config != nullptr && pointScaleFactor != 0;
    const float effectiveWidth = useRoundedComparison
                                   ? RoundValueToPixelGrid(availableWidth, pointScaleFactor, false, false)
                                   : availableWidth;
    const float effectiveHeight = useRoundedComparison
                                    ? RoundValueToPixelGrid(availableHeight, pointScaleFactor, false, false)
                                    : availableHeight;
    const float effectiveLastWidth =
      useRoundedComparison ? RoundValueToPixelGrid(lastAvailableWidth, pointScaleFactor, false, false)
                           : lastAvailableWidth;
    const float effectiveLastHeight =
      useRoundedComparison ? RoundValueToPixelGrid(lastAvailableHeight, pointScaleFactor, false, false)
                           : lastAvailableHeight;

    const bool hasSameWidthSpec =
      lastWidthMode == widthMode && Krys::InexactEquals(effectiveLastWidth, effectiveWidth);
    const bool hasSameHeightSpec =
      lastHeightMode == heightMode && Krys::InexactEquals(effectiveLastHeight, effectiveHeight);

    const bool widthIsCompatible =
      hasSameWidthSpec
      || SizeIsExactAndMatchesOldMeasuredSize(widthMode, availableWidth - marginRow, lastComputedWidth)
      || OldSizeIsMaxContentAndStillFits(widthMode, availableWidth - marginRow, lastWidthMode,
                                         lastComputedWidth)
      || NewSizeIsStricterAndStillValid(widthMode, availableWidth - marginRow, lastWidthMode,
                                        lastAvailableWidth, lastComputedWidth);

    const bool heightIsCompatible =
      hasSameHeightSpec
      || SizeIsExactAndMatchesOldMeasuredSize(heightMode, availableHeight - marginColumn, lastComputedHeight)
      || OldSizeIsMaxContentAndStillFits(heightMode, availableHeight - marginColumn, lastHeightMode,
                                         lastComputedHeight)
      || NewSizeIsStricterAndStillValid(heightMode, availableHeight - marginColumn, lastHeightMode,
                                        lastAvailableHeight, lastComputedHeight);

    return widthIsCompatible && heightIsCompatible;
  }
}