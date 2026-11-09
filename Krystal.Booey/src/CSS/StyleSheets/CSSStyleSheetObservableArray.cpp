/*
 * Copyright (C) 2022, 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheetObservableArray.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  CSSStyleSheetObservableArray::CSSStyleSheetObservableArray(dom::ContainerNode &treeScope) noexcept
      : _treeScope(CreateWeakPtr(&treeScope))
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
