#include "Krystal.Booey/SVG/Animation/SMILTime.hpp"

namespace krys::boo::svg
{
  SMILTime operator+(const SMILTime &a, const SMILTime &b) noexcept
  {
    if (a.IsUnresolved() || b.IsUnresolved())
    {
      return SMILTime::Unresolved();
    }
    if (a.IsIndefinite() || b.IsIndefinite())
    {
      return SMILTime::Indefinite();
    }
    return a.Value() + b.Value();
  }

  SMILTime operator-(const SMILTime &a, const SMILTime &b) noexcept
  {
    if (a.IsUnresolved() || b.IsUnresolved())
    {
      return SMILTime::Unresolved();
    }
    if (a.IsIndefinite() || b.IsIndefinite())
    {
      return SMILTime::Indefinite();
    }
    return a.Value() - b.Value();
  }

  SMILTime operator*(const SMILTime &a, const SMILTime &b) noexcept
  {
    if (a.IsUnresolved() || b.IsUnresolved())
    {
      return SMILTime::Unresolved();
    }
    if (!a.Value() || !b.Value())
    {
      return SMILTime(0);
    }
    if (a.IsIndefinite() || b.IsIndefinite())
    {
      return SMILTime::Indefinite();
    }
    return a.Value() * b.Value();
  }
}
