#include "Krystal.UI/DataListSuggestionsOverlay.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Platform/Choosers/DataListSuggestionsClient.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <format>

namespace krys::ui
{
  namespace
  {
    /// @brief The tallest the list gets, in suggestions, before it scrolls.
    constexpr int32 MaximumVisibleSuggestions = 10;
  }

  DataListSuggestionsOverlay::DataListSuggestionsOverlay(View &owner,
                                                         boo::DataListSuggestionsClient &client) noexcept
      : _owner(&owner), _client(&client)
  {
  }

  DataListSuggestionsOverlay::~DataListSuggestionsOverlay() noexcept = default;

  void DataListSuggestionsOverlay::Detach() noexcept
  {
    _client = nullptr;
    Hide();
  }

  void DataListSuggestionsOverlay::Close() noexcept
  {
    Hide();
  }

  void DataListSuggestionsOverlay::DisplayWithActivationType(boo::DataListSuggestionActivationType) noexcept
  {
    if (!_client || !_owner)
    {
      return;
    }

    _suggestions = _client->Suggestions();
    if (_suggestions.empty())
    {
      Hide();
      return;
    }

    // The highlight goes when the suggestions change.
    _highlightedIndex = -1;
    Build();
    if (!_visible)
    {
      _visible = true;
      ShowIn(*_owner, *this);
    }
  }

  void DataListSuggestionsOverlay::HandleKeydownWithIdentifier(utf8_stringview identifier) noexcept
  {
    auto count = static_cast<int32>(_suggestions.size());
    if (!_visible || !count)
    {
      return;
    }
    if (identifier == u8"Down")
    {
      Highlight(_highlightedIndex + 1 < count ? _highlightedIndex + 1 : 0);
    }
    else if (identifier == u8"Up")
    {
      Highlight(_highlightedIndex > 0 ? _highlightedIndex - 1 : count - 1);
    }
    else if (identifier == u8"Enter")
    {
      if (_highlightedIndex >= 0)
      {
        Choose(_highlightedIndex);
      }
      else
      {
        Hide();
      }
    }
    else if (identifier == u8"U+001B")
    {
      Hide();
    }
  }

  void DataListSuggestionsOverlay::Hide() noexcept
  {
    if (!_visible)
    {
      return;
    }

    // The owner lets go of the list, which its client still holds while it's told.
    Ref<DataListSuggestionsOverlay> protectedThis = ShareRef(*this);
    _visible = false;
    if (_owner)
    {
      HiddenFrom(*_owner, *this);
    }
    if (auto *client = _client)
    {
      client->DidCloseSuggestions();
    }
  }

  void DataListSuggestionsOverlay::Build() noexcept
  {
    krys_debug_assert(_client && _owner);

    // A right-to-left field's list is mirrored: its labels go on the left.
    bool rightToLeft = _client->IsElementRTL();
    utf8_string html = rightToLeft ? u8"<!DOCTYPE html><html dir=rtl><style>" : u8"<!DOCTYPE html><style>";
    html += u8"html { width: max-content; min-width: 100%; box-sizing: border-box; margin: 0;"
            u8" border: 1px solid GrayText; cursor: default; background: Canvas; color: CanvasText;"
            u8" font: 13px sans-serif; }\n"
            u8"body { margin: 0; padding: 2px 0; }\n"
            u8".item { display: flex; justify-content: space-between; gap: 16px; padding: 2px 6px;"
            u8" white-space: pre; }\n"
            u8".label { color: GrayText; }\n"
            u8".highlighted { background: Highlight; color: HighlightText; }\n"
            u8".highlighted .label { color: HighlightText; }\n"
            u8"</style><body>";
    for (size_t i = 0; i < _suggestions.size(); ++i)
    {
      html += std::format("<div data-index={} class=item><span>", i) | std::ranges::to<utf8_string>();
      AppendEscaped(html, _suggestions[i].Value);
      html += u8"</span>";
      if (!_suggestions[i].Label.empty())
      {
        html += u8"<span class=label>";
        AppendEscaped(html, _suggestions[i].Label);
        html += u8"</span>";
      }
      html += u8"</div>";
    }

    // At least the field's width, and as tall as the suggestions, up to a limit.
    auto fieldRect = _client->ElementRectInRootViewCoordinates();
    auto viewport = _owner->ViewportSize();
    auto width = std::max(fieldRect.Width(), 1);
    if (!_view)
    {
      _view = CreateUnique<View>(boo::IntSize(width, viewport.Height()));
    }
    else
    {
      _view->Resize(boo::IntSize(width, viewport.Height()));
    }
    ReplaceDocument(*_view, krys::move(html));
    (void)_view->Document().UpdateLayout();

    _items.assign(_suggestions.size(), nullptr);
    for (boo::dom::Node *node = _view->Document().DocumentElement().get(); node;
         node = boo::dom::TreeQueries::Next(*node))
    {
      if (auto *element = DynamicDowncast<boo::dom::Element>(*node))
      {
        if (auto index = element->GetAttribute(u8"data-index"))
        {
          auto value = std::stoul(std::string(index->begin(), index->end()));
          if (value < _items.size())
          {
            _items[value] = ShareRefPtr(element);
          }
        }
      }
    }

    boo::IntSize contentSize(width, 1);
    if (auto *root = _view->Document().DocumentElement().get())
    {
      if (auto *box = DynamicDowncast<boo::RenderBox>(root->Renderer()))
      {
        contentSize = boo::IntSize(box->Width().ToInt(), box->Height().ToInt());
      }
    }
    int32 itemHeight = 0;
    for (auto &item : _items)
    {
      if (auto *box = item ? DynamicDowncast<boo::RenderBox>(item->Renderer()) : nullptr)
      {
        itemHeight = std::max(itemHeight, box->Height().ToInt());
      }
    }
    if (itemHeight)
    {
      contentSize.SetHeight(std::min(contentSize.Height(), itemHeight * MaximumVisibleSuggestions + 6));
    }
    contentSize.SetWidth(std::max(contentSize.Width(), width));
    _rect = Place(fieldRect, contentSize, viewport, rightToLeft);
    _view->Resize(_rect.Size());

    auto highlighted = std::exchange(_highlightedIndex, -1);
    Highlight(highlighted);
  }

  void DataListSuggestionsOverlay::Highlight(int32 index) noexcept
  {
    auto element = [&](int32 i) -> boo::dom::Element *
    {
      return i >= 0 && static_cast<size_t>(i) < _items.size() ? _items[static_cast<size_t>(i)].get()
                                                              : nullptr;
    };
    if (auto *old = element(_highlightedIndex))
    {
      SetClass(*old, u8"highlighted", false);
    }
    _highlightedIndex = element(index) ? index : -1;
    if (auto *highlighted = element(_highlightedIndex))
    {
      SetClass(*highlighted, u8"highlighted", true);
    }
  }

  void DataListSuggestionsOverlay::Choose(int32 index) noexcept
  {
    if (index < 0 || static_cast<size_t>(index) >= _suggestions.size())
    {
      return;
    }

    // The list hides before its client hears of the choice, as a popup does.
    Ref<DataListSuggestionsOverlay> protectedThis = ShareRef(*this);
    auto value = _suggestions[static_cast<size_t>(index)].Value;
    auto *client = _client;
    Hide();
    if (client)
    {
      client->DidSelectDataListOption(value);
    }
  }

  int32 DataListSuggestionsOverlay::ItemAt(const boo::IntPoint &point) const noexcept
  {
    if (!_view)
    {
      return -1;
    }
    if (auto *item = ElementWithAttributeAt(*_view, point, u8"data-index"))
    {
      auto it = std::ranges::find_if(_items, [&](auto &element) { return element.get() == item; });
      if (it != _items.end())
      {
        return static_cast<int32>(it - _items.begin());
      }
    }
    return -1;
  }

  void DataListSuggestionsOverlay::MouseMoved(const boo::IntPoint &point) noexcept
  {
    if (auto index = ItemAt(point); index >= 0)
    {
      Highlight(index);
    }
  }

  void DataListSuggestionsOverlay::MouseReleased(const boo::IntPoint &point) noexcept
  {
    Choose(ItemAt(point));
  }

  bool DataListSuggestionsOverlay::MouseWheel(const boo::IntPoint &, float, float) noexcept
  {
    return false;
  }

  bool DataListSuggestionsOverlay::KeyPressed(pal::Key, pal::KeyModifiers) noexcept
  {
    return false;
  }
}
