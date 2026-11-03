#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html
{
  /// @brief The custom states of a custom element, which `:state()` matches; set through its internals.
  /// @see https://html.spec.whatwg.org/#customstateset
  class CustomStateSet : public RefCounted<CustomStateSet>
  {
  private:
    ListHashSet<dom::DOMStringAtom> _states;
    WeakPtr<dom::Element> _element;

  public:
    explicit CustomStateSet(dom::Element &element) noexcept;
    ~CustomStateSet() noexcept;

    KRYS_NODISCARD static Ref<CustomStateSet> Create(dom::Element &element) noexcept
    {
      return CreateRef<CustomStateSet>(element);
    }

    /// @return Whether @p state wasn't in the set.
    bool Add(dom::DOMStringAtom state) noexcept;

    /// @return Whether @p state was in the set.
    bool Delete(dom::DOMStringAtom state) noexcept;

    void Clear() noexcept;

    KRYS_NODISCARD bool Has(dom::DOMStringAtom state) const noexcept;

    KRYS_NODISCARD size_t Size() const noexcept;

    /// @brief The states, in the order they were added.
    KRYS_NODISCARD const ListHashSet<dom::DOMStringAtom> &States() const noexcept
    {
      return _states;
    }
  };
}
