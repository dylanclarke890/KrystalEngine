#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::Gfx
{
  template <typename T>
  struct InstanceData
  {
    BufferHandle Buffer;
    List<T> Data;
    VertexBufferLayout Layout {};

    constexpr static uint32 BatchSize = static_cast<uint32>(T::BatchSize);
    constexpr static uint32 BufferSize = static_cast<uint32>(T::BatchSize * sizeof(T));
  };
}