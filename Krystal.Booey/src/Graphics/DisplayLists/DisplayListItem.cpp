#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItem.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::display_list
{
  void ApplyItem(GraphicsContext &context, const Item &item) noexcept
  {
    std::visit([&](const auto &item) { item.Apply(context); }, item);
  }

  bool ShouldDumpItem(const Item &item, EnumSet<AsTextFlag> flags) noexcept
  {
    if (auto *setState = std::get_if<SetState>(&item))
    {
      if (!flags.Contains(AsTextFlag::IncludePlatformOperations))
      {
        return true;
      }
      // FIXME: for now, only drop the item if the only state-change flags are platform-specific.
      return setState->State().Changes()
             != GraphicsContextState::ChangeFlags {GraphicsContextState::Change::ShouldSubpixelQuantizeFonts};
    }
    return true;
  }

  void DumpItem(TextStream &ts, const Item &item, EnumSet<AsTextFlag> flags) noexcept
  {
    std::visit(
      [&]<typename TItem>(const TItem &item)
      {
        ts << TItem::Name;
        item.Dump(ts, flags);
      },
      item);
  }

  TextStream &operator<<(TextStream &ts, const Item &item) noexcept
  {
    DumpItem(ts, item, {AsTextFlag::IncludePlatformOperations, AsTextFlag::IncludeResourceIdentifiers});
    return ts;
  }
}
