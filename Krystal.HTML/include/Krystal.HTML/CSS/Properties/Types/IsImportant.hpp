#pragma once

#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  /// @brief Represents the `!important` flag in CSS declarations.
  struct IsImportant : public StronglyTypedBool<IsImportant>
  {
    using Base::Base;
  };
}