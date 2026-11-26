#pragma once

#include "Krystal.Text/EncodingRegistry.hpp"

namespace krys::web
{
  KRYS_NODISCARD const krys::text::EncodingRegistry &EncodingRegistry() noexcept;
}
