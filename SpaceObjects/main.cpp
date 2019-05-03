// Internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "ModelFactories.h"

// External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>
#include <il.h>

// Window size
static const GLsizei WIDTH = 1280, HEIGHT = 720;

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

static double mx = 0;
static double my = 0;
constexpr float scale = 1.0f;
const glm::vec3 forward(0.0f, 0.0f, -scale);
const glm::vec3 left(-scale, 0.0f, 0.0f);
const glm::vec3 up(0.0f, scale, 0.0f);

// Settings
static bool permitMouseMove = false;

// Prepare transformations
const auto perspective = glm::perspective(glm::radians(45.0f), float(WIDTH) / HEIGHT, 0.1f, 1000.0f);
static glm::vec3 camera_position(0.0f, 0.0f, -3.0f);
static glm::mat4 camera_rot(1.0f);

// Callback for mouse movement
static void mouseMove(GLFWwindow *window, double xpos, double ypos) {
    auto x1 = float(0.01 * xpos);
    auto y1 = float(0.01 * ypos);

    if (permitMouseMove) {
        camera_rot = glm::rotate(glm::mat4(1.0f), float(my - y1), glm::vec3(1.0f, 0.0f, 0.0f)) *
                     glm::rotate(glm::mat4(1.0f), float(mx - x1), glm::vec3(0.0f, 1.0f, 0.0f)) * camera_rot;
    }

    mx = x1;
    my = y1;
}

// Callback for actions with mouse buttons
// Permit camera movements with left button pressed only
static void mouseButton(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
float multiplier = 0.1f;
glm::vec3 step = {0.0f, 0.0f, 0.0f};
float rot_step = 0.0f;
static void keyboardControls(GLFWwindow *window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_W:
            if (action == GLFW_PRESS) {
                step += forward;
            } else if (action == GLFW_RELEASE) {
                step -= forward;
            }
            break;
        case GLFW_KEY_A:
            if (action == GLFW_PRESS) {
                step += left;
            } else if (action == GLFW_RELEASE) {
                step -= left;
            }
            break;
        case GLFW_KEY_S:
            if (action == GLFW_PRESS) {
                step -= forward;
            } else if (action == GLFW_RELEASE) {
                step += forward;
            }
            break;
        case GLFW_KEY_D:
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
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        default:
            break;
    }
}

enum class ShaderType {
    CLASSIC,
    SKYBOX,
    PARTICLES_PASSIVE,
    PARTICLES_ACTIVE,
};

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
    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetKeyCallback(window, keyboardControls);

    if (initGL() != 0)
        return -1;

    ilInit();

    // Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    std::unordered_map<ShaderType, ShaderProgram> shader_programs;
    shader_programs[ShaderType::CLASSIC] = ShaderProgram({
        {GL_VERTEX_SHADER,   "shaders/classic_vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/classic_fragment.glsl"},
    });
    shader_programs[ShaderType::SKYBOX] = ShaderProgram({
        {GL_VERTEX_SHADER,   "shaders/skybox_vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/skybox_fragment.glsl"},
    });
    shader_programs[ShaderType::PARTICLES_PASSIVE] = ShaderProgram({
        {GL_VERTEX_SHADER,   "shaders/particles_passive_vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/particles_passive_fragment.glsl"},
    });
    shader_programs[ShaderType::PARTICLES_ACTIVE] = ShaderProgram({
        {GL_VERTEX_SHADER,   "shaders/particles_active_vertex.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/particles_active_geometry.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/particles_active_fragment.glsl"},
    });
    GL_CHECK_ERRORS;

    const auto skybox = SkyBox::create({
        "models/necro_nebula/GalaxyTex_PositiveX.png",
        "models/necro_nebula/GalaxyTex_NegativeX.png",
        "models/necro_nebula/GalaxyTex_NegativeY.png",
        "models/necro_nebula/GalaxyTex_PositiveY.png",
        "models/necro_nebula/GalaxyTex_PositiveZ.png",
        "models/necro_nebula/GalaxyTex_NegativeZ.png",
    });

    Particles particles(1000);

    std::vector<Model> models = {
        create_model(ModelName::E45_AIRCRAFT),
//        create_model(ModelName::ENTERPRISE_NCC1701D),
    };

    glfwSwapInterval(1); // force 60 frames per second

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Game loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glViewport(0, 0, WIDTH, HEIGHT);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        // Modify environment
        const auto time = glfwGetTime();

        camera_rot = glm::rotate(glm::mat4(1.0f), rot_step, {0.0f, 0.0f, 1.0f}) * camera_rot;
        const auto camera_position_diff = multiplier * glm::transpose(glm::mat3(camera_rot)) * step;
        camera_position -= camera_position_diff;

        const auto world_transform = glm::translate(camera_rot, camera_position);
        const auto view_transform = perspective * world_transform;


        // Draw skybox
        {
            auto& program = shader_programs[ShaderType::SKYBOX];

            glDepthMask(GL_FALSE);
            program.StartUseShader();

            const auto transform = perspective * camera_rot;
            program.SetUniform("transform", transform);

            skybox.draw();

            program.StopUseShader();
            glDepthMask(GL_TRUE);
        }

        // Draw particles
        {
            const bool passive = step == glm::vec3(0, 0, 0);
            auto& program = passive ? shader_programs[ShaderType::PARTICLES_PASSIVE] : shader_programs[ShaderType::PARTICLES_ACTIVE];

            program.StartUseShader();

            program.SetUniform("world_transform", glm::translate(glm::mat4(1.0f), camera_position));
            program.SetUniform("perspective_transform", perspective * camera_rot);

            if (!passive) {
                program.SetUniform("velocity", camera_position_diff);
            }

            particles.draw();

            program.StopUseShader();
        }

        // Draw objects
        {
            auto& program = shader_programs[ShaderType::CLASSIC];

            // Modify objects
            //...

            program.StartUseShader();
            GL_CHECK_ERRORS;

            // Draw objects
            for (const auto &model : models) {
                const auto local = view_transform * model.getWorldTransform();
                for (const auto &object : model.objects) {
                    const auto transform = local * object.getWorldTransform();
                    program.SetUniform("transform", transform);

                    const auto color = object.getDiffuseColor();
                    program.SetUniform("diffuse_color", color);

                    const bool use_texture = object.haveTexture();
                    program.SetUniform("use_texture", use_texture);

                    const float opacity = object.getOpacity();
                    program.SetUniform("opacity", opacity);

                    object.draw();
                    GL_CHECK_ERRORS;
                }
            }

            program.StopUseShader();
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
