#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <GL/glew.h>
#include "shader.hpp"

using namespace std;

GLuint load_Shader(const char* vertex_file_path, const char* fragment_file_path)
{
    // Create the shaders
    GLuint v_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint f_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (!VertexShaderStream.is_open()) {
        printf("Error: Unable to open vertex shader file: %s\n", vertex_file_path);
        return 0;
    }
    std::string Line;
    while (getline(VertexShaderStream, Line))
        VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    else {
        printf("Error: Unable to open fragment shader file: %s\n", fragment_file_path);
        return 0;
    }

    GLint final = GL_FALSE;
    int log;

    // Compile Vertex Shader
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(v_id, 1, &VertexSourcePointer, nullptr);
    glCompileShader(v_id);

    // Check Vertex Shader
    glGetShaderiv(v_id, GL_COMPILE_STATUS, &final);
    glGetShaderiv(v_id, GL_INFO_LOG_LENGTH, &log);
    if (log > 0) {
        std::vector<char> VertexShaderErrorMessage(log + 1);
        glGetShaderInfoLog(v_id, log, nullptr, &VertexShaderErrorMessage[0]);
        printf("Vertex Shader Compilation Error: %s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(f_id, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(f_id);

    // Check Fragment Shader
    glGetShaderiv(f_id, GL_COMPILE_STATUS, &final);
    glGetShaderiv(f_id, GL_INFO_LOG_LENGTH, &log);
    if (log > 0) {
        std::vector<char> FragmentShaderErrorMessage(log + 1);
        glGetShaderInfoLog(f_id, log, nullptr, &FragmentShaderErrorMessage[0]);
        printf("Fragment Shader Compilation Error: %s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    GLuint ID_prog = glCreateProgram();
    glAttachShader(ID_prog, v_id);
    glAttachShader(ID_prog, f_id);
    glLinkProgram(ID_prog);

    // Check the program
    glGetProgramiv(ID_prog, GL_LINK_STATUS, &final);
    glGetProgramiv(ID_prog, GL_INFO_LOG_LENGTH, &log);
    if (log > 0) {
        std::vector<char> ProgramErrorMessage(log + 1);
        glGetProgramInfoLog(ID_prog, log, nullptr, &ProgramErrorMessage[0]);
        printf("Program Linking Error: %s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ID_prog, v_id);
    glDetachShader(ID_prog, f_id);

    glDeleteShader(v_id);
    glDeleteShader(f_id);

    return ID_prog;
}
