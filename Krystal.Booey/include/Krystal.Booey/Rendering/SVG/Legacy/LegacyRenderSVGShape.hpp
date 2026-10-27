#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::svg
{
  class SVGGraphicsElement;
}

namespace krys::boo
{
  class GraphicsContextStateSaver;

  enum class LegacyRenderSVGShapeType : uint8
  {
    Empty,
    Path,
    Line,
    Rectangle,
    RoundedRectangle,
    Ellipse,
    Circle
  };

  /// @brief The renderer of an SVG shape: it fills, strokes and hit tests the element's path.
  class LegacyRenderSVGShape : public LegacyRenderSVGModelObject
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGShape);

    friend FloatRect SVGRenderSupport::CalculateApproximateStrokeBoundingBox(const RenderElement &) noexcept;

  public:
    using ShapeType = LegacyRenderSVGShapeType;

    enum PointCoordinateSpace
    {
      GlobalCoordinateSpace,
      LocalCoordinateSpace
    };

  protected:
    FloatRect _fillBoundingBox;
    mutable Markable<FloatRect> _strokeBoundingBox;
    mutable Markable<FloatRect> _approximateStrokeBoundingBox;

  private:
    FloatRect _repaintBoundingBox;

    bool _needsBoundariesUpdate {false}; // Default is false, the cached rects are empty from the beginning.
    bool _needsShapeUpdate {true}; // Default is true, so we grab a Path object once from SVGGraphicsElement.
    bool _needsTransformUpdate {
      true}; // Default is true, so we grab a AffineTransform object once from SVGGraphicsElement.
    bool _fillRequiresClip {true};

  protected:
    ShapeType _shapeType {ShapeType::Empty};

  private:
    AffineTransform _localTransform;
    UniquePtr<Path> _path;

  public:
    ~LegacyRenderSVGShape() noexcept override;

    KRYS_NODISCARD svg::SVGGraphicsElement &GraphicsElement() const noexcept;

    void SetNeedsShapeUpdate() noexcept
    {
      _needsShapeUpdate = true;
    }
    void SetNeedsBoundariesUpdate() noexcept final
    {
      _needsBoundariesUpdate = true;
    }
    void SetNeedsTransformUpdate() noexcept final
    {
      _needsTransformUpdate = true;
    }
    virtual void FillShape(GraphicsContext &) const noexcept;
    virtual void StrokeShape(GraphicsContext &) const noexcept;
    KRYS_NODISCARD virtual bool IsRenderingDisabled() const noexcept = 0;

    KRYS_NODISCARD bool IsPointInFill(const FloatPoint &) noexcept;
    KRYS_NODISCARD bool IsPointInStroke(const FloatPoint &) noexcept;

    KRYS_NODISCARD float GetTotalLength() const noexcept;
    KRYS_NODISCARD FloatPoint GetPointAtLength(float distance) const noexcept;

    KRYS_NODISCARD bool HasPath() const noexcept
    {
      return static_cast<bool>(_path);
    }
    KRYS_NODISCARD Path &GetPath() const noexcept
    {
      krys_debug_assert(_path);
      return *_path;
    }
    void ClearPath() noexcept
    {
      _path = nullptr;
    }

    KRYS_NODISCARD ShapeType GetShapeType() const noexcept
    {
      return _shapeType;
    }

  protected:
    LegacyRenderSVGShape(Type, svg::SVGGraphicsElement &, RenderStyle &&) noexcept;

    Path &EnsurePath() noexcept;

    virtual void UpdateShapeFromElement() noexcept = 0;
    KRYS_NODISCARD virtual bool IsEmpty() const noexcept;
    KRYS_NODISCARD virtual bool
      ShapeDependentStrokeContains(const FloatPoint &, PointCoordinateSpace = GlobalCoordinateSpace) noexcept;
    KRYS_NODISCARD virtual bool ShapeDependentFillContains(const FloatPoint &, const WindRule) const noexcept;
    KRYS_NODISCARD float StrokeWidth() const noexcept;

    KRYS_NODISCARD bool HasNonScalingStroke() const noexcept;
    KRYS_NODISCARD AffineTransform NonScalingStrokeTransform() const noexcept;
    KRYS_NODISCARD Path *NonScalingStrokePath(const Path *, const AffineTransform &) const noexcept;

    KRYS_NODISCARD virtual FloatRect
      AdjustStrokeBoundingBoxForMarkersAndZeroLengthLinecaps(RepaintRectCalculation,
                                                             FloatRect strokeBoundingBox) const noexcept
    {
      return strokeBoundingBox;
    }

    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept final;
    KRYS_NODISCARD FloatRect ApproximateStrokeBoundingBox() const noexcept;

    KRYS_NODISCARD bool FillRequiresClip() const noexcept
    {
      return _fillRequiresClip;
    }

  private:
    // Hit-detection separated for the fill and the stroke
    KRYS_NODISCARD bool FillContains(const FloatPoint &, bool requiresFill = true,
                                     const WindRule fillRule = WindRule::NonZero) noexcept;
    KRYS_NODISCARD bool StrokeContains(const FloatPoint &, bool requiresStroke = true) noexcept;

    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept final;
    KRYS_NODISCARD FloatRect DecoratedBoundingBox() const noexcept final;
    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept final
    {
      return _localTransform;
    }
    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept final
    {
      return _localTransform;
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGShape";
    }

    void Layout() noexcept final;
    void Paint(PaintInfo &, const LayoutPoint &) noexcept final;
    void AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &additionalOffset,
                           const RenderLayerModelObject *paintContainer = nullptr) const noexcept final;

    bool NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &pointInParent,
                          HitTestAction) noexcept final;

    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept final
    {
      return _fillBoundingBox;
    }
    KRYS_NODISCARD FloatRect CalculateStrokeBoundingBox() const noexcept;
    void UpdateRepaintBoundingBox() noexcept;

    KRYS_NODISCARD bool SetupNonScalingStrokeContext(AffineTransform &, GraphicsContextStateSaver &) noexcept;

    KRYS_NODISCARD FloatRect CalculateApproximateStrokeBoundingBox() const noexcept;

    KRYS_NODISCARD UniquePtr<Path> CreatePath() const noexcept;

    void FillShape(const RenderStyle &, GraphicsContext &) noexcept;
    void StrokeShapeInternal(const RenderStyle &, GraphicsContext &) noexcept;
    void StrokeShape(const RenderStyle &, GraphicsContext &) noexcept;
    void FillStrokeMarkers(PaintInfo &) noexcept;

    virtual void DrawMarkers(PaintInfo &) noexcept
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGShape)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGShape();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
