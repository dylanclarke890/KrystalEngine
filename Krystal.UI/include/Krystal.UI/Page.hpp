#pragma once

#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/FontFaceCache.hpp"
#include <filesystem>

namespace krys::ui
{
  /// @brief Hosts a Booey document: parses it, shows it in a frame with a view of the viewport's size, keeps
  /// its style and layout up to date, and paints it into a `Renderer2D` through a display list.
  class Page
  {
    KRYS_NON_COPY_MOVABLE(Page);

  private:
    Ref<boo::LocalFrame> _frame;
    RefPtr<boo::dom::HTMLDocument> _document;
    FontFaceCache _fonts;

  public:
    /// @brief A page with an empty document and a view of @p viewportSize.
    explicit Page(const boo::IntSize &viewportSize) noexcept;
    ~Page() noexcept;

    /// @brief Replaces the document with one parsed from @p markup.
    void LoadHTML(utf8_string &&markup) noexcept;
    /// @brief Replaces the document with one parsed from the file at @p path.
    KRYS_NODISCARD Expected<void> LoadFile(const std::filesystem::path &path) noexcept;

    /// @brief Resizes the viewport; the document is laid out again at its next update.
    void Resize(const boo::IntSize &viewportSize) noexcept;
    KRYS_NODISCARD boo::IntSize ViewportSize() const noexcept;

    /// @brief Brings the document's style and layout up to date.
    void UpdateRendering() noexcept;

    /// @brief Paints the page, up to date, into a display list.
    KRYS_NODISCARD Ref<const boo::display_list::DisplayList> Record() noexcept;

    /// @brief Paints the page into @p renderer, between its `Begin` and `End`.
    void Paint(gfx::Renderer2D &renderer) noexcept;

    /// @brief Replays @p displayList into @p renderer, with this page's fonts, between its `Begin` and `End`.
    void Replay(const boo::display_list::DisplayList &displayList, gfx::Renderer2D &renderer) noexcept;

    KRYS_NODISCARD boo::dom::HTMLDocument &Document() const noexcept
    {
      return *_document;
    }

    KRYS_NODISCARD boo::LocalFrame &Frame() const noexcept
    {
      return *_frame;
    }

    KRYS_NODISCARD FontFaceCache &Fonts() noexcept
    {
      return _fonts;
    }
  };
}
