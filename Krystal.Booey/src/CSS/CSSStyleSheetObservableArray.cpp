#include "Krystal.Booey/CSS/CSSStyleSheetObservableArray.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  CSSStyleSheetObservableArray::CSSStyleSheetObservableArray(dom::ContainerNode &treeScope) noexcept
      : _treeScope(WeakPtr<dom::ContainerNode>::WithRef(treeScope))
  {
    krys_debug_assert(Is<dom::Document>(treeScope) || Is<dom::ShadowRoot>(treeScope));
  }

  dom::ExceptionOr<void> CSSStyleSheetObservableArray::SetValueAt(uint32 index, CSSStyleSheet &sheet) noexcept
  {
    krys_assert(index <= _sheets.size());

    if (auto exception = ShouldThrowWhenAddingSheet(sheet))
    {
      return krys::move(*exception);
    }

    if (index == _sheets.size())
    {
      _sheets.push_back(ShareRef(sheet));
    }
    else
    {
      auto oldSheet = krys::move(_sheets[index]);
      _sheets[index] = ShareRef(sheet);
      WillRemoveSheet(*oldSheet);
    }

    DidAddSheet(sheet);
    return {};
  }

  void CSSStyleSheetObservableArray::RemoveLast() noexcept
  {
    krys_assert(!_sheets.empty());
    auto sheet = krys::move(_sheets.back());
    _sheets.pop_back();
    WillRemoveSheet(*sheet);
  }

  void CSSStyleSheetObservableArray::ShrinkTo(uint32 length) noexcept
  {
    krys_assert(length <= _sheets.size());
    while (_sheets.size() > length)
    {
      RemoveLast();
    }
  }

  CSSStyleSheet *CSSStyleSheetObservableArray::ValueAt(uint32 index) const noexcept
  {
    if (index >= _sheets.size())
    {
      return nullptr;
    }
    return _sheets[index].get();
  }

  dom::ExceptionOr<void> CSSStyleSheetObservableArray::SetSheets(List<Ref<CSSStyleSheet>> &&sheets) noexcept
  {
    for (auto &sheet : sheets)
    {
      if (auto exception = ShouldThrowWhenAddingSheet(*sheet))
      {
        return krys::move(*exception);
      }
    }

    auto oldSheets = krys::move(_sheets);
    _sheets = krys::move(sheets);
    for (auto &sheet : oldSheets)
    {
      WillRemoveSheet(*sheet);
    }
    for (auto &sheet : _sheets)
    {
      DidAddSheet(*sheet);
    }

    return {};
  }

  Maybe<dom::Exception>
    CSSStyleSheetObservableArray::ShouldThrowWhenAddingSheet(const CSSStyleSheet &sheet) const noexcept
  {
    if (!sheet.WasConstructedByJS())
    {
      return dom::Exception(dom::ExceptionCode::NotAllowedError,
                            u8"Sheet needs to be constructed by JavaScript");
    }
    if (!_treeScope || sheet.ConstructorDocument() != &_treeScope->NodeDocument())
    {
      return dom::Exception(dom::ExceptionCode::NotAllowedError,
                            u8"Sheet constructor document doesn't match");
    }
    return null;
  }

  void CSSStyleSheetObservableArray::DidAddSheet(CSSStyleSheet &sheet) noexcept
  {
    if (_treeScope)
    {
      sheet.AddAdoptingTreeScope(*_treeScope);
    }
  }

  void CSSStyleSheetObservableArray::WillRemoveSheet(CSSStyleSheet &sheet) noexcept
  {
    if (!_treeScope)
    {
      return;
    }
    // A sheet can be adopted more than once; the tree scope still adopts it while a copy remains.
    if (std::ranges::any_of(_sheets, [&](const Ref<CSSStyleSheet> &other) { return other.get() == &sheet; }))
    {
      return;
    }
    sheet.RemoveAdoptingTreeScope(*_treeScope);
  }
}
