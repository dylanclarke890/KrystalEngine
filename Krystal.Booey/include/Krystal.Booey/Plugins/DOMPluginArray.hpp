#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief `navigator.plugins`: empty, as there are no plugins and no PDF viewer.
  /// @see https://html.spec.whatwg.org/#pluginarray
  class DOMPluginArray final : public RefCounted<DOMPluginArray>
  {
    KRYS_NON_COPY_MOVABLE(DOMPluginArray);

  public:
    DOMPluginArray() noexcept = default;

    KRYS_NODISCARD static Ref<DOMPluginArray> Create() noexcept
    {
      return CreateRef<DOMPluginArray>();
    }

    /// @see https://html.spec.whatwg.org/#dom-pluginarray-refresh
    void Refresh() const noexcept
    {
    }

    /// @see https://html.spec.whatwg.org/#dom-pluginarray-length
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return 0;
    }

    /// @brief Whether there's a plugin at @p index (or named @p name): never.
    /// @see https://html.spec.whatwg.org/#dom-pluginarray-item
    KRYS_NODISCARD bool HasItem(uint32 index) const noexcept
    {
      (void)index;
      return false;
    }

    /// @see https://html.spec.whatwg.org/#dom-pluginarray-nameditem
    KRYS_NODISCARD bool HasNamedItem(dom::DOMStringView name) const noexcept
    {
      (void)name;
      return false;
    }
  };
}
