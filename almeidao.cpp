#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>

// --- Integração com stb_image.h ---
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// ---------------------------------

#define PI 3.14159265359
#define GRAUS_PARA_RAD(graus) ((graus) * PI / 180.0f)

// --- Constantes e Variáveis Globais ---
GLuint idTexturaConcreto;
GLuint idTexturaTerra;
GLuint idTexturaConcretoExterno;
float anguloRotacaoZ = 0.0f;
float anguloRotacaoY = 90.0f;
float anguloRotacaoX = 0.0f;
float cameraDistanciaZ = 5.0f;

const int NUM_DEGRAUS = 15;
const float ALTURA_MIN_REAL = 0.2f;
const float ALTURA_MAX_REAL = 3.0f;
const float ESCALA_OPENGL = 10.0f;
const float ALTURA_MIN_ESC = ALTURA_MIN_REAL / ESCALA_OPENGL;
const float ALTURA_MAX_ESC = ALTURA_MAX_REAL / ESCALA_OPENGL;
const float Z_BASE_INICIAL = 0.0f;
const float Z_CHAO = -0.01f;
const float INCLINACAO_PAREDE_OFFSET = 0.05f;

// --- Funções Utilitárias (nomes mantidos em português, pois foram criadas por nós) ---
GLuint carregarTextura(const char *nomeArquivo) {
GLuint idTextura; int largura, altura, numCanais; stbi_set_flip_vertically_on_load(true); unsigned char *dados = stbi_load(nomeArquivo, &largura, &altura, &numCanais, 0); if (dados) { GLenum formato = GL_RGB; if (numCanais == 1) formato = GL_RED; else if (numCanais == 4) formato = GL_RGBA; glGenTextures(1, &idTextura); glBindTexture(GL_TEXTURE_2D, idTextura); glTexImage2D(GL_TEXTURE_2D, 0, formato, largura, altura, 0, formato, GL_UNSIGNED_BYTE, dados); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); stbi_image_free(dados); glBindTexture(GL_TEXTURE_2D, 0); printf("Textura '%s' carregada (ID: %u)\n", nomeArquivo, idTextura); return idTextura; } else { fprintf(stderr, "Erro ao carregar textura '%s': %s\n", nomeArquivo, stbi_failure_reason()); return 0; }
}
void desenharDegrauArquibancada(float cx, float cy, float rx_int, float ry_int, float rx_ext, float ry_ext, float z_base, float z_topo, float angulo_inicial_graus, float angulo_final_graus, int num_segmentos_curva) {
if (num_segmentos_curva <= 1) num_segmentos_curva = 2; float rad_inicial = GRAUS_PARA_RAD(angulo_inicial_graus); float rad_final = GRAUS_PARA_RAD(angulo_final_graus); float intervalo_rad = rad_final - rad_inicial; glBegin(GL_TRIANGLE_STRIP); for (int i = 0; i <= num_segmentos_curva; i++) { float fracao = (float)i / (float)num_segmentos_curva; float angulo_atual_rad = rad_inicial + fracao * intervalo_rad; float cos_a = cosf(angulo_atual_rad); float sin_a = sinf(angulo_atual_rad); float s_coord = fracao * 5.0f; glTexCoord2f(s_coord, 1.0f); glVertex3f(cx + rx_ext * cos_a, cy + ry_ext * sin_a, z_topo); glTexCoord2f(s_coord, 0.0f); glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_topo); } glEnd(); glBegin(GL_TRIANGLE_STRIP); for (int i = 0; i <= num_segmentos_curva; i++) { float fracao = (float)i / (float)num_segmentos_curva; float angulo_atual_rad = rad_inicial + fracao * intervalo_rad; float cos_a = cosf(angulo_atual_rad); float sin_a = sinf(angulo_atual_rad); float s_coord = fracao * 5.0f; glTexCoord2f(s_coord, 1.0f); glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_topo); glTexCoord2f(s_coord, 0.0f); glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_base); } glEnd();
}
void desenharParedeExterna(float cx, float cy, float rx_base, float ry_base, float rx_topo, float ry_topo, float z_baixo, float z_alto, float angulo_inicial_graus, float angulo_final_graus, int num_segmentos_curva) {
if (num_segmentos_curva <= 1) num_segmentos_curva = 2; float rad_inicial = GRAUS_PARA_RAD(angulo_inicial_graus); float rad_final = GRAUS_PARA_RAD(angulo_final_graus); float intervalo_rad = rad_final - rad_inicial; glBegin(GL_TRIANGLE_STRIP); for (int i = 0; i <= num_segmentos_curva; i++) { float fracao = (float)i / (float)num_segmentos_curva; float angulo_atual_rad = rad_inicial + fracao * intervalo_rad; float cos_a = cosf(angulo_atual_rad); float sin_a = sinf(angulo_atual_rad); float s_coord = fracao * 10.0f; glTexCoord2f(s_coord, 1.0f); glVertex3f(cx + rx_topo * cos_a, cy + ry_topo * sin_a, z_alto); glTexCoord2f(s_coord, 0.0f); glVertex3f(cx + rx_base * cos_a, cy + ry_base * sin_a, z_baixo); } glEnd();
}
void desenharTampaLateral(float cx, float cy, float angulo_graus, float rx_int_base, float ry_int_base, float z_int_base, float rx_ext_base, float ry_ext_base, float z_ext_base, float rx_ext_topo, float ry_ext_topo, float z_ext_topo, float rx_int_topo, float ry_int_topo, float z_int_topo) {
float angulo_rad = GRAUS_PARA_RAD(angulo_graus); float cos_a = cosf(angulo_rad); float sin_a = sinf(angulo_rad); float v1[3] = {cx + rx_int_base * cos_a, cy + ry_int_base * sin_a, z_int_base}; float v2[3] = {cx + rx_ext_base * cos_a, cy + ry_ext_base * sin_a, z_ext_base}; float v3[3] = {cx + rx_ext_topo * cos_a, cy + ry_ext_topo * sin_a, z_ext_topo}; float v4[3] = {cx + rx_int_topo * cos_a, cy + ry_int_topo * sin_a, z_int_topo}; glBegin(GL_QUADS); glTexCoord2f(0.0f, 0.0f); glVertex3fv(v1); glTexCoord2f(1.0f, 0.0f); glVertex3fv(v2); glTexCoord2f(1.0f, 1.0f); glVertex3fv(v3); glTexCoord2f(0.0f, 1.0f); glVertex3fv(v4); glEnd();
}

// --- Função de callback: Desenho ---
void display() { // <<< RENOMEADO
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      
// Configurações de Câmera e Rotação
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
gluLookAt(0.0, 0.0, cameraDistanciaZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
glRotatef(anguloRotacaoX, 1.0f, 0.0f, 0.0f);
glRotatef(anguloRotacaoY, 0.0f, 1.0f, 0.0f);
glRotatef(anguloRotacaoZ, 0.0f, 0.0f, 1.0f);

// 1. Desenhar o Chão
glColor3f(0.8f, 0.8f, 0.8f);
glBindTexture(GL_TEXTURE_2D, idTexturaTerra);
float tamChao = 20.0f; float repTexturaChao = 15.0f;
glBegin(GL_QUADS); /* ... vértices ... */
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-tamChao, -tamChao, Z_CHAO); glTexCoord2f(repTexturaChao, 0.0f); glVertex3f( tamChao, -tamChao, Z_CHAO); glTexCoord2f(repTexturaChao, repTexturaChao); glVertex3f( tamChao,  tamChao, Z_CHAO); glTexCoord2f(0.0f, repTexturaChao); glVertex3f(-tamChao,  tamChao, Z_CHAO);
glEnd();

// Parâmetros Comuns
float centro_x = 0.0f; float centro_y = 0.0f;
int segmentos_curva_degrau = 40; int segmentos_curva_parede = 60;
float raio_x_geral_int = 0.5f; float raio_y_geral_int = 0.7f;
float raio_x_geral_ext = 0.8f; float raio_y_geral_ext = 0.95f;
float largura_radial_x = raio_x_geral_ext - raio_x_geral_int; float largura_radial_y = raio_y_geral_ext - raio_y_geral_int;
float arcos_principais[][2] = { {0.0f, 60.0f}, {120.0f, 240.0f}, {300.0f, 360.0f} }; int num_arcos_principais = sizeof(arcos_principais) / sizeof(arcos_principais[0]);
float arcos_conexao[][2] = { {60.0f, 120.0f}, {240.0f, 300.0f} }; int num_arcos_conexao = sizeof(arcos_conexao) / sizeof(arcos_conexao[0]);
float altura_arco_conexao = ALTURA_MAX_ESC / 2.0f;

// 2. Desenhar Arquibancadas (Degraus)
glColor3f(1.0f, 1.0f, 1.0f);
glBindTexture(GL_TEXTURE_2D, idTexturaConcreto);
for (int i = 0; i < num_arcos_principais; ++i) { /* ... loop degraus ... */
     float ang_inicio_arco = arcos_principais[i][0]; float ang_fim_arco = arcos_principais[i][1]; float z_topo_anterior = Z_BASE_INICIAL; for (int k = 0; k < NUM_DEGRAUS; ++k) { float fracao_altura = (float)k / (NUM_DEGRAUS - 1); float z_topo_atual = Z_BASE_INICIAL + ALTURA_MIN_ESC + (ALTURA_MAX_ESC - ALTURA_MIN_ESC) * fracao_altura; float z_base_atual = z_topo_anterior; float fracao_raio_int = (float)k / NUM_DEGRAUS; float fracao_raio_ext = (float)(k + 1) / NUM_DEGRAUS; float rx_int_k = raio_x_geral_int + largura_radial_x * fracao_raio_int; float ry_int_k = raio_y_geral_int + largura_radial_y * fracao_raio_int; float rx_ext_k = raio_x_geral_int + largura_radial_x * fracao_raio_ext; float ry_ext_k = raio_y_geral_int + largura_radial_y * fracao_raio_ext; desenharDegrauArquibancada(centro_x, centro_y, rx_int_k, ry_int_k, rx_ext_k, ry_ext_k, z_base_atual, z_topo_atual, ang_inicio_arco, ang_fim_arco, segmentos_curva_degrau); z_topo_anterior = z_topo_atual; }
}

// 3. Desenhar Paredes Externas e Tampas
glColor3f(0.9f, 0.9f, 0.9f);
glBindTexture(GL_TEXTURE_2D, idTexturaConcretoExterno);
float rx_base_parede = raio_x_geral_ext; float ry_base_parede = raio_y_geral_ext;
float rx_topo_parede = rx_base_parede + INCLINACAO_PAREDE_OFFSET; float ry_topo_parede = ry_base_parede + INCLINACAO_PAREDE_OFFSET;

// Paredes Principais
for (int i = 0; i < num_arcos_principais; ++i) { /* ... desenha parede principal ... */
     float ang_inicio_arco = arcos_principais[i][0]; float ang_fim_arco = arcos_principais[i][1]; desenharParedeExterna(centro_x, centro_y, rx_base_parede, ry_base_parede, rx_topo_parede, ry_topo_parede, Z_BASE_INICIAL, ALTURA_MAX_ESC, ang_inicio_arco, ang_fim_arco, segmentos_curva_parede);
}
// Paredes de Conexão
for (int i = 0; i < num_arcos_conexao; ++i) { /* ... desenha parede conexão ... */
    float ang_inicio_arco = arcos_conexao[i][0]; float ang_fim_arco = arcos_conexao[i][1]; desenharParedeExterna(centro_x, centro_y, rx_base_parede, ry_base_parede, rx_topo_parede, ry_topo_parede, Z_BASE_INICIAL, altura_arco_conexao, ang_inicio_arco, ang_fim_arco, segmentos_curva_parede);
}
// Tampas Laterais
for (int i = 0; i < num_arcos_principais; ++i) { /* ... desenha tampas ... */
     float ang_inicio_arco = arcos_principais[i][0]; float ang_fim_arco = arcos_principais[i][1]; desenharTampaLateral(centro_x, centro_y, ang_inicio_arco, raio_x_geral_int, raio_y_geral_int, Z_BASE_INICIAL, rx_base_parede, ry_base_parede, Z_BASE_INICIAL, rx_topo_parede, ry_topo_parede, ALTURA_MAX_ESC, raio_x_geral_int, raio_y_geral_int, ALTURA_MIN_ESC); desenharTampaLateral(centro_x, centro_y, ang_fim_arco, raio_x_geral_int, raio_y_geral_int, Z_BASE_INICIAL, rx_base_parede, ry_base_parede, Z_BASE_INICIAL, rx_topo_parede, ry_topo_parede, ALTURA_MAX_ESC, raio_x_geral_int, raio_y_geral_int, ALTURA_MIN_ESC);
}

glBindTexture(GL_TEXTURE_2D, 0);
glutSwapBuffers();
}

// --- Função de callback: Inicialização ---
void init() { // <<< RENOMEADO
glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
glEnable(GL_TEXTURE_2D);
glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
glEnable(GL_DEPTH_TEST);

      
idTexturaConcreto = carregarTextura("concreto.jpg");
if (idTexturaConcreto == 0) { fprintf(stderr, "ERRO FATAL: Textura 'concreto.jpg' não carregada!\n"); exit(1); }
idTexturaTerra = carregarTextura("terra.jpeg");
if (idTexturaTerra == 0) { fprintf(stderr, "ERRO FATAL: Textura 'terra.jpg' não carregada!\n"); exit(1); }
idTexturaConcretoExterno = carregarTextura("concreto_externo.jpg");
if (idTexturaConcretoExterno == 0) { fprintf(stderr, "ERRO FATAL: Textura 'concreto_externo.jpg' não carregada!\n"); exit(1); }


}

// --- Função de callback: Redimensionamento ---
void reshape(int largura, int altura) { // <<< RENOMEADO
if (altura == 0) altura = 1; float proporcao = (float)largura / (float)altura;
glViewport(0, 0, largura, altura); glMatrixMode(GL_PROJECTION); glLoadIdentity();
gluPerspective(60.0, proporcao, 0.1, 100.0); glMatrixMode(GL_MODELVIEW);
}

// --- Função de callback: Teclado ---
void keyboard(unsigned char key, int x, int y) { // <<< RENOMEADO
float incrementoRot = 5.0f; float incrementoZoom = 0.2f; float zoomMin = 0.5f; float zoomMax = 20.0f;
switch (key) {
case 'a': case 'A': anguloRotacaoZ += incrementoRot; break;
case 'd': case 'D': anguloRotacaoZ -= incrementoRot; break;
case 'w': case 'W': anguloRotacaoY += incrementoRot; break;
case 's': case 'S': anguloRotacaoY -= incrementoRot; break;
case 'x': case 'X': anguloRotacaoX += incrementoRot; break;
case 'z': case 'Z': anguloRotacaoX -= incrementoRot; break;
case 'j': case 'J': cameraDistanciaZ += incrementoZoom; if (cameraDistanciaZ > zoomMax) cameraDistanciaZ = zoomMax; break;
case 'k': case 'K': cameraDistanciaZ -= incrementoZoom; if (cameraDistanciaZ < zoomMin) cameraDistanciaZ = zoomMin; break;
case 27: exit(0); break; // ESC
}
glutPostRedisplay();
}

// --- Função Principal ---
int main(int numArgumentos, char** argumentos) {
glutInit(&numArgumentos, argumentos);
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
glutInitWindowSize(1200, 800);
glutInitWindowPosition(100, 100);
glutCreateWindow("Almeidão Simulado - Nomes em Inglês"); // Título atualizado

      
init(); // <<< RENOMEADO: Chama a função de inicialização

// ---> REGISTRA CALLBACKS COM NOMES EM INGLÊS <---
glutDisplayFunc(display);   // Função de desenho
glutReshapeFunc(reshape);   // Função de redimensionamento
glutKeyboardFunc(keyboard); // Função de teclado

printf("Use A/D (Z), W/S (Y), X/Z (X) para girar.\nUse J/K para Zoom.\nESC para sair.\n");
glutMainLoop();
return 0;

}