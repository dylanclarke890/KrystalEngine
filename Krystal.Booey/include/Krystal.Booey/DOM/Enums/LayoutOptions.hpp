#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief What a document's layout update does besides laying it out.
  // FIXME: The other layout options (running post-layout tasks synchronously, ignoring pending style sheets,
  // forcing the layout of skipped content, not laying out ancestor documents, deferring layer position
  // updates), once there are any.
  enum class LayoutOptions : uint8
  {
    UpdateCompositingLayers,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::LayoutOptions, 1uz);
