#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css
{
  Ref<ValueList> CreateBorderImageValue(BorderImageComponents components) noexcept
  {
    // A value list can't hold the empty width of `slice / / outset`; the computed values that call this
    // always have every part.
    krys_debug_assert(!components.Outset || components.Width);

    ValueListBuilder parts;
    auto append = [](ValueListBuilder &list, RefPtr<Value> &part)
    {
      if (part)
      {
        list.push_back(Ref<Value>(krys::move(part)));
      }
    };

    append(parts, components.Source);
    if (components.Width || components.Outset)
    {
      // The slice, width and outset are one part, separated by slashes.
      ValueListBuilder sizes;
      append(sizes, components.Slice);
      append(sizes, components.Width);
      append(sizes, components.Outset);
      parts.push_back(ValueList::CreateSlashSeparated(krys::move(sizes)));
    }
    else
    {
      append(parts, components.Slice);
    }
    append(parts, components.Repeat);

    return ValueList::CreateSpaceSeparated(krys::move(parts));
  }
}
