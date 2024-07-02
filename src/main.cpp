#include <glm/geometric.hpp>

#include "Object.hpp"
#include "SkyBox.hpp"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Model3D.hpp"
#include "Shader.hpp"

int glWindowWidth  = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH  = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

glm::mat4 projection;

glm::vec3 lightDirInitial;
glm::vec3 lightColor;
glm::vec3 fogColor;
glm::vec3 lightPos;
glm::vec3 camPos;
bool lightning          = false;
float lightningTime     = 0.0f;
unsigned int indexFrame = 0;

gps::Camera camera(
    glm::vec3(-0.005404f, 0.967317f, 3.241670f),
    glm::vec3(-0.027700f, 0.832476f, 4.238724f),
    glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 5.0f;

std::pair<float, glm::vec3> lightningFrames[] = {
    { 0.05f, glm::vec3(0.5f) },
    { 0.1f, glm::vec3(1.0f) },
    { 0.2f, glm::vec3(0.6f) },
    { 0.3f, glm::vec3(1.0f) },
};
unsigned sizeArray = sizeof(lightningFrames) / sizeof(lightningFrames[0]);

std::pair<glm::vec3, glm::vec3> frames[] = {
    { glm::vec3(-4.016524f, 1.611700f, -5.595103f), glm::vec3(0.246925f, -0.024432f, 0.968726f) },
    { glm::vec3(7.676317f, 5.647618f, -1.333831f), glm::vec3(0.258880f, -0.340381f, -0.903948f) },
    { glm::vec3(10.524207f, 17.423429f, -39.580112f), glm::vec3(-0.441431f, -0.363254f, 0.820479f) },
    { glm::vec3(-3.674638f, 21.043348f, 49.152260f), glm::vec3(-0.645912f, -0.372991f, -0.666089f) },
    { glm::vec3(-2.925408f, 36.051998f, 80.302277f), glm::vec3(-0.125381f, -0.438376f, -0.890004f) },
    { glm::vec3(-0.005404f, 0.967317f, 3.241670f), glm::vec3(-0.022155f, -0.133986f, 0.990735f) },
};
unsigned int cameraTransitions = sizeof(frames) / sizeof(frames[0]);
int currentTransition          = 0;
float duration                 = 2.0f;
bool enableTransitions         = false;

bool pressedKeys[1024];
GLfloat lightAngle;
float lightRotationSpeed = 45.0f;  // deg per second

Object town;
Object windmil;
Object watermil;
Object lampOne;
gps::Model3D screenQuad;

gps::Shader townShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

gps::SkyBox skyBox;
gps::Shader skyboxShader;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
    // TODO
}

int viewState  = 0;
int lightState = 0;

void setTransition() {
    camera.setTransition(frames[currentTransition].first, frames[currentTransition].second, duration);
    currentTransition++;
    currentTransition %= cameraTransitions;
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        viewState = (viewState + 1) % 3;

        switch (viewState) {
            case 0:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            case 1:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case 2:
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                break;

            default: break;
        }
    }

    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        lightState = (lightState + 1) % 3;
    }

    if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
        indexFrame    = 0;
        lightningTime = 0;
        lightning     = true;
    }

    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        enableTransitions = !enableTransitions;
        setTransition();
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}

glm::vec2 lastClickPos(0.0f, 0.0f);
bool isMousePressed = false;
bool isFirstClick   = true;
float sensitivity   = 0.1f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (isFirstClick) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            isFirstClick = false;
            lastClickPos = glm::vec2(xpos, ypos);
        }

        float xOffset = xpos - lastClickPos.x;
        float yOffset = ypos - lastClickPos.y;
        lastClickPos  = glm::vec2(xpos, ypos);

        xOffset *= sensitivity;
        yOffset *= sensitivity;

        camera.rotate(xOffset, yOffset);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isFirstClick = true;
    }
}

glm::vec3 getLightDir() {
    auto lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    return glm::inverseTranspose(glm::mat3(lightRotation)) * lightDirInitial;
}
float watermilAngle = 0;
float windmilAngle  = 0;
float timer         = 0.0f;
float timeSpent     = 2.0f;
void processMovement(float deltaSec) {
    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= lightRotationSpeed * deltaSec;
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += lightRotationSpeed * deltaSec;
    }

    if (camera.transitioning) {
        camera.updateTransition(deltaSec);
        timer = 0.0f;
    } else if (enableTransitions) {
        timer += deltaSec;
        if (timer > timeSpent) {
            setTransition();
        }
    }

    else {
        if (pressedKeys[GLFW_KEY_W]) {
            camera.move(gps::MOVE_FORWARD, cameraSpeed * deltaSec);
        }

        if (pressedKeys[GLFW_KEY_S]) {
            camera.move(gps::MOVE_BACKWARD, cameraSpeed * deltaSec);
        }

        if (pressedKeys[GLFW_KEY_A]) {
            camera.move(gps::MOVE_LEFT, cameraSpeed * deltaSec);
        }

        if (pressedKeys[GLFW_KEY_D]) {
            camera.move(gps::MOVE_RIGHT, cameraSpeed * deltaSec);
        }
    }

    watermilAngle -= deltaSec * 8;
    windmilAngle -= deltaSec * 16;

    watermil.ResetModelMatrix();
    watermil.Translate(glm::vec3(-1.10, -0.45, 15.694));
    watermil.Rotate(watermilAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    watermil.ComputeNormalMatrix(camera.getViewMatrix());

    windmil.ResetModelMatrix();
    windmil.Translate(glm::vec3(-41.269f, 3.750f, 26.931f));
    windmil.Rotate(-12.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    windmil.Rotate(53.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    windmil.Rotate(windmilAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    windmil.Rotate(12.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    windmil.Rotate(-53.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    windmil.ComputeNormalMatrix(camera.getViewMatrix());
    if (!lightning)
        return;

    lightningTime += deltaSec;
    if (lightningTime < lightningFrames[indexFrame].first) {
        townShader.useShaderProgram();
        townShader.setUniform("lightColor", lightningFrames[indexFrame].second);
    } else {
        indexFrame++;
        if (indexFrame >= sizeArray) {
            lightning = false;
            townShader.useShaderProgram();
            townShader.setUniform("lightColor", lightColor);
        }
    }
}
bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    // for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    // glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);

    glfwSwapInterval(1);

#if not defined(__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
    const GLubyte* version  = glGetString(GL_VERSION);   // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // for RETINA display
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void debugMessageCallback(
    const unsigned source,
    const unsigned type,
    const unsigned id,
    const unsigned severity,
    const int length,
    const char* msg,
    const void* data) {
    std::string _source;
    std::string _type;
    std::string _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
            _source = "OTHER";
            break;

        default:
            _source = "UNKNOWN";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;

        default:
            _type = "UNKNOWN";
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            break;

        default:
            _severity = "UNKNOWN";
            break;
    }

    printf("Error %u --> %s of %s, raised from %s:\n%s\n\n", id, _type.c_str(), _severity.c_str(), _source.c_str(), msg);
}

void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);

    glEnable(GL_DEPTH_TEST);  // enable depth-testing
    glDepthFunc(GL_LESS);     // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE);   // cull face
    glCullFace(GL_BACK);      // cull back face
    glFrontFace(GL_CCW);      // GL_CCW for counter clock-wise

    glEnable(GL_FRAMEBUFFER_SRGB);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugMessageCallback, 0);
    glPointSize(15.0);  // Set the size of the points

    glColor3f(1.0, 1.0, 1.0);
}

void initObjects() {
    town.LoadModel("models/medieval/medieval.obj");
    windmil.LoadModel("models/windmil/windmil.obj");
    watermil.LoadModel("models/watermil/watermil.obj");
    lampOne.LoadModel("models/lamp/lamp.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
}

void initShaders() {
    townShader.loadShader("shaders/town.vert", "shaders/town.frag");
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");

    skyboxShader.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
    townShader.useShaderProgram();

    town.ResetModelMatrix();
    town.ComputeNormalMatrix(camera.getViewMatrix());

    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    townShader.setUniform("projection", projection);

    lightDirInitial = glm::normalize(glm::vec3(0.0f, 1.0f, 4.0f));
    townShader.setUniform("lightDir", getLightDir());

    lightColor = glm::vec3(0.05f, 0.05f, 0.2f);
    townShader.setUniform("lightColor", lightColor);

    fogColor = glm::vec3(1.0f, 1.0f, 1.0f);
    townShader.setUniform("fogColor", fogColor);

    lightPos = glm::vec3(9.2f, 2.1f, -12.6f);
    townShader.setUniform("lightPos", lightPos);

    townShader.setUniform("camPos", camera.position);

    townShader.setUniform("lightState", lightState);
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);

    // create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 lightView      = glm::lookAt(60.0f * getLightDir(), 
    glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 75.0f;
    glm::mat4 lightProjection    = glm::ortho(-65.0f, 65.0f, -50.0f, 50.0f, 
    near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    town.Draw(shader, !depthPass);
    windmil.Draw(shader, !depthPass);
    watermil.Draw(shader, !depthPass);

    lampOne.ResetModelMatrix();
    lampOne.Scale(glm::vec3(0.5f, 0.5f, 0.5f));
    lampOne.Translate(glm::vec3(16.512847f, 0.0f, -25.442291f));
    lampOne.Rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    lampOne.ComputeNormalMatrix(camera.getViewMatrix());

    lampOne.Draw(shader, !depthPass);
}

void renderScene() {
    // render the scene to the depth buffer
    depthMapShader.useShaderProgram();
    depthMapShader.setUniform("lightSpaceTrMatrix", computeLightSpaceTrMatrix());
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        screenQuadShader.setUniform("depthMap", 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);

        return;
    }

    // final scene rendering pass (with shadows)
    glViewport(0, 0, retina_width, retina_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    townShader.useShaderProgram();
    auto view = camera.getViewMatrix();
    townShader.setUniform("view", view);
    townShader.setUniform("lightDir", getLightDir());
    townShader.setUniform("camPos", camera.position);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    townShader.setUniform("shadowMap", 3);
    townShader.setUniform("lightSpaceTrMatrix", computeLightSpaceTrMatrix());
    townShader.setUniform("lightState", lightState);
    drawObjects(townShader, false);

    // draw skybox
    skyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);
    glfwTerminate();
}

void initSkyBox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/nightsky_rt.tga");
    faces.push_back("skybox/nightsky_lf.tga");
    faces.push_back("skybox/nightsky_up.tga");
    faces.push_back("skybox/nightsky_dn.tga");
    faces.push_back("skybox/nightsky_bk.tga");
    faces.push_back("skybox/nightsky_ft.tga");
    skyBox.Load(faces);
}

int main(int argc, const char* argv[]) {
    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initOpenGLState();
    initObjects();
    initShaders();
    initUniforms();
    initFBO();
    initSkyBox();

    double lastTimeStamp = glfwGetTime();

    while (!glfwWindowShouldClose(glWindow)) {
        double currentTimeStamp = glfwGetTime();
        float delta             = currentTimeStamp - lastTimeStamp;
        lastTimeStamp           = currentTimeStamp;

        processMovement(delta);
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();

    return 0;
}
