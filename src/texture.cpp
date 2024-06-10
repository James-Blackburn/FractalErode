#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include "texture.hpp" 

Texture::Texture(const char* aSource){
    source = aSource;
    loadTexture();
}

int Texture::loadTexture()
{
    // load textures
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);

    // load and generate the texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load(source, &width, &height, &nrChannels, 0);
    int format = GL_RGB;
    if (nrChannels == 4){
        format = GL_RGBA;
    } 
    
    if (data)
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    else {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
    }

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float maxAnisotropy = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // unbind texture and free uneeded data
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    
    return 0;
}