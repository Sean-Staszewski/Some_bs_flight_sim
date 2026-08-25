#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 UVcoord;
out vec4 FragColor;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;
uniform vec3 uObjectColor;
uniform float uAmbientStrength;
uniform float uSpecularStrength;
uniform float uShininess;
uniform sampler2D uTex;
uniform int uHasTex;

void main()
{
    vec3 normal = normalize(vNormal);

    vec3 ambient = uAmbientStrength * uLightColor;

    vec3 lightDir = normalize(uLightPos - vFragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 specular = uSpecularStrength * spec * uLightColor;

    vec3 texColor = (uHasTex == 1) ? texture(uTex, UVcoord).rgb : vec3(1.0);
    vec3 litColor = (ambient + diffuse + specular) * uObjectColor * texColor;
    FragColor = vec4(litColor, 1.0);
}
