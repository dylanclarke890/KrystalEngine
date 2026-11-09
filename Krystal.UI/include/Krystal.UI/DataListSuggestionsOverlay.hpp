#pragma once

#include "Krystal.Booey/Platform/Choosers/DataListSuggestionPicker.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.UI/Overlay.hpp"

namespace krys::boo
{
  class DataListSuggestionsClient;
}

namespace krys::ui
{
  /// @brief A text field's datalist suggestions, shown by the view of its page as an overlay below the field
  /// (or above it, if there's more room there), with their labels. It isn't modal: the field keeps the
  /// keyboard, passing on the arrow keys, Enter and Escape; a press elsewhere hides it and goes on to the
  /// page.
  class DataListSuggestionsOverlay final : public boo::DataListSuggestionPicker, public Overlay
  {
  private:
    View *_owner {nullptr};
    boo::DataListSuggestionsClient *_client {nullptr};
    UniquePtr<View> _view;
    List<boo::DataListSuggestion> _suggestions;
    /// @brief The element of each suggestion, by its index.
    List<RefPtr<boo::dom::Element>> _items;
    boo::IntRect _rect;
    int32 _highlightedIndex {-1};
    bool _visible {false};

  public:
    DataListSuggestionsOverlay(View &owner, boo::DataListSuggestionsClient &client) noexcept;
    ~DataListSuggestionsOverlay() noexcept override;

#pragma region DataListSuggestionPicker

    void Detach() noexcept override;
    void Close() noexcept override;
    /// @brief Up and Down move the highlight, Enter chooses the highlighted suggestion and Escape hides the
    /// list.
    void HandleKeydownWithIdentifier(utf8_stringview identifier) noexcept override;
    void DisplayWithActivationType(boo::DataListSuggestionActivationType type) noexcept override;

#pragma endregion

#pragma region Overlay

    void AddRef() const noexcept final
    {
      DataListSuggestionPicker::AddRef();
    }

    void SubRef() const noexcept final
    {
      DataListSuggestionPicker::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return DataListSuggestionPicker::GetRefCount();
    }

    KRYS_NODISCARD View &ContentView() const noexcept final
    {
      return *_view;
    }

    KRYS_NODISCARD const boo::IntRect &Rect() const noexcept final
    {
      return _rect;
    }

    KRYS_NODISCARD bool IsModal() const noexcept final
    {
      return false;
    }

    void Hide() noexcept final;
    /// @brief Highlights the suggestion under @p point.
    void MouseMoved(const boo::IntPoint &point) noexcept final;
    /// @brief Chooses the suggestion under @p point.
    void MouseReleased(const boo::IntPoint &point) noexcept final;
    bool MouseWheel(const boo::IntPoint &point, float notchesX, float notchesY) noexcept final;
    /// @brief The field takes the keys.
    bool KeyPressed(pal::Key key, pal::KeyModifiers modifiers) noexcept final;

#pragma endregion

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _visible;
    }

    KRYS_NODISCARD const List<boo::DataListSuggestion> &Suggestions() const noexcept
    {
      return _suggestions;
    }

    /// @brief The highlighted suggestion's index, or -1 if none is.
    KRYS_NODISCARD int32 HighlightedIndex() const noexcept
    {
      return _highlightedIndex;
    }

  private:
    /// @brief Fills the list's page with the suggestions, and places it.
    void Build() noexcept;
    void Highlight(int32 index) noexcept;
    void Choose(int32 index) noexcept;
    KRYS_NODISCARD int32 ItemAt(const boo::IntPoint &point) const noexcept;
  };
}
