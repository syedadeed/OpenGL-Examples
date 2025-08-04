#include "engine/client.h"
void Init(int width, int height){}
int initd = 0;
GLuint vao = 0;
GLuint shader_program = 0;

void init(){
    GLfloat vertices[] = {
        //coords                   color
        +0.000f, +0.000f, +0.000f, +0.0, +0.0, +0.0,
        +0.500f, +0.000f, +0.000f, +1.0, +0.0, +0.0,
        +0.250f, +0.433f, +0.000f, +0.0, +1.0, +0.0,
        -0.250f, +0.433f, +0.000f, +0.0, +0.0, +1.0, 
        -0.500f, +0.000f, +0.000f, +1.0, +0.0, +0.0,
        -0.250f, -0.433f, +0.000f, +0.0, +1.0, +0.0,
        +0.250f, -0.433f, +0.000f, +0.0, +0.0, +1.0,
    }; //used polar coords to calc these coordinates of hexagon
    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 5,
        0, 5, 6,
        0, 6, 1
    };
    GLuint vbo = 0;
    GLuint ebo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    shader_program = uglCreateProgram("/home/adeed/Downloads/Chaos/shaders/vertex.glsl", "/home/adeed/Downloads/Chaos/shaders/fragment.glsl");
}

void Render(int width, int height){
    if(!initd){
        init();
        initd = 1;
    }
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(vao);
    glUseProgram(shader_program);
    glUniform1f(glGetUniformLocation(shader_program, "alpha"), 1.0);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
}
