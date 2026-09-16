#pragma once

#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class MutationRecord;
  class MutationObserver;

  /// @see https://dom.spec.whatwg.org/#callbackdef-mutationcallback
  using MutationCallback = Func<void(const List<MutationRecord> &records, MutationObserver &observer)>;
}