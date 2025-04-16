#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Use #define STB_IMAGE_IMPLEMENTATION em UM .cpp file (aqui, por exemplo)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Certifique-se que este arquivo está no seu include path ou na pasta

#include <iostream>
#include <vector>
#include <string>

// Protótipos de Funções
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void setupField(unsigned int& VAO, unsigned int& VBO);
void setupStands(unsigned int& VAO, unsigned int& VBO);
void setupGoal(unsigned int& VAO, unsigned int& VBO);

// Configurações da Janela
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 700;

// Câmera
glm::vec3 cameraPos   = glm::vec3(0.0f, 20.0f, 100.0f); // Posição inicial elevada olhando pro campo
glm::vec3 cameraFront = glm::vec3(0.0f, -0.2f, -1.0f); // Olhando levemente para baixo e para frente
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.5f; // Velocidade de movimento da câmera

bool firstMouse = true;
float yaw   = -90.0f; // Yaw inicializado para olhar para -Z
float pitch = -11.5f; // Pitch inicializado para corresponder ao cameraFront inicial (aprox)
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float fov   = 45.0f; // Campo de visão (pode ser ajustado com scroll, opcional)
float mouseSensitivity = 0.1f;

// Timing
float deltaTime = 0.0f; // Tempo entre o frame atual e o último
float lastFrame = 0.0f;

// Shaders (Simples)
const char *vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal; // Adicionado Normal
    layout (location = 2) in vec2 aTexCoords; // Adicionado TexCoords

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoords;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal; // Transformar normais corretamente
        TexCoords = aTexCoords;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)glsl";

const char *fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoords;

    uniform sampler2D texture1; // Textura principal (grama, concreto)
    uniform vec3 objectColor;   // Cor base do objeto (se não usar textura)
    uniform vec3 lightColor;    // Cor da luz
    uniform vec3 lightPos;      // Posição da luz (usaremos como direção para luz direcional)
    uniform vec3 viewPos;       // Posição da câmera

    uniform bool useTexture;    // Flag para usar textura ou cor sólida

    void main()
    {
        vec3 resultColor;
        if(useTexture)
        {
            resultColor = texture(texture1, TexCoords).rgb;
        }
        else
        {
            resultColor = objectColor;
        }

        // Iluminação Ambiente Simples
        float ambientStrength = 0.8;
        vec3 ambient = ambientStrength * lightColor;

        // Iluminação Difusa Simples (Luz Direcional)
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(-lightPos); // Direção da luz vindo "de" lightPos
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Combina Ambiente e Difusa
        vec3 lighting = (ambient + diffuse) * resultColor;
        FragColor = vec4(lighting, 1.0);
    }
)glsl";


int main()
{
    // --- Inicialização GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // --- Criação da Janela ---
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Estádio Almeidão", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Captura o cursor
    glfwSetCursorPosCallback(window, mouse_callback); // Registra a função de callback
    // --- Inicialização GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    // --- Configurar Estado OpenGL ---
    glEnable(GL_DEPTH_TEST); // Habilitar teste de profundidade

    // --- Compilar Shaders ---
    // Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Checar erros de compilação (omitido por brevidade, mas importante em projetos reais)

    // Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Checar erros de compilação (omitido por brevidade)

    // Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Checar erros de link (omitido por brevidade)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- Preparar Geometria ---
    unsigned int fieldVAO, fieldVBO;
    setupField(fieldVAO, fieldVBO);

    unsigned int standsVAO, standsVBO;
    setupStands(standsVAO, standsVBO); // Arquibancada básica

    unsigned int goalVAO, goalVBO;
    setupGoal(goalVAO, goalVBO); // Gol básico

    // --- Carregar Texturas ---
    unsigned int grassTexture = loadTexture("grama.png"); 
    unsigned int concreteTexture = loadTexture("concreto.png");

    if (grassTexture == 0) {
         std::cerr << "WARN: Nao foi possivel carregar grama.png, campo ficara branco." << std::endl;
    }
     if (concreteTexture == 0) {
         std::cerr << "WARN: Nao foi possivel carregar concreto.png, arquibancada usara cor solida." << std::endl;
    }

    // Configurar unidade de textura no shader (só precisa fazer uma vez)
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); // Usa a unidade de textura 0

    // --- Loop de Renderização ---
    while (!glfwWindowShouldClose(window))
    {
        // --- Lógica por Frame (Tempo) ---
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- Input ---
        processInput(window);

        // --- Renderização ---
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Cor do céu (azul claro)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Ativar Shader
        glUseProgram(shaderProgram);

        // Configurar Matrizes de View/Projection (Câmera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Configurar Iluminação e Posição da Câmera (para specular, se adicionado)
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(glm::vec3(50.0f, 100.0f, 20.0f))); // Luz vindo de cima/lado
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f))); // Luz branca
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));


        // --- Desenhar Campo ---
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1); // Usar textura
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glBindVertexArray(fieldVAO);
        glm::mat4 model = glm::mat4(1.0f); // Matriz identidade (sem transformação extra)
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vértices para o quad

        // --- Desenhar Arquibancadas ---
        glBindVertexArray(standsVAO);
        if (concreteTexture != 0) {
             glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1); // Usar textura de concreto se carregada
             glBindTexture(GL_TEXTURE_2D, concreteTexture);
        } else {
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Usar cor sólida
            glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.6f, 0.6f, 0.6f); // Cor cinza
        }

        // Posições das arquibancadas (exemplo básico)
        float fieldWidth = 105.0f;
        float fieldLength = 68.0f;
        float standDepth = 20.0f; // Profundidade da arquibancada
        float standHeight = 10.0f; // Altura da arquibancada
        float standOffset = 5.0f; // Distância do campo

        // Arquibancada Lateral 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, standHeight / 2.0f, (fieldLength / 2.0f) + standOffset + (standDepth / 2.0f)));
        model = glm::scale(model, glm::vec3(fieldWidth, standHeight, standDepth)); // Escala para formar o bloco
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vértices para um cubo

        // Arquibancada Lateral 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, standHeight / 2.0f, -(fieldLength / 2.0f) - standOffset - (standDepth / 2.0f)));
        model = glm::scale(model, glm::vec3(fieldWidth, standHeight, standDepth));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- Desenhar Gols ---
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Não usar textura para os gols
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.9f, 0.9f, 0.9f); // Cor branca/cinza claro
        glBindVertexArray(goalVAO);

        // Gol 1 (Posicionar no final do campo)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((fieldWidth / 2.0f), 0.0f, 0.0f)); // Posição x
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // Desenha a estrutura base do gol

        // Gol 2 (Posicionar no outro final do campo)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-(fieldWidth / 2.0f), 0.0f, 0.0f)); // Posição x oposta
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Desabilitar VAO
        glBindVertexArray(0);

        // --- Troca de Buffers e Eventos ---
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Limpeza ---
    glDeleteVertexArrays(1, &fieldVAO);
    glDeleteBuffers(1, &fieldVBO);
    glDeleteVertexArrays(1, &standsVAO);
    glDeleteBuffers(1, &standsVBO);
    glDeleteVertexArrays(1, &goalVAO);
    glDeleteBuffers(1, &goalVBO);
    glDeleteProgram(shaderProgram);
    if (grassTexture != 0) glDeleteTextures(1, &grassTexture);
    if (concreteTexture != 0) glDeleteTextures(1, &concreteTexture);


    glfwTerminate();
    return 0;
}

// Função para processar input (movimento da câmera)
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float currentCameraSpeed = cameraSpeed * deltaTime * 50; // Ajustar velocidade com base no tempo
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;
     if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // Subir
        cameraPos += currentCameraSpeed * cameraUp;
     if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) // Descer
        cameraPos -= currentCameraSpeed * cameraUp;
}

// Callback de redimensionamento da janela
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Invertido pois coordenadas Y vão de baixo para cima
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Limita o pitch para não inverter a câmera
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Calcula o novo vetor cameraFront
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
// Função para carregar textura
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true); // Inverte a textura no carregamento (OpenGL espera coords Y de baixo pra cima)
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
             std::cerr << "Formato de imagem nao suportado para: " << path << std::endl;
             stbi_image_free(data);
             return 0; // Retorna 0 em caso de erro de formato
        }


        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Configurações de wrapping/filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Falha ao carregar textura: " << path << std::endl;
        stbi_image_free(data); // Mesmo que data seja NULL, é seguro chamar
        return 0; // Retorna 0 em caso de erro de carregamento
    }

    return textureID;
}

// Função para configurar a geometria do campo
void setupField(unsigned int& VAO, unsigned int& VBO) {
    // Tamanho padrão do campo (aproximado)
    float width = 105.0f / 2.0f; // Metade da largura
    float length = 68.0f / 2.0f; // Metade do comprimento

    // Vértices (Posição, Normal para cima, Coordenada de Textura)
     float fieldVertices[] = {
        // Posições          // Normais         // Coords Textura
         width, 0.0f,  length,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f, // Top Right
         width, 0.0f, -length,  0.0f, 1.0f, 0.0f,  10.0f, 0.0f,  // Bottom Right
        -width, 0.0f, -length,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,   // Bottom Left
        -width, 0.0f, -length,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,   // Bottom Left
        -width, 0.0f,  length,  0.0f, 1.0f, 0.0f,  0.0f, 10.0f,  // Top Left
         width, 0.0f,  length,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f  // Top Right
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fieldVertices), fieldVertices, GL_STATIC_DRAW);

    // Atributo Posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atributo Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Atributo Coordenada de Textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Função para configurar a geometria das arquibancadas (cubo básico)
void setupStands(unsigned int& VAO, unsigned int& VBO) {
    // Vértices de um cubo (posição, normal, texcoord - texcoord simples aqui)
    // As normais são essenciais para a iluminação funcionar
    float cubeVertices[] = {
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Atributo Posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atributo Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Atributo Coordenada de Textura (mesmo que não use sempre, o shader espera)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Função para configurar a geometria do gol (usa a mesma geometria do cubo, mas escalada depois)
void setupGoal(unsigned int& VAO, unsigned int& VBO) {
    // Usa a mesma função das arquibancadas, pois a geometria base (cubo) é a mesma
    // A diferença será na matriz 'model' aplicada no loop de renderização
    setupStands(VAO, VBO); // Reutiliza a criação do VAO/VBO do cubo
    // TODO: Poderia criar uma geometria mais específica (cilindros finos),
    // mas para este exemplo básico, escalar um cubo é mais simples.
    // No loop principal, vamos escalar este cubo para parecer um gol.

     // Vértices de uma estrutura de gol mais "realista" (ainda simples)
    float goalHeight = 2.44f;
    float goalWidth = 7.32f;
    float postRadius = 0.1f; // Raio das traves (aproximado)

    // Simplificação: Usaremos cubos escalados em vez de cilindros por agora
    // Vértices para as 3 partes do gol (2 postes, 1 travessão)
    // Usaremos o mesmo VAO/VBO do cubo genérico, e aplicaremos transformações
    // para cada parte no loop de renderização.
    // A função setupStands já cria um VAO/VBO de cubo unitário.

    // Vamos redefinir a função para criar um VAO/VBO específico para o gol
    // que representa as 3 partes juntas, embora ainda usando a geometria base do cubo.
    // (Alternativa: Desenhar 3 cubos escalados no loop principal usando o VAO do cubo)

     // Reutilizar setupStands é mais simples para este exemplo.
     // A "forma" do gol será definida pelas transformações 'model' no draw loop.
     // Exemplo de como ficariam as transforms no draw loop:
     /*
        // Trave Esquerda
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, goalHeight/2.0f, -goalWidth/2.0f));
        model = glm::scale(model, glm::vec3(postRadius*2, goalHeight, postRadius*2));
        // desenhar...

        // Trave Direita
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, goalHeight/2.0f, goalWidth/2.0f));
        model = glm::scale(model, glm::vec3(postRadius*2, goalHeight, postRadius*2));
        // desenhar...

        // Travessão
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, goalHeight, 0.0f));
        model = glm::scale(model, glm::vec3(postRadius*2, postRadius*2, goalWidth));
        // desenhar...
     */
     // Para este exemplo, vamos desenhar UM ÚNICO CUBO escalado representando a "caixa" do gol
     // na função principal, por simplicidade. A função setupGoal apenas garante
     // que temos a geometria base do cubo carregada.
}