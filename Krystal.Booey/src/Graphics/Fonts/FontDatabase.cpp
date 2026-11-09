#include "Krystal.Booey/Graphics/Fonts/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/BundledFonts.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDescription.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include <algorithm>
#include <fstream>

namespace krys::boo
{
  KRYS_NODISCARD static utf8_string FoldCase(utf8_stringview family) noexcept
  {
    utf8_string folded {family};
    std::ranges::transform(folded, folded.begin(), [](char8 c) { return text::ToASCIILower(c); });
    return folded;
  }

  FontSelectionCapabilities CapabilitiesOfFace(const text::FreeTypeFace &face) noexcept
  {
    const auto &style = face.Style();
    FontSelectionCapabilities capabilities;
    capabilities.Weight = FontSelectionRange {FontSelectionValue {style.Weight}};
    capabilities.Width = FontSelectionRange {FontSelectionValue {style.Width}};
    capabilities.Slope = FontSelectionRange {style.Italic ? ItalicValue() : NormalItalicValue()};

    // A variable font covers what its axes reach.
    for (const auto &axis : face.VariationAxes())
    {
      FontSelectionValue minimum {axis.Minimum};
      FontSelectionValue maximum {axis.Maximum};
      if (axis.Tag == FontAxisTag('w', 'g', 'h', 't'))
      {
        capabilities.Weight = FontSelectionRange {minimum, maximum};
      }
      else if (axis.Tag == FontAxisTag('w', 'd', 't', 'h'))
      {
        capabilities.Width = FontSelectionRange {minimum, maximum};
      }
      else if (axis.Tag == FontAxisTag('s', 'l', 'n', 't'))
      {
        // A positive slant axis value leans the other way from a positive oblique angle.
        capabilities.Slope =
          FontSelectionRange {FontSelectionValue {-axis.Maximum}, FontSelectionValue {-axis.Minimum}};
      }
      else if (axis.Tag == FontAxisTag('i', 't', 'a', 'l'))
      {
        capabilities.Slope = FontSelectionRange {NormalItalicValue(), ItalicValue()};
      }
    }
    return capabilities;
  }

  Ref<text::FreeTypeFace> InstanceOfFace(text::FreeTypeFace &face, const FontDescription &description,
                                         const FontSelectionSpecifiedCapabilities &descriptors) noexcept
  {
    if (face.VariationAxes().empty())
    {
      return ShareRef(face);
    }

    auto withinDescriptor = [](float value, const Maybe<FontSelectionRange> &range)
    {
      return range ? std::clamp(value, static_cast<float>(range->Minimum), static_cast<float>(range->Maximum))
                   : value;
    };

    List<text::FontVariationValue> values;
    values.push_back({FontAxisTag('w', 'g', 'h', 't'),
                      withinDescriptor(static_cast<float>(description.Weight()), descriptors.Weight)});
    values.push_back({FontAxisTag('w', 'd', 't', 'h'),
                      withinDescriptor(static_cast<float>(description.Width()), descriptors.Width)});
    if (auto slope = description.FontStyleSlope())
    {
      if (description.GetFontStyleAxis() == FontStyleAxis::ital)
      {
        values.push_back({FontAxisTag('i', 't', 'a', 'l'), 1});
      }
      else
      {
        values.push_back({FontAxisTag('s', 'l', 'n', 't'),
                          -withinDescriptor(static_cast<float>(*slope), descriptors.Slope)});
      }
    }
    if (description.OpticalSizing() == FontOpticalSizing::Auto)
    {
      values.push_back({FontAxisTag('o', 'p', 's', 'z'), description.ComputedSize()});
    }
    for (const auto &setting : description.VariationSettings())
    {
      auto tag = setting.Tag();
      values.push_back({FontAxisTag(tag[0], tag[1], tag[2], tag[3]), setting.Value()});
    }
    return face.WithVariations(values);
  }

  KRYS_NODISCARD static Ref<text::FreeTypeFace> CreateBundledFace(Span<const byte> data) noexcept
  {
    auto face = text::FreeTypeFace::Create(text::FontFileData::CreateStatic(data), 0);
    krys_assert_msg(face, "the bundled font can be read");
    Ref<text::FreeTypeFace> result = krys::move(face);
    return result;
  }

  FontDatabase::FontDatabase() noexcept
      : _bundledFace(CreateBundledFace(BundledLiberationSansRegular())),
        _bundledArabicFace(CreateBundledFace(BundledNotoSansArabicRegular())),
        _bundledMathFace(CreateBundledFace(BundledSTIXTwoMathRegular()))
  {
    AddFace(Ref {_bundledFace});
    AddFace(Ref {_bundledArabicFace});
    AddFace(Ref {_bundledMathFace});
  }

  FontDatabase &FontDatabase::Singleton() noexcept
  {
    static MainThreadNeverDestroyed<FontDatabase> database;
    return database;
  }

  uint32 FontDatabase::AddFontFile(const std::filesystem::path &path)
  {
    std::error_code error;
    auto canonical = std::filesystem::weakly_canonical(path, error);
    if (error)
    {
      canonical = path;
    }
    if (std::ranges::find(_registeredFiles, canonical) != _registeredFiles.end())
    {
      return 0;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
      return 0;
    }
    List<byte> bytes;
    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    if (size <= 0)
    {
      return 0;
    }
    bytes.resize(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char *>(bytes.data()), size))
    {
      return 0;
    }

    auto count = AddFontFileData(text::FontFileData::Create(krys::move(bytes)));
    if (count)
    {
      _registeredFiles.push_back(krys::move(canonical));
    }
    return count;
  }

  uint32 FontDatabase::AddFontDirectory(const std::filesystem::path &path)
  {
    std::error_code error;
    std::filesystem::directory_iterator it {path, error};
    if (error)
    {
      return 0;
    }

    // Register the files in name order, so that registration doesn't depend on the file system's order.
    List<std::filesystem::path> files;
    for (auto &entry : it)
    {
      if (!entry.is_regular_file(error))
      {
        continue;
      }
      auto extension = FoldCase(entry.path().extension().u8string());
      if (extension == u8".ttf" || extension == u8".otf" || extension == u8".ttc" || extension == u8".otc")
      {
        files.push_back(entry.path());
      }
    }
    std::ranges::sort(files);

    uint32 count = 0;
    for (auto &file : files)
    {
      count += AddFontFile(file);
    }
    return count;
  }

  uint32 FontDatabase::AddFontData(List<byte> &&bytes) noexcept
  {
    return AddFontFileData(text::FontFileData::Create(krys::move(bytes)));
  }

  uint32 FontDatabase::AddFontFileData(Ref<text::FontFileData> &&data) noexcept
  {
    auto faceCount = text::FreeTypeFace::FaceCount(*data);
    uint32 count = 0;
    for (uint32 faceIndex = 0; faceIndex < faceCount; ++faceIndex)
    {
      if (auto face = text::FreeTypeFace::Create(data, faceIndex))
      {
        Ref<text::FreeTypeFace> created = krys::move(face);
        AddFace(krys::move(created));
        ++count;
      }
    }
    if (count)
    {
      ++_generation;
      // Families that weren't found may be found now.
      FontCache::InvalidateAllFontCaches();
    }
    return count;
  }

  void FontDatabase::AddFace(Ref<text::FreeTypeFace> &&face) noexcept
  {
    if (!face->FamilyName().empty())
    {
      _families[FoldCase(face->FamilyName())].push_back(face);
    }
    _faces.push_back(krys::move(face));
  }

  Span<const Ref<text::FreeTypeFace>> FontDatabase::FacesInFamily(utf8_stringview family) const noexcept
  {
    auto it = _families.find(FoldCase(family));
    if (it == _families.end())
    {
      return {};
    }
    return it->second;
  }

  text::FreeTypeFace *FontDatabase::MatchFace(utf8_stringview family,
                                              const FontSelectionRequest &request) const noexcept
  {
    auto faces = FacesInFamily(family);
    if (faces.empty())
    {
      return nullptr;
    }

    List<FontSelectionCapabilities> capabilities;
    capabilities.reserve(faces.size());
    for (auto &face : faces)
    {
      capabilities.push_back(CapabilitiesOfFace(*face));
    }
    FontSelectionAlgorithm algorithm {request, capabilities};
    return faces[algorithm.IndexOfBestCapabilities()].get();
  }

  List<utf8_string> FontDatabase::Families() const noexcept
  {
    List<utf8_string> families;
    for (auto &[folded, faces] : _families)
    {
      families.push_back(faces.front()->FamilyName());
    }
    std::ranges::sort(families);
    return families;
  }
}
