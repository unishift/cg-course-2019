// Internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Object.h"

// External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

// Window size
static const GLsizei WIDTH = 640, HEIGHT = 480;

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

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

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
    shaders[GL_FRAGMENT_SHADER] = "shaders/fragment.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    // Create objects
    std::vector<Object> objects = {
        Object({
            0.5f,  0.5f, 0.0f,  // Верхний правый угол
            0.5f, -0.5f, 0.0f,  // Нижний правый угол
            -0.5f, -0.5f, 0.0f,  // Нижний левый угол
            -0.5f,  0.5f, 0.0f   // Верхний левый угол
        }, {
            0, 1, 3,
            1, 2, 3,
        }),
        Object({
            0.3f,  0.3f, 0.0f,  // Верхний правый угол
            0.3f, -0.7f, 0.0f,  // Нижний правый угол
            -0.7f, -0.7f, 0.0f,  // Нижний левый угол
            -0.7f,  0.3f, 0.0f   // Верхний левый угол
        }, {
            0, 1, 3,
            1, 2, 3,
        })
    };

    glfwSwapInterval(1); // force 60 frames per second

    // Prepare transformations
    const auto perspective = glm::perspective(glm::radians(45.0f), float(WIDTH) / HEIGHT, 0.1f, 100.0f);
    const auto view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    // Game loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const auto time = glfwGetTime();
        // Modify objects
        objects[0].move(cosf(time) * glm::vec3(0.0f, 0.01f, 0.0f));
        objects[0].rotate(0.1, glm::vec3(1.0f, 0.0f, 0.0f));

        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();
        GL_CHECK_ERRORS;

        // Draw objects
        for (const auto& object : objects) {
            const auto transform = perspective * view * object.getWorldTransform();
            program.SetUniform("transform", transform);
            object.draw();
        }

        program.StopUseShader();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
