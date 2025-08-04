#pragma once
#include "glad.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

GLuint uglCreateProgram(const char* vertex_shader_path, const char* fragment_shader_path);
