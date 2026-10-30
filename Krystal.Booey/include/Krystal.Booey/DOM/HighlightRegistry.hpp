#pragma once

#include "Krystal.Booey/DOM/Highlight.hpp"
#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  /// @brief A document's custom highlights by name, in the order they were registered (which is the order
  /// they're painted in, below their priority).
  /// @see https://drafts.csswg.org/css-highlight-api-1/#highlight-registry
  class HighlightRegistry : public RefCounted<HighlightRegistry>
  {
  private:
    HashMap<DOMStringAtom, Ref<Highlight>> _map;
    List<DOMStringAtom> _highlightNames;

    HighlightRegistry() noexcept = default;

  public:
    KRYS_NODISCARD static Ref<HighlightRegistry> Create() noexcept
    {
      return AdoptRef(*new HighlightRegistry);
    }

    /// @brief Registers @p value as the highlight named @p key, in place of any highlight of that name.
    void Set(DOMStringAtom key, Ref<Highlight> &&value) noexcept;

    /// @brief The highlight named @p key, if any.
    KRYS_NODISCARD Highlight *Get(DOMStringAtom key) const noexcept;

    KRYS_NODISCARD bool Has(DOMStringAtom key) const noexcept
    {
      return _map.contains(key);
    }

    /// @brief Unregisters every highlight, clearing them.
    void Clear() noexcept;

    /// @brief Unregisters the highlight named @p key.
    /// @return Whether there was one.
    bool Delete(DOMStringAtom key) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _map.empty();
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _map.size();
    }

    /// @brief Adds @p value to the registry's annotation highlight (the text fragments' highlight).
    void AddAnnotationHighlightWithRange(Ref<StaticRange> &&value) noexcept;

    KRYS_NODISCARD const HashMap<DOMStringAtom, Ref<Highlight>> &Map() const noexcept
    {
      return _map;
    }

    KRYS_NODISCARD const List<DOMStringAtom> &HighlightNames() const noexcept
    {
      return _highlightNames;
    }
  };
}
