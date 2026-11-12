/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::ax
{
  /// @brief An object with no node or renderer of its own (a slider's thumb, a menu list's pop-up): the
  /// object that makes it sets its parent and answers for it.
  class AccessibilityMockObject : public AccessibilityObject
  {
  protected:
    WeakPtr<AccessibilityObject> _parent;

  public:
    ~AccessibilityMockObject() noexcept override;

    KRYS_NODISCARD bool IsDetached() const noexcept override
    {
      return !AxObjectCache();
    }
    KRYS_NODISCARD bool IsMockObject() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD AccessibilityObject *ParentObject() const noexcept override
    {
      return _parent.get();
    }
    virtual void SetParent(AccessibilityObject *parent) noexcept
    {
      _parent = CreateWeakPtr(parent);
    }
    void DetachFromParent() noexcept override
    {
      _parent = nullptr;
    }
    KRYS_NODISCARD bool IsEnabled() const noexcept override
    {
      return true;
    }

  protected:
    AccessibilityMockObject(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override
    {
      return IsIgnoredByDefault();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityMockObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMockObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
