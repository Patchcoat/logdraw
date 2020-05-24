#include <stdio.h>
#include <iostream>
#include "draw.h"

// TODO
// timestamp and value stap for dot
// Previous  Current    Next
// Value     Value      Value
// Time      Time       Time

// window
int globalHeight = 800;
int globalWidth = 600;
int mousex = 0;
int mousey = 0;
// camera
glm::vec3 cameraPos   = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 2.5f;
// dleta time
float deltaTime = 0.0f;
float lastFrame = 0.0f;
// mouse movement
float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
bool canMove = false;
// scroll
float fov = 60.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    mousex = xpos/globalWidth;
    mousey = ypos/globalHeight;

    if (!canMove) {
        return;
    }
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraSpeed += (float)yoffset;
    if (cameraSpeed < 1.0f)
        cameraSpeed = 1.0f;
    if (cameraSpeed > 90.0f)
        cameraSpeed = 90.0f;
    //fov -= (float)yoffset;
    //if (fov < 1.0f)
    //    fov = 1.0f;
    //if (fov > 90.0f)
    //    fov = 90.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    globalHeight = height;
    globalWidth = width;
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        canMove = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        canMove = false;
        firstMouse = true;
    }
}

void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);

    // camera control
    const float cameraMove = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraMove * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraMove * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMove;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMove;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos += cameraUp * cameraMove;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos -= cameraUp * cameraMove;
}

void drawDt3dLine(grp* dtgrp){
    printf("Start\n");

    unsigned int grpSize = dtgrp->dt.used;
    unsigned int vertSize = 3 * grpSize;
    float vertices[vertSize];
    unsigned int indices[grpSize*2];

    for (int i = 0; i < grpSize; i++) {
        float x = dtgrp->dt.array[i].data.vec3d.x;
        float y = dtgrp->dt.array[i].data.vec3d.y;
        float z = dtgrp->dt.array[i].data.vec3d.z;
        vertices[i*3] = x;
        vertices[i*3+1] = y;
        vertices[i*3+2] = z;
        indices[i*2] = i;
        indices[i*2+1] = i+1;
    }

    unsigned int currentPoint = 0;

    cameraPos = glm::vec3(vertices[0], vertices[1], vertices[2] + 1);

    float axis[] = {
        -1000.0f,     0.0f,     0.0f,
         1000.0f,     0.0f,     0.0f,
            0.0f, -1000.0f,     0.0f,
            0.0f,  1000.0f,     0.0f,
            0.0f,     0.0f, -1000.0f,
            0.0f,     0.0f,  1000.0f,
    };
    
    unsigned int sphereIndices[] = {
        1,1,1,14,2,1,13,3,1,2,4,2,14,5,2,16,6,2,1,7,3,13,8,3,18,9,3,1,10,4,18,11,4,20,12,4,
        1,13,5,20,14,5,17,15,5,2,4,6,16,6,6,23,16,6,3,17,7,15,18,7,25,19,7,4,20,8,19,21,8,27,22,8,
        5,23,9,21,24,9,29,25,9,6,26,10,22,27,10,31,28,10,2,4,11,23,16,11,26,29,11,3,17,12,25,19,12,28,30,12,
        4,20,13,27,22,13,30,31,13,5,23,14,29,25,14,32,32,14,6,26,15,31,28,15,24,33,15,7,34,16,33,35,16,38,36,16,
        8,37,17,34,38,17,40,39,17,9,40,18,35,41,18,41,42,18,10,43,19,36,44,19,42,45,19,11,46,20,37,47,20,39,48,20,
        39,48,21,42,49,21,12,50,21,39,48,22,37,47,22,42,49,22,37,47,23,10,43,23,42,49,23,42,45,24,41,51,24,12,52,24,
        42,45,25,36,44,25,41,51,25,36,44,26,9,40,26,41,51,26,41,42,27,40,53,27,12,54,27,41,42,28,35,41,28,40,53,28,
        35,41,29,8,55,29,40,53,29,40,39,30,38,56,30,12,57,30,40,39,31,34,38,31,38,56,31,34,38,32,7,34,32,38,56,32,
        38,36,33,39,58,33,12,59,33,38,36,34,33,35,34,39,58,34,33,35,35,11,46,35,39,58,35,24,33,36,37,47,36,11,46,36,
        24,33,37,31,28,37,37,47,37,31,28,38,10,43,38,37,47,38,32,32,39,36,44,39,10,43,39,32,32,40,29,25,40,36,44,40,
        29,25,41,9,40,41,36,44,41,30,31,42,35,41,42,9,40,42,30,31,43,27,22,43,35,41,43,27,22,44,8,55,44,35,41,44,
        28,30,45,34,38,45,8,37,45,28,30,46,25,19,46,34,38,46,25,19,47,7,34,47,34,38,47,26,29,48,33,35,48,7,34,48,
        26,29,49,23,16,49,33,35,49,23,16,50,11,46,50,33,35,50,31,28,51,32,32,51,10,43,51,31,28,52,22,27,52,32,32,52,
        22,27,53,5,23,53,32,32,53,29,25,54,30,31,54,9,40,54,29,25,55,21,24,55,30,31,55,21,24,56,4,20,56,30,31,56,
        27,22,57,28,60,57,8,55,57,27,22,58,19,21,58,28,60,58,19,21,59,3,61,59,28,60,59,25,19,60,26,29,60,7,34,60,
        25,19,61,15,18,61,26,29,61,15,18,62,2,4,62,26,29,62,23,16,63,24,33,63,11,46,63,23,16,64,16,6,64,24,33,64,
        16,6,65,6,26,65,24,33,65,17,15,66,22,27,66,6,26,66,17,15,67,20,14,67,22,27,67,20,14,68,5,23,68,22,27,68,
        20,12,69,21,24,69,5,23,69,20,12,70,18,11,70,21,24,70,18,11,71,4,20,71,21,24,71,18,9,72,19,21,72,4,20,72,
        18,9,73,13,8,73,19,21,73,13,8,74,3,61,74,19,21,74,16,6,75,17,62,75,6,26,75,16,6,76,14,5,76,17,62,76,
        14,5,77,1,63,77,17,62,77,13,3,78,15,18,78,3,17,78,13,3,79,14,2,79,15,18,79,14,2,80,2,4,80,15,18,80
    };
    float sphere[] = {
        0.000000,-1.000000,0.000000,
        0.723607,-0.447220,0.525725,
        -0.276388,-0.447220,0.850649,
        -0.894426,-0.447216,0.000000,
        -0.276388,-0.447220,-0.850649,
        0.723607,-0.447220,-0.525725,
        0.276388,0.447220,0.850649,
        -0.723607,0.447220,0.525725,
        -0.723607,0.447220,-0.525725,
        0.276388,0.447220,-0.850649,
        0.894426,0.447216,0.000000,
        0.000000,1.000000,0.000000,
        -0.162456,-0.850654,0.499995,
        0.425323,-0.850654,0.309011,
        0.262869,-0.525738,0.809012,
        0.850648,-0.525736,0.000000,
        0.425323,-0.850654,-0.309011,
        -0.525730,-0.850652,0.000000,
        -0.688189,-0.525736,0.499997,
        -0.162456,-0.850654,-0.499995,
        -0.688189,-0.525736,-0.499997,
        0.262869,-0.525738,-0.809012,
        0.951058,0.000000,0.309013,
        0.951058,0.000000,-0.309013,
        0.000000,0.000000,1.000000,
        0.587786,0.000000,0.809017,
        -0.951058,0.000000,0.309013,
        -0.587786,0.000000,0.809017,
        -0.587786,0.000000,-0.809017,
        -0.951058,0.000000,-0.309013,
        0.587786,0.000000,-0.809017,
        0.000000,0.000000,-1.000000,
        0.688189,0.525736,0.499997,
        -0.262869,0.525738,0.809012,
        -0.850648,0.525736,0.000000,
        -0.262869,0.525738,-0.809012,
        0.688189,0.525736,-0.499997,
        0.162456,0.850654,0.499995,
        0.525730,0.850652,0.000000,
        -0.425323,0.850654,0.309011,
        -0.425323,0.850654,-0.309011,
        0.162456,0.850654,-0.499995
    };

    // Init Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create Window Object
    GLFWwindow* window = glfwCreateWindow(800, 600, "logdraw", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    // init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return;
    }

    // capture cursor
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // mouse callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // create viewport
    int width = 800;
    int height = 600;
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Shaders
    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 model;"
        "uniform mat4 view;"
        "uniform mat4 projection;"
        "void main(){\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
        "}\0";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec3 colorIn;\n"
        "void main(){\n"
        "    FragColor = vec4(colorIn, 1.0f);\n"
        "}\0";

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int colorIn = glGetUniformLocation(shaderProgram, "colorIn");

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::PROGRAM::LINK_FAILED\n%s\n", infoLog);
    }

    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int VAO;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int VBOAxis;
    glGenBuffers(1, &VBOAxis);

    unsigned int VAOAxis;
    glBindBuffer(GL_ARRAY_BUFFER, VBOAxis);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAOAxis);
    glBindVertexArray(VAOAxis);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int VBOSphere;
    glGenBuffers(1, &VBOSphere);

    unsigned int VAOSphere;
    glBindBuffer(GL_ARRAY_BUFFER, VBOSphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere), sphere, GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAOSphere);
    glBindVertexArray(VAOSphere);

    unsigned int EBOSphere;
    glGenBuffers(1, &EBOSphere);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOSphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices), sphereIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // enable depth test
    glEnable(GL_DEPTH_TEST);

    // model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // view matrix
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
    // projection matix
    glm::mat4 projection;

    // rendering loop
    while(!glfwWindowShouldClose(window)) {
        // process input
        processInput(window);
        // calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // rendering
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // shader program
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        // transforms
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(fov), (float)globalWidth/globalHeight, 0.1f, 100.0f);
        glUniform3f(colorIn, 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        // draw the graph
        glDrawElements(GL_LINES, grpSize*2-1, GL_UNSIGNED_INT, 0);

        // draw the axis
        glBindVertexArray(VAOAxis);
        glUniform3f(colorIn, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_LINES, 0, 6);

        // draw the sphere
        glBindVertexArray(VAOSphere);
        glUniform3f(colorIn, 1.0f, 1.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, 720, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    printf("End\n");
}
