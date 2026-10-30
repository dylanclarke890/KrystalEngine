#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

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
  };

  /// @brief The mouse cursor the page asks the host to show.
  // FIXME: Custom cursors (images and hot spots) come with cursor images.
  class Cursor
  {
  public:
    using Type = PlatformCursorType;

  private:
    Type _type {Type::Invalid};

  public:
    constexpr Cursor() noexcept = default;

    constexpr explicit Cursor(Type type) noexcept : _type(type)
    {
    }

    KRYS_NODISCARD constexpr Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD friend constexpr bool operator==(const Cursor &, const Cursor &) noexcept = default;
  };
}
