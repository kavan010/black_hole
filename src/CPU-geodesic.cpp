#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <chrono>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace glm;
using namespace std;
using Clock = std::chrono::high_resolution_clock;

// VARS
double lastPrintTime = 0.0;
int    framesCount   = 0;
double c = 299792458.0;
double G = 6.67430e-11;
bool useGeodesics = false;

struct Camera {
    vec3 pos;
    vec3 target;
    float fovY;
    float azimuth, elevation, radius;
    float minRadius = 1e12f, maxRadius = 1e20f;
    bool dragging = false;
    bool panning = false;
    double lastX = 0, lastY = 0;

    // Adjustable speeds
    float orbitSpeed = 0.008f;
    float panSpeed = 0.001f;
    float zoomSpeed = 1.08f; // closer to 1 = slower zoom

    Camera() : azimuth(0), elevation(M_PI / 2.0f), radius(6.34194e10), fovY(60.0f) {
        target = vec3(0, 0, 0);
        updateVectors();
    }

    void updateVectors() {
        pos.x = target.x + radius * sin(elevation) * cos(azimuth);
        pos.y = target.y + radius * cos(elevation);
        pos.z = target.z + radius * sin(elevation) * sin(azimuth);
    }
    void processMouse(GLFWwindow* window, double xpos, double ypos) {
        float dx = float(xpos - lastX), dy = float(ypos - lastY);
        if (dragging && !panning) {
            // Orbit
            azimuth   -= dx * orbitSpeed;
            elevation -= dy * orbitSpeed;
            elevation = clamp(elevation, 0.01f, float(M_PI)-0.01f);
        } else if (panning) {
            // Pan (move target in camera plane)
            vec3 forward = normalize(target - pos);
            vec3 right = normalize(cross(forward, vec3(0,1,0)));
            vec3 up = cross(right, forward);
            target += -right * dx * panSpeed * radius + up * dy * panSpeed * radius;
        }
        updateVectors();
        lastX = xpos; lastY = ypos;
    }
    void processScroll(double yoffset) {
        // Zoom (dolly in/out)
        if (yoffset < 0)
            radius *= pow(zoomSpeed, -yoffset);
        else
            radius /= pow(zoomSpeed, yoffset);
        radius = clamp(radius, minRadius, maxRadius);
        updateVectors();
    }
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(window);
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                cam->dragging = true;
                cam->panning = (mods & GLFW_MOD_SHIFT);
                double x, y; glfwGetCursorPos(window, &x, &y);
                cam->lastX = x; cam->lastY = y;
            } else if (action == GLFW_RELEASE) {
                cam->dragging = false;
                cam->panning = false;
            }
        }
    }
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(window);
        cam->processMouse(window, xpos, ypos);
    }
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(window);
        cam->processScroll(yoffset);
    }
};
Camera camera;

struct Ray;
void rk4Step(Ray& ray, double dλ, double rs);

struct Engine {
    // -- Quad & Texture render -- //
    GLFWwindow* window;
    GLuint quadVAO;
    GLuint texture;
    GLuint shaderProgram;
    int WIDTH = 800;
    int HEIGHT = 600;
    float width = 100000000000.0f; // Width of the viewport in meters
    float height = 75000000000.0f; // Height of the viewport in meters
    
    Engine() {
        if (!glfwInit()) {
            cerr << "GLFW init failed\n";
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Black Hole", nullptr, nullptr);
        if (!window) {
            cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        GLenum glewErr = glewInit();
        if (glewErr != GLEW_OK) {
            cerr << "Failed to initialize GLEW: "
                << (const char*)glewGetErrorString(glewErr)
                << "\n";
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        cout << "OpenGL " << glGetString(GL_VERSION) << "\n";
        this->shaderProgram = CreateShaderProgram();

        auto result = QuadVAO();
        this->quadVAO = result[0];
        this->texture = result[1];
    }
    GLuint CreateShaderProgram(){
        const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;  // Changed to vec2
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);  // Explicit z=0
            TexCoord = aTexCoord;
        })";

        const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoord);
        })";

        // vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        // fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    };
    vector<GLuint> QuadVAO(){
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,  // top left
            -1.0f, -1.0f,  0.0f, 0.0f,  // bottom left
            1.0f, -1.0f,  1.0f, 0.0f,  // bottom right

            -1.0f,  1.0f,  0.0f, 1.0f,  // top left
            1.0f, -1.0f,  1.0f, 0.0f,  // bottom right
            1.0f,  1.0f,  1.0f, 1.0f   // top right

        };
        
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        vector<GLuint> VAOtexture = {VAO, texture};
        return VAOtexture;
    }
    void renderScene(const vector<unsigned char>& pixels, int texWidth, int texHeight) {
        // update texture w/ ray-tracing results
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        // clear screen and draw textured quad
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        GLint textureLocation = glGetUniformLocation(shaderProgram, "screenTexture");
        glUniform1i(textureLocation, 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_G) {
                useGeodesics = !useGeodesics;
                cout << "Geodesics: " << (useGeodesics ? "ON\n" : "OFF\n");
            }
        }
    }
};
Engine engine;
struct BlackHole {
    vec3 position;
    double mass;
    double radius;
    double r_s;

    BlackHole(vec3 pos, float m) : position(pos), mass(m) {r_s = 2.0 * G * mass / (c*c);}
    bool Intercept(float px, float py, float pz) const {
        float dx = px - position.x;
        float dy = py - position.y;
        float dz = pz - position.z;
        float dist2 = dx * dx + dy * dy + dz * dz;
        return dist2 < r_s * r_s;
    }
};
BlackHole SagA(vec3(0.0f, 0.0f, 0.0f), 8.54e36); // Sagittarius A black hole
struct Ray{
    // -- cartesian coords -- //
    double x;   double y; double z;
    // -- polar coords -- //
    double r;   double phi; double theta;
    double dr;  double dphi; double dtheta;
    double E, L;             // conserved quantities

    Ray(vec3 pos, vec3 dir) : x(pos.x), y(pos.y), z(pos.z) {
        // Step 1: get spherical coords (r, theta, phi)
        r = sqrt(x*x + y*y + z*z);
        theta = acos(z / r);
        phi = atan2(y, x);

        // Step 2: seed velocities (dr, dtheta, dphi)
        // Convert direction to spherical basis
        double dx = dir.x, dy = dir.y, dz = dir.z;
        dr     = sin(theta)*cos(phi)*dx + sin(theta)*sin(phi)*dy + cos(theta)*dz;
        dtheta = cos(theta)*cos(phi)*dx + cos(theta)*sin(phi)*dy - sin(theta)*dz;
        dtheta /= r;
        dphi   = -sin(phi)*dx + cos(phi)*dy;
        dphi  /= (r * sin(theta));

        // Step 3: store conserved quantities
        L = r * r * sin(theta) * dphi;
        double f = 1.0 - SagA.r_s / r;
        double dt_dλ = sqrt((dr*dr)/f + r*r*dtheta*dtheta + r*r*sin(theta)*sin(theta)*dphi*dphi);
        E = f * dt_dλ;
    }
    void step(double dλ, double rs) {
        if (r <= rs) return;
        rk4Step(*this, dλ, rs);
        // convert back to cartesian
        this->x = r * sin(theta) * cos(phi);
        this->y = r * sin(theta) * sin(phi);
        this->z = r * cos(theta);
    }
};

void raytrace(vector<unsigned char>& pixels, int W, int H) {
    pixels.resize(W * H * 3);

    // build camera basis
    vec3 forward = normalize(camera.target - camera.pos);
    vec3 right   = normalize(cross(forward, vec3(0,1,0)));
    vec3 up      = cross(right, forward);
    float aspect = float(W) / float(H);
    float tanHalfFov = tan(radians(camera.fovY) * 0.5f);

    #pragma omp parallel for schedule(dynamic, 4)
    for(int y = 0; y < H; ++y) {
        for(int x = 0; x < W; ++x) {
            // NDC → screen space in [−1,1]
            float u = (2.0f * (x + 0.5f) / float(W)  - 1.0f) * aspect * tanHalfFov;
            float v = (1.0f - 2.0f * (y + 0.5f) / float(H))        * tanHalfFov;
            vec3 dir = normalize(u*right + v*up + forward);

            // construct your Ray
            Ray ray(camera.pos,  dir);

            const int MAX_STEPS = 10000;
            const double D_LAMBDA = 1e7;
            const double ESCAPE_R = 1e14;

            // 2) march the ray forward in λ
            vec3 color(0.0f);
            if (!useGeodesics) {
                double b = 2.0 * dot(camera.pos, dir);
                double c0 = dot(camera.pos, camera.pos) - SagA.r_s*SagA.r_s;
                double disc = b*b - 4.0*c0;
                if (disc > 0.0) {
                    double t1 = (-b - sqrt(disc)) * 0.5;
                    double t2 = (-b + sqrt(disc)) * 0.5;
                    if (t1 > 0.0 || t2 > 0.0)
                        color = vec3(1.0f, 0.0f, 0.0f);
                }
            }
            else {
                // full null‐geodesic march
                Ray ray(camera.pos, dir);
                for(int i = 0; i < MAX_STEPS; ++i) {
                    if (SagA.Intercept(ray.x, ray.y, ray.z)) {
                        color = vec3(1.0f, 0.0f, 0.0f);
                        break;
                    }
                    ray.step(D_LAMBDA, SagA.r_s);
                    if (ray.r > ESCAPE_R) {
                        // escaped to infinity → remains black
                        break;
                    }
                }
            }

            int idx = (y * W + x) * 3;
            pixels[idx+0] = (unsigned char)(color.r * 255);
            pixels[idx+1] = (unsigned char)(color.g * 255);
            pixels[idx+2] = (unsigned char)(color.b * 255);
        }
    }
}

void geodesicRHS(const Ray& ray, double rhs[6], double rs) {
    double r = ray.r;
    double theta = ray.theta;
    double dr = ray.dr;
    double dtheta = ray.dtheta;
    double dphi = ray.dphi;
    double E = ray.E;

    double f = 1.0 - rs / r;
    double dt_dlambda = E / f;

    // First derivatives
    rhs[0] = dr;
    rhs[1] = dtheta;
    rhs[2] = dphi;

    // Second derivatives (from 3D Schwarzschild null geodesics):
    rhs[3] = 
        - (rs / (2 * r * r)) * f * dt_dlambda * dt_dlambda
        + (rs / (2 * r * r * f)) * dr * dr
        + r * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);

    rhs[4] = 
        - (2.0 / r) * dr * dtheta
        + sin(theta) * cos(theta) * dphi * dphi;

    rhs[5] = 
        - (2.0 / r) * dr * dphi
        - 2.0 * cos(theta) / sin(theta) * dtheta * dphi;
}
void addState(const double a[6], const double b[6], double factor, double out[6]) {
    for (int i = 0; i < 6; i++)
        out[i] = a[i] + b[i] * factor;
}
void rk4Step(Ray& ray, double dλ, double rs) {
    double y0[6] = { ray.r, ray.theta, ray.phi, ray.dr, ray.dtheta, ray.dphi };
    double k1[6], k2[6], k3[6], k4[6], temp[6];

    geodesicRHS(ray, k1, rs);
    addState(y0, k1, dλ/2.0, temp);
    Ray r2 = ray;
    r2.r = temp[0]; r2.theta = temp[1]; r2.phi = temp[2];
    r2.dr = temp[3]; r2.dtheta = temp[4]; r2.dphi = temp[5];
    geodesicRHS(r2, k2, rs);

    addState(y0, k2, dλ/2.0, temp);
    Ray r3 = ray;
    r3.r = temp[0]; r3.theta = temp[1]; r3.phi = temp[2];
    r3.dr = temp[3]; r3.dtheta = temp[4]; r3.dphi = temp[5];
    geodesicRHS(r3, k3, rs);

    addState(y0, k3, dλ, temp);
    Ray r4 = ray;
    r4.r = temp[0]; r4.theta = temp[1]; r4.phi = temp[2];
    r4.dr = temp[3]; r4.dtheta = temp[4]; r4.dphi = temp[5];
    geodesicRHS(r4, k4, rs);

    ray.r      += (dλ/6.0)*(k1[0] + 2*k2[0] + 2*k3[0] + k4[0]);
    ray.theta  += (dλ/6.0)*(k1[1] + 2*k2[1] + 2*k3[1] + k4[1]);
    ray.phi    += (dλ/6.0)*(k1[2] + 2*k2[2] + 2*k3[2] + k4[2]);
    ray.dr     += (dλ/6.0)*(k1[3] + 2*k2[3] + 2*k3[3] + k4[3]);
    ray.dtheta += (dλ/6.0)*(k1[4] + 2*k2[4] + 2*k3[4] + k4[4]);
    ray.dphi   += (dλ/6.0)*(k1[5] + 2*k2[5] + 2*k3[5] + k4[5]);
}

void setupCameraCallbacks(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, &camera);
    glfwSetMouseButtonCallback(window, Camera::mouseButtonCallback);
    glfwSetCursorPosCallback(window, Camera::cursorPosCallback);
    glfwSetScrollCallback(window, Camera::scrollCallback);
    glfwSetKeyCallback(window, Engine::keyCallback);
}

// -- MAIN -- //
int main() {
    setupCameraCallbacks(engine.window);
    vector<unsigned char> pixels(engine.WIDTH * engine.HEIGHT * 3);

    auto t0 = Clock::now();
    lastPrintTime = std::chrono::duration<double>(t0.time_since_epoch()).count();

    while (!glfwWindowShouldClose(engine.window)) {
        raytrace(pixels, engine.WIDTH, engine.HEIGHT);
        engine.renderScene(pixels, engine.WIDTH, engine.HEIGHT);

        // 2) FPS counting
        framesCount++;
        auto t1 = Clock::now();
        double now = std::chrono::duration<double>(t1.time_since_epoch()).count();
        if (now - lastPrintTime >= 1.0) {
            cout << "FPS: " << framesCount / (now - lastPrintTime) << "\n";
            framesCount   = 0;
            lastPrintTime = now;
        }

    }

    glfwDestroyWindow(engine.window);
    glfwTerminate();
    return 0;
}


















        // 2) FPS counting
        // framesCount++;
        // auto t1 = Clock::now();
        // double now = std::chrono::duration<double>(t1.time_since_epoch()).count();
        // if (now - lastPrintTime >= 1.0) {
        //     cout << "FPS: " << framesCount / (now - lastPrintTime) << "\n";
        //     framesCount   = 0;
        //     lastPrintTime = now;
        // }
        //raytrace(pixels, engine.WIDTH, engine.HEIGHT);
