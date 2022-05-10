#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>

#include <string>
#include <iostream>
#include <random>

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

std::string loadText(const char *filePath);
void checkShaderCompilation(uint32_t shader);
void checkProgramLinking(uint32_t program);
uint32_t createComputeTexture(void *data);

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

    // build and compile our shader zprogram
    // ------------------------------------
    // Shader textureShader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    // Create vertex shader
    uint32_t vertexShader;
    std::string vertexShaderCode = loadText("../src/shaders/vertex.glsl");
    const char *pVertexShaderCode = vertexShaderCode.c_str();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &pVertexShaderCode, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader);
    // Create fragment shader
    uint32_t fragmentShader;
    std::string fragmentShaderCode = loadText("../src/shaders/fragment.glsl");
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
    float vertices[] = {
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };
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

    // Generate texture
    std::default_random_engine generator;
    generator.seed((uint32_t)41798);
    std::normal_distribution<float>
        distribution(0.0f, 1.0f);
    float image[COMPUTE_HEIGHT][COMPUTE_WIDTH][4];
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
            image[i][j][0] = redChannel;
            // GREEN
            image[i][j][1] = greenChannel;
            // BLUE
            image[i][j][2] = blueChannel;
            // ALPHA
            image[i][j][3] = alphaChannel;
        }
    }

    std::cout << "BEFORE TEXTURE CREATION" << std::endl;

    // Create OpenGL texture
    uint32_t texture = createComputeTexture(image);

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
            // // render
            // // ------
            // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            // glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(firstPassProgram);
            glDispatchCompute(ceil(COMPUTE_WIDTH / 8), ceil(COMPUTE_HEIGHT / 4), 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            glUseProgram(secondPassProgram);
            glDispatchCompute(ceil(COMPUTE_WIDTH / 8), ceil(COMPUTE_HEIGHT / 4), 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // render container
            glUseProgram(textureProgram);
            glBindTextureUnit(0, texture);
            glUniform1i(glGetUniformLocation(textureProgram, "texture"), 0);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);

            // only set lastFrameTime when you actually draw something
            lastFrameTime = now;
        }
        glfwPollEvents();
    }

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