#pragma once

#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief Shows a document in a view: the host creates a frame, gives it a view (`CreateView`) and a
  /// document (`SetDocument`), and sets the zoom factors and the media environment.
  /// @note FIXME: Only what layout and style need: no page, loader, navigation, frame tree, window,
  /// scripting, editing or printing. The media environment stands in for the page, its chrome client and the
  /// screen.
  class LocalFrame final : public RefCounted<LocalFrame>, public CanMakeWeakPtr<LocalFrame>
  {
    KRYS_NON_COPY_MOVABLE(LocalFrame);

  private:
    RefPtr<dom::Document> _document;
    RefPtr<LocalFrameView> _view;
    css::mq::MediaEnvironment _mediaEnvironment;
    float _pageZoomFactor {1};
    float _textZoomFactor {1};
    bool _documentIsBeingReplaced {false};

    LocalFrame() noexcept;

  public:
    ~LocalFrame() noexcept;

    KRYS_NODISCARD static Ref<LocalFrame> Create() noexcept;

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
