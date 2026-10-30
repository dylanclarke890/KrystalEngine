#include "Krystal.Booey/Editing/TemporarySelectionChange.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include <utility>

namespace krys::boo
{
  TemporarySelectionChange::TemporarySelectionChange(dom::Document &document,
                                                     Maybe<VisibleSelection> temporarySelection,
                                                     EnumSet<TemporarySelectionOption> options) noexcept
      : _document(ShareRefPtr(&document)), _options(options),
        _wasIgnoringSelectionChanges(document.GetEditor().IgnoreSelectionChanges())
  {
    if (options.Contains(TemporarySelectionOption::IgnoreSelectionChanges))
    {
      document.GetEditor().SetIgnoreSelectionChanges(true);
    }

    if (temporarySelection)
    {
      _selectionToRestore = document.Selection().Selection();
      SetSelection(*temporarySelection, IsTemporarySelection::Yes);
    }
  }

  void TemporarySelectionChange::Invalidate() noexcept
  {
    if (auto document = std::exchange(_document, nullptr))
    {
      document->GetEditor().SetIgnoreSelectionChanges(false, Editor::RevealSelection::No);
    }
  }

  TemporarySelectionChange::~TemporarySelectionChange() noexcept
  {
    if (!_document)
    {
      return;
    }

    if (_selectionToRestore)
    {
      SetSelection(*_selectionToRestore, IsTemporarySelection::No);
    }

    if (_options.Contains(TemporarySelectionOption::IgnoreSelectionChanges))
    {
      auto revealSelection = _options.Contains(TemporarySelectionOption::RevealSelection)
                               ? Editor::RevealSelection::Yes
                               : Editor::RevealSelection::No;
      _document->GetEditor().SetIgnoreSelectionChanges(_wasIgnoringSelectionChanges, revealSelection);
    }
  }

  void TemporarySelectionChange::SetSelection(const VisibleSelection &selection,
                                              IsTemporarySelection isTemporarySelection) noexcept
  {
    using SetSelectionOption = FrameSelection::SetSelectionOption;
    auto options = FrameSelection::DefaultSetSelectionOptions();

    if (_options.Contains(TemporarySelectionOption::UserTriggered))
    {
      options.Add(SetSelectionOption::IsUserTriggered);
    }

    if (_options.Contains(TemporarySelectionOption::DoNotSetFocus))
    {
      options.Add(SetSelectionOption::DoNotSetFocus);
    }

    if (isTemporarySelection == IsTemporarySelection::Yes)
    {
      if (_options.Contains(TemporarySelectionOption::RevealSelection))
      {
        options.Add(SetSelectionOption::RevealSelection);
      }
      if (_options.Contains(TemporarySelectionOption::DelegateMainFrameScroll))
      {
        options.Add(SetSelectionOption::DelegateMainFrameScroll);
      }
      if (_options.Contains(TemporarySelectionOption::SmoothScroll))
      {
        options.Add(SetSelectionOption::SmoothScroll);
      }
      if (_options.Contains(TemporarySelectionOption::RevealSelectionBounds))
      {
        options.Add(SetSelectionOption::RevealSelectionBounds);
      }
      if (_options.Contains(TemporarySelectionOption::ForceCenterScroll))
      {
        options.Add(SetSelectionOption::ForceCenterScroll);
      }
      if (_options.Contains(TemporarySelectionOption::OnlyAllowForwardScrolling))
      {
        options.Add(SetSelectionOption::OnlyAllowForwardScrolling);
      }
    }

    _document->Selection().SetSelection(selection, options);

    // The selection is brought up to date and revealed at the next rendering update, by which time a
    // temporary one is gone, so it's revealed now.
    // FIXME: Revealing only forward (OnlyAllowForwardScrolling), once revealing selections does.
    if (isTemporarySelection == IsTemporarySelection::Yes
        && _options.Contains(TemporarySelectionOption::RevealSelection))
    {
      _document->Selection().UpdateAndRevealSelection(
        _options.Contains(TemporarySelectionOption::SmoothScroll) ? ScrollBehavior::Smooth
                                                                  : ScrollBehavior::Instant,
        _options.Contains(TemporarySelectionOption::RevealSelectionBounds)
          ? RevealExtentOption::DoNotRevealExtent
          : RevealExtentOption::RevealExtent,
        _options.Contains(TemporarySelectionOption::ForceCenterScroll) ? ForceCenterScroll::Yes
                                                                       : ForceCenterScroll::No);
    }
  }
}
