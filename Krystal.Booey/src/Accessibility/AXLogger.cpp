/*
 * Copyright (C) 2020-2025 Apple Inc. All rights reserved.
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
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#include "Krystal.Booey/Accessibility/AXLogger.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include <format>

namespace krys::boo::ax
{
  namespace
  {
    KRYS_NODISCARD utf8_string NumberAsText(uint64 value) noexcept
    {
      auto text = std::format("{}", value);
      return utf8_string(text.begin(), text.end());
    }

    /// @brief @p text quoted, with quotes, backslashes and line breaks escaped.
    KRYS_NODISCARD utf8_string Quoted(utf8_stringview text) noexcept
    {
      utf8_string result = u8"\"";
      for (auto character : text)
      {
        switch (character)
        {
          case u8'"':  result += u8"\\\""; break;
          case u8'\\': result += u8"\\\\"; break;
          case u8'\n': result += u8"\\n"; break;
          default:     result += character; break;
        }
      }
      result += u8'"';
      return result;
    }

    void StreamSubtree(utf8_string &stream, AccessibilityObject &object, EnumSet<AXStreamOption> options,
                       uint32 depth) noexcept
    {
      stream.append(depth * 2, u8' ');
      stream += AXLogger::ObjectAsText(object, options);
      stream += u8'\n';
      for (const auto &child : object.Children())
      {
        StreamSubtree(stream, *child, options, depth + 1);
      }
    }
  }

  utf8_string AXLogger::ObjectAsText(AccessibilityObject &object, EnumSet<AXStreamOption> options) noexcept
  {
    utf8_string result;
    auto append = [&result](utf8_stringview text)
    {
      if (!result.empty())
      {
        result += u8' ';
      }
      result += text;
    };

    if (options.Contains(AXStreamOption::Role))
    {
      append(RoleToString(object.Role()));
    }
    if (options.Contains(AXStreamOption::ObjectID))
    {
      append(u8"objectID=" + NumberAsText(static_cast<uint64>(object.ObjectID())));
    }
    if (options.Contains(AXStreamOption::ParentID))
    {
      auto *parent = object.ParentObjectUnignored();
      append(u8"parentID=" + NumberAsText(parent ? static_cast<uint64>(parent->ObjectID()) : 0));
    }
    if (options.Contains(AXStreamOption::IdentifierAttribute))
    {
      if (auto *element = object.GetElement())
      {
        if (auto id = element->GetAttribute(u8"id"); id && !id->empty())
        {
          append(u8"identifier=" + Quoted(*id));
        }
      }
    }
    if (options.Contains(AXStreamOption::Name))
    {
      if (auto name = object.AccessibleName(); !name.empty())
      {
        append(u8"name=" + Quoted(name));
      }
      if (auto description = object.AccessibleDescription(); !description.empty())
      {
        append(u8"description=" + Quoted(description));
      }
    }
    if (options.Contains(AXStreamOption::Value))
    {
      if (object.IsStaticText() || object.IsTextControl())
      {
        append(u8"value=" + Quoted(object.StringValue()));
      }
    }
    if (options.Contains(AXStreamOption::RendererOrNode))
    {
      if (auto *renderer = object.Renderer())
      {
        append(u8"renderer=" + utf8_string(renderer->RenderName()));
      }
      else if (auto *node = object.GetNode())
      {
        append(u8"node=" + node->NodeName());
      }
    }
    if (options.Contains(AXStreamOption::DisplayContents))
    {
      if (auto *element = object.GetElement(); element && element->HasDisplayContents())
      {
        append(u8"hasDisplayContents");
      }
    }
    return result;
  }

  utf8_string AXLogger::TreeAsText(AccessibilityObject &root, EnumSet<AXStreamOption> options) noexcept
  {
    utf8_string stream;
    StreamSubtree(stream, root, options, 0);
    return stream;
  }

  utf8_string AXLogger::TreeAsText(AXObjectCache &cache, EnumSet<AXStreamOption> options) noexcept
  {
    auto *root = cache.RootObject();
    return root ? TreeAsText(*root, options) : utf8_string(u8"No root!\n");
  }
}
