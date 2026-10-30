#include "Krystal.Booey/CSS/Font/FontFaceSet.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/FontFaceSource.hpp"
#include "Krystal.Booey/CSS/Font/SegmentedFontFace.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Core/Hash.hpp"
#include <algorithm>

namespace krys::boo::css
{
  using namespace WebKitFontFamilyNames;

  size_t
    CSSFontFaceSet::FontSelectionRequestHash::operator()(const FontSelectionRequest &request) const noexcept
  {
    return Hash::Combine(request.Weight.RawValue(), request.Width.RawValue(), request.Slope.has_value(),
                         request.Slope ? request.Slope->RawValue() : 0);
  }

  CSSFontFaceSet::CSSFontFaceSet(CSSFontSelector *owningFontSelector) noexcept
      : _owningFontSelector(WeakPtr<CSSFontSelector>::WithRef(owningFontSelector))
  {
  }

  CSSFontFaceSet::~CSSFontFaceSet() noexcept
  {
    // The faces of installed families never have the set as a client.
    for (auto &face : _faces)
    {
      face->RemoveClient(*this);
    }
  }

  void CSSFontFaceSet::NotifyFontModified() noexcept
  {
    if (auto *fontSelector = _owningFontSelector.get())
    {
      fontSelector->FontModified();
    }
  }

  void CSSFontFaceSet::IncrementActiveCount() noexcept
  {
    ++_activeCount;
    if (_activeCount == 1)
    {
      _status = Status::Loading;
    }
  }

  void CSSFontFaceSet::DecrementActiveCount() noexcept
  {
    --_activeCount;
    if (!_activeCount)
    {
      _status = Status::Loaded;
    }
  }

  bool CSSFontFaceSet::HasFace(const CSSFontFace &face) const noexcept
  {
    return std::ranges::any_of(_faces, [&](const Ref<CSSFontFace> &myFace) { return &*myFace == &face; });
  }

  // Calling updateStyleIfNeeded() might delete |this|.
  void CSSFontFaceSet::UpdateStyleIfNeeded() noexcept
  {
    if (auto *fontSelector = _owningFontSelector.get())
    {
      Ref<CSSFontSelector> protectedFontSelector = ShareRef(*fontSelector);
      protectedFontSelector->UpdateStyleIfNeeded();
    }
  }

  void CSSFontFaceSet::EnsureLocalFontFacesForFamilyRegistered(const StringAtom &familyName) noexcept
  {
    krys_debug_assert(_owningFontSelector);
    if (_locallyInstalledFacesLookupTable.contains(FontFamilyName(familyName)))
    {
      return;
    }

    auto *owningFontSelector = _owningFontSelector.get();
    if (!owningFontSelector || !owningFontSelector->GetDocument())
    {
      return;
    }

    // FIXME: A setting that disallows user installed fonts, once there's one.
    auto capabilities =
      FontCache::Singleton().GetFontSelectionCapabilitiesInFamily(familyName, AllowUserInstalledFonts::Yes);
    if (capabilities.empty())
    {
      return;
    }

    List<Ref<CSSFontFace>> faces;
    for (auto item : capabilities)
    {
      auto face = CSSFontFace::Create(*owningFontSelector, nullptr, true);
      Ref<Value> family = ValuePool::MainThreadPool().CreateFontFamilyValue(familyName);
      face->SetFamily(*family);
      face->SetFontSelectionCapabilities(item);
      face->AdoptSource(UniquePtr<CSSFontFaceSource>(new CSSFontFaceSource(*face, familyName)));
      krys_debug_assert(!face->ComputeFailureState());
      faces.push_back(krys::move(face));
    }
    _locallyInstalledFacesLookupTable.emplace(FontFamilyName(familyName), krys::move(faces));
  }

  StringAtom CSSFontFaceSet::FamilyNameFromPrimitive(const PrimitiveValue &value) noexcept
  {
    if (value.IsFontFamily())
    {
      return StringAtom {value.StringValue()};
    }

    // We need to use the raw text for all the generic family types, since @font-face is a way of actually
    // defining what font to use for those types.
    switch (value.ValueId())
    {
      case ValueId::Serif:            return SerifFamily();
      case ValueId::SansSerif:        return SansSerifFamily();
      case ValueId::Cursive:          return CursiveFamily();
      case ValueId::Fantasy:          return FantasyFamily();
      case ValueId::Monospace:        return MonospaceFamily();
      case ValueId::WebkitPictograph: return PictographFamily();
      case ValueId::SystemUi:         return SystemUiFamily();
      case ValueId::Math:             return MathFamily();
      default:                        return StringAtom::Null();
    }
  }

  void CSSFontFaceSet::AddToFacesLookupTable(CSSFontFace &face) noexcept
  {
    auto family = face.FamilyCSSValue();
    if (!family)
    {
      // If the font has failed, there's no point in actually adding it to _facesLookupTable,
      // because no font requests can actually use it for anything. So, let's just ... not add it.
      return;
    }

    auto familyName = FamilyNameFromPrimitive(Downcast<PrimitiveValue>(*family));
    if (familyName == StringAtom::Null())
    {
      return;
    }

    auto [iterator, isNewEntry] = _facesLookupTable.try_emplace(FontFamilyName(familyName));
    if (isNewEntry)
    {
      // _locallyInstalledFacesLookupTable grows without bound, eventually incorporating every font installed
      // on the system. This is by design.
      if (_owningFontSelector)
      {
        EnsureLocalFontFacesForFamilyRegistered(familyName);
      }
    }

    iterator->second.push_back(ShareRef(face));
  }

  void CSSFontFaceSet::Add(CSSFontFace &face) noexcept
  {
    krys_debug_assert(!HasFace(face));

    NotifyFontModified();

    face.AddClient(*this);
    _cache.clear();

    if (face.CssConnection())
    {
      _faces.insert(_faces.begin() + static_cast<ptrdiff_t>(_facesPartitionIndex++), ShareRef(face));
    }
    else
    {
      _faces.push_back(ShareRef(face));
    }

    AddToFacesLookupTable(face);

    if (face.GetStatus() == CSSFontFace::Status::Loading || face.GetStatus() == CSSFontFace::Status::TimedOut)
    {
      IncrementActiveCount();
    }

    if (face.CssConnection())
    {
      krys_debug_assert(!_constituentCSSConnections.contains(face.CssConnection()));
      _constituentCSSConnections.emplace(face.CssConnection(), &face);
    }
  }

  void CSSFontFaceSet::RemoveFromFacesLookupTable(const CSSFontFace &face,
                                                  const Value &familyToSearchFor) noexcept
  {
    auto familyName = FamilyNameFromPrimitive(Downcast<PrimitiveValue>(familyToSearchFor));
    if (familyName == StringAtom::Null())
    {
      return;
    }

    auto iterator = _facesLookupTable.find(FontFamilyName(familyName));
    if (iterator == _facesLookupTable.end())
    {
      // The font may have failed even before addToFacesLookupTable() was called on it,
      // which means we never added it (because there's no point in adding a failed font).
      // So, if it was never added, removing it is free! Woohoo!
      return;
    }

    auto &familyFaces = iterator->second;
    auto found = std::ranges::find_if(familyFaces, [&](const Ref<CSSFontFace> &candidate)
                                      { return &*candidate == &face; });
    krys_debug_assert(found != familyFaces.end());
    if (found != familyFaces.end())
    {
      familyFaces.erase(found);
    }

    if (familyFaces.empty())
    {
      _facesLookupTable.erase(iterator);
    }
  }

  void CSSFontFaceSet::Remove(const CSSFontFace &face) noexcept
  {
    Ref<CSSFontFace> protect = ShareRef(const_cast<CSSFontFace &>(face));

    _cache.clear();

    NotifyFontModified();

    if (auto family = face.FamilyCSSValue())
    {
      RemoveFromFacesLookupTable(face, *family);
    }

    if (face.CssConnection())
    {
      krys_debug_assert(_constituentCSSConnections.contains(face.CssConnection())
                        && _constituentCSSConnections.at(face.CssConnection()) == &face);
      _constituentCSSConnections.erase(face.CssConnection());
    }

    for (size_t i = 0; i < _faces.size(); ++i)
    {
      if (&*_faces[i] == &face)
      {
        if (i < _facesPartitionIndex)
        {
          --_facesPartitionIndex;
        }
        _faces[i]->RemoveClient(*this);
        _faces.erase(_faces.begin() + static_cast<ptrdiff_t>(i));
        if (face.GetStatus() == CSSFontFace::Status::Loading
            || face.GetStatus() == CSSFontFace::Status::TimedOut)
        {
          DecrementActiveCount();
        }
        return;
      }
    }
    krys_debug_assert(false);
  }

  CSSFontFace *CSSFontFaceSet::LookUpByCSSConnection(FontFaceRule &target) noexcept
  {
    auto found = _constituentCSSConnections.find(&target);
    return found != _constituentCSSConnections.end() ? found->second : nullptr;
  }

  void CSSFontFaceSet::Purge() noexcept
  {
    List<Ref<CSSFontFace>> toRemove;
    for (auto &face : _faces)
    {
      if (face->Purgeable())
      {
        toRemove.push_back(face);
      }
    }

    for (auto &item : toRemove)
    {
      Remove(*item);
    }
  }

  void CSSFontFaceSet::EmptyCaches() noexcept
  {
    _cache.clear();
  }

  void CSSFontFaceSet::Clear() noexcept
  {
    for (auto &face : _faces)
    {
      face->RemoveClient(*this);
    }

    _faces.clear();
    _facesLookupTable.clear();
    _locallyInstalledFacesLookupTable.clear();
    _cache.clear();
    _constituentCSSConnections.clear();
    _facesPartitionIndex = 0;
    _status = Status::Loaded;
  }

  CSSFontFace &CSSFontFaceSet::operator[](size_t i) noexcept
  {
    krys_debug_assert(i < FaceCount());
    return *_faces[i];
  }

  CSSSegmentedFontFace *CSSFontFaceSet::GetFontFace(FontSelectionRequest request,
                                                    const StringAtom &family) noexcept
  {
    auto iterator = _facesLookupTable.find(FontFamilyName(family));
    if (iterator == _facesLookupTable.end())
    {
      return nullptr;
    }
    auto &familyFontFaces = iterator->second;

    auto &segmentedFontFaceCache = _cache[FontFamilyName(family)];

    auto &face = segmentedFontFaceCache[request];
    if (face)
    {
      return face.get();
    }

    face = CSSSegmentedFontFace::Create();

    List<CSSFontFace *> candidateFontFaces;
    candidateFontFaces.reserve(32);
    for (auto i = static_cast<ptrdiff_t>(familyFontFaces.size()) - 1; i >= 0; --i)
    {
      CSSFontFace &candidate = *familyFontFaces[static_cast<size_t>(i)];
      if (candidate.GetStatus() == CSSFontFace::Status::Failure)
      {
        continue;
      }
      if (!IsItalic(request.Slope) && IsItalic(candidate.GetFontSelectionCapabilities().Slope.Minimum))
      {
        continue;
      }
      candidateFontFaces.push_back(&candidate);
    }

    auto localIterator = _locallyInstalledFacesLookupTable.find(FontFamilyName(family));
    if (localIterator != _locallyInstalledFacesLookupTable.end())
    {
      for (auto &candidate : localIterator->second)
      {
        if (candidate->GetStatus() == CSSFontFace::Status::Failure)
        {
          continue;
        }
        if (!IsItalic(request.Slope) && IsItalic(candidate->GetFontSelectionCapabilities().Slope.Minimum))
        {
          continue;
        }
        candidateFontFaces.push_back(&*candidate);
      }
    }

    if (!candidateFontFaces.empty())
    {
      List<FontSelectionCapabilities> capabilities;
      capabilities.reserve(candidateFontFaces.size());
      for (auto *candidate : candidateFontFaces)
      {
        capabilities.push_back(candidate->GetFontSelectionCapabilities());
      }
      FontSelectionAlgorithm fontSelectionAlgorithm(request, capabilities);
      std::ranges::stable_sort(
        candidateFontFaces,
        [&fontSelectionAlgorithm](CSSFontFace *first, CSSFontFace *second)
        {
          auto firstCapabilities = first->GetFontSelectionCapabilities();
          auto secondCapabilities = second->GetFontSelectionCapabilities();

          auto widthDistanceFirst = fontSelectionAlgorithm.WidthDistance(firstCapabilities).Distance;
          auto widthDistanceSecond = fontSelectionAlgorithm.WidthDistance(secondCapabilities).Distance;
          if (widthDistanceFirst < widthDistanceSecond)
          {
            return true;
          }
          if (widthDistanceFirst > widthDistanceSecond)
          {
            return false;
          }

          auto styleDistanceFirst = fontSelectionAlgorithm.StyleDistance(firstCapabilities).Distance;
          auto styleDistanceSecond = fontSelectionAlgorithm.StyleDistance(secondCapabilities).Distance;
          if (styleDistanceFirst < styleDistanceSecond)
          {
            return true;
          }
          if (styleDistanceFirst > styleDistanceSecond)
          {
            return false;
          }

          auto weightDistanceFirst = fontSelectionAlgorithm.WeightDistance(firstCapabilities).Distance;
          auto weightDistanceSecond = fontSelectionAlgorithm.WeightDistance(secondCapabilities).Distance;
          return weightDistanceFirst < weightDistanceSecond;
        });
      CSSFontFace *previousCandidate = nullptr;
      for (auto *candidate : candidateFontFaces)
      {
        if (candidate == previousCandidate)
        {
          continue;
        }
        previousCandidate = candidate;
        face->AppendFontFace(ShareRef(*candidate));
      }
    }

    return face.get();
  }

  void CSSFontFaceSet::FontStateChanged(CSSFontFace &face, CSSFontFace::Status oldState,
                                        CSSFontFace::Status newState) noexcept
  {
    krys_debug_assert(HasFace(face));
    (void)face;
    if (oldState == CSSFontFace::Status::Pending)
    {
      krys_debug_assert(newState == CSSFontFace::Status::Loading);
      IncrementActiveCount();
    }
    if (newState == CSSFontFace::Status::Success || newState == CSSFontFace::Status::Failure)
    {
      krys_debug_assert(oldState == CSSFontFace::Status::Loading
                        || oldState == CSSFontFace::Status::TimedOut);
      DecrementActiveCount();
    }
  }

  void CSSFontFaceSet::FontPropertyChanged(CSSFontFace &face, Value *oldFamily) noexcept
  {
    _cache.clear();

    if (oldFamily)
    {
      RemoveFromFacesLookupTable(face, *oldFamily);
      AddToFacesLookupTable(face);
    }

    NotifyFontModified();
  }
}
