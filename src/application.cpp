// Standard libraries
#include <sstream>
#include <stdio.h>

// External libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
    Shader *fragmentShader = new Shader("shaders/plot_phase.glsl", ShaderType::FRAGMENT_SHADER);
    // Shader *fragmentShader = new Shader("shaders/texture_fragment.glsl", ShaderType::FRAGMENT_SHADER);
    if (!fragmentShader->isInitialised)
    {
        return;
    }
    // 2. Create shader program
    int programLinkingResult;
    VertexFragmentShaderProgram *textureProgram = new VertexFragmentShaderProgram(vertexShader, fragmentShader);
    if (!textureProgram->isInitialised)
    {
        return;
    }

    delete vertexShader;
    delete fragmentShader;

    // Set up compute shader
    Shader *firstComputeShader = new Shader("shaders/evolve_field.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *firstComputeProgram = new ComputeShaderProgram(firstComputeShader);
    if (!firstComputeProgram->isInitialised)
    {
        return;
    }
    Shader *laplacianComputeShader = new Shader("shaders/calculate_laplacian.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *laplacianComputeProgram = new ComputeShaderProgram(laplacianComputeShader);
    if (!laplacianComputeProgram->isInitialised)
    {
        return;
    }
    // Shader *secondComputeShader = new Shader("shaders/domain_walls.glsl", ShaderType::COMPUTE_SHADER);
    Shader *secondComputeShader = new Shader("shaders/single_axion.glsl", ShaderType::COMPUTE_SHADER);
    // Shader *secondComputeShader = new Shader("shaders/cosmic_strings.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *secondComputeProgram = new ComputeShaderProgram(secondComputeShader);
    if (!secondComputeProgram->isInitialised)
    {
        return;
    }

    delete firstComputeShader;
    delete laplacianComputeShader;
    delete secondComputeShader;

    this->m_textureProgram = textureProgram;

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

    // // Domain wall
    // SimulationLayout simulationLayout = {
    //     {UniformDataType::FLOAT, std::string("eta"), 1.0f, 0.0f, 10.0f},
    //     {UniformDataType::FLOAT, std::string("lam"), 5.0f, 0.1f, 10.0f}};

    // Single axion
    SimulationLayout simulationLayout = {
        {UniformDataType::FLOAT, std::string("eta"), 1.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("lam"), 5.0f, 0.1f, 10.0f},
        {UniformDataType::INT, std::string("colorAnomaly"), (int)3, (int)1, (int)10},
        {UniformDataType::FLOAT, std::string("axionStrength"), 0.025f, 0.1f, 5.0f},
        {UniformDataType::FLOAT, std::string("growthScale"), 75.0f, 50.0f, 100.0f},
        {UniformDataType::FLOAT, std::string("growthLaw"), 2.0f, 1.0f, 7.0f},
    };

    this->m_simulation = new Simulation(1, firstComputeProgram, laplacianComputeProgram, secondComputeProgram, simulationLayout);

    // m_simulation->setField(Texture2D::loadFromCTDDFile("data/vertical_strip.ctdd"));
    // m_simulation->setField(Texture2D::loadFromCTDDFile("data/laplacian_test.ctdd"));
    // m_simulation->setField(Texture2D::loadFromCTDDFile("data/domain_walls_M200_N200_np486761876.ctdd"));
    m_simulation->setField(Texture2D::loadFromCTDDFile("data/cosmic_strings_M200_N200_np489744.ctdd"));

    m_colorMap = Texture2D::loadFromCTDDFile("colormaps/twilight_shifted_colormap.ctdd")[0];

    // Initialisation complete
    this->isInitialised = true;
    logDebug("Application initialisation completed successfully!");

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
    delete m_textureProgram;
    delete m_mainViewportVertexArray;

    // Terminate
    glfwTerminate();

    logDebug("Application cleanup completed successfully!");
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
        m_tickRate = 1.0f / updateDelta;
        double frameDelta = now - m_lastFrameTime;
        if ((now - m_lastFrameTime) >= m_fpsLimit)
        {
            // UI rendering
            Application::beginImGuiFrame();
            Application::onImGuiRender();
            Application::endImGuiFrame();
            // Scene rendering
            Application::onRender();
            // Swap image buffer
            glfwSwapBuffers(m_windowHandle);

            // Update time of last frame
            m_lastFrameTime = now;
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

    // UPDATE SIMULATION
    m_simulation->update();

    m_textureProgram->use();
    // Bind uniforms here
    // glUniform1f(0, 1.0f);
    // Texture2D *renderTexture = m_simulation->getCurrentRenderTexture();
    m_colorMap->bind(0);
    m_simulation->getRenderTexture(0)->bind(1);
    m_simulation->getRenderTexture(1)->bind(2);

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
        // Pop the two style variables set earlier if in full screen mode
        ImGui::PopStyleVar(2);

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
    }
    ImGui::End();
    if (ImGui::Begin("Right hand Window"))
    {
        ImGui::Text("HELLO");
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

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}