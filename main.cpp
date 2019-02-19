//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

static GLsizei WIDTH = 1280, HEIGHT = 720; //размеры окна

using namespace LiteMath;

const float3 g_camPos_default(0, 0, 5);
float3 g_camPos = g_camPos_default;
float cam_rot[2] = {0, 0};
float mx = 0, my = 0;

//float4x4 rot_mat;
constexpr float scale = 1.0f;
const float3 forward_default(0.0f, 0.0f, -scale);
const float3 left_default(-scale, 0.0f, 0.0f);
const float3 up_default(0.0f, scale, 0.0f);

float3 forward;
float3 left;
float3 up;

void setDefaultSettings() {
    g_camPos = g_camPos_default;
    cam_rot[0] = 0.0f; cam_rot[1] = 0.0f;
    forward = forward_default;
    left = left_default;
    up = up_default;
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

        const float4x4 rot_mat = mul(rotate_Y_4x4(-cam_rot[1]), rotate_X_4x4(+cam_rot[0]));
        forward = mul(rot_mat, forward_default);
        left = mul(rot_mat, left_default);
        up = mul(rot_mat, up_default);
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
float3 step = {0.0f, 0.0f, 0.0f};
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
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS) {
                setDefaultSettings();
            }
            break;
        default:
            break;
    }
}

int initGL() {
    int res = 0;
    //грузим функции opengl через glad
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

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    //запрашиваем контекст opengl версии 3.3
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

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "shaders/vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "shaders/raytrace.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second

    //Создаем и загружаем геометрию поверхности
    //
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

    //цикл обработки сообщений и отрисовки сцены каждый кадр
    GLsizei i = 0;
    double current_time = glfwGetTime();
    double next_time = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        //очищаем экран каждый кадр
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();
        GL_CHECK_ERRORS;

        g_camPos += step;
        float4x4 g_rayMatrix = mul(rotate_Y_4x4(-cam_rot[1]), rotate_X_4x4(+cam_rot[0]));
        g_rayMatrix.M(3, 0) = g_camPos.x;
        g_rayMatrix.M(3, 1) = g_camPos.y;
        g_rayMatrix.M(3, 2) = g_camPos.z;
        program.SetUniform("g_rayMatrix", g_rayMatrix);

        program.SetUniform("g_screenWidth", WIDTH);
        program.SetUniform("g_screenHeight", HEIGHT);


        // очистка и заполнение экрана цветом
        //
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw call
        //
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        GL_CHECK_ERRORS;  // The last parameter of glDrawArrays is equal to VS invocations

        program.StopUseShader();

        glfwSwapBuffers(window);
        i = (i + 1) % 100;

        // Print fps
        if (i == 0) {
            next_time = glfwGetTime();
            const double elapsed_time = next_time - current_time;
            current_time = next_time;
            const std::string title = "RayTrace task -- FPS " + std::to_string(int(100 / elapsed_time));
            glfwSetWindowTitle(window, title.c_str());
        }
    }

    //очищаем vboи vao перед закрытием программы
    //
    glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteBuffers(1, &g_vertexBufferObject);

    glfwTerminate();
    return 0;
}
