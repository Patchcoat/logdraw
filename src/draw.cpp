#include <stdio.h>
#include <iostream>
#include "draw.h"

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

// TODO finish this
void drawDtFloatLine(grp* dtgrp) {
    printf("Start\n");

    unsigned int grpSize = dtgrp->dt.used;
    unsigned int vertSize = 3 * grpSize;
    float vertices[vertSize];
    unsigned int indices[grpSize*2];

    int time_front = dtgrp->dt.array[0].time.time;

    for (int i = 0; i < grpSize; i++) {
        float x = dtgrp->dt.array[i].data.f;
        int element = dtgrp->dt.array[i].time.time - time_front;
        float y = static_cast<float>(element);
        vertices[i*3] = x;
        vertices[i*3+1] = y;
        vertices[i*3+2] = 0;
        indices[i*2] = i;
        indices[i*2] = i+1;
    }

    unsigned int currentPoint = 0;

    cameraPos = glm::vec3(vertices[0], vertices[1], vertices[2] + 1);

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

    // mouse callbacks
    // TODO fix these to work in 2d
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
    
    // enable depth test
    glEnable(GL_DEPTH_TEST);

    // model matrix
    glm::mat4 model = glm::mat4(1.0f);
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

        glBindVertexArray(0);

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    printf("End\n");
}

// TODO finish
void drawDt2dLine(grp* dtgrp) {
    printf("Start\n");

    unsigned int grpSize = dtgrp->dt.used;
    unsigned int vertSize = 3 * grpSize;
    float vertices[vertSize];
    unsigned int indices[grpSize*2];

    for (int i = 0; i < grpSize; i++) {
        float x = dtgrp->dt.array[i].data.vec2d.x;
        float y = dtgrp->dt.array[i].data.vec2d.y;
        vertices[i*3] = x;
        vertices[i*3+1] = y;
        vertices[i*3+2] = 0;
        indices[i*2] = i;
        indices[i*2] = i+1;
    }

    unsigned int currentPoint = 0;

    unsigned int circleVertCount = 36;
    float circle[(circleVertCount+2)*3];
    
    // start the circle at the center
    circle[0] = 0.0f;
    circle[1] = 0.0f;
    circle[2] = 0.0f;

    // generate the circle
    for (int i = 1; i < circleVertCount; i++) {
        circle[i*3] = sin(glm::radians(360.0f/(circleVertCount)));
        circle[i*3+1] = cos(glm::radians(360.0f/(circleVertCount)));
        circle[i*3+2] = 0;
    }

    cameraPos = glm::vec3(vertices[0], vertices[1], vertices[2] + 1);
    
    float axis[] = {
        -1000.0f,     0.0f, 0.0f,
         1000.0f,     0.0f, 0.0f,
            0.0f, -1000.0f, 0.0f,
            0.0f,  1000.0f, 0.0f
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

    // mouse callbacks
    // TODO fix these to work in 2d
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
    
    unsigned int VBOCircle;
    glGenBuffers(1, &VBOCircle);

    unsigned int VAOCircle;
    glBindBuffer(GL_ARRAY_BUFFER, VBOCircle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAOCircle);
    glBindVertexArray(VAOCircle);

    //unsigned int EBOSphere;
    //glGenBuffers(1, &EBOSphere);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOSphere);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circleIndices), circleIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // enable depth test
    glEnable(GL_DEPTH_TEST);

    // model matrix
    glm::mat4 model = glm::mat4(1.0f);
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

        // draw the circle
        glBindVertexArray(VAOCircle);
        glm::mat4 circleModel = glm::mat4(1.0f);
        circleModel = glm::translate(circleModel, glm::vec3(vertices[currentPoint*3],vertices[currentPoint*3+1],vertices[currentPoint*3+2]));
        circleModel = glm::scale(circleModel, glm::vec3(0.1f,0.1f,0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(circleModel));
        glUniform3f(colorIn, 1.0f, 1.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertCount);
        //glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    printf("End\n");
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
    
    float t = 1.618033988749895;
    float sphere[] = {
        -1,  t,  0,
         1,  t,  0,
        -1, -t,  0,
         1, -t,  0,
         0, -1,  t,
         0,  1,  t,
         0, -1, -t,
         0,  1, -t,
         t,  0, -1,
         t,  0,  1,
        -t,  0, -1,
        -t,  0,  1
    };
    unsigned int sphereIndices[] = {
        0,11,5,
        0,5,1,
        0,1,7,
        0,7,10,
        0,10,11,
        1,5,9,
        5,11,4,
        11,10,2,
        10,7,6,
        7,1,8,
        3,9,4,
        3,4,2,
        3,2,6,
        3,6,8,
        3,8,9,
        4,9,5,
        2,4,11,
        6,2,10,
        8,6,7,
        9,8,1
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
        glm::mat4 sphereModel = glm::mat4(1.0f);
        sphereModel = glm::translate(sphereModel, glm::vec3(vertices[currentPoint*3],vertices[currentPoint*3+1],vertices[currentPoint*3+2]));
        sphereModel = glm::scale(sphereModel, glm::vec3(0.1f,0.1f,0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphereModel));
        glUniform3f(colorIn, 1.0f, 1.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    printf("End\n");
}
