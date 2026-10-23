#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectedContentElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"

namespace krys::boo::html
{
#pragma region HTMLSelectedContentElement

  HTMLSelectedContentElement *
    FormControlAlgorithms::GetEnabledSelectedContent(HTMLSelectElement &select) noexcept
  {
    // SPEC(1): If select has the multiple attribute, then return null.
    if (select.HasAttribute(u8"multiple"))
    {
      return nullptr;
    }

    // SPEC(2): Let selectedcontent be the first selectedcontent element descendant of select in tree order if
    // any such element exists; otherwise return null.
    auto range = dom::DescendantRange(select);
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
    auto documentFragment = CreateRef<dom::DocumentFragment>(option.NodeDocument());

    // SPEC(2):For each child of option's children:
    for (auto &child : dom::ChildNodeRange(option))
    {
      // SPEC(2.1): Let childClone be the result of running clone given child with subtree set to true.
      auto childClone = dom::NodeAlgorithms::CloneNode(child, nullptr, true);

      // SPEC(2.2): Append childClone to documentFragment.
      documentFragment->AppendChild(*childClone.Value());
    }

    // SPEC(3): Replace all with documentFragment within selectedcontent.
    (void)dom::MutationAlgorithms::ReplaceAll(documentFragment.get(), selectedcontent);
  }

#pragma endregion

#pragma region HTMLOptionElement

  void FormControlAlgorithms::MaybeCloneOptionIntoSelectedContent(HTMLOptionElement &option) noexcept
  {
    // SPEC(1): Let select be option's option element nearest ancestor select.
    auto *select = option.OwnerSelectElement();

    // SPEC(2): If all of the following conditions are true:
    // - select is not null;
    // - option's selectedness is true; and
    // - select's enabled selectedcontent is not null,
    // then run clone an option into a selectedcontent given option and select's enabled selectedcontent.
    if (select != nullptr && option.Selected())
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