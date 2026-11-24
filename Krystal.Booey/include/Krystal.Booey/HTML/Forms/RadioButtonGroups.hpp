#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"

namespace krys::boo
{
  class HTMLInputElement;
}

namespace krys::boo::html
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

    /// @brief Forgets every group.
    void Clear() noexcept;

    /// @brief Adds a radio button to its name's group, or removes it.
    void AddButton(HTMLInputElement &element) noexcept;

    /// @brief Updates @p element's group as its checkedness changes: the group's checked button, and the
    /// validity of its other buttons.
    void UpdateCheckedState(HTMLInputElement &element) noexcept;

    /// @brief Updates @p element's group as its `required` attribute changes.
    void RequiredStateChanged(HTMLInputElement &element) noexcept;

    void RemoveButton(HTMLInputElement &element) noexcept;

    /// @brief The checked button of the group named @p groupName, if any.
    KRYS_NODISCARD RefPtr<HTMLInputElement> CheckedButtonForGroup(DOMStringView groupName) const noexcept;

    /// @brief Whether @p element's group has a checked button, and whether any of its buttons is required.
    KRYS_NODISCARD bool HasCheckedButton(const HTMLInputElement &element) const noexcept;

    KRYS_NODISCARD bool IsInRequiredGroup(HTMLInputElement &element) const noexcept;

    /// @brief The buttons in @p element's group, in tree order.
    KRYS_NODISCARD List<Ref<HTMLInputElement>> GroupMembers(const HTMLInputElement &element) const noexcept;

  private:
    /// @brief The group named @p name, if any.
    KRYS_NODISCARD RadioButtonGroup *Find(DOMStringView name) const noexcept;
  };
}
