#version 420 core

in vec2 TextureCoords;
in vec4 BackgroundColour;
in vec4 BorderColour;
in vec2 Size;
in vec2 BorderThicknessAndRadius;

out vec4 FragmentColour;

float RectSDF(vec2 p, vec2 b, float r)
{
  vec2 d = abs(p) - b + vec2(r);
  return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;
}

void main()
{

  vec2 rectSize = Size;
  float borderThickness = BorderThicknessAndRadius.x;
  float radius = BorderThicknessAndRadius.y;
  
  if (borderThickness <= 0.0)
  {
    FragmentColour = BackgroundColour;
  }
  else 
  {
    vec2 pos = rectSize * TextureCoords;
    float fDist = RectSDF(pos-rectSize/2.0, rectSize/2.0 - borderThickness/2.0-1.0, radius);
    float fBlendAmount = smoothstep(-1.0, 1.0, abs(fDist) - borderThickness / 2.0);

    vec4 v4ToColor = (fDist < 0.0) ? BackgroundColour : vec4(0.0);
    FragmentColour = mix(BorderColour, v4ToColor, fBlendAmount);
  }
}