#ifndef DRAW_H
#define DRAW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "datapoint.h"

void drawDtInt(grp* dtgrp);
void drawDtFloat(grp* dtgrp);
void drawDtString(grp* dtgrp);
void drawDt2dLine(grp* dtgrp);
void drawDt3dLine(grp* dtgrp);
void drawDt4dLine(grp* dtgrp);
void drawDtColor(grp* dtgrp);

#endif
