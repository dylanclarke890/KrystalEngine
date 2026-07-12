#include "Krystal.HTML/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Types/SmallNodeList.hpp"
#include "Krystal.HTML/HTML/HTMLDataListElement.hpp"
#include "Krystal.HTML/HTML/HTMLHRElement.hpp"
#include "Krystal.HTML/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.HTML/HTML/HTMLOptionElement.hpp"
#include "Krystal.HTML/HTML/HTMLSelectedContentElement.hpp"
#include "Krystal.HTML/HTML/HTMLSelectElement.hpp"
#include "Krystal.HTML/HTML/MicroParsers/Numbers.hpp"

namespace Krys::HTML
{
#pragma region HTMLSelectElement

  size_t FormControlAlgorithms::DisplaySize(const HTMLSelectElement &element) noexcept
  {
    // SPEC: The display size of a select element is the result of applying the rules for parsing non-negative
    // integers to the value of the element's size attribute, if it has one and parsing it is successful. If
    // applying those rules to the attribute's value is not successful, or if the size attribute is absent,
    // then the element's display size is 4 if the element's multiple content attribute is present, and 1
    // otherwise.

    auto size = element.GetAttribute(u8"size");
    if (size.has_value())
    {
      if (auto result = MicroParsers::Numbers::ParseNonNegativeInteger(*size); result.Success())
      {
        return result.Value;
      }
    }

    return element.HasAttribute(u8"multiple") ? 4uz : 1uz;
  }

  HTMLOptionElementList FormControlAlgorithms::ListOfOptions(const HTMLSelectElement &select) noexcept
  {
    // SPEC(1): Let options be « ».
    HTMLOptionElementList options;

    // SPEC(2): Let node be the first child of select in tree order.
    auto *node = select.FirstChild();

    // SPEC(3): While node is not null:
    while (node != nullptr)
    {
      // SPEC(3.1): If node is an option element, then append node to options.
      if (Is<HTMLOptionElement>(node))
      {
        options.push_back(ShareRef(Downcast<HTMLOptionElement>(*node)));
      }

      // SPEC(3.2): If any of the following conditions are true:
      //  - node is a select element;
      //  - node is an hr element;
      //  - node is an option element;
      //  - node is a datalist element;
      //  - node is an optgroup element and node has an ancestor optgroup in between itself and select
      // Then set node to the next descendant of select in tree order, excluding node's descendants, if any
      // such node exists; otherwise null.
      // Otherwise, set node to the next descendant of select in tree order, if any such node exists;
      // otherwise null.
      if (Is<HTMLSelectElement>(node) || Is<HTMLHRElement>(node) || Is<HTMLOptionElement>(node)
          || Is<HTMLDataListElement>(node))
      {
        node = TreeQueries::NextSkippingChildren(*node, &select);
        continue;
      }

      if (Is<HTMLOptGroupElement>(node))
      {
        for (auto &ancestor : AncestorRange(*node))
        {
          if (Is<HTMLOptGroupElement>(ancestor))
          {
            node = TreeQueries::NextSkippingChildren(*node, &select);
            continue;
          }

          if (Is<HTMLSelectElement>(ancestor))
          {
            node = TreeQueries::Next(*node, &select);
            continue;
          }
        }
      }

      node = TreeQueries::Next(*node, &select);
    }

    // SPEC(4):  Return options.
    return options;
  }

  /// @see https://html.spec.whatwg.org/#selectedness-setting-algorithm
  void FormControlAlgorithms::SelectednessSettingAlgorithm(HTMLSelectElement &element) noexcept
  {
    bool hasMultiple = element.HasAttribute(u8"multiple");
    if (hasMultiple)
    {
      return;
    }

    auto options = ListOfOptions(element);
    if (options.empty())
    {
      return;
    }

    auto selectedCount = std::ranges::count_if(options, [](auto &option) { return option->_selectedness; });

    // SPEC(1): If element's multiple attribute is absent, and element's display size is 1, and no option
    // elements in the element's list of options have their selectedness set to true, then set the
    // selectedness of the first option element in the list of options in tree order that is not disabled,
    // if any, to true, and return.
    if (DisplaySize(element) == 1uz && selectedCount == 0uz)
    {
      for (auto &option : options)
      {
        if (!option->Disabled())
        {
          option->_selectedness = true;
          return;
        }
      }
    }

    // SPEC(2): If element's multiple attribute is absent, and two or more option elements in element's list
    // of options have their selectedness set to true, then set the selectedness of all but the last option
    // element with its selectedness set to true in the list of options in tree order to false.
    if (selectedCount > 1uz)
    {
      for (auto &option : options)
      {
        if (option->_selectedness)
        {
          option->_selectedness = false;
          --selectedCount;

          if (selectedCount == 1uz)
          {
            break;
          }
        }
      }
    }
  }

#pragma endregion

#pragma region HTMLSelectedContentElement

  RawPtr<HTMLSelectedContentElement>
    FormControlAlgorithms::GetEnabledSelectedContent(HTMLSelectElement &select) noexcept
  {
    // SPEC(1): If select has the multiple attribute, then return null.
    if (select.HasAttribute(u8"multiple"))
    {
      return nullptr;
    }

    // SPEC(2): Let selectedcontent be the first selectedcontent element descendant of select in tree order if
    // any such element exists; otherwise return null.
    auto range = DescendantRange(select);
    auto selectedcontentIt =
      First(range, [](auto &&node)
            { return Is<HTMLElement>(node) && Is<HTMLSelectedContentElement>(Downcast<HTMLElement>(node)); });
    if (selectedcontentIt == range.end())
    {
      return nullptr;
    }
    auto &selectedcontent = Downcast<HTMLSelectedContentElement>(*selectedcontentIt);

    // SPEC(3): If selectedcontent's disabled is true, then return null.
    if (selectedcontent.Disabled())
    {
      return nullptr;
    }

    // SPEC(4): Return selectedcontent.
    return &selectedcontent;
  }

  void FormControlAlgorithms::CloneOptionIntoSelectedContent(
    HTMLOptionElement &option, HTMLSelectedContentElement &selectedcontent) noexcept
  {
    // SPEC(1): Let documentFragment be a new DocumentFragment whose node document is option's node document.
    auto documentFragment = CreateRef<DocumentFragment>(option.NodeDocument());

    // SPEC(2):For each child of option's children:
    for (auto &child : ChildNodeRange(option))
    {
      // SPEC(2.1): Let childClone be the result of running clone given child with subtree set to true.
      auto childClone = NodeAlgorithms::CloneNode(child, nullptr, true);

      // SPEC(2.2): Append childClone to documentFragment.
      documentFragment->AppendChild(*childClone.Value());
    }

    // SPEC(3): Replace all with documentFragment within selectedcontent.
    (void)MutationAlgorithms::ReplaceAll(documentFragment.get(), selectedcontent);
  }

#pragma endregion

#pragma region HTMLOptionElement

  void FormControlAlgorithms::UpdateNearestAncestorSelect(HTMLOptionElement &option) noexcept
  {
    // SPEC(1): Let oldSelect be option's cached nearest ancestor select element.
    auto *oldSelect = option._cachedNearestSelectElement;

    // SPEC(2): Let newSelect be option's option element nearest ancestor select.
    auto *newSelect = NearestAncestorSelect(option);

    // SPEC(3): If oldSelect is not newSelect:
    if (oldSelect != newSelect)
    {
      // SPEC(3.1): If oldSelect is not null, then run the selectedness setting algorithm given oldSelect.
      if (oldSelect != nullptr)
      {
        SelectednessSettingAlgorithm(*oldSelect);
      }

      // SPEC(3.2): If newSelect is not null, then run the selectedness setting algorithm given newSelect.
      if (newSelect != nullptr)
      {
        SelectednessSettingAlgorithm(*newSelect);
      }
    }

    // SPEC(4): Set option's cached nearest ancestor select element to newSelect.
    option._cachedNearestSelectElement = newSelect;
  }

  RawPtr<HTMLSelectElement> FormControlAlgorithms::NearestAncestorSelect(Element &element) noexcept
  {
    // SPEC(1): Let ancestorOptgroup be null.
    RawPtr<HTMLOptGroupElement> ancestorOptgroup = nullptr;

    SmallElementList ancestors;
    for (auto &ancestor : AncestorRange(element))
    {
      if (Is<Element>(ancestor))
      {
        ancestors.push_back(ShareRef(Downcast<Element>(ancestor)));
      }
    }

    // SPEC(2): For each ancestor of element's ancestors, in reverse tree order:
    for (auto it = ancestors.rbegin(); it != ancestors.rend(); ++it)
    {
      auto &ancestor = Downcast<HTMLElement>(**it);

      // SPEC(2.1): If ancestor is a datalist, hr, or option element, then return null.
      if (Is<HTMLDataListElement>(ancestor) || Is<HTMLHRElement>(ancestor) || Is<HTMLOptionElement>(ancestor))
      {
        return nullptr;
      }

      // SPEC(2.2): If ancestor is an optgroup element:
      if (Is<HTMLOptGroupElement>(ancestor))
      {
        // SPEC(2.2.1): If ancestorOptgroup is not null, then return null.
        if (ancestorOptgroup != nullptr)
        {
          return nullptr;
        }

        // SPEC(2.2.2): Set ancestorOptgroup to ancestor.
        ancestorOptgroup = &Downcast<HTMLOptGroupElement>(ancestor);
      }

      // SPEC(2.3): If ancestor is a select, then return ancestor.
      if (Is<HTMLSelectElement>(ancestor))
      {
        return &Downcast<HTMLSelectElement>(ancestor);
      }
    }

    // SPEC(3): Return null.
    return nullptr;
  }

  void FormControlAlgorithms::MaybeCloneOptionIntoSelectedContent(HTMLOptionElement &option) noexcept
  {
    // SPEC(1): Let select be option's option element nearest ancestor select.
    auto *select = NearestAncestorSelect(option);

    // SPEC(2): If all of the following conditions are true:
    // - select is not null;
    // - option's selectedness is true; and
    // - select's enabled selectedcontent is not null,
    // then run clone an option into a selectedcontent given option and select's enabled selectedcontent.
    if (select != nullptr && option._selectedness)
    {
      auto *selectedcontent = GetEnabledSelectedContent(*select);
      if (selectedcontent != nullptr)
      {
        CloneOptionIntoSelectedContent(option, *selectedcontent);
      }
    }
  }

#pragma endregion
}