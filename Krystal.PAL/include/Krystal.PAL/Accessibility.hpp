#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::pal
{
  /// @brief A node of a window's accessibility tree, as its host numbers them; 0 is no node. A node keeps its
  /// ID while it exists, and an ID isn't reused for another node.
  using AccessibilityNodeID = uint64;

  /// @brief What a node is, as assistive technology presents it.
  enum class AccessibilityRole : uint8
  {
    Unknown,
    /// @brief A page or other document.
    Document,
    /// @brief A container with no meaning of its own.
    Group,
    /// @brief A scrollable view.
    Pane,
    Dialog,
    Alert,
    Button,
    ToggleButton,
    CheckBox,
    RadioButton,
    ComboBox,
    /// @brief An editable text field or area.
    Edit,
    Link,
    Image,
    Heading,
    Paragraph,
    /// @brief Text with no other role.
    Text,
    List,
    ListItem,
    Menu,
    MenuBar,
    MenuItem,
    ProgressBar,
    Slider,
    SpinButton,
    ScrollBar,
    TabList,
    Tab,
    TabPanel,
    Table,
    Row,
    Cell,
    ColumnHeader,
    RowHeader,
    Tree,
    TreeItem,
    Separator,
    ToolBar,
    ToolTip,
    StatusBar,
    /// @brief A region of the page (`main`, `nav` and the other landmarks); `AccessibilityNode::AriaRole`
    /// says which.
    Landmark,
    Figure,
    Math,
  };

  enum class AccessibilityState : uint8
  {
    Focusable,
    Focused,
    Disabled,
    /// @brief A check box, radio button or toggle button that's checked (or pressed).
    Checked,
    /// @brief A check box that's neither checked nor unchecked.
    Mixed,
    /// @brief Whether the node can be expanded and collapsed; `Expanded` says which it is.
    Expandable,
    Expanded,
    Selectable,
    Selected,
    MultiSelectable,
    ReadOnly,
    Required,
    Invalid,
    /// @brief None of the node is in view.
    Offscreen,
    /// @brief Being updated: its changes aren't announced yet.
    Busy,
    /// @brief A password field, whose text isn't exposed.
    Protected,
    Multiline,
    Visited,
    Modal,
    Editable,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::pal::AccessibilityState, 20uz);

namespace krys::pal
{
  enum class AccessibilityAction : uint8
  {
    /// @brief Activates the node, as a click would.
    Press,
    Focus,
    Increment,
    Decrement,
    Expand,
    Collapse,
    Select,
    Deselect,
    ScrollIntoView,
    /// @brief Sets the value with `IAccessibilityTree::SetValue` or `SetRangeValue`.
    SetValue,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::pal::AccessibilityAction, 10uz);

namespace krys::pal
{
  /// @brief How a live region announces its changes.
  enum class AccessibilityLiveSetting : uint8
  {
    Off,
    Polite,
    Assertive,
  };

  /// @brief A range control's value.
  struct AccessibilityRange
  {
    double Value {0};
    double Minimum {0};
    double Maximum {0};
    /// @brief The step it changes by, or 0 if it doesn't say.
    double Step {0};
  };

  /// @brief A rectangle in the window's client area, in pixels.
  struct AccessibilityRect
  {
    float X {0};
    float Y {0};
    float Width {0};
    float Height {0};
  };

  /// @brief What assistive technology knows of a node.
  struct AccessibilityNode
  {
    AccessibilityRole Role {AccessibilityRole::Unknown};
    /// @brief The node's ARIA role (or the one its HTML element maps to), as ARIA names it; empty if none.
    utf8_string AriaRole;
    utf8_string Name;
    utf8_string Description;
    /// @brief The node's value as text: a text field's text (not a password's), a list's chosen option, a
    /// range's value text.
    utf8_string Value;
    /// @brief A placeholder or other hint at what goes in a field.
    utf8_string Placeholder;
    /// @brief The keys that activate it (`aria-keyshortcuts`).
    utf8_string KeyboardShortcut;
    AccessibilityRect Bounds;
    EnumSet<AccessibilityState> States;
    /// @brief The actions `IAccessibilityTree::PerformAction` takes for the node.
    EnumSet<AccessibilityAction> Actions;
    Maybe<AccessibilityRange> Range;
    /// @brief A heading's level (1 to 6), or 0.
    uint32 HeadingLevel {0};
    /// @brief The node's level in a tree or list of lists, or 0.
    uint32 Level {0};
    /// @brief Its position among its siblings in a list, menu, tree or tab list (from 1), and how many there
    /// are; 0 if it isn't in one.
    uint32 PositionInSet {0};
    uint32 SizeOfSet {0};
    AccessibilityLiveSetting LiveSetting {AccessibilityLiveSetting::Off};
    /// @brief A table's or cell's rows and columns (a cell's from its start).
    uint32 RowIndex {0};
    uint32 ColumnIndex {0};
    uint32 RowCount {0};
    uint32 ColumnCount {0};
    uint32 RowSpan {1};
    uint32 ColumnSpan {1};
  };

  /// @brief A range of a node's text, in bytes of its UTF-8 text.
  struct AccessibilityTextRange
  {
    uint64 Start {0};
    uint64 Length {0};
  };

  /// @brief What changed in a node, which the window tells assistive technology.
  enum class AccessibilityEvent : uint8
  {
    /// @brief The node took the focus.
    Focused,
    /// @brief The node's children changed.
    ChildrenChanged,
    NameChanged,
    DescriptionChanged,
    ValueChanged,
    /// @brief A check box's, radio button's or toggle button's state changed.
    CheckedChanged,
    ExpandedChanged,
    /// @brief An option, tab or other item was selected (or deselected).
    SelectedChanged,
    /// @brief Which of a list's (or other container's) items are selected changed.
    SelectionChanged,
    /// @brief The node was enabled or disabled.
    EnabledChanged,
    /// @brief Another state changed (required, invalid, busy, read-only...).
    StateChanged,
    /// @brief The selected text (or the caret) moved in the node.
    TextSelectionChanged,
    /// @brief A live region's contents changed.
    LiveRegionChanged,
    /// @brief An alert appeared.
    Alert,
    MenuOpened,
    MenuClosed,
    /// @brief A document finished loading.
    DocumentLoaded,
    /// @brief The node's bounds changed (it scrolled, or the layout changed).
    BoundsChanged,
  };

  /// @brief Whether text was inserted or deleted.
  enum class AccessibilityTextChange : uint8
  {
    Inserted,
    Deleted,
  };

  /// @brief The tree a window exposes to assistive technology, which the host implements. A window asks
  /// for it only when assistive technology does, so a host can build its tree then.
  class IAccessibilityTree
  {
  public:
    virtual ~IAccessibilityTree() noexcept = default;

    /// @brief The node at the top of the tree (the window's content); 0 if there's nothing to expose.
    KRYS_NODISCARD virtual AccessibilityNodeID Root() noexcept = 0;
    /// @brief What's known of @p node; null if it's gone.
    KRYS_NODISCARD virtual Maybe<AccessibilityNode> Node(AccessibilityNodeID node) noexcept = 0;
    /// @brief The node above @p node; 0 for the root, or if it's gone.
    KRYS_NODISCARD virtual AccessibilityNodeID Parent(AccessibilityNodeID node) noexcept = 0;
    /// @brief The nodes below @p node, in order.
    KRYS_NODISCARD virtual List<AccessibilityNodeID> Children(AccessibilityNodeID node) noexcept = 0;
    /// @brief The node with the focus; 0 if none.
    KRYS_NODISCARD virtual AccessibilityNodeID Focus() noexcept = 0;
    /// @brief The deepest node at (@p x, @p y) in the window's client area; 0 if none.
    KRYS_NODISCARD virtual AccessibilityNodeID HitTest(float x, float y) noexcept = 0;
    /// @brief The selected items of a list, tree, grid or tab list.
    KRYS_NODISCARD virtual List<AccessibilityNodeID> SelectedChildren(AccessibilityNodeID node) noexcept = 0;

    /// @brief Takes @p action for @p node; whether it was taken.
    virtual bool PerformAction(AccessibilityNodeID node, AccessibilityAction action) noexcept = 0;
    /// @brief Sets a text field's text (as typing would, if it's focused); whether it was set.
    virtual bool SetValue(AccessibilityNodeID node, utf8_stringview value) noexcept = 0;
    /// @brief Sets a range control's value; whether it was set.
    virtual bool SetRangeValue(AccessibilityNodeID node, double value) noexcept = 0;

    /// @brief @p node's text (a text field's, or the text under it); empty for a password.
    KRYS_NODISCARD virtual utf8_string Text(AccessibilityNodeID node) noexcept = 0;
    /// @brief The selected part of @p node's text (empty at the caret), or null if the selection isn't in it.
    KRYS_NODISCARD virtual Maybe<AccessibilityTextRange> TextSelection(AccessibilityNodeID node) noexcept = 0;
    /// @brief Selects @p range of @p node's text; whether it was selected.
    virtual bool SetTextSelection(AccessibilityNodeID node, AccessibilityTextRange range) noexcept = 0;
    /// @brief The bounds of each line of @p range of @p node's text, in the window's client area.
    KRYS_NODISCARD virtual List<AccessibilityRect> TextBounds(AccessibilityNodeID node,
                                                              AccessibilityTextRange range) noexcept = 0;
    /// @brief Where each line of @p node's text after the first starts, as ascending offsets in its text, as
    /// it's laid out (wrapped lines included).
    KRYS_NODISCARD virtual List<uint64> TextLineStarts(AccessibilityNodeID node) noexcept = 0;
  };
}
