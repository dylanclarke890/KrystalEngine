#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class Font;
  class FontCascadeDescription;
  class FontDescription;
  class FontRanges;
  class FontSelector;

  /// @brief Told when the fonts a selector provides change.
  class FontSelectorClient
  {
  public:
    virtual ~FontSelectorClient() noexcept = default;

    virtual void FontsNeedUpdate(FontSelector &selector) noexcept = 0;
  };

  /// @brief Provides the fonts of the families a document names: its web fonts and the generic families its
  /// settings resolve.
  class FontSelector : public RefCounted<FontSelector>, public CanMakeWeakPtr<FontSelector>
  {
  public:
    virtual ~FontSelector() noexcept = default;

    KRYS_NODISCARD virtual FontRanges FontRangesForFamily(const FontDescription &description,
                                                          const StringAtom &family) noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<Font> FallbackFontAt(const FontDescription &description,
                                                       size_t index) noexcept = 0;
    KRYS_NODISCARD virtual size_t FallbackFontCount() noexcept = 0;

    virtual void OpportunisticallyStartFontDataURLLoading(const FontCascadeDescription &description,
                                                          const StringAtom &family) noexcept = 0;

    virtual void FontCacheInvalidated() noexcept
    {
    }

    virtual void RegisterForInvalidationCallbacks(FontSelectorClient &client) noexcept = 0;
    virtual void UnregisterForInvalidationCallbacks(FontSelectorClient &client) noexcept = 0;

    KRYS_NODISCARD virtual uint32 UniqueId() const noexcept = 0;
    KRYS_NODISCARD virtual uint32 Version() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsSimpleFontSelectorForDescription() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsCSSFontSelector() const noexcept
    {
      return false;
    }
  };
}
