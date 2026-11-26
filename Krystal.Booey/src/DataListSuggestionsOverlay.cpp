#include "Krystal.Booey/DataListSuggestionsOverlay.hpp"
#include "Krystal.Booey/View.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Platform/Choosers/DataListSuggestionsClient.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include <algorithm>
#include <format>

namespace krys::boo
{
  namespace
  {
    /// @brief The tallest the list gets, in suggestions, before it scrolls.
    constexpr int32 MaximumVisibleSuggestions = 10;
  }

  DataListSuggestionsOverlay::DataListSuggestionsOverlay(View &owner,
                                                         web::DataListSuggestionsClient &client) noexcept
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

  void DataListSuggestionsOverlay::DisplayWithActivationType(web::DataListSuggestionActivationType) noexcept
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

  void DataListSuggestionsOverlay::HandleKeyDown(utf8_stringview key) noexcept
  {
    auto count = static_cast<int32>(_suggestions.size());
    if (!_visible || !count)
    {
      return;
    }
    if (key == u8"ArrowDown")
    {
      Highlight(_highlightedIndex + 1 < count ? _highlightedIndex + 1 : 0);
    }
    else if (key == u8"ArrowUp")
    {
      Highlight(_highlightedIndex > 0 ? _highlightedIndex - 1 : count - 1);
    }
    else if (key == u8"Enter")
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
    else if (key == u8"Escape")
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
      _view = CreateUnique<View>(SizeI(width, viewport.Height()));
    }
    SizeContentView(*_view, *_owner, SizeI(width, viewport.Height()));
    ReplaceDocument(*_view, krys::move(html));
    (void)_view->Document().UpdateLayout();

    _items.assign(_suggestions.size(), nullptr);
    for (web::Node *node = _view->Document().DocumentElement().get(); node;
         node = web::dom::TreeQueries::Next(*node))
    {
      if (auto *element = DynamicDowncast<web::Element>(*node))
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

    SizeI contentSize(width, 1);
    if (auto *root = _view->Document().DocumentElement().get())
    {
      if (auto *box = DynamicDowncast<web::RenderBox>(root->Renderer()))
      {
        contentSize = SizeI(box->Width().ToInt(), box->Height().ToInt());
      }
    }
    int32 itemHeight = 0;
    for (auto &item : _items)
    {
      if (auto *box = item ? DynamicDowncast<web::RenderBox>(item->Renderer()) : nullptr)
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
    SizeContentView(*_view, *_owner, _rect.Size());

    auto highlighted = std::exchange(_highlightedIndex, -1);
    Highlight(highlighted);
  }

  void DataListSuggestionsOverlay::Highlight(int32 index) noexcept
  {
    auto element = [&](int32 i) -> web::Element *
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

  int32 DataListSuggestionsOverlay::ItemAt(const PointI &point) const noexcept
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

  void DataListSuggestionsOverlay::MouseMoved(const PointI &point) noexcept
  {
    if (auto index = ItemAt(point); index >= 0)
    {
      Highlight(index);
    }
  }

  void DataListSuggestionsOverlay::MouseReleased(const PointI &point) noexcept
  {
    Choose(ItemAt(point));
  }

  bool DataListSuggestionsOverlay::MouseWheel(const PointI &, float, float) noexcept
  {
    return false;
  }

  bool DataListSuggestionsOverlay::KeyPressed(pal::Key, pal::KeyModifiers) noexcept
  {
    return false;
  }
}
