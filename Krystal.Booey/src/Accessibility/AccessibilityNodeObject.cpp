/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityNodeObject.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityTableColumn.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityTableHeaderContainer.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Accessibility/AXObjectRareData.hpp"
#include "Krystal.Booey/Accessibility/AXTableHelpers.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDetailsElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadingElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Platform/Locale/PlatformLocale.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderTextControl.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTable.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCell.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <algorithm>
#include <charconv>
#include <limits>
#include <numeric>

namespace krys::boo::ax
{
  namespace
  {
    using html::TagName;
    using infra::StringAlgorithms;

    /// @brief @p node's HTML tag name, `Unknown` for anything that isn't an HTML element.
    KRYS_NODISCARD TagName HTMLTagNameOf(const dom::Node *node) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      return element ? element->TagQualifiedName().TagName() : TagName::Unknown;
    }

    KRYS_NODISCARD bool EqualsIgnoringCase(dom::DOMStringView a, dom::DOMStringView b) noexcept
    {
      return StringAlgorithms::ASCIICaseInsensitiveMatch(a, b);
    }

    /// @brief @p value as a number, if it's one (ARIA's numeric attributes).
    KRYS_NODISCARD Maybe<float> ParseFloatAttribute(dom::DOMStringView value) noexcept
    {
      if (value.empty())
      {
        return null;
      }
      float result = 0;
      const auto *begin = reinterpret_cast<const char *>(value.data());
      auto [end, error] = std::from_chars(begin, begin + value.size(), result);
      if (error != std::errc {} || end != begin + value.size())
      {
        return null;
      }
      return result;
    }

    /// @brief Whether @p node's ::before draws list markers that are in the tree (a list styled with
    /// `list-style: none` that draws its own).
    KRYS_NODISCARD bool ChildHasPseudoVisibleListItemMarkers(const dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      auto *before = element ? element->BeforePseudoElement() : nullptr;
      auto *cache = before ? before->NodeDocument().ExistingAXObjectCache() : nullptr;
      auto *object = cache ? cache->GetOrCreate(*before) : nullptr;
      if (!object)
      {
        return false;
      }
      if (!object->IsIgnored())
      {
        return true;
      }
      for (const auto &child : object->Children())
      {
        if (!child->IsIgnored())
        {
          return true;
        }
      }
      return false;
    }

    /// @brief Whether @p element has `aria-label`, `aria-labelledby` or `title`, which name it.
    KRYS_NODISCARD bool HasAccNameAttribute(const AccessibilityObject &object) noexcept
    {
      return !object.GetAttributeTrimmed(u8"aria-label").empty()
             || !object.GetAttributeTrimmed(u8"aria-labelledby").empty()
             || !object.GetAttributeTrimmed(u8"title").empty();
    }

    KRYS_NODISCARD bool ContentEditableAttributeIsEnabled(const dom::Element &element) noexcept
    {
      auto value = element.GetAttribute(u8"contenteditable");
      if (!value)
      {
        return false;
      }
      // Both "true" (case-insensitive) and the empty string count as true.
      return value->empty() || EqualsIgnoringCase(*value, u8"true")
             || EqualsIgnoringCase(*value, u8"plaintext-only");
    }

    /// @brief Whether a role's accessible name comes from its content (ARIA's "Name From: contents").
    KRYS_NODISCARD bool RoleAllowsNameFromContent(AccessibilityRole role) noexcept
    {
      switch (role)
      {
        case AccessibilityRole::Button:
        case AccessibilityRole::Cell:
        case AccessibilityRole::Checkbox:
        case AccessibilityRole::ColumnHeader:
        case AccessibilityRole::GridCell:
        case AccessibilityRole::Heading:
        case AccessibilityRole::Link:
        case AccessibilityRole::ListBoxOption:
        case AccessibilityRole::MenuItem:
        case AccessibilityRole::MenuItemCheckbox:
        case AccessibilityRole::MenuItemRadio:
        case AccessibilityRole::MenuListOption:
        case AccessibilityRole::PopUpButton:
        case AccessibilityRole::RadioButton:
        case AccessibilityRole::Row:
        case AccessibilityRole::RowHeader:
        case AccessibilityRole::SectionFooter:
        case AccessibilityRole::SectionHeader:
        case AccessibilityRole::Summary:
        case AccessibilityRole::Switch:
        case AccessibilityRole::Tab:
        case AccessibilityRole::ToggleButton:
        case AccessibilityRole::TreeItem:
        case AccessibilityRole::UserInterfaceTooltip: return true;
        default:                                      return false;
      }
    }

    /// @brief Computes a text alternative (an accessible name or description), as accname's "Computing
    /// the Text Alternative" does.
    /// @see https://w3c.github.io/accname/#computation-steps
    class TextAlternativeComputation
    {
    public:
      enum class Kind : uint8
      {
        Name,
        Description,
      };

    private:
      AXObjectCache &_cache;
      Kind _kind;
      const dom::Node &_root;
      HashSet<const dom::Node *> _visited;

      struct Context
      {
        /// @brief In an aria-labelledby or aria-describedby traversal, and whether the node it referred
        /// to was hidden.
        bool InLabelledByTraversal {false};
        bool ReferencedNodeWasHidden {false};
        /// @brief In a native text alternative element's traversal (a label), and whether its root was
        /// hidden.
        bool InNativeTextAlternativeTraversal {false};
        bool NativeRootWasHidden {false};
        /// @brief Recursing into a node's content or referenced nodes, rather than at the root.
        bool IsRecursion {false};
        /// @brief Referenced by aria-labelledby or aria-describedby itself.
        bool IsReferenced {false};
        /// @brief Inside a label (a native text alternative element).
        bool InNativeTextAlternative {false};
        /// @brief In the content of an element whose name is being computed from its content.
        bool InNameFromContent {false};
      };

    public:
      TextAlternativeComputation(AXObjectCache &cache, Kind kind, const dom::Node &root) noexcept
          : _cache(cache), _kind(kind), _root(root)
      {
      }

      KRYS_NODISCARD dom::DOMString Compute() noexcept
      {
        // 1. Set the root node to the given element, the current node to the root node, and the total
        //    accumulated text to the empty string. ...
        // 2. Compute the text alternative for the current node: ...
        // 3. Return the accumulated text.
        // (The result is a flat string: whitespace collapsed and trimmed.)
        if (_kind == Kind::Description)
        {
          // A description is the text of the elements aria-describedby refers to (step 2B for descriptions);
          // AccessibleDescription falls back to aria-description and the host language's.
          auto *element = DynamicDowncast<dom::Element>(_root);
          _visited.insert(&_root);
          if (!element)
          {
            return {};
          }
          auto description = StringAlgorithms::StripAndCollapseASCIIWhitespace(
            ComputeFromReferences(*element, u8"aria-describedby"));
          if (description.empty() && Is<svg::SVGElement>(*element))
          {
            description = StringAlgorithms::StripAndCollapseASCIIWhitespace(SVGDescription(*element));
          }
          return description;
        }
        return StringAlgorithms::StripAndCollapseASCIIWhitespace(ComputeNode(_root, {}));
      }

    private:
      KRYS_NODISCARD AccessibilityObject *ObjectFor(const dom::Node &node) const noexcept
      {
        return _cache.GetOrCreate(const_cast<dom::Node &>(node), AXObjectCache::IsPartOfRelation::Yes);
      }

      /// @brief Whether @p node is hidden, as accname defines it: not rendered, or `aria-hidden="true"` on it
      /// or an ancestor.
      /// @see https://w3c.github.io/accname/#dfn-hidden
      KRYS_NODISCARD static bool IsHidden(const dom::Node &node) noexcept
      {
        const dom::Element *element = DynamicDowncast<dom::Element>(node);
        if (!element)
        {
          element = node.ParentElementInComposedTree();
        }
        if (!element)
        {
          return false;
        }
        // An area isn't rendered, but is drawn as part of its image.
        if (auto *area = DynamicDowncast<html::HTMLAreaElement>(*element))
        {
          auto image = area->ImageElement();
          return !image || IsHidden(*image);
        }
        if (!node.Renderer() && !element->HasDisplayContents())
        {
          return !DynamicDowncast<dom::Text>(node) || !element->Renderer();
        }
        if (const auto *style =
              element->Renderer() ? &element->Renderer()->Style() : element->ExistingComputedStyle();
            style && style->UsedVisibility() != Visibility::Visible)
        {
          return true;
        }
        for (const dom::Element *ancestor = element; ancestor;
             ancestor = ancestor->ParentElementInComposedTree())
        {
          if (EqualsIgnoringCase(ancestor->GetAttribute(u8"aria-hidden").value_or(dom::DOMString {}),
                                 u8"true"))
          {
            return true;
          }
        }
        return false;
      }

      KRYS_NODISCARD static bool IsInline(const dom::Node &node) noexcept
      {
        auto *renderer = node.Renderer();
        return !renderer || renderer->IsInline();
      }

      KRYS_NODISCARD dom::DOMString ComputeNode(const dom::Node &current, Context context) noexcept
      {
        if (!_visited.insert(&current).second)
        {
          // A node already visited in this computation (a cycle of references) adds nothing more.
          return {};
        }
        auto *element = DynamicDowncast<dom::Element>(current);
        auto *object = element ? ObjectFor(current) : nullptr;
        bool hidden = IsHidden(current);

        // 2A. Hidden Not Referenced: If the current node is hidden and is: not part of an aria-labelledby or
        //     aria-describedby traversal, where the node directly referenced by that relation was hidden; nor
        //     part of a native host language text alternative element (e.g. label in HTML) or attribute
        //     traversal, where the root of that traversal was hidden. Return the empty string.
        if (hidden && !(context.InLabelledByTraversal && context.ReferencedNodeWasHidden)
            && !(context.InNativeTextAlternativeTraversal && context.NativeRootWasHidden))
        {
          return {};
        }

        if (element && object)
        {
          // 2B. LabelledBy: if computing a name, and the current node has an aria-labelledby attribute that
          //     contains at least one valid IDREF, and the current node is not already part of an ongoing
          //     aria-labelledby or aria-describedby traversal, process its IDREFs in order; likewise for a
          //     description and aria-describedby.
          // (Descriptions start at step 2B in Compute; the nodes they refer to are named as for a name.)
          if (!context.InLabelledByTraversal)
          {
            if (auto text = ComputeFromReferences(*element, u8"aria-labelledby"); !text.empty())
            {
              // 2B.iii. Return the accumulated text if it is not the empty string ("").
              return text;
            }
          }

          // 2C. Embedded Control: Otherwise, if the current node is a control embedded within the label (e.g.
          //     any element directly referenced by aria-labelledby) for another widget, where the user can
          //     adjust the embedded control's value, then return the embedded control as part of the text
          //     alternative.
          bool isEmbeddedControl = context.IsRecursion && IsEmbeddedControl(*object);
          if (isEmbeddedControl)
          {
            return EmbeddedControlText(*element, *object);
          }

          // 2D. AriaLabel: Otherwise, if the current node has an aria-label attribute whose value is not
          //     undefined, not the empty string, nor, when trimmed of whitespace, is not the empty string:
          //     ... return the value of aria-label.
          if (auto label = object->GetAttributeTrimmed(u8"aria-label"); !label.empty())
          {
            return label;
          }

          // 2E. Host Language Label: Otherwise, if the current node's native markup provides an attribute
          //     (e.g. alt) or element (e.g. HTML label or SVG title) that defines a text alternative, return
          //     that alternative in the form of a flat string as defined by the host language, unless the
          //     element is marked as presentational (role="presentation" or role="none").
          if (object->AriaRoleAttribute() != AccessibilityRole::Presentational)
          {
            if (auto text = HostLanguageLabel(*element, *object, hidden, context); !text.empty())
            {
              return text;
            }
          }
        }

        // 2F. Name From Content: Otherwise, if the current node's role allows name from content, or if the
        //     current node is referenced by aria-labelledby, aria-describedby, or is a native host language
        //     text alternative element (e.g. label in HTML), or is a descendant of a native host language
        //     text alternative element:
        // 2H. Recursive Name From Content: Otherwise, if the current node is a descendant of an element whose
        //     name is being computed, and contains descendants, proceed to 2F.i.
        if (element && object
            && (RoleAllowsNameFromContent(object->Role()) || context.IsReferenced
                || context.InNativeTextAlternative || context.InNameFromContent))
        {
          if (auto text = NameFromContent(*element, context); !text.empty())
          {
            // 2F.v. Return the accumulated text if it is not the empty string ("").
            return text;
          }
        }

        // 2G. Text Node: Otherwise, if the current node is a Text Node, return its textual contents.
        if (auto *text = DynamicDowncast<dom::Text>(current))
        {
          return dom::DOMString(text->Data());
        }

        // 2I. Tooltip Attribute: Otherwise, if the current node has a Tooltip attribute, return its value.
        if (object)
        {
          if (auto title = object->GetAttribute(u8"title"); !title.empty())
          {
            return title;
          }
        }
        return {};
      }

      KRYS_NODISCARD dom::DOMString ComputeFromReferences(const dom::Element &element,
                                                          dom::DOMStringView attribute) noexcept
      {
        auto references = element.ElementsForAttributeInternal(dom::DOMStringAtom(attribute));
        if (!references || references->empty())
        {
          return {};
        }
        // 2B.i. Set the accumulated text to the empty string.
        dom::DOMString accumulated;
        // 2B.ii. For each IDREF:
        for (const auto &reference : *references)
        {
          // 2B.ii.a. Set the current node to the node referenced by the IDREF.
          // 2B.ii.b. Compute the text alternative of the current node beginning with step 2. Set the result
          //          to that text alternative.
          Context referenced;
          referenced.InLabelledByTraversal = true;
          referenced.ReferencedNodeWasHidden = IsHidden(*reference);
          referenced.IsRecursion = true;
          referenced.IsReferenced = true;
          _visited.erase(reference.get());
          auto result = ComputeNode(*reference, referenced);
          // 2B.ii.c. Append the result, with a space, to the accumulated text.
          if (!result.empty())
          {
            if (!accumulated.empty())
            {
              accumulated += u8' ';
            }
            accumulated += result;
          }
        }
        return accumulated;
      }

      KRYS_NODISCARD static bool IsEmbeddedControl(const AccessibilityObject &object) noexcept
      {
        return object.IsTextControl() || object.IsListBox() || object.IsComboBox()
               || object.Role() == AccessibilityRole::PopUpButton || object.IsRangeControl();
      }

      KRYS_NODISCARD dom::DOMString EmbeddedControlText(const dom::Element &element,
                                                        const AccessibilityObject &object) noexcept
      {
        // 2C. ... textbox: return the value of the textbox.
        if (object.IsTextControl() && !object.IsComboBox())
        {
          return object.StringValue();
        }
        // 2C. ... combobox or listbox: return the text alternative of the chosen option.
        if (auto *select = DynamicDowncast<html::HTMLSelectElement>(element))
        {
          auto index = select->SelectedIndex();
          if (index >= 0 && static_cast<size_t>(index) < select->ListItems().size())
          {
            if (auto *option = DynamicDowncast<html::HTMLOptionElement>(select->ListItems()[index].get()))
            {
              return option->Label();
            }
          }
          return {};
        }
        if (object.IsComboBox())
        {
          return object.StringValue();
        }
        // (Like Chrome, a list box that has several chosen options gives all their names.)
        if (object.IsListBox())
        {
          dom::DOMString chosen;
          for (const auto &option : const_cast<AccessibilityObject &>(object).SelectedChildren())
          {
            auto name = option->AccessibleName();
            if (!name.empty())
            {
              if (!chosen.empty())
              {
                chosen += u8' ';
              }
              chosen += name;
            }
          }
          return chosen;
        }
        // 2C. ... range: if the embedded control has aria-valuetext, return its value; otherwise, if it has
        //     aria-valuenow, return its value; otherwise, use the value as specified by a host language
        //     attribute.
        if (auto valueText = object.GetAttributeTrimmed(u8"aria-valuetext"); !valueText.empty())
        {
          return valueText;
        }
        if (auto valueNow = object.GetAttributeTrimmed(u8"aria-valuenow"); !valueNow.empty())
        {
          return valueNow;
        }
        return object.StringValue();
      }

      /// @brief @p element's `title` or `desc` child (@p localName) whose language best matches the
      /// element's; if none matches, the first with no language.
      /// @see https://w3c.github.io/svg-aam/#mapping_additional_nd
      KRYS_NODISCARD static const dom::Element *
        SVGChildWithMatchingLanguage(const dom::Element &element, dom::DOMStringView localName) noexcept
      {
        auto language = css::EffectiveLang(element);
        if (language.empty())
        {
          language = dom::DOMString(DefaultLanguage());
        }
        auto primary = [](dom::DOMStringView tag)
        {
          return tag.substr(0, tag.find(u8'-'));
        };
        const dom::Element *fallback = nullptr;
        const dom::Element *primaryMatch = nullptr;
        for (auto *child = element.FirstElementChild().get(); child;
             child = child->NextElementSibling().get())
        {
          if (!Is<svg::SVGElement>(*child) || child->LocalName() != localName)
          {
            continue;
          }
          auto lang = child->GetAttribute(u8"lang").value_or(dom::DOMString {});
          if (lang.empty())
          {
            // The first equally good match is used.
            fallback = fallback ? fallback : child;
          }
          else if (EqualsIgnoringCase(lang, language))
          {
            return child;
          }
          else if (!primaryMatch && EqualsIgnoringCase(primary(lang), primary(language)))
          {
            primaryMatch = child;
          }
        }
        return primaryMatch ? primaryMatch : fallback;
      }

      /// @brief The element a `use` element refers to.
      KRYS_NODISCARD static const dom::Element *SVGUseTarget(const dom::Element &element) noexcept
      {
        auto *use = DynamicDowncast<svg::SVGElement>(element);
        if (!use || use->LocalName() != u8"use")
        {
          return nullptr;
        }
        auto href = element.GetAttributeNS(dom::DOMStringAtom(infra::Namespaces::XLink), u8"href");
        if (!href || href->empty())
        {
          href = element.GetAttribute(u8"href");
        }
        if (!href)
        {
          return nullptr;
        }
        return svg::SVGURIReference::TargetElementFromIRIString(*href, element.TreeScopeForSVGReferences())
          .Element.get();
      }

      /// @brief SVG-AAM's text alternative for @p element, after aria-labelledby and aria-label: its title
      /// child, an a's xlink:title, or the text alternative of what a use element refers to.
      /// @see https://w3c.github.io/svg-aam/#mapping_additional_nd
      KRYS_NODISCARD static dom::DOMString SVGTextAlternative(const dom::Element &element) noexcept
      {
        if (auto *title = SVGChildWithMatchingLanguage(element, u8"title"))
        {
          return title->TextContent().value_or(dom::DOMString {});
        }
        if (element.LocalName() == u8"a")
        {
          if (auto title = element.GetAttributeNS(dom::DOMStringAtom(infra::Namespaces::XLink), u8"title");
              title && !title->empty())
          {
            return *title;
          }
        }
        if (auto *target = SVGUseTarget(element))
        {
          return SVGTextAlternative(*target);
        }
        return {};
      }

      /// @brief SVG-AAM's description for @p element, after aria-describedby: its desc child, the description
      /// of what a use element refers to, or its title child if that isn't its name.
      /// @see https://w3c.github.io/svg-aam/#mapping_additional_nd
      KRYS_NODISCARD static dom::DOMString SVGDescription(const dom::Element &element) noexcept
      {
        if (auto *desc = SVGChildWithMatchingLanguage(element, u8"desc"))
        {
          return desc->TextContent().value_or(dom::DOMString {});
        }
        if (auto *target = SVGUseTarget(element))
        {
          return SVGDescription(*target);
        }
        // The title is the name unless aria-labelledby or aria-label gave one.
        if (auto *title = SVGChildWithMatchingLanguage(element, u8"title");
            title && (element.HasAttribute(u8"aria-labelledby") || element.HasAttribute(u8"aria-label")))
        {
          return title->TextContent().value_or(dom::DOMString {});
        }
        return {};
      }

      /// @brief The text alternative HTML-AAM gives @p element from its markup.
      /// @see https://w3c.github.io/html-aam/#accessible-name-computations-by-html-element
      KRYS_NODISCARD dom::DOMString HostLanguageLabel(const dom::Element &element,
                                                      const AccessibilityObject &object, bool hidden,
                                                      const Context &context) noexcept
      {
        auto tagName = HTMLTagNameOf(&element);
        if (tagName == TagName::Unknown)
        {
          if (Is<svg::SVGElement>(element))
          {
            return SVGTextAlternative(element);
          }
          // A math element's text alternative is its alttext.
          if (Is<mathml::MathMLElement>(element))
          {
            return element.GetAttribute(u8"alttext").value_or(dom::DOMString {});
          }
          return {};
        }

        auto *input = DynamicDowncast<html::HTMLInputElement>(element);
        if (input && (input->IsTextButton() || input->IsImageButton()))
        {
          // input type=button, submit and reset: the value attribute, else submit's and reset's default
          // label. input type=image: alt, else value, else "Submit Query".
          if (input->IsImageButton())
          {
            if (auto alt = element.GetAttribute(u8"alt"); alt && !alt->empty())
            {
              return *alt;
            }
          }
          if (auto value = element.GetAttribute(u8"value"); value && !value->empty())
          {
            return *value;
          }
          auto type = element.GetAttribute(u8"type").value_or(dom::DOMString {});
          if (input->IsImageButton())
          {
            return element.HasAttribute(u8"title") ? dom::DOMString {} : dom::DOMString(u8"Submit Query");
          }
          if (EqualsIgnoringCase(type, u8"submit"))
          {
            return dom::DOMString(u8"Submit");
          }
          if (EqualsIgnoringCase(type, u8"reset"))
          {
            return dom::DOMString(u8"Reset");
          }
          return {};
        }

        // Labelable elements: their label elements' text alternatives, in tree order.
        if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
            htmlElement && htmlElement->IsLabelable() && !context.InNativeTextAlternative)
        {
          if (auto labels = const_cast<html::HTMLElement *>(htmlElement)->Labels())
          {
            dom::DOMString accumulated;
            for (size_t index = 0; index < labels->Length(); ++index)
            {
              auto label = labels->Item(index);
              if (!label)
              {
                continue;
              }
              Context labelContext;
              labelContext.InNativeTextAlternativeTraversal = true;
              labelContext.NativeRootWasHidden = hidden || IsHidden(*label);
              labelContext.IsRecursion = true;
              labelContext.InNativeTextAlternative = true;
              _visited.erase(label.get());
              auto result = ComputeNode(*label, labelContext);
              if (!result.empty())
              {
                if (!accumulated.empty())
                {
                  accumulated += u8' ';
                }
                accumulated += result;
              }
            }
            if (!accumulated.empty())
            {
              return accumulated;
            }
          }
        }

        switch (tagName)
        {
          case TagName::img:
          case TagName::area:
            if (auto alt = element.GetAttribute(u8"alt"))
            {
              return *alt;
            }
            return {};
          case TagName::input:
          case TagName::textarea:
            // The title, then the placeholder (title is step 2I's, but comes first here).
            if (auto title = element.GetAttribute(u8"title"); title && !title->empty())
            {
              return *title;
            }
            if (auto placeholder = element.GetAttribute(u8"placeholder"))
            {
              return *placeholder;
            }
            return object.GetAttribute(u8"aria-placeholder");
          case TagName::fieldset: return ChildElementText(element, TagName::legend, context);
          case TagName::figure:   return ChildElementText(element, TagName::figcaption, context);
          case TagName::table:    return ChildElementText(element, TagName::caption, context);
          default:                return {};
        }
      }

      /// @brief The text alternative of @p element's first child with @p childTagName.
      KRYS_NODISCARD dom::DOMString ChildElementText(const dom::Element &element, TagName childTagName,
                                                     const Context &context) noexcept
      {
        for (auto *child = element.FirstChild(); child; child = child->NextSibling())
        {
          if (HTMLTagNameOf(child) == childTagName)
          {
            Context childContext = context;
            childContext.IsRecursion = true;
            childContext.InNameFromContent = true;
            return NameFromContent(*DynamicDowncast<dom::Element>(child), childContext);
          }
        }
        return {};
      }

      KRYS_NODISCARD dom::DOMString NameFromContent(const dom::Element &element,
                                                    const Context &context) noexcept
      {
        // 2F.i. Set the accumulated text to the empty string.
        dom::DOMString accumulated;
        // 2F.ii. Name From Generated Content: Set the result to the empty string. For each of the current
        //        node's ::before and ::after pseudo elements: ... If the pseudo element has an alternative
        //        text, use it; otherwise, its textual content. Prepend ::before's result and append
        //        ::after's to the accumulated text (block-level ones separated by a space).
        auto [before, after] = GeneratedContentText(element);

        // 2F.iii. Determine Child Nodes: Set the set of child nodes to contain the children of the current
        //         node and append the nodes specified by aria-owns on the current node.
        List<Ref<dom::Node>> childNodes;
        for (auto &child : dom::ComposedTreeChildren(const_cast<dom::Element &>(element)))
        {
          childNodes.push_back(ShareRef(child));
        }
        if (auto owned = element.ElementsForAttributeInternal(dom::DOMStringAtom(u8"aria-owns")))
        {
          for (auto &ownedElement : *owned)
          {
            // (A child it also owns isn't counted twice.)
            if (std::ranges::none_of(childNodes,
                                     [&](const auto &node) { return node.get() == ownedElement.get(); }))
            {
              childNodes.push_back(ownedElement);
            }
          }
        }
        // 2F.iv. Name From Each Child: For each child node of the current node:
        for (auto &childNode : childNodes)
        {
          auto &child = *childNode;
          // 2F.iv.a. Set the current node to the child node.
          // 2F.iv.b. Compute the text alternative of the current node beginning with step 2. Set the result
          //          to that text alternative.
          Context childContext = context;
          childContext.IsRecursion = true;
          childContext.IsReferenced = false;
          childContext.InNameFromContent = true;
          auto result = ComputeNode(child, childContext);
          // 2F.iv.c. Append the result to the accumulated text.
          // (Block-level children are separated by a space, as their text would be if laid out.)
          if (!IsInline(child))
          {
            accumulated += u8' ';
          }
          accumulated += result;
          if (!IsInline(child))
          {
            accumulated += u8' ';
          }
        }
        return before + accumulated + after;
      }

      /// @brief The text of @p element's `::before` and `::after` content (their CSS alt text where given),
      /// with a space around a block-level one.
      KRYS_NODISCARD static Pair<dom::DOMString, dom::DOMString>
        GeneratedContentText(const dom::Element &element) noexcept
      {
        auto textOf = [](const dom::PseudoElement *pseudo) -> dom::DOMString
        {
          auto *renderer = pseudo ? pseudo->Renderer() : nullptr;
          if (!renderer)
          {
            return {};
          }
          dom::DOMString text;
          for (auto *descendant = renderer->FirstChildSlow(); descendant;
               descendant = descendant->NextInPreOrder(renderer))
          {
            if (auto *fragment = DynamicDowncast<RenderTextFragment>(*descendant);
                fragment && fragment->AltText())
            {
              text += *fragment->AltText();
            }
            else if (auto *renderText = DynamicDowncast<RenderText>(*descendant))
            {
              text += renderText->Text();
            }
            else if (auto *image = DynamicDowncast<RenderImage>(*descendant))
            {
              text += image->AltText();
            }
          }
          if (!text.empty() && !renderer->IsInline())
          {
            text = u8' ' + text + u8' ';
          }
          return text;
        };
        return {textOf(element.BeforePseudoElement()), textOf(element.AfterPseudoElement())};
      }
    };
  }

  Ref<AccessibilityNodeObject> AccessibilityNodeObject::Create(AXID id, dom::Node &node,
                                                               AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityNodeObject(id, &node, cache));
  }

  AccessibilityNodeObject::AccessibilityNodeObject(AXID id, dom::Node *node, AXObjectCache &cache) noexcept
      : AccessibilityObject(id, cache), _node(CreateWeakPtr(node))
  {
  }

  AccessibilityNodeObject::~AccessibilityNodeObject() noexcept
  {
    krys_debug_assert(IsDetached());
  }

  void AccessibilityNodeObject::Init() noexcept
  {
    _ariaRole = DetermineAriaRoleAttribute();
    // The ARIA role must be known for IsTable().
    if (IsTable())
    {
      EnsureRareData().SetIsExposableTable(ComputeIsTableExposableThroughAccessibility());
    }
    AccessibilityObject::Init();
  }

  void AccessibilityNodeObject::DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept
  {
    // AccessibilityObject clears the children.
    AccessibilityObject::DetachRemoteParts(detachmentType);
    _node = nullptr;
  }

  AccessibilityObject *AccessibilityNodeObject::ParentObject() const noexcept
  {
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!node || !cache)
    {
      return nullptr;
    }

    // The composed tree's parent, skipping document fragments (shadow roots).
    dom::Node *parent = node->ParentInComposedTree();
    if (parent && !Is<dom::Element>(*parent) && !Is<dom::Document>(*parent))
    {
      parent = parent->ParentInComposedTree();
    }
    if (!parent)
    {
      return nullptr;
    }

    // An area's parent is its image map's image, not the map.
    if (auto *area = DynamicDowncast<html::HTMLAreaElement>(*node))
    {
      auto image = area->ImageElement();
      return image ? cache->GetOrCreate(*image) : nullptr;
    }

    if (auto *owner = OwnerParentObject()) KRYS_UNLIKELY
    {
      return owner;
    }
    return cache->GetOrCreate(*parent);
  }

  AccessibilityObject *AccessibilityNodeObject::OwnerParentObject() const noexcept
  {
    auto owners = Owners();
    krys_debug_assert(owners.size() <= 1);
    return owners.empty() ? nullptr : owners[0].get();
  }

  void AccessibilityNodeObject::UpdateOwnedChildrenIfNecessary() noexcept
  {
    auto ownedObjects = OwnedObjects();
    if (ownedObjects.empty())
    {
      return;
    }
    bool didRemoveChild = false;
    for (const auto &child : ownedObjects)
    {
      // A DOM child it also owns moves to its place in aria-owns order.
      if (auto found = std::ranges::find(_children, child); found != _children.end())
      {
        _children.erase(found);
        didRemoveChild = true;
      }
      AddChild(child.get());
    }
    if (didRemoveChild)
    {
      ResetChildrenIndexInParent();
    }
  }

  AccessibilityRole AccessibilityNodeObject::DetermineAccessibilityRole() noexcept
  {
    if (_ariaRole != AccessibilityRole::Unknown)
    {
      return _ariaRole;
    }
    if (IsExposableTable())
    {
      return AccessibilityRole::Table;
    }
    if (IsExposedTableRow())
    {
      return AccessibilityRole::Row;
    }
    if (auto *area = DynamicDowncast<html::HTMLAreaElement>(GetNode()))
    {
      // (The cache works the role out again when the href changes.)
      return !area->Href().empty() ? AccessibilityRole::Link : AccessibilityRole::Generic;
    }

    auto roleFromNode = DetermineAccessibilityRoleFromNode();
    if (IsTableCell() && roleFromNode != AccessibilityRole::ColumnHeader
        && roleFromNode != AccessibilityRole::RowHeader && roleFromNode != AccessibilityRole::Cell
        && roleFromNode != AccessibilityRole::GridCell)
    {
      auto *parentTable = ParentTable();
      if (parentTable && parentTable->IsExposableTable())
      {
        return parentTable->HasGridRole() ? AccessibilityRole::GridCell : AccessibilityRole::Cell;
      }
    }
    return roleFromNode;
  }

  AccessibilityRole AccessibilityNodeObject::DetermineAccessibilityRoleFromNode(
    TreatStyleFormatGroupAsInline treatStyleFormatGroupAsInline) const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return AccessibilityRole::Unknown;
    }
    if (Is<dom::Text>(*node))
    {
      return AccessibilityRole::StaticText;
    }

    auto *element = DynamicDowncast<html::HTMLElement>(*node);
    if (!element)
    {
      return AccessibilityRole::Unknown;
    }

    if (css::IsLink(*element))
    {
      return AccessibilityRole::Link;
    }
    if (auto *select = DynamicDowncast<html::HTMLSelectElement>(*element))
    {
      return select->Multiple() ? AccessibilityRole::ListBox : AccessibilityRole::PopUpButton;
    }

    auto tagName = element->TagQualifiedName().TagName();
    if (tagName == TagName::img && element->HasAttribute(u8"usemap"))
    {
      return AccessibilityRole::ImageMap;
    }
    switch (tagName)
    {
      case TagName::li:     return AccessibilityRole::ListItem;
      case TagName::button: return ButtonRoleType();
      case TagName::legend: return AccessibilityRole::Legend;
      case TagName::canvas: return AccessibilityRole::Canvas;
      case TagName::td:
      case TagName::th:
        // A layout table's cells; an exposed table's take their role from it.
        return table_helpers::LayoutTableCellRole;
      default: break;
    }

    if (auto *input = DynamicDowncast<html::HTMLInputElement>(*element))
    {
      return RoleFromInputElement(*input);
    }
    if (tagName == TagName::textarea || HasContentEditableAttributeSet())
    {
      return AccessibilityRole::TextArea;
    }
    if (HeadingLevel())
    {
      return AccessibilityRole::Heading;
    }

    switch (tagName)
    {
      case TagName::code:   return AccessibilityRole::Code;
      case TagName::del:
      case TagName::s:      return AccessibilityRole::Deletion;
      case TagName::ins:    return AccessibilityRole::Insertion;
      case TagName::sub:    return AccessibilityRole::Subscript;
      case TagName::sup:    return AccessibilityRole::Superscript;
      case TagName::strong: return AccessibilityRole::Strong;
      case TagName::kbd:
      case TagName::pre:
      case TagName::samp:
      case TagName::var:
      case TagName::cite:
        return treatStyleFormatGroupAsInline == TreatStyleFormatGroupAsInline::Yes
                 ? AccessibilityRole::Inline
                 : AccessibilityRole::TextGroup;
      case TagName::dd:       return AccessibilityRole::DescriptionListDetail;
      case TagName::dt:       return AccessibilityRole::DescriptionListTerm;
      case TagName::dl:       return AccessibilityRole::DescriptionList;
      case TagName::menu:
      case TagName::ol:
      case TagName::ul:       return AccessibilityRole::List;
      case TagName::fieldset: return AccessibilityRole::Group;
      case TagName::figure:   return AccessibilityRole::Figure;
      case TagName::p:        return AccessibilityRole::Paragraph;
      case TagName::label:    return AccessibilityRole::Label;
      case TagName::dfn:
        // The dfn element represents a term being defined, the "term" role rather than "definition".
        // https://html.spec.whatwg.org/#the-dfn-element
        return AccessibilityRole::Term;
      case TagName::div:     return AccessibilityRole::Generic;
      case TagName::form:    return AccessibilityRole::Form;
      case TagName::article: return AccessibilityRole::DocumentArticle;
      case TagName::main:    return AccessibilityRole::LandmarkMain;
      case TagName::nav:     return AccessibilityRole::LandmarkNavigation;
      case TagName::aside:
      {
        static constexpr TagName sectioningElements[] = {TagName::aside, TagName::article, TagName::section,
                                                         TagName::nav};
        if (_ariaRole == AccessibilityRole::LandmarkComplementary
            || !IsDescendantOfElementType(sectioningElements))
        {
          return AccessibilityRole::LandmarkComplementary;
        }
        // Complementary landmarks within sectioning content need names to be landmarks.
        // https://w3c.github.io/html-aam/#el-aside
        return HasAccNameAttribute() ? AccessibilityRole::LandmarkComplementary : AccessibilityRole::Generic;
      }
      case TagName::search: return AccessibilityRole::LandmarkSearch;
      case TagName::section:
        // A section is a region landmark only when it has a name. https://w3c.github.io/html-aam/#el-section
        return HasAccNameAttribute() ? AccessibilityRole::LandmarkRegion : AccessibilityRole::TextGroup;
      case TagName::address:    return AccessibilityRole::Group;
      case TagName::blockquote: return AccessibilityRole::Blockquote;
      case TagName::caption:
      case TagName::figcaption: return AccessibilityRole::Caption;
      case TagName::dialog:     return AccessibilityRole::ApplicationDialog;
      case TagName::mark:       return AccessibilityRole::Mark;
      case TagName::details:    return AccessibilityRole::Details;
      case TagName::summary:
        if (DynamicDowncast<html::HTMLSummaryElement>(*element)->IsActiveSummary())
        {
          return AccessibilityRole::Summary;
        }
        break;
      case TagName::output:
        // https://w3c.github.io/html-aam/#el-output
        return AccessibilityRole::ApplicationStatus;
      case TagName::video: return AccessibilityRole::Video;
      case TagName::audio: return AccessibilityRole::Audio;
      case TagName::html:
        // The document's web area stands for the html element.
        return AccessibilityRole::Ignored;
      case TagName::header:
      case TagName::footer:
      {
        // A header or footer is the page's banner or content info landmark when scoped to the body, and a
        // section header or footer within sectioning content.
        // https://w3c.github.io/html-aam/#el-header-ancestorbody
        static constexpr TagName sectioningElements[] = {TagName::article, TagName::aside, TagName::main,
                                                         TagName::nav, TagName::section};
        bool scopedToBody = !IsDescendantOfElementType(sectioningElements);
        if (tagName == TagName::header)
        {
          return scopedToBody ? AccessibilityRole::LandmarkBanner : AccessibilityRole::SectionHeader;
        }
        return scopedToBody ? AccessibilityRole::LandmarkContentInfo : AccessibilityRole::SectionFooter;
      }
      case TagName::time:   return AccessibilityRole::Time;
      case TagName::hr:     return AccessibilityRole::HorizontalRule;
      case TagName::em:     return AccessibilityRole::Emphasis;
      case TagName::hgroup: return AccessibilityRole::Group;
      default:              break;
    }

    // An element with no role but with ARIA attributes, or that can be focused, is exposed as a group.
    if (SupportsARIAAttributes() || CanSetFocusAttribute())
    {
      return AccessibilityRole::Group;
    }
    return AccessibilityRole::Unknown;
  }

  AccessibilityRole
    AccessibilityNodeObject::RoleFromInputElement(const html::HTMLInputElement &input) const noexcept
  {
    if (input.IsTextButton())
    {
      return ButtonRoleType();
    }
    if (input.HasAttribute(u8"switch") && input.IsCheckbox())
    {
      return AccessibilityRole::Switch;
    }
    if (input.IsCheckbox())
    {
      return AccessibilityRole::Checkbox;
    }
    if (input.IsRadioButton())
    {
      return AccessibilityRole::RadioButton;
    }
    if (input.IsTextField())
    {
      // A text field with a combobox ancestor (ARIA 1.1's pattern, since recommended against) is the
      // combobox.
      for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
      {
        if (ancestor->IsComboBox())
        {
          return AccessibilityRole::ComboBox;
        }
        if (!ancestor->IsGroup() && ancestor->Role() != AccessibilityRole::Generic)
        {
          break;
        }
      }
      return input.IsSearchField() ? AccessibilityRole::SearchField : AccessibilityRole::TextField;
    }
    if (input.IsDateField() || input.IsDateTimeLocalField() || input.IsMonthField() || input.IsTimeField()
        || input.IsWeekField())
    {
      return AccessibilityRole::DateTime;
    }
    if (input.IsFileUpload())
    {
      return AccessibilityRole::Button;
    }
    if (input.IsColorControl())
    {
      return AccessibilityRole::ColorWell;
    }
    if (input.IsInputTypeHidden())
    {
      return AccessibilityRole::Ignored;
    }
    if (input.IsRangeControl())
    {
      return AccessibilityRole::Slider;
    }
    if (input.IsImageButton())
    {
      return ButtonRoleType();
    }
    // Any other input type is a text field.
    return AccessibilityRole::TextField;
  }

  AccessibilityRole AccessibilityNodeObject::DetermineAriaRoleAttribute() const noexcept
  {
    auto ariaRole = GetAttribute(u8"role");
    if (ariaRole.empty())
    {
      return AccessibilityRole::Unknown;
    }

    auto role = AriaRoleToRole(ariaRole);

    // ARIA states if an item can get focus, it should not be presentational.
    if (role == AccessibilityRole::Presentational && CanSetFocusAttribute())
    {
      return AccessibilityRole::Unknown;
    }
    if (role == AccessibilityRole::Button)
    {
      role = ButtonRoleType();
    }

    // The textbox role is single- or multi-line, by aria-multiline or the element.
    if (role == AccessibilityRole::TextField)
    {
      auto multiline = GetAttribute(u8"aria-multiline");
      if (EqualsIgnoringCase(multiline, u8"true")
          || (!EqualsIgnoringCase(multiline, u8"false")
              && (HTMLTagNameOf(GetNode()) == TagName::textarea || HasContentEditableAttributeSet())))
      {
        role = AccessibilityRole::TextArea;
      }
    }

    role = RemapAriaRoleDueToParent(role);

    // Presentational roles are invalidated by the presence of ARIA attributes.
    if (role == AccessibilityRole::Presentational && SupportsARIAAttributes())
    {
      role = AccessibilityRole::Unknown;
    }

    // A form or region landmark with no name is an authoring error, treated as if no role had been given,
    // or as the next role listed. https://w3c.github.io/aria/#document-handling_author-errors_roles
    if ((role == AccessibilityRole::LandmarkRegion || role == AccessibilityRole::Form)
        && !HasAccNameAttribute())
    {
      auto skipped = role;
      auto nextRole =
        AriaRoleToRole(ariaRole, [skipped](AccessibilityRole candidate) { return candidate == skipped; });
      role = nextRole != role ? nextRole : AccessibilityRole::Unknown;
    }
    return role;
  }

  AccessibilityRole AccessibilityNodeObject::RemapAriaRoleDueToParent(AccessibilityRole role) const noexcept
  {
    // Some objects change their role based on their parent. Asking for the unignored parent calls
    // IsIgnored(), which can loop while the object is being created.
    if (role != AccessibilityRole::ListBoxOption && role != AccessibilityRole::MenuItem)
    {
      return role;
    }
    for (auto *parent = ParentObject(); parent && !parent->IsIgnored(); parent = parent->ParentObject())
    {
      auto parentAriaRole = parent->AriaRoleAttribute();
      // Selects and listboxes both have options as child roles, but options in a menu are menu items.
      if (role == AccessibilityRole::ListBoxOption && parentAriaRole == AccessibilityRole::Menu)
      {
        return AccessibilityRole::MenuItem;
      }
      // If the parent had a different role, then we don't need to continue searching up the chain.
      if (parentAriaRole != AccessibilityRole::Unknown)
      {
        break;
      }
    }
    return role;
  }

  bool AccessibilityNodeObject::IsDescendantOfElementType(Span<const TagName> tagNames) const noexcept
  {
    auto *node = GetNode();
    for (auto *ancestor = node ? node->ParentElement() : nullptr; ancestor;
         ancestor = ancestor->ParentElement())
    {
      auto tagName = HTMLTagNameOf(ancestor);
      for (auto candidate : tagNames)
      {
        if (tagName == candidate)
        {
          return true;
        }
      }
    }
    return false;
  }

  void AccessibilityNodeObject::UpdateChildrenIfNecessary() noexcept
  {
    if (NeedsToUpdateChildren())
    {
      ClearChildren();
    }
    AccessibilityObject::UpdateChildrenIfNecessary();
  }

  void AccessibilityNodeObject::ClearChildren() noexcept
  {
    // The objects with no node this object made for its children (a slider's thumb, a table's columns and
    // header container) are made again with them.
    List<AXID> mockObjects;
    for (const auto &child : _children)
    {
      if (child->IsMockObject() && child->ParentObject() == this)
      {
        mockObjects.push_back(child->ObjectID());
      }
    }
    if (_rareData)
    {
      for (const auto &column : _rareData->TableColumns())
      {
        mockObjects.push_back(column->ObjectID());
      }
      if (auto *headerContainer = _rareData->TableHeaderContainer())
      {
        mockObjects.push_back(headerContainer->ObjectID());
      }
    }

    AccessibilityObject::ClearChildren();
    _childrenDirty = false;
    if (_rareData)
    {
      _rareData->ResetChildrenDependentTableFields();
    }
    if (auto *cache = AxObjectCache())
    {
      for (auto id : mockObjects)
      {
        cache->Remove(id);
      }
    }
  }

  void AccessibilityNodeObject::AddChildren() noexcept
  {
    // If the need to add more children in addition to existing children arises, childrenChanged should have
    // been called, leaving the object with no children.
    krys_debug_assert(!_childrenInitialized);
    _childrenInitialized = true;

    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (node && cache && IsExposableTable())
    {
      // An exposed table's children are its caption and rows (and its columns and header container).
      AddTableChildrenAndCellSlots();
    }
    else if (node && cache)
    {
      if (auto *container = DynamicDowncast<dom::ContainerNode>(*node))
      {
        for (auto &child : dom::ComposedTreeChildren(*container))
        {
          AddChild(cache->GetOrCreate(child));
        }
      }
      UpdateOwnedChildrenIfNecessary();
    }
    _subtreeDirty = false;
  }

  bool AccessibilityNodeObject::CanHaveChildren() const noexcept
  {
    // When <noscript> is not being used (it has no renderer), ignore its children.
    auto *node = GetNode();
    if (node && !Renderer() && HTMLTagNameOf(node) == TagName::noscript)
    {
      return false;
    }

    // A native text control's text is its value: what's in its user agent shadow tree isn't in the tree,
    // except a spin button (which a mock object stands for).
    if (IsNativeTextControl())
    {
      auto *input = DynamicDowncast<html::HTMLInputElement>(node);
      return input && input->InnerSpinButtonElement();
    }

    // Elements that should not have children.
    switch (Role())
    {
      case AccessibilityRole::Button:
      case AccessibilityRole::PopUpButton:
      case AccessibilityRole::Checkbox:
      case AccessibilityRole::RadioButton:
      case AccessibilityRole::Tab:
      case AccessibilityRole::ToggleButton:
      case AccessibilityRole::StaticText:
      case AccessibilityRole::ListBoxOption:
      case AccessibilityRole::ScrollBar:
      case AccessibilityRole::ProgressIndicator:
      case AccessibilityRole::Switch:
      case AccessibilityRole::MenuItemCheckbox:
      case AccessibilityRole::MenuItemRadio:
      case AccessibilityRole::Splitter:
      case AccessibilityRole::Meter:             return false;
      default:                                   return true;
    }
  }

  bool AccessibilityNodeObject::IsRenderHidden() const noexcept
  {
    const auto *style = Style();
    return style
           && (style->EffectiveDisplay() == DisplayType::None
               || style->UsedVisibility() != Visibility::Visible);
  }

  bool AccessibilityNodeObject::IsValidTree() const noexcept
  {
    // https://w3c.github.io/aria/#tree: a tree's children are tree items, or groups of them.
    auto *node = GetNode();
    if (!node)
    {
      return false;
    }
    List<dom::Node *> queue;
    for (auto *child = node->FirstChild(); child; child = child->NextSibling())
    {
      queue.push_back(child);
    }
    for (size_t index = 0; index < queue.size(); ++index)
    {
      auto *element = DynamicDowncast<dom::Element>(queue[index]);
      if (!element || table_helpers::HasRole(*element, u8"treeitem"))
      {
        continue;
      }
      if (!table_helpers::HasRole(*element, u8"group") && !table_helpers::HasRole(*element, u8"presentation"))
      {
        return false;
      }
      for (auto *child = element->FirstChild(); child; child = child->NextSibling())
      {
        queue.push_back(child);
      }
    }
    return true;
  }

  bool AccessibilityNodeObject::ComputeIsIgnored() const noexcept
  {
    if (IsTree())
    {
      return IsIgnoredByDefault();
    }
    auto *node = GetNode();
    if (!node)
    {
      return true;
    }

    if (auto *text = DynamicDowncast<dom::Text>(*node); text && !Renderer())
    {
      // Whitespace only text elements should be ignored when they have no renderer.
      if (StringAlgorithms::StripLeadingAndTrailingWhitespace(text->Data()).empty())
      {
        return true;
      }
    }

    auto decision = DefaultObjectInclusion();
    if (decision == AccessibilityObjectInclusion::IncludeObject)
    {
      return false;
    }
    if (decision == AccessibilityObjectInclusion::IgnoreObject)
    {
      return true;
    }

    if (auto *area = DynamicDowncast<html::HTMLAreaElement>(*node); area && !area->Href().empty())
    {
      return false;
    }

    auto role = Role();
    if (role == AccessibilityRole::Ignored || role == AccessibilityRole::Unknown)
    {
      return true;
    }

    if (IsRenderHidden())
    {
      // Only canvas fallback content is in the tree while display: none or visibility: hidden.
      for (auto *ancestor = node->ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (HTMLTagNameOf(ancestor) == TagName::canvas)
        {
          return false;
        }
      }
      return true;
    }
    if (IsTableCell())
    {
      return !IsExposedTableCell();
    }
    return false;
  }

  float AccessibilityNodeObject::ValueForRange() const noexcept
  {
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(GetNode()); input && input->IsRangeControl())
    {
      return static_cast<float>(input->ValueAsNumber());
    }
    if (!IsRangeControl())
    {
      return 0;
    }
    // In ARIA 1.1, the implicit value for aria-valuenow on a spin button is 0. For other roles, it is half
    // way between aria-valuemin and aria-valuemax.
    if (auto value = ParseFloatAttribute(GetAttributeTrimmed(u8"aria-valuenow")))
    {
      return *value;
    }
    return IsSpinButton() ? 0 : std::midpoint(MinValueForRange(), MaxValueForRange());
  }

  float AccessibilityNodeObject::MaxValueForRange() const noexcept
  {
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(GetNode()); input && input->IsRangeControl())
    {
      return static_cast<float>(input->Maximum());
    }
    if (!IsRangeControl())
    {
      return 0;
    }
    if (auto value = ParseFloatAttribute(GetAttributeTrimmed(u8"aria-valuemax")))
    {
      return *value;
    }
    // A spin button has no maximum by default; the other roles' is 100.
    return IsSpinButton() ? std::numeric_limits<float>::max() : 100.0f;
  }

  float AccessibilityNodeObject::MinValueForRange() const noexcept
  {
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(GetNode()); input && input->IsRangeControl())
    {
      return static_cast<float>(input->Minimum());
    }
    if (!IsRangeControl())
    {
      return 0;
    }
    if (auto value = ParseFloatAttribute(GetAttributeTrimmed(u8"aria-valuemin")))
    {
      return *value;
    }
    // A spin button has no minimum by default; the other roles' is 0.
    return IsSpinButton() ? -std::numeric_limits<float>::max() : 0.0f;
  }

  dom::DOMString AccessibilityNodeObject::ValueDescription() const noexcept
  {
    return IsRangeControl() ? GetAttribute(u8"aria-valuetext") : dom::DOMString {};
  }

  Maybe<AccessibilityOrientation> AccessibilityNodeObject::OrientationFromARIA() const noexcept
  {
    auto orientation = GetAttribute(u8"aria-orientation");
    if (EqualsIgnoringCase(orientation, u8"horizontal"))
    {
      return AccessibilityOrientation::Horizontal;
    }
    if (EqualsIgnoringCase(orientation, u8"vertical"))
    {
      return AccessibilityOrientation::Vertical;
    }
    if (EqualsIgnoringCase(orientation, u8"undefined"))
    {
      return AccessibilityOrientation::Undefined;
    }
    return null;
  }

  bool AccessibilityNodeObject::IsAccessibilityList() const noexcept
  {
    if (!GetElement())
    {
      return false;
    }
    auto role = GetAttribute(u8"role");
    for (const auto &name : StringAlgorithms::SplitOnWhitespace(role))
    {
      if (EqualsIgnoringCase(name, u8"list") || EqualsIgnoringCase(name, u8"directory"))
      {
        return true;
      }
    }
    if (!role.empty())
    {
      return false;
    }
    auto tagName = HTMLTagNameOf(GetNode());
    return tagName == TagName::ul || tagName == TagName::ol || tagName == TagName::dl
           || tagName == TagName::menu;
  }

  bool AccessibilityNodeObject::IsDescriptionList() const noexcept
  {
    return HTMLTagNameOf(GetNode()) == TagName::dl;
  }

  bool AccessibilityNodeObject::IsUnorderedList() const noexcept
  {
    if (_ariaRole == AccessibilityRole::List)
    {
      return true;
    }
    auto tagName = HTMLTagNameOf(GetNode());
    return tagName == TagName::menu || tagName == TagName::ul;
  }

  bool AccessibilityNodeObject::IsOrderedList() const noexcept
  {
    return _ariaRole == AccessibilityRole::Directory || HTMLTagNameOf(GetNode()) == TagName::ol;
  }

  AccessibilityRole AccessibilityNodeObject::DetermineListRoleWithCleanChildren() noexcept
  {
    if (!IsAccessibilityList())
    {
      return AccessibilityRole::Unknown;
    }
    // Directory is mapped to list for now, but does not adhere to the same heuristics.
    if (_ariaRole == AccessibilityRole::Directory)
    {
      return AccessibilityRole::List;
    }

    // Heuristic to determine if an ambiguous list is relevant to convey to the accessibility tree:
    //   1. If it's an ordered list or has role="list" defined, then it's a list.
    //      1a. Unless the list has no children, then it's not a list.
    //   2. If it is contained in <nav> or <el role="navigation">, it's a list.
    //   3. If it displays visible list markers, it's a list.
    //   4. If it does not display list markers, it's not a list.
    //   5. If it has one or zero listitem children, it's not a list.
    //   6. Otherwise it's a list.
    auto role = AccessibilityRole::List;
    // The role is a list while the children are asked for (CanHaveChildren depends on it).
    SetForScope temporaryRole(_role, role);
    uint32 listItemCount = 0;
    bool hasVisibleMarkers = false;
    const auto &children = Children();

    // Description lists are always semantically description lists.
    if (IsDescriptionList() && !children.empty())
    {
      return AccessibilityRole::DescriptionList;
    }

    for (const auto &child : children)
    {
      if (child->AriaRoleAttribute() == AccessibilityRole::ListItem)
      {
        ++listItemCount;
      }
      else if (child->Role() == AccessibilityRole::ListItem)
      {
        // Rendered list items always count.
        if (auto *listItem = DynamicDowncast<RenderListItem>(child->Renderer()))
        {
          if (!hasVisibleMarkers
              && (listItem->MarkerRenderer()
                  || (child->GetNode() && ChildHasPseudoVisibleListItemMarkers(*child->GetNode()))))
          {
            hasVisibleMarkers = true;
          }
          ++listItemCount;
        }
        else if (HTMLTagNameOf(child->GetNode()) == TagName::li)
        {
          // Inline list items in a list with an explicit role count too.
          if (_ariaRole == AccessibilityRole::List)
          {
            ++listItemCount;
          }
          if (ChildHasPseudoVisibleListItemMarkers(*child->GetNode()))
          {
            hasVisibleMarkers = true;
            ++listItemCount;
          }
        }
      }
    }

    // Lists with an ARIA role only need one item; <ul> and <ol> lists need visible markers.
    if (_ariaRole != AccessibilityRole::Unknown)
    {
      if (!listItemCount)
      {
        role = AccessibilityRole::Group;
      }
    }
    else if (!hasVisibleMarkers)
    {
      // Lists inside navigation are still lists (webkit.org/b/193382).
      role = IsDescendantOfRole(AccessibilityRole::LandmarkNavigation) ? AccessibilityRole::List
                                                                       : AccessibilityRole::Group;
    }
    return role;
  }

  bool AccessibilityNodeObject::IsNativeTextControl() const noexcept
  {
    auto *node = GetNode();
    if (HTMLTagNameOf(node) == TagName::textarea)
    {
      return true;
    }
    auto *input = DynamicDowncast<html::HTMLInputElement>(node);
    return input && (input->IsTextField() || input->IsNumberField());
  }

  bool AccessibilityNodeObject::IsNonNativeTextControl() const noexcept
  {
    auto role = AriaRoleAttribute();
    bool isARIATextControl = role == AccessibilityRole::TextArea || role == AccessibilityRole::TextField
                             || role == AccessibilityRole::SearchField;
    return (isARIATextControl || HasContentEditableAttributeSet()) && !IsNativeTextControl();
  }

  bool AccessibilityNodeObject::HasContentEditableAttributeSet() const noexcept
  {
    auto *element = GetElement();
    return element && ContentEditableAttributeIsEnabled(*element);
  }

  bool AccessibilityNodeObject::HasAccNameAttribute() const noexcept
  {
    return ax::HasAccNameAttribute(*this);
  }

  bool AccessibilityNodeObject::HasAttributesRequiredForInclusion() const noexcept
  {
    auto *element = GetElement();
    if (!element)
    {
      return false;
    }
    if (HasAccNameAttribute())
    {
      return true;
    }
    if (Is<mathml::MathMLElement>(*element) && !GetAttribute(u8"alttext").empty())
    {
      return true;
    }
    return !GetAttribute(u8"alt").empty();
  }

  bool AccessibilityNodeObject::IsGenericFocusableElement() const noexcept
  {
    if (!CanSetFocusAttribute())
    {
      return false;
    }
    // If it's a control, it's not generic.
    if (IsControl())
    {
      return false;
    }
    auto role = Role();
    if (role == AccessibilityRole::Video || role == AccessibilityRole::Audio)
    {
      return false;
    }
    // If it has an aria role, it's not generic.
    if (_ariaRole != AccessibilityRole::Unknown)
    {
      return false;
    }
    // A contenteditable element is focusable for that, which is handled elsewhere.
    if (HasContentEditableAttributeSet())
    {
      return false;
    }
    // The web area and body element are both focusable, but handled elsewhere. An SVG root is focusable by
    // default, but probably not interactive (it can still be given an ARIA role).
    return role != AccessibilityRole::WebArea && HTMLTagNameOf(GetNode()) != TagName::body
           && role != AccessibilityRole::SVGRoot;
  }

  bool AccessibilityNodeObject::IsStyleFormatGroup() const noexcept
  {
    switch (HTMLTagNameOf(GetNode()))
    {
      case TagName::kbd:
      case TagName::code:
      case TagName::pre:
      case TagName::samp:
      case TagName::var:
      case TagName::cite:
      case TagName::ins:
      case TagName::del:
      case TagName::sup:
      case TagName::sub:  return true;
      default:            return false;
    }
  }

  bool AccessibilityNodeObject::IsFocused() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return false;
    }
    auto *focusedElement = node->NodeDocument().FocusedElement();
    if (!focusedElement)
    {
      return false;
    }
    if (focusedElement == node)
    {
      return true;
    }
    // A web area is the document, which isn't focusable: it's focused when its selection is focused and
    // active.
    return Role() == AccessibilityRole::WebArea && node->NodeDocument().Selection().IsFocusedAndActive();
  }

  bool AccessibilityNodeObject::IsEnabled() const noexcept
  {
    if (IsImageMapLink())
    {
      return true;
    }
    // ARIA says that the disabled status applies to the current element and all descendant elements.
    for (const AccessibilityObject *object = this; object; object = object->ParentObject())
    {
      auto disabled = object->GetAttribute(u8"aria-disabled");
      if (EqualsIgnoringCase(disabled, u8"true"))
      {
        return false;
      }
      if (EqualsIgnoringCase(disabled, u8"false"))
      {
        break;
      }
    }
    if (Role() == AccessibilityRole::HorizontalRule)
    {
      return false;
    }
    auto *element = GetElement();
    return !element || !element->IsDisabledFormControl();
  }

  bool AccessibilityNodeObject::IsIndeterminate() const noexcept
  {
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(GetNode()))
    {
      return (input->IsCheckbox() && input->Indeterminate()) || false;
    }
    if (IsCheckboxOrRadio() || IsSwitch() || IsToggleButton() || Role() == AccessibilityRole::MenuItemCheckbox
        || Role() == AccessibilityRole::MenuItemRadio)
    {
      return CheckboxOrRadioValue() == AccessibilityButtonState::Mixed;
    }
    // A progressbar with no aria-valuenow is indeterminate.
    if (_ariaRole == AccessibilityRole::ProgressIndicator)
    {
      return !HasAttribute(u8"aria-valuenow");
    }
    return false;
  }

  bool AccessibilityNodeObject::IsPressed() const noexcept
  {
    if (!IsButton())
    {
      return false;
    }
    // A toggle button's state is aria-pressed, rather than whether it's active.
    if (IsToggleButton())
    {
      return EqualsIgnoringCase(GetAttribute(u8"aria-pressed"), u8"true");
    }
    auto *element = GetElement();
    return element && element->Active();
  }

  bool AccessibilityNodeObject::IsChecked() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return false;
    }
    // Native checkedness first.
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(*node))
    {
      return input->MatchesCheckedPseudoClass();
    }
    // Else, if this is an ARIA checkbox or radio, respect the aria-checked attribute.
    switch (_ariaRole)
    {
      case AccessibilityRole::RadioButton:
      case AccessibilityRole::Checkbox:
      case AccessibilityRole::MenuItem:
      case AccessibilityRole::MenuItemCheckbox:
      case AccessibilityRole::MenuItemRadio:
      case AccessibilityRole::Switch:
      case AccessibilityRole::TreeItem:         return EqualsIgnoringCase(GetAttribute(u8"aria-checked"), u8"true");
      default:                                  return false;
    }
  }

  bool AccessibilityNodeObject::IsSelected() const noexcept
  {
    if (auto *option = DynamicDowncast<html::HTMLOptionElement>(GetNode()))
    {
      return option->Selected();
    }
    if (EqualsIgnoringCase(GetAttribute(u8"aria-selected"), u8"true"))
    {
      return true;
    }
    if (IsTabItem() && IsTabItemSelected())
    {
      return true;
    }
    // Assistive technologies treat the focused or active menu item as the selected one.
    if (IsMenuItem())
    {
      if (IsFocused())
      {
        return true;
      }
      auto *parent = ParentObjectUnignored();
      return parent && parent->ActiveDescendant() == this;
    }
    return false;
  }

  bool AccessibilityNodeObject::IsRequired() const noexcept
  {
    if (auto *element = GetElement(); element && element->IsRequiredFormControl())
    {
      return true;
    }
    return EqualsIgnoringCase(GetAttribute(u8"aria-required"), u8"true");
  }

  bool AccessibilityNodeObject::IsMultiSelectable() const noexcept
  {
    // ARIA: tables don't support selection; grids and tree grids are multi-selectable unless
    // aria-multiselectable says otherwise.
    if (HasGridRole())
    {
      return !EqualsIgnoringCase(GetAttribute(u8"aria-multiselectable"), u8"false");
    }
    if (IsTable())
    {
      return false;
    }
    auto multiSelectable = GetAttribute(u8"aria-multiselectable");
    if (EqualsIgnoringCase(multiSelectable, u8"true"))
    {
      return true;
    }
    if (EqualsIgnoringCase(multiSelectable, u8"false"))
    {
      return false;
    }
    auto *select = DynamicDowncast<html::HTMLSelectElement>(GetNode());
    return select && select->Multiple();
  }

  namespace
  {
    /// @brief The popover @p node (a button) shows and hides, if it's one whose expanded state is the
    /// popover's: a `commandfor` popover it sends a popover command, or (without `commandfor`) its
    /// `popovertarget`. HTML-AAM maps both to `aria-expanded`.
    KRYS_NODISCARD RefPtr<dom::Element> ExpandedStatePopover(const dom::Node *node) noexcept
    {
      if (auto *button = DynamicDowncast<html::HTMLButtonElement>(node))
      {
        if (auto target = button->CommandForElement())
        {
          switch (button->GetCommandType())
          {
            case dom::CommandType::ShowPopover:
            case dom::CommandType::HidePopover:
            case dom::CommandType::TogglePopover:
              if (target->GetPopoverState() != dom::PopoverState::None)
              {
                return target;
              }
              return nullptr;
            default: return nullptr;
          }
        }
      }
      if (auto *control = DynamicDowncast<html::HTMLFormControlElement>(node))
      {
        return control->PopoverTargetElement();
      }
      return nullptr;
    }
  }

  bool AccessibilityNodeObject::SupportsExpanded() const noexcept
  {
    if (ExpandedStatePopover(GetNode()) || HTMLTagNameOf(GetNode()) == TagName::details
        || Role() == AccessibilityRole::Summary)
    {
      return true;
    }
    // Undefined values aren't exposed, and only the roles ARIA allows aria-expanded on support it.
    auto expanded = GetAttribute(u8"aria-expanded");
    bool hasValidValue = EqualsIgnoringCase(expanded, u8"true") || EqualsIgnoringCase(expanded, u8"false");
    if (IsColumnHeader() || IsRowHeader())
    {
      return hasValidValue;
    }
    switch (Role())
    {
      case AccessibilityRole::Details:          return true;
      case AccessibilityRole::Button:
      case AccessibilityRole::Checkbox:
      case AccessibilityRole::ComboBox:
      case AccessibilityRole::GridCell:
      case AccessibilityRole::Link:
      case AccessibilityRole::MenuItem:
      case AccessibilityRole::MenuItemCheckbox:
      case AccessibilityRole::MenuItemRadio:
      case AccessibilityRole::Row:
      case AccessibilityRole::Switch:
      case AccessibilityRole::Tab:
      case AccessibilityRole::TreeItem:
      case AccessibilityRole::WebApplication:   return hasValidValue;
      default:                                  return false;
    }
  }

  bool AccessibilityNodeObject::IsExpanded() const noexcept
  {
    auto *node = GetNode();
    if (HTMLTagNameOf(node) == TagName::details)
    {
      return DynamicDowncast<dom::Element>(node)->HasAttribute(u8"open");
    }
    if (Role() == AccessibilityRole::Summary)
    {
      // A summary is expanded when its details is open.
      auto *details = node ? node->ParentElement() : nullptr;
      return details && HTMLTagNameOf(details) == TagName::details && details->HasAttribute(u8"open");
    }
    if (!SupportsExpanded())
    {
      return false;
    }
    if (auto popover = ExpandedStatePopover(node))
    {
      return popover->IsPopoverShowing();
    }
    return EqualsIgnoringCase(GetAttribute(u8"aria-expanded"), u8"true");
  }

  bool AccessibilityNodeObject::CanSetFocusAttribute() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return false;
    }
    if (IsWebArea())
    {
      return true;
    }
    auto *element = DynamicDowncast<dom::Element>(*node);
    return element && !element->IsDisabledFormControl() && html::FocusAlgorithms::IsFocusableArea(*element);
  }

  bool AccessibilityNodeObject::CanSetValueAttribute() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return false;
    }
    // The host language's readonly beats aria-readonly.
    if (auto *textArea = DynamicDowncast<html::HTMLTextAreaElement>(*node))
    {
      return !textArea->ReadOnly();
    }
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(*node); input && input->IsTextField())
    {
      return !input->ReadOnly();
    }
    auto readOnly = GetAttributeTrimmed(u8"aria-readonly");
    if (!readOnly.empty())
    {
      return !EqualsIgnoringCase(readOnly, u8"true");
    }
    if (IsNonNativeTextControl())
    {
      return true;
    }
    if (Role() == AccessibilityRole::Meter)
    {
      return false;
    }
    if (IsProgressIndicator() || IsSlider() || Role() == AccessibilityRole::ScrollBar)
    {
      return true;
    }
    if (IsWebArea())
    {
      auto *document = GetDocument();
      auto body = document ? document->Body() : nullptr;
      return document && ((body && body->HasEditableStyle()) || document->HasEditableStyle());
    }
    return node->HasEditableStyle();
  }

  namespace
  {
    /// @brief @p node, if it's a control a press acts on itself.
    KRYS_NODISCARD dom::Element *NodeActionElement(dom::Node &node) noexcept
    {
      if (auto *input = DynamicDowncast<html::HTMLInputElement>(node))
      {
        if (!input->IsDisabledFormControl()
            && (input->IsRadioButton() || input->IsCheckbox() || input->IsTextButton()
                || input->IsFileUpload() || input->IsImageButton() || input->IsTextField()
                || input->IsDateField() || input->IsDateTimeLocalField()))
        {
          return input;
        }
      }
      else if (auto tag = HTMLTagNameOf(&node); tag == TagName::button || tag == TagName::select)
      {
        return &Downcast<dom::Element>(node);
      }
      // Editable content takes presses too.
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && ContentEditableAttributeIsEnabled(*element) ? element : nullptr;
    }

    /// @brief A control the author hid inside an ARIA control (a group with role=button around a button).
    KRYS_NODISCARD dom::Element *NativeActionElement(dom::Node &start) noexcept
    {
      for (auto *child = start.FirstChild(); child; child = child->NextSibling())
      {
        if (auto *element = NodeActionElement(*child))
        {
          return element;
        }
        if (auto *element = NativeActionElement(*child))
        {
          return element;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD bool IsARIAInputRole(AccessibilityRole role) noexcept
    {
      switch (role)
      {
        case AccessibilityRole::RadioButton:
        case AccessibilityRole::Checkbox:
        case AccessibilityRole::TextField:
        case AccessibilityRole::Switch:
        case AccessibilityRole::SearchField: return true;
        default:                             return false;
      }
    }

    enum class StepAction : bool
    {
      Decrement,
      Increment,
    };
  }

  dom::Element *AccessibilityNodeObject::AnchorElement() const noexcept
  {
    auto *cache = AxObjectCache();
    for (auto *node = GetNode(); node; node = node->ParentNode())
    {
      if (Is<html::HTMLAnchorElement>(*node))
      {
        return &Downcast<dom::Element>(*node);
      }
      if (auto *renderer = node->Renderer(); renderer && cache)
      {
        if (auto *object = cache->GetOrCreate(*renderer); object && object->IsLink())
        {
          return DynamicDowncast<dom::Element>(*node);
        }
      }
    }
    return nullptr;
  }

  dom::Element *AccessibilityNodeObject::ActionElement() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return nullptr;
    }
    if (auto *element = NodeActionElement(*node))
    {
      return element;
    }
    if (IsARIAInputRole(_ariaRole))
    {
      return DynamicDowncast<dom::Element>(*node);
    }
    switch (Role())
    {
      case AccessibilityRole::Button:
      case AccessibilityRole::PopUpButton:
      case AccessibilityRole::ToggleButton:
      case AccessibilityRole::Tab:
      case AccessibilityRole::MenuItem:
      case AccessibilityRole::MenuItemCheckbox:
      case AccessibilityRole::MenuItemRadio:
      case AccessibilityRole::ListItem:
        if (auto *element = NativeActionElement(*node))
        {
          return element;
        }
        return DynamicDowncast<dom::Element>(*node);
      default: break;
    }
    if (auto *anchor = AnchorElement())
    {
      return anchor;
    }
    if (auto *clickable = ClickableSelfOrAncestor())
    {
      return clickable->GetElement();
    }
    return nullptr;
  }

  bool AccessibilityNodeObject::HasClickHandler() const noexcept
  {
    auto *element = GetElement();
    return element
           && (element->HasEventListener(dom::DOMStringAtom(EventNames::Click))
               || element->HasEventListener(dom::DOMStringAtom(EventNames::MouseDown))
               || element->HasEventListener(dom::DOMStringAtom(EventNames::MouseUp)));
  }

  void AccessibilityNodeObject::SetFocused(bool focused) noexcept
  {
    AccessibilityObject::SetFocused(focused);
    if (!CanSetFocusAttribute())
    {
      return;
    }
    RefPtr<dom::Document> document = ShareRefPtr(GetDocument());
    if (!document)
    {
      return;
    }
    Ref protectedThis = ShareRef(*this);
    if (auto *window = document->Window())
    {
      window->NotifyActivated();
    }
    auto *element = DynamicDowncast<dom::Element>(GetNode());
    auto *focusedElement = document->FocusedElement();
    // Focusing the focused element does nothing, so it's unfocused first (focus may have gone to the host's
    // chrome and come back).
    if (focusedElement && (!focused || !element || focusedElement == element))
    {
      html::FocusAlgorithms::UnfocusingSteps(*focusedElement);
    }
    if (focused && element)
    {
      html::FocusAlgorithms::FocusingSteps(*element);
    }
  }

  namespace
  {
    /// @brief Simulates the arrow key that steps @p object (one whose value can't be set), for the author to
    /// handle; whether the keys were handled.
    bool PostKeyboardKeysForValueChange(AccessibilityNodeObject &object, StepAction action) noexcept
    {
      auto *node = object.GetNode();
      auto *document = object.GetDocument();
      if (!node || !document)
      {
        return false;
      }
      // A spin button steps with the up and down arrows (https://w3c.github.io/aria/#spinbutton).
      bool vertical = object.Orientation() == AccessibilityOrientation::Vertical
                      || object.Role() == AccessibilityRole::SpinButton;
      bool increment = action == StepAction::Increment;
      dom::KeyboardEventInit init;
      init.Key =
        vertical ? (increment ? u8"ArrowUp" : u8"ArrowDown") : (increment ? u8"ArrowRight" : u8"ArrowLeft");
      init.Code = init.Key;
      init.KeyIdentifier = vertical ? (increment ? u8"Up" : u8"Down") : (increment ? u8"Right" : u8"Left");
      init.KeyCode = vertical ? (increment ? 38u : 40u) : (increment ? 39u : 37u);
      init.Which = init.KeyCode;
      init.View = document->Window();
      init.Bubbles = true;
      init.Cancelable = true;
      init.Composed = true;

      Ref protectedNode = ShareRef(*node);
      bool handled =
        !dom::EventDispatcher::FireEvent<dom::KeyboardEvent>(dom::DOMStringAtom(u8"keydown"), *node, init);
      handled |=
        !dom::EventDispatcher::FireEvent<dom::KeyboardEvent>(dom::DOMStringAtom(u8"keyup"), *node, init);
      return handled;
    }

    void SetNodeValue(AccessibilityNodeObject &object, StepAction action, float value) noexcept
    {
      if (object.SetValue(value))
      {
        if (auto *cache = object.AxObjectCache())
        {
          cache->PostNotification(&object, AXNotification::ValueChanged);
        }
        return;
      }
      (void)PostKeyboardKeysForValueChange(object, action);
    }

    void AlterRangeValue(AccessibilityNodeObject &object, StepAction action) noexcept
    {
      if (object.Role() != AccessibilityRole::Slider && object.Role() != AccessibilityRole::SpinButton)
      {
        return;
      }
      auto *element = object.GetElement();
      if (!element || element->IsDisabledFormControl())
      {
        return;
      }
      if (auto *document = object.GetDocument(); document && document->Window())
      {
        document->Window()->NotifyActivated();
      }

      auto value = object.ValueForRange();
      auto stepAttribute = object.GetAttributeTrimmed(u8"step");
      if (auto step = html::Numbers::ParseFloatingPoint(stepAttribute);
          !stepAttribute.empty() && step.Success())
      {
        value +=
          action == StepAction::Increment ? static_cast<float>(step.Value) : -static_cast<float>(step.Value);
      }
      else
      {
        // Five percent of the range, and at least one.
        auto change = (object.MaxValueForRange() - object.MinValueForRange()) * 0.05f;
        if (std::abs(change) < 1)
        {
          change = 1;
        }
        value += action == StepAction::Increment ? change : -change;
      }
      SetNodeValue(object, action, value);
    }
  }

  void AccessibilityNodeObject::Increment() noexcept
  {
    AlterRangeValue(*this, StepAction::Increment);
  }

  void AccessibilityNodeObject::Decrement() noexcept
  {
    AlterRangeValue(*this, StepAction::Decrement);
  }

  void AccessibilityNodeObject::SetIsExpanded(bool expanded) noexcept
  {
    if (auto *details = DynamicDowncast<html::HTMLDetailsElement>(GetNode());
        details && expanded != details->HasAttribute(dom::DOMStringAtom(u8"open")))
    {
      details->ToggleOpen();
    }
  }

  uint32 AccessibilityNodeObject::HeadingLevel() const noexcept
  {
    // An ARIA heading's level is aria-level, 2 by default (https://w3c.github.io/aria/#heading).
    if (_ariaRole == AccessibilityRole::Heading)
    {
      auto level = GetAttributeTrimmed(u8"aria-level");
      uint32 value = 0;
      for (auto character : level)
      {
        if (character < u8'0' || character > u8'9')
        {
          value = 0;
          break;
        }
        value = value * 10 + static_cast<uint32>(character - u8'0');
      }
      return value ? value : 2;
    }
    if (auto *heading = DynamicDowncast<html::HTMLHeadingElement>(GetNode()))
    {
      return heading->ComputedHeadingLevel();
    }
    return 0;
  }

  dom::DOMString AccessibilityNodeObject::AccessibleName() const noexcept
  {
    // A web area (the document's view, which has no node) is named by the html element's aria-label, or else
    // the document's title. (WebKit's order; a frame's web area would be named by its owner element first.)
    if (auto *document = IsWebArea() ? GetDocument() : nullptr)
    {
      if (auto root = document->DocumentElement())
      {
        if (auto label = StringAlgorithms::StripAndCollapseASCIIWhitespace(
              AttributeWithDefaultARIA(*root, u8"aria-label"));
            !label.empty())
        {
          return label;
        }
      }
      return document->Title();
    }
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!node || !cache)
    {
      return {};
    }
    return TextAlternativeComputation(*cache, TextAlternativeComputation::Kind::Name, *node).Compute();
  }

  dom::DOMString AccessibilityNodeObject::AccessibleDescription() const noexcept
  {
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!node || !cache)
    {
      return {};
    }
    // HTML-AAM's accessible description: aria-describedby, then aria-description, then the title if it isn't
    // the name.
    // https://w3c.github.io/html-aam/#accdesc-computation
    auto description =
      TextAlternativeComputation(*cache, TextAlternativeComputation::Kind::Description, *node).Compute();
    if (!description.empty())
    {
      return description;
    }
    if (auto ariaDescription =
          StringAlgorithms::StripAndCollapseASCIIWhitespace(GetAttribute(u8"aria-description"));
        !ariaDescription.empty())
    {
      return ariaDescription;
    }
    if (Is<svg::SVGElement>(*node))
    {
      return {};
    }
    auto title = StringAlgorithms::StripAndCollapseASCIIWhitespace(GetAttribute(u8"title"));
    if (!title.empty() && title != AccessibleName())
    {
      return title;
    }
    return {};
  }

  dom::DOMString AccessibilityNodeObject::StringValue() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return {};
    }
    if (auto *text = DynamicDowncast<dom::Text>(*node))
    {
      return dom::DOMString(text->Data());
    }
    if (auto *input = DynamicDowncast<html::HTMLInputElement>(*node))
    {
      // A password's characters aren't exposed.
      if (input->IsPasswordField())
      {
        return {};
      }
      return input->Value();
    }
    if (auto *textArea = DynamicDowncast<html::HTMLTextAreaElement>(*node))
    {
      return textArea->Value();
    }
    if (auto *select = DynamicDowncast<html::HTMLSelectElement>(*node); select && !select->Multiple())
    {
      auto index = select->SelectedIndex();
      if (index >= 0 && static_cast<size_t>(index) < select->ListItems().size())
      {
        if (auto *option = DynamicDowncast<html::HTMLOptionElement>(select->ListItems()[index].get()))
        {
          // The option's aria-label overrides its text.
          if (auto label = AttributeWithDefaultARIA(*option, u8"aria-label"); !label.empty())
          {
            return dom::DOMString(StringAlgorithms::StripLeadingAndTrailingWhitespace(label));
          }
          return option->Label();
        }
      }
      return {};
    }
    // An ARIA combo box's value is its list box's selected option's (its name, if it has no value: an ARIA
    // option has none, though WebKit leaves the combo box's value empty then).
    if (IsComboBox())
    {
      for (const auto &child : const_cast<AccessibilityNodeObject *>(this)->Children())
      {
        if (child->IsListBox())
        {
          if (auto selected = child->SelectedChildren(); !selected.empty())
          {
            auto value = selected.front()->StringValue();
            return value.empty() ? selected.front()->AccessibleName() : value;
          }
          break;
        }
      }
    }
    if (IsNonNativeTextControl())
    {
      return Text();
    }
    // (Range, colour and date inputs give their values above; ARIA ranges theirs through ValueForRange and
    // ValueDescription.)
    return {};
  }

  // Text ranges.

  namespace
  {
    /// @brief The text form control @p object is, if it's a native text control.
    KRYS_NODISCARD html::HTMLTextFormControlElement *
      NativeTextControl(const AccessibilityNodeObject &object) noexcept
    {
      return object.IsNativeTextControl()
               ? DynamicDowncast<html::HTMLTextFormControlElement>(object.GetNode())
               : nullptr;
    }

    KRYS_NODISCARD bool IsPasswordField(const AccessibilityNodeObject &object) noexcept
    {
      auto *input = DynamicDowncast<html::HTMLInputElement>(object.GetNode());
      return input && input->IsPasswordField();
    }
  }

  dom::DOMString AccessibilityNodeObject::Text() const noexcept
  {
    if (IsStaticText())
    {
      return StringValue();
    }
    if (!IsTextControl())
    {
      return {};
    }
    if (IsNativeTextControl())
    {
      // A password's characters aren't exposed.
      return StringValue();
    }
    auto *element = DynamicDowncast<html::HTMLElement>(GetElement());
    return element ? element->InnerText() : dom::DOMString {};
  }

  VisiblePositionRange AccessibilityNodeObject::GetVisiblePositionRange() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return {};
    }
    VisiblePosition start(FirstPositionInOrBeforeNode(node));
    VisiblePosition end(LastPositionInOrAfterNode(node));
    // The positions are the same for a node like a button: the end is after it.
    if (start == end)
    {
      end = end.Next();
      if (end.IsNull())
      {
        end = start;
      }
    }
    return {start, end};
  }

  VisiblePositionRange AccessibilityNodeObject::SelectedVisiblePositionRange() const noexcept
  {
    auto *document = GetDocument();
    if (!document || !document->Frame())
    {
      return {};
    }
    const auto &selection = document->Selection().Selection();
    if (selection.IsNone())
    {
      return {};
    }
    return {selection.VisibleStart(), selection.VisibleEnd()};
  }

  void
    AccessibilityNodeObject::SetSelectedVisiblePositionRange(const VisiblePositionRange &range) const noexcept
  {
    auto *document = GetDocument();
    if (range.IsNull() || !document || !document->Frame())
    {
      return;
    }
    if (auto *textControl = NativeTextControl(*this))
    {
      auto start = textControl->IndexForVisiblePosition(range.Start);
      auto end = textControl->IndexForVisiblePosition(range.End);
      // A range around the whole control (rather than in its inner text) selects all of it.
      if (range.Start.DeepEquivalent().AnchorNode() == textControl
          && range.End.DeepEquivalent().AnchorNode() == textControl)
      {
        start = 0;
        end = static_cast<int32>(Text().size());
      }
      Ref protectedControl = ShareRef(*textControl);
      textControl->Focus();
      (void)textControl->SetSelectionRange(static_cast<uint32>(std::max(start, 0)),
                                           static_cast<uint32>(std::max(end, 0)));
      return;
    }
    if (!Renderer())
    {
      return;
    }
    auto &frameSelection = document->Selection();
    if (range.Start == range.End)
    {
      // A caret outside the object goes to its start.
      auto start = range.Start;
      if (auto *node = GetNode(); node && !Contains(GetVisiblePositionRange(), start))
      {
        start = VisiblePosition(FirstPositionInOrBeforeNode(node));
      }
      frameSelection.MoveTo(start, UserTriggered::Yes);
    }
    else
    {
      frameSelection.SetSelection(VisibleSelection(range.Start, range.End),
                                  FrameSelection::DefaultSetSelectionOptions(UserTriggered::Yes));
    }
  }

  int32 AccessibilityNodeObject::IndexForVisiblePosition(const VisiblePosition &position) const noexcept
  {
    if (auto *textControl = NativeTextControl(*this))
    {
      return textControl->IndexForVisiblePosition(position);
    }
    auto *node = GetNode();
    return node ? boo::IndexForVisiblePosition(*node, position, {}) : 0;
  }

  VisiblePosition AccessibilityNodeObject::VisiblePositionForIndex(int32 index) const noexcept
  {
    if (auto *textControl = NativeTextControl(*this))
    {
      return textControl->VisiblePositionForIndex(std::clamp(index, 0, static_cast<int32>(Text().size())));
    }
    auto *node = GetNode();
    return node ? VisiblePositionForIndexUsingCharacterIterator(*node, index) : VisiblePosition {};
  }

  CharacterRange AccessibilityNodeObject::SelectedTextRange() const noexcept
  {
    if (auto *textControl = NativeTextControl(*this))
    {
      auto start = textControl->SelectionStart();
      return {start, textControl->SelectionEnd() - start};
    }
    // The document's selection, as indexes in the object's text.
    auto selection = SelectedVisiblePositionRange();
    if (selection.IsNull())
    {
      return {};
    }
    auto start = std::max(IndexForVisiblePosition(selection.Start), 0);
    auto end = std::max(IndexForVisiblePosition(selection.End), start);
    return {static_cast<uint64>(start), static_cast<uint64>(end - start)};
  }

  dom::DOMString AccessibilityNodeObject::SelectedText() const noexcept
  {
    if (!IsTextControl() || IsPasswordField(*this))
    {
      return {};
    }
    if (auto *textControl = NativeTextControl(*this))
    {
      return textControl->SelectedText();
    }
    return DoAXStringForRange(SelectedTextRange());
  }

  AccessibilityObject *AccessibilityNodeObject::ObservableObject() const noexcept
  {
    // A table tells the platform about its rows.
    if (auto *element = GetElement(); element && table_helpers::IsTableRowElement(*element))
    {
      if (auto *table = ParentTable(); table && table->IsExposableTable())
      {
        return table;
      }
    }

    // The nearest text control, list box or editable region.
    auto *cache = AxObjectCache();
    for (auto *renderer = Renderer(); cache && renderer && renderer->GetNode(); renderer = renderer->Parent())
    {
      auto *rendererElement = DynamicDowncast<dom::Element>(renderer->GetNode());
      if (Is<RenderTextControl>(*renderer) || Is<RenderListBox>(*renderer)
          || (rendererElement
              && (table_helpers::HasRole(*rendererElement, u8"listbox")
                  || table_helpers::HasRole(*rendererElement, u8"textbox")
                  || ContentEditableAttributeIsEnabled(*rendererElement))))
      {
        return cache->GetOrCreate(*renderer);
      }
    }
    return nullptr;
  }

  // Tables.

  AXObjectRareData &AccessibilityNodeObject::EnsureRareData() noexcept
  {
    if (!_rareData)
    {
      _rareData = CreateUnique<AXObjectRareData>();
    }
    return *_rareData;
  }

  bool AccessibilityNodeObject::IsTable() const noexcept
  {
    if (table_helpers::IsTableRole(_ariaRole))
    {
      return true;
    }
    if (_ariaRole != AccessibilityRole::Unknown)
    {
      // If the ARIA role is set to a non-table role, this isn't a table.
      return false;
    }
    // Some sites put display: table on tbody, thead or tfoot, making a table renderer for it: those aren't
    // tables (they're typically in an actual table element).
    auto *node = GetNode();
    return (Is<RenderTable>(Renderer()) && !Is<html::HTMLTableSectionElement>(node))
           || Is<html::HTMLTableElement>(node);
  }

  bool AccessibilityNodeObject::IsAriaTable() const noexcept
  {
    return table_helpers::IsTableRole(_ariaRole);
  }

  bool AccessibilityNodeObject::IsExposableTable() const noexcept
  {
    // IsTable() is last because it's the most expensive.
    return _rareData && _rareData->IsExposableTable() && IsTable();
  }

  bool AccessibilityNodeObject::IsTableRow() const noexcept
  {
    auto *element = GetElement();
    return element && table_helpers::IsTableRowElement(*element);
  }

  bool AccessibilityNodeObject::IsExposedTableRow() const noexcept
  {
    return ParentTableIfExposedTableRow();
  }

  bool AccessibilityNodeObject::IsARIAGridRow() const noexcept
  {
    auto *element = GetElement();
    return element && table_helpers::HasRowRole(*element);
  }

  bool AccessibilityNodeObject::IsTableCell() const noexcept
  {
    auto *element = GetElement();
    return element && table_helpers::IsTableCellElement(*element);
  }

  bool AccessibilityNodeObject::IsARIAGridCell() const noexcept
  {
    auto *element = GetElement();
    return element && table_helpers::HasCellARIARole(*element);
  }

  bool AccessibilityNodeObject::IsExposedTableCell() const noexcept
  {
    // If the parent table is an accessibility table, then we are a table cell. (Checking that the unignored
    // parent is a row is much slower in nested tables.)
    if (!IsTableCell())
    {
      return false;
    }
    auto *parentTable = ParentTable();
    return parentTable && parentTable->IsExposableTable();
  }

  AccessibilityObject *AccessibilityNodeObject::ParentTableIfExposedTableRow() const noexcept
  {
    if (!IsTableRow())
    {
      return nullptr;
    }
    auto *table = ParentTable();
    return table && table->IsExposableTable() ? table : nullptr;
  }

  AccessibilityObject *AccessibilityNodeObject::ParentTable() const noexcept
  {
    auto *cache = AxObjectCache();
    if (!cache)
    {
      return nullptr;
    }

    // Grid cells may have several levels of unignored ancestors that aren't the table, including rows and
    // interactive row groups; and badly formed grids may contain elements that pass the tests for inclusion.
    if (IsARIAGridCell())
    {
      for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
      {
        if (ancestor->IsExposableTable() && !ancestor->IsIgnored())
        {
          return ancestor;
        }
      }
      return nullptr;
    }

    if (IsTableCell())
    {
      // Get (not create) the table's object: this can be called while the render tree is being changed, when
      // creating a table's object can reach into the render tree in a bad state. The table's object is made
      // before its cells' when assistive technology asks for a table.
      AccessibilityObject *tableFromRenderTree = nullptr;
      if (auto *cellRenderer = DynamicDowncast<RenderTableCell>(Renderer()))
      {
        if (auto *table = cellRenderer->Table())
        {
          tableFromRenderTree = cache->Get(*table);
        }
      }

      if (!tableFromRenderTree || !tableFromRenderTree->IsTable())
      {
        if (!GetNode())
        {
          return nullptr;
        }
        for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
        {
          if (ancestor->IsTable())
          {
            return ancestor;
          }
        }
        return nullptr;
      }

      // The cell renderer's table can be anonymous: find the right table.
      if (!tableFromRenderTree->GetNode())
      {
        for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
        {
          // Stop at a table with a node that isn't exposed: another table further up isn't this cell's.
          if (ancestor->IsTable())
          {
            if (ancestor->IsExposableTable())
            {
              return ancestor;
            }
            if (ancestor->GetNode())
            {
              break;
            }
          }
        }
        return nullptr;
      }
      return tableFromRenderTree;
    }

    if (IsTableRow())
    {
      // The table might not be the row's parent: ARIA says grids only have rows, but they're handled if not.
      for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
      {
        if (!ancestor->IsTable())
        {
          continue;
        }
        bool isNonGridRowOrValidAriaTable =
          !IsARIAGridRow() || ancestor->IsAriaTable() || HTMLTagNameOf(GetNode()) == TagName::tr;
        if (ancestor->IsExposableTable() && isNonGridRowOrValidAriaTable)
        {
          return ancestor;
        }
        // Stop at a table with a node that isn't exposed, so as not to take an outer table as this row's.
        // Grids can have tables between their rows and themselves, so keep going for grid rows.
        if (!IsARIAGridRow() && ancestor->GetNode())
        {
          break;
        }
      }
    }
    return nullptr;
  }

  AccessibilityNodeObject *AccessibilityNodeObject::ParentRow() const noexcept
  {
    auto *parent = IsTableCell() ? ParentObjectUnignored() : nullptr;
    return parent && parent->IsExposedTableRow() ? DynamicDowncast<AccessibilityNodeObject>(parent) : nullptr;
  }

  bool AccessibilityNodeObject::IsDataTable() const noexcept
  {
    // A heuristic for whether a table element is a data table (exposed as a table) or for layout.
    auto *cache = AxObjectCache();
    if (!cache)
    {
      return false;
    }
    if (!table_helpers::IsTableRole(_ariaRole) && _ariaRole != AccessibilityRole::Unknown)
    {
      // Do not consider it a data table if it has a non-table ARIA role.
      return false;
    }

    // All the tables in an editable region are data tables, or users may not be able to work with rich text
    // editors' tables.
    auto *node = GetNode();
    if (node && node->HasEditableStyle())
    {
      return true;
    }

    if (auto *tableElement = table_helpers::TableElementIncludingAncestors(node, Renderer()))
    {
      if (table_helpers::TableElementIndicatesAccessibleTable(*tableElement))
      {
        return true;
      }
    }

    // The rest only applies to an actual table element.
    auto *table = DynamicDowncast<html::HTMLTableElement>(node);
    if (!table)
    {
      return false;
    }

    // A valid column or row count says the author wants a data table.
    auto ariaRowOrColCountIsSet = [this](dom::DOMStringView name)
    {
      auto count = IntegralAttribute(name);
      return count == -1 || count > 0;
    };
    if (ariaRowOrColCountIsSet(u8"aria-colcount") || ariaRowOrColCountIsSet(u8"aria-rowcount"))
    {
      return true;
    }
    return table_helpers::IsDataTableWithTraversal(*table, *cache);
  }

  void AccessibilityNodeObject::RecomputeIsExposableIfNecessary() noexcept
  {
    if (!IsTable())
    {
      return;
    }
    // Make sure the children are up to date: if the exposed state changes, UpdateRowDescendantRoles must go
    // over them before they change.
    UpdateChildrenIfNecessary();
    auto &rareData = EnsureRareData();

    bool previouslyExposable = rareData.IsExposableTable();
    bool newIsExposable = ComputeIsTableExposableThroughAccessibility();
    rareData.SetIsExposableTable(newIsExposable);
    if (previouslyExposable != newIsExposable)
    {
      // A table's role depends on whether it's exposed.
      UpdateRole();
      // A cell's role depends on whether its table is exposed: update them before the children are reset.
      UpdateRowDescendantRoles();
      _childrenDirty = true;
    }
  }

  AXObjectRareData *AccessibilityNodeObject::RareDataWithCleanTableChildren() noexcept
  {
    if (!IsTable())
    {
      return nullptr;
    }
    UpdateChildrenIfNecessary();
    return _rareData.get();
  }

  AccessibilityObject *AccessibilityNodeObject::CellForColumnAndRow(uint32 column, uint32 row) noexcept
  {
    auto *rareData = RareDataWithCleanTableChildren();
    if (!rareData)
    {
      return nullptr;
    }
    const auto &cellSlots = rareData->CellSlots();
    if (row >= cellSlots.size() || column >= cellSlots[row].size())
    {
      return nullptr;
    }
    auto *cache = AxObjectCache();
    auto cellID = cellSlots[row][column];
    return cellID && cache ? cache->ObjectForID(*cellID) : nullptr;
  }

  AccessibilityObject::AccessibilityChildrenVector AccessibilityNodeObject::Cells() noexcept
  {
    auto *rareData = RareDataWithCleanTableChildren();
    if (!rareData)
    {
      return {};
    }
    AccessibilityChildrenVector cells;
    // Rows times columns isn't exact with row and column spans, but it's close.
    cells.reserve(rareData->RowCount() * rareData->ColumnCount());
    for (const auto &row : rareData->TableRows())
    {
      const auto &rowChildren = row->Children();
      cells.insert(cells.end(), rowChildren.begin(), rowChildren.end());
    }
    return cells;
  }

  uint32 AccessibilityNodeObject::ColumnCount() noexcept
  {
    auto *rareData = RareDataWithCleanTableChildren();
    return rareData ? rareData->ColumnCount() : 0;
  }

  uint32 AccessibilityNodeObject::RowCount() noexcept
  {
    auto *rareData = RareDataWithCleanTableChildren();
    return rareData ? rareData->RowCount() : 0;
  }

  AccessibilityObject::AccessibilityChildrenVector AccessibilityNodeObject::Columns() noexcept
  {
    auto *rareData = RareDataWithCleanTableChildren();
    return rareData ? rareData->TableColumns() : AccessibilityChildrenVector {};
  }

  AccessibilityObject::AccessibilityChildrenVector AccessibilityNodeObject::Rows() noexcept
  {
    auto *rareData = RareDataWithCleanTableChildren();
    return rareData ? rareData->TableRows() : AccessibilityChildrenVector {};
  }

  int32 AccessibilityNodeObject::AxRowCount() const noexcept
  {
    if (!IsTable())
    {
      return 0;
    }
    // "Authors must set the value of aria-rowcount to an integer equal to the number of rows in the full
    // table. If the total number of rows is unknown, authors must set the value of aria-rowcount to -1"
    // (https://w3c.github.io/aria/#aria-rowcount).
    auto rowCount = IntegralAttribute(u8"aria-rowcount");
    if (rowCount == -1
        || rowCount >= static_cast<int32>(const_cast<AccessibilityNodeObject *>(this)->RowCount()))
    {
      return rowCount;
    }
    return 0;
  }

  int32 AccessibilityNodeObject::AxColumnCount() const noexcept
  {
    if (!IsTable())
    {
      return 0;
    }
    // As for aria-rowcount (https://w3c.github.io/aria/#aria-colcount).
    auto columnCount = IntegralAttribute(u8"aria-colcount");
    if (columnCount == -1
        || columnCount >= static_cast<int32>(const_cast<AccessibilityNodeObject *>(this)->ColumnCount()))
    {
      return columnCount;
    }
    return 0;
  }

  void AccessibilityNodeObject::UpdateRowDescendantRoles() noexcept
  {
    if (!_rareData || !IsTable())
    {
      return;
    }
    // Copied: updating a role can change the table's children.
    auto rows = _rareData->TableRows();
    for (const auto &row : rows)
    {
      row->UpdateRole();
      auto cells = row->Children();
      for (const auto &cell : cells)
      {
        cell->UpdateRole();
      }
    }
  }

  void AccessibilityNodeObject::SetCellSlotsDirty() noexcept
  {
    if (!IsTable())
    {
      return;
    }
    // The cell slots are worked out with the children, since the same things change both (a cell's column
    // span changing can widen the table, adding columns): clear the children to work them out again.
    ClearChildren();
  }

  AccessibilityObject *AccessibilityNodeObject::TableHeaderContainer() noexcept
  {
    auto *rareData = RareDataWithCleanTableChildren();
    if (!rareData)
    {
      return nullptr;
    }
    if (auto *headerContainer = rareData->TableHeaderContainer())
    {
      return headerContainer;
    }
    auto *cache = AxObjectCache();
    auto *tableHeader = cache ? DynamicDowncast<AccessibilityTableHeaderContainer>(
                                  cache->Create(AccessibilityRole::TableHeaderContainer))
                              : nullptr;
    if (!tableHeader)
    {
      return nullptr;
    }
    tableHeader->SetParent(this);
    rareData->SetTableHeaderContainer(*tableHeader);
    return tableHeader;
  }

  AccessibilityObject::AccessibilityChildrenVector AccessibilityNodeObject::RowHeaders() noexcept
  {
    AccessibilityChildrenVector headers;
    if (IsTableRow() || IsTable())
    {
      for (const auto &row : Rows())
      {
        if (auto *header = row->RowHeader())
        {
          headers.emplace_back(ShareRef(*header));
        }
      }
    }
    else if (IsTableCell())
    {
      auto parent = ShareRefPtr(ParentTable());
      if (!parent)
      {
        return headers;
      }
      auto rowRange = RowIndexRange();
      auto columnRange = ColumnIndexRange();
      for (uint32 column = 0; column < columnRange.first; ++column)
      {
        auto *tableCell = parent->CellForColumnAndRow(column, rowRange.first);
        if (!tableCell || tableCell == this
            || std::ranges::any_of(headers, [&](const auto &header) { return header.get() == tableCell; }))
        {
          continue;
        }
        if ((tableCell->CellScope() == u8"rowgroup" && IsTableCellInSameRowGroup(*tableCell))
            || tableCell->IsRowHeader())
        {
          headers.emplace_back(ShareRef(*tableCell));
        }
      }
    }
    return headers;
  }

  void AccessibilityNodeObject::AddTableChildrenAndCellSlots() noexcept
  {
    // IsExposableTable() is checked before this is called.
    krys_debug_assert(IsExposableTable());
    auto *cache = AxObjectCache();
    if (!IsExposableTable() || !cache)
    {
      return;
    }
    auto desiredColumnCount = ComputeCellSlots();

    auto &rareData = EnsureRareData();
    for (uint32 i = 0; i < desiredColumnCount; ++i)
    {
      auto *column = DynamicDowncast<AccessibilityTableColumn>(cache->Create(AccessibilityRole::Column));
      if (!column)
      {
        continue;
      }
      column->SetColumnIndex(i);
      column->SetParent(this);
      rareData.AppendColumn(*column);
      AddChild(column, DescendIfIgnored::No);
    }
    AddChild(TableHeaderContainer(), DescendIfIgnored::No);

    _subtreeDirty = false;
    // A cell can get the wrong role at first, made before its table works out whether it's exposed: update
    // the roles now the table knows (https://webkit.org/b/147001).
    UpdateRowDescendantRoles();
  }

  uint32 AccessibilityNodeObject::ComputeCellSlots() noexcept
  {
    if (!IsExposableTable())
    {
      return 0;
    }
    auto *cache = AxObjectCache();
    auto *tableNode = GetNode();
    if (!cache || !tableNode)
    {
      return 0;
    }
    // IsAriaTable() is true for ARIA grids, tree grids and tables.
    if (!Is<html::HTMLTableElement>(*tableNode) && !IsAriaTable())
    {
      return 0;
    }

    auto protectedThis = ShareRef(*this);
    auto &rareData = EnsureRareData();
    auto &cellSlots = rareData.MutableCellSlots();
    auto ensureRowAndColumn = [&](uint32 rowIndex, uint32 columnIndex)
    {
      if (cellSlots.size() < rowIndex + 1)
      {
        cellSlots.resize(rowIndex + 1);
      }
      if (cellSlots[rowIndex].size() < columnIndex + 1)
      {
        cellSlots[rowIndex].resize(columnIndex + 1);
      }
    };

    // The forming a table algorithm (https://html.spec.whatwg.org/#forming-a-table), working out where each
    // cell is and how far it spans.

    // Step 1.
    uint32 xWidth = 0;
    // Step 2.
    uint32 yHeight = 0;
    // Step 3: Let pending tfoot elements be a list of tfoot elements, initially empty.
    List<Ref<dom::Element>> pendingTfootElements;
    // Step 10.
    uint32 yCurrent = 0;
    bool didAddCaption = false;

    struct DownwardGrowingCell
    {
      Ref<AccessibilityNodeObject> Object;
      // The column the cell starts in.
      uint32 X;
      // The number of columns the cell spans (its width).
      uint32 ColSpan;
      uint32 RemainingRowsToSpan;
    };
    List<DownwardGrowingCell> downwardGrowingCells;

    // https://html.spec.whatwg.org/#algorithm-for-growing-downward-growing-cells
    auto growDownwardsCells = [&]
    {
      // Extend each downward-growing cell so it also covers the slots (x, yCurrent), where
      // cellX <= x < cellX + width.
      for (auto &cell : downwardGrowingCells)
      {
        if (!cell.RemainingRowsToSpan)
        {
          continue;
        }
        --cell.RemainingRowsToSpan;
        cell.Object->IncrementEffectiveRowSpan();
        for (auto column = cell.X; column < cell.X + cell.ColSpan; ++column)
        {
          ensureRowAndColumn(yCurrent, column);
          cellSlots[yCurrent][column] = cell.Object->ObjectID();
        }
      }
    };

    HashSet<const AccessibilityObject *> processedRows;
    // https://html.spec.whatwg.org/#algorithm-for-processing-rows
    auto processRow = [&](AccessibilityNodeObject *row)
    {
      if (!row || processedRows.contains(row))
      {
        return;
      }
      processedRows.insert(row);

      if (row->Role() != AccessibilityRole::Unknown && row->IsIgnored())
      {
        // Skip ignored rows (not those ignored for having an unknown role, which happens when a table that
        // stopped being exposed may be exposed again). The table's rows can't be aria-hidden or
        // presentational, unlike HTML's.
        return;
      }

      // Step 1: If yheight is equal to ycurrent, then increase yheight by 1.
      if (yHeight <= yCurrent)
      {
        yHeight = yCurrent + 1;
      }
      // Step 2.
      uint32 xCurrent = 0;
      // Step 3: Run the algorithm for growing downward-growing cells.
      growDownwardsCells();

      // Step 4: If the tr element being processed has no td or th element children, then increase ycurrent
      // by 1 and return.
      auto rowChildren = row->Children();
      for (const auto &child : rowChildren)
      {
        auto *currentCell = DynamicDowncast<AccessibilityNodeObject>(child.get());
        if (!currentCell || !currentCell->IsTableCell())
        {
          continue;
        }
        // Not specified: reset the cell's effective row span, which an earlier pass may have changed.
        currentCell->ResetEffectiveRowSpan();

        // Step 6: While the slot (xcurrent, ycurrent) already has a cell assigned to it, increase xcurrent
        // by 1.
        ensureRowAndColumn(yCurrent, xCurrent);
        while (cellSlots[yCurrent][xCurrent])
        {
          ++xCurrent;
          ensureRowAndColumn(yCurrent, xCurrent);
        }
        // Step 7: If xcurrent is equal to xwidth, increase xwidth by 1.
        if (xCurrent >= xWidth)
        {
          xWidth = xCurrent + 1;
        }
        // Steps 8 and 9: the cell's colspan and rowspan.
        auto colSpan = currentCell->ColSpan();
        auto rowSpan = currentCell->RowSpan();

        // Step 10 (a zero rowspan growing downward outside quirks mode) isn't implemented: rendering doesn't
        // implement it, and this follows what's drawn.

        // Step 11: If xwidth < xcurrent+colspan, then let xwidth be xcurrent+colspan.
        if (xWidth < xCurrent + colSpan)
        {
          xWidth = xCurrent + colSpan;
        }
        // Step 12 isn't followed: a rowspan can be far larger (up to 65534) than the rows it ends up
        // spanning. Only make sure yheight isn't less than ycurrent.
        if (yHeight < yCurrent)
        {
          yHeight = yCurrent;
        }

        // Step 13: cover the slots the cell spans. Rows are covered with the downward-growing cell algorithm
        // instead, which keeps cells from extending outside their row group.
        currentCell->SetRowIndex(yCurrent);
        currentCell->SetColumnIndex(xCurrent);
        for (auto x = xCurrent; x < xCurrent + colSpan; ++x)
        {
          ensureRowAndColumn(yCurrent, x);
          cellSlots[yCurrent][x] = currentCell->ObjectID();
        }

        // Step 14: add the cell to the downward-growing cells (for every row span, not only zero).
        if (rowSpan > 1)
        {
          downwardGrowingCells.push_back({ShareRef(*currentCell), xCurrent, colSpan, rowSpan - 1});
        }
        else if (!rowSpan)
        {
          // A zero rowspan spans all the remaining rows: let the cell grow for as long as the table extends.
          downwardGrowingCells.push_back({ShareRef(*currentCell), xCurrent, colSpan, MaxRowSpan - yCurrent});
        }

        // Step 15.
        xCurrent += colSpan;
        // Steps 16 to 18 are the loop.
      }

      // Not specified: the table's own record of its rows.
      rareData.AppendRow(*row);
      row->SetRowIndex(yCurrent);
      AddChild(row);

      // Step 16: increase ycurrent by 1.
      ++yCurrent;
    };
    auto needsToDescend = [&processedRows](const AccessibilityObject &object)
    {
      return !object.IsTableRow() && !processedRows.contains(&object);
    };
    auto processRowDescendingIfNeeded = [&](this const auto &self, AccessibilityObject &object) -> void
    {
      // Descend past anonymous renderers and things that aren't rows.
      if (needsToDescend(object))
      {
        auto children = object.Children();
        for (const auto &child : children)
        {
          self(*child);
        }
      }
      else if (object.IsTableRow())
      {
        processRow(DynamicDowncast<AccessibilityNodeObject>(object));
      }
    };
    // https://html.spec.whatwg.org/#algorithm-for-ending-a-row-group
    auto endRowGroup = [&]
    {
      // Step 1: While ycurrent is less than yheight, grow the downward-growing cells and increase ycurrent.
      while (yCurrent < yHeight)
      {
        growDownwardsCells();
        ++yCurrent;
      }
      // Step 2: Empty the list of downward-growing cells.
      downwardGrowingCells.clear();
    };
    // https://html.spec.whatwg.org/#algorithm-for-processing-row-groups
    auto processRowGroup = [&](dom::Element &sectionElement)
    {
      // Step 1 (ystart) is only needed for step 3, which isn't implemented.

      // Step 2: Process each tr element child of the row group, in tree order.
      if (auto *section = DynamicDowncast<html::HTMLTableSectionElement>(sectionElement))
      {
        for (auto *child = section->FirstElementChild().get(); child;
             child = child->NextElementSibling().get())
        {
          if (!Is<html::HTMLTableRowElement>(*child))
          {
            continue;
          }
          if (auto *row = cache->GetOrCreate(*child); row && row->IsTableRow())
          {
            processRow(DynamicDowncast<AccessibilityNodeObject>(row));
          }
        }
      }
      else if (auto *sectionObject = cache->GetOrCreate(sectionElement))
      {
        // A role=rowgroup element.
        auto children = sectionObject->Children();
        for (const auto &child : children)
        {
          processRowDescendingIfNeeded(*child);
        }
      }
      // Step 3 (making the last rows a row group of their own) isn't implemented.

      // Step 4: Run the algorithm for ending a row group.
      endRowGroup();
    };

    bool withinImplicitRowGroup = false;
    auto processTableDescendant = [&](this const auto &self, dom::Node &node) -> void
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      // Step 8: Skip children of the table that aren't rows or row groups.
      bool descendantIsRow =
        element && (HTMLTagNameOf(element) == TagName::tr || table_helpers::HasRole(*element, u8"row"));
      bool descendantIsRowGroup = !descendantIsRow && element && table_helpers::IsRowGroup(*element);

      if (HTMLTagNameOf(element) == TagName::caption)
      {
        // Step 6: Associate the first caption element child of the table element with the table.
        if (!didAddCaption)
        {
          if (auto *captionObject = cache->GetOrCreate(*element))
          {
            AddChild(captionObject, DescendIfIgnored::No);
            didAddCaption = true;
          }
        }
        return;
      }

      // Not specified: ARIA tables can have implicit row groups, which the downward-growing cell algorithm
      // needs to know about.
      withinImplicitRowGroup = !descendantIsRowGroup && IsAriaTable();

      // Step 9 (column groups) isn't implemented.
      // Step 11: Let the list of downward-growing cells be an empty list.
      if (!withinImplicitRowGroup)
      {
        downwardGrowingCells.clear();
      }
      // Step 12: Skip children of the table that aren't rows or row groups.
      if (!descendantIsRow && !descendantIsRowGroup)
      {
        if (IsAriaTable())
        {
          // Not specified, but as other browsers do: build an ARIA grid from rows below elements a grid
          // shouldn't have.
          if (auto *object = cache->GetOrCreate(node); object && needsToDescend(*object))
          {
            if (auto *container = DynamicDowncast<dom::ContainerNode>(node))
            {
              for (auto &child : dom::ComposedTreeChildren(*container))
              {
                self(child);
              }
            }
          }
        }
        return;
      }

      // Step 13: If the current element is a tr, run the algorithm for processing rows.
      if (descendantIsRow)
      {
        processRow(DynamicDowncast<AccessibilityNodeObject>(cache->GetOrCreate(*element)));
      }
      // Step 14: Run the algorithm for ending a row group.
      if (!withinImplicitRowGroup)
      {
        endRowGroup();
      }
      // Step 15: Leave a tfoot until the end.
      if (HTMLTagNameOf(element) == TagName::tfoot)
      {
        pendingTfootElements.emplace_back(ShareRef(*element));
        return;
      }
      // Step 16: Process a thead or tbody (or role=rowgroup element) as a row group.
      if (descendantIsRowGroup)
      {
        processRowGroup(*element);
      }
    };

    // Step 7: Go over the table element's children (steps 17 and 18 advance).
    for (auto *child = tableNode->FirstChild(); child; child = child->NextSibling())
    {
      processTableDescendant(*child);
    }

    // Step 19: Process each pending tfoot element, in tree order, as a row group.
    for (const auto &tfoot : pendingTfootElements)
    {
      processRowGroup(*tfoot);
    }
    return xWidth;
  }

  void AccessibilityNodeObject::SetRowIndex(uint32 rowIndex) noexcept
  {
    if (!HasCellOrRowRole())
    {
      return;
    }
    EnsureRareData().SetRowIndex(rowIndex);
  }

  void AccessibilityNodeObject::SetColumnIndex(uint32 columnIndex) noexcept
  {
    if (!IsTableCell())
    {
      return;
    }
    EnsureRareData().SetColumnIndex(columnIndex);
  }

  void AccessibilityNodeObject::IncrementEffectiveRowSpan() noexcept
  {
    if (_rareData)
    {
      _rareData->IncrementEffectiveRowSpan();
    }
  }

  void AccessibilityNodeObject::ResetEffectiveRowSpan() noexcept
  {
    if (_rareData)
    {
      _rareData->ResetEffectiveRowSpan();
    }
  }

  void AccessibilityNodeObject::SetAXColIndexFromRow(int32 index) noexcept
  {
    if (!_rareData && index == -1)
    {
      return;
    }
    EnsureRareData().SetAXColIndexFromRow(index);
  }

  Maybe<uint32> AccessibilityNodeObject::AxColumnIndex() const noexcept
  {
    if (!HasCellOrRowRole())
    {
      return null;
    }
    if (auto value = IntegralAttribute(u8"aria-colindex"); value >= 1)
    {
      return static_cast<uint32>(value);
    }
    // "If the set of columns which is present in the DOM is contiguous, and if there are no cells which span
    // more than one row or column in that set, then authors may place aria-colindex on each row, setting the
    // value to the index of the first column of the set" (https://w3c.github.io/aria/#aria-colindex): the
    // row sets its cells' indexes from it.
    if (_rareData && _rareData->AxColIndexFromRow() != -1 && ParentRow())
    {
      return static_cast<uint32>(_rareData->AxColIndexFromRow());
    }
    return null;
  }

  Maybe<uint32> AccessibilityNodeObject::AxRowIndex() const noexcept
  {
    if (!HasCellOrRowRole())
    {
      return null;
    }
    // "Authors SHOULD place aria-rowindex on each row. Authors MAY also place aria-rowindex on all of the
    // children or owned elements of each row" (https://w3c.github.io/aria/#aria-rowindex).
    if (auto value = IntegralAttribute(u8"aria-rowindex"); value >= 1)
    {
      return static_cast<uint32>(value);
    }
    if (auto *parentRow = ParentRow())
    {
      return parentRow->AxRowIndex();
    }
    return null;
  }

  dom::DOMString AccessibilityNodeObject::AxRowIndexText() const noexcept
  {
    if (HasAttribute(u8"aria-rowindextext"))
    {
      return GetAttribute(u8"aria-rowindextext");
    }
    if (auto *parentRow = IsTableCell() ? ParentRow() : nullptr)
    {
      return parentRow->AxRowIndexText();
    }
    return {};
  }

  dom::DOMString AccessibilityNodeObject::AxColumnIndexText() const noexcept
  {
    return GetAttribute(u8"aria-colindextext");
  }

  bool AccessibilityNodeObject::IsTableHeaderCell() const noexcept
  {
    auto *element = GetElement();
    auto tagName = HTMLTagNameOf(element);
    if (tagName == TagName::th)
    {
      return true;
    }
    if (tagName == TagName::td)
    {
      // In a well-formed table, the thead is two levels up from the td.
      auto *current = element->ParentNode();
      for (int i = 0; i < 2 && current; ++i)
      {
        if (HTMLTagNameOf(current) == TagName::thead)
        {
          return true;
        }
        current = current->ParentNode();
      }
    }
    return false;
  }

  bool AccessibilityNodeObject::IsColumnHeader() const noexcept
  {
    if (Role() == AccessibilityRole::ColumnHeader)
    {
      return true;
    }
    auto scope = GetAttribute(u8"scope");
    if (scope == u8"col" || scope == u8"colgroup")
    {
      return true;
    }
    if (scope == u8"row" || scope == u8"rowgroup" || !IsTableHeaderCell())
    {
      return false;
    }

    // HTML doesn't say which a th is without a scope: a th in a thead, or in the first row of a table or
    // tbody, is a column header.
    for (auto *ancestor = GetElement()->ParentNode(); ancestor; ancestor = ancestor->ParentNode())
    {
      auto tagName = HTMLTagNameOf(ancestor);
      if (tagName == TagName::thead)
      {
        return true;
      }
      if (tagName == TagName::tfoot)
      {
        return false;
      }
      if (tagName == TagName::table || tagName == TagName::tbody)
      {
        return !RowIndexRange().first;
      }
    }
    return false;
  }

  bool AccessibilityNodeObject::IsRowHeader() const noexcept
  {
    if (Role() == AccessibilityRole::RowHeader)
    {
      return true;
    }
    auto scope = GetAttribute(u8"scope");
    if (scope == u8"row" || scope == u8"rowgroup")
    {
      return true;
    }
    if (scope == u8"col" || scope == u8"colgroup" || !IsTableHeaderCell())
    {
      return false;
    }

    // HTML doesn't say which a th is without a scope: a th in the first column of a table, tbody or tfoot
    // (not a thead) is a row header.
    for (auto *ancestor = GetElement()->ParentNode(); ancestor; ancestor = ancestor->ParentNode())
    {
      auto tagName = HTMLTagNameOf(ancestor);
      if (tagName == TagName::tfoot || tagName == TagName::tbody || tagName == TagName::table)
      {
        return !ColumnIndexRange().first;
      }
      if (tagName == TagName::thead)
      {
        return false;
      }
    }
    return false;
  }

  Pair<uint32, uint32> AccessibilityNodeObject::RowIndexRange() const noexcept
  {
    EnsureIndexesUpToDate();
    return _rareData ? Pair<uint32, uint32> {_rareData->RowIndex(), _rareData->EffectiveRowSpan()}
                     : Pair<uint32, uint32> {0, 1};
  }

  Pair<uint32, uint32> AccessibilityNodeObject::ColumnIndexRange() const noexcept
  {
    EnsureIndexesUpToDate();
    return _rareData ? Pair<uint32, uint32> {_rareData->ColumnIndex(), ColSpan()}
                     : Pair<uint32, uint32> {0, 1};
  }

  void AccessibilityNodeObject::EnsureIndexesUpToDate() const noexcept
  {
    if (auto *parentTable = ParentTable())
    {
      parentTable->EnsureCellIndexesUpToDate();
    }
  }

  uint32 AccessibilityNodeObject::ColSpan() const noexcept
  {
    if (!IsTableCell())
    {
      return 1;
    }
    // "If colspan is greater than 1000, let it be 1000 instead"
    // (https://html.spec.whatwg.org/#attr-tdth-colspan).
    for (auto name : {u8"colspan", u8"aria-colspan"})
    {
      auto span = html::Numbers::ParseInteger(GetAttribute(name));
      if (span.Success() && span.Value >= 1)
      {
        return static_cast<uint32>(std::min<int64>(span.Value, 1'000));
      }
    }
    return 1;
  }

  uint32 AccessibilityNodeObject::RowSpan() const noexcept
  {
    if (!IsTableCell())
    {
      return 1;
    }
    // "If aria-rowspan is used on an element for which the host language provides an equivalent attribute,
    // user agents MUST ignore the value of aria-rowspan" (https://w3c.github.io/aria/#aria-rowspan).
    for (auto name : {u8"rowspan", u8"aria-rowspan"})
    {
      auto span = html::Numbers::ParseInteger(GetAttribute(name));
      if (span.Success())
      {
        return span.Value < 0 ? 1 : static_cast<uint32>(std::min<int64>(span.Value, MaxRowSpan));
      }
    }
    return 1;
  }

  LayoutRect AccessibilityNodeObject::ElementRect() const noexcept
  {
    // An area's rect is its shape's, in its image.
    if (auto *area = DynamicDowncast<html::HTMLAreaElement>(GetNode()))
    {
      for (auto *ancestor = area->ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (Is<html::HTMLMapElement>(*ancestor))
        {
          auto *renderer = ancestor->Renderer();
          return renderer ? area->ComputeRect(renderer) : LayoutRect();
        }
      }
      return {};
    }
    // A display: contents element covers its children.
    if (auto *element = GetElement(); element && element->HasDisplayContents())
    {
      LayoutRect contentsRect;
      for (const auto &child : const_cast<AccessibilityNodeObject *>(this)->Children())
      {
        contentsRect.Unite(child->ElementRect());
      }
      if (!contentsRect.IsEmpty())
      {
        return contentsRect;
      }
    }
    return NonEmptyAncestorBoundingBox();
  }

  LayoutRect AccessibilityNodeObject::NonEmptyAncestorBoundingBox() const noexcept
  {
    // An object with no box of its own gets one at its nearest rendered ancestor's position, as wide as it
    // and about a line of text high, so it's clearly inside that ancestor.
    for (auto *ancestor = ParentObject(); ancestor; ancestor = ancestor->ParentObject())
    {
      if (!ancestor->Renderer())
      {
        continue;
      }
      auto ancestorRect = ancestor->ElementRect();
      if (ancestorRect.IsEmpty())
      {
        continue;
      }
      return {ancestorRect.Location(),
              LayoutSize(ancestorRect.Width(), LayoutUnit(std::min(10.0f, ancestorRect.Height().ToFloat())))};
    }
    // A non-empty rect at the origin, failing that.
    return {0, 0, 1, 1};
  }
}
