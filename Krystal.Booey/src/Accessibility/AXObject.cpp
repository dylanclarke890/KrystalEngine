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

#include "Krystal.Booey/Accessibility/AXObject.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleUnits.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/ElementInternals.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <algorithm>
#include <format>
#include <limits>

namespace krys::boo
{
  namespace
  {
    using infra::StringAlgorithms;

    struct AriaRoleEntry
    {
      utf8_stringview Name;
      AXRole Role;
    };

    // clang-format off
    constexpr AriaRoleEntry AriaRoles[] = {
      {u8"alert", AXRole::ApplicationAlert},
      {u8"alertdialog", AXRole::ApplicationAlertDialog},
      {u8"application", AXRole::WebApplication},
      {u8"article", AXRole::DocumentArticle},
      {u8"banner", AXRole::LandmarkBanner},
      {u8"blockquote", AXRole::Blockquote},
      {u8"button", AXRole::Button},
      {u8"caption", AXRole::Caption},
      {u8"code", AXRole::Code},
      {u8"checkbox", AXRole::Checkbox},
      {u8"complementary", AXRole::LandmarkComplementary},
      {u8"contentinfo", AXRole::LandmarkContentInfo},
      {u8"deletion", AXRole::Deletion},
      {u8"dialog", AXRole::ApplicationDialog},
      {u8"directory", AXRole::Directory},
      // The doc-* roles are DPUB-ARIA's: https://w3c.github.io/dpub-aam/
      {u8"doc-abstract", AXRole::TextGroup},
      {u8"doc-acknowledgments", AXRole::LandmarkDocRegion},
      {u8"doc-afterword", AXRole::LandmarkDocRegion},
      {u8"doc-appendix", AXRole::LandmarkDocRegion},
      {u8"doc-backlink", AXRole::Link},
      {u8"doc-biblioentry", AXRole::ListItem},
      {u8"doc-bibliography", AXRole::LandmarkDocRegion},
      {u8"doc-biblioref", AXRole::Link},
      {u8"doc-chapter", AXRole::LandmarkDocRegion},
      {u8"doc-colophon", AXRole::TextGroup},
      {u8"doc-conclusion", AXRole::LandmarkDocRegion},
      {u8"doc-cover", AXRole::Image},
      {u8"doc-credit", AXRole::TextGroup},
      {u8"doc-credits", AXRole::LandmarkDocRegion},
      {u8"doc-dedication", AXRole::TextGroup},
      {u8"doc-endnote", AXRole::ListItem},
      {u8"doc-endnotes", AXRole::LandmarkDocRegion},
      {u8"doc-epigraph", AXRole::TextGroup},
      {u8"doc-epilogue", AXRole::LandmarkDocRegion},
      {u8"doc-errata", AXRole::LandmarkDocRegion},
      {u8"doc-example", AXRole::TextGroup},
      {u8"doc-footnote", AXRole::Footnote},
      {u8"doc-foreword", AXRole::LandmarkDocRegion},
      {u8"doc-glossary", AXRole::LandmarkDocRegion},
      {u8"doc-glossref", AXRole::Link},
      {u8"doc-index", AXRole::LandmarkNavigation},
      {u8"doc-introduction", AXRole::LandmarkDocRegion},
      {u8"doc-noteref", AXRole::Link},
      {u8"doc-notice", AXRole::DocumentNote},
      {u8"doc-pagebreak", AXRole::Splitter},
      {u8"doc-pagelist", AXRole::LandmarkNavigation},
      {u8"doc-part", AXRole::LandmarkDocRegion},
      {u8"doc-preface", AXRole::LandmarkDocRegion},
      {u8"doc-prologue", AXRole::LandmarkDocRegion},
      {u8"doc-pullquote", AXRole::TextGroup},
      {u8"doc-qna", AXRole::TextGroup},
      {u8"doc-subtitle", AXRole::Heading},
      {u8"doc-tip", AXRole::DocumentNote},
      {u8"doc-toc", AXRole::LandmarkNavigation},
      {u8"emphasis", AXRole::Emphasis},
      {u8"figure", AXRole::Figure},
      {u8"generic", AXRole::Generic},
      // The graphics-* roles are Graphics ARIA's: https://w3c.github.io/graphics-aam/
      {u8"graphics-document", AXRole::GraphicsDocument},
      {u8"graphics-object", AXRole::GraphicsObject},
      {u8"graphics-symbol", AXRole::GraphicsSymbol},
      {u8"grid", AXRole::Grid},
      {u8"gridcell", AXRole::GridCell},
      {u8"table", AXRole::Table},
      {u8"cell", AXRole::Cell},
      {u8"columnheader", AXRole::ColumnHeader},
      {u8"combobox", AXRole::ComboBox},
      {u8"definition", AXRole::Definition},
      {u8"document", AXRole::Document},
      {u8"feed", AXRole::Feed},
      {u8"form", AXRole::Form},
      {u8"rowheader", AXRole::RowHeader},
      {u8"group", AXRole::Group},
      {u8"heading", AXRole::Heading},
      // "image" is a synonym of "img": https://w3c.github.io/aria/#image
      {u8"image", AXRole::Image},
      {u8"img", AXRole::Image},
      {u8"insertion", AXRole::Insertion},
      {u8"link", AXRole::Link},
      {u8"list", AXRole::List},
      {u8"listitem", AXRole::ListItem},
      {u8"listbox", AXRole::ListBox},
      {u8"log", AXRole::ApplicationLog},
      {u8"main", AXRole::LandmarkMain},
      {u8"marquee", AXRole::ApplicationMarquee},
      {u8"math", AXRole::DocumentMath},
      {u8"mark", AXRole::Mark},
      {u8"menu", AXRole::Menu},
      {u8"menubar", AXRole::MenuBar},
      {u8"menuitem", AXRole::MenuItem},
      {u8"menuitemcheckbox", AXRole::MenuItemCheckbox},
      {u8"menuitemradio", AXRole::MenuItemRadio},
      {u8"meter", AXRole::Meter},
      {u8"none", AXRole::Presentational},
      {u8"note", AXRole::DocumentNote},
      {u8"navigation", AXRole::LandmarkNavigation},
      {u8"option", AXRole::ListBoxOption},
      {u8"paragraph", AXRole::Paragraph},
      {u8"presentation", AXRole::Presentational},
      {u8"progressbar", AXRole::ProgressIndicator},
      {u8"radio", AXRole::RadioButton},
      {u8"radiogroup", AXRole::RadioGroup},
      {u8"region", AXRole::LandmarkRegion},
      {u8"row", AXRole::Row},
      {u8"rowgroup", AXRole::RowGroup},
      {u8"scrollbar", AXRole::ScrollBar},
      {u8"search", AXRole::LandmarkSearch},
      {u8"searchbox", AXRole::SearchField},
      {u8"sectionfooter", AXRole::SectionFooter},
      {u8"sectionheader", AXRole::SectionHeader},
      {u8"separator", AXRole::Splitter},
      {u8"slider", AXRole::Slider},
      {u8"spinbutton", AXRole::SpinButton},
      {u8"status", AXRole::ApplicationStatus},
      {u8"subscript", AXRole::Subscript},
      {u8"suggestion", AXRole::Suggestion},
      {u8"superscript", AXRole::Superscript},
      {u8"strong", AXRole::Strong},
      {u8"switch", AXRole::Switch},
      {u8"tab", AXRole::Tab},
      {u8"tablist", AXRole::TabList},
      {u8"tabpanel", AXRole::TabPanel},
      {u8"text", AXRole::StaticText},
      {u8"textbox", AXRole::TextField},
      {u8"term", AXRole::Term},
      {u8"time", AXRole::Time},
      {u8"timer", AXRole::ApplicationTimer},
      {u8"toolbar", AXRole::Toolbar},
      {u8"tooltip", AXRole::UserInterfaceTooltip},
      {u8"tree", AXRole::Tree},
      {u8"treegrid", AXRole::TreeGrid},
      {u8"treeitem", AXRole::TreeItem},
    };
    // clang-format on

    /// @brief The ARIA role name each role maps back to, for `ComputedRoleString`.
    KRYS_NODISCARD const HashMap<AXRole, utf8_stringview> &ReverseAriaRoleMap() noexcept
    {
      static const HashMap<AXRole, utf8_stringview> map = []
      {
        HashMap<AXRole, utf8_stringview> result;
        for (const auto &entry : AriaRoles)
        {
          result.insert_or_assign(entry.Role, entry.Name);
        }
        // The synonyms the computed role uses.
        result.insert_or_assign(AXRole::DateTime, u8"textbox");
        result.insert_or_assign(AXRole::TextArea, u8"textbox");
        result.insert_or_assign(AXRole::DescriptionListDetail, u8"definition");
        result.insert_or_assign(AXRole::DescriptionListTerm, u8"term");
        result.insert_or_assign(AXRole::Details, u8"group");
        result.insert_or_assign(AXRole::Image, u8"image");
        result.insert_or_assign(AXRole::ListBoxOption, u8"option");
        result.insert_or_assign(AXRole::MenuListOption, u8"option");
        result.insert_or_assign(AXRole::Presentational, u8"none");
        return result;
      }();
      return map;
    }

    KRYS_NODISCARD utf8_stringview ReverseAriaRole(AXRole role) noexcept
    {
      const auto &map = ReverseAriaRoleMap();
      auto found = map.find(role);
      return found == map.end() ? utf8_stringview {} : found->second;
    }

    KRYS_NODISCARD bool IsARIAInput(AXRole role) noexcept
    {
      switch (role)
      {
        case AXRole::Checkbox:
        case AXRole::RadioButton:
        case AXRole::SearchField:
        case AXRole::Switch:
        case AXRole::TextField:   return true;
        default:                  return false;
      }
    }

    KRYS_NODISCARD bool IsAttributeTrue(DOMStringView value) noexcept
    {
      return StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true");
    }
  }

  utf8_stringview RoleToString(AXRole role) noexcept
  {
    switch (role)
    {
#define KRYS_AX_ROLE_NAME(name)                                                                              \
  case AXRole::name: return u8"" #name;
      KRYS_AX_ROLES(KRYS_AX_ROLE_NAME)
#undef KRYS_AX_ROLE_NAME
    }
    return {};
  }

  AXObject::AXObject(AXId id, AXObjectCache &cache) noexcept : _id(id), _axObjectCache(&cache)
  {
  }

  AXObject::~AXObject() noexcept = default;

  void AXObject::Init() noexcept
  {
    _role = DetermineAccessibilityRole();
  }

  void AXObject::Detach(AXDetachmentType detachmentType) noexcept
  {
    DetachRemoteParts(detachmentType);
    if (auto *cache = _axObjectCache)
    {
      cache->DetachWrapper(*this, detachmentType);
    }
    _axObjectCache = nullptr;
  }

  void AXObject::DetachRemoteParts(AXDetachmentType detachmentType) noexcept
  {
    if (detachmentType == AXDetachmentType::ElementDestroyed)
    {
      PostMenuClosedNotificationIfNecessary();
    }

    // Clear any children and call detachFromParent on them so that no children are left with dangling
    // pointers to their parent.
    ClearChildren();
  }

  void AXObject::PostMenuClosedNotificationIfNecessary() const noexcept
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

  utf8_string AXObject::DebugDescription() const noexcept
  {
    auto description = utf8_string(u8"{role: ") + utf8_string(RoleToString(_role));
    auto id = std::format("{}", static_cast<uint64>(_id));
    description += u8", ID " + utf8_string(id.begin(), id.end());
    if (auto *element = GetElement())
    {
      description += u8", " + element->LocalName().ToString();
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

  Element *AXObject::GetElement() const noexcept
  {
    return DynamicDowncast<Element>(GetNode());
  }

  Document *AXObject::GetDocument() const noexcept
  {
    auto *node = GetNode();
    return node ? &node->NodeDocument() : nullptr;
  }

  Page *AXObject::GetPage() const noexcept
  {
    auto *document = GetDocument();
    return document ? document->GetPage() : nullptr;
  }

  LocalFrameView *AXObject::DocumentFrameView() const noexcept
  {
    auto *document = GetDocument();
    return document ? document->View() : nullptr;
  }

  const RenderStyle *AXObject::Style() const noexcept
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
    return static_cast<Node &>(*element).ComputedStyle();
  }

  AXObject *AXObject::ParentObjectUnignored() const noexcept
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

  const AXObject::AXChildrenVector &AXObject::Children(bool updateChildrenIfNeeded) noexcept
  {
    if (updateChildrenIfNeeded)
    {
      UpdateChildrenIfNecessary();
    }
    return _children;
  }

  void AXObject::UpdateChildrenIfNecessary() noexcept
  {
    if (!_childrenInitialized)
    {
      // Each object's ignored state is cached while the children are added, so a deep tree doesn't work it
      // out again for each ancestor.
      AXAttributeCacheScope cacheScope(_axObjectCache);
      AddChildren();
    }
  }

  void AXObject::ClearChildren() noexcept
  {
    // Some objects have weak pointers to their parents and those associations need to be detached.
    for (const auto &child : _children)
    {
      child->DetachFromParent();
    }

    _children.clear();
    _childrenInitialized = false;
  }

  void AXObject::InsertChild(AXObject &child, uint32 index, DescendIfIgnored descendIfIgnored) noexcept
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

    auto insert = [this](Ref<AXObject> object, uint32 at)
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

  void AXObject::ResetChildrenIndexInParent() noexcept
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

  bool AXObject::IsDescendantOfObject(const AXObject &ancestor) const noexcept
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

  bool AXObject::IsAncestorOfObject(const AXObject &descendant) const noexcept
  {
    return this == &descendant || descendant.IsDescendantOfObject(*this);
  }

  bool AXObject::IsDescendantOfRole(AXRole role) const noexcept
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

  bool AXObject::IsIgnored() const noexcept
  {
    auto *cache = _axObjectCache;
    if (cache)
    {
      if (auto cached = cache->CachedIgnoredState(_id))
      {
        return *cached;
      }
    }
    bool ignored = IsIgnoredWithoutCache();
    // (Working it out may have started the cache.)
    if (cache)
    {
      cache->CacheIgnoredState(_id, ignored);
    }
    return ignored;
  }

  bool AXObject::IsIgnoredWithoutCache() const noexcept
  {
    // While the modal dialog is being worked out, whether an object is ignored can't depend on it.
    bool ignored =
      _axObjectCache && !_axObjectCache->IsRetrievingCurrentModalNode() && IgnoredFromModalPresence();
    if (!ignored)
    {
      ignored = ComputeIsIgnored();
    }

    auto previous = _lastKnownIsIgnoredValue;
    const_cast<AXObject *>(this)->_lastKnownIsIgnoredValue =
      ignored ? AXObjectInclusion::IgnoreObject : AXObjectInclusion::IncludeObject;

    if (auto *cache = _axObjectCache)
    {
      bool becameUnignored = previous == AXObjectInclusion::IgnoreObject && !ignored;
      bool becameIgnored = previous == AXObjectInclusion::IncludeObject && ignored;
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

  bool AXObject::IsModalDescendant(const Node &modalNode) const noexcept
  {
    for (const Node *ancestor = GetNode(); ancestor; ancestor = ancestor->ParentInComposedTree())
    {
      if (ancestor == &modalNode)
      {
        return true;
      }
    }
    return false;
  }

  bool AXObject::IgnoredFromModalPresence() const noexcept
  {
    // The top node isn't ignored.
    auto *node = GetNode();
    if (!node || !node->ParentNode() || !_axObjectCache)
    {
      return false;
    }
    auto *modalNode = _axObjectCache->ModalNode();
    if (!modalNode)
    {
      return false;
    }

    // Objects outside the dialog that are tied to something in it aren't ignored.
    for (auto *ancestor = this; ancestor; ancestor = ancestor->ParentObject())
    {
      for (const auto &controller : ancestor->Controllers())
      {
        if (controller->IsModalDescendant(*modalNode))
        {
          return false;
        }
      }
      for (const auto &activeDescendant : ancestor->ActiveDescendantOfObjects())
      {
        if (activeDescendant->IsModalDescendant(*modalNode))
        {
          return false;
        }
      }
    }

    // Nor is something stacked above the dialog.
    auto nearestRenderer = [](const Node &start) -> RenderObject *
    {
      for (auto *ancestor = &start; ancestor; ancestor = ancestor->ParentInComposedTree())
      {
        if (auto *renderer = ancestor->Renderer())
        {
          return renderer;
        }
      }
      return nullptr;
    };
    auto zIndex = [](RenderObject &renderer)
    {
      for (auto *layer = renderer.EnclosingLayer(); layer; layer = layer->Parent())
      {
        if (auto value = layer->ZIndex())
        {
          return value;
        }
      }
      return 0;
    };
    auto *renderer = nearestRenderer(*node);
    auto *modalRenderer = nearestRenderer(*modalNode);
    if (renderer && modalRenderer && zIndex(*renderer) > zIndex(*modalRenderer))
    {
      return false;
    }

    return !IsModalDescendant(*modalNode);
  }

  void AXObject::RecomputeIsIgnoredForDescendants(bool includeSelf) noexcept
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

  AXObjectInclusion AXObject::DefaultObjectInclusion() const noexcept
  {
    if (const auto *style = Style())
    {
      if (style->EffectiveInert())
      {
        return AXObjectInclusion::IgnoreObject;
      }
      if (style->UsedVisibility() != Visibility::Visible)
      {
        return AXObjectInclusion::IgnoreObject;
      }
    }

    if (IsARIAHidden())
    {
      return AXObjectInclusion::IgnoreObject;
    }

    bool ignoreARIAHidden = IsFocused();
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      // Not isRenderHidden(), which also checks visibility: a visibility: visible descendant of a
      // visibility: hidden ancestor is visible, and the object's own visibility is checked above.
      if (const auto *style = ancestor->Style(); style && style->EffectiveDisplay() == DisplayType::None)
      {
        return AXObjectInclusion::IgnoreObject;
      }
      if ((!ignoreARIAHidden && ancestor->IsARIAHidden()) || ancestor->AriaRoleHasPresentationalChildren()
          || !ancestor->CanHaveChildren())
      {
        return AXObjectInclusion::IgnoreObject;
      }
    }

    // Include <dialog> elements and elements with role="dialog".
    if (_role == AXRole::ApplicationDialog)
    {
      return AXObjectInclusion::IncludeObject;
    }
    return AXObjectInclusion::DefaultBehavior;
  }

  bool AXObject::IsARIAHidden() const noexcept
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
      if ((Is<HTMLElement>(*element) && (localName == u8"body" || localName == u8"html"))
          || (Is<SVGElement>(*element) && localName == u8"svg" && !element->ParentNode()))
      {
        return false;
      }
    }

    if (auto *slot = node ? dom::SlotAlgorithms::GetAssignedSlot(*node) : nullptr)
    {
      auto *slotObject = _axObjectCache ? _axObjectCache->Get(*slot) : nullptr;
      if (slotObject ? IsAttributeTrue(slotObject->GetAttribute(u8"aria-hidden"))
                     : IsAttributeTrue(slot->GetAttribute(u8"aria-hidden").value_or(DOMString {})))
      {
        return true;
      }
    }
    return element && IsAttributeTrue(GetAttribute(u8"aria-hidden"));
  }

  bool AXObject::IsAXHidden() const noexcept
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

  void AXObject::UpdateRole() noexcept
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

  AXRole AXObject::AriaRoleToRole(DOMStringView value) noexcept
  {
    return AriaRoleToRole(value, [](AXRole) { return false; });
  }

  AXRole AXObject::AriaRoleToRole(DOMStringView value, const Func<bool(AXRole)> &skipRole) noexcept
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
    return AXRole::Unknown;
  }

  utf8_stringview AXObject::ComputedRoleString() const noexcept
  {
    if (_role == AXRole::Image && IsIgnored())
    {
      return ReverseAriaRole(AXRole::Presentational);
    }

    // A role string is computed for block elements with author-provided roles.
    auto ariaRole = AriaRoleAttribute();
    if (ariaRole == AXRole::TextGroup || _role == AXRole::Footnote || _role == AXRole::GraphicsObject)
    {
      return ReverseAriaRole(AXRole::Group);
    }

    // No role string is computed for generic block elements with user-agent assigned roles.
    switch (_role)
    {
      case AXRole::TextGroup:         return {};
      case AXRole::GraphicsDocument:  return ReverseAriaRole(AXRole::Document);
      case AXRole::GraphicsSymbol:    return ReverseAriaRole(AXRole::Image);
      case AXRole::HorizontalRule:    return ReverseAriaRole(AXRole::Splitter);
      case AXRole::PopUpButton:
      case AXRole::ToggleButton:      return ReverseAriaRole(AXRole::Button);
      case AXRole::LandmarkDocRegion: return ReverseAriaRole(AXRole::LandmarkRegion);
      default:                        break;
    }
    // A header cell is a cell whose role says which kind of header it is.
    if (IsColumnHeader())
    {
      return ReverseAriaRole(AXRole::ColumnHeader);
    }
    if (IsRowHeader())
    {
      return ReverseAriaRole(AXRole::RowHeader);
    }
    return ReverseAriaRole(_role);
  }

  bool AXObject::AriaRoleHasPresentationalChildren() const noexcept
  {
    switch (AriaRoleAttribute())
    {
      case AXRole::Button:
      case AXRole::Slider:
      case AXRole::Image:
      case AXRole::ProgressIndicator:
      case AXRole::SpinButton:        return true;
      default:                        return false;
    }
  }

  bool AXObject::IsARIAControl(AXRole role) noexcept
  {
    if (IsARIAInput(role))
    {
      return true;
    }
    switch (role)
    {
      case AXRole::Button:
      case AXRole::ComboBox:
      case AXRole::ListBox:
      case AXRole::PopUpButton:
      case AXRole::Slider:
      case AXRole::TextArea:
      case AXRole::ToggleButton: return true;
      default:                   return false;
    }
  }

  bool AXObject::IsMenuItem() const noexcept
  {
    switch (_role)
    {
      case AXRole::MenuItem:
      case AXRole::MenuItemRadio:
      case AXRole::MenuItemCheckbox: return true;
      default:                       return false;
    }
  }

  bool AXObject::IsMenuRelated() const noexcept
  {
    return IsMenu() || IsMenuBar() || IsMenuItem();
  }

  bool AXObject::IsButton() const noexcept
  {
    switch (_role)
    {
      case AXRole::Button:
      case AXRole::PopUpButton:
      case AXRole::ToggleButton: return true;
      default:                   return false;
    }
  }

  bool AXObject::IsControl() const noexcept
  {
    switch (_role)
    {
      case AXRole::Button:
      case AXRole::Checkbox:
      case AXRole::ColorWell:
      case AXRole::ComboBox:
      case AXRole::DateTime:
      case AXRole::LandmarkSearch:
      case AXRole::ListBox:
      case AXRole::PopUpButton:
      case AXRole::RadioButton:
      case AXRole::SearchField:
      case AXRole::Slider:
      case AXRole::SliderThumb:
      case AXRole::Switch:
      case AXRole::TextArea:
      case AXRole::TextField:
      case AXRole::ToggleButton:   return true;
      default:
      {
        // A fieldset is a control too.
        auto *element = GetElement();
        return element && Is<HTMLElement>(*element) && element->LocalName() == u8"fieldset";
      }
    }
  }

  bool AXObject::IsTextControl() const noexcept
  {
    switch (_role)
    {
      case AXRole::ComboBox:
      case AXRole::SearchField:
      case AXRole::TextArea:
      case AXRole::TextField:   return true;
      default:                  return false;
    }
  }

  bool AXObject::IsImageMapLink() const noexcept
  {
    auto *element = DynamicDowncast<HTMLElement>(GetElement());
    return element && element->TagQualifiedName().TagName() == html::TagName::area;
  }

  bool AXObject::HasCellRole() const noexcept
  {
    return _role == AXRole::Cell || _role == AXRole::GridCell || _role == AXRole::ColumnHeader
           || _role == AXRole::RowHeader;
  }

  bool AXObject::HasRowGroupTag() const noexcept
  {
    auto *element = DynamicDowncast<HTMLElement>(GetElement());
    if (!element)
    {
      return false;
    }
    auto tagName = element->TagQualifiedName().TagName();
    return tagName == html::TagName::thead || tagName == html::TagName::tbody
           || tagName == html::TagName::tfoot;
  }

  AXObject *AXObject::ExposedTableAncestor(bool includeSelf) const noexcept
  {
    for (auto *ancestor = includeSelf ? const_cast<AXObject *>(this) : ParentObject(); ancestor;
         ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsExposableTable())
      {
        return ancestor;
      }
    }
    return nullptr;
  }

  AXObject *AXObject::ColumnHeader() noexcept
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

  AXObject *AXObject::RowHeader() noexcept
  {
    const auto &rowChildren = Children();
    if (rowChildren.empty())
    {
      return nullptr;
    }

    auto isTH = [](const AXObject &object)
    {
      auto *element = DynamicDowncast<HTMLElement>(object.GetElement());
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

  AXObject::AXChildrenVector AXObject::ColumnHeaders() noexcept
  {
    AXChildrenVector headers;
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

  Maybe<AXId> AXObject::RowGroupAncestorID() const noexcept
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

  bool AXObject::IsTableCellInSameRowGroup(const AXObject &otherTableCell) const noexcept
  {
    auto ancestorID = RowGroupAncestorID();
    return ancestorID && ancestorID == otherTableCell.RowGroupAncestorID();
  }

  bool AXObject::IsGroup() const noexcept
  {
    return _role == AXRole::Group || _role == AXRole::TextGroup;
  }

  bool AXObject::IsLandmark() const noexcept
  {
    switch (_role)
    {
      case AXRole::Form:
      case AXRole::LandmarkBanner:
      case AXRole::LandmarkComplementary:
      case AXRole::LandmarkContentInfo:
      case AXRole::LandmarkDocRegion:
      case AXRole::LandmarkMain:
      case AXRole::LandmarkNavigation:
      case AXRole::LandmarkRegion:
      case AXRole::LandmarkSearch:        return true;
      default:                            return false;
    }
  }

  AXObject::AXChildrenVector AXObject::RelatedObjects(AXRelation relation) const noexcept
  {
    auto *cache = AxObjectCache();
    auto ids = cache ? cache->RelatedObjectIDsFor(*this, relation) : null;
    if (!ids)
    {
      return {};
    }
    AXChildrenVector objects;
    for (auto id : *ids)
    {
      if (auto *object = cache->ObjectForID(id))
      {
        objects.push_back(ShareRef(*object));
      }
    }
    return objects;
  }

  AXObject *AXObject::ActiveDescendant() const noexcept
  {
    auto activeDescendants = RelatedObjects(AXRelation::ActiveDescendant);
    krys_debug_assert(activeDescendants.size() <= 1);
    return activeDescendants.empty() ? nullptr : activeDescendants[0].get();
  }

  bool AXObject::ShouldFocusActiveDescendant() const noexcept
  {
    switch (AriaRoleAttribute())
    {
      case AXRole::Group:
      case AXRole::ListBox:
      case AXRole::Menu:
      case AXRole::MenuBar:
      case AXRole::RadioGroup:
      case AXRole::Row:
      case AXRole::PopUpButton:
      case AXRole::Meter:
      case AXRole::ProgressIndicator:
      case AXRole::Toolbar:
      case AXRole::Tree:
      case AXRole::Grid:              return true;
      default:                        return false;
    }
  }

  bool AXObject::SupportsActiveDescendant() const noexcept
  {
    switch (_role)
    {
      case AXRole::ComboBox:
      case AXRole::Grid:
      case AXRole::List:
      case AXRole::ListBox:
      case AXRole::TextArea:
      case AXRole::TextField:
      case AXRole::Tree:
      case AXRole::TreeGrid:  return true;
      default:                return false;
    }
  }

  bool AXObject::CanHaveSelectedChildren() const noexcept
  {
    switch (_role)
    {
      // Containers whose children support aria-selected.
      case AXRole::Grid:
      case AXRole::ListBox:
      case AXRole::TabList:
      case AXRole::Tree:
      case AXRole::TreeGrid:
      case AXRole::List:
      // Containers whose focused or active descendant is the selected one.
      case AXRole::Menu:
      case AXRole::MenuBar:
      case AXRole::ComboBox:
      // A menu list's pop-up's selected option (as on WebKit's ATSPI port).
      case AXRole::MenuListPopup: return true;
      default:                    return false;
    }
  }

  AXObject *AXObject::ClickableSelfOrAncestor(bool includeBody) const noexcept
  {
    for (auto *object = const_cast<AXObject *>(this); object; object = object->ParentObject())
    {
      if (!includeBody)
      {
        if (auto *element = object->GetElement(); element && element->LocalName() == u8"body")
        {
          return nullptr;
        }
      }
      if (object->HasClickHandler())
      {
        // Presentational objects can't be clicked.
        return object->Role() == AXRole::Presentational ? nullptr : object;
      }
    }
    return nullptr;
  }

  AXObject::AXChildrenVector AXObject::SelectedChildren() noexcept
  {
    if (!CanHaveSelectedChildren())
    {
      return {};
    }
    auto single = [](AXObject *object) -> AXChildrenVector
    {
      if (!object)
      {
        return {};
      }
      return {ShareRef(*object)};
    };
    auto selectedOf = [this](auto &&include, bool stopAtFirst)
    {
      AXChildrenVector selected;
      for (const auto &child : Children())
      {
        if (include(*child))
        {
          selected.push_back(child);
          if (stopAtFirst)
          {
            break;
          }
        }
      }
      return selected;
    };

    switch (_role)
    {
      case AXRole::ComboBox: return single(ActiveDescendant());
      case AXRole::ListBox:
        return selectedOf([](AXObject &child)
                          { return child.Role() == AXRole::ListBoxOption && child.IsSelected(); },
                          !IsMultiSelectable());
      case AXRole::Grid:
      case AXRole::Tree:
      case AXRole::TreeGrid:
      {
        bool isMulti = IsMultiSelectable();
        AXChildrenVector selected;
        // The active descendant comes before the rows with aria-selected.
        if (auto *activeDescendant = ActiveDescendant();
            activeDescendant && (activeDescendant->IsTreeItem() || activeDescendant->IsExposedTableRow()))
        {
          selected.push_back(ShareRef(*activeDescendant));
          if (!isMulti)
          {
            return selected;
          }
        }
        AXChildrenVector rows;
        if (IsTree())
        {
          rows = AriaTreeRows();
        }
        else if (IsExposableTable() && HasGridRole())
        {
          rows = Rows();
        }
        for (auto &row : rows)
        {
          if ((row->IsSelected() || row->IsActiveDescendantOfFocusedContainer())
              && std::ranges::none_of(selected, [&](const auto &item) { return item.get() == row.get(); }))
          {
            selected.push_back(row);
            if (!isMulti)
            {
              break;
            }
          }
        }
        return selected;
      }
      case AXRole::TabList: return single(SelectedTabItem());
      case AXRole::List:
        return selectedOf([](AXObject &child) { return child.IsListItem() && child.IsSelected(); }, false);
      case AXRole::Menu:
      case AXRole::MenuBar:
        if (auto *activeDescendant = ActiveDescendant())
        {
          return single(activeDescendant);
        }
        return single(AxObjectCache() ? AxObjectCache()->FocusedObject() : nullptr);
      case AXRole::MenuListPopup:
        return selectedOf([](AXObject &child) { return child.IsSelected(); }, false);
      default: return {};
    }
  }

  AXObject *AXObject::SelectedRadioButton() noexcept
  {
    if (!IsRadioGroup())
    {
      return nullptr;
    }
    for (const auto &child : Children())
    {
      if (child->IsRadioButton() && child->CheckboxOrRadioValue() == AXButtonState::On)
      {
        return child.get();
      }
    }
    return nullptr;
  }

  AXObject *AXObject::SelectedTabItem() noexcept
  {
    if (!IsTabList())
    {
      return nullptr;
    }
    for (const auto &child : Children())
    {
      if (child->IsTabItem() && (child->IsChecked() || child->IsSelected()))
      {
        return child.get();
      }
    }
    return nullptr;
  }

  namespace
  {
    void AddAriaTreeRows(AXObject &object, AXObject::AXChildrenVector &rows,
                         List<AXObject *> &ancestors) noexcept
    {
      auto contains = [](const auto &list, const AXObject &item)
      {
        return std::ranges::any_of(list, [&](const auto &entry) { return &*entry == &item; });
      };
      auto ownedObjects = object.OwnedObjects();
      ancestors.push_back(&object);

      // The children not in aria-owns come first, then those it lists.
      for (const auto &child : object.Children())
      {
        if (child->IsTreeItem() && !contains(ownedObjects, *child) && !contains(rows, *child))
        {
          rows.push_back(child);
        }
        // Rows can be inside the child (in a group, or a tree item's subtree).
        AddAriaTreeRows(*child, rows, ancestors);
      }
      for (const auto &child : ownedObjects)
      {
        // aria-owns can make a cycle.
        if (contains(ancestors, *child))
        {
          continue;
        }
        if (child->IsTreeItem())
        {
          // An owned item listed earlier at the top of the tree moves to where aria-owns puts it.
          std::erase_if(rows, [&](const auto &row) { return row.get() == child.get(); });
          rows.push_back(child);
        }
        AddAriaTreeRows(*child, rows, ancestors);
      }
      ancestors.pop_back();
    }
  }

  AXObject::AXChildrenVector AXObject::AriaTreeRows() noexcept
  {
    AXChildrenVector rows;
    List<AXObject *> ancestors;
    AddAriaTreeRows(*this, rows, ancestors);
    return rows;
  }

  AXObject::AXChildrenVector AXObject::AriaTreeItemContent() noexcept
  {
    AXChildrenVector content;
    for (const auto &child : Children())
    {
      if (!child->IsGroup() && !child->IsTreeItem())
      {
        content.push_back(child);
      }
    }
    return content;
  }

  bool AXObject::IsActiveDescendantOfFocusedContainer() const noexcept
  {
    for (const auto &container : ActiveDescendantOfObjects())
    {
      if (container->IsFocused())
      {
        return true;
      }
    }
    return false;
  }

  bool AXObject::IsTabItemSelected() const noexcept
  {
    // ARIA: a tab is also selected when the focus is inside a tab panel it controls.
    auto *cache = AxObjectCache();
    if (!IsTabItem() || !cache)
    {
      return false;
    }
    auto *focused = cache->FocusedObject();
    if (!focused)
    {
      return false;
    }
    for (const auto &tabPanel : ControlledObjects())
    {
      if (tabPanel->Role() != AXRole::TabPanel)
      {
        continue;
      }
      for (auto *object = focused; object; object = object->ParentObject())
      {
        if (object == tabPanel.get())
        {
          return true;
        }
      }
    }
    return false;
  }

  bool AXObject::SupportsRowCountChange() const noexcept
  {
    switch (_role)
    {
      case AXRole::Tree:
      case AXRole::TreeGrid:
      case AXRole::Grid:
      case AXRole::Table:    return true;
      default:               return false;
    }
  }

  bool AXObject::DependsOnTextUnderElement() const noexcept
  {
    switch (_role)
    {
      case AXRole::PopUpButton:
        // A select's pop-up button's text is its value, not its name.
        if (auto *element = GetElement(); element && element->LocalName() == u8"select")
        {
          break;
        }
        [[fallthrough]];
      case AXRole::Summary:
      case AXRole::Button:
      case AXRole::ToggleButton:
      case AXRole::Checkbox:
      case AXRole::ListBoxOption:
      case AXRole::ListItem:
      case AXRole::MenuItem:
      case AXRole::MenuItemCheckbox:
      case AXRole::MenuItemRadio:
      case AXRole::RadioButton:
      case AXRole::Switch:
      case AXRole::Tab:              return true;
      default:                       break;
    }
    // A heading or link reads as one object, named by its text.
    return IsHeading() || IsLink() || _role == AXRole::ApplicationStatus;
  }

  bool AXObject::IsRangeControl() const noexcept
  {
    switch (_role)
    {
      case AXRole::Meter:
      case AXRole::ProgressIndicator:
      case AXRole::Slider:
      case AXRole::ScrollBar:
      case AXRole::SpinButton:        return true;
      case AXRole::Splitter:          return CanSetFocusAttribute();
      default:                        return false;
    }
  }

  AXOrientation AXObject::Orientation() const noexcept
  {
    if (auto orientation = ExplicitOrientation())
    {
      return *orientation;
    }

    // ARIA 1.1 made the implicit orientation undefined for the roles without one of their own.
    switch (_role)
    {
      case AXRole::DescriptionList:
      case AXRole::List:
      case AXRole::ListBox:
      case AXRole::Menu:
      case AXRole::ScrollBar:
      case AXRole::Tree:            return AXOrientation::Vertical;
      case AXRole::MenuBar:
      case AXRole::Slider:
      case AXRole::Splitter:
      case AXRole::TabList:
      case AXRole::Toolbar:         return AXOrientation::Horizontal;
      case AXRole::ComboBox:
      case AXRole::RadioGroup:
      case AXRole::TreeGrid:        return AXOrientation::Undefined;
      default:                      break;
    }

    // Lacking concrete evidence of orientation, horizontal means wider than tall.
    auto rect = ElementRect();
    if (rect.Width() > rect.Height())
    {
      return AXOrientation::Horizontal;
    }
    if (rect.Height() > rect.Width())
    {
      return AXOrientation::Vertical;
    }
    return AXOrientation::Undefined;
  }

  bool AXObject::IsRootWebArea() const noexcept
  {
    // The web area of the root scroll view (or with no scroll view above it, of no parent).
    if (!IsWebArea())
    {
      return false;
    }
    auto *parent = ParentObject();
    return !parent || (parent->IsScrollView() && !parent->ParentObject());
  }

  bool AXObject::HasAttribute(DOMStringView name) const noexcept
  {
    auto *element = GetElement();
    if (!element)
    {
      return false;
    }
    auto atom = DOMStringAtom(name);
    if (element->HasAttribute(atom))
    {
      return true;
    }
    auto *queue = element->ReactionQueue();
    auto *internals = queue ? queue->AttachedInternals() : nullptr;
    return internals && internals->InternalContentAttribute(atom).has_value();
  }

  DOMString AXObject::GetAttribute(DOMStringView name) const noexcept
  {
    auto *element = GetElement();
    return element ? AttributeWithDefaultARIA(*element, name) : DOMString {};
  }

  DOMString AXObject::AttributeWithDefaultARIA(const Element &element, DOMStringView name) noexcept
  {
    auto atom = DOMStringAtom(name);
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
    return internals->InternalContentAttribute(atom).value_or(DOMString {});
  }

  DOMString AXObject::GetAttributeTrimmed(DOMStringView name) const noexcept
  {
    return DOMString(StringAlgorithms::StripLeadingAndTrailingWhitespace(GetAttribute(name)));
  }

  int32 AXObject::IntegralAttribute(DOMStringView name) const noexcept
  {
    auto result = html::Numbers::ParseInteger(GetAttribute(name));
    if (!result.Success() || result.Value < std::numeric_limits<int32>::min()
        || result.Value > std::numeric_limits<int32>::max())
    {
      return 0;
    }
    return static_cast<int32>(result.Value);
  }

  IntPoint AXObject::ClickPoint() noexcept
  {
    if (IsHeading())
    {
      const auto &children = Children();
      if (children.size() == 1)
      {
        return children[0]->ClickPoint();
      }
    }
    return RoundedIntPoint(ElementRect().Center());
  }

  AXObject *AXObject::AccessibilityHitTest(const IntPoint &point) const noexcept
  {
    return ElementAccessibilityHitTest(point);
  }

  AXObject *AXObject::ElementAccessibilityHitTest(const IntPoint &point) const noexcept
  {
    // The objects an object makes itself have no renderer for a hit test to find.
    for (const auto &child : const_cast<AXObject *>(this)->Children(false))
    {
      if (child->IsMockObject() && child->ElementRect().Contains(LayoutPoint(point)))
      {
        return child->ElementAccessibilityHitTest(point);
      }
    }
    return const_cast<AXObject *>(this);
  }

  bool AXObject::Press() noexcept
  {
    // Pressing does nothing without an element to act on.
    RefPtr<Element> actionElement = ShareRefPtr(ActionElement());
    if (!actionElement)
    {
      return false;
    }

    // The element under the click point, if it's inside the one pressed, takes the press.
    RefPtr<Element> hitTestElement;
    RefPtr<Document> document = ShareRefPtr(GetDocument());
    if (document)
    {
      HitTestResult result {LayoutPoint(ClickPoint())};
      (void)document->HitTest(HitTestRequest(EnumSet<HitTestRequest::Type> {
                                HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
                                HitTestRequest::Type::AccessibilityHitTest}),
                              result);
      if (auto *innerNode = result.InnerNode())
      {
        if (auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(*innerNode))
        {
          hitTestElement = ShareRefPtr(shadowRoot->Host());
        }
        else if (auto *element = DynamicDowncast<Element>(*innerNode))
        {
          hitTestElement = ShareRefPtr(element);
        }
        else
        {
          hitTestElement = ShareRefPtr(innerNode->ParentElement());
        }
      }
    }

    // The action element is pressed if it's inside the object's element.
    RefPtr<Element> pressElement = ShareRefPtr(GetElement());
    if (!pressElement || actionElement->IsDescendantOf(*pressElement))
    {
      pressElement = actionElement;
    }
    if (hitTestElement && hitTestElement->IsDescendantOf(*pressElement))
    {
      pressElement = hitTestElement;
    }

    // A press is the user's activation.
    if (auto *window = document ? document->Window() : nullptr)
    {
      window->NotifyActivated();
    }
    return pressElement->AccessKeyAction(true)
           || pressElement->DispatchSimulatedClick(
             nullptr, dom::SimulatedClickMouseEventOptions::SendMouseUpDownEvents);
  }

  void AXObject::SetFocused(bool focused) noexcept
  {
    if (!focused)
    {
      return;
    }
    // Focusing anything in the page needs the page focused and active.
    auto *document = GetDocument();
    if (!document || document->IsFocusedAndActive())
    {
      return;
    }
    if (auto *page = document->GetPage())
    {
      page->GetChromeClient().Focus();
    }
  }

  bool AXObject::SetValue(float value) noexcept
  {
    auto text = std::format("{}", value);
    return SetValue(DOMStringView(reinterpret_cast<const char8_t *>(text.data()), text.size()));
  }

  void AXObject::ScrollToMakeVisible() const noexcept
  {
    if (IsScrollView())
    {
      if (auto *parent = ParentObject())
      {
        parent->ScrollToMakeVisible();
      }
    }
    if (auto *renderer = Renderer())
    {
      (void)LocalFrameView::ScrollRectToVisible(ElementRect(), *renderer, false, {});
    }
  }

  DOMString AXObject::StringForVisiblePositionRange(const VisiblePositionRange &range) noexcept
  {
    auto simpleRange = MakeSimpleRange(range);
    return simpleRange ? PlainText(*simpleRange) : DOMString {};
  }

  int32 AXObject::Index(const VisiblePosition &position) const noexcept
  {
    auto *node = GetNode();
    if (position.IsNull() || !IsTextControl() || !node)
    {
      return -1;
    }
    auto *positionNode = position.DeepEquivalent().DeprecatedNode();
    if (!positionNode || !dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*positionNode, *node))
    {
      return -1;
    }
    return IndexForVisiblePosition(position);
  }

  DOMString AXObject::DoAXStringForRange(const CharacterRange &range) const noexcept
  {
    if (!range.Length || !IsTextControl())
    {
      return {};
    }
    auto text = Text();
    if (range.Location >= text.size())
    {
      return {};
    }
    return text.substr(range.Location, range.Length);
  }

  namespace
  {
    /// @brief Whether @p position is at the end of a line that ends with a line break (a `br` or a newline).
    KRYS_NODISCARD bool IsHardLineBreak(const VisiblePosition &position) noexcept
    {
      if (!IsEndOfLine(position))
      {
        return false;
      }
      auto lineBreakRange = MakeSimpleRange(VisiblePositionRange {position, position.Next()});
      if (!lineBreakRange)
      {
        return false;
      }
      TextIterator iterator(*lineBreakRange);
      if (iterator.AtEnd())
      {
        return false;
      }
      if (auto *element = DynamicDowncast<HTMLElement>(iterator.GetNode());
          element && Is<HTMLBRElement>(*element))
      {
        return true;
      }
      if (iterator.GetNode() != position.DeepEquivalent().AnchorNode())
      {
        return false;
      }
      return iterator.Text() == u8"\n";
    }
  }

  CharacterRange AXObject::DoAXRangeForLine(uint32 lineNumber) const noexcept
  {
    if (!IsTextControl())
    {
      return {};
    }
    auto lineStart = VisiblePositionForIndex(0);
    for (auto lineCount = lineNumber; lineCount; --lineCount)
    {
      auto nextLineStart = NextLinePosition(lineStart, 0);
      if (nextLineStart.IsNull() || nextLineStart == lineStart)
      {
        return {};
      }
      lineStart = nextLineStart;
    }
    auto lineEnd = EndOfLine(lineStart);
    auto lineStartIndex = IndexForVisiblePosition(lineStart);
    auto lineEndIndex = IndexForVisiblePosition(lineEnd);
    if (IsHardLineBreak(lineEnd))
    {
      ++lineEndIndex;
    }
    if (lineStartIndex < 0 || lineEndIndex <= lineStartIndex)
    {
      return {};
    }
    return {static_cast<uint64>(lineStartIndex), static_cast<uint64>(lineEndIndex - lineStartIndex)};
  }

  int32 AXObject::LineForPosition(const VisiblePosition &position) const noexcept
  {
    auto *node = GetNode();
    auto *containerNode = position.IsNull() ? nullptr : position.DeepEquivalent().GetContainerNode();
    if (!node || !containerNode)
    {
      return -1;
    }
    // The position must be in the object's editable region.
    if (!dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*node, *containerNode)
        && !dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*containerNode, *node))
    {
      return -1;
    }
    // Text box roles count as editable for the lines.
    int32 lineCount = -1;
    auto current = position;
    VisiblePosition previous;
    do
    {
      previous = current;
      current = PreviousLinePosition(current, 0, EditableType::HasEditableAXRole);
      ++lineCount;
    } while (current.IsNotNull() && !InSameLine(current, previous));
    return lineCount;
  }

  CharacterRange
    AXObject::PlainTextRangeForVisiblePositionRange(const VisiblePositionRange &range) const noexcept
  {
    auto start = Index(range.Start);
    auto end = Index(range.End);
    if (start < 0 || end < 0 || start > end)
    {
      return {};
    }
    return {static_cast<uint64>(start), static_cast<uint64>(end - start)};
  }

  VisiblePositionRange AXObject::VisiblePositionRangeForRange(const CharacterRange &range) const noexcept
  {
    if (range.Location + range.Length > TextLength())
    {
      return {};
    }
    auto start = VisiblePositionForIndex(static_cast<int32>(range.Location));
    start.SetAffinity(Affinity::Downstream);
    return {start, VisiblePositionForIndex(static_cast<int32>(range.Location + range.Length))};
  }

  DOMStringView AXObject::DefaultLiveRegionStatusForRole(AXRole role) noexcept
  {
    switch (role)
    {
      case AXRole::ApplicationAlertDialog:
      case AXRole::ApplicationAlert:       return u8"assertive";
      case AXRole::ApplicationLog:
      case AXRole::ApplicationStatus:      return u8"polite";
      case AXRole::ApplicationTimer:
      case AXRole::ApplicationMarquee:     return u8"off";
      default:                             return {};
    }
  }

  bool AXObject::LiveRegionStatusIsEnabled(DOMStringView status) noexcept
  {
    return StringAlgorithms::ASCIICaseInsensitiveMatch(status, u8"polite")
           || StringAlgorithms::ASCIICaseInsensitiveMatch(status, u8"assertive");
  }

  DOMString AXObject::LiveRegionStatus() const noexcept
  {
    auto status = GetAttribute(u8"aria-live");
    return status.empty() ? DOMString(DefaultLiveRegionStatusForRole(Role())) : status;
  }

  DOMString AXObject::LiveRegionRelevant() const noexcept
  {
    auto relevant = GetAttribute(u8"aria-relevant");
    return relevant.empty() ? DOMString(u8"additions text") : relevant;
  }

  bool AXObject::LiveRegionAtomic() const noexcept
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
    return Role() == AXRole::ApplicationAlert || Role() == AXRole::ApplicationStatus;
  }

  bool AXObject::IsBusy() const noexcept
  {
    return StringAlgorithms::ASCIICaseInsensitiveMatch(GetAttribute(u8"aria-busy"), u8"true");
  }

  bool AXObject::SupportsLiveRegion(bool excludeIfOff) const noexcept
  {
    auto status = LiveRegionStatus();
    return excludeIfOff ? LiveRegionStatusIsEnabled(status) : !status.empty();
  }

  AXObject *AXObject::LiveRegionAncestor(bool excludeIfOff) const noexcept
  {
    for (auto *object = const_cast<AXObject *>(this); object; object = object->ParentObject())
    {
      if (object->SupportsLiveRegion(excludeIfOff))
      {
        return object;
      }
    }
    return nullptr;
  }

  bool AXObject::SupportsARIAAttributes() const noexcept
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

  AXButtonState AXObject::CheckboxOrRadioValue() const noexcept
  {
    // Native checkboxes and radio buttons are AXNodeObject's. ARIA checkboxes, radio buttons and
    // switches use aria-checked; toggle buttons use aria-pressed.
    if (IsToggleButton())
    {
      auto pressed = GetAttribute(u8"aria-pressed");
      if (IsAttributeTrue(pressed))
      {
        return AXButtonState::On;
      }
      if (StringAlgorithms::ASCIICaseInsensitiveMatch(pressed, u8"mixed"))
      {
        return AXButtonState::Mixed;
      }
      return AXButtonState::Off;
    }

    auto checked = GetAttribute(u8"aria-checked");
    if (IsAttributeTrue(checked))
    {
      return AXButtonState::On;
    }
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(checked, u8"mixed"))
    {
      // ARIA says that radio, menuitemradio, and switch elements must not expose button state mixed.
      auto ariaRole = AriaRoleAttribute();
      if (ariaRole == AXRole::RadioButton || ariaRole == AXRole::MenuItemRadio || ariaRole == AXRole::Switch)
      {
        return AXButtonState::Off;
      }
      return AXButtonState::Mixed;
    }
    return AXButtonState::Off;
  }

  bool AXObject::HasPopup() const noexcept
  {
    auto value = GetAttributeTrimmed(u8"aria-haspopup");
    if (value.empty())
    {
      // A combobox has a listbox pop-up by default.
      return _role == AXRole::ComboBox;
    }
    return !StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false");
  }

  AXRole AXObject::ButtonRoleType() const noexcept
  {
    // If aria-pressed is present, then it should be exposed as a toggle button.
    if (!GetAttribute(u8"aria-pressed").empty())
    {
      return AXRole::ToggleButton;
    }
    if (HasPopup())
    {
      return AXRole::PopUpButton;
    }
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (ancestor->IsLink() && ancestor->HasPopup())
      {
        return AXRole::PopUpButton;
      }
    }
    return AXRole::Button;
  }
}
