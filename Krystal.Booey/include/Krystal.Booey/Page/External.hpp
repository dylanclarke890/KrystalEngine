#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief `window.external`, whose members do nothing.
  /// @note Obsolete.
  /// @see https://html.spec.whatwg.org/#external
  class External final : public RefCounted<External>
  {
    KRYS_NON_COPY_MOVABLE(External);

  public:
    External() noexcept = default;

    KRYS_NODISCARD static Ref<External> Create() noexcept
    {
      return CreateRef<External>();
    }

    /// @see https://html.spec.whatwg.org/#dom-external-addsearchprovider
    void AddSearchProvider() const noexcept
    {
    }

    /// @see https://html.spec.whatwg.org/#dom-external-issearchproviderinstalled
    KRYS_NODISCARD uint32 IsSearchProviderInstalled() const noexcept
    {
      return 0;
    }
  };
}
