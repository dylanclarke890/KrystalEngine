#include "Krystal.Booey/DOM/PopoverData.hpp"

namespace krys::boo::dom
{
  Ref<ToggleEventTask> PopoverData::EnsureToggleEventTask(Element &element) noexcept
  {
    if (!_toggleEventTask)
    {
      _toggleEventTask = ToggleEventTask::Create(element);
    }
    return ShareRef(*_toggleEventTask);
  }
}
