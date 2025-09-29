#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Log/ILogger.hpp"

namespace Krys::Gfx::OpenGL
{
  struct UniformInfo
  {
    string Name {};        // Uniform name, e.g. "myMatrix" or "lights[0].position"
    GLint Location {};     // -1 if in a block
    GLenum Type {};        // e.g. GL_FLOAT_VEC3, GL_INT, etc.
    GLsizei ArraySize {};  // 1 if not an array
    GLint BlockIndex {};   // >=0 if this uniform is inside a UBO
    GLint Offset {};       // byte offset in UBO (if blockIndex>=0)
    GLint MatrixStride {}; // for mat arrays
    GLint ArrayStride {};  // for arrays
  };

  struct BlockInfo
  {
    string Name {};                      // block name
    GLuint Index {};                     // program block index
    GLuint Binding {};                   // Binding point
    GLsizei DataSize {};                 // total block size in bytes
    Map<string, UniformInfo> Members {}; // block uniforms
  };

  struct SamplerInfo
  {
    string Name {};       // sampler name, e.g. "myTex" or "myTex[0]"
    GLint Location {};    // uniform location
    GLenum Type {};       // GL_SAMPLER_2D, GL_SAMPLER_CUBE, etc.
    GLsizei ArraySize {}; // e.g. sampler2D myTex[4]
    GLint FirstUnit {};   // texture unit you assign (contiguous for arrays)
  };

  struct ProgramInputInfo
  {
    string Name {};
    GLenum Type {};
    GLsizei ArraySize {1};
    GLint Location {-1};
    GLint LocationComponent {0};
  };

  struct ProgramOutputInfo
  {
    string Name {};
    GLenum Type {};
    GLsizei ArraySize {1};
    GLint Location {-1};         // fragment color number or generic output location
    GLint LocationComponent {0}; //
    GLint Index {0};             // dual-source blending (FS only)
  };

  struct ShaderLayout
  {
    Map<string, UniformInfo> Uniforms;
    Map<string, BlockInfo> Blocks;
    Map<string, SamplerInfo> Samplers;
    Map<string, ProgramInputInfo> Inputs;
    Map<string, ProgramOutputInfo> Outputs;
  };

  struct ShaderReflector
  {
  private:
    mutable ShaderLayout _info;

  public:
    NO_DISCARD ShaderLayout Reflect(GLuint programId) const noexcept
    {
      _info = ShaderLayout {};
      ReflectUniforms(programId);
      ReflectInputs(programId);
      ReflectOutputs(programId);

      return _info;
    }

    void LogReflectionInfo(const ShaderLayout &info) const noexcept
    {
      Log::ILogger *logger = Log::GetGlobalLogger();
      if (!logger)
        return;

      logger->Info("=== Shader Reflection Info ===");
      logger->Info("Uniforms ({}):", info.Uniforms.size());
      for (const auto &[name, uniform] : info.Uniforms)
      {
        logger->Info("  Name: {}, Type: {}, ArraySize: {}, Location: {}", name, StringifyGLType(uniform.Type),
                     uniform.ArraySize, uniform.Location);
      }

      logger->Info("Uniform Blocks ({}):", info.Blocks.size());
      for (const auto &[blockName, block] : info.Blocks)
      {
        logger->Info("  Block Name: {}, Index: {}, Binding: {}, DataSize: {}", blockName, block.Index,
                     block.Binding, block.DataSize);
        for (const auto &[memberName, member] : block.Members)
        {
          logger->Info(
            "    Member Name: {}, Type: {}, ArraySize: {}, Offset: {}, ArrayStride: {}, MatrixStride: {}",
            memberName, StringifyGLType(member.Type), member.ArraySize, member.Offset, member.ArrayStride,
            member.MatrixStride);
        }
      }

      logger->Info("Samplers ({}):", info.Samplers.size());
      for (const auto &[samplerName, sampler] : info.Samplers)
      {
        logger->Info("  Name: {}, Type: {}, ArraySize: {}, Location: {}", samplerName,
                     StringifyGLType(sampler.Type), sampler.ArraySize, sampler.Location);
      }

      logger->Info("Program Inputs ({}):", info.Inputs.size());
      for (const auto &[inputName, input] : info.Inputs)
      {
        logger->Info("  Name: {}, Type: {}, ArraySize: {}, Location: {}, LocationComponent: {}", inputName,
                     StringifyGLType(input.Type), input.ArraySize, input.Location, input.LocationComponent);
      }

      logger->Info("Program Outputs ({}):", info.Outputs.size());
      for (const auto &[outputName, output] : info.Outputs)
      {
        logger->Info("  Name: {}, Type: {}, ArraySize: {}, Location: {}, LocationComponent: {}", outputName,
                     StringifyGLType(output.Type), output.ArraySize, output.Location,
                     output.LocationComponent);
      }
    }

  private:
    void ReflectUniforms(GLuint programId) const noexcept
    {
      // Uniform blocks
      GLint blockCount = 0;
      glGetProgramInterfaceiv(programId, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &blockCount);

      Array<GLenum, 4> blockProps = {GL_NAME_LENGTH, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE,
                                     GL_NUM_ACTIVE_VARIABLES};

      for (GLint i = 0; i < blockCount; i++)
      {
        GLint values[4] {};
        glGetProgramResourceiv(programId, GL_UNIFORM_BLOCK, i, 4, blockProps.data(), 4, nullptr, values);

        // Block name name
        List<char> nameData(values[0]);
        glGetProgramResourceName(programId, GL_UNIFORM_BLOCK, i, (GLsizei)nameData.size(), nullptr,
                                 nameData.data());
        string blockName(nameData.data(), nameData.size() - 1);

        BlockInfo block {};
        block.Name = blockName;
        block.Index = i;
        block.Binding = values[1];
        block.DataSize = values[2];

        List<GLint> members(values[3]);
        GLenum blockMembers = GL_ACTIVE_VARIABLES;
        glGetProgramResourceiv(programId, GL_UNIFORM_BLOCK, i, 1, &blockMembers, (GLsizei)members.size(),
                               nullptr, members.data());

        // for each member, fetch uniform info
        Array<GLenum, 7> memberProps = {GL_NAME_LENGTH,  GL_TYPE,          GL_ARRAY_SIZE, GL_OFFSET,
                                        GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_BLOCK_INDEX};
        for (GLint memberIx : members)
        {
          GLint memberValues[7] {};
          glGetProgramResourceiv(programId, GL_UNIFORM, memberIx, (GLsizei)memberProps.size(),
                                 memberProps.data(), (GLsizei)memberProps.size(), nullptr, memberValues);

          List<char> memberNameData(memberValues[0]);
          glGetProgramResourceName(programId, GL_UNIFORM, memberIx, (GLsizei)memberNameData.size(), nullptr,
                                   memberNameData.data());
          string memberName(memberNameData.data(), memberNameData.size() - 1);

          UniformInfo ui {};
          ui.Name = memberName;
          ui.Type = (GLenum)memberValues[1];
          ui.ArraySize = memberValues[2];
          ui.Offset = memberValues[3];
          ui.ArrayStride = memberValues[4];
          ui.MatrixStride = memberValues[5];
          ui.BlockIndex = memberValues[6];
          ui.Location = -1; // members never have loose locations

          block.Members[ui.Name] = ui;
        }

        _info.Blocks[block.Name] = block;
      }

      // Non-block uniforms
      GLint uniformCount = 0;
      glGetProgramInterfaceiv(programId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);

      Array<GLenum, 5> uniformProps = {GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE, GL_LOCATION, GL_BLOCK_INDEX};

      for (GLint i = 0; i < uniformCount; ++i)
      {
        GLint values[5] {};
        glGetProgramResourceiv(programId, GL_UNIFORM, i, (GLsizei)uniformProps.size(), uniformProps.data(),
                               (GLsizei)uniformProps.size(), nullptr, values);

        // skip block members (already handled above)
        if (values[4] != -1)
          continue;

        List<char> nameData(values[0]);
        glGetProgramResourceName(programId, GL_UNIFORM, i, (GLsizei)nameData.size(), nullptr,
                                 nameData.data());
        string name(nameData.data(), nameData.size() - 1);

        UniformInfo ui {};
        ui.Name = name;
        ui.Type = (GLenum)values[1];
        ui.ArraySize = values[2];
        ui.Location = values[3];
        ui.BlockIndex = -1;
        ui.Offset = -1;

        // if sampler/image then add to samplers, else to uniforms
        if (IsSamplerType(ui.Type))
        {
          SamplerInfo si {};
          si.Name = name;
          si.Location = ui.Location;
          si.Type = ui.Type;
          si.ArraySize = ui.ArraySize;
          si.FirstUnit = -1; // assign later

          _info.Samplers[si.Name] = si;
        }
        else
        {
          _info.Uniforms[ui.Name] = ui;
        }
      }
    }

    void ReflectInputs(GLuint programId) const noexcept
    {
      GLint count = 0;
      glGetProgramInterfaceiv(programId, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &count);

      // properties we want per input
      Array<GLenum, 5> props = {GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE, GL_LOCATION, GL_LOCATION_COMPONENT};

      for (GLint i = 0; i < count; i++)
      {
        GLint values[props.size()] {};
        glGetProgramResourceiv(programId, GL_PROGRAM_INPUT, i, (GLsizei)props.size(), props.data(),
                               (GLsizei)props.size(), nullptr, values);

        // name
        List<char> nameData(values[0]);
        glGetProgramResourceName(programId, GL_PROGRAM_INPUT, i, (GLsizei)nameData.size(), nullptr,
                                 nameData.data());
        string name(nameData.data(), nameData.size() - 1);

        ProgramInputInfo pi {};
        pi.Name = name;
        pi.Type = (GLenum)values[1];
        pi.ArraySize = (GLsizei)values[2];
        pi.Location = values[3];
        pi.LocationComponent = values[4];

        _info.Inputs[pi.Name] = pi;
      }
    }

    void ReflectOutputs(GLuint programId) const noexcept
    {
      GLint outputCount = 0;
      glGetProgramInterfaceiv(programId, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &outputCount);
      Array<GLenum, 1> props = {GL_NAME_LENGTH};
      for (GLint i = 0; i < outputCount; i++)
      {
        GLint values[1] {};
        glGetProgramResourceiv(programId, GL_PROGRAM_OUTPUT, i, (GLsizei)props.size(), props.data(),
                               (GLsizei)props.size(), nullptr, values);
        List<char> nameData(values[0]);
        glGetProgramResourceName(programId, GL_PROGRAM_OUTPUT, i, (GLsizei)nameData.size(), nullptr,
                                 nameData.data());
        string name(nameData.data(), nameData.size() - 1);
        ProgramOutputInfo po {};
        po.Name = name;
        _info.Outputs[po.Name] = po;
      }
    }

    static bool IsSamplerType(GLenum type) noexcept
    {
      switch (type)
      {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:       return true;
        default:                              return false;
      }
    }

    static string StringifyGLType(GLenum type) noexcept
    {
      switch (type)
      {
        case GL_FLOAT:                        return "float";
        case GL_FLOAT_VEC2:                   return "vec2";
        case GL_FLOAT_VEC3:                   return "vec3";
        case GL_FLOAT_VEC4:                   return "vec4";
        case GL_DOUBLE:                       return "double";
        case GL_DOUBLE_VEC2:                  return "dvec2";
        case GL_DOUBLE_VEC3:                  return "dvec3";
        case GL_DOUBLE_VEC4:                  return "dvec4";
        case GL_INT:                          return "int";
        case GL_INT_VEC2:                     return "ivec2";
        case GL_INT_VEC3:                     return "ivec3";
        case GL_INT_VEC4:                     return "ivec4";
        case GL_UNSIGNED_INT:                 return "uint";
        case GL_UNSIGNED_INT_VEC2:            return "uvec2";
        case GL_UNSIGNED_INT_VEC3:            return "uvec3";
        case GL_UNSIGNED_INT_VEC4:            return "uvec4";
        case GL_BOOL:                         return "bool";
        case GL_BOOL_VEC2:                    return "bvec2";
        case GL_BOOL_VEC3:                    return "bvec3";
        case GL_BOOL_VEC4:                    return "bvec4";
        case GL_FLOAT_MAT2:                   return "mat2";
        case GL_FLOAT_MAT3:                   return "mat3";
        case GL_FLOAT_MAT4:                   return "mat4";
        case GL_FLOAT_MAT2x3:                 return "mat2x3";
        case GL_FLOAT_MAT2x4:                 return "mat2x4";
        case GL_FLOAT_MAT3x2:                 return "mat3x2";
        case GL_FLOAT_MAT3x4:                 return "mat3x4";
        case GL_FLOAT_MAT4x2:                 return "mat4x2";
        case GL_FLOAT_MAT4x3:                 return "mat4x3";
        case GL_DOUBLE_MAT2:                  return "dmat2";
        case GL_DOUBLE_MAT3:                  return "dmat3";
        case GL_DOUBLE_MAT4:                  return "dmat4";
        case GL_DOUBLE_MAT2x3:                return "dmat2x3";
        case GL_DOUBLE_MAT2x4:                return "doublmat2x4";
        case GL_DOUBLE_MAT3x2:                return "dmat3x2";
        case GL_DOUBLE_MAT3x4:                return "dmat3x4";
        case GL_DOUBLE_MAT4x2:                return "dmat4x2";
        case GL_DOUBLE_MAT4x3:                return "dmat4x3";
        case GL_SAMPLER_1D:                   return "sampler1D";
        case GL_SAMPLER_2D:                   return "sampler2D";
        case GL_SAMPLER_3D:                   return "sampler3D";
        case GL_SAMPLER_CUBE:                 return "samplerCube";
        case GL_SAMPLER_1D_SHADOW:            return "sampler1DShadow";
        case GL_SAMPLER_2D_SHADOW:            return "sampler2DShadow";
        case GL_SAMPLER_1D_ARRAY:             return "sampler1DArray";
        case GL_SAMPLER_2D_ARRAY:             return "sampler2DArray";
        case GL_SAMPLER_1D_ARRAY_SHADOW:      return "sampler1DArrayShadow";
        case GL_SAMPLER_2D_ARRAY_SHADOW:      return "sampler2DArrayShadow";
        case GL_SAMPLER_2D_MULTISAMPLE:       return "sampler2DMS";
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return "sampler2DMSArray";
        case GL_SAMPLER_CUBE_SHADOW:          return "samplerCubeShadow";
        case GL_SAMPLER_BUFFER:               return "samplerBuffer";
        case GL_SAMPLER_2D_RECT:              return "sampler2DRect";
        case GL_SAMPLER_2D_RECT_SHADOW:       return "sampler2DRectShadow";
        default:                              return "unknown";
      }
    }
  };
}