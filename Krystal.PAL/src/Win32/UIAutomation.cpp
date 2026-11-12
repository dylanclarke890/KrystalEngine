#include "Krystal.PAL/Win32/UIAutomation.hpp"
#include "Krystal.Core/Utils/Move.hpp"

#include <ole2.h>
#include <uiautomation.h>

#include <algorithm>
#include <limits>
#include <string>

#pragma comment(lib, "Uiautomationcore.lib")

namespace krys::pal::win32
{
  namespace
  {
    KRYS_NODISCARD std::wstring Widen(utf8_stringview text) noexcept
    {
      if (text.empty())
      {
        return {};
      }
      auto source = reinterpret_cast<const char *>(text.data());
      int size = ::MultiByteToWideChar(CP_UTF8, 0, source, static_cast<int>(text.size()), nullptr, 0);
      std::wstring wide(static_cast<size_t>(size), L'\0');
      ::MultiByteToWideChar(CP_UTF8, 0, source, static_cast<int>(text.size()), wide.data(), size);
      return wide;
    }

    KRYS_NODISCARD utf8_string Narrow(const wchar_t *text) noexcept
    {
      if (!text || !*text)
      {
        return {};
      }
      int size = ::WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
      std::string narrow(static_cast<size_t>(size), '\0');
      ::WideCharToMultiByte(CP_UTF8, 0, text, -1, narrow.data(), size, nullptr, nullptr);
      // Without the terminator WideCharToMultiByte counted.
      narrow.pop_back();
      return utf8_string(reinterpret_cast<const char8_t *>(narrow.data()), narrow.size());
    }

    KRYS_NODISCARD BSTR ToBSTR(utf8_stringview text) noexcept
    {
      auto wide = Widen(text);
      return ::SysAllocStringLen(wide.data(), static_cast<UINT>(wide.size()));
    }

    /// @brief The number of bytes the UTF-8 sequence starting with @p lead takes.
    KRYS_NODISCARD uint32 SequenceLength(char8_t lead) noexcept
    {
      if (lead < 0x80)
      {
        return 1;
      }
      if ((lead & 0xE0) == 0xC0)
      {
        return 2;
      }
      if ((lead & 0xF0) == 0xE0)
      {
        return 3;
      }
      return (lead & 0xF8) == 0xF0 ? 4 : 1;
    }

    /// @brief The UTF-16 offset of the byte offset @p offset in @p text.
    KRYS_NODISCARD int32 ToUTF16Offset(utf8_stringview text, uint64 offset) noexcept
    {
      int32 units = 0;
      for (size_t i = 0; i < text.size() && i < offset;)
      {
        auto length = SequenceLength(text[i]);
        units += length == 4 ? 2 : 1;
        i += length;
      }
      return units;
    }

    /// @brief The byte offset of the UTF-16 offset @p offset in @p text.
    KRYS_NODISCARD uint64 ToUTF8Offset(utf8_stringview text, int32 offset) noexcept
    {
      int32 units = 0;
      size_t i = 0;
      while (i < text.size() && units < offset)
      {
        auto length = SequenceLength(text[i]);
        units += length == 4 ? 2 : 1;
        i += length;
      }
      return (std::min<uint64>)(i, text.size());
    }

    KRYS_NODISCARD POINT ClientOrigin(HWND window) noexcept
    {
      POINT origin {0, 0};
      ::ClientToScreen(window, &origin);
      return origin;
    }

    KRYS_NODISCARD UiaRect ToScreen(HWND window, const AccessibilityRect &rect) noexcept
    {
      auto origin = ClientOrigin(window);
      return {static_cast<double>(origin.x) + rect.X, static_cast<double>(origin.y) + rect.Y, rect.Width,
              rect.Height};
    }

    KRYS_NODISCARD long ToControlType(AccessibilityRole role) noexcept
    {
      using Role = AccessibilityRole;
      switch (role)
      {
        case Role::Document:     return UIA_DocumentControlTypeId;
        case Role::Pane:
        case Role::Dialog:
        case Role::TabPanel:     return UIA_PaneControlTypeId;
        case Role::Button:
        case Role::ToggleButton: return UIA_ButtonControlTypeId;
        case Role::CheckBox:     return UIA_CheckBoxControlTypeId;
        case Role::RadioButton:  return UIA_RadioButtonControlTypeId;
        case Role::ComboBox:     return UIA_ComboBoxControlTypeId;
        case Role::Edit:         return UIA_EditControlTypeId;
        case Role::Link:         return UIA_HyperlinkControlTypeId;
        case Role::Image:        return UIA_ImageControlTypeId;
        case Role::Heading:
        case Role::Text:         return UIA_TextControlTypeId;
        case Role::List:         return UIA_ListControlTypeId;
        case Role::ListItem:     return UIA_ListItemControlTypeId;
        case Role::Menu:         return UIA_MenuControlTypeId;
        case Role::MenuBar:      return UIA_MenuBarControlTypeId;
        case Role::MenuItem:     return UIA_MenuItemControlTypeId;
        case Role::ProgressBar:  return UIA_ProgressBarControlTypeId;
        case Role::Slider:       return UIA_SliderControlTypeId;
        case Role::SpinButton:   return UIA_SpinnerControlTypeId;
        case Role::ScrollBar:    return UIA_ScrollBarControlTypeId;
        case Role::TabList:      return UIA_TabControlTypeId;
        case Role::Tab:          return UIA_TabItemControlTypeId;
        case Role::Table:        return UIA_TableControlTypeId;
        case Role::Row:
        case Role::Cell:         return UIA_DataItemControlTypeId;
        case Role::ColumnHeader:
        case Role::RowHeader:    return UIA_HeaderItemControlTypeId;
        case Role::Tree:         return UIA_TreeControlTypeId;
        case Role::TreeItem:     return UIA_TreeItemControlTypeId;
        case Role::Separator:    return UIA_SeparatorControlTypeId;
        case Role::ToolBar:      return UIA_ToolBarControlTypeId;
        case Role::ToolTip:      return UIA_ToolTipControlTypeId;
        case Role::StatusBar:    return UIA_StatusBarControlTypeId;
        case Role::Unknown:
        case Role::Group:
        case Role::Alert:
        case Role::Paragraph:
        case Role::Landmark:
        case Role::Figure:
        case Role::Math:         return UIA_GroupControlTypeId;
      }
      return UIA_GroupControlTypeId;
    }

    KRYS_NODISCARD bool IsSelectionContainer(const AccessibilityNode &node) noexcept
    {
      using Role = AccessibilityRole;
      switch (node.Role)
      {
        case Role::List:
        case Role::Tree:
        case Role::TabList:
        case Role::Table:
        case Role::Menu:    return true;
        default:            return node.States.Contains(AccessibilityState::MultiSelectable);
      }
    }

    KRYS_NODISCARD bool Supports(PATTERNID pattern, const AccessibilityNode &node) noexcept
    {
      using Role = AccessibilityRole;
      bool isToggle = node.Role == Role::CheckBox || node.Role == Role::ToggleButton;
      switch (pattern)
      {
        case UIA_InvokePatternId:
          return node.Actions.Contains(AccessibilityAction::Press) && !isToggle
                 && node.Role != Role::RadioButton && node.Role != Role::ComboBox;
        case UIA_TogglePatternId: return isToggle;
        case UIA_ValuePatternId:
          return node.Role == Role::Edit || node.Role == Role::ComboBox
                 || (node.Actions.Contains(AccessibilityAction::SetValue) && !node.Range);
        case UIA_RangeValuePatternId:     return node.Range.has_value();
        case UIA_ExpandCollapsePatternId: return node.States.Contains(AccessibilityState::Expandable);
        case UIA_SelectionItemPatternId:
          return node.States.Contains(AccessibilityState::Selectable) || node.Role == Role::RadioButton;
        case UIA_SelectionPatternId:  return IsSelectionContainer(node);
        case UIA_ScrollItemPatternId: return node.Actions.Contains(AccessibilityAction::ScrollIntoView);
        case UIA_TextPatternId:       return node.Role == Role::Edit || node.Role == Role::Document;
        default:                      return false;
      }
    }

    void SetBool(VARIANT &value, bool on) noexcept
    {
      value.vt = VT_BOOL;
      value.boolVal = on ? VARIANT_TRUE : VARIANT_FALSE;
    }

    void SetInt(VARIANT &value, long number) noexcept
    {
      value.vt = VT_I4;
      value.lVal = number;
    }

    void SetString(VARIANT &value, utf8_stringview text) noexcept
    {
      value.vt = VT_BSTR;
      value.bstrVal = ToBSTR(text);
    }

    // Text units, for the text pattern. Word boundaries are where a run of letters and digits, or a mark,
    // starts; line boundaries are the paragraphs'.

    KRYS_NODISCARD int32 CharacterClass(wchar_t character) noexcept
    {
      if (::iswspace(character))
      {
        return 0;
      }
      return ::iswalnum(character) ? 1 : 2;
    }

    KRYS_NODISCARD bool IsBoundary(const std::wstring &text, int32 position, TextUnit unit) noexcept
    {
      auto length = static_cast<int32>(text.size());
      if (position <= 0 || position >= length)
      {
        return true;
      }
      switch (unit)
      {
        case TextUnit_Character: return true;
        case TextUnit_Word:
        {
          auto current = CharacterClass(text[static_cast<size_t>(position)]);
          auto previous = CharacterClass(text[static_cast<size_t>(position) - 1]);
          return current != 0 && (current != previous || current == 2);
        }
        case TextUnit_Line:
        case TextUnit_Paragraph: return text[static_cast<size_t>(position) - 1] == L'\n';
        default:                 return false;
      }
    }

    KRYS_NODISCARD int32 NextBoundary(const std::wstring &text, int32 position, TextUnit unit) noexcept
    {
      auto length = static_cast<int32>(text.size());
      for (int32 next = position + 1; next < length; ++next)
      {
        if (IsBoundary(text, next, unit))
        {
          return next;
        }
      }
      return length;
    }

    KRYS_NODISCARD int32 PreviousBoundary(const std::wstring &text, int32 position, TextUnit unit) noexcept
    {
      for (int32 previous = position - 1; previous > 0; --previous)
      {
        if (IsBoundary(text, previous, unit))
        {
          return previous;
        }
      }
      return 0;
    }

    /// @brief Identifies Krystal's text ranges, so a range passed back can be read.
    // {6E3A1F2C-8B4D-4C1E-9F27-3D5A0B7C9E11}
    constexpr GUID IID_KrystalTextRange = {
      0x6e'3a'1f'2c, 0x8b4d, 0x4c1e, {0x9f, 0x27, 0x3d, 0x5a, 0x0b, 0x7c, 0x9e, 0x11}};
  }

  /// @brief A node's (or the window's) element, with the patterns its node supports.
  class UIAElement final : public IRawElementProviderSimple,
                           public IRawElementProviderFragment,
                           public IRawElementProviderFragmentRoot,
                           public IInvokeProvider,
                           public IToggleProvider,
                           public IValueProvider,
                           public IRangeValueProvider,
                           public IExpandCollapseProvider,
                           public ISelectionItemProvider,
                           public ISelectionProvider,
                           public IScrollItemProvider
  {
    KRYS_NON_COPY_MOVABLE(UIAElement);

  private:
    ULONG _refCount {1};
    /// @brief Null once disconnected.
    UIAutomation *_owner {nullptr};
    /// @brief 0 for the window's element.
    AccessibilityNodeID _node {0};

  public:
    UIAElement(UIAutomation &owner, AccessibilityNodeID node) noexcept : _owner(&owner), _node(node)
    {
    }
    virtual ~UIAElement() noexcept = default;

    void Disconnect() noexcept
    {
      _owner = nullptr;
    }

    KRYS_NODISCARD AccessibilityNodeID Node() const noexcept
    {
      return _node;
    }

    KRYS_NODISCARD UIAutomation *Owner() const noexcept
    {
      return _owner;
    }

    /// @brief What's known of the node; null if it's gone or the element is the window's.
    KRYS_NODISCARD Maybe<AccessibilityNode> Info() const noexcept
    {
      if (!_owner || !_node)
      {
        return null;
      }
      return _owner->Tree().Node(_node);
    }

    void RuntimeID(int (&id)[3]) const noexcept
    {
      id[0] = UiaAppendRuntimeId;
      id[1] = static_cast<int>(_node & 0xFF'FF'FF'FFu);
      id[2] = static_cast<int>(_node >> 32);
    }

    /// @brief The value of @p property (a pattern's too), for its change events.
    void CurrentValue(PROPERTYID property, VARIANT &value) noexcept
    {
      ::VariantInit(&value);
      auto info = Info();
      if (!info)
      {
        return;
      }
      switch (property)
      {
        case UIA_ValueValuePropertyId: SetString(value, info->Value); return;
        case UIA_RangeValueValuePropertyId:
          value.vt = VT_R8;
          value.dblVal = info->Range ? info->Range->Value : 0;
          return;
        case UIA_ToggleToggleStatePropertyId: SetInt(value, ToggleState(*info)); return;
        case UIA_ExpandCollapseExpandCollapseStatePropertyId:
          SetInt(value, ExpandCollapseState(*info));
          return;
        case UIA_SelectionItemIsSelectedPropertyId: SetBool(value, IsSelected(*info)); return;
        case UIA_BoundingRectanglePropertyId:
        {
          auto rect = ToScreen(_owner->Window(), info->Bounds);
          value.vt = VT_R8 | VT_ARRAY;
          value.parray = ::SafeArrayCreateVector(VT_R8, 0, 4);
          if (value.parray)
          {
            double values[] = {rect.left, rect.top, rect.width, rect.height};
            for (LONG i = 0; i < 4; ++i)
            {
              ::SafeArrayPutElement(value.parray, &i, &values[i]);
            }
          }
          return;
        }
        default: (void)GetPropertyValue(property, &value); return;
      }
    }

    // IUnknown.

    IFACEMETHODIMP QueryInterface(REFIID riid, void **object) noexcept override
    {
      if (!object)
      {
        return E_POINTER;
      }
      *object = nullptr;
      if (riid == __uuidof(IUnknown) || riid == __uuidof(IRawElementProviderSimple))
      {
        *object = static_cast<IRawElementProviderSimple *>(this);
      }
      else if (riid == __uuidof(IRawElementProviderFragment))
      {
        *object = static_cast<IRawElementProviderFragment *>(this);
      }
      else if (riid == __uuidof(IRawElementProviderFragmentRoot) && !_node)
      {
        *object = static_cast<IRawElementProviderFragmentRoot *>(this);
      }
      else if (riid == __uuidof(IInvokeProvider))
      {
        *object = static_cast<IInvokeProvider *>(this);
      }
      else if (riid == __uuidof(IToggleProvider))
      {
        *object = static_cast<IToggleProvider *>(this);
      }
      else if (riid == __uuidof(IValueProvider))
      {
        *object = static_cast<IValueProvider *>(this);
      }
      else if (riid == __uuidof(IRangeValueProvider))
      {
        *object = static_cast<IRangeValueProvider *>(this);
      }
      else if (riid == __uuidof(IExpandCollapseProvider))
      {
        *object = static_cast<IExpandCollapseProvider *>(this);
      }
      else if (riid == __uuidof(ISelectionItemProvider))
      {
        *object = static_cast<ISelectionItemProvider *>(this);
      }
      else if (riid == __uuidof(ISelectionProvider))
      {
        *object = static_cast<ISelectionProvider *>(this);
      }
      else if (riid == __uuidof(IScrollItemProvider))
      {
        *object = static_cast<IScrollItemProvider *>(this);
      }
      else
      {
        return E_NOINTERFACE;
      }
      AddRef();
      return S_OK;
    }

    IFACEMETHODIMP_(ULONG) AddRef() noexcept override
    {
      return ::InterlockedIncrement(&_refCount);
    }

    IFACEMETHODIMP_(ULONG) Release() noexcept override
    {
      auto count = ::InterlockedDecrement(&_refCount);
      if (!count)
      {
        delete this;
      }
      return count;
    }

    // IRawElementProviderSimple.

    IFACEMETHODIMP get_ProviderOptions(ProviderOptions *options) noexcept override
    {
      if (!options)
      {
        return E_POINTER;
      }
      // COM threading brings UI Automation's calls to the window's thread.
      *options =
        static_cast<ProviderOptions>(ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading);
      return S_OK;
    }

    IFACEMETHODIMP GetPatternProvider(PATTERNID pattern, IUnknown **provider) noexcept override;

    IFACEMETHODIMP GetPropertyValue(PROPERTYID property, VARIANT *value) noexcept override
    {
      if (!value)
      {
        return E_POINTER;
      }
      value->vt = VT_EMPTY;
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      if (!_node)
      {
        if (property == UIA_ProviderDescriptionPropertyId)
        {
          SetString(*value, u8"Krystal");
        }
        return S_OK;
      }
      auto info = Info();
      if (!info)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      using State = AccessibilityState;
      const auto &states = info->States;
      switch (property)
      {
        case UIA_ControlTypePropertyId: SetInt(*value, ToControlType(info->Role)); break;
        case UIA_NamePropertyId:        SetString(*value, info->Name); break;
        case UIA_FullDescriptionPropertyId:
          if (!info->Description.empty())
          {
            SetString(*value, info->Description);
          }
          break;
        case UIA_HelpTextPropertyId:
          if (!info->Placeholder.empty())
          {
            SetString(*value, info->Placeholder);
          }
          break;
        case UIA_AriaRolePropertyId:
          if (!info->AriaRole.empty())
          {
            SetString(*value, info->AriaRole);
          }
          break;
        case UIA_LocalizedControlTypePropertyId:
          // The control types UI Automation has no name of its own for.
          if (info->Role == AccessibilityRole::Alert || info->Role == AccessibilityRole::Dialog
              || info->Role == AccessibilityRole::Figure || info->Role == AccessibilityRole::Math)
          {
            SetString(*value, info->AriaRole);
          }
          break;
        case UIA_AcceleratorKeyPropertyId:
          if (!info->KeyboardShortcut.empty())
          {
            SetString(*value, info->KeyboardShortcut);
          }
          break;
        case UIA_FrameworkIdPropertyId:         SetString(*value, u8"Krystal"); break;
        case UIA_ProviderDescriptionPropertyId: SetString(*value, u8"Krystal"); break;
        case UIA_IsKeyboardFocusablePropertyId: SetBool(*value, states.Contains(State::Focusable)); break;
        case UIA_HasKeyboardFocusPropertyId:    SetBool(*value, states.Contains(State::Focused)); break;
        case UIA_IsEnabledPropertyId:           SetBool(*value, !states.Contains(State::Disabled)); break;
        case UIA_IsOffscreenPropertyId:         SetBool(*value, states.Contains(State::Offscreen)); break;
        case UIA_IsPasswordPropertyId:          SetBool(*value, states.Contains(State::Protected)); break;
        case UIA_IsRequiredForFormPropertyId:   SetBool(*value, states.Contains(State::Required)); break;
        case UIA_IsDataValidForFormPropertyId:  SetBool(*value, !states.Contains(State::Invalid)); break;
        case UIA_IsDialogPropertyId:            SetBool(*value, info->Role == AccessibilityRole::Dialog); break;
        case UIA_IsControlElementPropertyId:
        case UIA_IsContentElementPropertyId:    SetBool(*value, true); break;
        case UIA_LevelPropertyId:
          if (info->Level)
          {
            SetInt(*value, static_cast<long>(info->Level));
          }
          break;
        case UIA_HeadingLevelPropertyId:
          SetInt(*value, info->HeadingLevel >= 1 && info->HeadingLevel <= 9
                           ? HeadingLevel1 + static_cast<long>(info->HeadingLevel) - 1
                           : HeadingLevel_None);
          break;
        case UIA_PositionInSetPropertyId:
          if (info->PositionInSet)
          {
            SetInt(*value, static_cast<long>(info->PositionInSet));
          }
          break;
        case UIA_SizeOfSetPropertyId:
          if (info->SizeOfSet)
          {
            SetInt(*value, static_cast<long>(info->SizeOfSet));
          }
          break;
        case UIA_LiveSettingPropertyId:
          SetInt(*value, info->LiveSetting == AccessibilityLiveSetting::Assertive
                           ? Assertive
                           : (info->LiveSetting == AccessibilityLiveSetting::Polite ? Polite : Off));
          break;
        case UIA_LandmarkTypePropertyId:
          if (info->Role == AccessibilityRole::Landmark)
          {
            const auto &role = info->AriaRole;
            SetInt(*value, role == u8"main"         ? UIA_MainLandmarkTypeId
                           : role == u8"navigation" ? UIA_NavigationLandmarkTypeId
                           : role == u8"search"     ? UIA_SearchLandmarkTypeId
                           : role == u8"form"       ? UIA_FormLandmarkTypeId
                                                    : UIA_CustomLandmarkTypeId);
          }
          break;
        case UIA_LocalizedLandmarkTypePropertyId:
          if (info->Role == AccessibilityRole::Landmark && info->AriaRole != u8"main"
              && info->AriaRole != u8"navigation" && info->AriaRole != u8"search"
              && info->AriaRole != u8"form")
          {
            SetString(*value, info->AriaRole);
          }
          break;
        default: break;
      }
      return S_OK;
    }

    IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple **provider) noexcept override
    {
      if (!provider)
      {
        return E_POINTER;
      }
      *provider = nullptr;
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      // The window's element is the window's: UI Automation takes its name and place from the window.
      return _node ? S_OK : ::UiaHostProviderFromHwnd(_owner->Window(), provider);
    }

    // IRawElementProviderFragment.

    IFACEMETHODIMP Navigate(NavigateDirection direction,
                            IRawElementProviderFragment **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = nullptr;
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto &tree = _owner->Tree();
      AccessibilityNodeID found = 0;
      bool foundWindow = false;
      if (!_node)
      {
        // The window's element has the tree's root as its only child.
        if (direction == NavigateDirection_FirstChild || direction == NavigateDirection_LastChild)
        {
          found = tree.Root();
        }
      }
      else
      {
        auto parent = tree.Parent(_node);
        bool isRoot = !parent && _node == tree.Root();
        switch (direction)
        {
          case NavigateDirection_Parent:
            found = parent;
            foundWindow = isRoot;
            break;
          case NavigateDirection_NextSibling:
          case NavigateDirection_PreviousSibling:
          {
            if (!parent)
            {
              break;
            }
            auto siblings = tree.Children(parent);
            auto it = std::ranges::find(siblings, _node);
            if (it == siblings.end())
            {
              break;
            }
            if (direction == NavigateDirection_NextSibling && it + 1 != siblings.end())
            {
              found = *(it + 1);
            }
            else if (direction == NavigateDirection_PreviousSibling && it != siblings.begin())
            {
              found = *(it - 1);
            }
            break;
          }
          case NavigateDirection_FirstChild:
          case NavigateDirection_LastChild:
          {
            auto children = tree.Children(_node);
            if (!children.empty())
            {
              found = direction == NavigateDirection_FirstChild ? children.front() : children.back();
            }
            break;
          }
        }
      }
      if (found || foundWindow)
      {
        auto &element = _owner->Element(found);
        element.AddRef();
        *result = static_cast<IRawElementProviderFragment *>(&element);
      }
      return S_OK;
    }

    IFACEMETHODIMP GetRuntimeId(SAFEARRAY **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = nullptr;
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      if (!_node)
      {
        // The window's runtime ID is its host's.
        return S_OK;
      }
      int id[3];
      RuntimeID(id);
      *result = ::SafeArrayCreateVector(VT_I4, 0, 3);
      if (!*result)
      {
        return E_OUTOFMEMORY;
      }
      for (LONG i = 0; i < 3; ++i)
      {
        ::SafeArrayPutElement(*result, &i, &id[i]);
      }
      return S_OK;
    }

    IFACEMETHODIMP get_BoundingRectangle(UiaRect *rect) noexcept override
    {
      if (!rect)
      {
        return E_POINTER;
      }
      *rect = {};
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      if (!_node)
      {
        return S_OK;
      }
      auto info = Info();
      if (!info)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      *rect = ToScreen(_owner->Window(), info->Bounds);
      return S_OK;
    }

    IFACEMETHODIMP GetEmbeddedFragmentRoots(SAFEARRAY **roots) noexcept override
    {
      if (!roots)
      {
        return E_POINTER;
      }
      *roots = nullptr;
      return S_OK;
    }

    IFACEMETHODIMP SetFocus() noexcept override
    {
      return Perform(AccessibilityAction::Focus);
    }

    IFACEMETHODIMP get_FragmentRoot(IRawElementProviderFragmentRoot **root) noexcept override
    {
      if (!root)
      {
        return E_POINTER;
      }
      *root = nullptr;
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto &element = _owner->Root();
      element.AddRef();
      *root = static_cast<IRawElementProviderFragmentRoot *>(&element);
      return S_OK;
    }

    // IRawElementProviderFragmentRoot, the window's element's.

    IFACEMETHODIMP ElementProviderFromPoint(double x, double y,
                                            IRawElementProviderFragment **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = nullptr;
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto origin = ClientOrigin(_owner->Window());
      auto node = _owner->Tree().HitTest(static_cast<float>(x - origin.x), static_cast<float>(y - origin.y));
      if (node)
      {
        auto &element = _owner->Element(node);
        element.AddRef();
        *result = static_cast<IRawElementProviderFragment *>(&element);
      }
      return S_OK;
    }

    IFACEMETHODIMP GetFocus(IRawElementProviderFragment **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = nullptr;
      if (!_owner)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      if (auto node = _owner->Tree().Focus())
      {
        auto &element = _owner->Element(node);
        element.AddRef();
        *result = static_cast<IRawElementProviderFragment *>(&element);
      }
      return S_OK;
    }

    // IInvokeProvider.

    IFACEMETHODIMP Invoke() noexcept override
    {
      return Perform(AccessibilityAction::Press);
    }

    // IToggleProvider.

    IFACEMETHODIMP Toggle() noexcept override
    {
      return Perform(AccessibilityAction::Press);
    }

    IFACEMETHODIMP get_ToggleState(::ToggleState *state) noexcept override
    {
      return Read(state, [](const AccessibilityNode &info) { return ToggleState(info); });
    }

    // IValueProvider.

    IFACEMETHODIMP SetValue(LPCWSTR value) noexcept override
    {
      if (!_owner || !Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      return _owner->Tree().SetValue(_node, Narrow(value)) ? S_OK : UIA_E_INVALIDOPERATION;
    }

    IFACEMETHODIMP get_Value(BSTR *value) noexcept override
    {
      if (!value)
      {
        return E_POINTER;
      }
      *value = nullptr;
      auto info = Info();
      if (!info)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      *value = ToBSTR(info->Value);
      return S_OK;
    }

    /// @note Both the value pattern's and the range value pattern's.
    IFACEMETHODIMP get_IsReadOnly(BOOL *readOnly) noexcept override
    {
      return Read(readOnly,
                  [](const AccessibilityNode &info)
                  {
                    return static_cast<BOOL>(!info.Actions.Contains(AccessibilityAction::SetValue)
                                             || info.States.Contains(AccessibilityState::ReadOnly));
                  });
    }

    // IRangeValueProvider.

    IFACEMETHODIMP SetValue(double value) noexcept override
    {
      if (!_owner || !Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      return _owner->Tree().SetRangeValue(_node, value) ? S_OK : UIA_E_INVALIDOPERATION;
    }

    IFACEMETHODIMP get_Value(double *value) noexcept override
    {
      return Read(value, [](const AccessibilityNode &info) { return info.Range ? info.Range->Value : 0.0; });
    }

    IFACEMETHODIMP get_Maximum(double *value) noexcept override
    {
      return Read(value,
                  [](const AccessibilityNode &info) { return info.Range ? info.Range->Maximum : 0.0; });
    }

    IFACEMETHODIMP get_Minimum(double *value) noexcept override
    {
      return Read(value,
                  [](const AccessibilityNode &info) { return info.Range ? info.Range->Minimum : 0.0; });
    }

    IFACEMETHODIMP get_LargeChange(double *value) noexcept override
    {
      return Read(value, [](const AccessibilityNode &info) { return SmallChange(info) * 10; });
    }

    IFACEMETHODIMP get_SmallChange(double *value) noexcept override
    {
      return Read(value, [](const AccessibilityNode &info) { return SmallChange(info); });
    }

    // IExpandCollapseProvider.

    IFACEMETHODIMP Expand() noexcept override
    {
      return Perform(AccessibilityAction::Expand);
    }

    IFACEMETHODIMP Collapse() noexcept override
    {
      return Perform(AccessibilityAction::Collapse);
    }

    IFACEMETHODIMP get_ExpandCollapseState(::ExpandCollapseState *state) noexcept override
    {
      return Read(state, [](const AccessibilityNode &info) { return ExpandCollapseState(info); });
    }

    // ISelectionItemProvider.

    IFACEMETHODIMP Select() noexcept override
    {
      auto info = Info();
      if (!info)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      // A radio button is selected by checking it.
      if (info->Role == AccessibilityRole::RadioButton)
      {
        return info->States.Contains(AccessibilityState::Checked) ? S_OK
                                                                  : Perform(AccessibilityAction::Press);
      }
      return Perform(AccessibilityAction::Select);
    }

    IFACEMETHODIMP AddToSelection() noexcept override
    {
      return Select();
    }

    IFACEMETHODIMP RemoveFromSelection() noexcept override
    {
      return Perform(AccessibilityAction::Deselect);
    }

    IFACEMETHODIMP get_IsSelected(BOOL *selected) noexcept override
    {
      return Read(selected,
                  [](const AccessibilityNode &info) { return static_cast<BOOL>(IsSelected(info)); });
    }

    IFACEMETHODIMP get_SelectionContainer(IRawElementProviderSimple **container) noexcept override
    {
      if (!container)
      {
        return E_POINTER;
      }
      *container = nullptr;
      if (!Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto &tree = _owner->Tree();
      for (auto node = tree.Parent(_node); node; node = tree.Parent(node))
      {
        if (auto info = tree.Node(node); info && IsSelectionContainer(*info))
        {
          auto &element = _owner->Element(node);
          element.AddRef();
          *container = static_cast<IRawElementProviderSimple *>(&element);
          break;
        }
      }
      return S_OK;
    }

    // ISelectionProvider.

    IFACEMETHODIMP GetSelection(SAFEARRAY **selection) noexcept override
    {
      if (!selection)
      {
        return E_POINTER;
      }
      *selection = nullptr;
      if (!Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto selected = _owner->Tree().SelectedChildren(_node);
      *selection = ::SafeArrayCreateVector(VT_UNKNOWN, 0, static_cast<ULONG>(selected.size()));
      if (!*selection)
      {
        return E_OUTOFMEMORY;
      }
      for (LONG i = 0; i < static_cast<LONG>(selected.size()); ++i)
      {
        auto &element = _owner->Element(selected[static_cast<size_t>(i)]);
        ::SafeArrayPutElement(*selection, &i, static_cast<IRawElementProviderSimple *>(&element));
      }
      return S_OK;
    }

    IFACEMETHODIMP get_CanSelectMultiple(BOOL *multiple) noexcept override
    {
      return Read(multiple, [](const AccessibilityNode &info)
                  { return static_cast<BOOL>(info.States.Contains(AccessibilityState::MultiSelectable)); });
    }

    IFACEMETHODIMP get_IsSelectionRequired(BOOL *required) noexcept override
    {
      return Read(required, [](const AccessibilityNode &) { return FALSE; });
    }

    // IScrollItemProvider.

    IFACEMETHODIMP ScrollIntoView() noexcept override
    {
      return Perform(AccessibilityAction::ScrollIntoView);
    }

  private:
    KRYS_NODISCARD static ::ToggleState ToggleState(const AccessibilityNode &info) noexcept
    {
      if (info.States.Contains(AccessibilityState::Mixed))
      {
        return ToggleState_Indeterminate;
      }
      return info.States.Contains(AccessibilityState::Checked) ? ToggleState_On : ToggleState_Off;
    }

    KRYS_NODISCARD static ::ExpandCollapseState ExpandCollapseState(const AccessibilityNode &info) noexcept
    {
      if (!info.States.Contains(AccessibilityState::Expandable))
      {
        return ExpandCollapseState_LeafNode;
      }
      return info.States.Contains(AccessibilityState::Expanded) ? ExpandCollapseState_Expanded
                                                                : ExpandCollapseState_Collapsed;
    }

    KRYS_NODISCARD static bool IsSelected(const AccessibilityNode &info) noexcept
    {
      return info.Role == AccessibilityRole::RadioButton ? info.States.Contains(AccessibilityState::Checked)
                                                         : info.States.Contains(AccessibilityState::Selected);
    }

    KRYS_NODISCARD static double SmallChange(const AccessibilityNode &info) noexcept
    {
      if (!info.Range)
      {
        return 0;
      }
      return info.Range->Step > 0 ? info.Range->Step : (info.Range->Maximum - info.Range->Minimum) / 100;
    }

    HRESULT Perform(AccessibilityAction action) noexcept
    {
      if (!_owner || !Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      return _owner->Tree().PerformAction(_node, action) ? S_OK : UIA_E_INVALIDOPERATION;
    }

    template <typename T, typename TRead>
    HRESULT Read(T *result, TRead read) noexcept
    {
      if (!result)
      {
        return E_POINTER;
      }
      auto info = Info();
      if (!info)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      *result = static_cast<T>(read(*info));
      return S_OK;
    }
  };

  /// @brief A range of a node's text, in UTF-16 code units (as UI Automation counts).
  class UIATextRange final : public ITextRangeProvider
  {
    KRYS_NON_COPY_MOVABLE(UIATextRange);

  private:
    ULONG _refCount {1};
    UIAElement *_element;
    int32 _start {0};
    int32 _end {0};

  public:
    UIATextRange(UIAElement &element, int32 start, int32 end) noexcept
        : _element(&element), _start(start), _end(end)
    {
      _element->AddRef();
    }
    ~UIATextRange() noexcept
    {
      _element->Release();
    }

    IFACEMETHODIMP QueryInterface(REFIID riid, void **object) noexcept override
    {
      if (!object)
      {
        return E_POINTER;
      }
      *object = nullptr;
      if (riid == __uuidof(IUnknown) || riid == __uuidof(ITextRangeProvider) || riid == IID_KrystalTextRange)
      {
        *object = static_cast<ITextRangeProvider *>(this);
        AddRef();
        return S_OK;
      }
      return E_NOINTERFACE;
    }

    IFACEMETHODIMP_(ULONG) AddRef() noexcept override
    {
      return ::InterlockedIncrement(&_refCount);
    }

    IFACEMETHODIMP_(ULONG) Release() noexcept override
    {
      auto count = ::InterlockedDecrement(&_refCount);
      if (!count)
      {
        delete this;
      }
      return count;
    }

    IFACEMETHODIMP Clone(ITextRangeProvider **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = new UIATextRange(*_element, _start, _end);
      return S_OK;
    }

    IFACEMETHODIMP Compare(ITextRangeProvider *other, BOOL *same) noexcept override
    {
      if (!same)
      {
        return E_POINTER;
      }
      auto *range = FromProvider(other);
      if (!range)
      {
        return E_INVALIDARG;
      }
      *same = range->_element == _element && range->_start == _start && range->_end == _end;
      range->Release();
      return S_OK;
    }

    IFACEMETHODIMP CompareEndpoints(TextPatternRangeEndpoint endpoint, ITextRangeProvider *other,
                                    TextPatternRangeEndpoint otherEndpoint, int *result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      auto *range = FromProvider(other);
      if (!range)
      {
        return E_INVALIDARG;
      }
      auto difference = Endpoint(endpoint) - range->Endpoint(otherEndpoint);
      range->Release();
      *result = difference < 0 ? -1 : (difference > 0 ? 1 : 0);
      return S_OK;
    }

    IFACEMETHODIMP ExpandToEnclosingUnit(TextUnit unit) noexcept override
    {
      std::wstring text;
      if (auto failure = ReadText(text); FAILED(failure))
      {
        return failure;
      }
      auto length = static_cast<int32>(text.size());
      Clamp(length);
      if (unit == TextUnit_Format || unit == TextUnit_Page || unit == TextUnit_Document)
      {
        _start = 0;
        _end = length;
        return S_OK;
      }
      if (_start == length && length > 0)
      {
        _start = PreviousBoundary(text, length, unit);
      }
      else if (!IsBoundary(text, _start, unit))
      {
        _start = PreviousBoundary(text, _start, unit);
      }
      _end = NextBoundary(text, _start, unit);
      return S_OK;
    }

    IFACEMETHODIMP FindAttribute(TEXTATTRIBUTEID, VARIANT, BOOL,
                                 ITextRangeProvider **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      // The text has no attributes to find.
      *result = nullptr;
      return S_OK;
    }

    IFACEMETHODIMP FindText(BSTR search, BOOL backward, BOOL ignoreCase,
                            ITextRangeProvider **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = nullptr;
      std::wstring text;
      if (auto failure = ReadText(text); FAILED(failure))
      {
        return failure;
      }
      Clamp(static_cast<int32>(text.size()));
      std::wstring needle(search ? search : L"");
      if (needle.empty())
      {
        return S_OK;
      }
      std::wstring haystack = text.substr(static_cast<size_t>(_start), static_cast<size_t>(_end - _start));
      if (ignoreCase)
      {
        ::CharLowerBuffW(haystack.data(), static_cast<DWORD>(haystack.size()));
        ::CharLowerBuffW(needle.data(), static_cast<DWORD>(needle.size()));
      }
      auto found = backward ? haystack.rfind(needle) : haystack.find(needle);
      if (found != std::wstring::npos)
      {
        auto start = _start + static_cast<int32>(found);
        *result = new UIATextRange(*_element, start, start + static_cast<int32>(needle.size()));
      }
      return S_OK;
    }

    IFACEMETHODIMP GetAttributeValue(TEXTATTRIBUTEID attribute, VARIANT *value) noexcept override
    {
      if (!value)
      {
        return E_POINTER;
      }
      ::VariantInit(value);
      auto info = _element->Info();
      if (!info)
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      if (attribute == UIA_IsReadOnlyAttributeId)
      {
        SetBool(*value, !info->States.Contains(AccessibilityState::Editable));
        return S_OK;
      }
      value->vt = VT_UNKNOWN;
      return ::UiaGetReservedNotSupportedValue(&value->punkVal);
    }

    IFACEMETHODIMP GetBoundingRectangles(SAFEARRAY **result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = nullptr;
      auto *owner = _element->Owner();
      if (!owner || !_element->Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto range = ToUTF8Range();
      auto bounds = owner->Tree().TextBounds(_element->Node(), range);
      *result = ::SafeArrayCreateVector(VT_R8, 0, static_cast<ULONG>(bounds.size() * 4));
      if (!*result)
      {
        return E_OUTOFMEMORY;
      }
      LONG index = 0;
      for (const auto &bound : bounds)
      {
        auto rect = ToScreen(owner->Window(), bound);
        for (double value : {rect.left, rect.top, rect.width, rect.height})
        {
          ::SafeArrayPutElement(*result, &index, &value);
          ++index;
        }
      }
      return S_OK;
    }

    IFACEMETHODIMP GetEnclosingElement(IRawElementProviderSimple **element) noexcept override
    {
      if (!element)
      {
        return E_POINTER;
      }
      _element->AddRef();
      *element = static_cast<IRawElementProviderSimple *>(_element);
      return S_OK;
    }

    IFACEMETHODIMP GetText(int maxLength, BSTR *result) noexcept override
    {
      if (!result)
      {
        return E_POINTER;
      }
      *result = nullptr;
      std::wstring text;
      if (auto failure = ReadText(text); FAILED(failure))
      {
        return failure;
      }
      Clamp(static_cast<int32>(text.size()));
      auto length = _end - _start;
      if (maxLength >= 0)
      {
        length = (std::min)(length, maxLength);
      }
      *result = ::SysAllocStringLen(text.data() + _start, static_cast<UINT>(length));
      return S_OK;
    }

    IFACEMETHODIMP Move(TextUnit unit, int count, int *moved) noexcept override
    {
      if (!moved)
      {
        return E_POINTER;
      }
      *moved = 0;
      std::wstring text;
      if (auto failure = ReadText(text); FAILED(failure))
      {
        return failure;
      }
      auto length = static_cast<int32>(text.size());
      Clamp(length);
      if (unit == TextUnit_Format || unit == TextUnit_Page || unit == TextUnit_Document)
      {
        // There's one of each, so there's nowhere to move to.
        return S_OK;
      }
      bool degenerate = _start == _end;
      auto position = IsBoundary(text, _start, unit) ? _start : PreviousBoundary(text, _start, unit);
      while (*moved < count)
      {
        auto next = NextBoundary(text, position, unit);
        // A unit can't start at the end, though the caret can be there.
        if (position >= length || (!degenerate && next >= length))
        {
          break;
        }
        position = next;
        ++*moved;
      }
      while (*moved > count && position > 0)
      {
        position = PreviousBoundary(text, position, unit);
        --*moved;
      }
      _start = position;
      _end = degenerate ? position : NextBoundary(text, position, unit);
      return S_OK;
    }

    IFACEMETHODIMP MoveEndpointByUnit(TextPatternRangeEndpoint endpoint, TextUnit unit, int count,
                                      int *moved) noexcept override
    {
      if (!moved)
      {
        return E_POINTER;
      }
      *moved = 0;
      std::wstring text;
      if (auto failure = ReadText(text); FAILED(failure))
      {
        return failure;
      }
      auto length = static_cast<int32>(text.size());
      Clamp(length);
      auto position = Endpoint(endpoint);
      if (unit == TextUnit_Format || unit == TextUnit_Page || unit == TextUnit_Document)
      {
        if (count > 0 && position < length)
        {
          position = length;
          *moved = 1;
        }
        else if (count < 0 && position > 0)
        {
          position = 0;
          *moved = -1;
        }
      }
      else
      {
        while (*moved < count && position < length)
        {
          position = NextBoundary(text, position, unit);
          ++*moved;
        }
        while (*moved > count && position > 0)
        {
          position = PreviousBoundary(text, position, unit);
          --*moved;
        }
      }
      SetEndpoint(endpoint, position);
      return S_OK;
    }

    IFACEMETHODIMP MoveEndpointByRange(TextPatternRangeEndpoint endpoint, ITextRangeProvider *other,
                                       TextPatternRangeEndpoint otherEndpoint) noexcept override
    {
      auto *range = FromProvider(other);
      if (!range)
      {
        return E_INVALIDARG;
      }
      SetEndpoint(endpoint, range->Endpoint(otherEndpoint));
      range->Release();
      return S_OK;
    }

    IFACEMETHODIMP Select() noexcept override
    {
      auto *owner = _element->Owner();
      if (!owner || !_element->Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      return owner->Tree().SetTextSelection(_element->Node(), ToUTF8Range()) ? S_OK : UIA_E_INVALIDOPERATION;
    }

    IFACEMETHODIMP AddToSelection() noexcept override
    {
      // The text has one selection.
      return UIA_E_INVALIDOPERATION;
    }

    IFACEMETHODIMP RemoveFromSelection() noexcept override
    {
      return UIA_E_INVALIDOPERATION;
    }

    IFACEMETHODIMP ScrollIntoView(BOOL) noexcept override
    {
      auto *owner = _element->Owner();
      if (!owner || !_element->Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      (void)owner->Tree().PerformAction(_element->Node(), AccessibilityAction::ScrollIntoView);
      return S_OK;
    }

    IFACEMETHODIMP GetChildren(SAFEARRAY **children) noexcept override
    {
      if (!children)
      {
        return E_POINTER;
      }
      // The text's embedded objects aren't exposed in it.
      *children = ::SafeArrayCreateVector(VT_UNKNOWN, 0, 0);
      return *children ? S_OK : E_OUTOFMEMORY;
    }

  private:
    /// @brief The Krystal range @p provider is (with a reference), or null if it's someone else's.
    KRYS_NODISCARD static UIATextRange *FromProvider(ITextRangeProvider *provider) noexcept
    {
      void *range = nullptr;
      if (!provider || FAILED(provider->QueryInterface(IID_KrystalTextRange, &range)))
      {
        return nullptr;
      }
      return static_cast<UIATextRange *>(static_cast<ITextRangeProvider *>(range));
    }

    KRYS_NODISCARD int32 Endpoint(TextPatternRangeEndpoint endpoint) const noexcept
    {
      return endpoint == TextPatternRangeEndpoint_Start ? _start : _end;
    }

    void SetEndpoint(TextPatternRangeEndpoint endpoint, int32 position) noexcept
    {
      // An endpoint moved past the other takes it along.
      if (endpoint == TextPatternRangeEndpoint_Start)
      {
        _start = position;
        _end = (std::max)(_end, position);
      }
      else
      {
        _end = position;
        _start = (std::min)(_start, position);
      }
    }

    void Clamp(int32 length) noexcept
    {
      _start = std::clamp(_start, 0, length);
      _end = std::clamp(_end, _start, length);
    }

    HRESULT ReadText(std::wstring &text) const noexcept
    {
      auto *owner = _element->Owner();
      if (!owner || !_element->Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      text = Widen(owner->Tree().Text(_element->Node()));
      return S_OK;
    }

    KRYS_NODISCARD AccessibilityTextRange ToUTF8Range() const noexcept
    {
      auto text = _element->Owner()->Tree().Text(_element->Node());
      auto start = ToUTF8Offset(text, _start);
      return {start, ToUTF8Offset(text, _end) - start};
    }
  };

  /// @brief An editable field's or a document's text pattern.
  class UIATextProvider final : public ITextProvider
  {
    KRYS_NON_COPY_MOVABLE(UIATextProvider);

  private:
    ULONG _refCount {1};
    UIAElement *_element;

  public:
    explicit UIATextProvider(UIAElement &element) noexcept : _element(&element)
    {
      _element->AddRef();
    }
    ~UIATextProvider() noexcept
    {
      _element->Release();
    }

    IFACEMETHODIMP QueryInterface(REFIID riid, void **object) noexcept override
    {
      if (!object)
      {
        return E_POINTER;
      }
      *object = nullptr;
      if (riid == __uuidof(IUnknown) || riid == __uuidof(ITextProvider))
      {
        *object = static_cast<ITextProvider *>(this);
        AddRef();
        return S_OK;
      }
      return E_NOINTERFACE;
    }

    IFACEMETHODIMP_(ULONG) AddRef() noexcept override
    {
      return ::InterlockedIncrement(&_refCount);
    }

    IFACEMETHODIMP_(ULONG) Release() noexcept override
    {
      auto count = ::InterlockedDecrement(&_refCount);
      if (!count)
      {
        delete this;
      }
      return count;
    }

    IFACEMETHODIMP GetSelection(SAFEARRAY **selection) noexcept override
    {
      if (!selection)
      {
        return E_POINTER;
      }
      *selection = nullptr;
      auto *owner = _element->Owner();
      if (!owner || !_element->Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto &tree = owner->Tree();
      auto range = tree.TextSelection(_element->Node());
      *selection = ::SafeArrayCreateVector(VT_UNKNOWN, 0, range ? 1 : 0);
      if (!*selection)
      {
        return E_OUTOFMEMORY;
      }
      if (range)
      {
        auto text = tree.Text(_element->Node());
        auto *provider = new UIATextRange(*_element, ToUTF16Offset(text, range->Start),
                                          ToUTF16Offset(text, range->Start + range->Length));
        LONG index = 0;
        ::SafeArrayPutElement(*selection, &index, static_cast<ITextRangeProvider *>(provider));
        provider->Release();
      }
      return S_OK;
    }

    IFACEMETHODIMP GetVisibleRanges(SAFEARRAY **ranges) noexcept override
    {
      if (!ranges)
      {
        return E_POINTER;
      }
      *ranges = nullptr;
      ITextRangeProvider *range = nullptr;
      if (auto failure = get_DocumentRange(&range); FAILED(failure))
      {
        return failure;
      }
      *ranges = ::SafeArrayCreateVector(VT_UNKNOWN, 0, 1);
      if (*ranges)
      {
        LONG index = 0;
        ::SafeArrayPutElement(*ranges, &index, range);
      }
      range->Release();
      return *ranges ? S_OK : E_OUTOFMEMORY;
    }

    IFACEMETHODIMP RangeFromChild(IRawElementProviderSimple *, ITextRangeProvider **range) noexcept override
    {
      // The text's embedded objects aren't exposed in it: the child is somewhere in all of it.
      return get_DocumentRange(range);
    }

    IFACEMETHODIMP RangeFromPoint(UiaPoint point, ITextRangeProvider **range) noexcept override
    {
      if (!range)
      {
        return E_POINTER;
      }
      *range = nullptr;
      auto *owner = _element->Owner();
      if (!owner || !_element->Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      // The caret goes before the character whose bounds hold the point, or the nearest line's last.
      auto &tree = owner->Tree();
      auto node = _element->Node();
      auto text = tree.Text(node);
      auto origin = ClientOrigin(owner->Window());
      auto x = static_cast<float>(point.x - origin.x);
      auto y = static_cast<float>(point.y - origin.y);
      int32 units = 0;
      int32 best = 0;
      float bestDistance = (std::numeric_limits<float>::max)();
      for (size_t i = 0; i < text.size();)
      {
        auto length = SequenceLength(text[i]);
        for (const auto &bound : tree.TextBounds(node, {i, length}))
        {
          if (x >= bound.X && x < bound.X + bound.Width && y >= bound.Y && y < bound.Y + bound.Height)
          {
            *range = new UIATextRange(*_element, units, units);
            return S_OK;
          }
          auto dx = x < bound.X ? bound.X - x : (std::max)(0.0f, x - (bound.X + bound.Width));
          auto dy = y < bound.Y ? bound.Y - y : (std::max)(0.0f, y - (bound.Y + bound.Height));
          if (dx * dx + dy * dy < bestDistance)
          {
            bestDistance = dx * dx + dy * dy;
            best = x > bound.X + bound.Width / 2 ? units + (length == 4 ? 2 : 1) : units;
          }
        }
        units += length == 4 ? 2 : 1;
        i += length;
      }
      *range = new UIATextRange(*_element, best, best);
      return S_OK;
    }

    IFACEMETHODIMP get_DocumentRange(ITextRangeProvider **range) noexcept override
    {
      if (!range)
      {
        return E_POINTER;
      }
      *range = nullptr;
      auto *owner = _element->Owner();
      if (!owner || !_element->Info())
      {
        return UIA_E_ELEMENTNOTAVAILABLE;
      }
      auto text = owner->Tree().Text(_element->Node());
      *range = new UIATextRange(*_element, 0, ToUTF16Offset(text, text.size()));
      return S_OK;
    }

    IFACEMETHODIMP get_SupportedTextSelection(SupportedTextSelection *selection) noexcept override
    {
      if (!selection)
      {
        return E_POINTER;
      }
      *selection = SupportedTextSelection_Single;
      return S_OK;
    }
  };

  HRESULT UIAElement::GetPatternProvider(PATTERNID pattern, IUnknown **provider) noexcept
  {
    if (!provider)
    {
      return E_POINTER;
    }
    *provider = nullptr;
    if (!_owner)
    {
      return UIA_E_ELEMENTNOTAVAILABLE;
    }
    if (!_node)
    {
      return S_OK;
    }
    auto info = Info();
    if (!info)
    {
      return UIA_E_ELEMENTNOTAVAILABLE;
    }
    if (!Supports(pattern, *info))
    {
      return S_OK;
    }
    switch (pattern)
    {
      case UIA_TextPatternId:           *provider = new UIATextProvider(*this); return S_OK;
      case UIA_InvokePatternId:         *provider = static_cast<IInvokeProvider *>(this); break;
      case UIA_TogglePatternId:         *provider = static_cast<IToggleProvider *>(this); break;
      case UIA_ValuePatternId:          *provider = static_cast<IValueProvider *>(this); break;
      case UIA_RangeValuePatternId:     *provider = static_cast<IRangeValueProvider *>(this); break;
      case UIA_ExpandCollapsePatternId: *provider = static_cast<IExpandCollapseProvider *>(this); break;
      case UIA_SelectionItemPatternId:  *provider = static_cast<ISelectionItemProvider *>(this); break;
      case UIA_SelectionPatternId:      *provider = static_cast<ISelectionProvider *>(this); break;
      case UIA_ScrollItemPatternId:     *provider = static_cast<IScrollItemProvider *>(this); break;
      default:                          return S_OK;
    }
    AddRef();
    return S_OK;
  }

  UIAutomation::UIAutomation(HWND window, IAccessibilityTree &tree) noexcept : _window(window), _tree(tree)
  {
  }

  UIAutomation::~UIAutomation() noexcept
  {
    for (auto &[node, element] : std::exchange(_elements, {}))
    {
      Disconnect(*element);
    }
    if (auto *root = std::exchange(_root, nullptr))
    {
      Disconnect(*root);
    }
  }

  Maybe<LRESULT> UIAutomation::HandleGetObject(WPARAM wParam, LPARAM lParam) noexcept
  {
    if (static_cast<long>(lParam) != static_cast<long>(UiaRootObjectId))
    {
      return null;
    }
    return ::UiaReturnRawElementProvider(_window, wParam, lParam, &Root());
  }

  void UIAutomation::WindowDestroyed() noexcept
  {
    // Tells UI Automation the window's provider is gone.
    ::UiaReturnRawElementProvider(_window, 0, 0, nullptr);
    for (auto &[node, element] : std::exchange(_elements, {}))
    {
      Disconnect(*element);
    }
    if (auto *root = std::exchange(_root, nullptr))
    {
      Disconnect(*root);
    }
  }

  bool UIAutomation::ClientsAreListening() noexcept
  {
    return ::UiaClientsAreListening();
  }

  UIAElement &UIAutomation::Root() noexcept
  {
    if (!_root)
    {
      _root = new UIAElement(*this, 0);
    }
    return *_root;
  }

  UIAElement &UIAutomation::Element(AccessibilityNodeID node) noexcept
  {
    if (!node)
    {
      return Root();
    }
    auto it = _elements.find(node);
    if (it == _elements.end())
    {
      it = _elements.emplace(node, new UIAElement(*this, node)).first;
    }
    return *it->second;
  }

  void UIAutomation::Disconnect(UIAElement &element) noexcept
  {
    element.Disconnect();
    ::UiaDisconnectProvider(static_cast<IRawElementProviderSimple *>(&element));
    element.Release();
  }

  void UIAutomation::NodeRemoved(AccessibilityNodeID node) noexcept
  {
    auto it = _elements.find(node);
    if (it == _elements.end())
    {
      return;
    }
    auto *element = it->second;
    _elements.erase(it);
    Disconnect(*element);
  }

  void UIAutomation::RaiseTextChange(AccessibilityNodeID node) noexcept
  {
    if (node && _tree.Node(node))
    {
      ::UiaRaiseAutomationEvent(&Element(node), UIA_Text_TextChangedEventId);
    }
  }

  void UIAutomation::RaiseEvent(AccessibilityNodeID node, AccessibilityEvent event) noexcept
  {
    auto info = node ? _tree.Node(node) : null;
    if (!info)
    {
      return;
    }
    auto &element = Element(node);
    auto *provider = static_cast<IRawElementProviderSimple *>(&element);
    auto raiseProperty = [&](PROPERTYID property)
    {
      VARIANT oldValue;
      VARIANT newValue;
      ::VariantInit(&oldValue);
      element.CurrentValue(property, newValue);
      ::UiaRaiseAutomationPropertyChangedEvent(provider, property, oldValue, newValue);
      ::VariantClear(&newValue);
    };
    switch (event)
    {
      case AccessibilityEvent::Focused:
        ::UiaRaiseAutomationEvent(provider, UIA_AutomationFocusChangedEventId);
        break;
      case AccessibilityEvent::ChildrenChanged:
      {
        int id[3];
        element.RuntimeID(id);
        ::UiaRaiseStructureChangedEvent(provider, StructureChangeType_ChildrenInvalidated, id, 3);
        break;
      }
      case AccessibilityEvent::NameChanged:        raiseProperty(UIA_NamePropertyId); break;
      case AccessibilityEvent::DescriptionChanged: raiseProperty(UIA_FullDescriptionPropertyId); break;
      case AccessibilityEvent::ValueChanged:
        raiseProperty(info->Range ? UIA_RangeValueValuePropertyId : UIA_ValueValuePropertyId);
        break;
      case AccessibilityEvent::CheckedChanged:
        raiseProperty(info->Role == AccessibilityRole::RadioButton ? UIA_SelectionItemIsSelectedPropertyId
                                                                   : UIA_ToggleToggleStatePropertyId);
        break;
      case AccessibilityEvent::ExpandedChanged:
        raiseProperty(UIA_ExpandCollapseExpandCollapseStatePropertyId);
        break;
      case AccessibilityEvent::SelectedChanged:
        raiseProperty(UIA_SelectionItemIsSelectedPropertyId);
        ::UiaRaiseAutomationEvent(provider, info->States.Contains(AccessibilityState::Selected)
                                              ? UIA_SelectionItem_ElementSelectedEventId
                                              : UIA_SelectionItem_ElementRemovedFromSelectionEventId);
        break;
      case AccessibilityEvent::SelectionChanged:
        ::UiaRaiseAutomationEvent(provider, UIA_Selection_InvalidatedEventId);
        break;
      case AccessibilityEvent::EnabledChanged: raiseProperty(UIA_IsEnabledPropertyId); break;
      case AccessibilityEvent::StateChanged:
        raiseProperty(UIA_IsRequiredForFormPropertyId);
        raiseProperty(UIA_IsDataValidForFormPropertyId);
        break;
      case AccessibilityEvent::TextSelectionChanged:
        ::UiaRaiseAutomationEvent(provider, UIA_Text_TextSelectionChangedEventId);
        break;
      case AccessibilityEvent::LiveRegionChanged:
        ::UiaRaiseAutomationEvent(provider, UIA_LiveRegionChangedEventId);
        break;
      case AccessibilityEvent::Alert:      ::UiaRaiseAutomationEvent(provider, UIA_SystemAlertEventId); break;
      case AccessibilityEvent::MenuOpened: ::UiaRaiseAutomationEvent(provider, UIA_MenuOpenedEventId); break;
      case AccessibilityEvent::MenuClosed: ::UiaRaiseAutomationEvent(provider, UIA_MenuClosedEventId); break;
      case AccessibilityEvent::DocumentLoaded:
      {
        ::UiaRaiseAsyncContentLoadedEvent(provider, AsyncContentLoadedState_Completed, 100.0);
        break;
      }
      case AccessibilityEvent::BoundsChanged: raiseProperty(UIA_BoundingRectanglePropertyId); break;
    }
  }
}
