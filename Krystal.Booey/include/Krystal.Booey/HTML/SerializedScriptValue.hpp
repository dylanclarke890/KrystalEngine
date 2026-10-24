#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @brief A value the page passes where script would pass any JavaScript value: a message's data, a
  /// history entry's state, an error. The host derives its own values from it; null stands for the spec's
  /// null and undefined.
  /// @see https://html.spec.whatwg.org/#structuredserialize
  class SerializedScriptValue : public RefCounted<SerializedScriptValue>
  {
    KRYS_NON_COPY_MOVABLE(SerializedScriptValue);

  public:
    SerializedScriptValue() noexcept = default;
    virtual ~SerializedScriptValue() noexcept = default;

    /// @brief A copy of the value, as structured cloning makes one: by default the value itself, for values
    /// that don't change.
    /// @see https://html.spec.whatwg.org/#structuredclone
    KRYS_NODISCARD virtual Ref<SerializedScriptValue> Clone() noexcept
    {
      return ShareRef(*this);
    }
  };
}
