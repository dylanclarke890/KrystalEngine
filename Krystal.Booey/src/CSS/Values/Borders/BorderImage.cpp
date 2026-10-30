#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css
{
  Ref<ValueList> CreateBorderImageValue(BorderImageComponents &&components) noexcept
  {
    ValueListBuilder list;
    if (components.Source)
    {
      list.push_back(krys::move(components.Source));
    }

    if (components.Width || components.Outset)
    {
      ValueListBuilder listSlash;
      if (components.Slice)
      {
        listSlash.push_back(krys::move(components.Slice));
      }

      if (components.Width)
      {
        listSlash.push_back(krys::move(components.Width));
      }

      if (components.Outset)
      {
        listSlash.push_back(krys::move(components.Outset));
      }

      list.push_back(ValueList::CreateSlashSeparated(krys::move(listSlash)));
    }
    else if (components.Slice)
    {
      list.push_back(krys::move(components.Slice));
    }

    if (components.Repeat)
    {
      list.push_back(krys::move(components.Repeat));
    }

    return ValueList::CreateSpaceSeparated(krys::move(list));
  }
}
