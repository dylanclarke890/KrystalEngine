#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextBreakingPositionContext.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::layout
{
  /// @brief Caches where text content breaks into inline text items, so that long text doesn't need breaking
  /// again when its content changes elsewhere or it's laid out again.
  // FIXME: Evict after a few idle seconds when the cache grows past its soft threshold, once there are
  // timers; it's evicted only at its hard cap for now.
  class TextBreakingPositionCache
  {
    KRYS_NON_COPY_MOVABLE(TextBreakingPositionCache);

  public:
    static constexpr size_t MinimumRequiredTextLengthForContentBreakCache = 5;
    static constexpr size_t MinimumRequiredContentBreaks = 3;

    struct Key
    {
      utf8_string Text;
      TextBreakingPositionContext Context;

      KRYS_NODISCARD friend bool operator==(const Key &, const Key &) noexcept = default;
    };
    using List = SmallList<size_t, 8>;

  private:
    struct KeyHash
    {
      KRYS_NODISCARD size_t operator()(const Key &key) const noexcept;
    };

    using TextBreakingPositionMap = HashMap<Key, List, KeyHash>;
    TextBreakingPositionMap _breakingPositionMap;
    size_t _cachedContentSize {0};

  public:
    KRYS_NODISCARD static TextBreakingPositionCache &Singleton() noexcept;

    TextBreakingPositionCache() noexcept = default;

    void Set(const Key &key, List &&breakingPositionList) noexcept;
    KRYS_NODISCARD const List *Get(const Key &key) const noexcept;

    void Clear() noexcept;

  private:
    void Evict() noexcept;
  };
}
