#pragma once

namespace krys::boo
{
  class RenderStyle;

  namespace dom
  {
    class Document;
    class Element;
  }
}

namespace krys::boo::style
{
  /// @brief Starts loading the images @p style uses that haven't loaded (backgrounds, generated content,
  /// cursors, list markers, border images, masks and `shape-outside`), and its first line style's.
  void LoadPendingResources(RenderStyle &, dom::Document &, const dom::Element *) noexcept;
}
