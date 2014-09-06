// http://glslsandbox.com/e#17516.3

// Shader by Nicolas Robert [NRX]
// Latest version: http://glsl.heroku.com/e#15442
// Forked from: https://www.shadertoy.com/view/ldfXzn

#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;
uniform sampler2D tex;
uniform vec3 color;

//#define USE_AA

#define DELTA           0.08
#define NORMAL_DELTA     0.1
#define RAY_COUNT       7
#define RAY_LENGTH_MAX      100.0
#define RAY_STEP_MAX        100
#define LIGHT           vec3 (1.0, 1.0, 1.0)
#define REFRACT_FACTOR      0.6
#define REFRACT_INDEX       1.4
#define AMBIENT         0.2
#define SPECULAR_POWER      3.0
#define SPECULAR_INTENSITY  0.5
#define FADE_POWER      1.0
#define M_PI            3.1415926535897932384626433832795
#define GLOW_FACTOR     2.5
#define LUMINOSITY_FACTOR   2.0

mat3 mRotate (in vec3 angle) {
    float c = cos (angle.x);
    float s = sin (angle.x);
    mat3 rx = mat3 (1.0, 0.0, 0.0, 0.0, c, s, 0.0, -s, c);

    c = cos (angle.y);
    s = sin (angle.y);
    mat3 ry = mat3 (c, 0.0, -s, 0.0, 1.0, 0.0, s, 0.0, c);

    c = cos (angle.z);
    s = sin (angle.z);
    mat3 rz = mat3 (c, s, 0.0, -s, c, 0.0, 0.0, 0.0, 1.0);

    return rz * ry * rx;
}

float box(in vec2 p, in vec2 l, in vec2 r)
{
    vec2 pl = l-p;
    vec2 pr = p-r;
    return max(max(pl.x, pl.y), max(pr.x, pr.y));
}

float circle(in vec2 p, in vec2 o, in vec2 r)
{
    p -= o;
    p.y *= r.x/r.y;
    return length(p)-r.x;
}


float getDistance (in vec3 p) {
    vec3 ap = abs(p)-vec3(23.0, 6.5, 4.0);
    float extra = max(ap.x, max(ap.y, ap.z));
    if(extra > 0.0)
        return extra + 2.0;

    p += vec3(21.0, 4.5, 0.0);
    vec2 z = p.xy;

    float r = 100000.0;

    // V
    r = min(r, box(z, vec2(0.0, 2.0), vec2(2.0, 9.0)));
    r = min(r, box(z, vec2(4.0, 2.0), vec2(6.0, 9.0)));
    r = min(r, box(z, vec2(1.0, 1.0), vec2(5.0, 2.0)));
    r = min(r, box(z, vec2(2.0, 0.0), vec2(4.0, 2.0)));
    r = min(r, box(z, vec2(1.0, 1.0), vec2(2.0, 3.0)));
    r = min(r, box(z, vec2(4.0, 1.0), vec2(5.0, 3.0)));

    // G
    r = min(r, box(z, vec2(8.0, 1.0), vec2(10.0, 8.0)));
    r = min(r, box(z, vec2(9.0, 0.0), vec2(10.0, 9.0)));
    r = min(r, box(z, vec2(9.0, 0.0), vec2(14.0, 1.0)));
    r = min(r, box(z, vec2(12.0, 0.0), vec2(14.0, 4.0)));
    r = min(r, box(z, vec2(11.0, 3.0), vec2(14.0, 4.0)));
    r = min(r, box(z, vec2(9.0, 8.0), vec2(13.0, 9.0)));
    r = min(r, box(z, vec2(12.0, 6.0), vec2(13.0, 9.0)));
    r = min(r, box(z, vec2(12.0, 6.0), vec2(14.0, 8.0)));

    // A
    r = min(r, box(z, vec2(16.0, 0.0), vec2(18.0, 7.0)));
    r = min(r, box(z, vec2(17.0, 0.0), vec2(18.0, 8.0)));
    r = min(r, box(z, vec2(20.0, 0.0), vec2(22.0, 7.0)));
    r = min(r, box(z, vec2(20.0, 0.0), vec2(21.0, 8.0)));
    r = min(r, box(z, vec2(17.0, 7.0), vec2(21.0, 8.0)));
    r = min(r, box(z, vec2(18.0, 7.0), vec2(20.0, 9.0)));
    r = min(r, box(z, vec2(17.0, 3.0), vec2(21.0, 4.0)));

    // F
    r = min(r, box(z, vec2(24.0, 0.0), vec2(26.7, 9.0)));
    r = min(r, box(z, vec2(24.0, 3.5), vec2(29.5, 5.4)));
    r = min(r, box(z, vec2(24.0, 7.2), vec2(29.5, 9.0)));

    // I
    r = min(r, box(z, vec2(30.9, 0.0), vec2(33.6, 9.0)));

    // B
    r = min(r, box(z, vec2(35.0, 0.0), vec2(37.6, 9.0)));
    r = min(r, box(z, vec2(35.0, 0.0), vec2(40.5, 1.8)));
    r = min(r, box(z, vec2(35.0, 3.5), vec2(40.5, 5.4)));
    r = min(r, box(z, vec2(35.0, 7.2), vec2(40.5, 9.0)));
    r = min(r, box(z, vec2(39.0, 0.0), vec2(40.5, 9.0)));
    r = min(r, circle(z, vec2(40.5, 2.25), vec2(1.5, 2.25)));
    r = min(r, circle(z, vec2(40.5, 6.75), vec2(1.5, 2.25)));

    r = max(r, abs(p.z)-1.0);
    return r;
}

float getDistance2 (in vec3 pq) {
    float scale = 0.06;
    vec3 p = pq*scale;

    vec3 ap = abs(p)-vec3(0.5, 0.125, 0.1);
    float extra = max(ap.x, max(ap.y, ap.z));
    if(extra > 0.0)
        return extra/scale + 0.50;

    vec4 lol = texture2D(tex, vec2(p.x, p.y*4.0)+0.5);
    return max(lol.r/scale, abs(pq.z)-0.8);
}

vec3 getFragmentColor (in vec3 origin, in vec3 direction) {
    vec3 lightDirection = normalize (LIGHT);
    vec2 delta = vec2 (NORMAL_DELTA, 0.0);

    vec3 fragColor = vec3 (0.0, 0.0, 0.0);
    float intensity = 1.0;

    float distanceFactor = 1.0;
    float refractionRatio = 1.0 / REFRACT_INDEX;
    float rayStepCount = 0.0;
    for (int rayIndex = 0; rayIndex < RAY_COUNT; ++rayIndex) {

        // Ray marching
        float dist = RAY_LENGTH_MAX;
        float rayLength = 0.0;
        for (int rayStep = 0; rayStep < RAY_STEP_MAX; ++rayStep) {
            dist = distanceFactor * getDistance (origin);
            float distMin = max (dist, DELTA);
            rayLength += distMin;
            if (dist < 0.0 || rayLength > RAY_LENGTH_MAX) {
                break;
            }
            origin += direction * distMin;
            ++rayStepCount;
        }

        // Check whether we hit something
        vec3 backColor = color * (0.1 + 0.2 * max (0.0, dot (direction, lightDirection)));
        if (dist >= 0.0) {
            fragColor = fragColor * (1.0 - intensity) + backColor * intensity;
            break;
        }

        // Get the normal
        vec3 normal = normalize (distanceFactor * vec3 (
            getDistance (origin + delta.xyy) - getDistance (origin - delta.xyy),
            getDistance (origin + delta.yxy) - getDistance (origin - delta.yxy),
            getDistance (origin + delta.yyx) - getDistance (origin - delta.yyx)));

        // Basic lighting
        vec3 reflection = reflect (direction, normal);
        if (distanceFactor > 0.0) {
            float relfectionDiffuse = max (0.0, dot (normal, lightDirection));
            float relfectionSpecular = pow (max (0.0, dot (reflection, lightDirection)), SPECULAR_POWER) * SPECULAR_INTENSITY;
            float fade = pow (1.0 - rayLength / RAY_LENGTH_MAX, FADE_POWER);

            vec3 localColor = color.yzx*0.3;
            localColor = (AMBIENT + relfectionDiffuse) * localColor + relfectionSpecular;
            localColor = mix (backColor, localColor, fade);

            fragColor = fragColor * (1.0 - intensity) + localColor * intensity;
            intensity *= REFRACT_FACTOR;
        }

        // Next ray...
        vec3 refraction = refract (direction, normal, refractionRatio);
        if (dot (refraction, refraction) < DELTA) {
            direction = reflection;
            origin += direction * DELTA * 2.0;
        }
        else {
            direction = refraction;
            distanceFactor = -distanceFactor;
            refractionRatio = 1.0 / refractionRatio;
        }
    }

    // Return the fragment color
    return fragColor * LUMINOSITY_FACTOR + GLOW_FACTOR * rayStepCount / float (RAY_STEP_MAX * RAY_COUNT);
//    return GLOW_FACTOR * rayStepCount / float (RAY_STEP_MAX * RAY_COUNT);
}

vec4 getPixelColor (in vec2 coord)
{
    // Define the ray corresponding to this fragment
    vec2 frag = (2.0 * coord - resolution.xy) / resolution.y;
    vec3 direction = normalize (vec3 (frag, 2.0));

    // Set the camera

    vec3 origin = vec3 (
        45.0 * cos (time * 0.1),
        15.0 * sin (time * 0.142233),
        45.0 * sin (time * 0.1));

    vec3 forward = -origin + vec3 (
        4.0 * cos (time * 0.062),
        0.5 * sin (time * 0.02),
        3.0 * sin (time * 0.124));

    vec3 up = vec3 (sin (time * 0.3), 2.0, 0.0);

    mat3 rotation;
    rotation [2] = normalize (forward);
    rotation [0] = normalize (cross (up, forward));
    rotation [1] = cross (rotation [2], rotation [0]);
    direction = rotation * direction;

    // Set the fragment color
    return vec4 (getFragmentColor (origin, direction), 1.0);
}

void main () {

#ifdef USE_AA
    gl_FragColor = (
        getPixelColor(gl_FragCoord.xy + vec2(0.0, 0.0)) +
        getPixelColor(gl_FragCoord.xy + vec2(0.5, 0.0)) +
        getPixelColor(gl_FragCoord.xy + vec2(0.0, 0.5)) +
        getPixelColor(gl_FragCoord.xy + vec2(0.5, 0.5))) * 0.25;
#else
    gl_FragColor = (
        getPixelColor(gl_FragCoord.xy + vec2(0.0, 0.0)));
#endif
}