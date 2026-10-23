#pragma once

#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  /// @brief How a temporary selection is made.
  enum class TemporarySelectionOption : uint8
  {
    RevealSelection,
    DoNotSetFocus,
    /// @brief Don't propagate selection changes to the client layer.
    IgnoreSelectionChanges,
    /// @brief Force the render tree to update selection state. Only respected on iOS.
    EnableAppearanceUpdates,
    SmoothScroll,
    DelegateMainFrameScroll,
    RevealSelectionBounds,
    UserTriggered,
    ForceCenterScroll,
    OnlyAllowForwardScrolling
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::TemporarySelectionOption, 10uz);

namespace krys::boo
{
  /// @brief Selects something for as long as it lives (to reveal it, say), then restores the selection.
  class TemporarySelectionChange
  {
    KRYS_NON_COPY_MOVABLE(TemporarySelectionChange);

  private:
    enum class IsTemporarySelection : bool
    {
      No,
      Yes
    };

    RefPtr<dom::Document> _document;
    EnumSet<TemporarySelectionOption> _options;
    Maybe<VisibleSelection> _selectionToRestore;

  public:
    explicit TemporarySelectionChange(dom::Document &document,
                                      Maybe<VisibleSelection> temporarySelection = std::nullopt,
                                      EnumSet<TemporarySelectionOption> options = {}) noexcept;
    ~TemporarySelectionChange() noexcept;

    /// @brief Keeps the temporary selection: nothing is restored.
    void Invalidate() noexcept;

  private:
    void SetSelection(const VisibleSelection &selection, IsTemporarySelection isTemporarySelection) noexcept;
  };
}
