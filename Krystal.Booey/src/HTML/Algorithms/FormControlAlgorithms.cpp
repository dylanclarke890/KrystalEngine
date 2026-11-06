#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectedContentElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::html
{
#pragma region HTMLSelectedContentElement

  void FormControlAlgorithms::UpdateDescendantSelectedContents(HTMLSelectElement &select) noexcept
  {
    // Each of the algorithms that update these also clones the selected option into the select button.
    select.CloneSelectedOptionIntoSelectButton();

    // 1. If select has the multiple attribute, then return.
    if (select.HasAttribute(u8"multiple"))
    {
      return;
    }

    // 2. Let descendantSelectedcontents be a list of select's descendants, in tree order, that are
    //    selectedcontent elements whose disabled is false.
    List<Ref<HTMLSelectedContentElement>> descendantSelectedcontents;
    for (auto &descendant : dom::DescendantRange(select))
    {
      if (auto *selectedcontent = DynamicDowncast<HTMLSelectedContentElement>(descendant);
          selectedcontent && !selectedcontent->Disabled())
      {
        descendantSelectedcontents.push_back(ShareRef(*selectedcontent));
      }
    }

    // 3. For each selectedcontent of descendantSelectedcontents: update a selectedcontent given select and
    //    selectedcontent.
    for (auto &selectedcontent : descendantSelectedcontents)
    {
      UpdateSelectedContent(select, *selectedcontent);
    }
  }

  void FormControlAlgorithms::UpdateSelectedContent(HTMLSelectElement &select,
                                                    HTMLSelectedContentElement &selectedcontent) noexcept
  {
    // 1. If selectedcontent's disabled is true, then return.
    if (selectedcontent.Disabled())
    {
      return;
    }

    // 2. Let option be the first option in select's list of options whose selectedness is true, if any such
    //    option exists; otherwise null.
    int32 selectedIndex = select.SelectedIndex();
    RefPtr<HTMLOptionElement> option =
      ShareRefPtr(selectedIndex >= 0 ? select.Item(static_cast<uint32>(selectedIndex)) : nullptr);

    // 3. If option is null, then clear a selectedcontent given selectedcontent.
    if (!option)
    {
      ClearSelectedContent(selectedcontent);
      return;
    }

    // 4. Otherwise, clone an option into a selectedcontent given option and selectedcontent.
    CloneOptionIntoSelectedContent(*option, selectedcontent);
  }

  void FormControlAlgorithms::CloneOptionIntoSelectedContent(
    HTMLOptionElement &option, HTMLSelectedContentElement &selectedcontent) noexcept
  {
    // 1. Let documentFragment be the result of creating a document fragment given option's node document.
    auto documentFragment = CreateRef<dom::DocumentFragment>(option.NodeDocument());

    // 2. For each child of option's children:
    for (auto &child : dom::ChildNodeRange(option))
    {
      // 2a. Let childClone be the result of running clone given child with subtree set to true.
      auto childClone = dom::NodeAlgorithms::CloneNode(child, nullptr, true);

      // 2b. Append childClone to documentFragment.
      (void)documentFragment->AppendChild(*childClone.Value());
    }

    // 3. Replace all with documentFragment within selectedcontent.
    (void)dom::MutationAlgorithms::ReplaceAll(documentFragment.get(), selectedcontent);
  }

  void FormControlAlgorithms::ClearSelectedContent(HTMLSelectedContentElement &selectedcontent) noexcept
  {
    // 1. Replace all with null within selectedcontent.
    (void)dom::MutationAlgorithms::ReplaceAll(nullptr, selectedcontent);
  }

  void FormControlAlgorithms::QueueUpdateDescendantSelectedContents(HTMLSelectElement &select) noexcept
  {
    EventLoop::Current().QueueMicrotask([select = ShareRef(select)]
                                        { UpdateDescendantSelectedContents(*select); });
  }

#pragma endregion

#pragma region HTMLOptionElement

  void FormControlAlgorithms::UpdateDescendantSelectedContentsForOption(HTMLOptionElement &option) noexcept
  {
    // 1. Let select be option's cached nearest ancestor select element.
    RefPtr<HTMLSelectElement> select = ShareRefPtr(option.OwnerSelectElement());

    // 2. If select is null or option's selectedness is false, then return.
    if (!select || !option.Selected())
    {
      return;
    }

    // 3. Update a select's descendant selectedcontent elements given select.
    UpdateDescendantSelectedContents(*select);
  }

#pragma endregion
}
