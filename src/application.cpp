// Standard libraries
#include <sstream>
#include <stdio.h>

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
#include "texture.h"
#include "simulation.h"

void GLAPIENTRY
messageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam)
{
    char outMessage[1024];
    sprintf(outMessage, "OpenGL: TYPE - 0x%x, SEVERITY - 0x%x, MESSAGE - %s",
            type, severity, message);
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        logError(outMessage);
    }
    else if (severity != 0x826b)
    {
        logDebug(outMessage);
    }
}

Application::Application(int width, int height, const char *title)
{
    logDebug("Initialising application...");
    // Initialise glfw and specify OpenGL version
    glfwInit();
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
    glfwMakeContextCurrent(window);

    // Set resize callback
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Initialise OpenGL via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logFatal("Failed to initialise GLAD");
        return;
    }

    // Set up debugging
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, 0);

    // Set up ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    this->m_windowHandle = window;
    this->m_imguiIO = &io;

    // Set up shaders
    // 1. Load text from file
    Shader *vertexShader = new Shader("shaders/texture_vertex.glsl", ShaderType::VERTEX_SHADER);
    if (!vertexShader->isInitialised)
    {
        return;
    }
    Shader *plotFieldShader = new Shader("shaders/plot_field.glsl", ShaderType::FRAGMENT_SHADER);
    if (!plotFieldShader->isInitialised)
    {
        return;
    }
    Shader *plotPhaseShader = new Shader("shaders/plot_phase.glsl", ShaderType::FRAGMENT_SHADER);
    if (!plotPhaseShader->isInitialised)
    {
        return;
    }
    // 2. Create shader program
    VertexFragmentShaderProgram *plotFieldProgram = new VertexFragmentShaderProgram(vertexShader, plotFieldShader);
    if (!plotFieldProgram->isInitialised)
    {
        return;
    }
    VertexFragmentShaderProgram *plotPhaseProgram = new VertexFragmentShaderProgram(vertexShader, plotPhaseShader);
    if (!plotPhaseProgram->isInitialised)
    {
        return;
    }

    this->m_plotFieldProgram = plotFieldProgram;
    this->m_plotPhaseProgram = plotPhaseProgram;

    delete vertexShader;
    delete plotFieldShader;
    delete plotPhaseShader;

    // Vertex array
    float vertices[] = {
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

    float verticesRIGHTSIDEUP[] = {
        // Top left -  screen coordinates
        -1.0f, +1.0f,
        // Top left - UV
        0.0f, 1.0f,

        // Top right -  screen coordinates
        +1.0f, +1.0f,
        // Top right - UV
        1.0f, 1.0f,

        // Bottom left -  screen coordinates
        -1.0f, -1.0f,
        // Bottom left - UV
        0.0f, 0.0f,

        // Bottom right -  screen coordinates
        1.0f, -1.0f,
        // Bottom right - UV
        1.0f, 0.0f};

    uint32_t indices[] = {
        1, 3, 0, // first triangle
        3, 2, 0  // second triangle
    };

    VertexBufferLayout layout = {{BufferElementType::FLOAT2, false}, {BufferElementType::FLOAT2, true}};

    VertexBuffer *vertexBuffer = new VertexBuffer((void *)vertices, sizeof(vertices), BufferUsageType::STATIC_DRAW, layout);
    IndexBuffer *indexBuffer = new IndexBuffer(indices, 6, BufferUsageType::STATIC_DRAW);

    VertexArray *vertexArray = new VertexArray();
    vertexArray->bindVertexBuffer(vertexBuffer);
    vertexArray->bindIndexBuffer(indexBuffer);

    this->m_mainViewportVertexArray = vertexArray;

    // Create framebuffer
    Framebuffer *framebuffer = new Framebuffer(1920, 1080);
    this->m_framebuffer = framebuffer;

    this->m_simulation = Simulation::createDomainWallSimulation();

    m_simulation->setField(Texture2D::loadFromCTDDFile("data/single_axion_n1.ctdd"));

    m_colorMap = Texture2D::loadFromCTDDFile("colormaps/twilight_shifted_colormap.ctdd")[0];

    // Initialisation complete
    this->isInitialised = true;
    logDebug("Application initialisation completed successfully!");

    // Initialise file dialog system
    NFD_Init();

    return;
}

Application::~Application()
{
    logDebug("Cleaning up application...");
    // Clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // De-allocate
    delete m_plotFieldProgram;
    delete m_plotPhaseProgram;
    delete m_mainViewportVertexArray;
    delete m_simulation;

    // Terminate
    glfwTerminate();

    logDebug("Application cleanup completed successfully!");

    // Quit file dialog system
    NFD_Quit();
}

void Application::run()
{
    // Application loop
    while (!glfwWindowShouldClose(m_windowHandle))
    {
        // Handle input
        processInput(m_windowHandle);
        // 1. Frame rate limited update
        double now = glfwGetTime();
        double updateDelta = now - m_lastUpdateTime;
        double frameDelta = now - m_lastFrameTime;
        double simulationUpdateDelta = now - m_lastSimUpdateTime;
        m_tickRate = 1.0f / simulationUpdateDelta;
        if ((now - m_lastFrameTime) >= (1.0f / m_fpsLimit))
        {
            // Scene rendering
            Application::onRender();
            // UI rendering
            Application::beginImGuiFrame();
            Application::onImGuiRender();
            Application::endImGuiFrame();
            // Swap image buffer
            glfwSwapBuffers(m_windowHandle);

            // Update time of last frame
            m_lastFrameTime = now;
        }

        if ((now - m_lastSimUpdateTime) >= (1.0f / m_simulationFPS))
        {
            // 3. Simulation update
            Application::onSimulationUpdate();

            m_lastSimUpdateTime = now;
        }

        // 2. Frame rate unlimited update
        Application::onUpdate();

        // Poll events
        glfwPollEvents();

        // Update time of last
        m_lastUpdateTime = now;
    }
}

void Application::onRender()
{
    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer->framebufferID);
    glViewport(0, 0, m_framebuffer->width, m_framebuffer->height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_currentPlottingProcedureIndex == 0 || m_simulation->fields.size() < 2)
    {
        m_plotFieldProgram->use();
        glUniform1f(0, 1.0f);
        m_colorMap->bind(0);
        if (m_showLaplacian)
        {
            m_simulation->getCurrentLaplacian()->bind(1);
        }
        else
        {
            m_simulation->getCurrentRenderTexture()->bind(1);
        }
    }
    else if (m_currentPlottingProcedureIndex == 1)
    {
        m_plotPhaseProgram->use();
        m_colorMap->bind(0);
        m_simulation->getRenderTexture(0)->bind(1);
        m_simulation->getRenderTexture(1)->bind(2);
    }

    // Bind VAO
    m_mainViewportVertexArray->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_imguiIO->Framerate, m_imguiIO->Framerate);

        // Show update speed
        ImGui::Text("Simulation average %.3f ms/tick (%.1f TPS)", 1000.0f / m_tickRate, m_tickRate);

        // Change simulation speed
        ImGui::SliderFloat("Simulation FPS", &m_simulationFPS, 1.0f, 1000.0f);

        // Current simulation time
        ImGui::Text("Simulation time %f", m_simulation->getCurrentSimulationTime());
        // Current simulation timestep
        ImGui::Text("Simulation timestep %d", m_simulation->getCurrentSimulationTimestep());

        // Control the number of timesteps
        ImGui::InputInt("Max Timesteps", &m_maxTimesteps, 100, 1000);
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
                std::vector<std::shared_ptr<Texture2D>> loadedTextures = Texture2D::loadFromCTDDFile(outPath);
                logTrace("The new ctdd contains %d fields", loadedTextures.size());
                m_simulation->setField(loadedTextures);
                m_simulation->originalFields = loadedTextures;
                m_simulation->originalFields.resize(loadedTextures.size());

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
        if (ImGui::Button("Save as"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"cosmotd Data Files", "ctdd"}};
            nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, nullptr);
            if (result == NFD_OKAY)
            {
                m_simulation->saveFields(outPath);
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

        // Change colormap
        if (ImGui::Button("Change colormap"))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = {{"cosmotd Data Files", "ctdd"}};
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
            if (result == NFD_OKAY)
            {
                m_colorMap = Texture2D::loadFromCTDDFile(outPath)[0];

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

        const char *availablePlottingProcedures[] = {"Field", "Phase"};

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
                    delete m_simulation;
                    if (n == 0)
                    {
                        m_simulation = Simulation::createDomainWallSimulation();
                        // Set field
                        m_simulation->setField(Texture2D::loadFromCTDDFile("data/domain_walls_M200_N200_np486761876.ctdd"));
                    }
                    else if (n == 1)
                    {
                        m_simulation = Simulation::createCosmicStringSimulation();
                        // Set field
                        m_simulation->setField(Texture2D::loadFromCTDDFile("data/cosmic_strings_M200_N200_np16579.ctdd"));
                    }
                    else if (n == 2)
                    {
                        m_simulation = Simulation::createSingleAxionSimulation();
                        // Set field
                        m_simulation->setField(Texture2D::loadFromCTDDFile("data/single_axion_n1.ctdd"));
                    }
                    else if (n == 3)
                    {
                        m_simulation = Simulation::createCompanionAxionSimulation();
                        // Set field
                        m_simulation->setField(Texture2D::loadFromCTDDFile("data/companion_axion_M200_N200_np23213241.ctdd"));
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
        ImGui::Checkbox("Show Laplacian", &m_showLaplacian);
        m_simulation->onUIRender();
    }
    ImGui::End();

    if (ImGui::Begin("Main Viewport", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        constexpr uint32_t imageWidth = 1176;
        constexpr uint32_t imageHeight = 1003;
        constexpr ImVec2 imageSize = ImVec2(imageWidth, imageHeight);
        ImGui::Image((void *)(intptr_t)m_framebuffer->renderTextureID, imageSize);
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
    // m_simulation->update();
}
void Application::onSimulationUpdate()
{
    if (m_simulation->getCurrentSimulationTimestep() >= m_maxTimesteps)
    {
        m_simulation->runFlag = false;
    }

    m_simulation->update();
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}