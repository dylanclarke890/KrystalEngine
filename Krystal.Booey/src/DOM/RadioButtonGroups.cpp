#include "Krystal.Booey/DOM/RadioButtonGroups.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  /// @brief A group of radio buttons that share a name, at most one of which is checked.
  class RadioButtonGroup
  {
  private:
    List<WeakPtr<html::HTMLInputElement>> _members;
    WeakPtr<html::HTMLInputElement> _checkedButton;
    size_t _requiredCount {0};

  public:
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return std::ranges::none_of(_members, [](auto &member) { return member.get() != nullptr; });
    }

    KRYS_NODISCARD bool IsRequired() const noexcept
    {
      return _requiredCount;
    }

    KRYS_NODISCARD RefPtr<html::HTMLInputElement> CheckedButton() const noexcept
    {
      return ShareRefPtr(_checkedButton.get());
    }

    void Add(html::HTMLInputElement &button) noexcept;
    void UpdateCheckedState(html::HTMLInputElement &button) noexcept;
    void RequiredStateChanged(html::HTMLInputElement &button) noexcept;
    void Remove(html::HTMLInputElement &button) noexcept;
    KRYS_NODISCARD bool Contains(const html::HTMLInputElement &button) const noexcept;
    KRYS_NODISCARD List<Ref<html::HTMLInputElement>> Members() const noexcept;

  private:
    void SetNeedsStyleRecalcForAllButtons() noexcept;
    void UpdateValidityForAllButtons() noexcept;
    KRYS_NODISCARD bool IsValid() const noexcept;
    void SetCheckedButton(html::HTMLInputElement *button) noexcept;
  };

  bool RadioButtonGroup::IsValid() const noexcept
  {
    return !IsRequired() || _checkedButton;
  }

  List<Ref<html::HTMLInputElement>> RadioButtonGroup::Members() const noexcept
  {
    List<Ref<html::HTMLInputElement>> sortedMembers;
    for (auto &member : _members)
    {
      if (auto *element = member.get())
      {
        sortedMembers.push_back(ShareRef(*element));
      }
    }
    std::ranges::sort(sortedMembers,
                      [](auto &a, auto &b) { return std::is_lt(TreeOrder<TreeType::ComposedTree>(*a, *b)); });
    return sortedMembers;
  }

  void RadioButtonGroup::SetCheckedButton(html::HTMLInputElement *button) noexcept
  {
    RefPtr<html::HTMLInputElement> oldCheckedButton = ShareRefPtr(_checkedButton.get());
    if (oldCheckedButton.get() == button)
    {
      return;
    }

    bool hadCheckedButton = _checkedButton.get();
    bool willHaveCheckedButton = button;
    if (hadCheckedButton != willHaveCheckedButton)
    {
      SetNeedsStyleRecalcForAllButtons();
    }
    _checkedButton = CreateWeakPtr(button);
    if (oldCheckedButton)
    {
      oldCheckedButton->SetChecked(false);
    }
  }

  void RadioButtonGroup::Add(html::HTMLInputElement &button) noexcept
  {
    krys_debug_assert(button.IsRadioButton());
    if (Contains(button))
    {
      return;
    }
    _members.push_back(CreateWeakPtr(&button));

    bool groupWasValid = IsValid();
    if (button.IsRequired())
    {
      ++_requiredCount;
    }
    if (button.Checked())
    {
      SetCheckedButton(&button);
    }

    bool groupIsValid = IsValid();
    if (groupWasValid != groupIsValid)
    {
      UpdateValidityForAllButtons();
    }
    else if (!groupIsValid)
    {
      // A radio button not in a group is always valid. We need to make it invalid only if the group is
      // invalid.
      button.UpdateValidity();
    }
  }

  void RadioButtonGroup::UpdateCheckedState(html::HTMLInputElement &button) noexcept
  {
    krys_debug_assert(button.IsRadioButton());
    krys_debug_assert(Contains(button));
    bool wasValid = IsValid();
    if (button.Checked())
    {
      SetCheckedButton(&button);
    }
    else
    {
      if (_checkedButton.get() == &button)
      {
        SetCheckedButton(nullptr);
      }
    }
    if (wasValid != IsValid())
    {
      UpdateValidityForAllButtons();
    }
  }

  void RadioButtonGroup::RequiredStateChanged(html::HTMLInputElement &button) noexcept
  {
    krys_debug_assert(button.IsRadioButton());
    krys_debug_assert(Contains(button));
    bool wasValid = IsValid();
    if (button.IsRequired())
    {
      ++_requiredCount;
    }
    else
    {
      krys_debug_assert(_requiredCount);
      --_requiredCount;
    }
    if (wasValid != IsValid())
    {
      UpdateValidityForAllButtons();
    }
  }

  void RadioButtonGroup::Remove(html::HTMLInputElement &button) noexcept
  {
    krys_debug_assert(button.IsRadioButton());
    auto member = std::ranges::find_if(_members, [&](auto &member) { return member.get() == &button; });
    if (member == _members.end())
    {
      return;
    }

    bool wasValid = IsValid();
    _members.erase(member);
    if (button.IsRequired())
    {
      krys_debug_assert(_requiredCount);
      --_requiredCount;
    }
    if (_checkedButton)
    {
      button.InvalidateStyleForSubtree();
      if (_checkedButton.get() == &button)
      {
        _checkedButton = nullptr;
        SetNeedsStyleRecalcForAllButtons();
      }
    }

    if (IsEmpty())
    {
      krys_debug_assert(!_requiredCount);
      krys_debug_assert(!_checkedButton);
    }
    else if (wasValid != IsValid())
    {
      UpdateValidityForAllButtons();
    }
    if (!wasValid)
    {
      // A radio button not in a group is always valid. We need to make it valid only if the group was
      // invalid.
      button.UpdateValidity();
    }
  }

  void RadioButtonGroup::SetNeedsStyleRecalcForAllButtons() noexcept
  {
    for (auto &button : Members())
    {
      krys_debug_assert(button->IsRadioButton());
      button->InvalidateStyleForSubtree();
    }
  }

  void RadioButtonGroup::UpdateValidityForAllButtons() noexcept
  {
    for (auto &button : Members())
    {
      krys_debug_assert(button->IsRadioButton());
      button->UpdateValidity();
    }
  }

  bool RadioButtonGroup::Contains(const html::HTMLInputElement &button) const noexcept
  {
    return std::ranges::any_of(_members, [&](auto &member) { return member.get() == &button; });
  }

  RadioButtonGroups::RadioButtonGroups() noexcept = default;

  RadioButtonGroups::~RadioButtonGroups() noexcept = default;

  RadioButtonGroup *RadioButtonGroups::Find(DOMStringView name) const noexcept
  {
    auto group = _nameToGroupMap.find(DOMStringAtom(name));
    return group != _nameToGroupMap.end() ? group->second.get() : nullptr;
  }

  void RadioButtonGroups::Clear() noexcept
  {
    _nameToGroupMap.clear();
  }

  void RadioButtonGroups::AddButton(html::HTMLInputElement &element) noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return;
    }

    auto &group = _nameToGroupMap[DOMStringAtom(name)];
    if (!group)
    {
      group = CreateUnique<RadioButtonGroup>();
    }
    group->Add(element);

    // FIXME: Telling accessibility (AXObjectCache::onRadioGroupMembershipChanged), once there's
    // accessibility.
  }

  List<Ref<html::HTMLInputElement>>
    RadioButtonGroups::GroupMembers(const html::HTMLInputElement &element) const noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    if (!element.IsRadioButton() || !element.HasAttribute(u8"name"))
    {
      return {};
    }

    auto *group = Find(element.Name());
    return group ? group->Members() : List<Ref<html::HTMLInputElement>> {};
  }

  void RadioButtonGroups::UpdateCheckedState(html::HTMLInputElement &element) noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return;
    }
    if (auto *group = Find(name))
    {
      group->UpdateCheckedState(element);
    }
  }

  void RadioButtonGroups::RequiredStateChanged(html::HTMLInputElement &element) noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return;
    }
    if (auto *group = Find(name))
    {
      group->RequiredStateChanged(element);
    }
  }

  RefPtr<html::HTMLInputElement> RadioButtonGroups::CheckedButtonForGroup(DOMStringView name) const noexcept
  {
    auto *group = Find(name);
    return group ? group->CheckedButton() : nullptr;
  }

  bool RadioButtonGroups::HasCheckedButton(const html::HTMLInputElement &element) const noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return element.Checked();
    }
    auto *group = Find(name);
    return group && group->CheckedButton();
  }

  bool RadioButtonGroups::IsInRequiredGroup(html::HTMLInputElement &element) const noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return false;
    }
    auto *group = Find(name);
    return group && group->IsRequired() && group->Contains(element);
  }

  void RadioButtonGroups::RemoveButton(html::HTMLInputElement &element) noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return;
    }

    auto group = _nameToGroupMap.find(DOMStringAtom(name));
    if (group == _nameToGroupMap.end())
    {
      return;
    }
    group->second->Remove(element);
    if (group->second->IsEmpty())
    {
      _nameToGroupMap.erase(group);
    }

    // FIXME: Telling accessibility (AXObjectCache::onRadioGroupMembershipChanged), once there's
    // accessibility.
  }
}
