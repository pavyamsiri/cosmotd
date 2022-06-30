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
#include <buffer.h>
#include <texture.h>

int Application::initialise(int width, int height, const char *title, Application *app)
{
    // Initialise glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        logFatal("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Initialise OpenGL via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logFatal("Failed to initialise GLAD");
        return -1;
    }

    // Set up ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    app->m_windowHandle = window;
    app->m_imguiIO = &io;

    // Set up shaders
    // 1. Load text from file
    int shaderCompilationResult;
    Shader vertexShader;
    shaderCompilationResult = Shader::compileShaderFromFile(
        "shaders/texture_vertex.glsl", ShaderType::VERTEX_SHADER, &vertexShader);
    if (shaderCompilationResult != 0)
    {
        return shaderCompilationResult;
    }
    Shader fragmentShader;
    shaderCompilationResult = Shader::compileShaderFromFile(
        "shaders/texture_fragment.glsl", ShaderType::FRAGMENT_SHADER, &fragmentShader);
    if (shaderCompilationResult != 0)
    {
        return shaderCompilationResult;
    }
    // 2. Create shader program
    int programLinkingResult;
    uint32_t textureProgramID;
    programLinkingResult = ShaderProgram::linkVertexFragmentProgram(vertexShader, fragmentShader, &textureProgramID);
    if (programLinkingResult != 0)
    {
        return programLinkingResult;
    }

    // NOTE: Need to delete shader after linking to program?
    vertexShader.deleteShader();
    fragmentShader.deleteShader();

    app->m_textureProgramID = textureProgramID;

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

    app->m_mainViewportVertexArray = vertexArray;

    // Create framebuffer
    Framebuffer *framebuffer = new Framebuffer(1920, 1080);
    app->framebuffer = framebuffer;

    std::vector<Texture2D> textures = Texture2D::loadFromCTDDFile("data/companion_axion_M200_N200_np23213241.ctdd");
    logDebug("Finished loading textures");
    app->m_fields = textures;

    return 0;
}

void Application::run()
{
    while (!glfwWindowShouldClose(m_windowHandle))
    {
        processInput(m_windowHandle);
        // 1. Frame rate limited loop
        double now = glfwGetTime();
        double updateDelta = now - m_lastUpdateTime;
        double frameDelta = now - m_lastFrameTime;
        if ((now - m_lastFrameTime) >= m_fpsLimit)
        {
            Application::beginImGuiFrame();
            Application::onImGuiRender();
            Application::onRender();
            Application::endImGuiFrame();
            // Swap image buffer
            glfwSwapBuffers(m_windowHandle);

            // Update time of last frame
            m_lastFrameTime = now;
        }

        Application::onUpdate();

        // 2. ASAP loop
        glfwPollEvents();

        // Update time of last
        m_lastUpdateTime = now;
    }
}

void Application::onRender()
{
    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebufferID);
    // TODO: Set this automatically from framebuffer object
    glViewport(0, 0, 1920, 1080);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_textureProgramID);
    // Bind uniforms here
    glBindTextureUnit(0, m_fields[0].textureID);
    glUniform1i(0, 0);

    // Bind VAO
    m_mainViewportVertexArray->bind();
    // glDrawArrays(GL_TRIANGLES, 0, 3);
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

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
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
        ImGui::Text("HELLO");
    }
    ImGui::End();
    if (ImGui::Begin("Right hand Window"))
    {
        ImGui::Text("HELLO");
    }
    ImGui::End();

    if (ImGui::Begin("Main Viewport"))
    {
        constexpr uint32_t imageWidth = 1152;
        constexpr uint32_t imageHeight = 1000;
        ImGui::Image((void *)(intptr_t)framebuffer->renderTextureID, ImVec2(imageWidth, imageHeight));
    }
    ImGui::End();
}

void Application::endImGuiFrame()
{
    // ImGUI rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::onUpdate()
{
}

void Application::cleanup()
{
    // Clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // De-allocate

    // Terminate
    glfwTerminate();
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