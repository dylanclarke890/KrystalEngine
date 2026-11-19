#pragma once

#include "Krystal.Text/EncodingRegistry.hpp"

namespace krys::boo
{
  KRYS_NODISCARD const krys::text::EncodingRegistry &EncodingRegistry() noexcept;
}
