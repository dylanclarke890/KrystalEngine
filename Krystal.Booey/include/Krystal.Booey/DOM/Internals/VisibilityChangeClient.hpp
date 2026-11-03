#pragma once

#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::dom
{
  /// @brief An object told when its document's visibility state changes, once registered with it
  /// (`Document::RegisterForVisibilityStateChangedCallbacks`).
  class VisibilityChangeClient : public AbstractRefCountedAndCanMakeWeakPtr<VisibilityChangeClient>
  {
  public:
    virtual ~VisibilityChangeClient() noexcept = default;

    /// @brief The document's visibility state changed.
    virtual void VisibilityStateChanged() noexcept = 0;
  };
}
