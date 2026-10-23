#include "Krystal.UI/DateTimeChooserOverlay.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "Krystal.Booey/Platform/DateTimeChooserClient.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>

namespace krys::ui
{
  namespace
  {
    constexpr double MsPerDay = 86'400'000.0;

    KRYS_NODISCARD int32 FloorDiv(int32 a, int32 b) noexcept
    {
      return a / b - ((a % b != 0) && ((a < 0) != (b < 0)) ? 1 : 0);
    }

    KRYS_NODISCARD int32 FloorMod(int32 a, int32 b) noexcept
    {
      return a - FloorDiv(a, b) * b;
    }

    /// @brief Days since 1970-01-01 of a date in the proleptic Gregorian calendar (@p month from 1).
    KRYS_NODISCARD int32 DaysFromCivil(int32 year, int32 month, int32 day) noexcept
    {
      year -= month <= 2 ? 1 : 0;
      auto era = FloorDiv(year, 400);
      auto yearOfEra = year - era * 400;
      auto dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
      auto dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
      return era * 146'097 + dayOfEra - 719'468;
    }

    /// @brief The first day (days since 1970-01-01) of the month @p month (months since January 1970).
    KRYS_NODISCARD int32 FirstDayOfMonth(int32 month) noexcept
    {
      return DaysFromCivil(1'970 + FloorDiv(month, 12), FloorMod(month, 12) + 1, 1);
    }

    /// @brief The day of the week of @p day (days since 1970-01-01): 0 for Sunday.
    KRYS_NODISCARD int32 WeekDay(int32 day) noexcept
    {
      return FloorMod(day + 4, 7);
    }

    KRYS_NODISCARD Maybe<boo::DateComponents> DayToDate(int32 day) noexcept
    {
      return boo::DateComponents::FromMillisecondsSinceEpochForDate(day * MsPerDay);
    }

    KRYS_NODISCARD int32 DayOf(double milliseconds) noexcept
    {
      return static_cast<int32>(std::floor(milliseconds / MsPerDay));
    }

    KRYS_NODISCARD utf8_string Number(int32 value) noexcept
    {
      auto text = std::format("{}", value);
      return utf8_string(text.begin(), text.end());
    }

    KRYS_NODISCARD Maybe<int32> ParseInteger(utf8_stringview text) noexcept
    {
      int32 value = 0;
      auto *begin = reinterpret_cast<const char *>(text.data());
      auto [end, error] = std::from_chars(begin, begin + text.size(), value);
      if (error != std::errc() || end != begin + text.size())
      {
        return null;
      }
      return value;
    }

    // FIXME: The locale's names of the days of the week, and its first day of the week.
    constexpr std::array<utf8_stringview, 7> WeekDayLabels {u8"Su", u8"Mo", u8"Tu", u8"We",
                                                            u8"Th", u8"Fr", u8"Sa"};
  }

  DateTimeChooserOverlay::DateTimeChooserOverlay(View &owner, boo::DateTimeChooserClient &client) noexcept
      : _owner(&owner), _client(&client)
  {
  }

  DateTimeChooserOverlay::~DateTimeChooserOverlay() noexcept = default;

  void DateTimeChooserOverlay::ShowChooser(const boo::DateTimeChooserParameters &parameters) noexcept
  {
    if (!_client || !_owner)
    {
      return;
    }

    _parameters = parameters;
    if (_parameters.Type == u8"week")
    {
      _mode = Mode::Weeks;
    }
    else if (_parameters.Type == u8"month")
    {
      _mode = Mode::Months;
    }
    else
    {
      _mode = Mode::Days;
    }

    // What the value chooses, and today.
    _selected = null;
    auto &value = _parameters.CurrentValue;
    switch (_mode)
    {
      case Mode::Days:
        if (auto date = boo::DateComponents::FromParsingDate(value))
        {
          _selected = DayOf(date->MillisecondsSinceEpoch());
        }
        else if (auto dateTime = boo::DateComponents::FromParsingDateTimeLocal(value))
        {
          _selected = DayOf(dateTime->MillisecondsSinceEpoch());
        }
        break;
      case Mode::Weeks:
        if (auto week = boo::DateComponents::FromParsingWeek(value))
        {
          _selected = DayOf(week->MillisecondsSinceEpoch());
        }
        break;
      case Mode::Months:
        if (auto month = boo::DateComponents::FromParsingMonth(value))
        {
          _selected = static_cast<int32>(month->MonthsSinceEpoch());
        }
        break;
    }

    auto today = DayOf(boo::LocalTimeNow());
    if (_mode == Mode::Months)
    {
      auto date = DayToDate(today);
      today = date ? (date->FullYear() - 1'970) * 12 + date->Month() : 0;
    }
    _highlighted = _selected.value_or(today);
    _page = PageOf(_highlighted);
    Build();

    if (!_visible)
    {
      _visible = true;
      ShowIn(*_owner, *this);
    }
  }

  void DateTimeChooserOverlay::EndChooser() noexcept
  {
    // The owner and the client let go of the picker as they're told.
    Ref<DateTimeChooserOverlay> protectedThis = ShareRef(*this);
    if (std::exchange(_visible, false) && _owner)
    {
      HiddenFrom(*_owner, *this);
    }
    if (auto *client = std::exchange(_client, nullptr))
    {
      client->DidEndChooser();
    }
  }

  void DateTimeChooserOverlay::Hide() noexcept
  {
    EndChooser();
  }

  int32 DateTimeChooserOverlay::PageOf(int32 value) const noexcept
  {
    if (_mode == Mode::Months)
    {
      return FloorDiv(value, 12);
    }
    auto date = DayToDate(value);
    return date ? (date->FullYear() - 1'970) * 12 + date->Month() : 0;
  }

  bool DateTimeChooserOverlay::IsEnabled(int32 value) const noexcept
  {
    auto minimum = _parameters.Minimum;
    auto maximum = _parameters.Maximum;
    switch (_mode)
    {
      case Mode::Days:
      {
        // A local date time's range can start or end within a day.
        auto start = value * MsPerDay;
        auto end = _parameters.Type == u8"datetime-local" ? start + MsPerDay - 1 : start;
        return end >= minimum && start <= maximum;
      }
      case Mode::Weeks:
      {
        auto monday = value - FloorMod(WeekDay(value) + 6, 7);
        auto start = monday * MsPerDay;
        return start >= minimum && start <= maximum;
      }
      case Mode::Months: return value >= minimum && value <= maximum;
    }
    return false;
  }

  utf8_string DateTimeChooserOverlay::ValueFor(int32 value) const noexcept
  {
    switch (_mode)
    {
      case Mode::Days:
      {
        auto date = DayToDate(value);
        if (!date)
        {
          return {};
        }
        auto string = date->ToString();
        if (_parameters.Type == u8"datetime-local")
        {
          // The day changes; the time stays.
          auto &current = _parameters.CurrentValue;
          auto separator = current.find(u8'T');
          string += separator == utf8_string::npos ? utf8_string(u8"T00:00") : current.substr(separator);
        }
        return string;
      }
      case Mode::Weeks:
      {
        auto week = boo::DateComponents::FromMillisecondsSinceEpochForWeek(value * MsPerDay);
        return week ? week->ToString() : utf8_string();
      }
      case Mode::Months:
      {
        auto month = boo::DateComponents::FromMonthsSinceEpoch(value);
        return month ? month->ToString() : utf8_string();
      }
    }
    return {};
  }

  utf8_string DateTimeChooserOverlay::HighlightedValue() const noexcept
  {
    return ValueFor(_highlighted);
  }

  void DateTimeChooserOverlay::Build() noexcept
  {
    krys_debug_assert(_owner);

    auto locale = boo::Locale::Create(_parameters.Locale);

    utf8_string html = u8"<!DOCTYPE html><style>"
                       u8"html { width: max-content; margin: 0; border: 1px solid GrayText; cursor: default;"
                       u8" background: Canvas; color: CanvasText; font: 13px sans-serif;";
    if (_parameters.UseDarkAppearance)
    {
      html += u8" color-scheme: dark;";
    }
    html +=
      u8" }\nbody { margin: 0; padding: 4px; }\n"
      u8".header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 4px;"
      u8" font-weight: bold; }\n"
      u8".nav { padding: 0 8px; }\n"
      u8"table { border-collapse: collapse; }\n"
      u8"th { font-weight: normal; color: GrayText; }\n"
      u8"th, td { padding: 2px 4px; min-width: 2em; text-align: center; }\n"
      u8".other, .week { color: GrayText; }\n"
      u8".disabled { color: GrayText; opacity: 0.5; }\n"
      u8".today { font-weight: bold; }\n"
      u8".selected { outline: 1px solid Highlight; outline-offset: -1px; }\n"
      u8".highlighted { background: Highlight; color: HighlightText; }\n"
      u8"</style><body><div class=header><span class=nav data-action=-1>&lt;</span><span>";

    auto cellClasses = [&](int32 value, bool other)
    {
      utf8_string classes;
      auto add = [&](utf8_stringview name)
      {
        if (!classes.empty())
        {
          classes += u8' ';
        }
        classes += name;
      };
      if (other)
      {
        add(u8"other");
      }
      if (!IsEnabled(value))
      {
        add(u8"disabled");
      }
      if (_selected == value && _mode != Mode::Weeks)
      {
        add(u8"selected");
      }
      return classes;
    };

    if (_mode == Mode::Months)
    {
      html += Number(1'970 + _page);
      html += u8"</span><span class=nav data-action=1>&gt;</span></div><table>";
      auto &labels = locale->ShortStandAloneMonthLabels();
      for (int32 row = 0; row < 4; ++row)
      {
        html += u8"<tr>";
        for (int32 column = 0; column < 3; ++column)
        {
          auto month = row * 3 + column;
          auto value = _page * 12 + month;
          html += u8"<td data-value=" + Number(value) + u8" class=\"" + cellClasses(value, false) + u8"\">";
          AppendEscaped(html, labels[static_cast<size_t>(month)]);
          html += u8"</td>";
        }
        html += u8"</tr>";
      }
    }
    else
    {
      auto first = FirstDayOfMonth(_page);
      auto last = FirstDayOfMonth(_page + 1) - 1;
      boo::DateComponents month;
      if (auto date = boo::DateComponents::FromMonthsSinceEpoch(_page))
      {
        month = *date;
      }
      AppendEscaped(html, locale->FormatDateTime(month));
      html += u8"</span><span class=nav data-action=1>&gt;</span></div><table><tr>";

      // Weeks start on Monday, as ISO weeks do.
      auto firstWeekDay = _mode == Mode::Weeks ? 1 : 0;
      if (_mode == Mode::Weeks)
      {
        html += u8"<th></th>";
      }
      for (int32 i = 0; i < 7; ++i)
      {
        html += u8"<th>";
        html += WeekDayLabels[static_cast<size_t>((firstWeekDay + i) % 7)];
        html += u8"</th>";
      }
      html += u8"</tr>";

      auto today = DayOf(boo::LocalTimeNow());
      auto start = first - FloorMod(WeekDay(first) - firstWeekDay, 7);
      for (int32 row = 0; row < 6; ++row)
      {
        auto rowStart = start + row * 7;
        if (_mode == Mode::Weeks)
        {
          bool selectedWeek = _selected && *_selected >= rowStart && *_selected < rowStart + 7;
          html += selectedWeek ? u8"<tr class=selected>" : u8"<tr>";
          auto week = boo::DateComponents::FromMillisecondsSinceEpochForWeek(rowStart * MsPerDay);
          html += u8"<td class=week>" + Number(week ? week->Week() : 0) + u8"</td>";
        }
        else
        {
          html += u8"<tr>";
        }
        for (int32 column = 0; column < 7; ++column)
        {
          auto day = rowStart + column;
          auto classes = cellClasses(day, day < first || day > last);
          if (day == today)
          {
            classes += classes.empty() ? u8"today" : u8" today";
          }
          auto date = DayToDate(day);
          html += u8"<td data-value=" + Number(day) + u8" class=\"" + classes + u8"\">"
                  + Number(date ? date->MonthDay() : 0) + u8"</td>";
        }
        html += u8"</tr>";
      }
    }
    html += u8"</table>";

    auto viewport = _owner->ViewportSize();
    if (!_view)
    {
      _view = CreateUnique<View>(viewport);
    }
    else
    {
      _view->Resize(viewport);
    }
    _view->LoadHTML(krys::move(html));
    (void)_view->Document().UpdateLayout();

    _cells.clear();
    for (boo::dom::Node *node = _view->Document().DocumentElement().get(); node;
         node = boo::dom::TreeQueries::Next(*node))
    {
      if (auto *element = DynamicDowncast<boo::dom::Element>(*node))
      {
        if (auto attribute = element->GetAttribute(u8"data-value"))
        {
          if (auto value = ParseInteger(*attribute))
          {
            _cells.insert_or_assign(*value, ShareRefPtr(element));
          }
        }
      }
    }

    boo::IntSize contentSize(1, 1);
    if (auto *root = _view->Document().DocumentElement().get())
    {
      if (auto *box = DynamicDowncast<boo::RenderBox>(root->Renderer()))
      {
        contentSize = boo::IntSize(box->Width().ToInt(), box->Height().ToInt());
      }
    }
    _rect = Place(_parameters.AnchorRectInRootView, contentSize, viewport);
    _view->Resize(_rect.Size());

    SetHighlightedClass(_highlighted, true);
  }

  void DateTimeChooserOverlay::SetHighlightedClass(int32 value, bool on) noexcept
  {
    auto cell = _cells.find(value);
    if (cell == _cells.end() || !cell->second)
    {
      return;
    }
    // A week's row is highlighted as a whole.
    auto *element = cell->second.get();
    if (_mode == Mode::Weeks)
    {
      element = element->ParentElement();
    }
    if (element)
    {
      SetClass(*element, u8"highlighted", on);
    }
  }

  void DateTimeChooserOverlay::Highlight(int32 value) noexcept
  {
    if (PageOf(value) != _page)
    {
      _highlighted = value;
      _page = PageOf(value);
      Build();
      return;
    }

    MoveHighlight(value);
  }

  void DateTimeChooserOverlay::MoveHighlight(int32 value) noexcept
  {
    SetHighlightedClass(_highlighted, false);
    _highlighted = value;
    SetHighlightedClass(_highlighted, true);
  }

  void DateTimeChooserOverlay::TurnPage(int32 pages) noexcept
  {
    if (_mode == Mode::Months)
    {
      Highlight(_highlighted + pages * 12);
      return;
    }

    // The same day of the month, or the month's last if it's shorter.
    auto date = DayToDate(_highlighted);
    auto monthDay = date ? date->MonthDay() : 1;
    auto month = PageOf(_highlighted) + pages;
    auto first = FirstDayOfMonth(month);
    auto length = FirstDayOfMonth(month + 1) - first;
    Highlight(first + std::min(monthDay, length) - 1);
  }

  void DateTimeChooserOverlay::Choose(int32 value) noexcept
  {
    if (!IsEnabled(value))
    {
      return;
    }

    // The picker hides before its client hears of the choice, as a popup does.
    Ref<DateTimeChooserOverlay> protectedThis = ShareRef(*this);
    auto string = ValueFor(value);
    auto *client = _client;
    EndChooser();
    if (client && !string.empty())
    {
      client->DidChooseValue(string);
    }
  }

  void DateTimeChooserOverlay::MouseMoved(const boo::IntPoint &point) noexcept
  {
    if (!_view)
    {
      return;
    }
    if (auto *cell = ElementWithAttributeAt(*_view, point, u8"data-value"))
    {
      // The days of the months either side are shown too: hovering one highlights it without turning the
      // page, which would move the days under the mouse.
      if (auto value = ParseInteger(*cell->GetAttribute(u8"data-value")); value && IsEnabled(*value))
      {
        MoveHighlight(*value);
      }
    }
  }

  void DateTimeChooserOverlay::MouseReleased(const boo::IntPoint &point) noexcept
  {
    if (!_view)
    {
      return;
    }
    if (auto *action = ElementWithAttributeAt(*_view, point, u8"data-action"))
    {
      if (auto pages = ParseInteger(*action->GetAttribute(u8"data-action")))
      {
        TurnPage(*pages);
      }
      return;
    }
    if (auto *cell = ElementWithAttributeAt(*_view, point, u8"data-value"))
    {
      if (auto value = ParseInteger(*cell->GetAttribute(u8"data-value")))
      {
        Choose(*value);
      }
    }
  }

  bool DateTimeChooserOverlay::MouseWheel(const boo::IntPoint &, float, float notchesY) noexcept
  {
    // The wheel turns the page.
    if (notchesY)
    {
      TurnPage(notchesY > 0 ? -1 : 1);
    }
    return true;
  }

  bool DateTimeChooserOverlay::KeyPressed(pal::Key key, pal::KeyModifiers) noexcept
  {
    bool months = _mode == Mode::Months;
    switch (key)
    {
      case pal::Key::LeftArrow:   Highlight(_highlighted - 1); return true;
      case pal::Key::RightArrow:  Highlight(_highlighted + 1); return true;
      case pal::Key::UpArrow:     Highlight(_highlighted - (months ? 3 : 7)); return true;
      case pal::Key::DownArrow:   Highlight(_highlighted + (months ? 3 : 7)); return true;
      case pal::Key::PageUp:      TurnPage(-1); return true;
      case pal::Key::PageDown:    TurnPage(1); return true;
      case pal::Key::Home:        Highlight(months ? _page * 12 : FirstDayOfMonth(_page)); return true;
      case pal::Key::End:         Highlight(months ? _page * 12 + 11 : FirstDayOfMonth(_page + 1) - 1); return true;
      case pal::Key::Enter:
      case pal::Key::KeypadEnter:
      case pal::Key::Space:       Choose(_highlighted); return true;
      case pal::Key::Escape:
      case pal::Key::Tab:         EndChooser(); return true;
      default:                    return true;
    }
  }
}
