/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Transforms/Quaternion.hpp"
#include <cmath>

namespace krys::boo
{
  // Perform a spherical linear interpolation between the two
  // passed quaternions with 0 <= t <= 1.
  Quaternion Quaternion::Slerp(const Quaternion &other, double t) const noexcept
  {
    constexpr double epsilon = 1e-5;
    Quaternion copy = *this;

    double cosHalfAngle = copy.X * other.X + copy.Y * other.Y + copy.Z * other.Z + copy.W * other.W;

    if (cosHalfAngle < 0.0)
    {
      copy.X = -copy.X;
      copy.Y = -copy.Y;
      copy.Z = -copy.Z;
      copy.W = -copy.W;
      cosHalfAngle = -cosHalfAngle;
    }

    if (cosHalfAngle > 1)
    {
      cosHalfAngle = 1;
    }

    double sinHalfAngle = std::sqrt(1.0 - cosHalfAngle * cosHalfAngle);
    if (sinHalfAngle < epsilon)
    {
      // Quaternions share common axis and angle.
      return *this;
    }

    double halfAngle = std::acos(cosHalfAngle);
    double scale = std::sin((1 - t) * halfAngle) / sinHalfAngle;
    double invscale = std::sin(t * halfAngle) / sinHalfAngle;

    return {copy.X * scale + other.X * invscale, copy.Y * scale + other.Y * invscale,
            copy.Z * scale + other.Z * invscale, copy.W * scale + other.W * invscale};
  }

  // Compute quaternion multiplication
  Quaternion Quaternion::Accumulate(const Quaternion &other) const noexcept
  {
    return {W * other.X + X * other.W + Y * other.Z - Z * other.Y,
            W * other.Y - X * other.Z + Y * other.W + Z * other.X,
            W * other.Z + X * other.Y - Y * other.X + Z * other.W,
            W * other.W - X * other.X - Y * other.Y - Z * other.Z};
  }

  Quaternion Quaternion::Interpolate(const Quaternion &other, double progress,
                                     CompositeOperation compositeOperation) const noexcept
  {
    if (compositeOperation == CompositeOperation::Accumulate)
    {
      return Accumulate(other);
    }
    return Slerp(other, progress);
  }
}
