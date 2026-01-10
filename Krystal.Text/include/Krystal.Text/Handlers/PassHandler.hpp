#pragma once

#include "Krystal.Text/Handlers/PassThroughHandler.hpp"

namespace Krys::Text::Handlers
{
  /// @brief An error handler that tells an encoding that it will pass through any errors, without doing any
  /// adjustment, correction or checking. Does not imply it is ignorable, unlike AssumeValid which can
  /// invoke UB if an error occurs.
  class PassHandler : public ::Krys::Text::Handlers::PassThroughHandlerWith<false>
  {
  };

  /// @brief An instance of PassHandler for ease of use.
  constexpr inline PassHandler Pass = {};
}