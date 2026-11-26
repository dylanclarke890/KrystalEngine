#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Web.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Web/Accessibility/AXLogger.hpp"
#include "Krystal.Web/Accessibility/AXObject.hpp"
#include "Krystal.Web/Accessibility/AXObjectCache.hpp"
#include "Krystal.Web/Page/Clients/WebClient.hpp"
#include <utility>

namespace krys::web::ax::tests
{
  /// @brief A Krystal.Web client that records the accessibility notifications posted to it.
  class NotificationRecorder final : public WebClient
  {
  public:
    List<Pair<Ref<AXObject>, AXNotification>> Notifications;

    struct TextChange
    {
      Ref<AXObject> Object;
      AXTextChange Change;
      uint64 Offset;
      utf8_string Text;
    };

    List<TextChange> TextChanges;
    List<AXId> DetachedObjects;

    void AttachRootGraphicsLayer(web::Frame &, GraphicsLayer *) noexcept final
    {
    }

    void PostAccessibilityNotification(AXObject &object, AXNotification notification) noexcept final
    {
      Notifications.emplace_back(ShareRef(object), notification);
    }

    void AccessibilityObjectDetached(AXObject &object, AXDetachmentType) noexcept final
    {
      DetachedObjects.push_back(object.ObjectID());
    }

    void PostAccessibilityTextChange(AXObject &object, AXTextChange change, uint64 offset,
                                     utf8_stringview text) noexcept final
    {
      TextChanges.push_back({ShareRef(object), change, offset, utf8_string(text)});
    }
  };

  /// @brief Records an `AXFixture`'s notifications: the Krystal.Web client of its frame, made before the frame.
  struct AXRecorder
  {
    NotificationRecorder *Recorder {nullptr};

    KRYS_NODISCARD Ref<web::Frame> CreateFrame() noexcept
    {
      auto recorder = CreateUnique<NotificationRecorder>();
      Recorder = recorder.get();
      // An editor client keeps the undo stack.
      return web::Frame::Create(krys::move(recorder), CreateUnique<web::tests::TestEditorClient>());
    }
  };

  /// @brief A laid-out page with accessibility enabled (for the fixture's lifetime), whose notifications to
  /// the host are recorded.
  struct AXFixture : AXRecorder, web::tests::PageInFrameFixture
  {
    explicit AXFixture(utf8_string &&markup, SizeI viewSize = {400, 300}) noexcept
        : PageInFrameFixture(CreateFrame(), krys::move(markup), viewSize, CachePolicy::Use)
    {
      AXObjectCache::EnableAccessibility();
      (void)Document->UpdateLayout();
    }

    ~AXFixture() noexcept
    {
      AXObjectCache::DisableAccessibility();
    }

    KRYS_NODISCARD AXObjectCache &Cache() noexcept
    {
      auto *cache = Document->AxObjectCache();
      REQUIRE(cache);
      return *cache;
    }

    /// @brief The object for the element with @p id, created if needed; fails the test if there's none.
    KRYS_NODISCARD AXObject &Object(DOMStringView id) noexcept
    {
      auto *object = Cache().GetOrCreate(ById(id));
      REQUIRE(object);
      return *object;
    }

    /// @brief Runs the tasks the change queued (updating the tree and posting its notifications), then
    /// returns the notifications posted to the host since the last call.
    KRYS_NODISCARD List<Pair<Ref<AXObject>, AXNotification>> TakeNotifications() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
      return std::exchange(Recorder->Notifications, {});
    }

    /// @brief The text changes posted to the host since last asked.
    KRYS_NODISCARD List<NotificationRecorder::TextChange> TakeTextChanges() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
      return std::exchange(Recorder->TextChanges, {});
    }

    /// @brief Whether @p notification was posted for @p object, among @p notifications.
    KRYS_NODISCARD static bool WasPosted(const List<Pair<Ref<AXObject>, AXNotification>> &notifications,
                                         const AXObject &object, AXNotification notification) noexcept
    {
      for (const auto &[posted, kind] : notifications)
      {
        if (posted.get() == &object && kind == notification)
        {
          return true;
        }
      }
      return false;
    }

    /// @brief The tree from the root, as `AXLogger` dumps it.
    KRYS_NODISCARD utf8_string Tree(EnumSet<AXStreamOption> options = {AXStreamOption::Role,
                                                                       AXStreamOption::Name}) noexcept
    {
      return AXLogger::TreeAsText(Cache(), options);
    }
  };
}
