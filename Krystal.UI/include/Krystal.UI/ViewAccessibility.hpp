#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.PAL/Accessibility.hpp"

namespace krys::pal
{
  class IWindow;
}

namespace krys::boo::ax
{
  class AccessibilityObject;
  class AXObjectCache;
  enum class AXNotification : uint8;
  enum class AXTextChange : uint8;
}

namespace krys::ui
{
  class Overlay;
  class View;

  /// @brief Exposes a view's page to assistive technology through the window showing it: the page's
  /// accessibility tree (Booey's, turned on the first time it's asked for) as Krystal.PAL's nodes, with the
  /// page's notifications passed on as the window's events.
  /// @note Nodes are the page's accessibility objects, by ID; a node's bounds are in the window's client
  /// area, where the view's top left is at the origin the host gives. While an overlay (a menu list's
  /// popup, a picker) is shown, its page's tree is the root's last child, its nodes' IDs marked as the
  /// overlay's; a modal one has the focus and is hit first.
  class ViewAccessibility final : public pal::IAccessibilityTree
  {
    KRYS_NON_COPY_MOVABLE(ViewAccessibility);

    /// @brief Another page's cache, by its document (a cache offers no weak pointers): still the one if the
    /// document still has it.
    struct ForeignCache
    {
      WeakPtr<boo::dom::Document> Document;
      const boo::ax::AXObjectCache *Cache {nullptr};

      KRYS_NODISCARD boo::ax::AXObjectCache *Get() const noexcept;
    };

  private:
    View &_view;
    pal::IWindow *_window {nullptr};
    boo::IntPoint _origin;
    /// @brief The other pages' caches whose objects are nodes (a shown overlay's page, SVG images'
    /// documents), by their tag in node IDs less one. A slot is cleared, never reused, when its nodes go, so
    /// a stale ID can't be taken for a node of another page.
    mutable List<ForeignCache> _foreignCaches;

  public:
    explicit ViewAccessibility(View &view) noexcept;
    ~ViewAccessibility() noexcept override;

    /// @brief Exposes the page through @p window, whose client area has the view's top left at @p origin.
    void Attach(pal::IWindow &window, const boo::IntPoint &origin) noexcept;
    /// @brief Stops exposing the page.
    void Detach() noexcept;
    /// @brief Moves the view's top left in the window's client area to @p origin.
    void SetOrigin(const boo::IntPoint &origin) noexcept
    {
      _origin = origin;
    }
    KRYS_NODISCARD pal::IWindow *Window() const noexcept
    {
      return _window;
    }

    // The page's side, from the view's chrome client.

    void NotificationPosted(boo::ax::AccessibilityObject &object,
                            boo::ax::AXNotification notification) noexcept;
    void TextChanged(boo::ax::AccessibilityObject &object, boo::ax::AXTextChange change, uint64 offset,
                     utf8_stringview text) noexcept;
    void ObjectDetached(boo::ax::AccessibilityObject &object) noexcept;
    /// @brief The view showed @p overlay: its page's tree joins the view's, and its notifications come here.
    void OverlayShown(Overlay &overlay) noexcept;
    /// @brief The view hid @p overlay.
    void OverlayHidden(Overlay &overlay) noexcept;

    // pal::IAccessibilityTree.

    KRYS_NODISCARD pal::AccessibilityNodeID Root() noexcept override;
    KRYS_NODISCARD Maybe<pal::AccessibilityNode> Node(pal::AccessibilityNodeID node) noexcept override;
    KRYS_NODISCARD pal::AccessibilityNodeID Parent(pal::AccessibilityNodeID node) noexcept override;
    KRYS_NODISCARD List<pal::AccessibilityNodeID> Children(pal::AccessibilityNodeID node) noexcept override;
    KRYS_NODISCARD pal::AccessibilityNodeID Focus() noexcept override;
    KRYS_NODISCARD pal::AccessibilityNodeID HitTest(float x, float y) noexcept override;
    KRYS_NODISCARD List<pal::AccessibilityNodeID>
      SelectedChildren(pal::AccessibilityNodeID node) noexcept override;
    bool PerformAction(pal::AccessibilityNodeID node, pal::AccessibilityAction action) noexcept override;
    bool SetValue(pal::AccessibilityNodeID node, utf8_stringview value) noexcept override;
    bool SetRangeValue(pal::AccessibilityNodeID node, double value) noexcept override;
    KRYS_NODISCARD utf8_string Text(pal::AccessibilityNodeID node) noexcept override;
    KRYS_NODISCARD Maybe<pal::AccessibilityTextRange>
      TextSelection(pal::AccessibilityNodeID node) noexcept override;
    bool SetTextSelection(pal::AccessibilityNodeID node, pal::AccessibilityTextRange range) noexcept override;
    KRYS_NODISCARD List<pal::AccessibilityRect>
      TextBounds(pal::AccessibilityNodeID node, pal::AccessibilityTextRange range) noexcept override;
    KRYS_NODISCARD List<uint64> TextLineStarts(pal::AccessibilityNodeID node) noexcept override;
    KRYS_NODISCARD Maybe<pal::AccessibilityTextRun> TextRunAt(pal::AccessibilityNodeID node,
                                                              uint64 offset) noexcept override;

  private:
    /// @brief The page's accessibility tree, turning accessibility on if it's off.
    KRYS_NODISCARD boo::ax::AXObjectCache *Cache() noexcept;
    /// @brief The shown overlay's page's view, if there's an overlay.
    KRYS_NODISCARD View *OverlayView() const noexcept;
    /// @brief The shown overlay's page's accessibility tree, if there's an overlay.
    KRYS_NODISCARD boo::ax::AXObjectCache *OverlayCache() const noexcept;
    /// @brief Forgets @p cache's nodes: its objects' IDs stop resolving, and later ones are new.
    void ForgetForeignCache(const boo::ax::AXObjectCache &cache) noexcept;
    /// @brief @p object's node ID (tagged with its cache for an object of another page); 0 for none.
    KRYS_NODISCARD pal::AccessibilityNodeID ID(const boo::ax::AccessibilityObject *object) const noexcept;
    /// @brief The unignored object for @p node; null if it's gone.
    KRYS_NODISCARD boo::ax::AccessibilityObject *Object(pal::AccessibilityNodeID node) noexcept;
    /// @brief The view @p object's page is in, and its top left in the window's client area.
    KRYS_NODISCARD Pair<View *, boo::IntPoint>
      ViewAndOrigin(const boo::ax::AccessibilityObject &object) const noexcept;
    /// @brief @p rect (in the page's coordinates for @p object) in the window's client area.
    KRYS_NODISCARD pal::AccessibilityRect ToWindow(const boo::ax::AccessibilityObject &object, float x,
                                                   float y, float width, float height) const noexcept;
    /// @brief The deepest unignored object at @p viewPoint (in @p view's coordinates) in @p cache's tree.
    KRYS_NODISCARD boo::ax::AccessibilityObject *HitTestIn(boo::ax::AXObjectCache &cache, View &view,
                                                           const boo::IntPoint &viewPoint) noexcept;
  };
}
