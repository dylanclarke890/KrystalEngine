#pragma once

#include "Krystal.Booey/Loader/Icons/LinkIcon.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Document;
  }

  /// @brief The icons @p document's head names with `<link rel=icon>`, with `http(s):` and `data:` URLs:
  /// the largest first, then in tree order.
  KRYS_NODISCARD List<LinkIcon> CollectLinkIcons(const dom::Document &document) noexcept;
}
