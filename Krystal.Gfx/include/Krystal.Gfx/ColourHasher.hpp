#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Lib/Core/Hash.hpp"

namespace std
{
  template <>
  struct hash<Krys::Gfx::Colourf>
  {
    size_t operator()(const Krys::Gfx::Colourf &colour) const
    {
      return Krys::Hash::Combine(colour.red, colour.green, colour.blue, colour.alpha);
    }
  };

  template <>
  struct hash<Krys::Gfx::Colourb>
  {
    size_t operator()(const Krys::Gfx::Colourb &colour) const
    {
      return Krys::Hash::Combine(colour.red, colour.green, colour.blue, colour.alpha);
    }
  };

  template <>
  struct hash<Krys::Gfx::ColourbPremultiplied>
  {
    size_t operator()(const Krys::Gfx::ColourbPremultiplied &colour) const
    {
      return Krys::Hash::Combine(colour.red, colour.green, colour.blue, colour.alpha);
    }
  };
}
