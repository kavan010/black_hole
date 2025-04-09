#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cmath>
// #include <cuda_runtime.h>
// #include <cuda_gl_interop.h>
// #include <device_launch_parameters.h>

using namespace glm;
using namespace std;

// global vars
const int WIDTH = 800;
const int HEIGHT = 600;
const float G = 6.67430 * pow(10, -11);

// functions

// structures and classes :D
class Engine{
public:
    // -- Quad & Texture render
    GLFWwindow* window;
    GLuint quadVAO;
    GLuint texture;
    GLuint shaderProgram;

    Engine(){
        this->window = StartGLFW();
        this->shaderProgram = CreateShaderProgram();
        
        auto result = QuadVAO();
        this->quadVAO = result[0];
        this->texture = result[1];
    }
    GLFWwindow* StartGLFW(){
        if(!glfwInit()){
            std::cerr<<"glfw failed init, PANIC PANIC!"<<std::endl;
            return nullptr;
        }

        GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "ray tracer", NULL, NULL);
        glfwMakeContextCurrent(window);
        
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW." << std::endl;
            glfwTerminate();
            return nullptr;
        }

        glViewport(0, 0, WIDTH, HEIGHT);
        return window;
    };
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

    std::vector<GLuint> QuadVAO(){
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
        std::vector<GLuint> VAOtexture = {VAO, texture};
        return VAOtexture;
    }
    void renderScene(const std::vector<unsigned char>& pixels, int texWidth, int texHeight) {
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
    std::vector<int> OptimizeMovement(double lastMovementTime){
        double currentTime = glfwGetTime();
        bool isMoving = (currentTime - lastMovementTime < 0.2);
        int renderFactor = isMoving ? 4 : 2;
        int rWidth = WIDTH / renderFactor;
        int rHeight = HEIGHT / renderFactor;
        std::vector<int> vec = {rWidth, rHeight};
        return vec;
    }
};
class Camera{
public:
    vec3 target;
    float distance;
    float pitch;
    float yaw;
    vec3 position;
    vec3 up;
    
    // mouse handling
    bool middleMousePressed = false;
    double lastX = 0.0, lastY = 0.0;
    float orbitSpeed = 0.4f;
    float zoomSpeed = 2.0f;

    float fov = 60.0f; 
    double lastMovementTime = 0.0;
    // default: look at (0,0,0), 5 units far.
    Camera(vec3 t = vec3(0.0f, 0.0f, -19.0f), float dist = 5.0f, float yawVal = -90.0f, float pitchVal = 0.0f, float fovVal = 90.0f)
        : target(t), distance(dist), yaw(yawVal), pitch(pitchVal), fov(fovVal) {
        up = vec3(0, 1, 0);
        updatePosition();
    }
    void updatePosition() {
        float radYaw = radians(yaw);
        float radPitch = radians(pitch);
        position.x = target.x + distance * cos(radPitch) * cos(radYaw);
        position.y = target.y + distance * sin(radPitch);
        position.z = target.z + distance * cos(radPitch) * sin(radYaw);
    }
    
    // Member function to handle mouse button events.
    void handleMouseButton(int button, int action, int mods, GLFWwindow* window) {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (action == GLFW_PRESS) {
                middleMousePressed = true;
                glfwGetCursorPos(window, &lastX, &lastY);
                lastMovementTime = glfwGetTime();
            } else if (action == GLFW_RELEASE) {
                middleMousePressed = false;
            }
        }
    }
    void handleCursorPosition(double xpos, double ypos, GLFWwindow* window) {
        if (!middleMousePressed)
            return;

        double deltaX = xpos - lastX;
        double deltaY = -(ypos - lastY);

        // If shift is held, pan the camera's target.
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            vec3 forward = normalize(target - position);
            vec3 right = normalize(cross(forward, up));
            vec3 camUp = cross(right, forward);
            float panSpeed = 0.005f * distance;
            target += -right * (float)deltaX * panSpeed + camUp * (float)deltaY * panSpeed;
        }
        // Otherwise, orbit the camera.
        else {
            yaw   += (float)deltaX * orbitSpeed;
            pitch += (float)deltaY * orbitSpeed;
            if (pitch > 89.0f)  pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }
        updatePosition();
        lastX = xpos;
        lastY = ypos;
        lastMovementTime = glfwGetTime();
    }
    void handleScroll(double xoffset, double yoffset, GLFWwindow* window) {
        // If this is the first input, initialize mouse position
        if (lastX == 0 && lastY == 0) {
            glfwGetCursorPos(window, &lastX, &lastY);
        }

        distance -= (float)yoffset * zoomSpeed;
        if (distance < 1.0f)
            distance = 1.0f;
        
        updatePosition();
        lastMovementTime = glfwGetTime();
    }
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        cam->handleMouseButton(button, action, mods, window);
    }
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        cam->handleCursorPosition(xpos, ypos, window);
    }
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        cam->handleScroll(xoffset, yoffset, window);
    }

    void registerCallbacks(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, Camera::mouseButtonCallback);
        glfwSetCursorPosCallback(window, Camera::cursorPositionCallback);
        glfwSetScrollCallback(window, Camera::scrollCallback);
    }
};

struct Ray{
    vec3 direction;
    vec3 origin;
    Ray(vec3 o, vec3 d) : origin(o), direction(normalize(d)){}
};
struct Material{
    vec3 color;
    float specular;
    float emission;
    float shiny;
    Material(vec3 c, float s, float e, float sh = 4.0) : color(c), specular(s), emission(e), shiny(sh) {}
};
struct Object{
    vec3 position;
    vec3 velocity;
    float radius;
    float mass = 7.3 * pow(10, 22);
    Material material;

    Object(vec3 p, float r, Material m, vec3 v = vec3(0.0)) : position(p), radius(r), material(m), velocity(v) {}
    // raytracing
    bool Intersect(Ray &ray, float &t) const {
        vec3 oc = ray.origin - position;             // centre to origin
        float a = dot(ray.direction, ray.direction); // ray straightness / magnitute
        float b = 2.0f * dot(oc, ray.direction);     // orientation towards 
        float c = dot(oc, oc) - radius * radius;     // adjustment by sphere radius
        double discriminant = b*b - 4*a*c;
        if(discriminant < 0){return false;}          // no intersection with sphere

        float intercept = (-b - sqrt(discriminant)) / (2.0f*a);
        // if(intercept < 0){
        //     intercept = (-b + sqrt(discriminant)) / (2.0f*a);
        //     if(intercept<0){return false;}           // intersection is behind origin
        // }
        t = intercept;
        return true;
    };
    vec3 getNormal(vec3 &point) const{
        return normalize(point - position);
    }
    void UpdatePos(){
        this->position[0] += this->velocity[0];
        this->position[1] += this->velocity[1];
        this->position[2] += this->velocity[2];
    }
    // gravity
    void accelerate(float x, float y, float z){
        this->velocity[0] += x;
        this->velocity[1] += y;
        this->velocity[2] += z;
    }
};

class Scene {
public:
    std::vector<Object> objs;
    //std::vector<Object> lightPos;
    vector<const Object*> lights;

    vec3 trace(Ray &ray, int depth = 0){
        const int maxDepth = 4;
        if (depth >= maxDepth) return vec3(0.0f, 0.0f, 0.0f);

        float closest = INFINITY;
        const Object* hitObj = nullptr;

        for(auto& obj : objs){
            float t;                    // distance to intersection
            if(obj.Intersect(ray, t)){
                if(t < closest) {
                    closest = t;
                    hitObj = &obj;
                }
            }
        };

        if(!hitObj){return vec3(0.05f, 0.05f, 0.1f);}
        
        // hit
        vec3 hitPoint = ray.origin + ray.direction * closest;     // point on obj hit by ray
        vec3 normal = hitObj->getNormal(hitPoint);
        vec3 viewDir = normalize(-ray.direction);                 // direction to camera
        vec3 finalColor = hitObj->material.color * 0.3f;

        if (hitObj->material.emission > 0.0f) {
            finalColor += hitObj->material.color * hitObj->material.emission;
        }

        for(auto& light : lights) {
            if (light == hitObj) continue;
            vec3 lightDir = normalize(light->position - hitPoint); // light to hitpoint dir
            float distanceToLight = length(light->position - hitPoint);
            if (distanceToLight < 0.001f) continue;

            // Compute diffuse lighting
            vec3 lightColor = light->material.color * light->material.emission;
            float diff = std::max(dot(normal, lightDir), 0.0f);
            float attenuation = 1.0f / (distanceToLight * distanceToLight);
            vec3 diffuse = hitObj->material.color * lightColor * diff * attenuation;

            // Compute specular Lighting
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(std::max(dot(viewDir, reflectDir), 0.0f), hitObj->material.shiny);
            vec3 specular = lightColor * hitObj->material.specular * spec * attenuation;

            Ray shadowRay(hitPoint + normal * 0.001f, lightDir);
            bool inShadow = false;
            for(const auto& obj : objs) {
                if (&obj != hitObj) {
                    float t;
                    if (obj.Intersect(shadowRay, t) && t < distanceToLight) {
                        inShadow = true;
                        break;
                    }
                }
            }
            // Add light contribution if not in shadow
            if (!inShadow) {
                finalColor += diffuse + specular;
            }

        }
        if(hitObj->material.specular > 0.0f) {
            vec3 reflectDir = reflect(ray.direction, normal); // Reflect ray direction
            Ray reflectRay(hitPoint + normal * 0.001f, reflectDir);
            vec3 reflectedColor = trace(reflectRay, depth + 1); // Recurse
            finalColor += reflectedColor * hitObj->material.specular * 0.3f; // Add reflection scaled by specular intensity
        }
        return finalColor;
    };
};

// --- main loop ---- //
int main(){
    Engine engine;
    Scene scene;
    Camera camera(vec3(0.0f, 0.0f, -9.0f), -15.0f, -90.0f, 0.0f, 90.0f);
    camera.registerCallbacks(engine.window);

    scene.objs = {
                      // position     radius   material:  color               specular   emission
        Object(vec3(0.0f, -5.0f, -19.0f), 12.0f, Material(vec3(1.0f, 0.0f, 0.0f), 0.9f, 10.0f)),
        Object(vec3(20.0f, -2.0f, -11.0f), 5.5f, Material(vec3(0.1f, 1.0f, 0.1f), 0.2f, 0.5f)),
        Object(vec3(-17.0f, -1.0f, -6.0f), 7.0f, Material(vec3(0.1f, 0.1f, 1.0f), 0.8f, 0.3f)),
        Object(vec3(-17.0f, -10.0f, 9.0f), 7.0f, Material(vec3(0.1f, 1.0f, 1.0f), 0.0f, 0.3f)),
    };
    // -- loop -- //
    double lastFrame = glfwGetTime();
    while(!glfwWindowShouldClose(engine.window)){
        glClear(GL_COLOR_BUFFER_BIT);
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        int rWidth = engine.OptimizeMovement(camera.lastMovementTime)[0];
        int rHeight = engine.OptimizeMovement(camera.lastMovementTime)[1];
        std::vector<unsigned char> pixels(rWidth * rHeight * 3);

        // Update light sources
        scene.lights.clear();
        for (const auto& obj : scene.objs) {
            if (obj.material.emission > 0.0f) {
                scene.lights.push_back(&obj);
            }
        }

        // render texture (pxl by pxl)
        for(int y = 0; y < rHeight; ++y){
            for(int x = 0; x < rWidth; ++x){
                float scale = tan(radians(camera.fov * 0.5f));

                float aspectRatio = float(rWidth) / float(rHeight);
                float u = float(x) / float(rWidth); // % of width
                float v = float(y) / float(rHeight); // % of height
                
                // Convert screen coordinates to camera space coordinates with FOV adjustment
                float x_camera = (2.0f * u - 1.0f) * aspectRatio * scale;
                float y_camera = (1.0f - 2.0f * v) * scale; // (1 - 2*v) is equivalent to -(2*v - 1)

                // Transform the ray from camera space to world space.
                vec3 forward = normalize(camera.target - camera.position);
                vec3 right = normalize(cross(forward, vec3(0.0f, 1.0f, 0.0f)));
                vec3 up = cross(right, forward);

                vec3 direction = normalize(x_camera * right + y_camera * up + forward);

                Ray ray(camera.position, direction);
                vec3 color = scene.trace(ray);
                color = color / (color + vec3(0.5f));  // Reinhard tone mapping
                color = clamp(color, 0.0f, 1.0f);

                int index = (y * rWidth  + x) * 3;
                pixels[index + 0] = static_cast<unsigned char>(color.r * 255);
                pixels[index + 1] = static_cast<unsigned char>(color.g * 255);
                pixels[index + 2] = static_cast<unsigned char>(color.b * 255);
            }
        }

        for(auto& obj : scene.objs) {
            if(obj.position[1] > 0){
                obj.velocity *= -0.8f;
                obj.position[1] = 0.1f;
            }
            obj.accelerate(0.0, 9.81 * deltaTime, 0.0);
            obj.UpdatePos();
        }
        
        engine.renderScene(pixels, rWidth, rHeight);
    }

    glfwTerminate();
}


// func dec's





