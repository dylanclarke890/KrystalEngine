/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascadeCache.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo::css
{
  class CSSSegmentedFontFace;
  class PrimitiveValue;

  /// @brief A document's font faces, looked up by family and by the `@font-face` rule they came from, with
  /// the faces of installed families that share a name with one.
  class CSSFontFaceSet final : public RefCounted<CSSFontFaceSet>, public CSSFontFaceClient
  {
    KRYS_NON_COPY_MOVABLE(CSSFontFaceSet);

  public:
    enum class Status : uint8
    {
      Loading,
      Loaded
    };

  private:
    struct FontFamilyNameHash
    {
      KRYS_NODISCARD size_t operator()(const FontFamilyName &name) const noexcept
      {
        return name.Hash();
      }
    };

    struct FontSelectionRequestHash
    {
      KRYS_NODISCARD size_t operator()(const FontSelectionRequest &request) const noexcept;
    };

    using FontSelectionHashMap =
      HashMap<FontSelectionRequest, RefPtr<CSSSegmentedFontFace>, FontSelectionRequestHash>;

    // _faces should hold all the same fonts as the ones inside inside _facesLookupTable.
    List<Ref<CSSFontFace>> _faces;
    HashMap<FontFamilyName, List<Ref<CSSFontFace>>, FontFamilyNameHash> _facesLookupTable;
    HashMap<FontFamilyName, List<Ref<CSSFontFace>>, FontFamilyNameHash> _locallyInstalledFacesLookupTable;
    HashMap<FontFamilyName, FontSelectionHashMap, FontFamilyNameHash> _cache;
    HashMap<FontFaceRule *, CSSFontFace *> _constituentCSSConnections;
    size_t _facesPartitionIndex {0}; // All entries in _faces before this index are CSS-connected.
    Status _status {Status::Loaded};
    WeakPtr<CSSFontSelector> _owningFontSelector;
    uint32 _activeCount {0};

    explicit CSSFontFaceSet(CSSFontSelector *owningFontSelector) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSFontFaceSet> Create(CSSFontSelector *owningFontSelector = nullptr) noexcept
    {
      return AdoptRef(*new CSSFontFaceSet(owningFontSelector));
    }

    ~CSSFontFaceSet() noexcept override;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }
    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    // Calling updateStyleIfNeeded() might delete |this|.
    void UpdateStyleIfNeeded() noexcept;

    KRYS_NODISCARD bool HasFace(const CSSFontFace &face) const noexcept;
    KRYS_NODISCARD size_t FaceCount() const noexcept
    {
      return _faces.size();
    }
    void Add(CSSFontFace &face) noexcept;
    void Remove(const CSSFontFace &face) noexcept;
    void Purge() noexcept;
    void EmptyCaches() noexcept;
    void Clear() noexcept;
    KRYS_NODISCARD CSSFontFace &operator[](size_t i) noexcept;

    KRYS_NODISCARD CSSFontFace *LookUpByCSSConnection(FontFaceRule &target) noexcept;

    /// @brief The faces of @p family for @p request, best first, or null if no `@font-face` rule names it.
    KRYS_NODISCARD CSSSegmentedFontFace *GetFontFace(FontSelectionRequest request,
                                                     const StringAtom &family) noexcept;

    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }
    KRYS_NODISCARD bool HasActiveFontFaces() const noexcept
    {
      return GetStatus() == Status::Loading;
    }

    KRYS_NODISCARD size_t FacesPartitionIndex() const noexcept
    {
      return _facesPartitionIndex;
    }

    // FIXME: Should this be implemented?
    void UpdateStyleIfNeeded(CSSFontFace &) noexcept final
    {
    }

  private:
    void RemoveFromFacesLookupTable(const CSSFontFace &face, const Value &familyToSearchFor) noexcept;
    void AddToFacesLookupTable(CSSFontFace &face) noexcept;

    void IncrementActiveCount() noexcept;
    void DecrementActiveCount() noexcept;

    void FontStateChanged(CSSFontFace &face, CSSFontFace::Status oldState,
                          CSSFontFace::Status newState) noexcept final;
    void FontPropertyChanged(CSSFontFace &face, Value *oldFamily = nullptr) noexcept final;

    void EnsureLocalFontFacesForFamilyRegistered(const StringAtom &familyName) noexcept;
    void NotifyFontModified() noexcept;

    KRYS_NODISCARD static StringAtom FamilyNameFromPrimitive(const PrimitiveValue &value) noexcept;
  };
}
