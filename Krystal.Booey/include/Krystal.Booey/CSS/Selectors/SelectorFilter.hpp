#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/BloomFilter.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
}

namespace krys::boo::css
{
  class Selector;

  /// @brief A bloom filter of the identifiers (tag names, IDs, classes and attribute names) of the ancestors
  /// of the element being styled, for fast rejection of descendant and child selectors.
  class SelectorFilter
  {
  public:
    using Hashes = Array<uint32, 4>;

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
      Yes
    };

    struct ParentStackFrame
    {
      const dom::Element *Element {nullptr};
      SmallList<uint32, 4> IdentifierHashes;
    };

    List<ParentStackFrame> _parentStack;

    // With 100 unique strings in the filter, 2^12 slot table has false positive rate of ~0.2%.
    static constexpr uint32 BloomFilterKeyBits = 12;
    CountingBloomFilter<BloomFilterKeyBits> _ancestorIdentifierFilter;

  public:
    void PushParent(const dom::Element *parent) noexcept;
    void PushParentInitializingIfNeeded(const dom::Element &parent) noexcept;
    void PopParent() noexcept;
    void PopParentsUntil(const dom::Element *parent) noexcept;

    KRYS_NODISCARD bool ParentStackIsEmpty() const noexcept
    {
      return _parentStack.empty();
    }

    KRYS_NODISCARD bool ParentStackIsConsistent(const dom::ContainerNode *parentNode) const noexcept;

    void ParentStackReserveInitialCapacity(size_t initialCapacity) noexcept
    {
      _parentStack.reserve(initialCapacity);
    }

    KRYS_NODISCARD bool FastRejectSelector(const Hashes &hashes) const noexcept;

    KRYS_NODISCARD static Hashes CollectHashes(const Selector &selector) noexcept;
    static void CollectElementIdentifierHashes(const dom::Element &element,
                                               SmallList<uint32, 4> &identifierHashes) noexcept;
    static void CollectSimpleSelectorHash(CollectedSelectorHashes &collectedHashes,
                                          const Selector &selector) noexcept;

  private:
    void InitializeParentStack(const dom::Element &parent) noexcept;

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
