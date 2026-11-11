/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 *
 * Portions are Copyright (C) 2001-6 mozilla.org
 *
 * Other contributors:
 *   Stuart Parmenter <stuart@mozilla.com>
 *
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Booey/Graphics/Images/ImageOrientation.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  /// @brief What's read from an image's EXIF metadata: its orientation, and the size it prefers to be shown
  /// at.
  struct EXIFMetadata
  {
    ImageOrientation Orientation {ImageOrientation::Orientation::None};
    /// @brief The image's preferred density-corrected dimensions: its EXIF pixel dimensions, when its
    /// resolution (in inches) scales its size to them.
    Maybe<IntSize> DensityCorrectedSize;

    /// @brief Reads the metadata of an image of @p physicalSize from @p tiff, the TIFF block EXIF stores
    /// (from its byte order mark, `II` or `MM`); what can't be read is left out.
    KRYS_NODISCARD static EXIFMetadata Read(Span<const uint8> tiff, const IntSize &physicalSize) noexcept;
  };
}
