#ifndef TEXTURE_HPP_INCLUDED
#define TEXTURE_HPP_INCLUDED

struct Texture{
    const char* source;
    unsigned int glID;

    Texture(const char*);
    int loadTexture();
    void clean();
};

#endif