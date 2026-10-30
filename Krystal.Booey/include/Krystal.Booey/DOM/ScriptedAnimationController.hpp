#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Page;
}

namespace krys::boo::dom
{
  class Document;

  /// @brief Called with the frame's timestamp: milliseconds since the window's time origin.
  /// @see https://html.spec.whatwg.org/#framerequestcallback
  using FrameRequestCallback = Func<void(double)>;

  /// @brief A document's animation frame callbacks (`requestAnimationFrame()`), run by the rendering update.
  /// @see https://html.spec.whatwg.org/#animation-frames
  class ScriptedAnimationController
  {
    KRYS_NON_COPY_MOVABLE(ScriptedAnimationController);

  public:
    using CallbackId = int32;

  private:
    struct CallbackData
    {
      CallbackId Id {0};
      FrameRequestCallback Callback;
      bool FiredOrCancelled {false};
    };

    WeakPtr<Document> _document;
    List<CallbackData> _callbackDataList;
    CallbackId _nextCallbackId {0};
    uint32 _suspendCount {0};
    double _lastAnimationFrameTimestamp {0};

  public:
    explicit ScriptedAnimationController(Document &document) noexcept;
    ~ScriptedAnimationController() noexcept;

    /// @return The callback's handle, positive.
    CallbackId RegisterCallback(FrameRequestCallback &&callback) noexcept;
    void CancelCallback(CallbackId callbackId) noexcept;

    /// @brief Runs the callbacks registered before this frame, with @p timestamp (milliseconds since the
    /// window's time origin).
    /// @see https://html.spec.whatwg.org/#run-the-animation-frame-callbacks
    void ServiceRequestAnimationFrameCallbacks(double timestamp) noexcept;

    /// @brief Holds the callbacks back (while the document is suspended), until as many resumes.
    void Suspend() noexcept;
    void Resume() noexcept;

    KRYS_NODISCARD bool HasCallbacks() const noexcept
    {
      return !_callbackDataList.empty();
    }

  private:
    KRYS_NODISCARD Page *GetPage() const noexcept;

    /// @brief Asks the page for a rendering update that runs the callbacks.
    void ScheduleAnimation() noexcept;
  };
}
