#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief `navigator.mimeTypes`: empty, as there are no plugins and no PDF viewer.
  /// @see https://html.spec.whatwg.org/#mimetypearray
  class DOMMimeTypeArray final : public RefCounted<DOMMimeTypeArray>
  {
    KRYS_NON_COPY_MOVABLE(DOMMimeTypeArray);

  public:
    DOMMimeTypeArray() noexcept = default;

    KRYS_NODISCARD static Ref<DOMMimeTypeArray> Create() noexcept
    {
      return CreateRef<DOMMimeTypeArray>();
    }

    /// @see https://html.spec.whatwg.org/#dom-mimetypearray-length
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return 0;
    }

    /// @brief Whether there's a MIME type at @p index (or named @p name): never.
    /// @see https://html.spec.whatwg.org/#dom-mimetypearray-item
    KRYS_NODISCARD bool HasItem(uint32 index) const noexcept
    {
      (void)index;
      return false;
    }

    /// @see https://html.spec.whatwg.org/#dom-mimetypearray-nameditem
    KRYS_NODISCARD bool HasNamedItem(dom::DOMStringView name) const noexcept
    {
      (void)name;
      return false;
    }
  };
}
