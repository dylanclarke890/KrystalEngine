#pragma once

#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.PAL/Input/KeyModifiers.hpp"
#include "Krystal.PAL/Input/Keys.hpp"

namespace krys::boo
{
  class Element;
}

namespace krys::ui
{
  class View;

  /// @brief Something a view shows above its page for one of its controls (a menu list's popup, a date or
  /// colour picker): an internal view of its own, placed in the owner's viewport. While it's shown, it takes
  /// the keys and the mouse over it; a press outside it hides it.
  class Overlay : public AbstractRefCounted
  {
  public:
    /// @brief The view showing the overlay's page.
    KRYS_NODISCARD virtual View &ContentView() const noexcept = 0;

    /// @brief Where the overlay is, in its owner's viewport.
    KRYS_NODISCARD virtual const RectI &Rect() const noexcept = 0;

    /// @brief Hides the overlay, telling its control.
    virtual void Hide() noexcept = 0;

    /// @brief The mouse moved over the overlay, to @p point (in the overlay's coordinates).
    virtual void MouseMoved(const PointI &point) noexcept = 0;

    /// @brief A button was released over the overlay, at @p point (in the overlay's coordinates).
    virtual void MouseReleased(const PointI &point) noexcept = 0;

    /// @brief The wheel turned over the overlay, at @p point (in the overlay's coordinates).
    virtual bool MouseWheel(const PointI &point, float notchesX, float notchesY) noexcept = 0;

    /// @brief A key was pressed while the overlay was shown.
    virtual bool KeyPressed(pal::Key key, pal::KeyModifiers modifiers) noexcept = 0;

    /// @brief Whether the overlay takes the keys and text input, and a press outside it only hides it. A
    /// suggestion list isn't modal: its field keeps them, and the press goes on to the page.
    KRYS_NODISCARD virtual bool IsModal() const noexcept
    {
      return true;
    }

  protected:
    /// @brief Shows @p overlay over @p owner's page, hiding the one shown before.
    static void ShowIn(View &owner, Overlay &overlay) noexcept;

    /// @brief Replaces @p view's document with one parsed from @p markup at once: an overlay's content.
    static void ReplaceDocument(View &view, utf8_string &&markup) noexcept;

    /// @brief Tells @p owner that @p overlay is hidden.
    static void HiddenFrom(View &owner, Overlay &overlay) noexcept;

    /// @brief Where an overlay of @p size goes for a control at @p controlRect, in a viewport of
    /// @p viewportSize: below the control if it fits, else wherever there's more room, shrunk to fit there;
    /// and at the control's left edge (its right edge if @p rightToLeft), kept in the viewport.
    KRYS_NODISCARD static RectI Place(const RectI &controlRect, const SizeI &size, const SizeI &viewportSize,
                                      bool rightToLeft = false) noexcept;

    /// @brief The left of an overlay @p width wide for a control at @p controlRect: lined up with the
    /// control's left edge, or its right edge if @p rightToLeft, and kept in a viewport @p viewportWidth
    /// wide.
    KRYS_NODISCARD static int32 AlignedLeft(const RectI &controlRect, int32 width, int32 viewportWidth,
                                            bool rightToLeft) noexcept;

    /// @brief Sizes @p view, an overlay's, to @p size CSS pixels, at @p owner's device scale factor.
    static void SizeContentView(View &view, const View &owner, const SizeI &size) noexcept;

    /// @brief The topmost element at @p point (in CSS pixels) in @p view, or null.
    KRYS_NODISCARD static boo::Element *HitTest(View &view, const PointI &point) noexcept;

    /// @brief Turns the wheel at @p point (in CSS pixels) in @p view, as `View::MouseWheel` does.
    static bool WheelView(View &view, const PointI &point, float notchesX, float notchesY) noexcept;

    /// @brief The element at @p point in @p view, or its nearest ancestor, with the attribute @p name.
    KRYS_NODISCARD static boo::Element *ElementWithAttributeAt(View &view, const PointI &point,
                                                               utf8_stringview name) noexcept;

    /// @brief Adds the class @p className to @p element's classes, or removes it.
    static void SetClass(boo::Element &element, utf8_stringview className, bool on) noexcept;

    /// @brief Appends @p text to @p html, escaped.
    static void AppendEscaped(utf8_string &html, utf8_stringview text) noexcept;
  };
}
