#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::pal
{
  /// @brief The system cursors a window can show over its client area.
  enum class CursorShape : uint8
  {
    /// @brief The normal arrow.
    Arrow,
    /// @brief The text selection I-beam.
    IBeam,
    /// @brief The pointing hand, over links and buttons.
    Hand,
    Crosshair,
    /// @brief Busy: the application can't take input.
    Wait,
    /// @brief Working in the background: the arrow with a busy indicator.
    Progress,
    /// @brief The arrow with a question mark.
    Help,
    NotAllowed,
    /// @brief Arrows in four directions.
    Move,
    /// @brief Resizing up or down.
    ResizeNS,
    /// @brief Resizing left or right.
    ResizeEW,
    /// @brief Resizing towards the top right or the bottom left.
    ResizeNESW,
    /// @brief Resizing towards the top left or the bottom right.
    ResizeNWSE,
    /// @brief A drag that a drop wouldn't take: the system's drag and drop cursor.
    DragNone,
    /// @brief A drag that a drop would move: the system's drag and drop cursor.
    DragMove,
    /// @brief A drag that a drop would copy: the system's drag and drop cursor.
    DragCopy,
    /// @brief A drag that a drop would link to: the system's drag and drop cursor.
    DragLink,
    /// @brief No cursor.
    Hidden,
  };

  /// @brief A cursor drawn from an image, which a window can show in place of a system cursor.
  class CursorImage final : public RefCounted<CursorImage>
  {
    KRYS_NON_COPY_MOVABLE(CursorImage);

  private:
    uint32 _width;
    uint32 _height;
    List<uint32> _pixels;
    int32 _hotSpotX;
    int32 _hotSpotY;

  public:
    /// @brief An image @p width by @p height, of @p pixels (row after row, each ARGB with straight alpha in
    /// the high byte), that points at (@p hotSpotX, @p hotSpotY) in it.
    CursorImage(uint32 width, uint32 height, List<uint32> pixels, int32 hotSpotX, int32 hotSpotY) noexcept
        : _width(width), _height(height), _pixels(krys::move(pixels)), _hotSpotX(hotSpotX),
          _hotSpotY(hotSpotY)
    {
    }

    KRYS_NODISCARD uint32 Width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD uint32 Height() const noexcept
    {
      return _height;
    }

    KRYS_NODISCARD const List<uint32> &Pixels() const noexcept
    {
      return _pixels;
    }

    KRYS_NODISCARD int32 HotSpotX() const noexcept
    {
      return _hotSpotX;
    }

    KRYS_NODISCARD int32 HotSpotY() const noexcept
    {
      return _hotSpotY;
    }
  };
}
