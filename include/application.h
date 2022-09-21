#pragma once
// Standard libraries

// External libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Internal libraries
#include "buffer.h"
#include "framebuffer.h"
#include "log.h"
#include "shader_program.h"
#include "simulation.h"
#include "texture.h"

// The cosmotd application class. Handles the UI rendering and simulation.
class Application
{
public:
    // Signals that the application was successfully initialised if true.
    bool isInitialised = false;

    // Constructor
    Application(int width, int height, const char *title);
    // Releases all acquired resources
    ~Application();

    // Run loop
    void run();
    // Render method
    void onRender();
    // Update method (frame rate independent)
    void onUpdate();
    // Simulation update method (frame rate determined by user)
    void onSimulationUpdate();

    // Begins an ImGui frame
    void beginImGuiFrame();
    // Renders the UI
    void onImGuiRender();
    // Ends an ImGui frame
    void endImGuiFrame();

private:
    // Window handle
    GLFWwindow *m_WindowHandle = nullptr;
    // ImGui IO interface
    ImGuiIO *m_ImGuiIO = nullptr;
    // Framebuffer to draw simulation to
    Framebuffer *m_Framebuffer = nullptr;
    // Full size texture quad draw target parameters
    VertexArray *m_MainViewportVertexArray = nullptr;

    // Rendering Methods
    // Plots the field value
    VertexFragmentShaderProgram *m_PlotFieldProgram = nullptr;
    // Plots the phase
    VertexFragmentShaderProgram *m_PlotPhaseProgram = nullptr;
    // Plots the phase and strings
    VertexFragmentShaderProgram *m_PlotStringsProgram = nullptr;

    // Current simulation
    Simulation *m_Simulation;

    // Field color map
    Texture2D *m_FieldColorMap;
    // Phase color map
    Texture2D *m_PhaseColorMap;
    // Discrete color map for strings
    Texture2D *m_DiscreteColorMap;

    // Frame rate limit
    float m_MaxRenderFPS = 60.0f;
    // Simulation tick rate limit
    float m_MaxSimulationTPS = 60.0f;

    // Timing information
    // Time since last update (unlimited rate)
    double m_LastUpdateTime = 0.0f;
    // Time since last frame (limited by frame rate)
    double m_LastFrameTime = 0.0f;
    // Time since last simulation update (limited by simulation tick rate)
    double m_LastSimulationUpdateTime = 0.0f;

    // Stores the average simulation tick rate
    double m_AverageTickRate = 0.0f;

    // TODO: Make this cleaner and easier to add to
    const char *m_currentPlottingProcedure = "Strings";
    size_t m_currentPlottingProcedureIndex = 4;
    const char *m_currentSimulationProcedure = "Cosmic strings";
};
