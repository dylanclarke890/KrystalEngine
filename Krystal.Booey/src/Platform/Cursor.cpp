#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD IntPoint DetermineHotSpot(const Image &image, const IntPoint &specifiedHotSpot,
                                             float scale) noexcept
    {
      if (image.IsNull())
      {
        return {};
      }

      // Hot spot must be inside cursor rectangle.
      auto size = image.Size();
      IntRect imageRect(IntPoint(), IntSize(static_cast<int32>(size.Width() / scale),
                                            static_cast<int32>(size.Height() / scale)));
      if (imageRect.Contains(specifiedHotSpot))
      {
        return specifiedHotSpot;
      }

      // If hot spot is not specified externally, it can be extracted from some image formats (e.g. .cur).
      if (auto intrinsicHotSpot = image.HotSpot())
      {
        if (imageRect.Contains(*intrinsicHotSpot))
        {
          return *intrinsicHotSpot;
        }
      }

      return {};
    }
  }

  Cursor::Cursor(Image &image, const IntPoint &hotSpot, float scale) noexcept
      : _type(Type::Custom), _image(ShareRefPtr(&image)), _hotSpot(DetermineHotSpot(image, hotSpot, scale)),
        _imageScaleFactor(scale)
  {
  }
}
