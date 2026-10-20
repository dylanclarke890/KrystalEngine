#pragma once

#include "Krystal.Booey/DOM/EventSender.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderImageResource;

  namespace dom
  {
    class Document;
    class Element;
  }

  class ImageLoader;
  using ImageEventSender = dom::EventSender<ImageLoader>;

  enum class RelevantMutation : bool
  {
    No,
    Yes
  };

  /// @brief Loads the image of an element (`<img>`, and later `<input type=image>`, video posters and
  /// `<object>`), keeps its renderer's image up to date, and fires its `load` and `error` events.
  // FIXME: Lazy loading (`loading=lazy`), decode() promises, loading manually, image documents, archives, and
  // whether the image is visible in the viewport (for animations, G7d).
  class ImageLoader : public CachedImageClient
  {
    KRYS_NON_COPY_MOVABLE(ImageLoader);

  private:
    dom::Element &_element;
    CachedResourceHandle<CachedImage> _image;
    Timer _derefElementTimer;
    RefPtr<dom::Element> _protectedElement;
    dom::DOMString _failedLoadURL;
    dom::DOMString _pendingURL;
    bool _hasPendingLoadEvent : 1 {false};
    bool _hasPendingErrorEvent : 1 {false};
    bool _imageComplete : 1 {true};
    bool _elementIsProtected : 1 {false};

  public:
    ~ImageLoader() noexcept override;

    /// @brief Called when the element is attached to a document; starts loading if a load hasn't already been
    /// started.
    void UpdateFromElement(RelevantMutation relevantMutation = RelevantMutation::No) noexcept;

    /// @brief Called whenever the `src` attribute is set. Starts a new load unconditionally (matches Firefox
    /// and Opera behavior).
    void UpdateFromElementIgnoringPreviousError(
      RelevantMutation relevantMutation = RelevantMutation::No) noexcept;

    void UpdateFromElementIgnoringPreviousErrorToSameValue() noexcept;

    void ElementDidMoveToNewDocument(dom::Document &oldDocument) noexcept;

    KRYS_NODISCARD dom::Element &GetElement() noexcept
    {
      return _element;
    }
    KRYS_NODISCARD const dom::Element &GetElement() const noexcept
    {
      return _element;
    }

    KRYS_NODISCARD bool ImageComplete() const noexcept
    {
      return _imageComplete;
    }

    KRYS_NODISCARD CachedImage *Image() const noexcept
    {
      return _image.get();
    }

    /// @brief Cancels pending load events, and doesn't dispatch new ones.
    void ClearImage() noexcept;

    KRYS_NODISCARD bool HasPendingActivity() const noexcept;

    void DispatchPendingEvent(ImageEventSender *eventSender, const StringAtom &eventType) noexcept;

    /// @brief Dispatches the `load` and `error` events waiting to be dispatched soon, of every element.
    static void DispatchPendingLoadEvents() noexcept;

    KRYS_NODISCARD dom::Document &GetDocument() noexcept;

  protected:
    explicit ImageLoader(dom::Element &element) noexcept;

    void NotifyFinished(CachedResource &resource) noexcept override;

  private:
    virtual void DispatchLoadEvent() noexcept = 0;

    void UpdatedHasPendingEvent() noexcept;
    void DidUpdateCachedImage(RelevantMutation relevantMutation,
                              CachedResourceHandle<CachedImage> &&newImage) noexcept;

    void DispatchPendingLoadEvent() noexcept;
    void DispatchPendingErrorEvent() noexcept;

    KRYS_NODISCARD RenderImageResource *GetRenderImageResource() noexcept;
    void UpdateRenderer() noexcept;

    void ClearImageWithoutConsideringPendingLoadEvent() noexcept;
    void ClearFailedLoadURL() noexcept;

    void TimerFired() noexcept;

    void SetImageCompleteAndMaybeUpdateRenderer() noexcept;
  };
}
