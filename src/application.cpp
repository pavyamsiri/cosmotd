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

int Application::initialise(int width, int height, const char *title, Application *app)
{
    // Initialise glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
        "shaders/triangle_vertex.glsl", ShaderType::VERTEX_SHADER, &vertexShader);
    if (shaderCompilationResult != 0)
    {
        return shaderCompilationResult;
    }
    Shader fragmentShader;
    shaderCompilationResult = Shader::compileShaderFromFile(
        "shaders/triangle_fragment.glsl", ShaderType::FRAGMENT_SHADER, &fragmentShader);
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
        0.0f,
        0.5f,
        0.0f,
        0.5f,
        -0.5f,
        0.0f,
        -0.5f,
        -0.5f,
        0.0f,
    };

    VertexBufferLayout layout = {{BufferElementType::FLOAT3, true}};

    VertexBuffer *vertexBuffer = new VertexBuffer((void *)vertices, sizeof(vertices), BufferUsageType::STATIC_DRAW, layout);

    VertexArray *vertexArray = new VertexArray();
    vertexArray->bindVertexBuffer(vertexBuffer);

    app->m_mainViewportVertexArray = vertexArray;

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
            Application::onRender();
            Application::beginImGuiFrame();
            Application::onImGuiRender();
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_textureProgramID);
    m_mainViewportVertexArray->bind();
    // glBindVertexArray(m_mainViewportVertexArray->arrayID);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::beginImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::onImGuiRender()
{
    ImGui::Begin("TEST");
    ImGui::Text("HELLO WORLD!");
    if (ImGui ::Button("TEST"))
    {
        logTrace("Trest");
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