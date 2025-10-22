#version 330 core
#define NUMBER_OF_POINT_LIGHTS 4

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emissive;   // opcional
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;

uniform int   useEmissive = 0;
uniform float emissiveStrength = 1.0;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 sampleAlbedo()   { return texture(material.diffuse,  TexCoords).rgb; }
float sampleSpecular(){ return texture(material.specular, TexCoords).r; }

void main()
{
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    vec3 emissive = vec3(0.0);
    if (useEmissive == 1)
        emissive = texture(material.emissive, TexCoords).rgb * emissiveStrength;

    float alphaTex = texture(material.diffuse, TexCoords).a;
    if (transparency == 1 && alphaTex < 0.1) discard;
    float outAlpha = (transparency == 1) ? alphaTex : 1.0;

    // Con glEnable(GL_FRAMEBUFFER_SRGB) en main.cpp NO apliques pow aquí.
    // Si NO usas sRGB, puedes habilitar gamma out:
    // vec3 gammaOut = pow(result + emissive, vec3(1.0/2.2));
    // color = vec4(gammaOut, outAlpha);

    color = vec4(result + emissive, outAlpha);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient  = light.ambient  * sampleAlbedo();
    vec3 diffuse  = light.diffuse  * diff * sampleAlbedo();
    vec3 specular = light.specular * spec * sampleSpecular();
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance*distance));

    vec3 ambient  = light.ambient  * sampleAlbedo();
    vec3 diffuse  = light.diffuse  * diff * sampleAlbedo();
    vec3 specular = light.specular * spec * sampleSpecular();

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance*distance));

    float theta   = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / max(epsilon, 1e-4), 0.0, 1.0);

    vec3 ambient  = light.ambient  * sampleAlbedo();
    vec3 diffuse  = light.diffuse  * diff * sampleAlbedo();
    vec3 specular = light.specular * spec * sampleSpecular();

    ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}
