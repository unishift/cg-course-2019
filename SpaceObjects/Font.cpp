#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>

Font::Font(const std::string &path) {
    FT_Library ft_lib;
    const auto res_lib = FT_Init_FreeType(&ft_lib);
    if (res_lib != 0) {
        std::cerr << "Error initializing freetype" << std::endl;
    }

    FT_Face ft_face;
    const auto res_face = FT_New_Face(ft_lib, path.c_str(), 0, &ft_face);
    if (res_face != 0) {
        std::cerr << "Error loading font" << std::endl;
    }

    FT_Set_Pixel_Sizes(ft_face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char ch = 0; ch < 128; ch++) {
        const auto res_char = FT_Load_Char(ft_face, ch, FT_LOAD_RENDER);
        if (res_char != 0) {
            std::cerr << "Error loading character" << std::endl;
        }

        const auto width = ft_face->glyph->bitmap.width;
        const auto height = ft_face->glyph->bitmap.rows;
        const auto data = ft_face->glyph->bitmap.buffer;
        const GLuint shift = ft_face->glyph->advance.x;
        const auto offset_x = ft_face->glyph->bitmap_left;
        const auto offset_y = ft_face->glyph->bitmap_top;

        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        chars[ch] = {texture_id, shift, {width, height}, {offset_x, offset_y}};
    }

    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void Font::draw(const std::string &text) const {
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);

    float cursor = 0.0f;
    for (const auto& sym : text) {
        const auto ch = chars[sym];

        const auto texture_id = ch.texture_id;
        const auto width = ch.size.x;
        const auto height = ch.size.y;

        const auto xpos = cursor + ch.offset.x;
        const auto ypos = ch.offset.y - ch.size.y;

        const GLfloat vertices[] = {
            xpos, ypos + height, 0.0, 0.0,
            xpos, ypos, 0.0, 1.0f,
            xpos + width, ypos, 1.0f, 1.0f,

            xpos, ypos + height, 0.0, 0.0,
            xpos + width, ypos, 1.0, 1.0f,
            xpos + width, ypos + height, 1.0f, 0.0f,
        };

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        cursor += ch.shift >> 6;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
