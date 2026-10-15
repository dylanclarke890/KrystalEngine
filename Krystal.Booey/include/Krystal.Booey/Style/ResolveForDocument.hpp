#pragma once

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  /// @brief The style of the document itself: the initial containing block's, which the document element
  /// inherits from.
  RenderStyle ResolveForDocument(const dom::Document &document) noexcept;
}
