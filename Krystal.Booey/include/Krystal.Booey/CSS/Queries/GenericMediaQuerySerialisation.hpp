#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"

namespace krys::boo::css::mq
{
  void Serialise(CSSOMString &builder, const Feature &feature) noexcept;

  void Serialise(CSSOMString &builder, const Condition &condition) noexcept;
}
