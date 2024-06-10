#include "shaderProgram.hpp"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>

ShaderProgram::ShaderProgram(std::vector<Shader> aShaders){
    shaders = aShaders;
    create();
}

ShaderProgram::~ShaderProgram() {
    clean();
}

void ShaderProgram::clean() {
    glDeleteProgram(glID);
}

int ShaderProgram::create(){
    // load and compile shaders
    for (Shader& shader : shaders){
        // load shader
        std::string line, text;
        std::ifstream file(shader.source);
        while(std::getline(file, line)){
            text += line + "\n";
        }
        const char* data = text.c_str();

        // compile shader
        shader.glID = glCreateShader(shader.type);
        glShaderSource(shader.glID, 1, &data, NULL);
        glCompileShader(shader.glID);

        // check for opengl errors after compiling shader
        int success_status;
        char error_data[512];
        glGetShaderiv(shader.glID, GL_COMPILE_STATUS, &success_status);
        if(!success_status){
            glGetShaderInfoLog(shader.glID, 512, NULL, error_data);
            std::cout << "Error in shader: " << shader.source << std::endl;
            std::cout << "Shader failed to compile: " << error_data << std::endl;
            return -1;
        }
    }

    // link program
    glID = glCreateProgram();
    for (Shader& shader : shaders){
        glAttachShader(glID, shader.glID);
    }
    glLinkProgram(glID);
    
    // check for opengl errors after linking shader program
    int success_status;
    char error_data[512];
    glGetProgramiv(glID, GL_LINK_STATUS, &success_status);
    if(!success_status){
        glGetProgramInfoLog(glID, 512, NULL, error_data);
        std::cout << "Shader failed to link: " << error_data << std::endl;
        return -1;
    }

    // discard uneeded shader objects
    for (Shader& shader : shaders){
        glDeleteShader(shader.glID);
    }

    return 0;
}