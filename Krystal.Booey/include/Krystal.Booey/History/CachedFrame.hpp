#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class DocumentLoader;
  class LocalFrame;
  class LocalFrameView;

  /// @brief A frame's document in the back/forward cache: the document (suspended, detached from the frame),
  /// its loader and the view it was shown in, until the frame shows them again or the cache drops them.
  // FIXME: Child frames, once there are frames other than the main frame (not planned).
  class CachedFrame final
  {
    KRYS_NON_COPY_MOVABLE(CachedFrame);

  private:
    RefPtr<dom::Document> _document;
    RefPtr<DocumentLoader> _documentLoader;
    RefPtr<LocalFrameView> _view;
    url::URL _url;
    bool _isMainFrame {true};

  public:
    /// @brief Caches @p frame's document: suspends it and detaches it from the frame.
    explicit CachedFrame(LocalFrame &frame) noexcept;
    ~CachedFrame() noexcept;

    /// @brief Shows the document in its frame again.
    void Open() noexcept;

    /// @brief Resumes the document, as its frame shows it again.
    void Restore() noexcept;

    /// @brief Forgets the document: it's the frame's again, or it was destroyed.
    void Clear() noexcept;

    /// @brief Destroys the document, which the cache drops.
    void Destroy() noexcept;

    KRYS_NODISCARD dom::Document *Document() const noexcept;
    KRYS_NODISCARD DocumentLoader *GetDocumentLoader() const noexcept;
    KRYS_NODISCARD LocalFrameView *View() const noexcept;

    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _url;
    }

    KRYS_NODISCARD bool IsMainFrame() const noexcept
    {
      return _isMainFrame;
    }
  };
}
