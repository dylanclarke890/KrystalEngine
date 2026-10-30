#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class FloatingObject;
  class RenderBlockFlow;
  class RenderBox;

  /// @brief The shape of @p renderer's `shape-outside`.
  KRYS_NODISCARD Ref<const LayoutShape> MakeShapeForShapeOutside(const RenderBox &renderer) noexcept;

  /// @brief How far a float's shape lets a line into its margin box, on each side.
  class ShapeOutsideDeltas final
  {
  private:
    LayoutUnit _leftMarginBoxDelta;
    LayoutUnit _rightMarginBoxDelta;
    LayoutUnit _borderBoxLineTop;
    LayoutUnit _lineHeight;
    bool _lineOverlapsShape : 1 {false};
    bool _isValid : 1 {false};

  public:
    ShapeOutsideDeltas() noexcept = default;

    ShapeOutsideDeltas(LayoutUnit leftMarginBoxDelta, LayoutUnit rightMarginBoxDelta, bool lineOverlapsShape,
                       LayoutUnit borderBoxLineTop, LayoutUnit lineHeight) noexcept
        : _leftMarginBoxDelta(leftMarginBoxDelta), _rightMarginBoxDelta(rightMarginBoxDelta),
          _borderBoxLineTop(borderBoxLineTop), _lineHeight(lineHeight), _lineOverlapsShape(lineOverlapsShape),
          _isValid(true)
    {
    }

    KRYS_NODISCARD bool IsForLine(LayoutUnit borderBoxLineTop, LayoutUnit lineHeight) const noexcept
    {
      return _isValid && _borderBoxLineTop == borderBoxLineTop && _lineHeight == lineHeight;
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return _isValid;
    }
    KRYS_NODISCARD LayoutUnit LeftMarginBoxDelta() const noexcept
    {
      krys_debug_assert(_isValid);
      return _leftMarginBoxDelta;
    }
    KRYS_NODISCARD LayoutUnit RightMarginBoxDelta() const noexcept
    {
      krys_debug_assert(_isValid);
      return _rightMarginBoxDelta;
    }
    KRYS_NODISCARD bool LineOverlapsShape() const noexcept
    {
      krys_debug_assert(_isValid);
      return _lineOverlapsShape;
    }
  };

  /// @brief A float's `shape-outside`: its shape, and the room it leaves lines.
  class ShapeOutsideInfo final
  {
  private:
    const RenderBox &_renderer;

    mutable RefPtr<const LayoutShape> _shape;
    LayoutSize _cachedShapeLogicalSize;

    ShapeOutsideDeltas _shapeOutsideDeltas;

  public:
    explicit ShapeOutsideInfo(const RenderBox &renderer) noexcept : _renderer(renderer)
    {
    }

    KRYS_NODISCARD static bool IsEnabledFor(const RenderBox &box) noexcept;

    KRYS_NODISCARD ShapeOutsideDeltas ComputeDeltasForContainingBlockLine(
      const RenderBlockFlow &containingBlock, const FloatingObject &floatingObject, LayoutUnit lineTop,
      LayoutUnit lineHeight) noexcept;

    void InvalidateForSizeChangeIfNeeded() noexcept;

    KRYS_NODISCARD LayoutUnit ShapeLogicalBottom() const noexcept
    {
      return ComputedShape().ShapeMarginLogicalBoundingBox().MaxY();
    }

    void MarkShapeAsDirty() noexcept
    {
      _shape = nullptr;
    }
    KRYS_NODISCARD bool IsShapeDirty() const noexcept
    {
      return !_shape;
    }

    KRYS_NODISCARD LayoutRect ComputedShapePhysicalBoundingBox() const noexcept;
    KRYS_NODISCARD FloatPoint ShapeToRendererPoint(const FloatPoint &point) const noexcept;

    KRYS_NODISCARD const LayoutShape &ComputedShape() const noexcept;
  };
}
