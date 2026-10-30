#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::dom
{
  class Element;
  class IdTargetObserverRegistry;

  /// @brief Observes which element has an ID in a tree scope, while it lives.
  class IdTargetObserver
  {
    KRYS_NON_COPY_MOVABLE(IdTargetObserver);

  private:
    IdTargetObserverRegistry *_registry {};
    DOMStringAtom _id;

  public:
    virtual ~IdTargetObserver() noexcept;

    /// @brief Called when @p element gains or loses the observed ID.
    virtual void IdTargetChanged(Element &element) noexcept = 0;

  protected:
    IdTargetObserver(IdTargetObserverRegistry &registry, DOMStringAtom id) noexcept;
  };
}
