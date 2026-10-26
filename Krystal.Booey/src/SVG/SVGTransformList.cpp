#include "Krystal.Booey/SVG/SVGTransformList.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/SVGTransformable.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  dom::ExceptionOr<RefPtr<SVGTransform>> SVGTransformList::Consolidate() noexcept
  {
    auto result = CanAlterList();
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    krys_assert(result.ReleaseValue());

    // Spec: If the list was empty, then a value of null is returned.
    if (_items.empty())
    {
      return RefPtr<SVGTransform> {};
    }

    if (_items.size() == 1)
    {
      return RefPtr<SVGTransform> {At(0)};
    }

    auto newItem = SVGTransform::Create(Concatenate());
    ClearItems();

    auto item = Append(krys::move(newItem));
    CommitChange();
    return RefPtr<SVGTransform> {item};
  }

  AffineTransform SVGTransformList::Concatenate() const noexcept
  {
    AffineTransform result;
    for (auto &transform : _items)
    {
      result *= transform->Matrix().Value();
    }
    return result;
  }

  bool SVGTransformList::ParseGeneric(dom::DOMStringView &buffer, ListReplacement listReplacement) noexcept
  {
    bool delimParsed = false;
    SkipOptionalSVGSpaces(buffer);

    size_t itemIndex = 0;
    auto currentListReplacement = listReplacement;

    while (!buffer.empty())
    {
      delimParsed = false;

      auto parsedTransformType = SVGTransformable::ParseTransformType(buffer);
      if (!parsedTransformType)
      {
        return false;
      }

      if (currentListReplacement == ListReplacement::Replace && itemIndex < _items.size()
          && *parsedTransformType == _items[itemIndex]->Type())
      {
        if (!SVGTransformable::ParseAndReplaceTransform(*parsedTransformType, buffer, *_items[itemIndex]))
        {
          return false;
        }
      }
      else
      {
        // Switch to `Append` mode and remove the existing SVGTransforms starting from `itemIndex`.
        if (currentListReplacement == ListReplacement::Replace)
        {
          currentListReplacement = ListReplacement::Append;
          Resize(itemIndex);
        }

        RefPtr parsedTransform = SVGTransformable::ParseTransform(*parsedTransformType, buffer);
        if (!parsedTransform)
        {
          return false;
        }

        Append(Ref<SVGTransform> {krys::move(parsedTransform)});
      }

      SkipOptionalSVGSpaces(buffer);

      if (!buffer.empty() && buffer.front() == u8',')
      {
        buffer.remove_prefix(1);
        delimParsed = true;
      }

      SkipOptionalSVGSpaces(buffer);
      ++itemIndex;
    }

    if (itemIndex < _items.size())
    {
      krys_assert(currentListReplacement == ListReplacement::Replace);
      Resize(itemIndex);
    }

    return !delimParsed;
  }

  void SVGTransformList::Parse(dom::DOMStringView value) noexcept
  {
    auto buffer = value;
    bool parsingSucceeded = ParseGeneric(buffer, ListReplacement::Replace);

    if (!parsingSucceeded)
    {
      ClearItems();
    }
  }

  dom::DOMString SVGTransformList::ValueAsString() const noexcept
  {
    StringBuilder builder;
    for (const auto &transform : _items)
    {
      if (builder.Length())
      {
        builder.Append(u8' ');
      }

      builder.Append(transform->Value().ValueAsString());
    }
    return krys::move(builder).ToString();
  }
}
