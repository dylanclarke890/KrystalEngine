#pragma once

#include "Krystal.Booey/Overlay.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Web/Graphics/Color/Color.hpp"
#include "Krystal.Web/Platform/Choosers/ColorChooser.hpp"

namespace krys::web
{
  class ColorChooserClient;
}

namespace krys::boo
{
  /// @brief A colour input's picker, shown by the view of its page as an overlay: a palette of colours (the
  /// input's suggestions first) below the input (or above it, if there's more room there) within the
  /// viewport. Choosing a colour, Escape and pressing outside the picker hide it, and go back through its
  /// client.
  class ColorChooserOverlay final : public web::ColorChooser, public Overlay
  {
  public:
    /// @brief The palette's colours per row.
    constexpr static int32 Columns = 8;

  private:
    View *_owner {nullptr};
    web::ColorChooserClient *_client {nullptr};
    UniquePtr<View> _view;
    RectI _rect;
    /// @brief The colours shown, in order.
    List<web::Color> _colors;
    /// @brief The element of each colour, by its index.
    List<RefPtr<web::Element>> _swatches;
    web::Color _selectedColor;
    int32 _highlightedIndex {-1};
    bool _visible {false};
    /// @brief Whether the control lays out right to left, so the overlay is mirrored.
    bool _rightToLeft {false};

  public:
    /// @brief Makes the picker, and shows it with @p initialColor chosen.
    ColorChooserOverlay(View &owner, web::ColorChooserClient &client, web::Color initialColor) noexcept;

    ~ColorChooserOverlay() noexcept override;

#pragma region ColorChooser

    void ReattachColorChooser(const web::Color &color) noexcept override;

    void SetSelectedColor(const web::Color &color) noexcept override;

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

    KRYS_NODISCARD const RectI &Rect() const noexcept final
    {
      return _rect;
    }

    void Hide() noexcept final;

    /// @brief Highlights the colour under @p point.
    void MouseMoved(const PointI &point) noexcept final;

    /// @brief Chooses the colour under @p point.
    void MouseReleased(const PointI &point) noexcept final;

    bool MouseWheel(const PointI &point, float notchesX, float notchesY) noexcept final;

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

    KRYS_NODISCARD const List<web::Color> &Colors() const noexcept
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
