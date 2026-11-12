#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXLogger.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include <utility>

namespace krys::boo::ax::tests
{
  /// @brief A chrome client that records the accessibility notifications posted to it.
  class NotificationRecorder final : public ChromeClient
  {
  public:
    List<Pair<Ref<AccessibilityObject>, AXNotification>> Notifications;
    struct TextChange
    {
      Ref<AccessibilityObject> Object;
      AXTextChange Change;
      uint64 Offset;
      utf8_string Text;
    };
    List<TextChange> TextChanges;

    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
    {
    }

    void PostAccessibilityNotification(AccessibilityObject &object,
                                       AXNotification notification) noexcept final
    {
      Notifications.emplace_back(ShareRef(object), notification);
    }

    void PostAccessibilityTextChange(AccessibilityObject &object, AXTextChange change, uint64 offset,
                                     utf8_stringview text) noexcept final
    {
      TextChanges.push_back({ShareRef(object), change, offset, utf8_string(text)});
    }
  };

  /// @brief The page an `AccessibilityFixture` is in, made before the fixture's frame.
  struct AccessibilityPage
  {
    NotificationRecorder *Recorder {nullptr};
    Ref<Page> TestPage {CreatePage()};

    KRYS_NODISCARD Ref<Page> CreatePage() noexcept
    {
      auto recorder = CreateUnique<NotificationRecorder>();
      Recorder = recorder.get();
      // An editor client keeps the undo stack.
      return Page::Create(krys::move(recorder), CreateUnique<boo::tests::TestEditorClient>());
    }
  };

  /// @brief A laid-out page with accessibility enabled (for the fixture's lifetime), whose notifications to
  /// the host are recorded.
  struct AccessibilityFixture : AccessibilityPage, boo::tests::PageInFrameFixture
  {
    explicit AccessibilityFixture(utf8_string &&markup, IntSize viewSize = {400, 300}) noexcept
        : PageInFrameFixture(*TestPage, krys::move(markup), viewSize)
    {
      AXObjectCache::EnableAccessibility();
      (void)Document->UpdateLayout();
    }

    ~AccessibilityFixture() noexcept
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
    KRYS_NODISCARD AccessibilityObject &Object(dom::DOMStringView id) noexcept
    {
      auto *object = Cache().GetOrCreate(ById(id));
      REQUIRE(object);
      return *object;
    }

    /// @brief Runs the tasks the change queued (updating the tree and posting its notifications), then
    /// returns the notifications posted to the host since the last call.
    KRYS_NODISCARD List<Pair<Ref<AccessibilityObject>, AXNotification>> TakeNotifications() noexcept
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
    KRYS_NODISCARD static bool
      WasPosted(const List<Pair<Ref<AccessibilityObject>, AXNotification>> &notifications,
                const AccessibilityObject &object, AXNotification notification) noexcept
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
