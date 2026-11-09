/*
 * Copyright (C) 2016-2019 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/coords.html#InterfaceSVGPreserveAspectRatio
  class SVGPreserveAspectRatio : public SVGValueProperty<SVGPreserveAspectRatioValue>
  {
    using Base = SVGValueProperty<SVGPreserveAspectRatioValue>;
    using Base::_value;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGPreserveAspectRatio>
      Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
             const SVGPreserveAspectRatioValue &value = {}) noexcept
    {
      return AdoptRef(*new SVGPreserveAspectRatio(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGPreserveAspectRatio>>
      Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGPreserveAspectRatio(value.ReleaseValue()));
    }

    KRYS_NODISCARD uint16 Align() const noexcept
    {
      return _value.Align();
    }

    dom::ExceptionOr<void> SetAlign(uint16 value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.SetAlign(value);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }

    KRYS_NODISCARD uint16 MeetOrSlice() const noexcept
    {
      return _value.MeetOrSlice();
    }

    dom::ExceptionOr<void> SetMeetOrSlice(uint16 value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.SetMeetOrSlice(value);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return _value.ValueAsString();
    }
  };
}
