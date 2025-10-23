#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/List.hpp"

namespace Krys::Gfx
{
  template <typename T>
  struct InstanceData
  {
    BufferHandle Buffer;
    List<T> Data;
    VertexBufferLayout Layout {};
  };
}