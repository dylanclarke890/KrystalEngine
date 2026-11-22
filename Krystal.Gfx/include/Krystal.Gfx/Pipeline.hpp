#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Gfx/CompareOp.hpp"
#include "Krystal.Gfx/Formats.hpp"
#include "Krystal.Gfx/Handles.hpp"

namespace krys::gfx
{
  enum class VertexInputRate : uint8
  {
    PerVertex,
    PerInstance,
  };

  struct VertexAttribute
  {
    /// @brief The shader input's `layout(location = N)`.
    uint32 Location {0u};
    VertexFormat Format {VertexFormat::Float};
    /// @brief The offset of the attribute in a vertex, in bytes.
    uint32 Offset {0u};
  };

  /// @brief The layout of one vertex buffer slot.
  struct VertexBufferLayout
  {
    uint32 Stride {0u};
    VertexInputRate InputRate {VertexInputRate::PerVertex};
    SmallList<VertexAttribute, 8> Attributes;
  };

  enum class PrimitiveTopology : uint8
  {
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
  };

  enum class BlendFactor : uint8
  {
    Zero,
    One,
    SourceColour,
    OneMinusSourceColour,
    DestinationColour,
    OneMinusDestinationColour,
    SourceAlpha,
    OneMinusSourceAlpha,
    DestinationAlpha,
    OneMinusDestinationAlpha,
    ConstantColour,
    OneMinusConstantColour,
  };

  enum class BlendOp : uint8
  {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
  };

  enum class ColourChannel : uint8
  {
    Red,
    Green,
    Blue,
    Alpha,
  };

  consteval ColourChannel MaxEnumValue(ColourChannel) noexcept
  {
    return ColourChannel::Alpha;
  }

  using ColourWriteMask = EnumSet<ColourChannel>;

  constexpr ColourWriteMask AllColourChannels {ColourChannel::Red, ColourChannel::Green, ColourChannel::Blue,
                                               ColourChannel::Alpha};

  /// @brief How fragment output is combined with the colour target. One state applies to every colour target.
  struct BlendState
  {
    bool Enabled {false};
    BlendFactor SourceColour {BlendFactor::One};
    BlendFactor DestinationColour {BlendFactor::Zero};
    BlendOp ColourOp {BlendOp::Add};
    BlendFactor SourceAlpha {BlendFactor::One};
    BlendFactor DestinationAlpha {BlendFactor::Zero};
    BlendOp AlphaOp {BlendOp::Add};
    ColourWriteMask WriteMask {AllColourChannels};

    constexpr bool operator==(const BlendState &) const noexcept = default;

    /// @brief Source-over blending of premultiplied colours.
    KRYS_NODISCARD constexpr static BlendState PremultipliedAlpha() noexcept
    {
      return {.Enabled = true,
              .SourceColour = BlendFactor::One,
              .DestinationColour = BlendFactor::OneMinusSourceAlpha,
              .SourceAlpha = BlendFactor::One,
              .DestinationAlpha = BlendFactor::OneMinusSourceAlpha};
    }

    /// @brief Source-over blending of straight (unpremultiplied) colours.
    KRYS_NODISCARD constexpr static BlendState StraightAlpha() noexcept
    {
      return {.Enabled = true,
              .SourceColour = BlendFactor::SourceAlpha,
              .DestinationColour = BlendFactor::OneMinusSourceAlpha,
              .SourceAlpha = BlendFactor::One,
              .DestinationAlpha = BlendFactor::OneMinusSourceAlpha};
    }
  };

  enum class StencilOp : uint8
  {
    Keep,
    Zero,
    Replace,
    IncrementAndClamp,
    DecrementAndClamp,
    Invert,
    IncrementAndWrap,
    DecrementAndWrap,
  };

  struct StencilFaceState
  {
    CompareOp Compare {CompareOp::Always};
    StencilOp Fail {StencilOp::Keep};
    StencilOp DepthFail {StencilOp::Keep};
    StencilOp Pass {StencilOp::Keep};

    constexpr bool operator==(const StencilFaceState &) const noexcept = default;
  };

  struct DepthStencilState
  {
    bool DepthTest {false};
    bool DepthWrite {false};
    CompareOp DepthCompare {CompareOp::Less};
    bool StencilTest {false};
    uint8 StencilReadMask {0xFFu};
    uint8 StencilWriteMask {0xFFu};
    StencilFaceState StencilFront {};
    StencilFaceState StencilBack {};

    constexpr bool operator==(const DepthStencilState &) const noexcept = default;
  };

  enum class CullMode : uint8
  {
    None,
    Front,
    Back,
  };

  enum class FrontFace : uint8
  {
    CounterClockwise,
    Clockwise,
  };

  enum class FillMode : uint8
  {
    Solid,
    Wireframe,
  };

  struct RasterState
  {
    CullMode Cull {CullMode::None};
    FrontFace Front {FrontFace::CounterClockwise};
    FillMode Fill {FillMode::Solid};
    /// @brief Whether draws are clipped to the scissor rectangle set with `CommandBuffer::SetScissor`.
    bool ScissorTest {false};

    constexpr bool operator==(const RasterState &) const noexcept = default;
  };

  /// @brief Everything that affects how the GPU runs a draw's shaders, built once and bound per draw.
  struct PipelineDesc
  {
    ShaderHandle VertexShader {};
    ShaderHandle FragmentShader {};
    /// @brief One layout per vertex buffer slot.
    SmallList<VertexBufferLayout, 2> VertexBuffers;
    PrimitiveTopology Topology {PrimitiveTopology::Triangles};
    BlendState Blend {};
    DepthStencilState DepthStencil {};
    RasterState Raster {};
    /// @brief The formats of the colour targets the pipeline renders into.
    SmallList<TextureFormat, 4> ColourFormats;
    TextureFormat DepthStencilFormat {TextureFormat::Undefined};
    utf8_stringview DebugName;
  };
}
