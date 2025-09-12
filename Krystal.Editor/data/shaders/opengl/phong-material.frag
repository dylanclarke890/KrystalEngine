#version 330 core

struct Material
{
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  float shininess;
};

struct DirectionalLight
{
  vec3 direction;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight
{
 vec3 position;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct PointLight
{
  vec3 position;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct SpotLight
{
  vec3 position;
  vec3 direction;
  float cutOff;
  float outerCutOff;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  
  float constant;
  float linear;
  float quadratic;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform DirectionalLight directionalLight;

#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform SpotLight spotLight;

uniform float time;

out vec4 FragColor;

vec3 CalcDirectionalLight(DirectionalLight light);
vec3 CalcPointLight(PointLight light);
vec3 CalcSpotLight(SpotLight light);
vec3 CalcEmission();

void main()
{
  vec3 result = vec3(0.0);
  result += CalcDirectionalLight(directionalLight);
  for (int i = 0; i < NR_POINT_LIGHTS; i++)
    result += CalcPointLight(pointLights[i]);
  result += CalcSpotLight(spotLight);
  // result += CalcEmission();

  FragColor = vec4(result, 1.0);
}

vec3 CalcDirectionalLight(DirectionalLight light)
{
  // ambient
  vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
  	
  // diffuse 
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(-light.direction);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));  
    
  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  float spec = pow(max(dot(norm, halfwayDir), 0.0),  material.shininess);
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));  
        
  vec3 result = ambient + diffuse + specular;
  return result;
}

vec3 CalcPointLight(PointLight light)
{
  // ambient
  vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    
  // diffuse 
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.position - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));  
    
  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  float spec = pow(max(dot(norm, halfwayDir), 0.0),  material.shininess);
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));  
        
  // attenuation
  float distance    = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + 
             light.quadratic * (distance * distance));    
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  vec3 result = ambient + diffuse + specular;
  return result;
}

vec3 CalcSpotLight(SpotLight light)
{
  // ambient
  vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    
  // diffuse 
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.position - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));  
    
  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  float spec = pow(max(dot(norm, halfwayDir), 0.0),  material.shininess);
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));  
        
  // attenuation
  float distance    = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + 
             light.quadratic * (distance * distance));    
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  
  // spotlight intensity
  float theta     = dot(lightDir, normalize(-light.direction)); 
  float epsilon   = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
  diffuse  *= intensity;
  specular *= intensity;
  
  vec3 result = ambient + diffuse + specular;
  return result;
}

vec3 CalcEmission()
{
  vec3 emission = vec3(texture(material.emission, TexCoords));
  emission *= (sin(time) * 0.5 + 0.5);
  return emission;
}