#pragma once

#include "Krystal.Booey/DOM/Events/EventSender.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Time/Timer.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class Document;
  class Element;
  class RenderImageResource;

  class ImageLoader;
  using ImageEventSender = dom::EventSender<ImageLoader>;

  /// @brief Whether the update comes from a relevant mutation, which restarts the image's events even for
  /// the same image.
  /// @see https://html.spec.whatwg.org/#relevant-mutations
  enum class RelevantMutation : bool
  {
    No,
    Yes,
  };

  /// @brief Where a lazy image is: not lazy, waiting to near the viewport, about to load, or loading.
  enum class LazyImageLoadState : uint8
  {
    None,
    Deferred,
    LoadImmediately,
    FullImage,
  };

  /// @brief Loads the image of an element (`<img>`, `<input type=image>`, and later video posters and
  /// `<object>`), keeps its renderer's image up to date, and fires its `load` and `error` events.
  // FIXME: Archives.
  class ImageLoader : public CachedImageClient
  {
    KRYS_NON_COPY_MOVABLE(ImageLoader);

  public:
    /// @brief Settles a `decode()` promise: with nothing once the image is decoded, or with an
    /// `EncodingError`.
    using DecodePromise = Func<void(ExceptionOr<void> &&)>;

  private:
    boo::Element &_element;
    CachedResourceHandle<CachedImage> _image;
    Timer _derefElementTimer;
    RefPtr<boo::Element> _protectedElement;
    DOMString _failedLoadUrl;
    DOMString _pendingUrl;
    bool _hasPendingLoadEvent : 1 {false};
    bool _hasPendingErrorEvent : 1 {false};
    bool _imageComplete : 1 {true};
    bool _elementIsProtected : 1 {false};
    bool _loadManually : 1 {false};
    LazyImageLoadState _lazyImageLoadState {LazyImageLoadState::None};
    List<DecodePromise> _decodingPromises;

  public:
    ~ImageLoader() noexcept override;

    /// @brief While set, a new source makes an image its owner gives the data to (an image document's),
    /// rather than requesting it.
    void SetLoadManually(bool loadManually) noexcept
    {
      _loadManually = loadManually;
    }

    /// @brief Updates the image for the element's source: requests a new image, or fires `error` for a
    /// missing one, unless the source is the one that failed last.
    void UpdateFromElement(RelevantMutation relevantMutation = RelevantMutation::No) noexcept;

    /// @brief The same, trying a source that failed last again, as setting the source does.
    void UpdateFromElementIgnoringPreviousError(
      RelevantMutation relevantMutation = RelevantMutation::No) noexcept;

    /// @brief For a source set to the value it had: a cached image that's loaded fires `load` again; anything
    /// else updates as above.
    void UpdateFromElementIgnoringPreviousErrorToSameValue() noexcept;

    /// @brief Drops the image as the element moves to another document, which loads it again.
    void ElementDidMoveToNewDocument(boo::Document &oldDocument) noexcept;

    /// @brief The element whose image it loads.
    KRYS_NODISCARD boo::Element &GetElement() noexcept
    {
      return _element;
    }

    KRYS_NODISCARD const boo::Element &GetElement() const noexcept
    {
      return _element;
    }

    /// @brief Whether the load is over, or there's nothing to load.
    KRYS_NODISCARD bool ImageComplete() const noexcept
    {
      return _imageComplete;
    }

    /// @brief The image, if there's a source.
    KRYS_NODISCARD CachedImage *Image() const noexcept
    {
      return _image.get();
    }

    /// @brief Calls @p promise, as a microtask, once the image has loaded and decoded, so it can be drawn
    /// without decoding. It's rejected with an `EncodingError` if the document isn't fully active, the
    /// element has no source, or the image fails to load or decode, or its source changes first.
    /// @see https://html.spec.whatwg.org/#dom-img-decode
    void Decode(DecodePromise promise) noexcept;

    /// @brief Drops the image, canceling its pending events.
    void ClearImage() noexcept;

    /// @brief Whether a `load` or `error` event is still to come (not for a lazy load that hasn't started),
    /// which keeps the element alive.
    KRYS_NODISCARD bool HasPendingActivity() const noexcept;

    /// @brief Fires the pending `load` or `error` event, for the event sender.
    void DispatchPendingEvent(ImageEventSender *eventSender, const StringAtom &eventType) noexcept;

    /// @brief Dispatches the `load` and `error` events waiting to be dispatched soon, of every element.
    static void DispatchPendingLoadEvents() noexcept;

    /// @brief The element's document.
    KRYS_NODISCARD boo::Document &GetDocument() noexcept;

    /// @brief Starts the deferred load of a lazily loaded image, once it intersects the viewport.
    void LoadDeferredImage() noexcept;

    /// @brief Whether the image's load is deferred until it intersects the viewport, or about to start.
    KRYS_NODISCARD bool IsDeferred() const noexcept
    {
      return _lazyImageLoadState == LazyImageLoadState::Deferred
             || _lazyImageLoadState == LazyImageLoadState::LoadImmediately;
    }

  protected:
    /// @brief A loader for @p element's image.
    explicit ImageLoader(boo::Element &element) noexcept;

    void NotifyFinished(CachedResource &resource) noexcept override;

  private:
    /// @brief Fires the element's `load` event.
    virtual void DispatchLoadEvent() noexcept = 0;

    /// @brief Keeps the element alive while there's pending activity, and lets it go (in a task) once there's
    /// none.
    void UpdatedHasPendingEvent() noexcept;

    /// @brief Switches to @p newImage (or to none): cancels the old image's pending events and `decode()`
    /// promises, gives the renderer the new one, and becomes its client.
    void DidUpdateCachedImage(RelevantMutation relevantMutation,
                              const CachedResourceHandle<CachedImage> &newImage) noexcept;

    /// @brief Fires the pending `load` or `error` event, if the document renders.
    void DispatchPendingLoadEvent() noexcept;

    void DispatchPendingErrorEvent() noexcept;

    /// @brief The image resource of the element's renderer, if it shows the element's image.
    KRYS_NODISCARD RenderImageResource *GetRenderImageResource() noexcept;

    /// @brief Gives the renderer the image, once it's complete.
    void UpdateRenderer() noexcept;

    /// @brief Drops the image and cancels its events, leaving the element's protection as it is.
    void ClearImageWithoutConsideringPendingLoadEvent() noexcept;

    /// @brief Forgets the source that failed last.
    void ClearFailedLoadUrl() noexcept;

    /// @brief Lets the element go.
    void TimerFired() noexcept;

    /// @brief Marks the load over, and gives the renderer the image.
    void SetImageCompleteAndMaybeUpdateRenderer() noexcept;

    /// @brief Stops waiting for a lazy image to near the viewport.
    void ResetLazyImageLoading(boo::Document &document) noexcept;

    /// @brief Decodes the loaded image for the waiting `decode()` promises, settling them once it's decoded.
    void DecodeImage() noexcept;

    /// @brief Settles the waiting `decode()` promises.
    void ResolveDecodePromises() noexcept;

    void RejectDecodePromises(utf8_stringview message) noexcept;
  };
}
