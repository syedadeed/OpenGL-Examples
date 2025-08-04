#include "../include/opengl_utils.h"

static char* get_file_content(const char* file_path){
    FILE* file = fopen(file_path, "rb");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char* content = (char*)malloc((file_size + 1) * sizeof(char));
    assert(content != NULL);
    unsigned long bytes_read = fread(content, 1, file_size, file);
    assert(bytes_read == file_size);
    content[bytes_read] = '\0';
    fclose(file);
    return content;
}

static int shader_compile_err_check(GLuint shader){
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success){
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("%s\n", info_log);
        return 1;
    }
    return 0;
}

GLuint uglCreateProgram(const char* vertex_shader_path, const char* fragment_shader_path){
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_shader_content = get_file_content(vertex_shader_path);
    glShaderSource(vertex_shader, 1, &vertex_shader_content, NULL);
    glCompileShader(vertex_shader);
    assert(shader_compile_err_check(vertex_shader) == 0);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);;
    const char* fragment_shader_content = get_file_content(fragment_shader_path);
    glShaderSource(fragment_shader, 1, &fragment_shader_content, NULL);
    glCompileShader(fragment_shader);
    assert(shader_compile_err_check(fragment_shader) == 0);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    int success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success){
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        printf("%s\n", info_log);
        assert(0 == 1);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    free((void*)vertex_shader_content);
    free((void*)fragment_shader_content);

    return shader_program;
}
