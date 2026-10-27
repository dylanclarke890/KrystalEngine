#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/BlendMode.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"

namespace krys::gfx
{
  /// @brief The colour space a filter node computes in, and its result is in.
  enum class FilterColourSpace2D : uint8
  {
    SRGB,
    LinearSRGB
  };

  /// @brief What a filter node reads past the edges of its region.
  enum class FilterEdgeMode2D : uint8
  {
    /// @brief Transparent black.
    None,
    /// @brief The nearest pixel of the region.
    Duplicate,
    /// @brief The region repeated.
    Wrap
  };

  /// @brief A colour channel, as a displacement map selects one.
  enum class FilterChannel2D : uint8
  {
    Red,
    Green,
    Blue,
    Alpha
  };

  /// @brief How two filter inputs combine, as SVG's `feComposite` operators: the first input is the source
  /// and the second the destination, both premultiplied.
  enum class FilterCompositeOperator2D : uint8
  {
    Over,
    In,
    Out,
    Atop,
    Xor,
    Lighter,
    /// @brief k1 * i1 * i2 + k2 * i1 + k3 * i2 + k4, with `FilterNode2D::K`.
    Arithmetic
  };

  /// @brief What a filter node reads: one of the graph's sources, or an earlier node's result.
  struct FilterInput2D
  {
    enum class Kind : uint8
    {
      None,
      Source,
      Node
    };

    Kind Type {Kind::None};
    uint32 Index {0u};

    KRYS_NODISCARD static constexpr FilterInput2D Source(uint32 index) noexcept
    {
      return {.Type = Kind::Source, .Index = index};
    }
    KRYS_NODISCARD static constexpr FilterInput2D Node(uint32 index) noexcept
    {
      return {.Type = Kind::Node, .Index = index};
    }
  };

  /// @brief A texture a filter graph reads, its texel (0, 0) at the top-left of `Region`, in the graph's
  /// pixels. Texels outside the region read as transparent.
  struct FilterSource2D
  {
    TextureHandle Texture {};
    PixelRect2D Region {};
    FilterColourSpace2D ColourSpace {FilterColourSpace2D::SRGB};
    /// @brief Whether the texels' colours are multiplied by their alpha.
    bool Premultiplied {true};
  };

  /// @brief The light of a lighting node: in the direction of a distant light, or at a point, spreading in
  /// all directions or in a spot's cone.
  struct FilterLight2D
  {
    enum class Kind : uint8
    {
      Distant,
      Point,
      Spot
    };

    Kind Type {Kind::Distant};
    /// @brief A distant light: the unit vector towards it. A point or spot: where it is, in pixels from the
    /// top-left of the node's region, its z scaled as x is.
    Array<float, 3> Vector {0.f, 0.f, 1.f};
    /// @brief A spot: the unit vector it points along.
    Array<float, 3> Direction {0.f, 0.f, -1.f};
    /// @brief A spot: how its light falls off from the cone's axis.
    float SpecularExponent {1.f};
    /// @brief A spot: the cosine past which there's no light (0 for no cone), and the cosine past which the
    /// light starts to fade (it fades over the rest of the way, which antialiases the cone's edge).
    float ConeCutOff {0.f};
    float ConeFullLight {0.f};
  };

  /// @brief One step of a filter graph, which writes a texture over its region. Inputs read outside their
  /// own regions as transparent.
  struct FilterNode2D
  {
    enum class Kind : uint8
    {
      /// @brief `Matrix` on the input's unpremultiplied colours, as `feColorMatrix`.
      ColourMatrix,
      /// @brief `Table` on each unpremultiplied channel, as `feComponentTransfer`.
      ComponentTransfer,
      /// @brief The input moved by `Offset`, in pixels.
      Offset,
      /// @brief `Colour` all over.
      Flood,
      /// @brief The first input blended over the second with `Blend`, as `feBlend`.
      Blend,
      /// @brief The inputs combined with `Operator`, as `feComposite`.
      Composite,
      /// @brief `Kernel` convolved with the input, as `feConvolveMatrix`.
      ConvolveMatrix,
      /// @brief Each pixel's minimum (eroded) or maximum (dilated) over `Radius`, as `feMorphology`.
      Morphology,
      /// @brief A Gaussian blur by `StandardDeviation`, as `feGaussianBlur`.
      Blur,
      /// @brief The first input displaced by the second, as `feDisplacementMap`.
      DisplacementMap,
      /// @brief Light from `Light` lighting the input's alpha as a surface, as `feDiffuseLighting` and
      /// `feSpecularLighting`.
      DiffuseLighting,
      SpecularLighting,
      /// @brief Perlin noise, as `feTurbulence`.
      Turbulence,
      /// @brief The input's `TileRect` repeated, as `feTile`.
      Tile
    };

    Kind Type {Kind::Offset};
    Array<FilterInput2D, 2> Inputs {};
    /// @brief The pixels the node writes, in the graph's pixels.
    PixelRect2D Region {};
    FilterColourSpace2D ColourSpace {FilterColourSpace2D::SRGB};

    /// @brief `ColourMatrix`: four rows of five, each row's last value added, on colours from 0 to 1.
    Array<float, 20> Matrix {1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                             0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f};
    /// @brief `ComponentTransfer`: for each channel value from 0 to 255, the red, green, blue and alpha it
    /// becomes, from 0 to 255.
    List<Array<uint8, 4>> Table {};
    /// @brief `Offset`: in pixels. `DisplacementMap`: the scale, in pixels.
    Array<float, 2> Offset {};
    /// @brief `Flood`: premultiplied, from 0 to 1, in the node's colour space.
    Array<float, 4> Colour {};
    BlendMode Blend {BlendMode::Normal};
    FilterCompositeOperator2D Operator {FilterCompositeOperator2D::Over};
    Array<float, 4> K {};

    /// @brief `ConvolveMatrix`: the kernel's columns and rows, its values row by row, and the pixel it's
    /// centred on. At most 1024 values.
    Array<uint32, 2> Order {3u, 3u};
    List<float> Kernel {};
    Array<int32, 2> Target {1, 1};
    float Divisor {1.f};
    /// @brief From 0 to 1, added to each channel.
    float Bias {0.f};
    bool PreserveAlpha {false};
    /// @brief `ConvolveMatrix` and `Blur`: what's read past the region's edges.
    FilterEdgeMode2D EdgeMode {FilterEdgeMode2D::None};

    /// @brief `Morphology`: in whole pixels.
    Array<int32, 2> Radius {};
    bool Dilate {false};
    /// @brief `Blur`: in pixels, across and down.
    Array<float, 2> StandardDeviation {};

    /// @brief `DisplacementMap`: the second input's channels that move pixels across and down.
    FilterChannel2D XChannel {FilterChannel2D::Alpha};
    FilterChannel2D YChannel {FilterChannel2D::Alpha};

    /// @brief Lighting: the alpha surface's height scale, the constants and exponent, the light's colour (in
    /// the node's colour space, from 0 to 1) and the light.
    float SurfaceScale {1.f};
    float DiffuseConstant {1.f};
    float SpecularConstant {1.f};
    float SpecularExponent {1.f};
    Array<float, 3> LightingColour {1.f, 1.f, 1.f};
    FilterLight2D Light {};

    /// @brief `Turbulence`: fractal noise rather than turbulence, the base frequencies (in user units), the
    /// octaves and seed, whether tiles stitch, the tile they stitch (in user units), and the graph's pixels
    /// per user unit.
    bool FractalNoise {false};
    Array<float, 2> BaseFrequency {};
    uint32 Octaves {1u};
    int32 Seed {0};
    bool StitchTiles {false};
    Array<float, 2> StitchTileSize {};
    Array<float, 2> Scale {1.f, 1.f};

    /// @brief `Tile`: the rect of the input that repeats, in the graph's pixels.
    PixelRect2D TileRect {};
  };

  /// @brief A filter: nodes in the order they're evaluated, each reading sources or earlier nodes; the last
  /// node's result is the filter's.
  struct FilterGraph2D
  {
    List<FilterSource2D> Sources {};
    List<FilterNode2D> Nodes {};
  };

  /// @brief A filter's result: a premultiplied texture whose texel (0, 0) is the top-left of `Region`, and
  /// the result's colour space.
  struct FilterResult2D
  {
    TextureHandle Texture {};
    PixelRect2D Region {};
    FilterColourSpace2D ColourSpace {FilterColourSpace2D::SRGB};
  };
}
