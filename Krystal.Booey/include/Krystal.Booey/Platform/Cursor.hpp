#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  enum class PlatformCursorType : uint8
  {
    Invalid,
    Pointer,
    Cross,
    Hand,
    IBeam,
    Wait,
    Help,
    EastResize,
    NorthResize,
    NorthEastResize,
    NorthWestResize,
    SouthResize,
    SouthEastResize,
    SouthWestResize,
    WestResize,
    NorthSouthResize,
    EastWestResize,
    NorthEastSouthWestResize,
    NorthWestSouthEastResize,
    ColumnResize,
    RowResize,
    Move,
    VerticalText,
    Cell,
    ContextMenu,
    Alias,
    Progress,
    NoDrop,
    Copy,
    None,
    NotAllowed,
    ZoomIn,
    ZoomOut,
    Grab,
    Grabbing,
    MiddlePanning,
    EastPanning,
    NorthPanning,
    NorthEastPanning,
    NorthWestPanning,
    SouthPanning,
    SouthEastPanning,
    SouthWestPanning,
    WestPanning,
    /// @brief An image (`cursor: url()`).
    Custom,
  };

  /// @brief The mouse cursor the page asks the host to show: a predefined one, or an image with its hot spot.
  class Cursor
  {
  public:
    using Type = PlatformCursorType;

  private:
    Type _type {Type::Invalid};
    RefPtr<Image> _image;
    IntPoint _hotSpot;
    float _imageScaleFactor {1};

  public:
    Cursor() noexcept = default;

    explicit Cursor(Type type) noexcept : _type(type)
    {
    }

    /// @brief A cursor showing @p image, an image @p scale times its natural size in pixels (from an
    /// `image-set()`), with the hot spot at @p hotSpot (in the image's natural size) if that's in it, or at
    /// the image's own hot spot (a `.cur` file's), or else at its top left.
    Cursor(Image &image, const IntPoint &hotSpot, float scale) noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    /// @brief A custom cursor's image, or null.
    KRYS_NODISCARD Image *GetImage() const noexcept
    {
      return _image.get();
    }

    /// @brief Where a custom cursor points, in its image's natural size.
    KRYS_NODISCARD const IntPoint &HotSpot() const noexcept
    {
      return _hotSpot;
    }

    /// @brief How many of a custom cursor's image pixels make a pixel of its natural size.
    KRYS_NODISCARD float ImageScaleFactor() const noexcept
    {
      return _imageScaleFactor;
    }

    KRYS_NODISCARD friend bool operator==(const Cursor &, const Cursor &) noexcept = default;
  };
}
