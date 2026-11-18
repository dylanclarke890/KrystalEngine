#include "Krystal.UI/ViewAccessibility.hpp"
#include "Krystal.Booey/Accessibility/AXNotifications.hpp"
#include "Krystal.Booey/Accessibility/AXObject.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Accessibility/AXScrollView.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleUnits.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascadeDescription.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.UI/View.hpp"
#include <cmath>

namespace krys::ui
{
  namespace
  {
    using boo::AXNotification;
    using boo::AXObject;
    using boo::AXRole;

    // Each page's cache numbers its objects from the same counter, so the nodes of other pages than the
    // view's (an overlay's, an SVG image's) have the top bit set and their cache's tag above their objects'
    // IDs.
    constexpr pal::AccessibilityNodeId ForeignNodeBit = 1ull << 63;
    constexpr uint32 ForeignTagShift = 40;
    constexpr pal::AccessibilityNodeId ForeignObjectIDMask = (1ull << ForeignTagShift) - 1;

    KRYS_NODISCARD pal::AccessibilityRole ToPalRole(const AXObject &object) noexcept
    {
      using Role = pal::AccessibilityRole;
      switch (object.Role())
      {
        case AXRole::WebArea:
        case AXRole::Document:
        case AXRole::DocumentArticle:
        case AXRole::GraphicsDocument:       return Role::Document;
        case AXRole::ScrollArea:             return Role::Pane;
        case AXRole::ApplicationDialog:
        case AXRole::ApplicationAlertDialog: return Role::Dialog;
        case AXRole::ApplicationAlert:       return Role::Alert;
        case AXRole::Button:
        case AXRole::PopUpButton:            return Role::Button;
        case AXRole::ToggleButton:
        case AXRole::Switch:                 return Role::ToggleButton;
        case AXRole::Checkbox:
        case AXRole::MenuItemCheckbox:
          return object.Role() == AXRole::Checkbox ? Role::CheckBox : Role::MenuItem;
        case AXRole::RadioButton:           return Role::RadioButton;
        case AXRole::ComboBox:              return Role::ComboBox;
        case AXRole::TextField:
        case AXRole::TextArea:
        case AXRole::SearchField:           return Role::Edit;
        case AXRole::Link:                  return Role::Link;
        case AXRole::Image:
        case AXRole::ImageMap:
        case AXRole::Canvas:
        case AXRole::GraphicsObject:
        case AXRole::GraphicsSymbol:
        case AXRole::SVGRoot:               return Role::Image;
        case AXRole::Heading:               return Role::Heading;
        case AXRole::Paragraph:             return Role::Paragraph;
        case AXRole::StaticText:
        case AXRole::Label:
        case AXRole::ListMarker:            return Role::Text;
        case AXRole::List:
        case AXRole::ListBox:
        case AXRole::DescriptionList:
        case AXRole::Directory:
        case AXRole::Feed:                  return Role::List;
        case AXRole::ListItem:
        case AXRole::ListBoxOption:
        case AXRole::MenuListOption:
        case AXRole::DescriptionListTerm:
        case AXRole::DescriptionListDetail: return Role::ListItem;
        case AXRole::Menu:
        case AXRole::MenuListPopup:         return Role::Menu;
        case AXRole::MenuBar:               return Role::MenuBar;
        case AXRole::MenuItem:
        case AXRole::MenuItemRadio:         return Role::MenuItem;
        case AXRole::ProgressIndicator:
        case AXRole::Meter:                 return Role::ProgressBar;
        case AXRole::Slider:                return Role::Slider;
        case AXRole::SpinButton:            return Role::SpinButton;
        case AXRole::SpinButtonPart:        return Role::Button;
        case AXRole::ScrollBar:             return Role::ScrollBar;
        case AXRole::TabList:               return Role::TabList;
        case AXRole::Tab:                   return Role::Tab;
        case AXRole::TabPanel:              return Role::TabPanel;
        case AXRole::Table:
        case AXRole::Grid:
        case AXRole::TreeGrid:              return Role::Table;
        case AXRole::Row:                   return Role::Row;
        case AXRole::Cell:
        case AXRole::GridCell:              return Role::Cell;
        case AXRole::ColumnHeader:          return Role::ColumnHeader;
        case AXRole::RowHeader:             return Role::RowHeader;
        case AXRole::Tree:                  return Role::Tree;
        case AXRole::TreeItem:              return Role::TreeItem;
        case AXRole::HorizontalRule:
        case AXRole::Splitter:              return Role::Separator;
        case AXRole::Toolbar:               return Role::ToolBar;
        case AXRole::UserInterfaceTooltip:  return Role::ToolTip;
        case AXRole::ApplicationStatus:     return Role::StatusBar;
        case AXRole::LandmarkBanner:
        case AXRole::LandmarkComplementary:
        case AXRole::LandmarkContentInfo:
        case AXRole::LandmarkDocRegion:
        case AXRole::LandmarkMain:
        case AXRole::LandmarkNavigation:
        case AXRole::LandmarkRegion:
        case AXRole::LandmarkSearch:
        case AXRole::Form:                  return Role::Landmark;
        case AXRole::Figure:                return Role::Figure;
        case AXRole::DocumentMath:          return Role::Math;
        case AXRole::Unknown:               return Role::Unknown;
        default:                            return Role::Group;
      }
    }

    KRYS_NODISCARD uint32 UnsignedAttribute(const AXObject &object, boo::DOMStringView name) noexcept
    {
      auto value = object.GetAttributeTrimmed(name);
      auto parsed = boo::html::Numbers::ParseNonNegativeInteger(value);
      return !value.empty() && parsed.Success() ? static_cast<uint32>(parsed.Value) : 0;
    }

    KRYS_NODISCARD bool IsPassword(const AXObject &object) noexcept
    {
      auto *input = DynamicDowncast<boo::HTMLInputElement>(object.GetNode());
      return input && input->IsPasswordField();
    }

    KRYS_NODISCARD bool IsRangeControl(const AXObject &object) noexcept
    {
      switch (object.Role())
      {
        case AXRole::Slider:
        case AXRole::ProgressIndicator:
        case AXRole::Meter:
        case AXRole::ScrollBar:
        case AXRole::SpinButton:        return true;
        default:                        return false;
      }
    }

    /// @brief @p color as 0xAARRGGBB.
    KRYS_NODISCARD uint32 ToARGB(const boo::Color &color) noexcept
    {
      auto [r, g, b, a] = color.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
      return (static_cast<uint32>(a) << 24) | (static_cast<uint32>(r) << 16) | (static_cast<uint32>(g) << 8)
             | b;
    }

    /// @brief How the text @p renderer draws looks, for assistive technology.
    KRYS_NODISCARD pal::AccessibilityTextStyle TextStyleOf(const boo::RenderObject &renderer) noexcept
    {
      const auto &style = renderer.Style();
      const auto &font = style.GetFontDescription();
      pal::AccessibilityTextStyle result;
      // The face used, rather than the family asked for (which may be a generic family).
      result.FontName = style.GetFontCascade().PrimaryFont()->PlatformData().FamilyName();
      if (result.FontName.empty() && font.FamilyCount())
      {
        result.FontName = font.FirstFamily().ToString();
      }
      result.FontSize = style.ComputedFontSize();
      result.FontWeight = static_cast<uint16>(static_cast<float>(font.Weight()));
      result.Italic = boo::IsItalic(font.FontStyleSlope());
      auto decorations = style.TextDecorationLineInEffect();
      result.Underline = decorations.HasUnderline();
      result.Strikethrough = decorations.HasLineThrough();
      result.ForegroundColor = ToARGB(style.ColorResolvingCurrentColor(boo::css::PropertyId::Color));
      // The background is the nearest one drawn behind the text.
      for (auto *ancestor = &renderer; ancestor; ancestor = ancestor->Parent())
      {
        auto background = ancestor->Style().ColorResolvingCurrentColor(boo::css::PropertyId::BackgroundColor);
        if (background.IsVisible())
        {
          result.BackgroundColor = ToARGB(background);
          break;
        }
      }
      return result;
    }

    /// @brief The event Krystal.PAL has for @p notification, if it has one.
    KRYS_NODISCARD Maybe<pal::AccessibilityEvent> ToPalEvent(const AXObject &object,
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
          if (object.Role() == AXRole::ApplicationAlert || object.Role() == AXRole::ApplicationAlertDialog)
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

  View *ViewAccessibility::OverlayView() const noexcept
  {
    auto *overlay = _view.ShownOverlay();
    return overlay ? &overlay->ContentView() : nullptr;
  }

  boo::AXObjectCache *ViewAccessibility::OverlayCache() const noexcept
  {
    auto *view = OverlayView();
    return view && boo::AXObjectCache::AccessibilityEnabled() ? view->Document().AxObjectCache() : nullptr;
  }

  boo::AXObjectCache *ViewAccessibility::ForeignCache::Get() const noexcept
  {
    auto *document = Document.get();
    auto *cache = document ? document->ExistingAXObjectCache() : nullptr;
    return cache && cache == Cache ? cache : nullptr;
  }

  void ViewAccessibility::ForgetForeignCache(const boo::AXObjectCache &cache) noexcept
  {
    for (auto &slot : _foreignCaches)
    {
      if (slot.Cache == &cache)
      {
        slot = {};
      }
    }
  }

  pal::AccessibilityNodeId ViewAccessibility::ID(const AXObject *object) const noexcept
  {
    if (!object)
    {
      return 0;
    }
    auto id = static_cast<pal::AccessibilityNodeId>(object->ObjectID());
    auto *cache = object->AxObjectCache();
    if (!cache || cache == _view.Document().ExistingAXObjectCache())
    {
      return id;
    }
    krys_debug_assert(id <= ForeignObjectIDMask);
    // Another page's cache gets a tag the first time one of its objects is a node.
    size_t index = 0;
    while (index < _foreignCaches.size() && _foreignCaches[index].Get() != cache)
    {
      ++index;
    }
    if (index == _foreignCaches.size())
    {
      _foreignCaches.push_back({CreateWeakPtr(&cache->GetDocument()), cache});
    }
    return ForeignNodeBit | (static_cast<pal::AccessibilityNodeId>(index + 1) << ForeignTagShift) | id;
  }

  Pair<View *, boo::IntPoint> ViewAccessibility::ViewAndOrigin(const AXObject &object) const noexcept
  {
    // An object is in the overlay if its page, or the page of the image it's in, is the overlay's.
    auto *overlayCache = OverlayCache();
    auto *cache = _view.Document().ExistingAXObjectCache();
    for (const auto *ancestor = &object; overlayCache && ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->AxObjectCache() == overlayCache)
      {
        // The overlay is placed in the view's CSS pixels.
        auto location = _view.ShownOverlay()->Rect().Location();
        auto scale = _view.DeviceScaleFactor();
        return {OverlayView(),
                boo::IntPoint(_origin.X() + static_cast<int32>(std::lround(location.X() * scale)),
                              _origin.Y() + static_cast<int32>(std::lround(location.Y() * scale)))};
      }
      if (ancestor->AxObjectCache() == cache)
      {
        break;
      }
    }
    return {&_view, _origin};
  }

  void ViewAccessibility::OverlayShown(Overlay &overlay) noexcept
  {
    if (!_window || !_window->IsAccessibilityActive())
    {
      return;
    }
    // The overlay's page joins the tree as the root's last child.
    _window->RaiseAccessibilityEvent(Root(), pal::AccessibilityEvent::ChildrenChanged);
    auto *cache = OverlayCache();
    if (!cache)
    {
      return;
    }
    if (auto *root = cache->RootObject())
    {
      _window->RaiseAccessibilityEvent(ID(root), pal::AccessibilityEvent::MenuOpened);
    }
    if (overlay.IsModal())
    {
      if (auto *focused = cache->FocusedObject())
      {
        _window->RaiseAccessibilityEvent(ID(focused), pal::AccessibilityEvent::Focused);
      }
    }
  }

  void ViewAccessibility::OverlayHidden(Overlay &overlay) noexcept
  {
    // The overlay's nodes go with it (its page may outlive it, but isn't exposed; if it's shown again, its
    // nodes are new nodes).
    auto *cache = overlay.ContentView().Document().ExistingAXObjectCache();
    if (!_window || !_window->IsAccessibilityActive())
    {
      if (cache)
      {
        ForgetForeignCache(*cache);
      }
      return;
    }
    if (auto *root = cache ? cache->RootObject() : nullptr)
    {
      auto id = ID(root);
      _window->RaiseAccessibilityEvent(id, pal::AccessibilityEvent::MenuClosed);
      _window->AccessibilityNodeRemoved(id);
    }
    if (cache)
    {
      ForgetForeignCache(*cache);
    }
    _window->RaiseAccessibilityEvent(Root(), pal::AccessibilityEvent::ChildrenChanged);
    if (auto *focused = Cache() ? Cache()->FocusedObject() : nullptr)
    {
      _window->RaiseAccessibilityEvent(ID(focused), pal::AccessibilityEvent::Focused);
    }
  }

  boo::AXObjectCache *ViewAccessibility::Cache() noexcept
  {
    // Assistive technology asking for the tree is what turns accessibility on.
    if (!boo::AXObjectCache::AccessibilityEnabled())
    {
      boo::AXObjectCache::EnableAccessibility();
    }
    return _view.Document().AxObjectCache();
  }

  AXObject *ViewAccessibility::Object(pal::AccessibilityNodeId node) noexcept
  {
    auto *cache = Cache();
    if (!cache || !node)
    {
      return nullptr;
    }
    AXObject *object = nullptr;
    if (node & ForeignNodeBit)
    {
      auto tag = static_cast<size_t>((node & ~ForeignNodeBit) >> ForeignTagShift);
      auto *foreignCache = tag && tag <= _foreignCaches.size() ? _foreignCaches[tag - 1].Get() : nullptr;
      if (foreignCache)
      {
        object = foreignCache->ObjectForID(static_cast<boo::AXId>(node & ForeignObjectIDMask));
      }
    }
    else
    {
      object = cache->ObjectForID(static_cast<boo::AXId>(node));
    }
    return object && !object->IsDetached() ? object : nullptr;
  }

  pal::AccessibilityRect ViewAccessibility::ToWindow(const AXObject &object, float x, float y, float width,
                                                     float height) const noexcept
  {
    // The view and its scroll bars are in the view's coordinates; everything else is in the document's,
    // which the view is scrolled into.
    auto [view, origin] = ViewAndOrigin(object);
    if (!object.IsScrollView() && object.Role() != AXRole::ScrollBar)
    {
      auto scroll = view->ScrollPosition();
      x -= static_cast<float>(scroll.X());
      y -= static_cast<float>(scroll.Y());
    }
    // The page is in CSS pixels; the window, in the view's.
    auto scale = view->DeviceScaleFactor();
    return {x * scale + static_cast<float>(origin.X()), y * scale + static_cast<float>(origin.Y()),
            width * scale, height * scale};
  }

  void ViewAccessibility::NotificationPosted(AXObject &object, AXNotification notification) noexcept
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

  void ViewAccessibility::TextChanged(AXObject &object, boo::AXTextChange change, uint64 offset,
                                      utf8_stringview text) noexcept
  {
    if (_window && _window->IsAccessibilityActive())
    {
      _window->RaiseAccessibilityTextChange(ID(&object),
                                            change == boo::AXTextChange::Inserted
                                              ? pal::AccessibilityTextChange::Inserted
                                              : pal::AccessibilityTextChange::Deleted,
                                            offset, text);
    }
  }

  void ViewAccessibility::ObjectDetached(AXObject &object) noexcept
  {
    if (_window && _window->IsAccessibilityActive())
    {
      _window->AccessibilityNodeRemoved(ID(&object));
    }
  }

  pal::AccessibilityNodeId ViewAccessibility::Root() noexcept
  {
    auto *cache = Cache();
    return ID(cache ? cache->RootObject() : nullptr);
  }

  Maybe<pal::AccessibilityNode> ViewAccessibility::Node(pal::AccessibilityNodeId node) noexcept
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
    setState(State::Multiline, object->Role() == AXRole::TextArea);
    setState(State::Visited, object->IsVisited());
    auto *cache = object->AxObjectCache();
    setState(State::Modal, cache && object->GetNode() && cache->ModalNode() == object->GetNode());
    setState(State::Editable, isTextControl && object->CanSetValueAttribute());

    using Action = pal::AccessibilityAction;
    auto &actions = result.Actions;
    if (object->ActionElement() || object->IsMenuList() || object->Role() == AXRole::SpinButtonPart)
    {
      actions.Add(Action::Press);
    }
    if (object->CanSetFocusAttribute())
    {
      actions.Add(Action::Focus);
    }
    if (object->Role() == AXRole::Slider || object->Role() == AXRole::SpinButton)
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

  pal::AccessibilityNodeId ViewAccessibility::Parent(pal::AccessibilityNodeId node) noexcept
  {
    auto *object = Object(node);
    if (!object)
    {
      return 0;
    }
    // The overlay's page is the view's root's last child.
    if (auto *overlayCache = OverlayCache(); overlayCache && object == overlayCache->RootObject())
    {
      return Root();
    }
    return ID(object->ParentObjectUnignored());
  }

  List<pal::AccessibilityNodeId> ViewAccessibility::Children(pal::AccessibilityNodeId node) noexcept
  {
    List<pal::AccessibilityNodeId> children;
    if (auto *object = Object(node))
    {
      for (const auto &child : object->Children())
      {
        children.push_back(ID(child.get()));
      }
      if (auto *overlayCache = OverlayCache(); overlayCache && object == Cache()->RootObject())
      {
        if (auto *overlayRoot = overlayCache->RootObject())
        {
          children.push_back(ID(overlayRoot));
        }
      }
    }
    return children;
  }

  pal::AccessibilityNodeId ViewAccessibility::Focus() noexcept
  {
    // A modal overlay has the focus.
    if (auto *overlay = _view.ShownOverlay(); overlay && overlay->IsModal())
    {
      if (auto *overlayCache = OverlayCache())
      {
        if (auto *focused = overlayCache->FocusedObject())
        {
          return ID(focused);
        }
      }
    }
    auto *cache = Cache();
    return ID(cache ? cache->FocusedObject() : nullptr);
  }

  pal::AccessibilityNodeId ViewAccessibility::HitTest(float x, float y) noexcept
  {
    auto *cache = Cache();
    if (!cache)
    {
      return 0;
    }
    // The window is in the view's pixels; the page, in CSS pixels.
    auto scale = _view.DeviceScaleFactor();
    boo::IntPoint viewPoint(static_cast<int32>(std::floor((x - static_cast<float>(_origin.X())) / scale)),
                            static_cast<int32>(std::floor((y - static_cast<float>(_origin.Y())) / scale)));
    // The overlay is over the page.
    if (auto *overlay = _view.ShownOverlay(); overlay && overlay->Rect().Contains(viewPoint))
    {
      if (auto *overlayCache = OverlayCache())
      {
        auto location = overlay->Rect().Location();
        return ID(HitTestIn(*overlayCache, overlay->ContentView(),
                            boo::IntPoint(viewPoint.X() - location.X(), viewPoint.Y() - location.Y())));
      }
    }
    return ID(HitTestIn(*cache, _view, viewPoint));
  }

  AXObject *ViewAccessibility::HitTestIn(boo::AXObjectCache &cache, View &view,
                                         const boo::IntPoint &viewPoint) noexcept
  {
    auto *root = cache.RootObject();
    if (!root)
    {
      return nullptr;
    }
    // The view's scroll bars are in its coordinates; the page is in the document's.
    for (const auto &child : root->Children())
    {
      if (child->Role() == AXRole::ScrollBar && child->ElementRect().Contains(boo::LayoutPoint(viewPoint)))
      {
        return child.get();
      }
    }
    auto *webArea = root->IsScrollView() ? DynamicDowncast<boo::AXScrollView>(*root)->WebAreaObject() : root;
    if (!webArea)
    {
      return nullptr;
    }
    auto scroll = view.ScrollPosition();
    auto *hit =
      webArea->AccessibilityHitTest(boo::IntPoint(viewPoint.X() + scroll.X(), viewPoint.Y() + scroll.Y()));
    return hit ? hit : webArea;
  }

  List<pal::AccessibilityNodeId> ViewAccessibility::SelectedChildren(pal::AccessibilityNodeId node) noexcept
  {
    List<pal::AccessibilityNodeId> selected;
    if (auto *object = Object(node))
    {
      for (const auto &child : object->SelectedChildren())
      {
        selected.push_back(ID(child.get()));
      }
    }
    return selected;
  }

  bool ViewAccessibility::PerformAction(pal::AccessibilityNodeId node,
                                        pal::AccessibilityAction action) noexcept
  {
    RefPtr<AXObject> object = ShareRefPtr(Object(node));
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

  bool ViewAccessibility::SetValue(pal::AccessibilityNodeId node, utf8_stringview value) noexcept
  {
    RefPtr<AXObject> object = ShareRefPtr(Object(node));
    return object && object->CanSetValueAttribute() && object->SetValue(boo::DOMStringView(value));
  }

  bool ViewAccessibility::SetRangeValue(pal::AccessibilityNodeId node, double value) noexcept
  {
    RefPtr<AXObject> object = ShareRefPtr(Object(node));
    return object && object->CanSetValueAttribute() && object->SetValue(static_cast<float>(value));
  }

  utf8_string ViewAccessibility::Text(pal::AccessibilityNodeId node) noexcept
  {
    auto *object = Object(node);
    if (!object || IsPassword(*object))
    {
      return {};
    }
    auto text = object->Text();
    if (text.empty() && !object->IsTextControl())
    {
      text = AXObject::StringForVisiblePositionRange(object->GetVisiblePositionRange());
    }
    return text;
  }

  Maybe<pal::AccessibilityTextRange> ViewAccessibility::TextSelection(pal::AccessibilityNodeId node) noexcept
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

  bool ViewAccessibility::SetTextSelection(pal::AccessibilityNodeId node,
                                           pal::AccessibilityTextRange range) noexcept
  {
    RefPtr<AXObject> object = ShareRefPtr(Object(node));
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

  List<pal::AccessibilityRect> ViewAccessibility::TextBounds(pal::AccessibilityNodeId node,
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
    // The rects are in the viewport's CSS pixels, so only the view's scale and origin move them.
    auto [view, origin] = ViewAndOrigin(*object);
    auto scale = view->DeviceScaleFactor();
    for (const auto &rect : boo::RenderObject::ClientBorderAndTextRects(*simpleRange))
    {
      bounds.push_back({rect.X() * scale + static_cast<float>(origin.X()),
                        rect.Y() * scale + static_cast<float>(origin.Y()), rect.Width() * scale,
                        rect.Height() * scale});
    }
    return bounds;
  }

  List<uint64> ViewAccessibility::TextLineStarts(pal::AccessibilityNodeId node) noexcept
  {
    List<uint64> starts;
    auto *object = Object(node);
    if (!object || IsPassword(*object))
    {
      return starts;
    }
    auto range = object->GetVisiblePositionRange();
    if (range.IsNull())
    {
      return starts;
    }
    // Each line starts where moving down from the previous one's start lands, at its start.
    auto length = Text(node).size();
    auto lineStart = object->VisiblePositionForIndex(0);
    int32 previous = 0;
    while (true)
    {
      auto next = boo::StartOfLine(boo::NextLinePosition(lineStart, 0, boo::EditableType::ContentIsEditable));
      if (next.IsNull() || next == lineStart)
      {
        break;
      }
      auto index = object->IndexForVisiblePosition(next);
      if (index <= previous || static_cast<uint64>(index) >= length)
      {
        break;
      }
      starts.push_back(static_cast<uint64>(index));
      previous = index;
      lineStart = next;
    }
    return starts;
  }

  Maybe<pal::AccessibilityTextRun> ViewAccessibility::TextRunAt(pal::AccessibilityNodeId node,
                                                                uint64 offset) noexcept
  {
    auto *object = Object(node);
    auto text = object ? Text(node) : utf8_string {};
    if (text.empty())
    {
      return null;
    }
    offset = (std::min)(offset, static_cast<uint64>(text.size() - 1));
    // The character at the offset is the one before the position after it, in the text node it's in.
    auto position = object->VisiblePositionForIndex(static_cast<int32>(offset + 1)).DeepEquivalent();
    auto *container = position.GetContainerNode();
    const boo::RenderObject *renderer = nullptr;
    for (auto *ancestor = container; ancestor && !renderer; ancestor = ancestor->ParentNode())
    {
      renderer = ancestor->Renderer();
    }
    if (!renderer)
    {
      return null;
    }
    pal::AccessibilityTextRun run {offset, offset + 1, TextStyleOf(*renderer)};
    // A text node is drawn alike throughout.
    if (auto *textNode = DynamicDowncast<boo::Text>(container))
    {
      auto start =
        object->IndexForVisiblePosition(boo::VisiblePosition(boo::FirstPositionInOrBeforeNode(textNode)));
      auto end =
        object->IndexForVisiblePosition(boo::VisiblePosition(boo::LastPositionInOrAfterNode(textNode)));
      if (start >= 0 && static_cast<uint64>(start) <= offset)
      {
        run.Start = static_cast<uint64>(start);
      }
      if (end > 0 && static_cast<uint64>(end) > offset)
      {
        run.End = (std::min)(static_cast<uint64>(end), static_cast<uint64>(text.size()));
      }
    }
    return run;
  }
}
