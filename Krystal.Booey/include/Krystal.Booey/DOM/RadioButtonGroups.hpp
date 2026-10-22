#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;
}

namespace krys::boo::dom
{
  class RadioButtonGroup;

  /// @brief The radio button groups of a form, or of a tree's radio buttons without a form, by name.
  /// @see https://html.spec.whatwg.org/#radio-button-group
  class RadioButtonGroups
  {
    KRYS_NON_COPY_MOVABLE(RadioButtonGroups);

  private:
    using NameToGroupMap = HashMap<DOMStringAtom, UniquePtr<RadioButtonGroup>>;

    NameToGroupMap _nameToGroupMap;

  public:
    RadioButtonGroups() noexcept;
    ~RadioButtonGroups() noexcept;

    void Clear() noexcept;
    void AddButton(html::HTMLInputElement &element) noexcept;
    void UpdateCheckedState(html::HTMLInputElement &element) noexcept;
    void RequiredStateChanged(html::HTMLInputElement &element) noexcept;
    void RemoveButton(html::HTMLInputElement &element) noexcept;
    KRYS_NODISCARD RefPtr<html::HTMLInputElement>
      CheckedButtonForGroup(DOMStringView groupName) const noexcept;
    KRYS_NODISCARD bool HasCheckedButton(const html::HTMLInputElement &element) const noexcept;
    KRYS_NODISCARD bool IsInRequiredGroup(html::HTMLInputElement &element) const noexcept;

    /// @brief The buttons in @p element's group, in tree order.
    KRYS_NODISCARD List<Ref<html::HTMLInputElement>>
      GroupMembers(const html::HTMLInputElement &element) const noexcept;

  private:
    KRYS_NODISCARD RadioButtonGroup *Find(DOMStringView name) const noexcept;
  };
}
