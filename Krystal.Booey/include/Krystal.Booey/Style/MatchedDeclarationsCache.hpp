#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  class CustomPropertyData;
  class Resolver;

  /// @brief Caches the styles built from identical matched declarations, so an element whose declarations
  /// match an earlier element's copies its non-inherited properties instead of applying them.
  class MatchedDeclarationsCache
  {
    KRYS_NON_COPY_MOVABLE(MatchedDeclarationsCache);

  public:
    struct Entry
    {
      RefPtr<const style::MatchResult> MatchResult;
      UniquePtr<const boo::RenderStyle> RenderStyle;
      UniquePtr<const boo::RenderStyle> ParentRenderStyle;

      KRYS_NODISCARD bool IsUsableAfterHighPriorityProperties(const boo::RenderStyle &) const noexcept;
    };

    struct Result
    {
      const MatchedDeclarationsCache::Entry &Entry;
      bool InheritedEqual;
    };

  private:
    WeakPtr<const Resolver> _owner;
    HashMap<uint32, List<Entry>> _entries;
    uint32 _additionsSinceLastSweep {0};
    Timer _sweepTimer;

  public:
    explicit MatchedDeclarationsCache(const Resolver &) noexcept;
    ~MatchedDeclarationsCache() noexcept;

    KRYS_NODISCARD static bool IsCacheable(const dom::Element &, const RenderStyle &,
                                           const RenderStyle &parentStyle) noexcept;
    KRYS_NODISCARD static uint32 ComputeHash(const MatchResult &,
                                             const CustomPropertyData &inheritedCustomProperties) noexcept;

    KRYS_NODISCARD Maybe<Result> Find(uint32 hash, const MatchResult &,
                                      const CustomPropertyData &inheritedCustomProperties,
                                      const RenderStyle &) noexcept;
    void Add(const RenderStyle &, const RenderStyle &parentStyle, uint32 hash, const MatchResult &) noexcept;
    void Remove(uint32 hash) noexcept;

    // Every N additions to the matched declaration cache trigger a sweep where entries holding
    // the last reference to a style declaration are garbage collected.
    void Invalidate() noexcept;
    void ClearEntriesAffectedByViewportUnits() noexcept;

  private:
    template <typename TCallback>
    void RemoveAllMatching(const TCallback &matches) noexcept;

    void Sweep() noexcept;
  };
}
