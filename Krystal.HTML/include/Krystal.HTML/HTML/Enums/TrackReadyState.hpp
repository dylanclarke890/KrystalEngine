#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::TrackReadyState, 4uz);