#pragma once
// Standard libraries

// External libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Internal libraries
#include <log.h>
#include <shader_program.h>
#include <buffer.h>
#include <framebuffer.h>
#include <texture.h>
#include "simulation.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

class Application
{
public:
    // Flag that signals that the application has been initialised correctly.
    bool isInitialised = false;
    int swapFields = 0;

    // Constructor
    Application(int width, int height, const char *title);
    ~Application();

    // Run loop
    void run();
    // Render method
    void onRender();
    // ImGui render methods
    void beginImGuiFrame();
    void onImGuiRender();
    void endImGuiFrame();
    // Update method (frame rate independent)
    void onUpdate();

private:
    GLFWwindow *m_windowHandle = nullptr;
    ImGuiIO *m_imguiIO = nullptr;
    Framebuffer *m_framebuffer = nullptr;

    // Timing information
    double m_lastUpdateTime = 0.0f;
    double m_lastFrameTime = 0.0f;

    double m_tickRate = 0.0f;

    // Frame rate
    double m_fpsLimit = 1.0f / 60.0f;

    // Rendering
    VertexFragmentShaderProgram *m_textureProgram;
    VertexArray *m_mainViewportVertexArray;

    Simulation *m_simulation;

    std::shared_ptr<Texture2D> m_colorMap;
};
