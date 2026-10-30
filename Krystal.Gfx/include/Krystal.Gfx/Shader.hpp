#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::gfx
{
  enum class ShaderStage : uint8
  {
    Vertex,
    Fragment,
  };

  /// @brief A shader stage compiled from source.
  ///
  /// Sources are GLSL 4.50. Resources bind by slot, declared in the shader: `layout(binding = N, std140)` for
  /// uniform blocks and `layout(binding = N)` for samplers; vertex inputs use `layout(location = N)`.
  struct ShaderDesc
  {
    ShaderStage Stage {ShaderStage::Vertex};
    /// @brief The source; it must start with a `#version` line.
    utf8_stringview Source {};
    utf8_stringview DebugName {};
  };
}
