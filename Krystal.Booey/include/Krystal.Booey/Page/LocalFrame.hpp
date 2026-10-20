#pragma once

#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Loader/Cache/CachePolicy.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Page;

  /// @brief Shows a document in a view: the host creates a frame, gives it a view (`CreateView`) and a
  /// document (`SetDocument`), and sets the zoom factors and the media environment.
  /// @note FIXME: Only what layout and style need: no loader, navigation, frame tree, window, scripting,
  /// editing or printing. The media environment stands in for the page's chrome client and the screen.
  class LocalFrame final : public RefCounted<LocalFrame>, public CanMakeWeakPtr<LocalFrame>
  {
    KRYS_NON_COPY_MOVABLE(LocalFrame);

  private:
    WeakPtr<Page> _page;
    RefPtr<dom::Document> _document;
    RefPtr<LocalFrameView> _view;
    UniquePtr<EventHandler> _eventHandler;
    css::mq::MediaEnvironment _mediaEnvironment;
    float _pageZoomFactor {1};
    float _textZoomFactor {1};
    CachePolicy _subresourceCachePolicy {CachePolicy::Verify};
    bool _documentIsBeingReplaced {false};

    explicit LocalFrame(Page *page) noexcept;

  public:
    ~LocalFrame() noexcept;

    /// @brief A frame, the main frame of @p page if given.
    KRYS_NODISCARD static Ref<LocalFrame> Create(Page *page = nullptr) noexcept;

    /// @brief How the document's subresources use the memory cache: the host sets `Revalidate` when it
    /// reloads a page, so edited files load again.
    // FIXME: The frame loader's load types (reloads, back and forward), and going back to `Verify` once the
    // document has loaded, once there's a frame loader.
    KRYS_NODISCARD CachePolicy SubresourceCachePolicy() const noexcept
    {
      return _subresourceCachePolicy;
    }
    void SetSubresourceCachePolicy(CachePolicy policy) noexcept
    {
      _subresourceCachePolicy = policy;
    }

    /// @brief The page the frame is in, if any.
    KRYS_NODISCARD Page *GetPage() const noexcept
    {
      return _page.get();
    }

    /// @brief The document the frame shows, if any.
    KRYS_NODISCARD dom::Document *Document() const noexcept
    {
      return _document.get();
    }

    /// @brief The view the frame's document is laid out in, if any.
    KRYS_NODISCARD LocalFrameView *View() const noexcept
    {
      return _view.get();
    }

    /// @brief Replaces the frame's view. The document's render tree is destroyed with the old view and
    /// created with the new one.
    /// @brief Turns the host's mouse input into DOM events for this frame.
    KRYS_NODISCARD EventHandler &GetEventHandler() const noexcept
    {
      return *_eventHandler;
    }

    void SetView(RefPtr<LocalFrameView> &&view) noexcept;

    /// @brief Gives the frame a new view of @p viewportSize, laid out at @p fixedLayoutSize if
    /// @p useFixedLayout.
    void CreateView(const IntSize &viewportSize, const IntSize &fixedLayoutSize = {},
                    bool useFixedLayout = false) noexcept;

    /// @brief Shows @p newDocument in the frame, in place of its current document. @p newDocument must not be
    /// in a frame already.
    void SetDocument(RefPtr<dom::Document> &&newDocument) noexcept;

    /// @brief The zoom applied to the whole document (the root style's `zoom`).
    KRYS_NODISCARD float PageZoomFactor() const noexcept
    {
      return _pageZoomFactor;
    }

    /// @brief The zoom applied to font sizes, on top of the page zoom.
    KRYS_NODISCARD float TextZoomFactor() const noexcept
    {
      return _textZoomFactor;
    }

    void SetPageZoomFactor(float factor) noexcept;
    void SetTextZoomFactor(float factor) noexcept;
    void SetPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor) noexcept;

    /// @brief The device and user state the frame's document is presented with.
    KRYS_NODISCARD const css::mq::MediaEnvironment &GetMediaEnvironment() const noexcept
    {
      return _mediaEnvironment;
    }

    /// @brief Changes the media environment, and re-evaluates the document's style sheets if it changed. The
    /// host then reports the changes to `MediaQueryList`s (`Document::EvaluateMediaQueriesAndReportChanges`).
    void SetMediaEnvironment(const css::mq::MediaEnvironment &environment) noexcept;
  };
}
