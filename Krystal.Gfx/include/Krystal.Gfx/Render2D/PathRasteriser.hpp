#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"

namespace krys::gfx
{
  /// @brief How much of each pixel in a rect a shape covers.
  struct CoverageMask
  {
    /// @brief The pixels the mask covers; empty when the shape covers none.
    PixelRect2D Bounds {};
    /// @brief One byte per pixel, from 0 (outside) to 255 (inside), rows from the top.
    List<uint8> Coverage {};

    KRYS_NODISCARD uint8 At(int32 x, int32 y) const noexcept
    {
      return Coverage[static_cast<size_t>(y - Bounds.Y) * Bounds.Width + static_cast<size_t>(x - Bounds.X)];
    }
  };

  /// @brief Rasterises paths into coverage masks on the CPU.
  ///
  /// The path is transformed into pixels and its curves flattened into lines that stay within a fiftieth of
  /// a pixel of them. The lines cut inside convex curves, so curved edges sit slightly inward; the tolerance
  /// keeps that well below what thin curved strokes would show. Each row of pixels is then sampled along 16
  /// sub-scanlines: along each, the edges' crossings and windings give the spans inside the path under the
  /// fill rule, and every pixel adds the length of the spans over it. Coverage is exact across a row and in
  /// sixteenths down it.
  ///
  /// The rasteriser keeps its working memory between paths, so reuse one.
  class PathRasteriser
  {
    KRYS_NON_COPYABLE(PathRasteriser);

    /// @brief A non-horizontal line, from its top to its bottom.
    struct Edge
    {
      float X0;
      float Y0;
      float X1;
      float Y1;
      /// @brief +1 if the path runs down it, -1 if up.
      int32 Winding;
    };

    struct Crossing
    {
      float X;
      int32 Winding;
    };

  private:
    List<Edge> _edges {};
    /// @brief Indices of the edges that overlap the current row.
    List<uint32> _active {};
    List<Crossing> _crossings {};
    List<float> _row {};

  public:
    constexpr static uint32 SubScanlines = 16u;
    /// @brief The furthest a flattened curve strays from the curve, in pixels.
    constexpr static float FlatteningTolerance = 0.02f;

    PathRasteriser() noexcept = default;
    PathRasteriser(PathRasteriser &&) noexcept = default;
    PathRasteriser &operator=(PathRasteriser &&) noexcept = default;
    ~PathRasteriser() noexcept = default;

    /// @brief Rasterises `path`, mapped into pixels by `transform`, within `clip`. Paths with non-finite
    /// points cover nothing.
    KRYS_NODISCARD CoverageMask Rasterise(const Path2D &path, const Transform2D &transform, FillRule rule,
                                          const PixelRect2D &clip) noexcept;

  private:
    /// @brief Flattens the transformed path into `_edges`; false if a point isn't finite.
    KRYS_NODISCARD bool BuildEdges(const Path2D &path, const Transform2D &transform) noexcept;
    void AddLine(Vec2f from, Vec2f to) noexcept;
    void AddQuad(Vec2f from, Vec2f control, Vec2f to) noexcept;
    void AddCubic(Vec2f from, Vec2f control1, Vec2f control2, Vec2f to) noexcept;
    /// @brief Adds `weight` times the length of [left, right) that falls over each pixel of the row.
    void AccumulateSpan(float left, float right, float weight, int32 rowLeft, int32 rowRight) noexcept;
  };
}
