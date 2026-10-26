#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;
}

namespace krys::boo
{
  /// @brief The renderer of `<input type=file>`. Each RenderFileUploadControl contains a RenderButton (for
  /// opening the file chooser), and sufficient space to draw a file icon and filename. The RenderButton has a
  /// shadow node associated with it to receive click/hover events.
  // FIXME: The file icon (the input's `icon()`), with the host's file icons.
  class RenderFileUploadControl final : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderFileUploadControl);

  private:
    bool _canReceiveDroppedFiles;

  public:
    RenderFileUploadControl(html::HTMLInputElement &, RenderStyle &&) noexcept;
    ~RenderFileUploadControl() noexcept override;

    KRYS_NODISCARD dom::DOMString ButtonValue() const noexcept;
    /// @brief The text drawn after the button: the files' names (or how many there are), or that none is
    /// chosen, truncated to fit.
    KRYS_NODISCARD utf8_string FileTextValue() const noexcept;

    KRYS_NODISCARD html::HTMLInputElement &InputElement() const noexcept;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderFileUploadControl";
    }

    void UpdateFromElement() noexcept override;
    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;
    void ComputePreferredLogicalWidths() noexcept override;
    void PaintObject(PaintInfo &, const LayoutPoint &) noexcept override;
    void PaintControl(PaintInfo &, const LayoutPoint &) noexcept;

    KRYS_NODISCARD int32 MaxFilenameLogicalWidth() const noexcept;

    KRYS_NODISCARD dom::PositionWithAffinity PositionForPoint(const LayoutPoint &,
                                                              HitTestSource) noexcept override;

    KRYS_NODISCARD html::HTMLInputElement *UploadButton() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderFileUploadControl)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderFileUploadControl();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
