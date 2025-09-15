

#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

in vec3 localPosition;

uniform vec3 lightColor;

uniform bool countsTowardsBloom = true;

void main()
{           
  FragColor = vec4(lightColor, 1.0);



  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (countsTowardsBloom && brightness > 1.0) {
  // This was my playaround - lower the bloom effect based on rough distance from center of model
    //vec3 factor = (abs(localPosition));
    //float f = (factor.x + factor.y + factor.z) / 3.0;
    //f*= 0.5;
    //BrightColor = vec4(FragColor.rgb * f, 1.0);
    BrightColor = vec4(FragColor.rgb, 1.0);

  }
	else {
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}

