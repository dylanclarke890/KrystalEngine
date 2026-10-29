#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Platform/ColorChooser.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.UI/Overlay.hpp"

namespace krys::boo
{
  class ColorChooserClient;
}

namespace krys::ui
{
  /// @brief A colour input's picker, shown by the view of its page as an overlay: a palette of colours (the
  /// input's suggestions first) below the input (or above it, if there's more room there) within the
  /// viewport. Choosing a colour, Escape and pressing outside the picker hide it, and go back through its
  /// client.
  class ColorChooserOverlay final : public boo::ColorChooser, public Overlay
  {
  public:
    /// @brief The palette's colours per row.
    static constexpr int32 Columns = 8;

  private:
    View *_owner {nullptr};
    boo::ColorChooserClient *_client {nullptr};
    UniquePtr<View> _view;
    boo::IntRect _rect;
    /// @brief The colours shown, in order.
    List<boo::Color> _colors;
    /// @brief The element of each colour, by its index.
    List<RefPtr<boo::dom::Element>> _swatches;
    boo::Color _selectedColor;
    int32 _highlightedIndex {-1};
    bool _visible {false};
    /// @brief Whether the control lays out right to left, so the overlay is mirrored.
    bool _rightToLeft {false};

  public:
    /// @brief Makes the picker, and shows it with @p initialColor chosen.
    ColorChooserOverlay(View &owner, boo::ColorChooserClient &client,
                        const boo::Color &initialColor) noexcept;
    ~ColorChooserOverlay() noexcept override;

#pragma region ColorChooser

    void ReattachColorChooser(const boo::Color &color) noexcept override;
    void SetSelectedColor(const boo::Color &color) noexcept override;
    void EndChooser() noexcept override;

#pragma endregion

#pragma region Overlay

    void AddRef() const noexcept final
    {
      ColorChooser::AddRef();
    }

    void SubRef() const noexcept final
    {
      ColorChooser::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return ColorChooser::GetRefCount();
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
    /// @brief Highlights the colour under @p point.
    void MouseMoved(const boo::IntPoint &point) noexcept final;
    /// @brief Chooses the colour under @p point.
    void MouseReleased(const boo::IntPoint &point) noexcept final;
    bool MouseWheel(const boo::IntPoint &point, float notchesX, float notchesY) noexcept final;
    /// @brief The arrow keys move the highlight; Enter and Space choose; Escape and Tab hide the picker.
    bool KeyPressed(pal::Key key, pal::KeyModifiers modifiers) noexcept final;

#pragma endregion

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _visible;
    }

    /// @brief The colour the keys and the mouse have chosen, by its index in the palette, or -1.
    KRYS_NODISCARD int32 HighlightedIndex() const noexcept
    {
      return _highlightedIndex;
    }

    KRYS_NODISCARD const List<boo::Color> &Colors() const noexcept
    {
      return _colors;
    }

  private:
    void Show() noexcept;
    /// @brief Fills the picker's page with the palette, and places it.
    void Build() noexcept;
    void Highlight(int32 index) noexcept;
    void Choose(int32 index) noexcept;
  };
}
