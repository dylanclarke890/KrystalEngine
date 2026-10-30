#include "Krystal.Booey/DOM/UserActionElementSet.hpp"
#include "Krystal.Booey/DOM/Element.hpp"

namespace krys::boo::dom
{
  void UserActionElementSet::Clear() noexcept
  {
    for (auto &[element, flags] : _elements)
    {
      const_cast<Element *>(element)->SetUserActionElement(false);
    }
    _elements.clear();
  }

  bool UserActionElementSet::HasFlag(const Element &element, Flag flag) const noexcept
  {
    // Caller has the responsibility to check IsUserActionElement before calling this.
    krys_debug_assert(element.IsUserActionElement());

    auto iterator = _elements.find(&element);
    return iterator != _elements.end() && iterator->second.Contains(flag);
  }

  void UserActionElementSet::ClearFlags(Element &element, EnumSet<Flag> flags) noexcept
  {
    krys_debug_assert(!flags.IsEmpty());

    if (!element.IsUserActionElement())
    {
      return;
    }

    auto iterator = _elements.find(&element);
    krys_debug_assert(iterator != _elements.end());
    auto updatedFlags = iterator->second - flags;
    if (updatedFlags.IsEmpty())
    {
      element.SetUserActionElement(false);
      _elements.erase(iterator);
    }
    else
    {
      iterator->second = updatedFlags;
    }
  }

  void UserActionElementSet::SetFlags(Element &element, EnumSet<Flag> flags) noexcept
  {
    krys_debug_assert(!flags.IsEmpty());

    _elements[&element].Add(flags);

    element.SetUserActionElement(true);
  }
}
