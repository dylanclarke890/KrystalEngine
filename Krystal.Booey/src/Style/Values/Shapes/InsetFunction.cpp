#include "Krystal.Booey/Style/Values/Shapes/InsetFunction.hpp"

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/TinyLRUCache.hpp"

namespace krys::boo::style
{
  // MARK: - Path Caching

  struct RoundedInsetPathPolicy : public TinyLRUCachePolicy<FloatRoundedRect, boo::Path>
  {
  public:
    KRYS_NODISCARD static bool IsKeyNull(const FloatRoundedRect &rect) noexcept
    {
      return rect.IsEmpty();
    }

    KRYS_NODISCARD static boo::Path CreateValueForKey(const FloatRoundedRect &rect) noexcept
    {
      boo::Path path;
      path.AddRoundedRect(rect, PathRoundedRect::Strategy::PreferBezier);
      return path;
    }
  };

  static const boo::Path &CachedRoundedInsetPath(const FloatRoundedRect &rect)
  {
    static NeverDestroyed<TinyLRUCache<FloatRoundedRect, boo::Path, 4, RoundedInsetPathPolicy>> cache;
    return cache.get().Get(rect);
  }

  // MARK: - Path

  boo::Path PathComputation<Inset>::operator()(const Inset &value, const FloatRect &boundingBox) noexcept
  {
    auto boundingSize = boundingBox.Size();

    auto left = Evaluate<float>(value.Insets.Left(), boundingSize.Width(), style::ZoomNeeded {});
    auto top = Evaluate<float>(value.Insets.Top(), boundingSize.Height(), style::ZoomNeeded {});
    auto rect = FloatRect {
      left + boundingBox.X(), top + boundingBox.Y(),
      std::max<float>(boundingSize.Width() - left
                        - Evaluate<float>(value.Insets.Right(), boundingSize.Width(), style::ZoomNeeded {}),
                      0),
      std::max<float>(boundingSize.Height() - top
                        - Evaluate<float>(value.Insets.Bottom(), boundingSize.Height(), style::ZoomNeeded {}),
                      0)};

    auto radii = Evaluate<FloatRoundedRect::Radii>(value.Radii, boundingSize, style::ZoomNeeded {});
    radii.Scale(CalcBorderRadiiConstraintScaleFor(rect, radii));

    return CachedRoundedInsetPath(FloatRoundedRect {rect, radii});
  }
}
