// Internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"

// External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>
#include <IL/il.h>

static GLsizei WIDTH = 512, HEIGHT = 512;

using namespace LiteMath;

const float3 g_camPos_default(0, 0, 5);
float3 g_camPos = g_camPos_default;
float cam_rot[] = {0, 0, 0};
float mx = 0, my = 0;

constexpr float scale = 1.0f;
const float3 forward(0.0f, 0.0f, -scale);
const float3 left(-scale, 0.0f, 0.0f);
const float3 up(0.0f, scale, 0.0f);

// Settings
static bool inc_time = true;
static bool g_softShadows = true;
static bool g_reflect = true;
static bool g_refract = true;
static bool g_ambient = true;
static bool g_antiAlias = false;

void setDefaultSettings() {
    g_camPos = g_camPos_default;
    cam_rot[0] = 0.0f; cam_rot[1] = 0.0f; cam_rot[2] = 0.0f;
}

void windowResize(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
}

static bool permitMouseMove = false;

// Callback for mouse movement
static void mouseMove(GLFWwindow *window, double xpos, double ypos) {
    xpos *= 0.01f;
    ypos *= 0.01f;

    auto x1 = float(xpos);
    auto y1 = float(ypos);

    if (permitMouseMove) {
        cam_rot[0] -= y1 - my;
        cam_rot[1] -= x1 - mx;
        constexpr float pi_2 = DEG_TO_RAD * 90;
        cam_rot[0] = std::max(-pi_2, std::min(pi_2, cam_rot[0]));
    }

    mx = x1;
    my = y1;
}

// Callback for actions with mouse buttons
// Permit camera movements with left button pressed only
static void mouseButton(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            permitMouseMove = true;

        } else if (action == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            permitMouseMove = false;
        }
    }
}

// Callback for movement controls
// W - forward
// A - left
// S - backward
// D - right
// Q - up
// E - down
// SPACE - reset position
float multiplier = 1.0f;
float3 step = {0.0f, 0.0f, 0.0f};
float rot_step = 0.0f;
static void keyboardControls(GLFWwindow *window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_W:
        case GLFW_KEY_UP:
            if (action == GLFW_PRESS) {
                step += forward;
            } else if (action == GLFW_RELEASE) {
                step -= forward;
            }
            break;
        case GLFW_KEY_A:
        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS) {
                step += left;
            } else if (action == GLFW_RELEASE) {
                step -= left;
            }
            break;
        case GLFW_KEY_S:
        case GLFW_KEY_DOWN:
            if (action == GLFW_PRESS) {
                step -= forward;
            } else if (action == GLFW_RELEASE) {
                step += forward;
            }
            break;
        case GLFW_KEY_D:
        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS) {
                step -= left;
            } else if (action == GLFW_RELEASE) {
                step += left;
            }
            break;
        case GLFW_KEY_R:
            if (action == GLFW_PRESS) {
                step += up;
            } else if (action == GLFW_RELEASE) {
                step -= up;
            }
            break;
        case GLFW_KEY_F:
            if (action == GLFW_PRESS) {
                step -= up;
            } else if (action == GLFW_RELEASE) {
                step += up;
            }
            break;
        case GLFW_KEY_Q:
            if (action == GLFW_PRESS) {
                rot_step += 0.1f;
            } else if (action == GLFW_RELEASE) {
                rot_step -= 0.1f;
            }
            break;
        case GLFW_KEY_E:
            if (action == GLFW_PRESS) {
                rot_step -= 0.1f;
            } else if (action == GLFW_RELEASE) {
                rot_step += 0.1f;
            }
            break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            if (action == GLFW_PRESS) {
                multiplier *= 2;
            } else if (action == GLFW_RELEASE) {
                multiplier /= 2;
            }
            break;
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS) {
                inc_time = !inc_time;
            }
            break;
        case GLFW_KEY_0:
            if (action == GLFW_PRESS) {
                setDefaultSettings();
                g_softShadows = true;
                g_reflect = true;
                g_refract = true;
                g_ambient = true;
            }
            break;
        case GLFW_KEY_1:
            if (action == GLFW_PRESS) {
                g_softShadows = !g_softShadows;
            }
            break;
        case GLFW_KEY_2:
            if (action == GLFW_PRESS) {
                g_reflect = !g_reflect;
            }
            break;
        case GLFW_KEY_3:
            if (action == GLFW_PRESS) {
                g_refract = !g_refract;
            }
            break;
        case GLFW_KEY_4:
            if (action == GLFW_PRESS) {
                g_ambient = !g_ambient;
            }
            break;
        case GLFW_KEY_5:
            if (action == GLFW_PRESS) {
                g_antiAlias = !g_antiAlias;
            }
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        default:
            break;
    }
}

int initGL() {
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return 0;
}

uint loadSkybox(const std::vector<std::string> &file_names) {
    uint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (int i = 0; i < file_names.size(); i++) {
        uint image = ilGenImage();
        ilBindImage(image);
        ilLoadImage(file_names[i].c_str());

        const int w = ilGetInteger(IL_IMAGE_WIDTH);
        const int h = ilGetInteger(IL_IMAGE_HEIGHT);
        const int format = ilGetInteger(IL_IMAGE_FORMAT);
        const int type = ilGetInteger(IL_IMAGE_TYPE);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB, w, h, 0, format, type, ilGetData());

        ilDeleteImage(image);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return id;
}

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "RayTrace task -- FPS ??", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    setDefaultSettings();
    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetWindowSizeCallback(window, windowResize);
    glfwSetKeyCallback(window, keyboardControls);

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (initGL() != 0)
        return -1;

    // Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "shaders/vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "shaders/raytrace.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second

    GLuint g_vertexBufferObject;
    GLuint g_vertexArrayObject;
    {

        float quadPos[] =
            {
                -1.0f, 1.0f,    // v0 - top left corner
                -1.0f, -1.0f,    // v1 - bottom left corner
                1.0f, 1.0f,    // v2 - top right corner
                1.0f, -1.0f      // v3 - bottom right corner
            };

        g_vertexBufferObject = 0;
        GLuint vertexLocation = 0; // simple layout, assume have only positions at location = 0

        glGenBuffers(1, &g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), (GLfloat *) quadPos, GL_STATIC_DRAW);
        GL_CHECK_ERRORS;

        glGenVertexArrays(1, &g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;

        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glEnableVertexAttribArray(vertexLocation);
        GL_CHECK_ERRORS;
        glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
        GL_CHECK_ERRORS;

        glBindVertexArray(0);
    }

    // Initialize DevIL
    ilInit();
    if (ilGetError() != IL_NO_ERROR) {
        return -1;
    }

    // Load skybox texture
    const std::string path("../skybox/mp_hexagon/hexagon");
    const std::string ext(".tga");
    auto skybox = loadSkybox({
                             path + "_ft" + ext,
                             path + "_bk" + ext,
                             path + "_dn" + ext,
                             path + "_up" + ext,
                             path + "_rt" + ext,
                             path + "_lf" + ext,
                             });


    unsigned frame_index = 0;
    GLsizei g_time = 0;
    double current_time = glfwGetTime();
    double next_time = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();
        GL_CHECK_ERRORS;

        cam_rot[2] += rot_step;
        float4x4 g_rayMatrix = mul(rotate_Z_4x4(cam_rot[2]), mul(rotate_Y_4x4(-cam_rot[1]), rotate_X_4x4(+cam_rot[0])));

        g_camPos += mul(g_rayMatrix, multiplier * step);
        g_rayMatrix.M(3, 0) = g_camPos.x;
        g_rayMatrix.M(3, 1) = g_camPos.y;
        g_rayMatrix.M(3, 2) = g_camPos.z;

        program.SetUniform("g_rayMatrix", g_rayMatrix);

        program.SetUniform("g_screenWidth", WIDTH);
        program.SetUniform("g_screenHeight", HEIGHT);

        program.SetUniform("g_time", g_time);
        if (inc_time) {
            g_time++;
        }

        program.SetUniform("g_softShadows", g_softShadows);
        program.SetUniform("g_reflect", g_reflect);
        program.SetUniform("g_refract", g_refract);
        program.SetUniform("g_ambient", g_ambient);
        program.SetUniform("g_antiAlias", g_antiAlias);

        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        GL_CHECK_ERRORS;  // The last parameter of glDrawArrays is equal to VS invocations

        program.StopUseShader();

        glfwSwapBuffers(window);
        frame_index = (frame_index + 1) % 60;

        // Print fps to window title
        if (frame_index == 0) {
            next_time = glfwGetTime();
            const double elapsed_time = next_time - current_time;
            current_time = next_time;
            const std::string title = "RayTrace task -- FPS " + std::to_string(int(60.0 / elapsed_time));
            glfwSetWindowTitle(window, title.c_str());
        }
    }

    glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteBuffers(1, &g_vertexBufferObject);

    glfwTerminate();
    return 0;
}
