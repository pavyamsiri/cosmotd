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
    // Simulation update method (frame rate determined by user)
    void onSimulationUpdate();

private:
    GLFWwindow *m_WindowHandle = nullptr;
    ImGuiIO *m_ImGuiIO = nullptr;
    Framebuffer *m_Framebuffer = nullptr;

    // Timing information
    double m_lastUpdateTime = 0.0f;
    double m_lastFrameTime = 0.0f;
    double m_lastSimUpdateTime = 0.0f;

    double m_tickRate = 0.0f;

    // Frame rate
    float m_fpsLimit = 60.0f;
    // Simulation frame rate
    float m_simulationFPS = 60.0f;

    // Rendering
    VertexFragmentShaderProgram *m_PlotFieldProgram;
    VertexFragmentShaderProgram *m_PlotPhaseProgram;
    VertexFragmentShaderProgram *m_PlotStringsProgram;
    VertexArray *m_MainViewportVertexArray;

    Simulation *m_Simulation;

    Texture2D *m_colorMap;
    Texture2D *m_discreteColorMap;

    const char *m_currentPlottingProcedure = "Smooth Phase";
    size_t m_currentPlottingProcedureIndex = 2;

    const char *m_currentSimulationProcedure = "Domain walls";
};
