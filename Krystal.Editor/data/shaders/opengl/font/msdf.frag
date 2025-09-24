#version 330 core

uniform sampler2D atlas;

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 textColor;

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

void main() {
  vec3 sample = texture(atlas, TexCoords).rgb;
    // reconstruct signed distance (median of RGB)
  vec3 msd = texture(atlas, TexCoords).rgb;
  float sd = median(msd.r, msd.g, msd.b) - 0.5;
  float w = fwidth(sd);
  float alpha = smoothstep(-w, w, sd);
  FragColor = vec4(textColor, alpha);
}
