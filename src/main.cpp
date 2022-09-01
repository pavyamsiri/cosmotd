// Standard libraries
#include <sstream>

// External libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Internal libraries
#include "application.h"
#include "errors.h"
#include "log.h"
#include "shader.h"
#include "shader_program.h"

// Application specification
constexpr uint32_t SCREEN_WIDTH = 800;
constexpr uint32_t SCREEN_HEIGHT = 600;
constexpr const char *APPLICATION_TITLE = "Cosmological Topological Defects";

int main()
{
    Application *app = new Application(SCREEN_WIDTH, SCREEN_HEIGHT, APPLICATION_TITLE);
    if (!app->isInitialised)
    {
        logFatal("Failed to initialise application!");
        return APPLICATION_INITIALISATION_FAILURE;
    }

    app->run();

    delete app;

    return APPLICATION_SUCCESS;
}