#include <GL/glut.h> 
#include <math.h>    
#include <stdio.h>   
//incluindo stb image pra permit eu adicionar textura
#define STB_IMAGE_IMPLEMENTATION // Define isso em UM ARQUIVO .c ou .cpp
#include "stb_image.h" 

// Constantes e Macros
#define PI 3.14159265359
#define GRAUS_PARA_RAD(deg) ((deg) * PI / 180.0f) // Converte graus para radianos

GLuint idTexturaConcreto;

GLuint carregarTextura(const char *nomeArquivo) {
    GLuint idTextura;
    int largura, altura, numCanais;
    stbi_set_flip_vertically_on_load(true); // OpenGL espera 0,0 no canto inferior esquerdo
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
        printf("Textura '%s' carregada (ID: %u)\n", nomeArquivo, idTextura);
        return idTextura;
    } else {
        fprintf(stderr, "Erro ao carregar textura '%s': %s\n", nomeArquivo, stbi_failure_reason());
        return 0;
    }
}
void desenharFaixaElipseTexturizada(float cx, float cy, float rx_int, float ry_int, 
    float rx_ext, float ry_ext, float angulo_inicial_graus, float angulo_final_graus, int num_segmentos)
{
    if (num_segmentos <= 1) num_segmentos = 2;

    float rad_inicial = GRAUS_PARA_RAD(angulo_inicial_graus);
    float rad_final = GRAUS_PARA_RAD(angulo_final_graus);
    float intervalo_rad = rad_final - rad_inicial;

    // Usaremos GL_TRIANGLE_STRIP para eficiência
    glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i <= num_segmentos; i++) {
        float fracao = (float)i / (float)num_segmentos;
        float angulo_atual_rad = rad_inicial + fracao * intervalo_rad;

        float cos_a = cosf(angulo_atual_rad);
        float sin_a = sinf(angulo_atual_rad);

        // Calcula ponto externo
        float x_ext = cx + rx_ext * cos_a;
        float y_ext = cy + ry_ext * sin_a;

        // Calcula ponto interno
        float x_int = cx + rx_int * cos_a;
        float y_int = cy + ry_int * sin_a;

        // Coordenadas de Textura:
        // s (horizontal) varia com a fração do arco
        // t (vertical) é 0.0 para interno, 1.0 para externo
        float s_coord = fracao; // Mapeia [0, 1] ao longo do arco angular
        // (Alternativa: mapear baseado em comprimento de arco, mais complexo)

        // Vértice Externo (t=1.0)
        glTexCoord2f(s_coord, 1.0f);
        glVertex2f(x_ext, y_ext);

        // Vértice Interno (t=0.0)
        glTexCoord2f(s_coord, 0.0f);
        glVertex2f(x_int, y_int);
    }
    glEnd();
}

void desenharContornoArcoElipse(float cx, float cy, float rx, float ry,
                           float start_angle_deg, float end_angle_deg, int num_segments)
{    if (num_segments <= 0) num_segments = 1;

    float start_rad = GRAUS_PARA_RAD(start_angle_deg);
    float end_rad = GRAUS_PARA_RAD(end_angle_deg);
    float angle_range = end_rad - start_rad;

    glBegin(GL_LINE_STRIP);

    for (int i = 0; i <= num_segments; i++) { 
        float fraction = (float)i / (float)num_segments;
        float theta = start_rad + fraction * angle_range;

        //fórmula da elipse
        float x = rx * cosf(theta);
        float y = ry * sinf(theta);

        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Define a cor base (branco, será multiplicada pela textura se o modo for GL_MODULATE)
    glColor3f(1.0f, 1.0f, 1.0f);

    // --- Parâmetros das Elipses ---
    float centro_x = 0.0f;
    float centro_y = 0.0f;
    int segmentos_arco_curto = 40;
    int segmentos_arco_longo = 60; // Para o arco 120-240

    // Define os raios internos e externos DIRETAMENTE
    float raio_x_int = 0.5f;
    float raio_y_int = 0.7f;
    float raio_x_ext = 0.8f; // Seu raio externo X
    float raio_y_ext = 0.95f; // Seu raio externo Y

    // ---> ATIVA A TEXTURA ANTES DE DESENHAR <---
    glBindTexture(GL_TEXTURE_2D, idTexturaConcreto);

    
    //    Substitui TODAS as chamadas anteriores para desenhar contornos e linhas

    // Faixa 1: 0 a 60 graus
    desenharFaixaElipseTexturizada(centro_x, centro_y,
                                    raio_x_int, raio_y_int, raio_x_ext, raio_y_ext,
                                    0.0f, 60.0f, segmentos_arco_curto);

    // Faixa 2: 120 a 240 graus
    desenharFaixaElipseTexturizada(centro_x, centro_y,
                                    raio_x_int, raio_y_int, raio_x_ext, raio_y_ext,
                                    120.0f, 240.0f, segmentos_arco_longo);

    // Faixa 3: 300 a 360 graus
    desenharFaixaElipseTexturizada(centro_x, centro_y,
                                    raio_x_int, raio_y_int, raio_x_ext, raio_y_ext,
                                    300.0f, 360.0f, segmentos_arco_curto); // Usei segmentos_arco_curto aqui, ajuste se necessário

    
    glBindTexture(GL_TEXTURE_2D, 0);

    glFlush();
    
}

void reshape(int width, int height) {
    if (height == 0) height = 1;
    float aspect = (float)width / (float)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();          

    if (width >= height) {
        gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
    } else {
        gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init() {
    //cor do céu
    glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    idTexturaConcreto = carregarTextura("concreto.jpg");
    if (idTexturaConcreto == 0) {
        fprintf(stderr, "ERRO FATAL: Textura 'concreto.jpg' não carregada!\n");
        exit(1);
    }

}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

    glutInitWindowSize(600, 600);

    glutInitWindowPosition(100, 100);

    // Cria a janela com um título
    glutCreateWindow("Almeidão");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape); 

    glutMainLoop();

    return 0;
}