#include "texture_loader.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "stb_image.h"

#include <fstream>
#include <vector>

namespace TextureLoader {

unsigned int LoadPngAsTexture(const fs::path& file, int& outW, int& outH) {
    std::error_code ec;
    if (!fs::exists(file, ec)) return 0;

    std::ifstream f(file, std::ios::binary);
    if (!f.is_open()) return 0;

    std::vector<unsigned char> buf((std::istreambuf_iterator<char>(f)),
                                     std::istreambuf_iterator<char>());
    if (buf.empty()) return 0;

    int w = 0, h = 0, channels = 0;
    unsigned char* pixels = stbi_load_from_memory(buf.data(), (int)buf.size(),
                                                    &w, &h, &channels, 4 /* force RGBA */);
    if (!pixels) return 0;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    stbi_image_free(pixels);

    outW = w;
    outH = h;
    return (unsigned int)tex;
}

}
