#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <limits>
#include <random>

namespace Krys::Maths
{
  class Random
  {
  public:
    static void Init()
    {
      _randomEngine.seed(std::random_device()());
    }

    static uint32 UInt()
    {
      return Rand();
    }

    static uint32 UInt(uint32 max)
    {
      return Rand() % (max + 1);
    }

    static uint32 UInt(uint32 min, uint32 max)
    {
      return min + (Rand() % (max - min + 1));
    }

    static int32 Int()
    {
      return static_cast<int>(Rand());
    }

    static int32 Int(uint32 max)
    {
      return static_cast<int>(UInt(max));
    }

    static int32 Int(uint32 min, uint32 max)
    {
      return min + (Rand() % (max - min + 1));
    }

    static float Float()
    {
      return static_cast<float>(Rand()) / static_cast<float>(std::numeric_limits<uint32>::max());
    }

    static float Float(float max)
    {
      return Float(0, max);
    }

    static float Float(float min, float max)
    {
      return Float() * (max - min) + min;
    }

    template <VECTOR_TEMPLATE_PARAMS>
    static auto Vector()
    {
      VECTOR_TYPE vec;
      vec = MapEach(vec, [](auto) { return T(Float()); });
      return vec;
    }

    template <VECTOR_TEMPLATE_PARAMS>
    static auto Vector(float min, float max)
    {
      VECTOR_TYPE vec;
      vec = MapEach(vec, [&min, &max](auto) { return T(Float(min, max)); });
      return vec;
    }

  private:
    inline static std::mt19937 _randomEngine;
    inline static std::uniform_int_distribution<std::mt19937::result_type> _distribution;

    inline static uint32 Rand()
    {
      return _distribution(_randomEngine);
    }
  };
}