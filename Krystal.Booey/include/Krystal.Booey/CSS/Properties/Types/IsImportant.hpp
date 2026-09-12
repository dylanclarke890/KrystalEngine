#pragma once

#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::css
{
  /// @brief Represents the `!important` flag in CSS declarations.
  struct IsImportant : public StronglyTypedBool<IsImportant>
  {
    using Base::Base;
  };
}