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
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadingElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLElement.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
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
          // A description is the text of the elements aria-describedby refers to (step 2B for descriptions).
          // FIXME: aria-description, and the host language's description (title when the name didn't use it),
          // with descriptions.
          auto *element = DynamicDowncast<dom::Element>(_root);
          _visited.insert(&_root);
          return element ? StringAlgorithms::StripAndCollapseASCIIWhitespace(
                             ComputeFromReferences(*element, u8"aria-describedby"))
                         : dom::DOMString {};
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
        // FIXME: The chosen option of an ARIA listbox (aria-selected), with list boxes.
        if (object.IsListBox())
        {
          return {};
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

      /// @brief The text alternative HTML-AAM gives @p element from its markup.
      /// @see https://w3c.github.io/html-aam/#accessible-name-computations-by-html-element
      KRYS_NODISCARD dom::DOMString HostLanguageLabel(const dom::Element &element,
                                                      const AccessibilityObject &object, bool hidden,
                                                      const Context &context) noexcept
      {
        auto tagName = HTMLTagNameOf(&element);
        if (tagName == TagName::Unknown)
        {
          // FIXME: SVG's title element and MathML's alttext, with their objects.
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
        // 2F.ii. Name From Generated Content: ...
        // FIXME: ::before and ::after content, with pseudo-element objects.

        // 2F.iii. Determine Child Nodes: ...
        // FIXME: aria-owns children, with relations.
        // 2F.iv. Name From Each Child: For each child node of the current node:
        for (auto &child : dom::ComposedTreeChildren(const_cast<dom::Element &>(element)))
        {
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
        return accumulated;
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
    // FIXME: Whether a table is exposed (computeIsTableExposableThroughAccessibility), with tables.
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

    // FIXME: An area's parent is its image map's image (not the map), with image maps; and an aria-owned
    // object's parent is its owner, with relations.
    return cache->GetOrCreate(*parent);
  }

  AccessibilityRole AccessibilityNodeObject::DetermineAccessibilityRole() noexcept
  {
    if (_ariaRole != AccessibilityRole::Unknown)
    {
      return _ariaRole;
    }
    // FIXME: Exposed tables, rows and cells and image map links, with them.
    return DetermineAccessibilityRoleFromNode();
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
        return AccessibilityRole::TextGroup;
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
    AccessibilityObject::ClearChildren();
    _childrenDirty = false;
  }

  void AccessibilityNodeObject::AddChildren() noexcept
  {
    // If the need to add more children in addition to existing children arises, childrenChanged should have
    // been called, leaving the object with no children.
    krys_debug_assert(!_childrenInitialized);
    _childrenInitialized = true;

    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (node && cache)
    {
      if (auto *container = DynamicDowncast<dom::ContainerNode>(*node))
      {
        for (auto &child : dom::ComposedTreeChildren(*container))
        {
          AddChild(cache->GetOrCreate(child));
        }
      }
      // FIXME: aria-owned children (updateOwnedChildrenIfNecessary), with relations.
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

    // A native text control's text is its value: what's in its user agent shadow tree isn't in the tree.
    if (IsNativeTextControl())
    {
      return false;
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

  bool AccessibilityNodeObject::ComputeIsIgnored() const noexcept
  {
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
    // FIXME: A web area is focused when its frame's selection is focused and active (the document isn't
    // focusable), with the selection.
    return focusedElement == node;
  }

  bool AccessibilityNodeObject::IsEnabled() const noexcept
  {
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
    // FIXME: Selected tabs (isTabItemSelected) and the selection of ARIA grids, with them.
    return EqualsIgnoringCase(GetAttribute(u8"aria-selected"), u8"true");
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
    // FIXME: Grids and tree grids (multi-selectable unless aria-multiselectable is false) and tables (never),
    // with tables.
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

  bool AccessibilityNodeObject::SupportsExpanded() const noexcept
  {
    if (HTMLTagNameOf(GetNode()) == TagName::details || Role() == AccessibilityRole::Summary)
    {
      return true;
    }
    // FIXME: Pop-up buttons and the popover invokers (popovertarget, commandfor), with them.
    return HasAttribute(u8"aria-expanded");
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
    auto *node = GetNode();
    auto *cache = AxObjectCache();
    if (!node || !cache)
    {
      return {};
    }
    // FIXME: The web area's name (the document's title), with the web area's specialised parts.
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
    return TextAlternativeComputation(*cache, TextAlternativeComputation::Kind::Description, *node).Compute();
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
          return option->Label();
        }
      }
      return {};
    }
    // FIXME: Non-native text controls' text (contenteditable and role=textbox), with text ranges; and the
    // values of ranges, colour wells and dates, with the specialised objects.
    return {};
  }
}
