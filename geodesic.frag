// Fragment-shader port of geodesic.comp for platforms without OpenGL 4.3
// compute shaders (macOS is capped at OpenGL 4.1). The physics is identical:
// per-pixel Schwarzschild geodesic integration via RK4. Instead of imageStore
// into an image unit, we render a fullscreen quad into an offscreen FBO and
// write the ray color as the fragment output.
#version 410 core

in vec2 TexCoord;
out vec4 FragColor;

// Note: GLSL 4.10 does not allow layout(binding=N) on uniform blocks;
// block bindings are assigned from the C++ side with glUniformBlockBinding.
layout(std140) uniform Camera {
    vec3 camPos;     float _pad0;
    vec3 camRight;   float _pad1;
    vec3 camUp;      float _pad2;
    vec3 camForward; float _pad3;
    float tanHalfFov;
    float aspect;
    bool moving;
    int   _pad4;
} cam;

layout(std140) uniform Disk {
    float disk_r1;
    float disk_r2;
    float disk_num;
    float thickness;
};

layout(std140) uniform Objects {
    int numObjects;
    vec4 objPosRadius[16];
    vec4 objColor[16];
    float  mass[16];
};

const float SagA_rs = 1.269e10;
const float D_LAMBDA = 1e7;
// Rays are considered escaped once they pass the camera's maximum zoom-out
// distance (1e12); integrating out to 1e30 would waste millions of steps.
const float ESCAPE_R = 2e12;

// Globals to store hit info
vec4 objectColor = vec4(0.0);
vec3 hitCenter = vec3(0.0);
float hitRadius = 0.0;

struct Ray {
    float x, y, z, r, theta, phi;
    float dr, dtheta, dphi;
    float E, L;
};
Ray initRay(vec3 pos, vec3 dir) {
    Ray ray;
    ray.x = pos.x; ray.y = pos.y; ray.z = pos.z;
    ray.r = length(pos);
    ray.theta = acos(pos.z / ray.r);
    ray.phi = atan(pos.y, pos.x);

    float dx = dir.x, dy = dir.y, dz = dir.z;
    ray.dr     = sin(ray.theta)*cos(ray.phi)*dx + sin(ray.theta)*sin(ray.phi)*dy + cos(ray.theta)*dz;
    ray.dtheta = (cos(ray.theta)*cos(ray.phi)*dx + cos(ray.theta)*sin(ray.phi)*dy - sin(ray.theta)*dz) / ray.r;
    ray.dphi   = (-sin(ray.phi)*dx + cos(ray.phi)*dy) / (ray.r * sin(ray.theta));

    ray.L = ray.r * ray.r * sin(ray.theta) * ray.dphi;
    float f = 1.0 - SagA_rs / ray.r;
    float dt_dL = sqrt((ray.dr*ray.dr)/f + ray.r*ray.r*(ray.dtheta*ray.dtheta + sin(ray.theta)*sin(ray.theta)*ray.dphi*ray.dphi));
    ray.E = f * dt_dL;

    return ray;
}

bool intercept(Ray ray, float rs) {
    return ray.r <= rs;
}
// Returns true on hit, captures center, radius, and base color
bool interceptObject(Ray ray) {
    vec3 P = vec3(ray.x, ray.y, ray.z);
    for (int i = 0; i < numObjects; ++i) {
        vec3 center = objPosRadius[i].xyz;
        float radius = objPosRadius[i].w;
        if (distance(P, center) <= radius) {
            objectColor = objColor[i];
            hitCenter = center;
            hitRadius = radius;
            return true;
        }
    }
    return false;
}

void geodesicRHS(Ray ray, out vec3 d1, out vec3 d2) {
    float r = ray.r, theta = ray.theta;
    float dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;
    float f = 1.0 - SagA_rs / r;
    float dt_dL = ray.E / f;

    d1 = vec3(dr, dtheta, dphi);
    d2.x = - (SagA_rs / (2.0 * r*r)) * f * dt_dL * dt_dL
         + (SagA_rs / (2.0 * r*r * f)) * dr * dr
         + r * (dtheta*dtheta + sin(theta)*sin(theta)*dphi*dphi);
    d2.y = -2.0*dr*dtheta/r + sin(theta)*cos(theta)*dphi*dphi;
    d2.z = -2.0*dr*dphi/r - 2.0*cos(theta)/(sin(theta)) * dtheta * dphi;
}
void rk4Step(inout Ray ray, float dL) {
    vec3 k1a, k1b;
    geodesicRHS(ray, k1a, k1b);

    ray.r      += dL * k1a.x;
    ray.theta  += dL * k1a.y;
    ray.phi    += dL * k1a.z;
    ray.dr     += dL * k1b.x;
    ray.dtheta += dL * k1b.y;
    ray.dphi   += dL * k1b.z;

    ray.x = ray.r * sin(ray.theta) * cos(ray.phi);
    ray.y = ray.r * sin(ray.theta) * sin(ray.phi);
    ray.z = ray.r * cos(ray.theta);
}
bool crossesEquatorialPlane(vec3 oldPos, vec3 newPos) {
    bool crossed = (oldPos.y * newPos.y < 0.0);
    float r = length(vec2(newPos.x, newPos.z));
    return crossed && (r >= disk_r1 && r <= disk_r2);
}

void main() {
    int WIDTH  = cam.moving ? 200 : 200;
    int HEIGHT = cam.moving ? 150 : 150;

    // gl_FragCoord (0.5, 0.5) corresponds to compute-shader pixel (0, 0),
    // so the ray setup below matches the compute version exactly.
    ivec2 pix = ivec2(gl_FragCoord.xy);
    if (pix.x >= WIDTH || pix.y >= HEIGHT) { FragColor = vec4(0.0); return; }

    // Init Ray
    float u = (2.0 * (pix.x + 0.5) / WIDTH - 1.0) * cam.aspect * cam.tanHalfFov;
    float v = (1.0 - 2.0 * (pix.y + 0.5) / HEIGHT) * cam.tanHalfFov;
    vec3 dir = normalize(u * cam.camRight - v * cam.camUp + cam.camForward);
    Ray ray = initRay(cam.camPos, dir);

    vec4 color = vec4(0.0);
    vec3 prevPos = vec3(ray.x, ray.y, ray.z);
    float lambda = 0.0;

    bool hitBlackHole = false;
    bool hitDisk      = false;
    bool hitObject    = false;

    int steps = cam.moving ? 20000 : 60000;

    for (int i = 0; i < steps; ++i) {
        if (intercept(ray, SagA_rs)) { hitBlackHole = true; break; }
        // Adaptive step: near the hole (small r) step at D_LAMBDA for accurate
        // lensing; far away, where spacetime is nearly flat, take larger steps.
        // Keeps each ray to a few thousand steps so the whole frame stays well
        // under the macOS GPU watchdog limit.
        float dL = D_LAMBDA * clamp(ray.r / (2.0 * SagA_rs), 1.0, 50.0);
        rk4Step(ray, dL);
        lambda += dL;

        vec3 newPos = vec3(ray.x, ray.y, ray.z);
        if (crossesEquatorialPlane(prevPos, newPos)) { hitDisk = true; break; }
        if (interceptObject(ray)) { hitObject = true; break; }
        prevPos = newPos;
        if (ray.r > ESCAPE_R) break;
    }

    if (hitDisk) {
        float r = length(vec3(ray.x, ray.y, ray.z)) / disk_r2;
        vec3 diskColor = vec3(1.0, r, 0.2);
        //r = 1.0 - abs(r - 0.5) * 2.0;
        color = vec4(diskColor, r);

    } else if (hitBlackHole) {
        color = vec4(0.0, 0.0, 0.0, 1.0);

    } else if (hitObject) {
        // Compute shading
        vec3 P = vec3(ray.x, ray.y, ray.z);
        vec3 N = normalize(P - hitCenter);
        vec3 V = normalize(cam.camPos - P);
        float ambient = 0.1;
        float diff = max(dot(N, V), 0.0);
        float intensity = ambient + (1.0 - ambient) * diff;
        vec3 shaded = objectColor.rgb * intensity;
        color = vec4(shaded, objectColor.a);

    } else {
        color = vec4(0.0);
    }

    FragColor = color;
}
