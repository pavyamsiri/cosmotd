#ifndef APPLICATION_H
#define APPLICATION_H
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

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

class Application
{
public:
    // Methods
    static int initialise(int width, int height, const char *title, Application *app);
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
    // Clean up resources
    void cleanup();

private:
    GLFWwindow *m_windowHandle = nullptr;
    ImGuiIO *m_imguiIO = nullptr;
    Framebuffer *framebuffer = nullptr;

    // Timing information
    double m_lastUpdateTime = 0.0f;
    double m_lastFrameTime = 0.0f;

    // Frame rate
    double m_fpsLimit = 1.0f / 60.0f;

    // Rendering
    uint32_t m_textureProgramID;
    VertexArray *m_mainViewportVertexArray;
    std::vector<Texture2D> m_fields;
};

#endif