#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#dom-track-none
  enum class TrackReadyState : uint8
  {
    None,
    Loading,
    Loaded,
    Error,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::TrackReadyState, 4uz);