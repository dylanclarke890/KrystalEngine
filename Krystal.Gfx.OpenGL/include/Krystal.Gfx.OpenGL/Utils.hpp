#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Shader.hpp"
#include "Krystal.Gfx/Light.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"

namespace Krys::Gfx::OpenGL
{
  class Utils
  {
  public:
    static GLenum MapVertexAttributeType(VertexAttributeType type) noexcept
    {
      switch (type)
      {
        case VertexAttributeType::Int32:  return GL_INT;
        case VertexAttributeType::UInt32: return GL_UNSIGNED_INT;
        case VertexAttributeType::Float:  return GL_FLOAT;
        case VertexAttributeType::Double: return GL_DOUBLE;
        default:                          return 0;
      }
    }

    static uint32 ApplyVertexBufferLayout(const VertexBufferLayout &layout,
                                          size_t attributeIndexOffset = 0u) noexcept
    {
      uint32 stride = 0;
      for (const auto &element : layout)
      {
        stride += element.Count * VertexBufferElement::GetSizeOfType(element.Type);
      }

      uint32 offset = 0;
      for (uint32 i = 0; i < layout.size(); i++)
      {
        const auto &element = layout[i];
        if (element.Enabled)
        {
          uint32 attributeIndex = i + static_cast<uint32>(attributeIndexOffset);
          glEnableVertexAttribArray(attributeIndex);
          switch (element.Type)
          {
            case VertexAttributeType::Int32:
            case VertexAttributeType::UInt32:
              glVertexAttribIPointer(attributeIndex, element.Count, MapVertexAttributeType(element.Type),
                                     stride, (const void *)(uintptr_t)offset);
              break;
            case VertexAttributeType::Double:
              glVertexAttribLPointer(attributeIndex, element.Count, MapVertexAttributeType(element.Type),
                                     stride, (const void *)(uintptr_t)offset);
              break;
            case VertexAttributeType::Float:
              glVertexAttribPointer(attributeIndex, element.Count, MapVertexAttributeType(element.Type),
                                    element.Normalized ? GL_TRUE : GL_FALSE, stride,
                                    (const void *)(uintptr_t)offset);
            default: break;
          }

          if (element.InputRate == VertexInputRate::PerInstance)
            glVertexAttribDivisor(attributeIndex, 1);
          else
            glVertexAttribDivisor(attributeIndex, 0);
        }

        offset += element.Count * VertexBufferElement::GetSizeOfType(element.Type);
      }

      return offset;
    }

    static void SetDirectionalLightUniforms(Shader &shader, DirectionalLight &light,
                                            const string &uniformPrefix = "directionalLight")
    {
      shader.SetUniform(uniformPrefix + ".direction", light.Direction);
      shader.SetUniform(uniformPrefix + ".diffuse", light.Colour.Diffuse.ToVec3());
      shader.SetUniform(uniformPrefix + ".ambient", light.Colour.Ambient.ToVec3());
      shader.SetUniform(uniformPrefix + ".specular", light.Colour.Specular.ToVec3());
    }

    static void SetPointLightUniforms(Shader &shader, const PointLight &light,
                                      const string &uniformPrefix = "pointLight")
    {
      shader.SetUniform(uniformPrefix + ".position", light.Position);
      shader.SetUniform(uniformPrefix + ".ambient", light.Colour.Ambient.ToVec3());
      shader.SetUniform(uniformPrefix + ".diffuse", light.Colour.Diffuse.ToVec3());
      shader.SetUniform(uniformPrefix + ".specular", light.Colour.Specular.ToVec3());
      shader.SetUniform(uniformPrefix + ".constant", light.Attenuation.Constant);
      shader.SetUniform(uniformPrefix + ".linear", light.Attenuation.Linear);
      shader.SetUniform(uniformPrefix + ".quadratic", light.Attenuation.Quadratic);
    }

    static void SetSpotLightUniforms(Shader &shader, const SpotLight &light,
                                     const string &uniformPrefix = "spotLight")
    {
      shader.SetUniform(uniformPrefix + ".position", light.Position);
      shader.SetUniform(uniformPrefix + ".direction", light.Direction);
      shader.SetUniform(uniformPrefix + ".ambient", light.Colour.Ambient.ToVec3());
      shader.SetUniform(uniformPrefix + ".diffuse", light.Colour.Diffuse.ToVec3());
      shader.SetUniform(uniformPrefix + ".specular", light.Colour.Specular.ToVec3());
      shader.SetUniform(uniformPrefix + ".constant", light.Attenuation.Constant);
      shader.SetUniform(uniformPrefix + ".linear", light.Attenuation.Linear);
      shader.SetUniform(uniformPrefix + ".quadratic", light.Attenuation.Quadratic);
      shader.SetUniform(uniformPrefix + ".cutOff", std::cos(light.CutOffRadians));
      shader.SetUniform(uniformPrefix + ".outerCutOff", std::cos(light.OuterCutOffRadians));
    }
  };
}