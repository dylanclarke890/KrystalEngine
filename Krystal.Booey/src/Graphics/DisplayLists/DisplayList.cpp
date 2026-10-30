#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::display_list
{
  DisplayList::DisplayList(List<Item> &&items) noexcept : _items(krys::move(items))
  {
  }

  DisplayList::~DisplayList() noexcept = default;

  utf8_string DisplayList::AsText(EnumSet<AsTextFlag> flags) const noexcept
  {
    TextStream stream(TextStream::LineMode::MultipleLine, TextStream::Formatting::SVGStyleRect);
    if (flags.Contains(AsTextFlag::IncludeResourceIdentifiers))
    {
      stream.DumpProperty(u8"display-list-identifier", static_cast<const void *>(this));
    }
    for (const auto &item : _items)
    {
      if (!ShouldDumpItem(item, flags))
      {
        continue;
      }
      TextStream::GroupScope group(stream);
      DumpItem(stream, item, flags);
    }
    return stream.Release();
  }

  void DisplayList::Dump(TextStream &ts) const noexcept
  {
    TextStream::GroupScope group(ts);
    ts << "display list";
    for (const auto &item : _items)
    {
      TextStream::GroupScope itemGroup(ts);
      DumpItem(ts, item, {AsTextFlag::IncludePlatformOperations, AsTextFlag::IncludeResourceIdentifiers});
    }
  }

  TextStream &operator<<(TextStream &ts, const DisplayList &displayList) noexcept
  {
    displayList.Dump(ts);
    return ts;
  }
}
