// Standard libraries
#include <sstream>

// External libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Internal libraries
#include <log.h>
#include <application.h>
#include <shader.h>
#include <shader_program.h>

// Constants
constexpr uint32_t SCREEN_WIDTH = 800;
constexpr uint32_t SCREEN_HEIGHT = 600;
constexpr const char *APPLICATION_TITLE = "Cosmological Topological Defects";

int main()
{
    Application app;
    logDebug("Before initialisation!");

    int initialisationResult = Application::initialise(SCREEN_WIDTH, SCREEN_HEIGHT, APPLICATION_TITLE, &app);
    if (initialisationResult != 0)
    {
        return initialisationResult;
    }

    // Set up vertex data

    app.run();

    app.cleanup();

    // Return
    logTrace("FINISHED EXECUTION");
    return 0;
}