// GLEngine by Joshua Senouf - 2016
// Credits to Joey de Vries (LearnOpenGL) and Kevin Fung (Glitter)


#include "shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

//---------------
// GLFW Callbacks
//---------------

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

//---------------------
// Functions prototypes
//---------------------
void imGuiSetup();

//---------------------------------
// Variables & objects declarations
//---------------------------------
bool guiIsOpen = true;
GLfloat WIDTH = 512.f;
GLfloat HEIGHT = 512.f;

GLfloat zNear = 0.1f;
GLfloat zFar = 100.f;

glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
glm::mat4 projection = glm::perspective(glm::radians(90.0f), WIDTH / HEIGHT, zNear, zFar);

auto worldToCamera = projection * view;
auto cameraToWorld = glm::inverse(worldToCamera);

glm::vec3 screenSpacePos = glm::vec3(WIDTH/2.f, HEIGHT/2.f, 0.f);


GLfloat quadVertices[] = {
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};


int main(int argc, char* argv[])
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* glfwMode = glfwGetVideoMode(glfwMonitor);

    glfwWindowHint(GLFW_RED_BITS, glfwMode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, glfwMode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, glfwMode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, glfwMode->refreshRate);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Perspective Test", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    gladLoadGL();

    glViewport(0, 0, WIDTH, HEIGHT);

    ImGui_ImplGlfwGL3_Init(window, true);
    glfwSetKeyCallback(window, key_callback);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    Shader shader;
    //Shape render;
    shader.setShader("simple.vert", "simple.frag");
    shader.useShader();

    GLuint shapeVAO, shapeVBO;

    glGenVertexArrays(1, &shapeVAO);
    glGenBuffers(1, &shapeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindVertexArray(shapeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);
    
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(glGetUniformLocation(shader.Program, "lightColor"), 1, glm::value_ptr(glm::vec4(0.f, 1.f, 0.f, 1.f)));


    do {
        glm::vec3 normScreenspace = glm::vec3(
            screenSpacePos.x / WIDTH * 2.f - 1.f,
            screenSpacePos.y / HEIGHT * 2.f - 1.f,
            screenSpacePos.z
        );

        // screen space to word space
        auto worldPos = glm::vec4(normScreenspace.x, normScreenspace.y, normScreenspace.z, 1.f) * cameraToWorld;

        // reverse perspective division
        worldPos.w = 1.0 / worldPos.w;
        worldPos.x *= worldPos.w;
        worldPos.y *= worldPos.w;
        worldPos.z *= worldPos.w;


        glClear(GL_COLOR_BUFFER_BIT);

        imGuiSetup();
        shader.useShader();


        glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(worldPos));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // draw quad
        glBindVertexArray(shapeVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        ImGui::Render();
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    //---------
    // Cleaning
    //---------
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}



void imGuiSetup()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::Begin("GLEngine", &guiIsOpen, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoSavedSettings);
    ImGui::SetWindowSize(ImVec2(350, HEIGHT));

    if (ImGui::CollapsingHeader("Rendering", 0, true, true))
    {
        ImGui::SliderFloat("x (screen)", (float*)&screenSpacePos.x, 0, WIDTH);
        ImGui::SliderFloat("y (screen)", (float*)&screenSpacePos.y, 0, HEIGHT);
        ImGui::SliderFloat("z", (float*)&screenSpacePos.z, 0.f, zFar);
    }

    if (ImGui::CollapsingHeader("Application Info", 0, true, true))
    {
        char* glInfos = (char*)glGetString(GL_VERSION);
        char* hardwareInfos = (char*)glGetString(GL_RENDERER);

        ImGui::Text("OpenGL Version :");
        ImGui::Text(glInfos);
        ImGui::Text("Hardware Informations :");
        ImGui::Text(hardwareInfos);
        ImGui::Text("\nFramerate %.2f FPS / Frametime %.4f ms", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    }

    ImGui::End();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}