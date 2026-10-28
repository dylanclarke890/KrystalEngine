#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/CSS/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "Krystal.Booey/Graphics/SourceImage.hpp"

namespace krys::boo::html
{
  Ref<CanvasPattern> CanvasPattern::Create(SourceImage &&image, bool repeatX, bool repeatY) noexcept
  {
    return AdoptRef(*new CanvasPattern(krys::move(image), repeatX, repeatY));
  }

  CanvasPattern::CanvasPattern(SourceImage &&image, bool repeatX, bool repeatY) noexcept
      : _pattern(Pattern::Create(krys::move(image), {repeatX, repeatY}))
  {
  }

  CanvasPattern::~CanvasPattern() noexcept = default;

  bool CanvasPattern::ParseRepetitionType(dom::DOMStringView type, bool &repeatX, bool &repeatY) noexcept
  {
    if (type.empty() || type == u8"repeat")
    {
      repeatX = true;
      repeatY = true;
      return true;
    }
    if (type == u8"no-repeat")
    {
      repeatX = false;
      repeatY = false;
      return true;
    }
    if (type == u8"repeat-x")
    {
      repeatX = true;
      repeatY = false;
      return true;
    }
    if (type == u8"repeat-y")
    {
      repeatX = false;
      repeatY = true;
      return true;
    }
    return false;
  }

  dom::ExceptionOr<void> CanvasPattern::SetTransform(css::DOMMatrix2DInit &&matrixInit) noexcept
  {
    auto checkValid = css::DOMMatrixReadOnly::ValidateAndFixup(matrixInit);
    if (checkValid.HasException())
    {
      return checkValid.ReleaseException();
    }

    _pattern->SetPatternSpaceTransform(
      {*matrixInit.M11, *matrixInit.M12, *matrixInit.M21, *matrixInit.M22, *matrixInit.M41, *matrixInit.M42});
    return {};
  }
}
