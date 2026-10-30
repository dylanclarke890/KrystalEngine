#pragma once

#include "Krystal.Booey/Graphics/FontCascadeFonts.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FontCascadeDescription;
  class FontSelector;

  /// @brief The parts of a font description other than the family that matter when fetching fonts.
  struct FontDescriptionKey
  {
    FontDescription Description;

    FontDescriptionKey() noexcept = default;

    explicit FontDescriptionKey(const FontDescription &description) noexcept : Description(description)
    {
    }

    KRYS_NODISCARD bool operator==(const FontDescriptionKey &) const noexcept = default;
  };

  struct FontDescriptionKeyHash
  {
    KRYS_NODISCARD size_t operator()(const FontDescriptionKey &key) const noexcept;
  };

  /// @brief The name of a font family, hashed and compared with the rules for font family names (ASCII
  /// case-insensitively) instead of as a string.
  class FontFamilyName
  {
  private:
    StringAtom _name;

  public:
    FontFamilyName() noexcept;
    FontFamilyName(const StringAtom &name) noexcept;

    KRYS_NODISCARD const StringAtom &String() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD bool operator==(const FontFamilyName &other) const noexcept;
    KRYS_NODISCARD size_t Hash() const noexcept;
  };

  struct FontCascadeCacheKey
  {
    // Shared with the lower level FontCache (caching Font objects)
    FontDescriptionKey DescriptionKey;
    SmallList<FontFamilyName, 3> Families;
    uint32 FontSelectorId {0};
    uint32 FontSelectorVersion {0};
    bool HasComplexFontSelector {true};

    KRYS_NODISCARD bool operator==(const FontCascadeCacheKey &) const noexcept = default;
  };

  struct FontCascadeCacheKeyHash
  {
    KRYS_NODISCARD size_t operator()(const FontCascadeCacheKey &key) const noexcept;
  };

  /// @brief The fonts of every font cascade description in use, shared by the cascades with equal
  /// descriptions.
  class FontCascadeCache
  {
    KRYS_NON_COPY_MOVABLE(FontCascadeCache);

  private:
    HashMap<FontCascadeCacheKey, Ref<FontCascadeFonts>, FontCascadeCacheKeyHash> _entries;
    uint32 _pruneCounter {0};

  public:
    FontCascadeCache() noexcept = default;

    KRYS_NODISCARD static FontCascadeCache &Singleton() noexcept;

    void Invalidate() noexcept;
    void ClearWidthCaches() noexcept;
    void PruneUnreferencedEntries() noexcept;
    void PruneSystemFallbackFonts() noexcept;

    KRYS_NODISCARD Ref<FontCascadeFonts> RetrieveOrAddCachedFonts(const FontCascadeDescription &description,
                                                                  FontSelector *fontSelector) noexcept;

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _entries.size();
    }
  };
}
