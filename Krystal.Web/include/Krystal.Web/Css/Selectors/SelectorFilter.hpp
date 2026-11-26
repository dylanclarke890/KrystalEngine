#pragma once

#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Containers/BloomFilter.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"

namespace krys::web
{
  class ContainerNode;
  class Element;
}

namespace krys::web::css
{
  class Selector;

  /// @brief A counting Bloom filter of the identifiers (tag names, IDs, classes and attribute names) on the
  /// ancestors of the element being styled. A selector whose descendant and child combinators require an
  /// identifier the filter doesn't have can't match, so it's rejected without walking the tree.
  class SelectorFilter
  {
  public:
    /// @brief A selector's hashes for `FastRejectSelector`: up to four, ending at the first zero.
    using Hashes = Array<uint32, 4>;

    /// @brief A selector's identifier hashes by kind.
    struct CollectedSelectorHashes
    {
      using HashVector = SmallList<uint32, 8>;
      HashVector Ids;
      HashVector Classes;
      HashVector Tags;
      HashVector Attributes;
    };

  private:
    enum class IncludeRightmost : bool
    {
      No,
      Yes,
    };

    struct ParentStackFrame
    {
      const web::Element *Element {nullptr};
      SmallList<uint32, 4> IdentifierHashes;
    };

    /// @brief The ancestors, root first, with the hashes each added to the filter.
    List<ParentStackFrame> _parentStack;
    // 2^12 slots hold 100 distinct identifiers with about 0.2% false positives.
    constexpr static uint32 BloomFilterKeyBits = 12;
    CountingBloomFilter<BloomFilterKeyBits> _ancestorIdentifierFilter;

  public:
    /// @brief Adds @p parent, whose parent must be the last element pushed (or which must be the root if the
    /// stack is empty).
    void PushParent(const web::Element &parent) noexcept;

    /// @brief Adds @p parent, first adding all its ancestors if the stack is empty.
    void PushParentInitializingIfNeeded(const web::Element &parent) noexcept;

    /// @brief Removes the last element pushed.
    void PopParent() noexcept;

    /// @brief Removes elements until @p parent is the last, or all of them if @p parent is null or absent.
    void PopParentsUntil(const web::Element *parent) noexcept;

    /// @brief Whether the stack ends at @p parentNode, or is empty if @p parentNode isn't an element.
    KRYS_NODISCARD bool ParentStackIsConsistent(const ContainerNode *parentNode) const noexcept;

    void ParentStackReserveInitialCapacity(size_t initialCapacity) noexcept
    {
      _parentStack.reserve(initialCapacity);
    }

    /// @brief Whether a selector with @p hashes certainly can't match an element whose ancestors are the
    /// stack.
    KRYS_NODISCARD bool FastRejectSelector(const Hashes &hashes) const noexcept;

    /// @brief The hashes of the identifiers @p selector requires of the element's ancestors, the most
    /// selective first.
    KRYS_NODISCARD static Hashes CollectHashes(const Selector &selector) noexcept;

    /// @brief Appends the hashes of @p element's identifiers to @p identifierHashes.
    static void CollectElementIdentifierHashes(const web::Element &element,
                                               SmallList<uint32, 4> &identifierHashes) noexcept;

    /// @brief Adds the hash of the identifier simple selector @p selector requires, if any, to
    /// @p collectedHashes.
    static void CollectSimpleSelectorHash(CollectedSelectorHashes &collectedHashes,
                                          const Selector &selector) noexcept;

  private:
    void InitializeParentStack(const web::Element &parent) noexcept;

    static void CollectSelectorHashes(CollectedSelectorHashes &collectedHashes,
                                      const Selector &rightmostSelector,
                                      IncludeRightmost includeRightmost) noexcept;

    KRYS_NODISCARD static Hashes
      ChooseSelectorHashesForFilter(const CollectedSelectorHashes &collectedSelectorHashes) noexcept;
  };

  inline bool SelectorFilter::FastRejectSelector(const Hashes &hashes) const noexcept
  {
    for (auto hash : hashes)
    {
      if (!hash)
      {
        return false;
      }
      if (!_ancestorIdentifierFilter.MayContain(hash))
      {
        return true;
      }
    }
    return false;
  }
}
