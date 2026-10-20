#pragma once

#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class MutationRecord;
  class MutationObserver;

  /// @see https://dom.spec.whatwg.org/#callbackdef-mutationcallback
  /// @see https://dom.spec.whatwg.org/#callbackdef-mutationcallback
  using MutationCallback = Func<void(const List<Ref<MutationRecord>> &records, MutationObserver &observer)>;
}