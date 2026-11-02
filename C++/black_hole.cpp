#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "sim.h"

struct Engine {
    GLuint gridShaderProgram;
    // -- Quad & Texture render -- //
    GLFWwindow* window;
    GLuint quadVAO;
    GLuint texture;
    GLuint shaderProgram;
    GLuint computeProgram = 0;
    // -- UBOs -- //
    GLuint cameraUBO = 0;
    GLuint diskUBO = 0;
    GLuint objectsUBO = 0;
    // -- grid mess vars -- //
    GLuint gridVAO = 0;
    GLuint gridVBO = 0;
    GLuint gridEBO = 0;
    int gridIndexCount = 0;

    int WIDTH = 800;  // Window width
    int HEIGHT = 600; // Window height
    int COMPUTE_WIDTH  = 200;   // Compute resolution width
    int COMPUTE_HEIGHT = 150;  // Compute resolution height
    float width = 100000000000.0f; // Width of the viewport in meters
    float height = 75000000000.0f; // Height of the viewport in meters
    
    Engine() {
        if (!glfwInit()) {
            cerr << "GLFW init failed\n";
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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
        gridShaderProgram = CreateShaderProgram("grid.vert", "grid.frag");

        computeProgram = CreateComputeProgram("geodesic.comp");
        glGenBuffers(1, &cameraUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
        glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW); // alloc ~128 bytes
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, cameraUBO); // binding = 1 matches shader

        glGenBuffers(1, &diskUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, diskUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW); // 3 values + 1 padding
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, diskUBO); // binding = 2 matches compute shader

        glGenBuffers(1, &objectsUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, objectsUBO);
        // allocate space for 16 objects: 
        // sizeof(int) + padding + 16×(vec4 posRadius + vec4 color)
        GLsizeiptr objUBOSize = sizeof(int) + 3 * sizeof(float)
            + 16 * (sizeof(vec4) + sizeof(vec4))
            + 16 * sizeof(float); // 16 floats for mass
        glBufferData(GL_UNIFORM_BUFFER, objUBOSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, objectsUBO);  // binding = 3 matches shader

        auto result = QuadVAO();
        this->quadVAO = result[0];
        this->texture = result[1];
    }
    void generateGrid(const vector<ObjectData>& objects) {
        vector<vec3> vertices;
        vector<uint32_t> indices;
        buildGrid(objects, vertices, indices);

        // 🔌 Upload to GPU
        if (gridVAO == 0) glGenVertexArrays(1, &gridVAO);
        if (gridVBO == 0) glGenBuffers(1, &gridVBO);
        if (gridEBO == 0) glGenBuffers(1, &gridEBO);

        glBindVertexArray(gridVAO);

        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // location = 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

        gridIndexCount = indices.size();

        glBindVertexArray(0);
    }
    void drawGrid(const mat4& viewProj) {
        glUseProgram(gridShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "viewProj"),
                        1, GL_FALSE, glm::value_ptr(viewProj));
        glBindVertexArray(gridVAO);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDrawElements(GL_LINES, gridIndexCount, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }
    void drawFullScreenQuad() {
        glUseProgram(shaderProgram); // fragment + vertex shader
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);

        glDisable(GL_DEPTH_TEST);  // draw as background
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);  // 2 triangles
        glEnable(GL_DEPTH_TEST);
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
    GLuint CreateShaderProgram(const char* vertPath, const char* fragPath) {
        auto loadShader = [](const char* path, GLenum type) -> GLuint {
            std::ifstream in(path);
            if (!in.is_open()) {
                std::cerr << "Failed to open shader: " << path << "\n";
                exit(EXIT_FAILURE);
            }
            std::stringstream ss;
            ss << in.rdbuf();
            std::string srcStr = ss.str();
            const char* src = srcStr.c_str();

            GLuint shader = glCreateShader(type);
            glShaderSource(shader, 1, &src, nullptr);
            glCompileShader(shader);

            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                GLint logLen;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
                std::vector<char> log(logLen);
                glGetShaderInfoLog(shader, logLen, nullptr, log.data());
                std::cerr << "Shader compile error (" << path << "):\n" << log.data() << "\n";
                exit(EXIT_FAILURE);
            }
            return shader;
        };

        GLuint vertShader = loadShader(vertPath, GL_VERTEX_SHADER);
        GLuint fragShader = loadShader(fragPath, GL_FRAGMENT_SHADER);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        GLint linkSuccess;
        glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess) {
            GLint logLen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(program, logLen, nullptr, log.data());
            std::cerr << "Shader link error:\n" << log.data() << "\n";
            exit(EXIT_FAILURE);
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return program;
    }
    GLuint CreateComputeProgram(const char* path) {
        // 1) read GLSL source
        std::ifstream in(path);
        if(!in.is_open()) {
            std::cerr << "Failed to open compute shader: " << path << "\n";
            exit(EXIT_FAILURE);
        }
        std::stringstream ss;
        ss << in.rdbuf();
        std::string srcStr = ss.str();
        const char* src = srcStr.c_str();

        // 2) compile
        GLuint cs = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(cs, 1, &src, nullptr);
        glCompileShader(cs);
        GLint ok; 
        glGetShaderiv(cs, GL_COMPILE_STATUS, &ok);
        if(!ok) {
            GLint logLen;
            glGetShaderiv(cs, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetShaderInfoLog(cs, logLen, nullptr, log.data());
            std::cerr << "Compute shader compile error:\n" << log.data() << "\n";
            exit(EXIT_FAILURE);
        }

        // 3) link
        GLuint prog = glCreateProgram();
        glAttachShader(prog, cs);
        glLinkProgram(prog);
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if(!ok) {
            GLint logLen;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(prog, logLen, nullptr, log.data());
            std::cerr << "Compute shader link error:\n" << log.data() << "\n";
            exit(EXIT_FAILURE);
        }

        glDeleteShader(cs);
        return prog;
    }
    void dispatchCompute(const Camera& cam) {
        // determine target compute‐res
        int cw = cam.moving ? COMPUTE_WIDTH  : 200;
        int ch = cam.moving ? COMPUTE_HEIGHT : 150;

        // 1) reallocate the texture if needed
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                    0,                // mip
                    GL_RGBA8,         // internal format
                    cw,               // width
                    ch,               // height
                    0, GL_RGBA, 
                    GL_UNSIGNED_BYTE, 
                    nullptr);

        // 2) bind compute program & UBOs
        glUseProgram(computeProgram);
        uploadCameraUBO(cam);
        uploadDiskUBO();
        uploadObjectsUBO(objects);

        // 3) bind it as image unit 0
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

        // 4) dispatch grid
        GLuint groupsX = (GLuint)std::ceil(cw / 16.0f);
        GLuint groupsY = (GLuint)std::ceil(ch / 16.0f);
        glDispatchCompute(groupsX, groupsY, 1);

        // 5) sync
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    void uploadCameraUBO(const Camera& cam) {
        struct UBOData {
            vec3 pos; float _pad0;
            vec3 right; float _pad1;
            vec3 up; float _pad2;
            vec3 forward; float _pad3;
            float tanHalfFov;
            float aspect;
            bool moving;
            int _pad4;
        } data;
        vec3 fwd = normalize(cam.target - cam.position());
        vec3 up = vec3(0, 1, 0); // y axis is up, so disk is in x-z plane
        vec3 right = normalize(cross(fwd, up));
        up = cross(right, fwd);

        data.pos = cam.position();
        data.right = right;
        data.up = up;
        data.forward = fwd;
        data.tanHalfFov = tan(radians(60.0f * 0.5f));
        data.aspect = float(WIDTH) / float(HEIGHT);
        data.moving = cam.dragging || cam.panning;

        glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &data);
    }
    void uploadObjectsUBO(const vector<ObjectData>& objs) {
        struct UBOData {
            int   numObjects;
            float _pad0, _pad1, _pad2;        // <-- pad out to 16 bytes
            vec4  posRadius[16];
            vec4  color[16];
            float  mass[16]; 
        } data;

        size_t count = std::min(objs.size(), size_t(16));
        data.numObjects = static_cast<int>(count);

        for (size_t i = 0; i < count; ++i) {
            data.posRadius[i] = objs[i].posRadius;
            data.color[i] = objs[i].color;
            data.mass[i] = objs[i].mass;
        }

        // Upload
        glBindBuffer(GL_UNIFORM_BUFFER, objectsUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
    }
    void uploadDiskUBO() {
        // disk
        float r1 = SagA.r_s * 2.2f;    // inner radius just outside the event horizon
        float r2 = SagA.r_s * 5.2f;   // outer radius of the disk
        float num = 2.0;               // number of rays
        float thickness = 1e9f;          // padding for std140 alignment
        float diskData[4] = { r1, r2, num, thickness };

        glBindBuffer(GL_UNIFORM_BUFFER, diskUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(diskData), diskData);
    }
    
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
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                    0,             // mip
                    GL_RGBA8,      // internal format
                    COMPUTE_WIDTH,
                    COMPUTE_HEIGHT,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    nullptr);
        vector<GLuint> VAOtexture = {VAO, texture};
        return VAOtexture;
    }
    void renderScene() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(quadVAO);
        // make sure your fragment shader samples from texture unit 0:
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
};
Engine engine;


// -- MAIN -- //
int main() {
    setupCameraCallbacks(engine.window);
    vector<unsigned char> pixels(engine.WIDTH * engine.HEIGHT * 3);

    auto t0 = Clock::now();
    lastPrintTime = chrono::duration<double>(t0.time_since_epoch()).count();

    double lastTime = glfwGetTime();
    int   renderW  = 800, renderH = 600, numSteps = 80000;
    while (!glfwWindowShouldClose(engine.window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // optional, but good practice
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double now   = glfwGetTime();
        double dt    = now - lastTime;   // seconds since last frame
        lastTime     = now;

        stepGravity();



        // ---------- GRID ------------- //
        // 2) rebuild grid mesh on CPU
        engine.generateGrid(objects);
        // 5) overlay the bent grid
        mat4 view = lookAt(camera.position(), camera.target, vec3(0,1,0));
        mat4 proj = perspective(radians(60.0f), float(engine.COMPUTE_WIDTH)/engine.COMPUTE_HEIGHT, 1e9f, 1e14f);
        mat4 viewProj = proj * view;
        engine.drawGrid(viewProj);

        // ---------- RUN RAYTRACER ------------- //
        glViewport(0, 0, engine.WIDTH, engine.HEIGHT);
        engine.dispatchCompute(camera);
        engine.drawFullScreenQuad();

        // 6) present to screen
        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }

    glfwDestroyWindow(engine.window);
    glfwTerminate();
    return 0;
}
