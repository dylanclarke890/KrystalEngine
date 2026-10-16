#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::mq
{
  void Serialise(StringBuilder &builder, const Feature &feature) noexcept;
  void Serialise(StringBuilder &builder, const Condition &condition) noexcept;
}
