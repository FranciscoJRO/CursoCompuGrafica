#version 330 core
out vec4 FragColor;

uniform bool  uIsTransparent;   // true para el agua
uniform float uAlphaFallback;   // opacidad a usar si uIsTransparent=true (ej. 0.35)

in vec2 TexCoords;
uniform sampler2D texture_diffuse1;

// (opcional) ligero tinte para el agua
uniform bool  uUseTint;
uniform vec3  uTint;            // ej. vec3(0.25, 0.45, 0.55)

void main()
{
    vec4 tex = texture(texture_diffuse1, TexCoords);

    float alpha = uIsTransparent ? uAlphaFallback : 1.0;

    // si decides honrar el alpha del PNG cuando exista:
    // if (uIsTransparent) alpha = max(tex.a, uAlphaFallback);

    vec3 rgb = tex.rgb;
    if (uIsTransparent && uUseTint) {
        // mezcla sutil hacia el tinte
        rgb = mix(rgb, uTint, 0.4);   // 40% tinte
    }

    // descarta fragmentos “muy transparentes” (evita halos)
    if (uIsTransparent && alpha < 0.05) discard;

    FragColor = vec4(rgb, alpha);
}
