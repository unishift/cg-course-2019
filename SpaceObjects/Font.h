#ifndef SPACEOBJECTS_FONT_H
#define SPACEOBJECTS_FONT_H

#include <array>
#include <glad/glad.h>
#include <glm/vec2.hpp>

class Font {
    struct Character {
        GLuint texture_id;
        GLuint shift;
        glm::vec2 size;
        glm::vec2 offset;
    };

    std::array<Character, 128> chars;
    GLuint VAO, VBO;
public:
    explicit Font(const std::string& path);

    void draw(const std::string& text) const;

};

#endif //SPACEOBJECTS_FONT_H
