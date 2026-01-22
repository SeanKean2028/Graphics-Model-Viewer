#version 330 core

in vec3 worldPos;
out vec4 FragColor;

// ---- Configurable Uniforms ----
uniform float cellSize;       // size of each grid cell
uniform float lineWidth;      // width of the grid lines
uniform float fadeDistance;   // how far grid fades out
uniform vec3  gridColor;      // line color
uniform vec3  bgColor;        // background color (floor surface)

void main()
{
    // Project world position to XZ plane (grid on ground)
    float x = worldPos.x;
    float z = worldPos.z;

    // Compute how far each fragment is from a vertical/horizontal grid line
    float gx = abs(fract(x / cellSize) - 0.5);
    float gz = abs(fract(z / cellSize) - 0.5);

    // Closer to 0.0 ⇒ nearer to a line center
    float gridLine = min(gx, gz);

    // Grid line intensity (smooth anti-aliased blending)
    float gridIntensity = 1.0 - smoothstep(0.0, lineWidth, gridLine);

    // Compute distance-based fading (optional)
    float dist = length(worldPos.xz);
    float fade = clamp(1.0 - dist / fadeDistance, 0.0, 1.0);

    // Final color blending
    vec3 color = mix(bgColor, gridColor, gridIntensity * fade);

    FragColor = vec4(color, 0.25f);
}