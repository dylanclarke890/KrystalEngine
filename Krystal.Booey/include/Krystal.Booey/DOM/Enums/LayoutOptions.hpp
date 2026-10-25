#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief What a document's layout update does besides laying it out.
  // FIXME: The other layout options (running post-layout tasks synchronously, not laying out ancestor
  // documents, deferring layer position updates), once there are any.
  enum class LayoutOptions : uint8
  {
    IgnorePendingStylesheets,
    TreatContentVisibilityHiddenAsVisible,
    TreatContentVisibilityAutoAsVisible,
    TreatRevealedWhenFoundAsVisible,
    UpdateCompositingLayers,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::LayoutOptions, 5uz);
