// Standard libraries
#include <sstream>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

// External libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <nfd.h>

// Internal libraries
#include "application.h"
#include "buffer.h"
#include "log.h"
#include "simulation.h"
#include "texture.h"

// Vertex array for a fullscreen textured quad
const float vertices[] = {
    // Top left -  screen coordinates
    -1.0f, +1.0f,
    // Top left - UV
    0.0f, 0.0f,

    // Top right -  screen coordinates
    +1.0f, +1.0f,
    // Top right - UV
    1.0f, 0.0f,

    // Bottom left -  screen coordinates
    -1.0f, -1.0f,
    // Bottom left - UV
    0.0f, 1.0f,

    // Bottom right -  screen coordinates
    1.0f, -1.0f,
    // Bottom right - UV
    1.0f, 1.0f};

// Index array for a fullscreen textured quad
const uint32_t indices[] = {
    1, 3, 0, // first triangle
    3, 2, 0  // second triangle
};

// Processes user input
void processInput(GLFWwindow *window)
{
    // Escape to close
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// This is a callback that resizes the viewport upon framebuffer resize.
void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    // Resize viewport
    glViewport(0, 0, width, height);
}

// This is a callback that logs debug or error information coming from OpenGL.
void GLAPIENTRY messageCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam)
{
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        logError("OpenGL: TYPE - 0x%x, SEVERITY - 0x%x, MESSAGE - %s", type, severity, message);
    }
    // Only log debug information that is severe enough
    else if (severity != 0x826b)
    {
        logDebug("OpenGL: TYPE - 0x%x, SEVERITY - 0x%x, MESSAGE - %s", type, severity, message);
    }
    // These tend to be too verbose
    else
    {
        // logTrace("OpenGL: TYPE - 0x%x, SEVERITY - 0x%x, MESSAGE - %s", type, severity, message);
    }
}

Application::Application(int width, int height, const char *title)
{
    logDebug("Application is being initialised...");
    // Initialise glfw
    glfwInit();
    // OpenGL Core version 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create glfw window
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        logFatal("Failed to create GLFW window.");
        return;
    }
    logTrace("GLFW window successfully created.");

    // Set context to current window
    glfwMakeContextCurrent(window);

    // Set resize callback
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Initialise OpenGL via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logFatal("Failed to initialise GLAD");
        return;
    }

    logTrace("OpenGL successfully loaded via GLAD.");

    // Set up debugging
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, nullptr);

    // Set up ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    logTrace("ImGui successfully initialised with OpenGL as its backend.");

    // Save window handle and ImGuiIO
    this->m_WindowHandle = window;
    this->m_ImGuiIO = &io;

    // Compile shaders
    // Texture vertex shader
    Shader *vertexShader = new Shader("shaders/texture_vertex.glsl", ShaderType::VERTEX_SHADER);
    if (!vertexShader->isInitialised)
    {
        return;
    }
    // Plots the field value using a colormap
    Shader *plotFieldShader = new Shader("shaders/plot_field.glsl", ShaderType::FRAGMENT_SHADER);
    if (!plotFieldShader->isInitialised)
    {
        return;
    }
    // Plots the phase using a colormap
    Shader *plotPhaseShader = new Shader("shaders/plot_phase.glsl", ShaderType::FRAGMENT_SHADER);
    if (!plotPhaseShader->isInitialised)
    {
        return;
    }
    // Plots the phase and highlights strings using a colormap
    Shader *plotStringsShader = new Shader("shaders/plot_strings.glsl", ShaderType::FRAGMENT_SHADER);
    if (!plotStringsShader->isInitialised)
    {
        return;
    }

    // Link the vertex and fragment shader stages into programs
    // Link field shader program
    VertexFragmentShaderProgram *plotFieldProgram = new VertexFragmentShaderProgram(vertexShader, plotFieldShader);
    if (!plotFieldProgram->isInitialised)
    {
        return;
    }
    // Link phase shader program
    VertexFragmentShaderProgram *plotPhaseProgram = new VertexFragmentShaderProgram(vertexShader, plotPhaseShader);
    if (!plotPhaseProgram->isInitialised)
    {
        return;
    }
    // Link strings shader program
    VertexFragmentShaderProgram *plotStringsProgram = new VertexFragmentShaderProgram(vertexShader, plotStringsShader);
    if (!plotStringsProgram->isInitialised)
    {
        return;
    }

    logTrace("Plotting shader programs successfully linked.");

    // Save shader programs
    this->m_PlotFieldProgram = plotFieldProgram;
    this->m_PlotPhaseProgram = plotPhaseProgram;
    this->m_PlotStringsProgram = plotStringsProgram;

    // Delete shaders as they are no longer necessary
    delete vertexShader;
    delete plotFieldShader;
    delete plotPhaseShader;
    delete plotStringsShader;

    // Define vertex buffer layout - screen position (x, y) and texture coordinates (u, v)
    VertexBufferLayout layout = {{BufferElementType::FLOAT2, false}, {BufferElementType::FLOAT2, true}};
    // Create vertex and index buffer for a fullscreen textured quad
    VertexBuffer *vertexBuffer = new VertexBuffer((void *)vertices, sizeof(vertices), BufferUsageType::STATIC_DRAW, layout);
    IndexBuffer *indexBuffer = new IndexBuffer(indices, sizeof(indices), BufferUsageType::STATIC_DRAW);
    // Save vertex buffer and index buffer into a vertex array
    VertexArray *vertexArray = new VertexArray();
    vertexArray->bindVertexBuffer(vertexBuffer);
    vertexArray->bindIndexBuffer(indexBuffer);
    // Save vertex array
    this->m_MainViewportVertexArray = vertexArray;

    // Create framebuffer (1080p)
    Framebuffer *framebuffer = new Framebuffer(1024, 1024);
    this->m_Framebuffer = framebuffer;

    // Create topological defect simulation. Default is domain walls.
    this->m_Simulation = Simulation::createDomainWallSimulation();
    // Set default field
    m_Simulation->setField(Texture2D::loadCTDD("data/default/domain_walls_M200_N200_np20228.ctdd"));

    logTrace("Default simulation successfully initialised.");

    // Set default field colormap
    m_FieldColorMap = Texture2D::loadPNG("colormaps/viridis_colormap.png");
    // Set default phase colormap
    m_PhaseColorMap = Texture2D::loadPNG("colormaps/twilight_shifted_colormap.png");
    // Set default discrete colormap
    m_DiscreteColorMap = Texture2D::loadPNG("colormaps/cosmic_string_highlight_colormap.png");

    logTrace("All colormaps successfully loaded.");

    // Initialise file dialog system
    NFD_Init();

    logTrace("File dialog system successfully initialised.");

    // Initialisation complete
    this->isInitialised = true;
    logDebug("Application successfully initialised.");

    return;
}

Application::~Application()
{
    logDebug("Application is being cleaned up...");

    // Clean up ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    logTrace("ImGui successfully shut down.");

    // Clean up plotting shader programs
    delete m_PlotFieldProgram;
    delete m_PlotPhaseProgram;
    delete m_PlotStringsProgram;

    // Clean up remaining OpenGL resources
    delete m_Framebuffer;
    delete m_MainViewportVertexArray;
    delete m_FieldColorMap;
    delete m_PhaseColorMap;
    delete m_DiscreteColorMap;

    // Clean up simulation
    delete m_Simulation;

    // Clean up GLFW resources
    glfwDestroyWindow(m_WindowHandle);
    glfwTerminate();

    logTrace("GLFW resources successfully destroyed.");

    // Quit file dialog system
    NFD_Quit();

    logTrace("File dialog system successfully shut down.");

    logDebug("Application successfully shut down.");
}

void Application::run()
{
    // Application loop
    while (!glfwWindowShouldClose(m_WindowHandle))
    {
        // Handle input
        processInput(m_WindowHandle);
        // Update timings
        double now = glfwGetTime();
        double updateDelta = now - m_LastUpdateTime;
        double frameDelta = now - m_LastFrameTime;
        double simulationUpdateDelta = now - m_LastSimulationUpdateTime;
        m_AverageTickRate = 1.0f / simulationUpdateDelta;

        // Render frame
        if (frameDelta >= (1.0f / m_MaxRenderFPS))
        {
            // Scene rendering
            Application::onRender();
            // UI rendering
            Application::beginImGuiFrame();
            Application::onImGuiRender();
            Application::endImGuiFrame();
            // Swap image buffer
            glfwSwapBuffers(m_WindowHandle);

            // Update time of last frame
            m_LastFrameTime = now;
        }

        // Simulation tick
        if (simulationUpdateDelta >= (1.0f / m_MaxSimulationTPS))
        {
            // Simulation update tick
            Application::onSimulationUpdate();

            // Update time of last simulation tick
            m_LastSimulationUpdateTime = now;
        }

        // (Unlimited) update frame
        Application::onUpdate();

        // Poll events
        glfwPollEvents();

        // Update time of last (unlimited) update
        m_LastUpdateTime = now;
    }
}

void Application::onRender()
{
    // Bind target framebuffer
    m_Framebuffer->bind();
    glViewport(0, 0, m_Framebuffer->getTextureWidth(), m_Framebuffer->getTextureHeight());

    // Clear background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Check if the phase is possible to plot
    bool phaseAvailable = m_Simulation->hasStrings();

    // Plot the field
    if (m_currentPlottingProcedureIndex == 0)
    {
        m_PlotFieldProgram->use();
        glUniform1f(0, m_Simulation->getMaxValue());
        m_FieldColorMap->bindUnit(0);
        m_Simulation->getCurrentRenderTexture()->bindUnit(1);
    }
    // Plot the raw phase (sample from phase texture)
    else if (m_currentPlottingProcedureIndex == 1 && phaseAvailable)
    {

        m_PlotFieldProgram->use();
        glUniform1f(0, M_PI);
        m_PhaseColorMap->bindUnit(0);
        m_Simulation->getCurrentPhase()->bindUnit(1);
    }
    // Plot the smooth phase (sample from real and imaginary fields and compute. This is smoother looking due to the linear interpolation)
    else if (m_currentPlottingProcedureIndex == 2 && phaseAvailable)
    {
        m_PlotPhaseProgram->use();
        m_PhaseColorMap->bindUnit(0);
        m_Simulation->getCurrentRealTexture()->bindUnit(1);
        m_Simulation->getCurrentImagTexture()->bindUnit(2);
    }
    // Plot the Laplacian
    else if (m_currentPlottingProcedureIndex == 3)
    {
        m_PlotFieldProgram->use();
        glUniform1f(0, m_Simulation->getMaxValue());
        m_FieldColorMap->bindUnit(0);
        m_Simulation->getCurrentLaplacian()->bindUnit(1);
    }
    // Plot the detected strings
    else if (m_currentPlottingProcedureIndex == 4 && phaseAvailable)
    {
        m_PlotStringsProgram->use();
        m_PhaseColorMap->bindUnit(0);
        m_DiscreteColorMap->bindUnit(1);
        m_Simulation->getCurrentRealTexture()->bindUnit(2);
        m_Simulation->getCurrentImagTexture()->bindUnit(3);
        m_Simulation->getCurrentStrings()->bindUnit(4);
    }
    // For undefined indices or if the phase is unavailable, just plot the phase
    else
    {
        m_PlotFieldProgram->use();
        glUniform1f(0, m_Simulation->getMaxValue());
        m_FieldColorMap->bindUnit(0);
        m_Simulation->getCurrentRenderTexture()->bindUnit(1);
    }

    // Bind VAO
    m_MainViewportVertexArray->bind();
    // Draw the texture to the framebuffer
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Unbind framebuffer
    m_Framebuffer->unbind();
}

void Application::beginImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Note: Switch this to true to enable dockspace
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistent = true;
    bool opt_fullscreen = opt_fullscreen_persistent;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // Set up main docking window. This can't be docked into otherwise there can be a recursive docking relationship.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    // Set the docking window to take up the full screen
    if (opt_fullscreen)
    {
        // Set window size to full screen
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        // Turn off window rounding and take off borders
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        // Remove title bar, make it uncollapsible and unresizable
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        // Make it immovable, and unfocusable
        window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole,
    // so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Dockspace window should not be padded
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Main Dockspace", &dockspaceOpen, window_flags);
    // Pop window style variable now that the window has been created
    ImGui::PopStyleVar();

    if (opt_fullscreen)
    {
        // Pop the two style variables set earlier if in full screen mode
        ImGui::PopStyleVar(2);
    }

    // DockSpace
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    float minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("Main Dockspace");
        // Create dockspace
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    ImGui::End();
}

void Application::onImGuiRender()
{
    if (ImGui::Begin("Left Hand Window"))
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_ImGuiIO->Framerate, m_ImGuiIO->Framerate);

        // Show update speed
        ImGui::Text("Simulation average %.3f ms/tick (%.1f TPS)", 1000.0f / m_AverageTickRate, m_AverageTickRate);

        // Change simulation speed
        ImGui::SliderFloat("Simulation FPS", &m_MaxSimulationTPS, 1.0f, 1000.0f);

        // Current simulation time
        ImGui::Text("Simulation time %f", m_Simulation->getCurrentSimulationTime());
        // Current simulation timestep
        ImGui::Text("Simulation timestep %d", m_Simulation->getCurrentSimulationTimestep());

        // Control the number of timesteps
        ImGui::InputInt("Max Timesteps", &m_Simulation->maxTimesteps, 100, 1000);

        if (m_Simulation->hasStrings())
        {
            // Number of strings
            std::vector<int> stringNumbers = m_Simulation->getCurrentStringNumber();
            ImGui::Text("Number of strings:");
            int stringIndex = 1;
            for (const auto &currentStringNumber : stringNumbers)
            {
                ImGui::Text("Pair %d: %d", stringIndex++, currentStringNumber);
            }
        }
    }
    ImGui::End();
    if (ImGui::Begin("Right hand Window"))
    {
        // File dialogs
        if (ImGui::Button("Open file"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"cosmotd Data Files", "ctdd"}};
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
            if (result == NFD_OKAY)
            {
                std::vector<std::shared_ptr<Texture2D>> loadedTextures = Texture2D::loadCTDD(outPath);
                logTrace("The new ctdd contains %d fields", loadedTextures.size());
                m_Simulation->setField(loadedTextures);

                // Free file path after use
                NFD_FreePath(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                logDebug("Cancelling open file dialog...");
            }
            else
            {
                logError("Open file dialog error: %s", NFD_GetError());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save field as"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"cosmotd Data Files", "ctdd"}};
            nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, nullptr);
            if (result == NFD_OKAY)
            {
                m_Simulation->saveFields(outPath);
                logDebug("Saving field at path %s", outPath);

                // Free file path after use
                NFD_FreePath(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                logDebug("Cancelling save file dialog...");
            }
            else
            {
                logError("Save file dialog error: %s", NFD_GetError());
            }
        }
        if (ImGui::Button("Save phase as"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"cosmotd Data Files", "ctdd"}};
            nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, nullptr);
            if (result == NFD_OKAY)
            {
                m_Simulation->savePhases(outPath);
                logDebug("Saving phases at path %s", outPath);

                // Free file path after use
                NFD_FreePath(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                logDebug("Cancelling save file dialog...");
            }
            else
            {
                logError("Save file dialog error: %s", NFD_GetError());
            }
        }
        if (ImGui::Button("Save string counts as"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"string count data files", "data"}};
            nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, nullptr);
            if (result == NFD_OKAY)
            {
                m_Simulation->saveStringNumbers(outPath);
                logDebug("Saving string counts at path %s", outPath);

                // Free file path after use
                NFD_FreePath(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                logDebug("Cancelling save file dialog...");
            }
            else
            {
                logError("Save file dialog error: %s", NFD_GetError());
            }
        }

        // Random
        static int fieldWidth = 256;
        static int fieldHeight = 256;
        static int seed = 0;
        ImGui::SliderInt("Field width", &fieldWidth, 0, 1000);
        ImGui::SliderInt("Field height", &fieldHeight, 0, 1000);
        ImGui::InputInt("Seed", &seed);
        if (ImGui::Button("Randomise Fields"))
        {
            m_Simulation->randomiseFields((uint32_t)fieldWidth, (uint32_t)fieldHeight, (uint32_t)seed);
        }

        // Change colormap
        if (ImGui::Button("Change field colormap"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"Portable Network Graphics", "png"}};
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
            if (result == NFD_OKAY)
            {
                m_FieldColorMap = Texture2D::loadPNG(outPath);

                // Free file path after use
                NFD_FreePath(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                logDebug("Cancelling open file dialog...");
            }
            else
            {
                logError("Open file dialog error: %s", NFD_GetError());
            }
        }

        ImGui::SameLine();

        // Change colormap
        if (ImGui::Button("Change phase colormap"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"Portable Network Graphics", "png"}};
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
            if (result == NFD_OKAY)
            {
                m_PhaseColorMap = Texture2D::loadPNG(outPath);

                // Free file path after use
                NFD_FreePath(outPath);
            }
            else if (result == NFD_CANCEL)
            {
                logDebug("Cancelling open file dialog...");
            }
            else
            {
                logError("Open file dialog error: %s", NFD_GetError());
            }
        }

        if (ImGui::Button("Run trials"))
        {
            m_Simulation->runRandomTrials(100);
        }

        const char *availablePlottingProcedures[] = {"Field", "Raw Phase", "Smooth Phase", "Laplacian", "Strings"};

        if (ImGui::BeginCombo("Plotting", m_currentPlottingProcedure))
        {
            for (int n = 0; n < IM_ARRAYSIZE(availablePlottingProcedures); n++)
            {
                bool isSelected = (m_currentPlottingProcedure == availablePlottingProcedures[n]);
                if (ImGui::Selectable(availablePlottingProcedures[n], isSelected))
                {
                    m_currentPlottingProcedure = availablePlottingProcedures[n];
                    m_currentPlottingProcedureIndex = n;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        const char *availableSimulationProcedures[] = {"Domain walls", "Cosmic strings", "Single axion", "Companion axion"};

        if (ImGui::BeginCombo("Simulation Type", m_currentSimulationProcedure))
        {
            for (int n = 0; n < IM_ARRAYSIZE(availableSimulationProcedures); n++)
            {
                bool isSelected = (m_currentSimulationProcedure == availableSimulationProcedures[n]);
                if (ImGui::Selectable(availableSimulationProcedures[n], isSelected))
                {
                    m_currentSimulationProcedure = availableSimulationProcedures[n];

                    // Set new simulation
                    delete m_Simulation;
                    if (n == 0)
                    {
                        m_Simulation = Simulation::createDomainWallSimulation();
                        // Set field
                        m_Simulation->setField(Texture2D::loadCTDD("data/default/domain_walls_M200_N200_np20228.ctdd"));
                    }
                    else if (n == 1)
                    {
                        m_Simulation = Simulation::createCosmicStringSimulation();
                        // Set field
                        m_Simulation->setField(Texture2D::loadCTDD("data/default/cosmic_strings_M200_N200_np20228.ctdd"));
                    }
                    else if (n == 2)
                    {
                        m_Simulation = Simulation::createSingleAxionSimulation();
                        // Set field
                        m_Simulation->setField(Texture2D::loadCTDD("data/default/single_axion_M300_N300_np20228.ctdd"));
                    }
                    else if (n == 3)
                    {
                        m_Simulation = Simulation::createCompanionAxionSimulation();
                        // Set field
                        m_Simulation->setField(Texture2D::loadCTDD("data/default/companion_axion_M300_N300_np20228.ctdd"));
                    }
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Simulation Controls and Parameters");
        m_Simulation->onUIRender();
    }
    ImGui::End();

    if (ImGui::Begin("Main Viewport", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        constexpr uint32_t imageWidth = 1176;
        constexpr uint32_t imageHeight = 1003;
        constexpr ImVec2 imageSize = ImVec2(imageWidth, imageHeight);
        ImGui::Image((void *)(intptr_t)m_Framebuffer->getTextureID(), imageSize);
    }
    ImGui::End();
}

void Application::endImGuiFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::onUpdate()
{
    // m_Simulation->update();
}
void Application::onSimulationUpdate()
{
    m_Simulation->update();
}