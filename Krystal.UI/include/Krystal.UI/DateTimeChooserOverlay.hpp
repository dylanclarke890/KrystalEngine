#pragma once

#include "Krystal.Booey/Platform/DateTimeChooser.hpp"
#include "Krystal.Booey/Platform/DateTimeChooserParameters.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.UI/Overlay.hpp"

namespace krys::boo
{
  class DateTimeChooserClient;
}

namespace krys::ui
{
  /// @brief A date input's picker, shown by the view of its page as an overlay: a month's calendar to choose
  /// a day from (or a week, for week inputs), or a year's months for month inputs, below the input (or above
  /// it, if there's more room there) within the viewport. Choosing, Escape and pressing outside the picker
  /// hide it, and go back through its client.
  class DateTimeChooserOverlay final : public boo::DateTimeChooser, public Overlay
  {
  public:
    enum class Mode : uint8
    {
      /// @brief A month's days: dates and local date times.
      Days,
      /// @brief A month's days, in weeks: weeks.
      Weeks,
      /// @brief A year's months: months.
      Months,
    };

  private:
    View *_owner {nullptr};
    boo::DateTimeChooserClient *_client {nullptr};
    UniquePtr<View> _view;
    boo::DateTimeChooserParameters _parameters;
    Mode _mode {Mode::Days};
    boo::IntRect _rect;
    /// @brief The month shown (months since January 1970) in the day modes, or the year shown in the month
    /// mode.
    int32 _page {0};
    /// @brief What's highlighted: a day (days since 1970-01-01) in the day modes, or a month (months since
    /// January 1970) in the month mode.
    int32 _highlighted {0};
    /// @brief What the input's value chooses, in the same units as `_highlighted`, if anything.
    Maybe<int32> _selected;
    /// @brief The element showing each choosable day or month, by its value in `_highlighted`'s units.
    HashMap<int32, RefPtr<boo::dom::Element>> _cells;
    bool _visible {false};

  public:
    DateTimeChooserOverlay(View &owner, boo::DateTimeChooserClient &client) noexcept;
    ~DateTimeChooserOverlay() noexcept override;

#pragma region DateTimeChooser

    void EndChooser() noexcept override;
    void ShowChooser(const boo::DateTimeChooserParameters &parameters) noexcept override;

#pragma endregion

#pragma region Overlay

    void AddRef() const noexcept final
    {
      DateTimeChooser::AddRef();
    }

    void SubRef() const noexcept final
    {
      DateTimeChooser::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return DateTimeChooser::GetRefCount();
    }

    KRYS_NODISCARD View &ContentView() const noexcept final
    {
      return *_view;
    }

    KRYS_NODISCARD const boo::IntRect &Rect() const noexcept final
    {
      return _rect;
    }

    void Hide() noexcept final;
    /// @brief Highlights the day or month under @p point.
    void MouseMoved(const boo::IntPoint &point) noexcept final;
    /// @brief Chooses the day or month under @p point, or turns the page.
    void MouseReleased(const boo::IntPoint &point) noexcept final;
    bool MouseWheel(const boo::IntPoint &point, float notchesX, float notchesY) noexcept final;
    /// @brief The arrow keys move the highlight, Page Up and Page Down turn the page, Home and End go to its
    /// start and end; Enter and Space choose; Escape and Tab hide the picker.
    bool KeyPressed(pal::Key key, pal::KeyModifiers modifiers) noexcept final;

#pragma endregion

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _visible;
    }

    KRYS_NODISCARD Mode GetMode() const noexcept
    {
      return _mode;
    }

    /// @brief The value the highlighted day or month would choose.
    KRYS_NODISCARD utf8_string HighlightedValue() const noexcept;

  private:
    /// @brief Fills the picker's page with the page of days or months shown, and places it.
    void Build() noexcept;
    void Highlight(int32 value) noexcept;
    void SetHighlightedClass(int32 value, bool on) noexcept;
    void Choose(int32 value) noexcept;
    /// @brief Turns the page by @p pages, keeping the highlight on it.
    void TurnPage(int32 pages) noexcept;
    KRYS_NODISCARD bool IsEnabled(int32 value) const noexcept;
    /// @brief The value the day or month @p value chooses.
    KRYS_NODISCARD utf8_string ValueFor(int32 value) const noexcept;
    /// @brief The page (month or year) @p value is on.
    KRYS_NODISCARD int32 PageOf(int32 value) const noexcept;
  };
}
