#pragma once

#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class MutationRecord;
  class MutationObserver;

  /// @see https://dom.spec.whatwg.org/#callbackdef-mutationcallback
  using MutationCallback = Func<void(const List<MutationRecord> &records, MutationObserver &observer)>;
}