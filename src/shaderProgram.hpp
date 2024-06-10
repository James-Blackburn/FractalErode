#ifndef SHADERPROGRAM_HPP_INCLUDED
#define SHADERPROGRAM_HPP_INCLUDED

#include <vector>

struct Shader{
    const char* source;
    unsigned int type;
    unsigned int glID;
};

struct ShaderProgram{
    std::vector<Shader> shaders;
    unsigned int glID;
    
    ShaderProgram() = default;
    ShaderProgram(std::vector<Shader>);
    ~ShaderProgram();
    int create();
    void clean();
};

#endif