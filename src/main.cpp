#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <random>

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

std::string loadText(const char *filePath);
void checkShaderCompilation(uint32_t shader);
void checkProgramLinking(uint32_t program);
uint32_t createComputeTexture(void *data);
uint32_t initialiseField(uint32_t height, uint32_t width, uint32_t seed);

// settings
constexpr uint32_t SCREEN_WIDTH = 800;
constexpr uint32_t SCREEN_HEIGHT = 600;
constexpr uint32_t COMPUTE_WIDTH = 200;
constexpr uint32_t COMPUTE_HEIGHT = 200;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "BEFORE SHADER COMPILATION" << std::endl;

    // Shader
    uint32_t vertexShader;
    std::string vertexShaderCode = loadText("../src/shaders/vertex.glsl");
    const char *pVertexShaderCode = vertexShaderCode.c_str();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &pVertexShaderCode, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader);
    // Create fragment shader
    uint32_t fragmentShader;
    std::string fragmentShaderCode = loadText("../src/shaders/plot_field.glsl");
    const char *pFragmentShaderCode = fragmentShaderCode.c_str();
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &pFragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompilation(fragmentShader);

    // Create shader program
    uint32_t textureProgram;
    textureProgram = glCreateProgram();
    // Attach shaders
    glAttachShader(textureProgram, vertexShader);
    glAttachShader(textureProgram, fragmentShader);
    // Link program
    glLinkProgram(textureProgram);
    checkProgramLinking(textureProgram);

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::cout
        << "AFTER SHADER COMPILATION" << std::endl;

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // Top left plot
    float vertices[] = {
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top right
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.6f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -0.6f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };
    // // Top right plot
    // float vertices[] = {
    //     0.6f, 1.0f, 0.0f, 1.0f, 1.0f, // top right
    //     0.6f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
    //     0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom left
    //     0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    // };
    uint32_t indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    uint32_t VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // UV attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    std::cout << "AFTER VAO, VBO and EBO" << std::endl;

    std::cout << "BEFORE TEXTURE CREATION" << std::endl;

    uint32_t texture = initialiseField(COMPUTE_HEIGHT, COMPUTE_WIDTH, 478948);

    std::cout << "AFTER TEXTURE CREATION" << std::endl;

    // Create evolve field shader
    uint32_t evolveFieldShader;
    std::string evolveFieldShaderCode = loadText("../src/shaders/evolve_field.glsl");
    const char *pEvolveFieldShaderCode = evolveFieldShaderCode.c_str();
    evolveFieldShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(evolveFieldShader, 1, &pEvolveFieldShaderCode, NULL);
    glCompileShader(evolveFieldShader);
    checkShaderCompilation(evolveFieldShader);

    // Create compute shader program
    uint32_t firstPassProgram;
    firstPassProgram = glCreateProgram();
    // Attach shaders
    glAttachShader(firstPassProgram, evolveFieldShader);
    // Link program
    glLinkProgram(firstPassProgram);
    checkProgramLinking(firstPassProgram);

    // Delete shaders
    glDeleteShader(evolveFieldShader);

    // Create evolve velocity and acceleration shader
    uint32_t evolveVelocityAccelerationShader;
    std::string evolveVelocityAccelerationShaderCode = loadText("../src/shaders/evolve_velocity_acceleration.glsl");
    const char *pEvolveVelocityAccelerationShaderCode = evolveVelocityAccelerationShaderCode.c_str();
    evolveVelocityAccelerationShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(evolveVelocityAccelerationShader, 1, &pEvolveVelocityAccelerationShaderCode, NULL);
    glCompileShader(evolveVelocityAccelerationShader);
    checkShaderCompilation(evolveVelocityAccelerationShader);

    // Create compute shader program
    uint32_t secondPassProgram;
    secondPassProgram = glCreateProgram();
    // Attach shaders
    glAttachShader(secondPassProgram, evolveVelocityAccelerationShader);
    // Link program
    glLinkProgram(secondPassProgram);
    checkProgramLinking(secondPassProgram);

    // Delete shaders
    glDeleteShader(evolveVelocityAccelerationShader);

    int work_grp_cnt[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
    std::cout << "Max work groups per compute shader"
              << " x:" << work_grp_cnt[0] << " y:" << work_grp_cnt[1] << " z:" << work_grp_cnt[2] << "\n";

    int work_grp_size[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
    std::cout << "Max work group sizes"
              << " x:" << work_grp_size[0] << " y:" << work_grp_size[1] << " z:" << work_grp_size[2] << "\n";

    int work_grp_inv;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    std::cout << "Max invocations count per work group: " << work_grp_inv << "\n";

    std::cout << "BEFORE RENDER LOOP" << std::endl;

    const double fpsLimit = 1.0 / 60.0;
    double lastUpdateTime = 0; // number of seconds since the last loop
    double lastFrameTime = 0;  // number of seconds since the last frame

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    float dx = 1.0f;
    float dt = 0.1f;
    float eta = 1.0f;
    float lam = 5.0f;
    float alpha = 2.0f;
    float era = 1.0f;
    bool simulate = false;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        double now = glfwGetTime();
        double deltaTime = now - lastUpdateTime;

        // This if-statement only executes once every 60th of a second
        if ((now - lastFrameTime) >= fpsLimit)
        {
            // render
            // ------
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Note: Switch this to true to enable dockspace
            static bool dockspaceOpen = true;
            static bool opt_fullscreen_persistant = true;
            bool opt_fullscreen = opt_fullscreen_persistant;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                ImGuiViewport *viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
            // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
            // all active windows docked into it will lose their parent and become undocked.
            // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
            // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
            ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // DockSpace
            ImGuiIO &io = ImGui::GetIO();
            ImGuiStyle &style = ImGui::GetStyle();
            float minWinSizeX = style.WindowMinSize.x;
            style.WindowMinSize.x = 370.0f;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            ImGui::End();
            {
                ImGui::Begin("Control Panel"); // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

                ImGui::Text("lambda");
                ImGui::SameLine();
                ImGui::SliderFloat("##lam", &lam, 0.0f, 10.f); // Edit 1 float using a slider from 0.0f to 1.0f

                if (simulate)
                {
                    if (ImGui::Button("Stop", ImVec2(50.0f, 19.0f)))
                    {
                        simulate = false;
                    }
                }
                else
                {
                    if (ImGui::Button("Start", ImVec2(50.0f, 19.0f)))
                    {
                        simulate = true;
                    }
                }

                if (ImGui::Button("Restart"))
                {
                    glDeleteTextures(1, &texture);
                    texture = initialiseField(COMPUTE_HEIGHT, COMPUTE_WIDTH, 879481);
                }

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }
            {
                ImGui::Begin("Viewport");
                ImGui::BeginChild("CHILD");
                ImGui::Text("DRAW");
                ImGui::EndChild();
                ImGui::End();
            }

            // Right panel
            {
                ImGui::Begin("Right Panel"); // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

                ImGui::Text("lambda");
                ImGui::SameLine();
                ImGui::SliderFloat("##lam", &lam, 0.0f, 10.f); // Edit 1 float using a slider from 0.0f to 1.0f

                if (simulate)
                {
                    if (ImGui::Button("Stop", ImVec2(50.0f, 19.0f)))
                    {
                        simulate = false;
                    }
                }
                else
                {
                    if (ImGui::Button("Start", ImVec2(50.0f, 19.0f)))
                    {
                        simulate = true;
                    }
                }

                if (ImGui::Button("Restart"))
                {
                    glDeleteTextures(1, &texture);
                    texture = initialiseField(COMPUTE_HEIGHT, COMPUTE_WIDTH, 879481);
                }

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            if (simulate)
            {
                glUseProgram(firstPassProgram);
                glUniform1f(0, dx);
                glUniform1f(1, dt);
                glBindTextureUnit(0, texture);
                glDispatchCompute(ceil(COMPUTE_WIDTH / 8), ceil(COMPUTE_HEIGHT / 4), 1);
                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                glUseProgram(secondPassProgram);
                glUniform1f(0, dx);
                glUniform1f(1, dt);
                glUniform1f(2, eta);
                glUniform1f(3, lam);
                glUniform1f(4, alpha);
                glUniform1f(5, era);
                glBindTextureUnit(0, texture);
                glDispatchCompute(ceil(COMPUTE_WIDTH / 8), ceil(COMPUTE_HEIGHT / 4), 1);
                glMemoryBarrier(GL_ALL_BARRIER_BITS);
            }

            // render container
            glUseProgram(textureProgram);
            glUniform1f(0, eta);
            glBindTextureUnit(0, texture);
            glUniform1i(glGetUniformLocation(textureProgram, "displayTexture"), 0);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Rendering
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);

            // only set lastFrameTime when you actually draw something
            lastFrameTime = now;
        }
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

std::string loadText(const char *filePath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string text;
    std::ifstream textFile;
    // ensure ifstream objects can throw exceptions:
    textFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        textFile.open(filePath);
        std::stringstream textFileStream, fShaderStream;
        // read file's buffer contents into streams
        textFileStream << textFile.rdbuf();
        // close file handlers
        textFile.close();
        // convert stream into string
        text = textFileStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cout << "ERROR: File " << filePath << " failed to open.\n"
                  << e.what() << std::endl;
    }
    return text;
}

void checkShaderCompilation(uint32_t shader)
{
    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR: Shader failed to compile! "
                  << "\n"
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}

void checkProgramLinking(uint32_t program)
{
    int success;
    char infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cout << "ERROR: Program failed to link! "
                  << "\n"
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}

uint32_t createComputeTexture(void *data)
{
    // load and create a texture
    // -------------------------
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, COMPUTE_WIDTH, COMPUTE_HEIGHT, 0, GL_RGBA, GL_FLOAT, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTextureStorage2D(texture, 1, GL_RGBA32F, COMPUTE_WIDTH, COMPUTE_HEIGHT);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    return texture;
}

uint32_t initialiseField(uint32_t height, uint32_t width, uint32_t seed)
{
    float field[height][width][4];

    // Generate texture
    std::default_random_engine generator;
    generator.seed(seed);
    std::normal_distribution<float>
        distribution(0.0f, 1.0f);

    for (int i = 0; i < COMPUTE_HEIGHT; i++)
    {
        for (int j = 0; j < COMPUTE_WIDTH; j++)
        {
            float redChannel;
            float greenChannel;
            float blueChannel;
            float alphaChannel;

            redChannel = 0.1f * distribution(generator);
            greenChannel = 0.0f;
            blueChannel = 0.0f;
            alphaChannel = 0.1f;

            // RED
            field[i][j][0] = redChannel;
            // GREEN
            field[i][j][1] = greenChannel;
            // BLUE
            field[i][j][2] = blueChannel;
            // ALPHA
            field[i][j][3] = alphaChannel;
        }
    }

    uint32_t texture = createComputeTexture(field);

    return texture;
}