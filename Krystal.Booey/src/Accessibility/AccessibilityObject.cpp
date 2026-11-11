/*
 * Copyright (C) 2023-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (C) 2008-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/ElementInternals.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <algorithm>
#include <format>
#include <limits>

namespace krys::boo::ax
{
  namespace
  {
    using infra::StringAlgorithms;

    struct AriaRoleEntry
    {
      utf8_stringview Name;
      AccessibilityRole Role;
    };

    // clang-format off
    constexpr AriaRoleEntry AriaRoles[] = {
      {u8"alert", AccessibilityRole::ApplicationAlert},
      {u8"alertdialog", AccessibilityRole::ApplicationAlertDialog},
      {u8"application", AccessibilityRole::WebApplication},
      {u8"article", AccessibilityRole::DocumentArticle},
      {u8"banner", AccessibilityRole::LandmarkBanner},
      {u8"blockquote", AccessibilityRole::Blockquote},
      {u8"button", AccessibilityRole::Button},
      {u8"caption", AccessibilityRole::Caption},
      {u8"code", AccessibilityRole::Code},
      {u8"checkbox", AccessibilityRole::Checkbox},
      {u8"complementary", AccessibilityRole::LandmarkComplementary},
      {u8"contentinfo", AccessibilityRole::LandmarkContentInfo},
      {u8"deletion", AccessibilityRole::Deletion},
      {u8"dialog", AccessibilityRole::ApplicationDialog},
      {u8"directory", AccessibilityRole::Directory},
      // The doc-* roles are DPUB-ARIA's: https://w3c.github.io/dpub-aam/
      {u8"doc-abstract", AccessibilityRole::TextGroup},
      {u8"doc-acknowledgments", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-afterword", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-appendix", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-backlink", AccessibilityRole::Link},
      {u8"doc-biblioentry", AccessibilityRole::ListItem},
      {u8"doc-bibliography", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-biblioref", AccessibilityRole::Link},
      {u8"doc-chapter", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-colophon", AccessibilityRole::TextGroup},
      {u8"doc-conclusion", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-cover", AccessibilityRole::Image},
      {u8"doc-credit", AccessibilityRole::TextGroup},
      {u8"doc-credits", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-dedication", AccessibilityRole::TextGroup},
      {u8"doc-endnote", AccessibilityRole::ListItem},
      {u8"doc-endnotes", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-epigraph", AccessibilityRole::TextGroup},
      {u8"doc-epilogue", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-errata", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-example", AccessibilityRole::TextGroup},
      {u8"doc-footnote", AccessibilityRole::Footnote},
      {u8"doc-foreword", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-glossary", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-glossref", AccessibilityRole::Link},
      {u8"doc-index", AccessibilityRole::LandmarkNavigation},
      {u8"doc-introduction", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-noteref", AccessibilityRole::Link},
      {u8"doc-notice", AccessibilityRole::DocumentNote},
      {u8"doc-pagebreak", AccessibilityRole::Splitter},
      {u8"doc-pagelist", AccessibilityRole::LandmarkNavigation},
      {u8"doc-part", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-preface", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-prologue", AccessibilityRole::LandmarkDocRegion},
      {u8"doc-pullquote", AccessibilityRole::TextGroup},
      {u8"doc-qna", AccessibilityRole::TextGroup},
      {u8"doc-subtitle", AccessibilityRole::Heading},
      {u8"doc-tip", AccessibilityRole::DocumentNote},
      {u8"doc-toc", AccessibilityRole::LandmarkNavigation},
      {u8"emphasis", AccessibilityRole::Emphasis},
      {u8"figure", AccessibilityRole::Figure},
      {u8"generic", AccessibilityRole::Generic},
      // The graphics-* roles are Graphics ARIA's: https://w3c.github.io/graphics-aam/
      {u8"graphics-document", AccessibilityRole::GraphicsDocument},
      {u8"graphics-object", AccessibilityRole::GraphicsObject},
      {u8"graphics-symbol", AccessibilityRole::GraphicsSymbol},
      {u8"grid", AccessibilityRole::Grid},
      {u8"gridcell", AccessibilityRole::GridCell},
      {u8"table", AccessibilityRole::Table},
      {u8"cell", AccessibilityRole::Cell},
      {u8"columnheader", AccessibilityRole::ColumnHeader},
      {u8"combobox", AccessibilityRole::ComboBox},
      {u8"definition", AccessibilityRole::Definition},
      {u8"document", AccessibilityRole::Document},
      {u8"feed", AccessibilityRole::Feed},
      {u8"form", AccessibilityRole::Form},
      {u8"rowheader", AccessibilityRole::RowHeader},
      {u8"group", AccessibilityRole::Group},
      {u8"heading", AccessibilityRole::Heading},
      // "image" is a synonym of "img": https://w3c.github.io/aria/#image
      {u8"image", AccessibilityRole::Image},
      {u8"img", AccessibilityRole::Image},
      {u8"insertion", AccessibilityRole::Insertion},
      {u8"link", AccessibilityRole::Link},
      {u8"list", AccessibilityRole::List},
      {u8"listitem", AccessibilityRole::ListItem},
      {u8"listbox", AccessibilityRole::ListBox},
      {u8"log", AccessibilityRole::ApplicationLog},
      {u8"main", AccessibilityRole::LandmarkMain},
      {u8"marquee", AccessibilityRole::ApplicationMarquee},
      {u8"math", AccessibilityRole::DocumentMath},
      {u8"mark", AccessibilityRole::Mark},
      {u8"menu", AccessibilityRole::Menu},
      {u8"menubar", AccessibilityRole::MenuBar},
      {u8"menuitem", AccessibilityRole::MenuItem},
      {u8"menuitemcheckbox", AccessibilityRole::MenuItemCheckbox},
      {u8"menuitemradio", AccessibilityRole::MenuItemRadio},
      {u8"meter", AccessibilityRole::Meter},
      {u8"none", AccessibilityRole::Presentational},
      {u8"note", AccessibilityRole::DocumentNote},
      {u8"navigation", AccessibilityRole::LandmarkNavigation},
      {u8"option", AccessibilityRole::ListBoxOption},
      {u8"paragraph", AccessibilityRole::Paragraph},
      {u8"presentation", AccessibilityRole::Presentational},
      {u8"progressbar", AccessibilityRole::ProgressIndicator},
      {u8"radio", AccessibilityRole::RadioButton},
      {u8"radiogroup", AccessibilityRole::RadioGroup},
      {u8"region", AccessibilityRole::LandmarkRegion},
      {u8"row", AccessibilityRole::Row},
      {u8"rowgroup", AccessibilityRole::RowGroup},
      {u8"scrollbar", AccessibilityRole::ScrollBar},
      {u8"search", AccessibilityRole::LandmarkSearch},
      {u8"searchbox", AccessibilityRole::SearchField},
      {u8"sectionfooter", AccessibilityRole::SectionFooter},
      {u8"sectionheader", AccessibilityRole::SectionHeader},
      {u8"separator", AccessibilityRole::Splitter},
      {u8"slider", AccessibilityRole::Slider},
      {u8"spinbutton", AccessibilityRole::SpinButton},
      {u8"status", AccessibilityRole::ApplicationStatus},
      {u8"subscript", AccessibilityRole::Subscript},
      {u8"suggestion", AccessibilityRole::Suggestion},
      {u8"superscript", AccessibilityRole::Superscript},
      {u8"strong", AccessibilityRole::Strong},
      {u8"switch", AccessibilityRole::Switch},
      {u8"tab", AccessibilityRole::Tab},
      {u8"tablist", AccessibilityRole::TabList},
      {u8"tabpanel", AccessibilityRole::TabPanel},
      {u8"text", AccessibilityRole::StaticText},
      {u8"textbox", AccessibilityRole::TextField},
      {u8"term", AccessibilityRole::Term},
      {u8"time", AccessibilityRole::Time},
      {u8"timer", AccessibilityRole::ApplicationTimer},
      {u8"toolbar", AccessibilityRole::Toolbar},
      {u8"tooltip", AccessibilityRole::UserInterfaceTooltip},
      {u8"tree", AccessibilityRole::Tree},
      {u8"treegrid", AccessibilityRole::TreeGrid},
      {u8"treeitem", AccessibilityRole::TreeItem},
    };
    // clang-format on

    /// @brief The ARIA role name each role maps back to, for `ComputedRoleString`.
    KRYS_NODISCARD const HashMap<AccessibilityRole, utf8_stringview> &ReverseAriaRoleMap() noexcept
    {
      static const HashMap<AccessibilityRole, utf8_stringview> map = []
      {
        HashMap<AccessibilityRole, utf8_stringview> result;
        for (const auto &entry : AriaRoles)
        {
          result.insert_or_assign(entry.Role, entry.Name);
        }
        // The synonyms the computed role uses.
        result.insert_or_assign(AccessibilityRole::DateTime, u8"textbox");
        result.insert_or_assign(AccessibilityRole::TextArea, u8"textbox");
        result.insert_or_assign(AccessibilityRole::DescriptionListDetail, u8"definition");
        result.insert_or_assign(AccessibilityRole::DescriptionListTerm, u8"term");
        result.insert_or_assign(AccessibilityRole::Details, u8"group");
        result.insert_or_assign(AccessibilityRole::Image, u8"image");
        result.insert_or_assign(AccessibilityRole::ListBoxOption, u8"option");
        result.insert_or_assign(AccessibilityRole::MenuListOption, u8"option");
        result.insert_or_assign(AccessibilityRole::Presentational, u8"none");
        return result;
      }();
      return map;
    }

    KRYS_NODISCARD utf8_stringview ReverseAriaRole(AccessibilityRole role) noexcept
    {
      const auto &map = ReverseAriaRoleMap();
      auto found = map.find(role);
      return found == map.end() ? utf8_stringview {} : found->second;
    }

    KRYS_NODISCARD bool IsARIAInput(AccessibilityRole role) noexcept
    {
      switch (role)
      {
        case AccessibilityRole::Checkbox:
        case AccessibilityRole::RadioButton:
        case AccessibilityRole::SearchField:
        case AccessibilityRole::Switch:
        case AccessibilityRole::TextField:   return true;
        default:                             return false;
      }
    }

    KRYS_NODISCARD bool IsAttributeTrue(dom::DOMStringView value) noexcept
    {
      return StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true");
    }
  }

  utf8_stringview RoleToString(AccessibilityRole role) noexcept
  {
    switch (role)
    {
#define KRYS_AX_ROLE_NAME(name)                                                                              \
  case AccessibilityRole::name: return u8"" #name;
      KRYS_AX_ROLES(KRYS_AX_ROLE_NAME)
#undef KRYS_AX_ROLE_NAME
    }
    return {};
  }

  AccessibilityObject::AccessibilityObject(AXID id, AXObjectCache &cache) noexcept
      : _id(id), _axObjectCache(&cache)
  {
  }

  AccessibilityObject::~AccessibilityObject() noexcept = default;

  void AccessibilityObject::Init() noexcept
  {
    _role = DetermineAccessibilityRole();
  }

  void AccessibilityObject::Detach(AccessibilityDetachmentType detachmentType) noexcept
  {
    DetachRemoteParts(detachmentType);
    if (auto *cache = _axObjectCache)
    {
      cache->DetachWrapper(*this, detachmentType);
    }
    _axObjectCache = nullptr;
  }

  void AccessibilityObject::DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept
  {
    if (detachmentType == AccessibilityDetachmentType::ElementDestroyed)
    {
      PostMenuClosedNotificationIfNecessary();
    }

    // Clear any children and call detachFromParent on them so that no children are left with dangling
    // pointers to their parent.
    ClearChildren();
  }

  void AccessibilityObject::PostMenuClosedNotificationIfNecessary() const noexcept
  {
    if (!IsMenu())
    {
      return;
    }
    if (auto *cache = _axObjectCache)
    {
      // Assistive technologies need to be informed when menus close. The web area stands in for the menu,
      // which is being destroyed.
      cache->PostNotification(cache->RootObject(), AXNotification::MenuClosed);
    }
  }

  utf8_string AccessibilityObject::DebugDescription() const noexcept
  {
    auto description = utf8_string(u8"{role: ") + utf8_string(RoleToString(_role));
    auto id = std::format("{}", static_cast<uint64>(_id));
    description += u8", ID " + utf8_string(id.begin(), id.end());
    if (auto *element = GetElement())
    {
      description += u8", " + utf8_string(element->LocalName().View());
    }
    else if (auto *node = GetNode())
    {
      description += u8", " + utf8_string(node->NodeName());
    }
    else if (Renderer())
    {
      description += u8", anonymous renderer";
    }
    description += u8"}";
    return description;
  }

  dom::Element *AccessibilityObject::GetElement() const noexcept
  {
    return DynamicDowncast<dom::Element>(GetNode());
  }

  dom::Document *AccessibilityObject::GetDocument() const noexcept
  {
    auto *node = GetNode();
    return node ? &node->NodeDocument() : nullptr;
  }

  Page *AccessibilityObject::GetPage() const noexcept
  {
    auto *document = GetDocument();
    return document ? document->GetPage() : nullptr;
  }

  LocalFrameView *AccessibilityObject::DocumentFrameView() const noexcept
  {
    auto *document = GetDocument();
    return document ? document->View() : nullptr;
  }

  const RenderStyle *AccessibilityObject::Style() const noexcept
  {
    if (auto *renderer = Renderer())
    {
      // A text renderer's style is its parent's, so it has none while it has no parent (before it's inserted,
      // or while a render tree update is destroying it).
      if (Is<RenderText>(*renderer))
      {
        return renderer->Parent() ? &renderer->Style() : nullptr;
      }
      return &renderer->Style();
    }
    auto *element = GetElement();
    if (!element)
    {
      return nullptr;
    }
    // Style can't be resolved in the middle of a render tree update.
    if (RenderTreeBuilder::Current())
    {
      return element->ExistingComputedStyle();
    }
    return static_cast<dom::Node &>(*element).ComputedStyle();
  }

  AccessibilityObject *AccessibilityObject::ParentObjectUnignored() const noexcept
  {
    for (auto *parent = ParentObject(); parent; parent = parent->ParentObject())
    {
      if (!parent->IsIgnored())
      {
        return parent;
      }
    }
    return nullptr;
  }

  const AccessibilityObject::AccessibilityChildrenVector &
    AccessibilityObject::Children(bool updateChildrenIfNeeded) noexcept
  {
    if (updateChildrenIfNeeded)
    {
      UpdateChildrenIfNecessary();
    }
    return _children;
  }

  void AccessibilityObject::UpdateChildrenIfNecessary() noexcept
  {
    if (!_childrenInitialized)
    {
      // FIXME: Caching each object's ignored state while the children are added (AXAttributeCacheScope),
      // so a deep tree doesn't work it out again for each ancestor.
      AddChildren();
    }
  }

  void AccessibilityObject::ClearChildren() noexcept
  {
    // Some objects have weak pointers to their parents and those associations need to be detached.
    for (const auto &child : _children)
    {
      child->DetachFromParent();
    }

    _children.clear();
    _childrenInitialized = false;
  }

  void AccessibilityObject::InsertChild(AccessibilityObject &child, uint32 index,
                                        DescendIfIgnored descendIfIgnored) noexcept
  {
    // A child aria-owned by another object is that object's child, not this one's.
    if (auto owners = child.Owners(); !owners.empty())
    {
      if (std::ranges::none_of(owners, [this](const auto &owner) { return owner.get() == this; }))
      {
        return;
      }
    }

    // An area is its image's child (not its map's): only the image inserts it.
    if (child.IsImageMapLink() && child.ParentObject() != this) KRYS_UNLIKELY
    {
      return;
    }

    // If the parent is asking for this child's children, then either it's the first time (and clearing is a
    // no-op), or its visibility has changed. In the latter case, this child may have a stale child cached.
    // This can prevent aria-hidden changes from working correctly. Hence, whenever a parent is getting
    // children, ensure data is not stale.
    if (child.NeedsToUpdateChildren() || _subtreeDirty)
    {
      child.ClearChildren();
      // Pass the dirty subtree down to the child so that its children get reset too.
      if (_subtreeDirty)
      {
        child.SetNeedsToUpdateSubtree();
      }
    }

    auto insert = [this](Ref<AccessibilityObject> object, uint32 at)
    {
      if (ShouldSetChildIndexInParent())
      {
        object->_indexInParent = at;
      }
      _children.insert(_children.begin() + at, krys::move(object));
    };

    if (child.IsIgnored())
    {
      if (descendIfIgnored == DescendIfIgnored::Yes)
      {
        auto insertionIndex = index;
        for (const auto &grandchild : child.Children())
        {
          // Asking the child for its ignored state or its children can lay out, which can clear this
          // object's children, leaving the insertion index past the end.
          if (insertionIndex > _children.size())
          {
            break;
          }
          insert(grandchild, insertionIndex);
          ++insertionIndex;
        }
      }
    }
    else
    {
      insert(ShareRef(child), index);
    }
  }

  void AccessibilityObject::ResetChildrenIndexInParent() noexcept
  {
    if (!ShouldSetChildIndexInParent())
    {
      return;
    }
    uint32 index = 0;
    for (const auto &child : _children)
    {
      child->_indexInParent = index++;
    }
  }

  bool AccessibilityObject::IsDescendantOfObject(const AccessibilityObject &ancestor) const noexcept
  {
    for (auto *parent = ParentObject(); parent; parent = parent->ParentObject())
    {
      if (parent == &ancestor)
      {
        return true;
      }
    }
    return false;
  }

  bool AccessibilityObject::IsAncestorOfObject(const AccessibilityObject &descendant) const noexcept
  {
    return this == &descendant || descendant.IsDescendantOfObject(*this);
  }

  bool AccessibilityObject::IsDescendantOfRole(AccessibilityRole role) const noexcept
  {
    for (auto *parent = ParentObject(); parent; parent = parent->ParentObject())
    {
      if (parent->Role() == role)
      {
        return true;
      }
    }
    return false;
  }

  bool AccessibilityObject::IsIgnored() const noexcept
  {
    // FIXME: The cache of computed attributes (AXComputedObjectAttributeCache), while the tree is built.
    return IsIgnoredWithoutCache();
  }

  bool AccessibilityObject::IsIgnoredWithoutCache() const noexcept
  {
    // FIXME: Objects outside the modal dialog that's open (ignoredFromModalPresence), with the modal
    // tracking.
    bool ignored = ComputeIsIgnored();

    auto previous = _lastKnownIsIgnoredValue;
    const_cast<AccessibilityObject *>(this)->_lastKnownIsIgnoredValue =
      ignored ? AccessibilityObjectInclusion::IgnoreObject : AccessibilityObjectInclusion::IncludeObject;

    if (auto *cache = _axObjectCache)
    {
      bool becameUnignored = previous == AccessibilityObjectInclusion::IgnoreObject && !ignored;
      bool becameIgnored = previous == AccessibilityObjectInclusion::IncludeObject && ignored;
      if (becameIgnored)
      {
        // If a menu became ignored, e.g. because it became display:none, ATs need to be informed.
        PostMenuClosedNotificationIfNecessary();
      }
      if (becameUnignored || becameIgnored)
      {
        cache->ChildrenChanged(ParentObject());
      }
    }
    return ignored;
  }

  void AccessibilityObject::RecomputeIsIgnoredForDescendants(bool includeSelf) noexcept
  {
    if (includeSelf)
    {
      RecomputeIsIgnored();
    }
    for (const auto &child : Children(false))
    {
      child->RecomputeIsIgnoredForDescendants(true);
    }
  }

  AccessibilityObjectInclusion AccessibilityObject::DefaultObjectInclusion() const noexcept
  {
    if (const auto *style = Style())
    {
      if (style->EffectiveInert())
      {
        return AccessibilityObjectInclusion::IgnoreObject;
      }
      // FIXME: Including hidden="until-found" content (isHiddenUntilFoundContainer), with search.
      if (style->UsedVisibility() != Visibility::Visible)
      {
        return AccessibilityObjectInclusion::IgnoreObject;
      }
    }

    if (IsARIAHidden())
    {
      return AccessibilityObjectInclusion::IgnoreObject;
    }

    bool ignoreARIAHidden = IsFocused();
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      // Not isRenderHidden(), which also checks visibility: a visibility: visible descendant of a
      // visibility: hidden ancestor is visible, and the object's own visibility is checked above.
      if (const auto *style = ancestor->Style(); style && style->EffectiveDisplay() == DisplayType::None)
      {
        return AccessibilityObjectInclusion::IgnoreObject;
      }
      if ((!ignoreARIAHidden && ancestor->IsARIAHidden()) || ancestor->AriaRoleHasPresentationalChildren()
          || !ancestor->CanHaveChildren())
      {
        return AccessibilityObjectInclusion::IgnoreObject;
      }
    }

    // Include <dialog> elements and elements with role="dialog".
    if (_role == AccessibilityRole::ApplicationDialog)
    {
      return AccessibilityObjectInclusion::IncludeObject;
    }
    return AccessibilityObjectInclusion::DefaultBehavior;
  }

  bool AccessibilityObject::IsARIAHidden() const noexcept
  {
    if (IsFocused())
    {
      return false;
    }

    auto *node = GetNode();
    auto *element = GetElement();
    // To prevent authors from hiding all content from assistive technology users, aria-hidden isn't
    // respected on html, body, or a document's root svg element (https://github.com/w3c/aria/pull/1880).
    if (element)
    {
      auto localName = element->LocalName();
      if ((Is<html::HTMLElement>(*element) && (localName == u8"body" || localName == u8"html"))
          || (Is<svg::SVGElement>(*element) && localName == u8"svg" && !element->ParentNode()))
      {
        return false;
      }
    }

    if (auto *slot = node ? dom::SlotAlgorithms::GetAssignedSlot(*node) : nullptr)
    {
      auto *slotObject = _axObjectCache ? _axObjectCache->Get(*slot) : nullptr;
      if (slotObject ? IsAttributeTrue(slotObject->GetAttribute(u8"aria-hidden"))
                     : IsAttributeTrue(slot->GetAttribute(u8"aria-hidden").value_or(dom::DOMString {})))
      {
        return true;
      }
    }
    return element && IsAttributeTrue(GetAttribute(u8"aria-hidden"));
  }

  bool AccessibilityObject::IsAXHidden() const noexcept
  {
    if (IsFocused())
    {
      return false;
    }
    for (auto *object = this; object; object = object->ParentObject())
    {
      if (object->IsARIAHidden())
      {
        return true;
      }
    }
    return false;
  }

  void AccessibilityObject::UpdateRole() noexcept
  {
    auto previousRole = _role;
    RecomputeAriaRole();
    _role = DetermineAccessibilityRole();
    if (previousRole != _role)
    {
      if (auto *cache = _axObjectCache)
      {
        cache->HandleRoleChanged(*this, previousRole);
      }
    }
  }

  AccessibilityRole AccessibilityObject::AriaRoleToRole(dom::DOMStringView value) noexcept
  {
    return AriaRoleToRole(value, [](AccessibilityRole) { return false; });
  }

  AccessibilityRole
    AccessibilityObject::AriaRoleToRole(dom::DOMStringView value,
                                        const Func<bool(AccessibilityRole)> &skipRole) noexcept
  {
    for (const auto &roleName : StringAlgorithms::SplitOnWhitespace(value))
    {
      for (const auto &entry : AriaRoles)
      {
        if (!StringAlgorithms::ASCIICaseInsensitiveMatch(roleName, entry.Name))
        {
          continue;
        }
        if (!skipRole(entry.Role))
        {
          return entry.Role;
        }
        break;
      }
    }
    return AccessibilityRole::Unknown;
  }

  utf8_stringview AccessibilityObject::ComputedRoleString() const noexcept
  {
    if (_role == AccessibilityRole::Image && IsIgnored())
    {
      return ReverseAriaRole(AccessibilityRole::Presentational);
    }

    // A role string is computed for block elements with author-provided roles.
    auto ariaRole = AriaRoleAttribute();
    if (ariaRole == AccessibilityRole::TextGroup || _role == AccessibilityRole::Footnote
        || _role == AccessibilityRole::GraphicsObject)
    {
      return ReverseAriaRole(AccessibilityRole::Group);
    }

    // No role string is computed for generic block elements with user-agent assigned roles.
    switch (_role)
    {
      case AccessibilityRole::TextGroup:         return {};
      case AccessibilityRole::GraphicsDocument:  return ReverseAriaRole(AccessibilityRole::Document);
      case AccessibilityRole::GraphicsSymbol:    return ReverseAriaRole(AccessibilityRole::Image);
      case AccessibilityRole::HorizontalRule:    return ReverseAriaRole(AccessibilityRole::Splitter);
      case AccessibilityRole::PopUpButton:
      case AccessibilityRole::ToggleButton:      return ReverseAriaRole(AccessibilityRole::Button);
      case AccessibilityRole::LandmarkDocRegion: return ReverseAriaRole(AccessibilityRole::LandmarkRegion);
      default:                                   break;
    }
    // A header cell is a cell whose role says which kind of header it is.
    if (IsColumnHeader())
    {
      return ReverseAriaRole(AccessibilityRole::ColumnHeader);
    }
    if (IsRowHeader())
    {
      return ReverseAriaRole(AccessibilityRole::RowHeader);
    }
    return ReverseAriaRole(_role);
  }

  bool AccessibilityObject::AriaRoleHasPresentationalChildren() const noexcept
  {
    switch (AriaRoleAttribute())
    {
      case AccessibilityRole::Button:
      case AccessibilityRole::Slider:
      case AccessibilityRole::Image:
      case AccessibilityRole::ProgressIndicator:
      case AccessibilityRole::SpinButton:        return true;
      default:                                   return false;
    }
  }

  bool AccessibilityObject::IsARIAControl(AccessibilityRole role) noexcept
  {
    if (IsARIAInput(role))
    {
      return true;
    }
    switch (role)
    {
      case AccessibilityRole::Button:
      case AccessibilityRole::ComboBox:
      case AccessibilityRole::ListBox:
      case AccessibilityRole::PopUpButton:
      case AccessibilityRole::Slider:
      case AccessibilityRole::TextArea:
      case AccessibilityRole::ToggleButton: return true;
      default:                              return false;
    }
  }

  bool AccessibilityObject::IsMenuItem() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::MenuItem:
      case AccessibilityRole::MenuItemRadio:
      case AccessibilityRole::MenuItemCheckbox: return true;
      default:                                  return false;
    }
  }

  bool AccessibilityObject::IsMenuRelated() const noexcept
  {
    return IsMenu() || IsMenuBar() || IsMenuItem();
  }

  bool AccessibilityObject::IsButton() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::Button:
      case AccessibilityRole::PopUpButton:
      case AccessibilityRole::ToggleButton: return true;
      default:                              return false;
    }
  }

  bool AccessibilityObject::IsControl() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::Button:
      case AccessibilityRole::Checkbox:
      case AccessibilityRole::ColorWell:
      case AccessibilityRole::ComboBox:
      case AccessibilityRole::DateTime:
      case AccessibilityRole::LandmarkSearch:
      case AccessibilityRole::ListBox:
      case AccessibilityRole::PopUpButton:
      case AccessibilityRole::RadioButton:
      case AccessibilityRole::SearchField:
      case AccessibilityRole::Slider:
      case AccessibilityRole::SliderThumb:
      case AccessibilityRole::Switch:
      case AccessibilityRole::TextArea:
      case AccessibilityRole::TextField:
      case AccessibilityRole::ToggleButton:   return true;
      default:
      {
        // A fieldset is a control too.
        auto *element = GetElement();
        return element && Is<html::HTMLElement>(*element) && element->LocalName() == u8"fieldset";
      }
    }
  }

  bool AccessibilityObject::IsTextControl() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::ComboBox:
      case AccessibilityRole::SearchField:
      case AccessibilityRole::TextArea:
      case AccessibilityRole::TextField:   return true;
      default:                             return false;
    }
  }

  bool AccessibilityObject::IsImageMapLink() const noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(GetElement());
    return element && element->TagQualifiedName().TagName() == html::TagName::area;
  }

  bool AccessibilityObject::HasCellRole() const noexcept
  {
    return _role == AccessibilityRole::Cell || _role == AccessibilityRole::GridCell
           || _role == AccessibilityRole::ColumnHeader || _role == AccessibilityRole::RowHeader;
  }

  bool AccessibilityObject::HasRowGroupTag() const noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(GetElement());
    if (!element)
    {
      return false;
    }
    auto tagName = element->TagQualifiedName().TagName();
    return tagName == html::TagName::thead || tagName == html::TagName::tbody
           || tagName == html::TagName::tfoot;
  }

  AccessibilityObject *AccessibilityObject::ExposedTableAncestor(bool includeSelf) const noexcept
  {
    for (auto *ancestor = includeSelf ? const_cast<AccessibilityObject *>(this) : ParentObject(); ancestor;
         ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsExposableTable())
      {
        return ancestor;
      }
    }
    return nullptr;
  }

  AccessibilityObject *AccessibilityObject::ColumnHeader() noexcept
  {
    if (!IsTableColumn())
    {
      return nullptr;
    }
    auto *parent = ParentObject();
    if (!parent || !parent->IsExposableTable())
    {
      return nullptr;
    }
    for (const auto &cell : Children())
    {
      if (cell->IsColumnHeader())
      {
        return cell.get();
      }
    }
    return nullptr;
  }

  AccessibilityObject *AccessibilityObject::RowHeader() noexcept
  {
    const auto &rowChildren = Children();
    if (rowChildren.empty())
    {
      return nullptr;
    }

    auto isTH = [](const AccessibilityObject &object)
    {
      auto *element = DynamicDowncast<html::HTMLElement>(object.GetElement());
      return element && element->TagQualifiedName().TagName() == html::TagName::th;
    };
    bool isARIAGridRow = IsARIAGridRow();
    auto *firstCell = rowChildren[0].get();
    if (!isARIAGridRow && !isTH(*firstCell))
    {
      return nullptr;
    }

    // A row of nothing but headers (outside a grid) is unlikely to have a row header.
    for (const auto &child : rowChildren)
    {
      // A cell that isn't a header: the first cell is the row's header.
      if (!isARIAGridRow && !isTH(*child))
      {
        return firstCell;
      }
      // In a grid row, the first row header.
      if (isARIAGridRow && child->IsRowHeader())
      {
        return child.get();
      }
    }
    return nullptr;
  }

  AccessibilityObject::AccessibilityChildrenVector AccessibilityObject::ColumnHeaders() noexcept
  {
    AccessibilityChildrenVector headers;
    if (HasCellRole())
    {
      auto parentTable = ShareRefPtr(ExposedTableAncestor());
      if (!parentTable)
      {
        return {};
      }
      // The cells the headers attribute names are the column headers, when there are any.
      headers = RelatedObjects(AXRelation::Headers);
      if (!headers.empty())
      {
        return headers;
      }

      auto rowRange = RowIndexRange();
      auto columnRange = ColumnIndexRange();
      auto rowGroupAncestor = RowGroupAncestorID();
      for (uint32 row = 0; row < rowRange.first; ++row)
      {
        auto *tableCell = parentTable->CellForColumnAndRow(columnRange.first, row);
        if (!tableCell || tableCell == this
            || std::ranges::any_of(headers, [&](const auto &header) { return header.get() == tableCell; }))
        {
          continue;
        }
        if ((tableCell->CellScope() == u8"colgroup" && tableCell->RowGroupAncestorID() == rowGroupAncestor)
            || tableCell->IsColumnHeader())
        {
          headers.emplace_back(ShareRef(*tableCell));
        }
      }
    }
    else if (IsTable())
    {
      for (const auto &column : Columns())
      {
        if (auto *header = column->ColumnHeader())
        {
          headers.emplace_back(ShareRef(*header));
        }
      }
    }
    return headers;
  }

  Maybe<AXID> AccessibilityObject::RowGroupAncestorID() const noexcept
  {
    if (!HasCellRole())
    {
      return null;
    }
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->HasRowGroupTag())
      {
        return ancestor->ObjectID();
      }
    }
    return null;
  }

  bool
    AccessibilityObject::IsTableCellInSameRowGroup(const AccessibilityObject &otherTableCell) const noexcept
  {
    auto ancestorID = RowGroupAncestorID();
    return ancestorID && ancestorID == otherTableCell.RowGroupAncestorID();
  }

  bool AccessibilityObject::IsGroup() const noexcept
  {
    return _role == AccessibilityRole::Group || _role == AccessibilityRole::TextGroup;
  }

  bool AccessibilityObject::IsLandmark() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::Form:
      case AccessibilityRole::LandmarkBanner:
      case AccessibilityRole::LandmarkComplementary:
      case AccessibilityRole::LandmarkContentInfo:
      case AccessibilityRole::LandmarkDocRegion:
      case AccessibilityRole::LandmarkMain:
      case AccessibilityRole::LandmarkNavigation:
      case AccessibilityRole::LandmarkRegion:
      case AccessibilityRole::LandmarkSearch:        return true;
      default:                                       return false;
    }
  }

  AccessibilityObject::AccessibilityChildrenVector
    AccessibilityObject::RelatedObjects(AXRelation relation) const noexcept
  {
    auto *cache = AxObjectCache();
    auto ids = cache ? cache->RelatedObjectIDsFor(*this, relation) : null;
    if (!ids)
    {
      return {};
    }
    AccessibilityChildrenVector objects;
    for (auto id : *ids)
    {
      if (auto *object = cache->ObjectForID(id))
      {
        objects.push_back(ShareRef(*object));
      }
    }
    return objects;
  }

  AccessibilityObject *AccessibilityObject::ActiveDescendant() const noexcept
  {
    auto activeDescendants = RelatedObjects(AXRelation::ActiveDescendant);
    krys_debug_assert(activeDescendants.size() <= 1);
    return activeDescendants.empty() ? nullptr : activeDescendants[0].get();
  }

  bool AccessibilityObject::ShouldFocusActiveDescendant() const noexcept
  {
    switch (AriaRoleAttribute())
    {
      case AccessibilityRole::Group:
      case AccessibilityRole::ListBox:
      case AccessibilityRole::Menu:
      case AccessibilityRole::MenuBar:
      case AccessibilityRole::RadioGroup:
      case AccessibilityRole::Row:
      case AccessibilityRole::PopUpButton:
      case AccessibilityRole::Meter:
      case AccessibilityRole::ProgressIndicator:
      case AccessibilityRole::Toolbar:
      case AccessibilityRole::Tree:
      case AccessibilityRole::Grid:              return true;
      default:                                   return false;
    }
  }

  bool AccessibilityObject::SupportsActiveDescendant() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::ComboBox:
      case AccessibilityRole::Grid:
      case AccessibilityRole::List:
      case AccessibilityRole::ListBox:
      case AccessibilityRole::TextArea:
      case AccessibilityRole::TextField:
      case AccessibilityRole::Tree:
      case AccessibilityRole::TreeGrid:  return true;
      default:                           return false;
    }
  }

  bool AccessibilityObject::CanHaveSelectedChildren() const noexcept
  {
    switch (_role)
    {
      // Containers whose children support aria-selected.
      case AccessibilityRole::Grid:
      case AccessibilityRole::ListBox:
      case AccessibilityRole::TabList:
      case AccessibilityRole::Tree:
      case AccessibilityRole::TreeGrid:
      case AccessibilityRole::List:
      // Containers whose focused or active descendant is the selected one.
      case AccessibilityRole::Menu:
      case AccessibilityRole::MenuBar:
      case AccessibilityRole::ComboBox: return true;
      default:                          return false;
    }
  }

  bool AccessibilityObject::SupportsRowCountChange() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::Tree:
      case AccessibilityRole::TreeGrid:
      case AccessibilityRole::Grid:
      case AccessibilityRole::Table:    return true;
      default:                          return false;
    }
  }

  bool AccessibilityObject::DependsOnTextUnderElement() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::PopUpButton:
        // A select's pop-up button's text is its value, not its name.
        if (auto *element = GetElement(); element && element->LocalName() == u8"select")
        {
          break;
        }
        [[fallthrough]];
      case AccessibilityRole::Summary:
      case AccessibilityRole::Button:
      case AccessibilityRole::ToggleButton:
      case AccessibilityRole::Checkbox:
      case AccessibilityRole::ListBoxOption:
      case AccessibilityRole::ListItem:
      case AccessibilityRole::MenuItem:
      case AccessibilityRole::MenuItemCheckbox:
      case AccessibilityRole::MenuItemRadio:
      case AccessibilityRole::RadioButton:
      case AccessibilityRole::Switch:
      case AccessibilityRole::Tab:              return true;
      default:                                  break;
    }
    // A heading or link reads as one object, named by its text.
    return IsHeading() || IsLink() || _role == AccessibilityRole::ApplicationStatus;
  }

  bool AccessibilityObject::IsRangeControl() const noexcept
  {
    switch (_role)
    {
      case AccessibilityRole::Meter:
      case AccessibilityRole::ProgressIndicator:
      case AccessibilityRole::Slider:
      case AccessibilityRole::ScrollBar:
      case AccessibilityRole::SpinButton:        return true;
      case AccessibilityRole::Splitter:          return CanSetFocusAttribute();
      default:                                   return false;
    }
  }

  AccessibilityOrientation AccessibilityObject::Orientation() const noexcept
  {
    if (auto orientation = ExplicitOrientation())
    {
      return *orientation;
    }

    // ARIA 1.1 made the implicit orientation undefined for the roles without one of their own.
    switch (_role)
    {
      case AccessibilityRole::DescriptionList:
      case AccessibilityRole::List:
      case AccessibilityRole::ListBox:
      case AccessibilityRole::Menu:
      case AccessibilityRole::ScrollBar:
      case AccessibilityRole::Tree:            return AccessibilityOrientation::Vertical;
      case AccessibilityRole::MenuBar:
      case AccessibilityRole::Slider:
      case AccessibilityRole::Splitter:
      case AccessibilityRole::TabList:
      case AccessibilityRole::Toolbar:         return AccessibilityOrientation::Horizontal;
      case AccessibilityRole::ComboBox:
      case AccessibilityRole::RadioGroup:
      case AccessibilityRole::TreeGrid:        return AccessibilityOrientation::Undefined;
      default:                                 break;
    }

    // Lacking concrete evidence of orientation, horizontal means wider than tall.
    auto rect = ElementRect();
    if (rect.Width() > rect.Height())
    {
      return AccessibilityOrientation::Horizontal;
    }
    if (rect.Height() > rect.Width())
    {
      return AccessibilityOrientation::Vertical;
    }
    return AccessibilityOrientation::Undefined;
  }

  bool AccessibilityObject::IsRootWebArea() const noexcept
  {
    // The web area of the root scroll view (or with no scroll view above it, of no parent).
    if (!IsWebArea())
    {
      return false;
    }
    auto *parent = ParentObject();
    return !parent || (parent->IsScrollView() && !parent->ParentObject());
  }

  bool AccessibilityObject::HasAttribute(dom::DOMStringView name) const noexcept
  {
    auto *element = GetElement();
    if (!element)
    {
      return false;
    }
    auto atom = dom::DOMStringAtom(name);
    if (element->HasAttribute(atom))
    {
      return true;
    }
    auto *queue = element->ReactionQueue();
    auto *internals = queue ? queue->AttachedInternals() : nullptr;
    return internals && internals->InternalContentAttribute(atom).has_value();
  }

  dom::DOMString AccessibilityObject::GetAttribute(dom::DOMStringView name) const noexcept
  {
    auto *element = GetElement();
    return element ? AttributeWithDefaultARIA(*element, name) : dom::DOMString {};
  }

  dom::DOMString AccessibilityObject::AttributeWithDefaultARIA(const dom::Element &element,
                                                               dom::DOMStringView name) noexcept
  {
    auto atom = dom::DOMStringAtom(name);
    if (auto value = element.GetAttribute(atom))
    {
      return krys::move(*value);
    }
    // A custom element's default ARIA semantics
    // (https://html.spec.whatwg.org/#the-aria-role-and-states-and-properties-defaults); the internals'
    // content attribute map holds only role and aria-* attributes.
    auto *queue = element.ReactionQueue();
    auto *internals = queue ? queue->AttachedInternals() : nullptr;
    if (!internals)
    {
      return {};
    }
    return internals->InternalContentAttribute(atom).value_or(dom::DOMString {});
  }

  dom::DOMString AccessibilityObject::GetAttributeTrimmed(dom::DOMStringView name) const noexcept
  {
    return dom::DOMString(StringAlgorithms::StripLeadingAndTrailingWhitespace(GetAttribute(name)));
  }

  int32 AccessibilityObject::IntegralAttribute(dom::DOMStringView name) const noexcept
  {
    auto result = html::Numbers::ParseInteger(GetAttribute(name));
    if (!result.Success() || result.Value < std::numeric_limits<int32>::min()
        || result.Value > std::numeric_limits<int32>::max())
    {
      return 0;
    }
    return static_cast<int32>(result.Value);
  }

  dom::DOMStringView AccessibilityObject::DefaultLiveRegionStatusForRole(AccessibilityRole role) noexcept
  {
    switch (role)
    {
      case AccessibilityRole::ApplicationAlertDialog:
      case AccessibilityRole::ApplicationAlert:       return u8"assertive";
      case AccessibilityRole::ApplicationLog:
      case AccessibilityRole::ApplicationStatus:      return u8"polite";
      case AccessibilityRole::ApplicationTimer:
      case AccessibilityRole::ApplicationMarquee:     return u8"off";
      default:                                        return {};
    }
  }

  bool AccessibilityObject::LiveRegionStatusIsEnabled(dom::DOMStringView status) noexcept
  {
    return StringAlgorithms::ASCIICaseInsensitiveMatch(status, u8"polite")
           || StringAlgorithms::ASCIICaseInsensitiveMatch(status, u8"assertive");
  }

  dom::DOMString AccessibilityObject::LiveRegionStatus() const noexcept
  {
    auto status = GetAttribute(u8"aria-live");
    return status.empty() ? dom::DOMString(DefaultLiveRegionStatusForRole(Role())) : status;
  }

  dom::DOMString AccessibilityObject::LiveRegionRelevant() const noexcept
  {
    auto relevant = GetAttribute(u8"aria-relevant");
    return relevant.empty() ? dom::DOMString(u8"additions text") : relevant;
  }

  bool AccessibilityObject::LiveRegionAtomic() const noexcept
  {
    auto atomic = GetAttribute(u8"aria-atomic");
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(atomic, u8"true"))
    {
      return true;
    }
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(atomic, u8"false"))
    {
      return false;
    }
    // Alerts and statuses are atomic unless they say otherwise.
    return Role() == AccessibilityRole::ApplicationAlert || Role() == AccessibilityRole::ApplicationStatus;
  }

  bool AccessibilityObject::IsBusy() const noexcept
  {
    return StringAlgorithms::ASCIICaseInsensitiveMatch(GetAttribute(u8"aria-busy"), u8"true");
  }

  bool AccessibilityObject::SupportsLiveRegion(bool excludeIfOff) const noexcept
  {
    auto status = LiveRegionStatus();
    return excludeIfOff ? LiveRegionStatusIsEnabled(status) : !status.empty();
  }

  AccessibilityObject *AccessibilityObject::LiveRegionAncestor(bool excludeIfOff) const noexcept
  {
    for (auto *object = const_cast<AccessibilityObject *>(this); object; object = object->ParentObject())
    {
      if (object->SupportsLiveRegion(excludeIfOff))
      {
        return object;
      }
    }
    return nullptr;
  }

  bool AccessibilityObject::SupportsARIAAttributes() const noexcept
  {
    // Whether the element has any global ARIA state or property.
    if (SupportsLiveRegion())
    {
      return true;
    }
    for (auto name :
         {u8"aria-atomic", u8"aria-busy", u8"aria-controls", u8"aria-current", u8"aria-describedby",
          u8"aria-details", u8"aria-disabled", u8"aria-dropeffect", u8"aria-errormessage", u8"aria-flowto",
          u8"aria-grabbed", u8"aria-haspopup", u8"aria-invalid", u8"aria-label", u8"aria-labelledby",
          u8"aria-owns", u8"aria-relevant"})
    {
      if (HasAttribute(name))
      {
        return true;
      }
    }
    return false;
  }

  AccessibilityButtonState AccessibilityObject::CheckboxOrRadioValue() const noexcept
  {
    // Native checkboxes and radio buttons are AccessibilityNodeObject's. ARIA checkboxes, radio buttons and
    // switches use aria-checked; toggle buttons use aria-pressed.
    if (IsToggleButton())
    {
      auto pressed = GetAttribute(u8"aria-pressed");
      if (IsAttributeTrue(pressed))
      {
        return AccessibilityButtonState::On;
      }
      if (StringAlgorithms::ASCIICaseInsensitiveMatch(pressed, u8"mixed"))
      {
        return AccessibilityButtonState::Mixed;
      }
      return AccessibilityButtonState::Off;
    }

    auto checked = GetAttribute(u8"aria-checked");
    if (IsAttributeTrue(checked))
    {
      return AccessibilityButtonState::On;
    }
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(checked, u8"mixed"))
    {
      // ARIA says that radio, menuitemradio, and switch elements must not expose button state mixed.
      auto ariaRole = AriaRoleAttribute();
      if (ariaRole == AccessibilityRole::RadioButton || ariaRole == AccessibilityRole::MenuItemRadio
          || ariaRole == AccessibilityRole::Switch)
      {
        return AccessibilityButtonState::Off;
      }
      return AccessibilityButtonState::Mixed;
    }
    return AccessibilityButtonState::Off;
  }

  bool AccessibilityObject::HasPopup() const noexcept
  {
    auto value = GetAttributeTrimmed(u8"aria-haspopup");
    if (value.empty())
    {
      // A combobox has a listbox pop-up by default.
      return _role == AccessibilityRole::ComboBox;
    }
    return !StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false");
  }

  AccessibilityRole AccessibilityObject::ButtonRoleType() const noexcept
  {
    // If aria-pressed is present, then it should be exposed as a toggle button.
    if (!GetAttribute(u8"aria-pressed").empty())
    {
      return AccessibilityRole::ToggleButton;
    }
    if (HasPopup())
    {
      return AccessibilityRole::PopUpButton;
    }
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsLink() && ancestor->HasPopup())
      {
        return AccessibilityRole::PopUpButton;
      }
    }
    return AccessibilityRole::Button;
  }
}
