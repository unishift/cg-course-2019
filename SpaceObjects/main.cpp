// Internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "ModelFactories.h"
#include "Camera.h"

// External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>
#include <il.h>
#include <glm/gtx/vector_angle.hpp>
#include <list>

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
//const auto perspective = glm::perspective(glm::radians(45.0f), float(WIDTH) / HEIGHT, 0.1f, 1000.0f);
const auto perspective = glm::infinitePerspective(glm::radians(45.0f), float(WIDTH) / HEIGHT, 0.1f);
//static glm::vec3 camera_position(0.0f, 0.0f, -3.0f);
//static glm::mat4 camera_rot(1.0f);

static float yaw = 0.0;
static float pitch = 0.0;
// Callback for mouse movement
static void mouseMove(GLFWwindow *window, double xpos, double ypos) {
    auto x1 = float(0.01 * xpos);
    auto y1 = float(0.01 * ypos);

    if (permitMouseMove) {
        yaw = glm::clamp(yaw + float(my - y1), -M_PI_2f32, M_PI_2f32);
        pitch += mx - x1;
//        camera_rot = glm::rotate(glm::mat4(1.0f), float(my - y1), glm::vec3(1.0f, 0.0f, 0.0f)) *
//                     glm::rotate(glm::mat4(1.0f), float(mx - x1), glm::vec3(0.0f, 1.0f, 0.0f)) * camera_rot;
    }

    mx = x1;
    my = y1;
}

static bool shoot = false;
// Callback for actions with mouse buttons
// Permit camera movements with left button pressed only
static void mouseButton(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            permitMouseMove = true;

        } else if (action == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            permitMouseMove = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
           shoot = true;
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
CameraMode camera_mode = CameraMode::THIRD_PERSON;
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
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            if (action == GLFW_PRESS) {
                multiplier *= 2;
            } else if (action == GLFW_RELEASE) {
                multiplier /= 2;
            }
            break;
        case GLFW_KEY_F2:
            if (action == GLFW_PRESS) {
                camera_mode = CameraMode::FIRST_PERSON;
            }
            break;
        case GLFW_KEY_F3:
            if (action == GLFW_PRESS) {
                camera_mode = CameraMode::THIRD_PERSON;
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
    CROSSHAIR,
};

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    glViewport(0, 0, WIDTH, HEIGHT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    GL_CHECK_ERRORS;

    std::cout << "Initializing environment... ";
    std::cout << "\x1b[32mDone\x1b[0m" << std::endl;

    std::cout << "Compiling shaders... ";

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
    shader_programs[ShaderType::CROSSHAIR] = ShaderProgram({
        {GL_VERTEX_SHADER,   "shaders/crosshair_vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/crosshair_fragment.glsl"},
    });
    GL_CHECK_ERRORS;

    std::cout << "\x1b[32mDone\x1b[0m" << std::endl;

    Camera camera;

    std::cout << "Loading skybox... ";

    const auto skybox = SkyBox::create({
        "models/necro_nebula/GalaxyTex_PositiveX.png",
        "models/necro_nebula/GalaxyTex_NegativeX.png",
        "models/necro_nebula/GalaxyTex_NegativeY.png",
        "models/necro_nebula/GalaxyTex_PositiveY.png",
        "models/necro_nebula/GalaxyTex_PositiveZ.png",
        "models/necro_nebula/GalaxyTex_NegativeZ.png",
    });

    std::cout << "\x1b[32mDone\x1b[0m" << std::endl;

    Particles particles(1000);

    Crosshair crosshair;

    std::cout << "Loading models... ";

    ModelFactory model_factory;

    std::cout << "\x1b[32mDone\x1b[0m" << std::endl;

    float main_ship_hp = 100.0;
    auto main_ship = model_factory.get_model(ModelName::E45_AIRCRAFT, {0.0f, -3.0f, -3.0f}, {0.0f, M_PI, 0.0f});

    std::list<Model> enemies;

    glfwSwapInterval(1); // force 60 frames per second

    glm::vec3 smooth_step(0.0f);
    const glm::vec3 enemies_speed(0.0f, 0.0f, 0.5f);
    glm::vec3 particles_state(0.0f, 0.0f, 0.0f);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Game loop
    while (!glfwWindowShouldClose(window) && main_ship_hp > 0.0f) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        // Modify environment
        const auto time = glfwGetTime();

        smooth_step += 0.05f * (step - smooth_step);
        const auto camera_shift = multiplier * smooth_step;
        camera.mode = camera_mode;
        camera.rot = glm::quat({yaw, pitch, 0.0f});
        camera.move(camera_shift);

        const auto view_transform = camera.getViewTransform();
        const auto perspective_transform = perspective * view_transform;

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Kill enemies
        if (shoot) {

            for (auto it = enemies.begin(); it != enemies.end(); it++) {
                const auto model = view_transform * it->getWorldTransform();
                const glm::vec4 view_port(0.0f, 0.0f, WIDTH, HEIGHT);
                const auto bbox_min = glm::project(it->bbox.min, model, perspective, view_port);
                const auto bbox_max = glm::project(it->bbox.max, model, perspective, view_port);

                const float min_x = glm::min(bbox_min.x, bbox_max.x);
                const float min_y = glm::min(bbox_min.y, bbox_max.y);
                const float max_x = glm::max(bbox_min.x, bbox_max.x);
                const float max_y = glm::max(bbox_min.y, bbox_max.y);
                if (xpos >= min_x && xpos <= max_x &&
                    HEIGHT - ypos >= min_y && HEIGHT - ypos <= max_y) {

                    enemies.erase(it);
                    break;
                }
            }
            shoot = false;
        }

        // Draw skybox
        {
            auto& program = shader_programs[ShaderType::SKYBOX];

            glDepthMask(GL_FALSE);
            program.StartUseShader();

            const auto transform = perspective * glm::mat4(glm::mat3(view_transform));
            program.SetUniform("transform", transform);

            skybox.draw();

            program.StopUseShader();
            glDepthMask(GL_TRUE);
        }

        // Draw particles
        {
            auto& program = shader_programs[ShaderType::PARTICLES_ACTIVE];

            particles_state += enemies_speed;

            program.StartUseShader();

            program.SetUniform("world_transform", glm::translate(glm::mat4(1.0f), particles_state - camera.position));
            program.SetUniform("perspective_transform", perspective * glm::mat4(glm::mat3(view_transform)));

            program.SetUniform("velocity", enemies_speed - camera_shift);

            particles.draw();

            program.StopUseShader();
        }

        // Draw objects
        {
            auto& program = shader_programs[ShaderType::CLASSIC];

            // Modify objects
            main_ship.move(camera_shift);

            if (rand() % 300 == 0) {
                const float x = rand() % 50 - 25;
                const float y = rand() % 50 - 25;
                enemies.push_back(model_factory.get_model(ModelName::REPVENATOR, {x, y, -200.0f}, glm::vec3(0.0f), 1.0f));
            }

            for (auto it = enemies.begin(); it != enemies.end(); it++) {
                it->move(enemies_speed);
                if (it->world_pos.z > 200.0f) {
                    enemies.erase(it--);
                    continue;
                }

                if (intersect(main_ship.getBBox(), it->getBBox())) {
                    main_ship_hp -= it->damage;
                    enemies.erase(it--);
                    continue;
                }
            }


            program.StartUseShader();
            GL_CHECK_ERRORS;

            {
                const auto local = perspective_transform * main_ship.getWorldTransform();
                for (const auto &object : main_ship.objects) {
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

            // Draw objects
            for (const auto &model : enemies) {
                const auto local = perspective_transform * model.getWorldTransform();
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
            std::cout << "Health Points: " << main_ship_hp << '\r' << std::flush;
        }

        // Draw crosshair
        {
            auto& program = shader_programs[ShaderType::CROSSHAIR];

            program.StartUseShader();

            program.SetUniform("position", glm::vec2(2.0 * xpos / WIDTH - 1.0, -2.0 * ypos / HEIGHT + 1.0));

            crosshair.draw();

            program.StopUseShader();
        }

        glfwSwapBuffers(window);

    }
    std::cout << "\nGame Over!" << std::endl;

    glfwTerminate();
    return 0;
}
