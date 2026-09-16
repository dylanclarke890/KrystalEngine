#pragma once

#include "Krystal.Core/Hash.hpp"
#include "Krystal.Gfx/Colour.hpp"

namespace std
{
  template <>
  struct hash<krys::Gfx::Colourf>
  {
    size_t operator()(const krys::Gfx::Colourf &colour) const
    {
      return krys::Hash::Combine(colour.red, colour.green, colour.blue, colour.alpha);
    }
  };

  template <>
  struct hash<krys::Gfx::Colourb>
  {
    size_t operator()(const krys::Gfx::Colourb &colour) const
    {
      return krys::Hash::Combine(colour.red, colour.green, colour.blue, colour.alpha);
    }
  };

  template <>
  struct hash<krys::Gfx::ColourbPremultiplied>
  {
    size_t operator()(const krys::Gfx::ColourbPremultiplied &colour) const
    {
      return krys::Hash::Combine(colour.red, colour.green, colour.blue, colour.alpha);
    }
  };
}
