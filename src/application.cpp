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
#include "buffer.h"
#include "log.h"
#include "texture.h"

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
    Shader *fragmentShader = new Shader("shaders/plot_field.glsl", ShaderType::FRAGMENT_SHADER);
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
    Shader *secondComputeShader = new Shader("shaders/evolve_velocity_acceleration.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *secondComputeProgram = new ComputeShaderProgram(secondComputeShader);

    this->m_textureProgram = textureProgram;
    this->m_firstComputeProgram = firstComputeProgram;
    this->m_secondComputeProgram = secondComputeProgram;

    // Vertex array
    float vertices[] = {
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

    std::vector<std::shared_ptr<Texture2D>> textures = Texture2D::loadFromCTDDFile("data/companion_axion_M200_N200_np23213241.ctdd");
    this->m_fields = textures;

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
    m_fields.clear();

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
    // Compute pass
    static float dx = 1.0f;
    static float dt = 0.1f;
    static float eta = 1.0f;
    static float lam = 5.0f;
    static float alpha = 2.0f;
    static float era = 1.0f;
    static uint32_t fieldSwitch = 0;

    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer->framebufferID);
    glViewport(0, 0, m_framebuffer->width, m_framebuffer->height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_firstComputeProgram->use();
    glUniform1f(0, dx);
    glUniform1f(1, dt);
    m_fields[fieldSwitch % 2]->bind(0);
    m_fields[(fieldSwitch + 1) % 2]->bind(1);
    glDispatchCompute(ceil(200 / 8), ceil(200 / 4), 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    m_secondComputeProgram->use();
    glUniform1f(0, dx);
    glUniform1f(1, dt);
    glUniform1f(2, eta);
    glUniform1f(3, lam);
    glUniform1f(4, alpha);
    glUniform1f(5, era);
    m_fields[fieldSwitch % 2]->bind(0);
    m_fields[(fieldSwitch + 1) % 2]->bind(1);
    glDispatchCompute(ceil(200 / 8), ceil(200 / 4), 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    m_textureProgram->use();
    // Bind uniforms here
    m_fields[0]->bind(0);
    // m_fields[fieldSwitch % 2]->bind(0);

    // Bind VAO
    m_mainViewportVertexArray->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fieldSwitch++;
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
    if (ImGui::Begin("Left Hand Window", nullptr, ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_imguiIO->Framerate, m_imguiIO->Framerate);
    }
    ImGui::End();
    if (ImGui::Begin("Right hand Window", nullptr, ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("HELLO");
    }
    ImGui::End();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Main Viewport", nullptr, windowFlags))
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