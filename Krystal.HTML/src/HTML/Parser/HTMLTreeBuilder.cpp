#include "Krystal.HTML/HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/HTML/HTMLTableCellElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableRowElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableSectionElement.hpp"
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  HTMLTreeBuilder::HTMLTreeBuilder(Document &document) noexcept : _document(document)
  {
  }

  HTMLTreeBuilder::HTMLTreeBuilder(DocumentFragment &fragment, Element &contextElement) noexcept
      : _document(fragment.NodeDocument()),
        _fragmentParsingContext(FragmentParsingContext(fragment, contextElement))
  {
  }

  void HTMLTreeBuilder::ProcessToken(HTMLTokenAtom &&token) noexcept
  {
    // TODO
    // If the stack of open elements is empty
    // If the adjusted current node is an element in the HTML namespace
    // If the adjusted current node is a MathML text integration point and the token is a start tag whose tag
    //   name is neither "mglyph" nor "malignmark"
    // If the adjusted current node is a MathML text integration point and the token is a character token
    // If the adjusted current node is a MathML annotation-xml element and the token is a start tag whose tag
    //   name is "svg"
    // If the adjusted current node is an HTML integration point and the token is a start tag
    // If the adjusted current node is an HTML integration point and the token is a character token
    // If the token is an end-of-file token
    //     Process the token according to the rules given in the section corresponding to the current
    //     insertion mode in HTML content.
    // Otherwise
    //     Process the token according to the rules given in the section for parsing tokens in foreign
    //     content.
  }

  void HTMLTreeBuilder::ResetInsertionModeAppropriately() noexcept
  {
    bool last = false;

    auto *stackElement = &_openElementStack.Bottom();
    auto *node = &stackElement->Node();

    while (true)
    {
      if (node == &_openElementStack.Top().Node())
      {
        last = true;

        if (_fragmentParsingContext.has_value())
        {
          node = &_fragmentParsingContext->ContextElement();
        }
      }

      if (auto *htmlElement = DynamicDowncast<HTMLElement>(node))
      {
        switch (stackElement->Name())
        {
          case TagName::td:
          case TagName::th:
          {
            if (!last)
            {
              _insertionMode = InsertionMode::InCell;
              return;
            }

            break;
          }
          case TagName::tr:
          {
            _insertionMode = InsertionMode::InRow;
            return;
          }
          case TagName::tbody:
          case TagName::thead:
          case TagName::tfoot:
          {
            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          case TagName::caption:
          {
            _insertionMode = InsertionMode::InCaption;
            return;
          }
          case TagName::colgroup:
          {
            _insertionMode = InsertionMode::InColumnGroup;
            return;
          }
          case TagName::table:
          {
            _insertionMode = InsertionMode::InTable;
            return;
          }
          case TagName::template_:
          {
            _insertionMode = CurrentTemplateInsertionMode();
            return;
          }
          case TagName::head:
          {
            if (!last)
            {
              _insertionMode = InsertionMode::InHead;
              return;
            }

            break;
          }
          case TagName::body:
          {
            _insertionMode = InsertionMode::InBody;
            return;
          }
          case TagName::frameset:
          {
            _insertionMode = InsertionMode::InFrameset;
            return;
          }
          case TagName::html:
          {
            if (_head == nullptr)
            {
              _insertionMode = InsertionMode::BeforeHead;
            }
            else
            {
              _insertionMode = InsertionMode::AfterHead;
            }
            return;
          }
          default:
          {
            break;
          }
        }
      }

      if (last)
      {
        _insertionMode = InsertionMode::InBody;
        return;
      }

      stackElement = _openElementStack.EntryBefore(*node);
      assert(stackElement != nullptr);

      node = &stackElement->Node();
    }
  }

  InsertionMode HTMLTreeBuilder::CurrentTemplateInsertionMode() const noexcept
  {
    assert(!_templateInsertionModes.empty());
    return _templateInsertionModes.back();
  }

  ContainerNode &HTMLTreeBuilder::CurrentNode() noexcept
  {
    return _openElementStack.Bottom().Node();
  }

  ContainerNode &HTMLTreeBuilder::AdjustedCurrentNode() noexcept
  {
    if (_fragmentParsingContext.has_value() && _openElementStack.Size() == 1)
    {
      return _fragmentParsingContext->ContextElement();
    }
    else
    {
      return CurrentNode();
    }
  }

  bool HTMLTreeBuilder::HasElementInScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->Name() == targetNode)
      {
        return true;
      }

      switch (node->Name())
      {
        case TagName::applet:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLTreeBuilder::HasElementInListItemScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->Name() == targetNode)
      {
        return true;
      }

      switch (node->Name())
      {
        case TagName::applet:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::li:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        case TagName::ul:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLTreeBuilder::HasElementInButtonScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->Name() == targetNode)
      {
        return true;
      }

      switch (node->Name())
      {
        case TagName::applet:
        case TagName::button:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLTreeBuilder::HasElementInTableScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->Name() == targetNode)
      {
        return true;
      }

      switch (node->Name())
      {
        case TagName::html:
        case TagName::table:
        case TagName::template_:
        {
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  AdjustedInsertionLocation
    HTMLTreeBuilder::AppropriateInsertionLocation(RawPtr<ContainerNode> targetOverride) noexcept
  {
    auto &target = targetOverride ? *targetOverride : CurrentNode();

    AdjustedInsertionLocation location {};

    auto *targetHTMLElement = DynamicDowncast<HTMLElement>(target);
    if (_fosterParenting
        && IsOneOf<HTMLTableElement, HTMLTableSectionElement, HTMLTableRowElement>(targetHTMLElement))
    {
      auto [table, template_, elementBeforeTable, isTemplateMostRecent] =
        _openElementStack.LastTableAndTemplate();

      if (template_ != nullptr && (table == nullptr || isTemplateMostRecent))
      {
        location = {.Parent = template_->Content().get()};
      }
      else if (table == nullptr)
      {
        location = {.Parent = &_openElementStack.Top().Node()}; // should be the html element
      }
      else if (table->ParentNode() != nullptr)
      {
        location = {.Parent = table->ParentNode(), .BeforeSibling = table};
      }
      else
      {
        location = {.Parent = elementBeforeTable};
      }
    }
    else
    {
      location.Parent = &target;
      location.BeforeSibling = nullptr;
    }

    if (auto *template_ = DynamicDowncast<HTMLTemplateElement>(location.Parent))
    {
      location.Parent = template_->Content().get();
      location.BeforeSibling = nullptr;
    }

    return location;
  }
}
