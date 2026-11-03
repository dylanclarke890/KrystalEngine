#include "Krystal.Booey/HTML/Forms/RadioButtonGroups.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>

namespace krys::boo::html
{
  /// @brief A group of radio buttons that share a name, at most one of which is checked.
  class RadioButtonGroup
  {
  private:
    List<WeakPtr<HTMLInputElement>> _members;
    WeakPtr<HTMLInputElement> _checkedButton;
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

    KRYS_NODISCARD RefPtr<HTMLInputElement> CheckedButton() const noexcept
    {
      return ShareRefPtr(_checkedButton.get());
    }

    void Add(HTMLInputElement &button) noexcept;
    void UpdateCheckedState(HTMLInputElement &button) noexcept;
    void RequiredStateChanged(HTMLInputElement &button) noexcept;
    void Remove(HTMLInputElement &button) noexcept;
    KRYS_NODISCARD bool Contains(const HTMLInputElement &button) const noexcept;
    KRYS_NODISCARD List<Ref<HTMLInputElement>> Members() const noexcept;

  private:
    void SetNeedsStyleRecalcForAllButtons() noexcept;
    void UpdateValidityForAllButtons() noexcept;
    KRYS_NODISCARD bool IsValid() const noexcept;
    void SetCheckedButton(HTMLInputElement *button) noexcept;
  };

  bool RadioButtonGroup::IsValid() const noexcept
  {
    return !IsRequired() || _checkedButton;
  }

  List<Ref<HTMLInputElement>> RadioButtonGroup::Members() const noexcept
  {
    List<Ref<HTMLInputElement>> sortedMembers;
    for (auto &member : _members)
    {
      if (auto *element = member.get())
      {
        sortedMembers.push_back(ShareRef(*element));
      }
    }
    std::ranges::sort(sortedMembers, [](auto &a, auto &b)
                      { return std::is_lt(dom::TreeOrder<dom::TreeType::ComposedTree>(*a, *b)); });
    return sortedMembers;
  }

  void RadioButtonGroup::SetCheckedButton(HTMLInputElement *button) noexcept
  {
    RefPtr<HTMLInputElement> oldCheckedButton = ShareRefPtr(_checkedButton.get());
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

  void RadioButtonGroup::Add(HTMLInputElement &button) noexcept
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

  void RadioButtonGroup::UpdateCheckedState(HTMLInputElement &button) noexcept
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

  void RadioButtonGroup::RequiredStateChanged(HTMLInputElement &button) noexcept
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

  void RadioButtonGroup::Remove(HTMLInputElement &button) noexcept
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

  bool RadioButtonGroup::Contains(const HTMLInputElement &button) const noexcept
  {
    return std::ranges::any_of(_members, [&](auto &member) { return member.get() == &button; });
  }

  RadioButtonGroups::RadioButtonGroups() noexcept = default;

  RadioButtonGroups::~RadioButtonGroups() noexcept = default;

  RadioButtonGroup *RadioButtonGroups::Find(dom::DOMStringView name) const noexcept
  {
    auto group = _nameToGroupMap.find(dom::DOMStringAtom(name));
    return group != _nameToGroupMap.end() ? group->second.get() : nullptr;
  }

  void RadioButtonGroups::Clear() noexcept
  {
    _nameToGroupMap.clear();
  }

  void RadioButtonGroups::AddButton(HTMLInputElement &element) noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return;
    }

    auto &group = _nameToGroupMap[dom::DOMStringAtom(name)];
    if (!group)
    {
      group = CreateUnique<RadioButtonGroup>();
    }
    group->Add(element);

    // FIXME: Telling accessibility (AXObjectCache::onRadioGroupMembershipChanged), once there's
    // accessibility.
  }

  List<Ref<HTMLInputElement>> RadioButtonGroups::GroupMembers(const HTMLInputElement &element) const noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    if (!element.IsRadioButton() || !element.HasAttribute(u8"name"))
    {
      return {};
    }

    auto *group = Find(element.Name());
    return group ? group->Members() : List<Ref<HTMLInputElement>> {};
  }

  void RadioButtonGroups::UpdateCheckedState(HTMLInputElement &element) noexcept
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

  void RadioButtonGroups::RequiredStateChanged(HTMLInputElement &element) noexcept
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

  RefPtr<HTMLInputElement> RadioButtonGroups::CheckedButtonForGroup(dom::DOMStringView name) const noexcept
  {
    auto *group = Find(name);
    return group ? group->CheckedButton() : nullptr;
  }

  bool RadioButtonGroups::HasCheckedButton(const HTMLInputElement &element) const noexcept
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

  bool RadioButtonGroups::IsInRequiredGroup(HTMLInputElement &element) const noexcept
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

  void RadioButtonGroups::RemoveButton(HTMLInputElement &element) noexcept
  {
    krys_debug_assert(element.IsRadioButton());
    auto name = element.Name();
    if (name.empty())
    {
      return;
    }

    auto group = _nameToGroupMap.find(dom::DOMStringAtom(name));
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
