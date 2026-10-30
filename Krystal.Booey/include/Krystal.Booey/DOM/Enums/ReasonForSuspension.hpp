#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  /// @brief Why a document's tasks and active objects (its timers, its animation frame callbacks) are
  /// suspended.
  enum class ReasonForSuspension : uint8
  {
    JavaScriptDebuggerPaused,
    WillDeferLoading,
    /// @brief The document is in the back/forward cache.
    BackForwardCache,
    /// @brief The host suspended the page.
    PageWillBeSuspended
  };
}
