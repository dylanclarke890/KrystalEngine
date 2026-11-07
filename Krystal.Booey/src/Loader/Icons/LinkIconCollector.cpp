#include "Krystal.Booey/Loader/Icons/LinkIconCollector.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringToIntegerConversion.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    /// @brief The width the first of @p sizes gives: "60x70" is 60.
    KRYS_NODISCARD Maybe<uint32> FirstIconSize(dom::DOMStringView sizes) noexcept
    {
      auto start = std::ranges::find_if_not(sizes, [](char8 c) { return text::IsASCIIWhitespace(c); });
      if (start == sizes.end())
      {
        return null;
      }
      auto first = dom::DOMStringView(start, sizes.end());
      return text::ParseIntegerAllowingTrailingJunk<uint32>(first);
    }
  }

  List<LinkIcon> CollectLinkIcons(const dom::Document &document) noexcept
  {
    auto head = document.Head();
    if (!head)
    {
      return {};
    }

    List<LinkIcon> icons;
    for (const auto *child = head->FirstChild(); child; child = child->NextSibling())
    {
      const auto *link = DynamicDowncast<html::HTMLLinkElement>(*child);
      if (!link || !link->IsIcon())
      {
        continue;
      }

      auto href = link->GetAttribute(u8"href");
      if (!href || infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(*href).empty())
      {
        continue;
      }
      auto url = document.CompleteURL(*href);
      if (!url.ProtocolIsInHTTPFamily() && !url.ProtocolIsData())
      {
        continue;
      }

      icons.push_back({krys::move(url), dom::DOMString(link->GetAttribute(u8"type").value_or(u8"")),
                       FirstIconSize(link->GetAttribute(u8"sizes").value_or(u8""))});
    }

    // The largest first; icons without a size are the smallest.
    std::ranges::stable_sort(icons, std::ranges::greater {},
                             [](const LinkIcon &icon) { return icon.Size.value_or(0); });
    return icons;
  }
}
