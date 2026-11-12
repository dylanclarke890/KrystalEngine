#include "Krystal.UI/ViewAccessibility.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityScrollView.hpp"
#include "Krystal.Booey/Accessibility/AXNotifications.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.UI/View.hpp"
#include <cmath>

namespace krys::ui
{
  namespace
  {
    using boo::ax::AccessibilityObject;
    using boo::ax::AccessibilityRole;
    using boo::ax::AXNotification;

    KRYS_NODISCARD pal::AccessibilityNodeID ID(const AccessibilityObject *object) noexcept
    {
      return object ? static_cast<pal::AccessibilityNodeID>(object->ObjectID()) : 0;
    }

    KRYS_NODISCARD pal::AccessibilityRole ToPalRole(const AccessibilityObject &object) noexcept
    {
      using Role = pal::AccessibilityRole;
      switch (object.Role())
      {
        case AccessibilityRole::WebArea:
        case AccessibilityRole::Document:
        case AccessibilityRole::DocumentArticle:
        case AccessibilityRole::GraphicsDocument:       return Role::Document;
        case AccessibilityRole::ScrollArea:             return Role::Pane;
        case AccessibilityRole::ApplicationDialog:
        case AccessibilityRole::ApplicationAlertDialog: return Role::Dialog;
        case AccessibilityRole::ApplicationAlert:       return Role::Alert;
        case AccessibilityRole::Button:
        case AccessibilityRole::PopUpButton:            return Role::Button;
        case AccessibilityRole::ToggleButton:
        case AccessibilityRole::Switch:                 return Role::ToggleButton;
        case AccessibilityRole::Checkbox:
        case AccessibilityRole::MenuItemCheckbox:
          return object.Role() == AccessibilityRole::Checkbox ? Role::CheckBox : Role::MenuItem;
        case AccessibilityRole::RadioButton:           return Role::RadioButton;
        case AccessibilityRole::ComboBox:              return Role::ComboBox;
        case AccessibilityRole::TextField:
        case AccessibilityRole::TextArea:
        case AccessibilityRole::SearchField:           return Role::Edit;
        case AccessibilityRole::Link:                  return Role::Link;
        case AccessibilityRole::Image:
        case AccessibilityRole::ImageMap:
        case AccessibilityRole::Canvas:
        case AccessibilityRole::GraphicsObject:
        case AccessibilityRole::GraphicsSymbol:
        case AccessibilityRole::SVGRoot:               return Role::Image;
        case AccessibilityRole::Heading:               return Role::Heading;
        case AccessibilityRole::Paragraph:             return Role::Paragraph;
        case AccessibilityRole::StaticText:
        case AccessibilityRole::Label:
        case AccessibilityRole::ListMarker:            return Role::Text;
        case AccessibilityRole::List:
        case AccessibilityRole::ListBox:
        case AccessibilityRole::DescriptionList:
        case AccessibilityRole::Directory:
        case AccessibilityRole::Feed:                  return Role::List;
        case AccessibilityRole::ListItem:
        case AccessibilityRole::ListBoxOption:
        case AccessibilityRole::MenuListOption:
        case AccessibilityRole::DescriptionListTerm:
        case AccessibilityRole::DescriptionListDetail: return Role::ListItem;
        case AccessibilityRole::Menu:
        case AccessibilityRole::MenuListPopup:         return Role::Menu;
        case AccessibilityRole::MenuBar:               return Role::MenuBar;
        case AccessibilityRole::MenuItem:
        case AccessibilityRole::MenuItemRadio:         return Role::MenuItem;
        case AccessibilityRole::ProgressIndicator:
        case AccessibilityRole::Meter:                 return Role::ProgressBar;
        case AccessibilityRole::Slider:                return Role::Slider;
        case AccessibilityRole::SpinButton:            return Role::SpinButton;
        case AccessibilityRole::SpinButtonPart:        return Role::Button;
        case AccessibilityRole::ScrollBar:             return Role::ScrollBar;
        case AccessibilityRole::TabList:               return Role::TabList;
        case AccessibilityRole::Tab:                   return Role::Tab;
        case AccessibilityRole::TabPanel:              return Role::TabPanel;
        case AccessibilityRole::Table:
        case AccessibilityRole::Grid:
        case AccessibilityRole::TreeGrid:              return Role::Table;
        case AccessibilityRole::Row:                   return Role::Row;
        case AccessibilityRole::Cell:
        case AccessibilityRole::GridCell:              return Role::Cell;
        case AccessibilityRole::ColumnHeader:          return Role::ColumnHeader;
        case AccessibilityRole::RowHeader:             return Role::RowHeader;
        case AccessibilityRole::Tree:                  return Role::Tree;
        case AccessibilityRole::TreeItem:              return Role::TreeItem;
        case AccessibilityRole::HorizontalRule:
        case AccessibilityRole::Splitter:              return Role::Separator;
        case AccessibilityRole::Toolbar:               return Role::ToolBar;
        case AccessibilityRole::UserInterfaceTooltip:  return Role::ToolTip;
        case AccessibilityRole::ApplicationStatus:     return Role::StatusBar;
        case AccessibilityRole::LandmarkBanner:
        case AccessibilityRole::LandmarkComplementary:
        case AccessibilityRole::LandmarkContentInfo:
        case AccessibilityRole::LandmarkDocRegion:
        case AccessibilityRole::LandmarkMain:
        case AccessibilityRole::LandmarkNavigation:
        case AccessibilityRole::LandmarkRegion:
        case AccessibilityRole::LandmarkSearch:
        case AccessibilityRole::Form:                  return Role::Landmark;
        case AccessibilityRole::Figure:                return Role::Figure;
        case AccessibilityRole::DocumentMath:          return Role::Math;
        case AccessibilityRole::Unknown:               return Role::Unknown;
        default:                                       return Role::Group;
      }
    }

    KRYS_NODISCARD uint32 UnsignedAttribute(const AccessibilityObject &object,
                                            boo::dom::DOMStringView name) noexcept
    {
      auto value = object.GetAttributeTrimmed(name);
      auto parsed = boo::html::Numbers::ParseNonNegativeInteger(value);
      return !value.empty() && parsed.Success() ? static_cast<uint32>(parsed.Value) : 0;
    }

    KRYS_NODISCARD bool IsPassword(const AccessibilityObject &object) noexcept
    {
      auto *input = DynamicDowncast<boo::html::HTMLInputElement>(object.GetNode());
      return input && input->IsPasswordField();
    }

    KRYS_NODISCARD bool IsRangeControl(const AccessibilityObject &object) noexcept
    {
      switch (object.Role())
      {
        case AccessibilityRole::Slider:
        case AccessibilityRole::ProgressIndicator:
        case AccessibilityRole::Meter:
        case AccessibilityRole::ScrollBar:
        case AccessibilityRole::SpinButton:        return true;
        default:                                   return false;
      }
    }

    /// @brief The event Krystal.PAL has for @p notification, if it has one.
    KRYS_NODISCARD Maybe<pal::AccessibilityEvent> ToPalEvent(const AccessibilityObject &object,
                                                             AXNotification notification) noexcept
    {
      using Event = pal::AccessibilityEvent;
      switch (notification)
      {
        case AXNotification::FocusedUIElementChanged:    return Event::Focused;
        case AXNotification::ChildrenChanged:            return Event::ChildrenChanged;
        case AXNotification::NameChanged:
        case AXNotification::LabelChanged:
        case AXNotification::TextChanged:
        case AXNotification::TextUnderElementChanged:    return Event::NameChanged;
        case AXNotification::DescribedByChanged:
        case AXNotification::ExtendedDescriptionChanged: return Event::DescriptionChanged;
        case AXNotification::ValueChanged:
        case AXNotification::MenuListValueChanged:       return Event::ValueChanged;
        case AXNotification::CheckedStateChanged:
        case AXNotification::PressedStateChanged:        return Event::CheckedChanged;
        case AXNotification::ExpandedChanged:
        case AXNotification::RowExpanded:
        case AXNotification::RowCollapsed:               return Event::ExpandedChanged;
        case AXNotification::SelectedStateChanged:
        case AXNotification::MenuListItemSelected:       return Event::SelectedChanged;
        case AXNotification::SelectedChildrenChanged:
        case AXNotification::SelectedCellsChanged:       return Event::SelectionChanged;
        case AXNotification::DisabledStateChanged:       return Event::EnabledChanged;
        case AXNotification::RequiredStatusChanged:
        case AXNotification::InvalidStatusChanged:
        case AXNotification::ReadOnlyStatusChanged:
        case AXNotification::ElementBusyChanged:
        case AXNotification::FocusableStateChanged:
        case AXNotification::VisitedStateChanged:        return Event::StateChanged;
        case AXNotification::SelectedTextChanged:        return Event::TextSelectionChanged;
        case AXNotification::LiveRegionChanged:          return Event::LiveRegionChanged;
        case AXNotification::LiveRegionCreated:
          // An alert appearing is announced as one; another live region announces its changes.
          if (object.Role() == AccessibilityRole::ApplicationAlert
              || object.Role() == AccessibilityRole::ApplicationAlertDialog)
          {
            return Event::Alert;
          }
          return null;
        case AXNotification::MenuOpened:              return Event::MenuOpened;
        case AXNotification::MenuClosed:              return Event::MenuClosed;
        case AXNotification::LoadComplete:
        case AXNotification::NewDocumentLoadComplete: return Event::DocumentLoaded;
        case AXNotification::PageScrolled:
        case AXNotification::ScrolledToAnchor:        return Event::BoundsChanged;
        default:                                      return null;
      }
    }
  }

  ViewAccessibility::ViewAccessibility(View &view) noexcept : _view(view)
  {
  }

  ViewAccessibility::~ViewAccessibility() noexcept
  {
    Detach();
  }

  void ViewAccessibility::Attach(pal::IWindow &window, const boo::IntPoint &origin) noexcept
  {
    Detach();
    _window = &window;
    _origin = origin;
    _window->SetAccessibilityTree(this);
  }

  void ViewAccessibility::Detach() noexcept
  {
    if (_window)
    {
      _window->SetAccessibilityTree(nullptr);
      _window = nullptr;
    }
  }

  boo::ax::AXObjectCache *ViewAccessibility::Cache() noexcept
  {
    // Assistive technology asking for the tree is what turns accessibility on.
    if (!boo::ax::AXObjectCache::AccessibilityEnabled())
    {
      boo::ax::AXObjectCache::EnableAccessibility();
    }
    return _view.Document().AxObjectCache();
  }

  AccessibilityObject *ViewAccessibility::Object(pal::AccessibilityNodeID node) noexcept
  {
    auto *cache = Cache();
    auto *object = cache && node ? cache->ObjectForID(static_cast<boo::ax::AXID>(node)) : nullptr;
    return object && !object->IsDetached() ? object : nullptr;
  }

  pal::AccessibilityRect ViewAccessibility::ToWindow(const AccessibilityObject &object, float x, float y,
                                                     float width, float height) const noexcept
  {
    // The view and its scroll bars are in the view's coordinates; everything else is in the document's,
    // which the view is scrolled into.
    if (!object.IsScrollView() && object.Role() != AccessibilityRole::ScrollBar)
    {
      auto scroll = _view.ScrollPosition();
      x -= static_cast<float>(scroll.X());
      y -= static_cast<float>(scroll.Y());
    }
    return {x + static_cast<float>(_origin.X()), y + static_cast<float>(_origin.Y()), width, height};
  }

  void ViewAccessibility::NotificationPosted(AccessibilityObject &object,
                                             AXNotification notification) noexcept
  {
    if (!_window || !_window->IsAccessibilityActive())
    {
      return;
    }
    // An ignored object isn't a node: the change is its nearest node's.
    auto *target = object.IsIgnored() ? object.ParentObjectUnignored() : &object;
    if (!target)
    {
      return;
    }
    if (auto event = ToPalEvent(*target, notification))
    {
      _window->RaiseAccessibilityEvent(ID(target), *event);
    }
  }

  void ViewAccessibility::TextChanged(AccessibilityObject &object, boo::ax::AXTextChange change,
                                      uint64 offset, utf8_stringview text) noexcept
  {
    if (_window && _window->IsAccessibilityActive())
    {
      _window->RaiseAccessibilityTextChange(ID(&object),
                                            change == boo::ax::AXTextChange::Inserted
                                              ? pal::AccessibilityTextChange::Inserted
                                              : pal::AccessibilityTextChange::Deleted,
                                            offset, text);
    }
  }

  void ViewAccessibility::ObjectDetached(AccessibilityObject &object) noexcept
  {
    if (_window && _window->IsAccessibilityActive())
    {
      _window->AccessibilityNodeRemoved(ID(&object));
    }
  }

  pal::AccessibilityNodeID ViewAccessibility::Root() noexcept
  {
    auto *cache = Cache();
    return ID(cache ? cache->RootObject() : nullptr);
  }

  Maybe<pal::AccessibilityNode> ViewAccessibility::Node(pal::AccessibilityNodeID node) noexcept
  {
    auto *object = Object(node);
    if (!object)
    {
      return null;
    }

    pal::AccessibilityNode result;
    result.Role = ToPalRole(*object);
    result.AriaRole = utf8_string(object->ComputedRoleString());
    result.Name = object->AccessibleName();
    result.Description = object->AccessibleDescription();
    bool isPassword = IsPassword(*object);
    if (!isPassword)
    {
      auto valueDescription = object->ValueDescription();
      result.Value = valueDescription.empty() ? object->StringValue() : valueDescription;
    }
    result.Placeholder = object->GetAttribute(u8"aria-placeholder");
    if (result.Placeholder.empty())
    {
      result.Placeholder = object->GetAttribute(u8"placeholder");
    }
    result.KeyboardShortcut = object->GetAttribute(u8"aria-keyshortcuts");

    auto rect = object->ElementRect();
    result.Bounds = ToWindow(*object, rect.X().ToFloat(), rect.Y().ToFloat(), rect.Width().ToFloat(),
                             rect.Height().ToFloat());

    using State = pal::AccessibilityState;
    auto &states = result.States;
    auto setState = [&](State state, bool on)
    {
      if (on)
      {
        states.Add(state);
      }
    };
    bool isTextControl = object->IsTextControl() || object->IsNonNativeTextControl();
    setState(State::Focusable, object->CanSetFocusAttribute());
    setState(State::Focused, object->IsFocused());
    setState(State::Disabled, !object->IsEnabled());
    setState(State::Checked, object->IsChecked() || object->IsPressed());
    setState(State::Mixed, object->IsIndeterminate());
    setState(State::Expandable, object->SupportsExpanded() || object->IsMenuList());
    setState(State::Expanded, object->IsExpanded() || (object->IsMenuList() && !object->IsCollapsed()));
    setState(State::Selectable, object->CanSetSelectedAttribute());
    setState(State::Selected, object->IsSelected());
    setState(State::MultiSelectable, object->IsMultiSelectable());
    setState(State::ReadOnly, (isTextControl || IsRangeControl(*object)) && !object->CanSetValueAttribute());
    setState(State::Required, object->IsRequired());
    auto invalid = object->GetAttributeTrimmed(u8"aria-invalid");
    setState(State::Invalid,
             !invalid.empty()
               && !boo::infra::StringAlgorithms::ASCIICaseInsensitiveMatch(invalid, u8"false"));
    setState(State::Offscreen, object->IsOffScreen());
    setState(State::Busy, object->IsBusy());
    setState(State::Protected, isPassword);
    setState(State::Multiline, object->Role() == AccessibilityRole::TextArea);
    setState(State::Visited, object->IsVisited());
    auto *cache = object->AxObjectCache();
    setState(State::Modal, cache && object->GetNode() && cache->ModalNode() == object->GetNode());
    setState(State::Editable, isTextControl && object->CanSetValueAttribute());

    using Action = pal::AccessibilityAction;
    auto &actions = result.Actions;
    if (object->ActionElement() || object->IsMenuList()
        || object->Role() == AccessibilityRole::SpinButtonPart)
    {
      actions.Add(Action::Press);
    }
    if (object->CanSetFocusAttribute())
    {
      actions.Add(Action::Focus);
    }
    if (object->Role() == AccessibilityRole::Slider || object->Role() == AccessibilityRole::SpinButton)
    {
      actions.Add(Action::Increment);
      actions.Add(Action::Decrement);
    }
    if (states.Contains(State::Expandable))
    {
      actions.Add(Action::Expand);
      actions.Add(Action::Collapse);
    }
    if (object->CanSetSelectedAttribute())
    {
      actions.Add(Action::Select);
      actions.Add(Action::Deselect);
    }
    actions.Add(Action::ScrollIntoView);
    if (object->CanSetValueAttribute())
    {
      actions.Add(Action::SetValue);
    }

    if (IsRangeControl(*object))
    {
      auto step = object->GetAttributeTrimmed(u8"step");
      auto parsedStep = boo::html::Numbers::ParseFloatingPoint(step);
      result.Range = pal::AccessibilityRange {object->ValueForRange(), object->MinValueForRange(),
                                              object->MaxValueForRange(),
                                              !step.empty() && parsedStep.Success() ? parsedStep.Value : 0.0};
    }
    result.HeadingLevel = object->HeadingLevel();
    result.Level = UnsignedAttribute(*object, u8"aria-level");
    result.PositionInSet = UnsignedAttribute(*object, u8"aria-posinset");
    result.SizeOfSet = UnsignedAttribute(*object, u8"aria-setsize");
    auto liveStatus = object->LiveRegionStatus();
    result.LiveSetting = boo::infra::StringAlgorithms::ASCIICaseInsensitiveMatch(liveStatus, u8"assertive")
                           ? pal::AccessibilityLiveSetting::Assertive
                           : (boo::infra::StringAlgorithms::ASCIICaseInsensitiveMatch(liveStatus, u8"polite")
                                ? pal::AccessibilityLiveSetting::Polite
                                : pal::AccessibilityLiveSetting::Off);

    if (object->IsExposableTable())
    {
      result.RowCount = object->RowCount();
      result.ColumnCount = object->ColumnCount();
    }
    if (object->IsTableCell())
    {
      auto [row, rowSpan] = object->RowIndexRange();
      auto [column, columnSpan] = object->ColumnIndexRange();
      result.RowIndex = row;
      result.RowSpan = rowSpan;
      result.ColumnIndex = column;
      result.ColumnSpan = columnSpan;
    }
    return result;
  }

  pal::AccessibilityNodeID ViewAccessibility::Parent(pal::AccessibilityNodeID node) noexcept
  {
    auto *object = Object(node);
    return ID(object ? object->ParentObjectUnignored() : nullptr);
  }

  List<pal::AccessibilityNodeID> ViewAccessibility::Children(pal::AccessibilityNodeID node) noexcept
  {
    List<pal::AccessibilityNodeID> children;
    if (auto *object = Object(node))
    {
      for (const auto &child : object->Children())
      {
        children.push_back(ID(child.get()));
      }
    }
    return children;
  }

  pal::AccessibilityNodeID ViewAccessibility::Focus() noexcept
  {
    auto *cache = Cache();
    return ID(cache ? cache->FocusedObject() : nullptr);
  }

  pal::AccessibilityNodeID ViewAccessibility::HitTest(float x, float y) noexcept
  {
    auto *cache = Cache();
    auto *root = cache ? cache->RootObject() : nullptr;
    if (!root)
    {
      return 0;
    }
    boo::IntPoint viewPoint(static_cast<int32>(std::floor(x)) - _origin.X(),
                            static_cast<int32>(std::floor(y)) - _origin.Y());
    // The view's scroll bars are in its coordinates; the page is in the document's.
    for (const auto &child : root->Children())
    {
      if (child->Role() == AccessibilityRole::ScrollBar
          && child->ElementRect().Contains(boo::LayoutPoint(viewPoint)))
      {
        return ID(child.get());
      }
    }
    auto *webArea =
      root->IsScrollView() ? DynamicDowncast<boo::ax::AccessibilityScrollView>(*root)->WebAreaObject() : root;
    if (!webArea)
    {
      return 0;
    }
    auto scroll = _view.ScrollPosition();
    auto *hit =
      webArea->AccessibilityHitTest(boo::IntPoint(viewPoint.X() + scroll.X(), viewPoint.Y() + scroll.Y()));
    return ID(hit ? hit : webArea);
  }

  List<pal::AccessibilityNodeID> ViewAccessibility::SelectedChildren(pal::AccessibilityNodeID node) noexcept
  {
    List<pal::AccessibilityNodeID> selected;
    if (auto *object = Object(node))
    {
      for (const auto &child : object->SelectedChildren())
      {
        selected.push_back(ID(child.get()));
      }
    }
    return selected;
  }

  bool ViewAccessibility::PerformAction(pal::AccessibilityNodeID node,
                                        pal::AccessibilityAction action) noexcept
  {
    RefPtr<AccessibilityObject> object = ShareRefPtr(Object(node));
    if (!object)
    {
      return false;
    }
    using Action = pal::AccessibilityAction;
    switch (action)
    {
      case Action::Press: return object->Press();
      case Action::Focus:
        if (!object->CanSetFocusAttribute())
        {
          return false;
        }
        object->SetFocused(true);
        return true;
      case Action::Increment: object->Increment(); return true;
      case Action::Decrement: object->Decrement(); return true;
      case Action::Expand:
      case Action::Collapse:
      {
        bool expand = action == Action::Expand;
        // A menu list opens and closes its pop-up when pressed, as an ARIA disclosure does.
        if (object->IsMenuList())
        {
          return object->IsCollapsed() == expand && object->Press();
        }
        if (object->GetElement() && object->GetElement()->LocalName() == u8"details")
        {
          object->SetIsExpanded(expand);
          return true;
        }
        return object->SupportsExpanded() && object->IsExpanded() != expand && object->Press();
      }
      case Action::Select:
      case Action::Deselect:
        if (!object->CanSetSelectedAttribute())
        {
          return false;
        }
        object->SetSelected(action == Action::Select);
        return true;
      case Action::ScrollIntoView: object->ScrollToMakeVisible(); return true;
      case Action::SetValue:       return false;
    }
    return false;
  }

  bool ViewAccessibility::SetValue(pal::AccessibilityNodeID node, utf8_stringview value) noexcept
  {
    RefPtr<AccessibilityObject> object = ShareRefPtr(Object(node));
    return object && object->CanSetValueAttribute() && object->SetValue(boo::dom::DOMStringView(value));
  }

  bool ViewAccessibility::SetRangeValue(pal::AccessibilityNodeID node, double value) noexcept
  {
    RefPtr<AccessibilityObject> object = ShareRefPtr(Object(node));
    return object && object->CanSetValueAttribute() && object->SetValue(static_cast<float>(value));
  }

  utf8_string ViewAccessibility::Text(pal::AccessibilityNodeID node) noexcept
  {
    auto *object = Object(node);
    if (!object || IsPassword(*object))
    {
      return {};
    }
    auto text = object->Text();
    if (text.empty() && !object->IsTextControl())
    {
      text = AccessibilityObject::StringForVisiblePositionRange(object->GetVisiblePositionRange());
    }
    return text;
  }

  Maybe<pal::AccessibilityTextRange> ViewAccessibility::TextSelection(pal::AccessibilityNodeID node) noexcept
  {
    auto *object = Object(node);
    if (!object || !(object->IsTextControl() || object->IsNonNativeTextControl()))
    {
      return null;
    }
    // Only the selection in the object counts.
    auto selection = object->SelectedVisiblePositionRange();
    if (selection.IsNull() || (!object->IsTextControl() && object->Index(selection.Start) < 0))
    {
      return null;
    }
    auto range = object->SelectedTextRange();
    return pal::AccessibilityTextRange {range.Location, range.Length};
  }

  bool ViewAccessibility::SetTextSelection(pal::AccessibilityNodeID node,
                                           pal::AccessibilityTextRange range) noexcept
  {
    RefPtr<AccessibilityObject> object = ShareRefPtr(Object(node));
    if (!object)
    {
      return false;
    }
    auto positions = object->VisiblePositionRangeForRange({range.Start, range.Length});
    if (positions.IsNull())
    {
      return false;
    }
    object->SetSelectedVisiblePositionRange(positions);
    return true;
  }

  List<pal::AccessibilityRect> ViewAccessibility::TextBounds(pal::AccessibilityNodeID node,
                                                             pal::AccessibilityTextRange range) noexcept
  {
    List<pal::AccessibilityRect> bounds;
    auto *object = Object(node);
    if (!object)
    {
      return bounds;
    }
    auto simpleRange =
      boo::MakeSimpleRange(object->VisiblePositionRangeForRange({range.Start, range.Length}));
    if (!simpleRange)
    {
      return bounds;
    }
    // The rects are in the viewport's coordinates, so only the view's origin moves them.
    for (const auto &rect : boo::RenderObject::ClientBorderAndTextRects(*simpleRange))
    {
      bounds.push_back({rect.X() + static_cast<float>(_origin.X()),
                        rect.Y() + static_cast<float>(_origin.Y()), rect.Width(), rect.Height()});
    }
    return bounds;
  }
}
