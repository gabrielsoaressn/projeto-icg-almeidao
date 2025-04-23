#include <GL/glut.h> 
#include <GL/glu.h>
#include <math.h>    
#include <stdio.h>   
//incluindo stb image pra permit eu adicionar textura
#define STB_IMAGE_IMPLEMENTATION // Define isso em UM ARQUIVO .c ou .cpp
#include "stb_image.h" 

// Constantes e Macros
#define PI 3.14159265359
#define GRAUS_PARA_RAD(deg) ((deg) * PI / 180.0f) // Converte graus para radianos

GLuint idTexturaConcreto;
float anguloRotacaoZ = 0.0f;
float anguloRotacaoY = 0.0f;
float anguloRotacaoX = 0.0f;
float cameraDistanciaZ = 5.0f;

const int NUM_BATENTES_POR_ARCO = 15;
const float ALTURA_MIN_REAL = 0.3f;  // 40 cm em metros
const float ALTURA_MAX_REAL = 4.0f;  // 6 m em metros
const float ESCALA_OPENGL = 10.0f; // 1 unidade OpenGL = 10 metros

const float ALTURA_MIN_ESC = ALTURA_MIN_REAL / ESCALA_OPENGL; // Altura escalonada do topo do 1º degrau
const float ALTURA_MAX_ESC = ALTURA_MAX_REAL / ESCALA_OPENGL; // Altura escalonada do topo do último degrau
const float Z_BASE_INICIAL = 0.0f; // Altura da base do primeiro degrau (no chão)

GLuint carregarTextura(const char *nomeArquivo) {
    GLuint idTextura;
    int largura, altura, numCanais;
    stbi_set_flip_vertically_on_load(true); 
    unsigned char *dados = stbi_load(nomeArquivo, &largura, &altura, &numCanais, 0);
    if (dados) {
        GLenum formato = GL_RGB;
        if (numCanais == 1) formato = GL_RED;
        else if (numCanais == 4) formato = GL_RGBA;

        glGenTextures(1, &idTextura);
        glBindTexture(GL_TEXTURE_2D, idTextura);
        glTexImage2D(GL_TEXTURE_2D, 0, formato, largura, altura, 0, formato, GL_UNSIGNED_BYTE, dados);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repetir textura horizontalmente
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repetir textura verticalmente
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtro para diminuir
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtro para aumentar

        stbi_image_free(dados); // Libera memória da imagem original
        glBindTexture(GL_TEXTURE_2D, 0); // Desfaz o bind
        return idTextura;
    } else {
        fprintf(stderr, "Erro ao carregar textura '%s': %s\n", nomeArquivo, stbi_failure_reason());
        return 0;
    }
}

void desenharDegrauArquibancada(float cx, float cy,
    float rx_int, float ry_int, // Raio interno deste degrau
    float rx_ext, float ry_ext, // Raio externo deste degrau
    float z_base, float z_topo, // Z inferior e superior
    float angulo_inicial_graus, float angulo_final_graus,
    int num_segmentos_curva) // Qualidade da curva
{
    if (num_segmentos_curva <= 1) num_segmentos_curva = 2;

    float rad_inicial = GRAUS_PARA_RAD(angulo_inicial_graus);
    float rad_final = GRAUS_PARA_RAD(angulo_final_graus);
    float intervalo_rad = rad_final - rad_inicial;

    // --- 1. Desenhar a Face SUPERIOR (Piso do Degrau) ---
    // Faixa horizontal na altura z_topo
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= num_segmentos_curva; i++) {
    float fracao = (float)i / (float)num_segmentos_curva;
    float angulo_atual_rad = rad_inicial + fracao * intervalo_rad;
    float cos_a = cosf(angulo_atual_rad);
    float sin_a = sinf(angulo_atual_rad);

    // Coordenada S da textura acompanha o ângulo
    float s_coord = fracao * 5.0f; // Repete a textura 5x ao longo do arco (ajuste!)

    // Vértice Externo (t=1.0)
    glTexCoord2f(s_coord, 1.0f);
    glVertex3f(cx + rx_ext * cos_a, cy + ry_ext * sin_a, z_topo);

    // Vértice Interno (t=0.0)
    glTexCoord2f(s_coord, 0.0f);
    glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_topo);
    }
    glEnd();

    // --- 2. Desenhar a Face FRONTAL (Espelho do Degrau) ---
    // Faixa vertical no raio interno (rx_int, ry_int) indo de z_base até z_topo
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= num_segmentos_curva; i++) {
    float fracao = (float)i / (float)num_segmentos_curva;
    float angulo_atual_rad = rad_inicial + fracao * intervalo_rad;
    float cos_a = cosf(angulo_atual_rad);
    float sin_a = sinf(angulo_atual_rad);

    // Coordenada S da textura acompanha o ângulo
    float s_coord = fracao * 5.0f; // Repete a textura 5x ao longo do arco

    // Vértice Superior (t=1.0) no raio interno
    glTexCoord2f(s_coord, 1.0f);
    glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_topo);

    // Vértice Inferior (t=0.0) no raio interno
    glTexCoord2f(s_coord, 0.0f);
    glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_base);
    }
    glEnd();

    // --- Opcional: Desenhar Faces Laterais (Tampas) ---
    // Se desejar fechar as laterais dos degraus
    // Face Inicial
    glBegin(GL_QUADS);
    float cos_ini = cosf(rad_inicial); float sin_ini = sinf(rad_inicial);
    glTexCoord2f(0,0); glVertex3f(cx + rx_int*cos_ini, cy + ry_int*sin_ini, z_base);
    glTexCoord2f(1,0); glVertex3f(cx + rx_ext*cos_ini, cy + ry_ext*sin_ini, z_base);
    glTexCoord2f(1,1); glVertex3f(cx + rx_ext*cos_ini, cy + ry_ext*sin_ini, z_topo);
    glTexCoord2f(0,1); glVertex3f(cx + rx_int*cos_ini, cy + ry_int*sin_ini, z_topo);
    glEnd();
    // Face Final
    glBegin(GL_QUADS);
    float cos_fim = cosf(rad_final); float sin_fim = sinf(rad_final);
    glTexCoord2f(0,0); glVertex3f(cx + rx_int*cos_fim, cy + ry_int*sin_fim, z_base);
    glTexCoord2f(0,1); glVertex3f(cx + rx_int*cos_fim, cy + ry_int*sin_fim, z_topo);
    glTexCoord2f(1,1); glVertex3f(cx + rx_ext*cos_fim, cy + ry_ext*sin_fim, z_topo);
    glTexCoord2f(1,0); glVertex3f(cx + rx_ext*cos_fim, cy + ry_ext*sin_fim, z_base);
    glEnd();

}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f); // Cor base branca (para modular textura)

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, cameraDistanciaZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Aplica Rotações Globais
    glRotatef(anguloRotacaoX, 1.0f, 0.0f, 0.0f);
    glRotatef(anguloRotacaoY, 0.0f, 1.0f, 0.0f);
    glRotatef(anguloRotacaoZ, 0.0f, 0.0f, 1.0f);

    // --- Parâmetros ---
    float centro_x = 0.0f; float centro_y = 0.0f;
    int segmentos_curva = 40; // Qualidade da curva para cada degrau

    // Raios gerais da arquibancada (limites interno e externo)
    float raio_x_geral_int = 0.5f; float raio_y_geral_int = 0.7f;
    float raio_x_geral_ext = 0.8f; float raio_y_geral_ext = 0.95f;

    // Largura radial total
    float largura_radial_x = raio_x_geral_ext - raio_x_geral_int;
    float largura_radial_y = raio_y_geral_ext - raio_y_geral_int;

    // Arcos da arquibancada (ângulos em graus)
    float arcos[][2] = { {0.0f, 60.0f}, {120.0f, 240.0f}, {300.0f, 360.0f} };
    int num_arcos = sizeof(arcos) / sizeof(arcos[0]);

    // --- Desenha as Arquibancadas (Degrau por Degrau) ---
    glBindTexture(GL_TEXTURE_2D, idTexturaConcreto);

    for (int i = 0; i < num_arcos; ++i) { // Loop pelas 3 seções de arquibancada
        float ang_inicio_arco = arcos[i][0];
        float ang_fim_arco = arcos[i][1];

        float z_topo_anterior = Z_BASE_INICIAL; // Z do topo do degrau anterior (começa no chão)

        for (int k = 0; k < NUM_BATENTES_POR_ARCO; ++k) { // Loop pelos 15 degraus (k=0 a 14)

            // Calcula Z (altura) do topo deste degrau (k)
            float fracao_altura = (float)k / (NUM_BATENTES_POR_ARCO - 1); // 0 para k=0, 1 para k=14
            float z_topo_atual = Z_BASE_INICIAL + ALTURA_MIN_ESC + (ALTURA_MAX_ESC - ALTURA_MIN_ESC) * fracao_altura;
            // A base deste degrau é o topo do anterior
            float z_base_atual = z_topo_anterior;

            // Calcula raios interno e externo PARA ESTE DEGRAU (k)
            float fracao_raio_int = (float)k / NUM_BATENTES_POR_ARCO;
            float fracao_raio_ext = (float)(k + 1) / NUM_BATENTES_POR_ARCO;

            float rx_int_k = raio_x_geral_int + largura_radial_x * fracao_raio_int;
            float ry_int_k = raio_y_geral_int + largura_radial_y * fracao_raio_int;
            float rx_ext_k = raio_x_geral_int + largura_radial_x * fracao_raio_ext;
            float ry_ext_k = raio_y_geral_int + largura_radial_y * fracao_raio_ext;

            // Desenha o degrau k
            desenharDegrauArquibancada(centro_x, centro_y,
                                        rx_int_k, ry_int_k, rx_ext_k, ry_ext_k,
                                        z_base_atual, z_topo_atual,
                                        ang_inicio_arco, ang_fim_arco,
                                        segmentos_curva);

            // Atualiza o Z do topo anterior para a próxima iteração
            z_topo_anterior = z_topo_atual;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0); // Desativa textura

    glutSwapBuffers();
}
    

    

void reshape(int largura, int altura) {
    if (altura == 0) altura = 1;
    float proporcao = (float)largura / (float)altura;

    glViewport(0, 0, largura, altura);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();           
    gluPerspective(60.0, proporcao, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // Fundo azul claro
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_DEPTH_TEST);

    idTexturaConcreto = carregarTextura("concreto.jpg");
    if (idTexturaConcreto == 0) {
        fprintf(stderr, "ERRO FATAL: Textura 'concreto.jpg' não carregada!\n");
        exit(1);
    }

}
void teclado(unsigned char key, int x, int y) {
    float incremento = 5.0f; // Quantos graus girar a cada tecla
    float incrementoZoom = 0.2f;
    float zoomMin = 0.5f;
    float zoomMax = 20.0f;

    switch (key) {
        case 'a': 
        case 'A':
            anguloRotacaoZ += incremento;
            break;
        case 'd': 
        case 'D':
            anguloRotacaoZ -= incremento;
            break;
        case 'w': 
        case 'W':
            anguloRotacaoY += incremento; 
            break;
        case 's': 
        case 'S':
            anguloRotacaoY -= incremento; 
            break;
        case 'x':
        case 'X':
            anguloRotacaoX += incremento; 
            break;
        case 'z':
        case 'Z':
            anguloRotacaoX -= incremento; 
            break;

        case 'j': case 'J': // Afastar (aumenta distância)
            cameraDistanciaZ += incrementoZoom;
            if (cameraDistanciaZ > zoomMax) cameraDistanciaZ = zoomMax; // Limita distância máxima
            break;
        case 'k': case 'K': // Aproximar (diminui distância)
            cameraDistanciaZ -= incrementoZoom;
            if (cameraDistanciaZ < zoomMin) cameraDistanciaZ = zoomMin; // Limita distância mínima
            break;

        case 27: // Tecla ESC (ASCII 27)
            exit(0); // Sai do programa
            break;
            
    }glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Almeidao"); 
    init(); 
    glutDisplayFunc(display);
    glutReshapeFunc(reshape); 
    glutKeyboardFunc(teclado);

    printf("Use 'A'/'D' para girar em torno de Z (profundidade).\n");
    printf("Use 'W'/'S' para girar em torno de Y (horizontal).\n");
    printf("Use 'X'/'Z' para girar em torno de X (vertical).\n");

    printf("Pressione ESC para sair.\n");

    glutMainLoop();
    return 0;
}