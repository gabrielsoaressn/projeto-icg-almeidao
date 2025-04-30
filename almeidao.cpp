#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h> 

// --- Integração com stb_image.h ---
// Define esta macro em *um* arquivo .c ou .cpp antes de incluir stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#define FLOAT_COMPARISON_TOLERANCE 1e-5
#include "stb_image.h"
// ---------------------------------

// --- Constantes Matemáticas ---
#define PI 3.14159265359
#define GRAUS_PARA_RAD(graus) ((graus) * PI / 180.0f)

// --- Constantes e Variáveis Globais ---

// IDs das Texturas OpenGL
GLuint idTexturaConcreto;
GLuint idTexturaTerra;
GLuint idTexturaConcretoExterno;

// Variáveis de Câmera/Rotação
float anguloRotacaoZ = 0.0f;    // Rotação em torno do eixo Z (vista de cima)
float anguloRotacaoY = 0.0f;   // Rotação em torno do eixo Y (inclinação vertical inicial)
float anguloRotacaoX = 280.0f;    // Rotação em torno do eixo X
float cameraDistanciaZ = 5.0f;  // Distância da câmera ao centro (zoom)

// Constantes da Geometria da Arquibancada/Estádio
const int NUM_DEGRAUS = 15;                 // Número de degraus na arquibancada
const float ALTURA_MIN_REAL = 0.2f;         // Altura mínima da estrutura em "unidades reais" (ex: metros)
const float ALTURA_MAX_REAL = 3.0f;         // Altura máxima da estrutura em "unidades reais"
const float ESCALA_OPENGL = 10.0f;          // Fator de escala para converter unidades reais para OpenGL
// Alturas escaladas para uso no OpenGL
const float ALTURA_MIN_ESC = ALTURA_MIN_REAL / ESCALA_OPENGL; // Altura inicial/conexão em unidades OpenGL
const float ALTURA_MAX_ESC = ALTURA_MAX_REAL / ESCALA_OPENGL; // Altura máxima em unidades OpenGL
const float Z_BASE_INICIAL = 0.0f;          // Coordenada Z da base dos degraus/paredes
const float Z_CHAO = -0.01f;                // Coordenada Z do plano do chão (ligeiramente abaixo da base)
const float INCLINACAO_PAREDE_OFFSET = 0.05f; // Deslocamento radial no topo da parede para criar inclinação

bool leftMousePressed = false;
int lastMouseX = 0;
float cameraAngle = 0.0f; // Ângulo da câmera em graus
float camera_position[3] = {0.0f, 5.0f, 20.0f}; // Posição inicial
float camera_target[3] = {0.0f, 0.0f, 0.0f};    // Ponto onde a câmera olha
float zoomSpeed = 0.5f;

// Parâmetro de controle da curva (de 0.0 a 1.0)
float t = 0.0f;

// Pontos de controle da curva de Bézier
float p0[3] = {0.0f, 5.0f, 10.0f};
float p1[3] = {5.0f, 8.0f, 5.0f};
float p2[3] = {10.0f, 2.0f, 5.0f};
float p3[3] = {15.0f, 5.0f, 10.0f};

bool mouseEsquerdoPressionado = false;
int ultimoXMouse = 0;

int modoNoite = 0;            // 0 = dia, 1 = noite
float alphaFiltro = 0.0f;     // 0.0 = dia, 1.0 = noite
float passoTransicao = 1.0f / 300.0f; // Transição de 30 segundos

void atualizarTransicao(int valor) {
    if (modoNoite && alphaFiltro < 1.5f) {
        alphaFiltro += passoTransicao;
        if (alphaFiltro > 1.5f) alphaFiltro = 1.5f;
    } else if (!modoNoite && alphaFiltro > 0.0f) {
        alphaFiltro -= passoTransicao;
        if (alphaFiltro < 0.0f) alphaFiltro = 0.0f;
    }

    glutPostRedisplay();  // Atualiza a tela
    glutTimerFunc(100, atualizarTransicao, 0);  // Chama a função novamente após 100ms
}


//  Função para capturar o clique do mouse
void mouse(int botao, int estado, int x, int y) {
    if (botao == GLUT_LEFT_BUTTON) {
        if (estado == GLUT_DOWN) {
            mouseEsquerdoPressionado = true;
            ultimoXMouse = x;
        } else if (estado == GLUT_UP) {
            mouseEsquerdoPressionado = false;
        }
    }
}


// Mouse - Pressionar ou soltar botão
void motion(int x, int y) {
    if (mouseEsquerdoPressionado) {
        int deltaX = x - ultimoXMouse;
        cameraAngle += deltaX * 0.5f; // Sensibilidade da rotação
        if (cameraAngle > 360.0f) cameraAngle -= 360.0f;
        if (cameraAngle < 0.0f) cameraAngle += 360.0f;
        ultimoXMouse = x;
        glutPostRedisplay(); // Pede novo desenho da cena
    }
}


// Função para calcular o ponto na curva de Bézier
void bezier_curve(float t, float *p0, float *p1, float *p2, float *p3, float *point) {
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float ttt = tt * t;
    float uuu = uu * u;

    point[0] = uuu * p0[0] + 3 * uu * t * p1[0] + 3 * u * tt * p2[0] + ttt * p3[0];
    point[1] = uuu * p0[1] + 3 * uu * t * p1[1] + 3 * u * tt * p2[1] + ttt * p3[1];
    point[2] = uuu * p0[2] + 3 * uu * t * p1[2] + 3 * u * tt * p2[2] + ttt * p3[2];
}

// Atualiza a posição da câmera na curva
void update_camera(float t, float *camera_position, float *p0, float *p1, float *p2, float *p3) {
    float point[3];
    bezier_curve(t, p0, p1, p2, p3, point);

    camera_position[0] = point[0];
    camera_position[1] = point[1];
    camera_position[2] = point[2];
}

// --- Funções Utilitárias ---
GLuint carregarTextura(const char *nomeArquivo) {
    GLuint idTextura;
    int largura, altura, numCanais;

    // Configura stb_image para inverter a imagem verticalmente no carregamento
    // (necessário pois OpenGL espera origem no canto inferior esquerdo)
    stbi_set_flip_vertically_on_load(true);
    unsigned char *dados = stbi_load(nomeArquivo, &largura, &altura, &numCanais, 0);

    if (dados) {
        GLenum formato = GL_RGB; // Formato padrão (3 canais)
        if (numCanais == 1) {
            formato = GL_RED; // Formato para imagens em escala de cinza (1 canal)
        } else if (numCanais == 4) {
            formato = GL_RGBA; // Formato para imagens com canal alfa (4 canais)
        }

        glGenTextures(1, &idTextura);               // Gera um ID de textura
        glBindTexture(GL_TEXTURE_2D, idTextura);    // Vincula a textura para configuração

        // Envia os dados da imagem para a GPU
        glTexImage2D(GL_TEXTURE_2D, 0, formato, largura, altura, 0, formato, GL_UNSIGNED_BYTE, dados);

        // Define parâmetros de repetição e filtragem da textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repete a textura na coordenada S (horizontal)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repete a textura na coordenada T (vertical)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtragem linear para minificação (quando objeto está longe)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtragem linear para magnificação (quando objeto está perto)

        // Libera a memória da imagem carregada na CPU, pois já está na GPU
        stbi_image_free(dados);

        // Desvincula a textura (boa prática)
        glBindTexture(GL_TEXTURE_2D, 0);

        printf("Textura '%s' carregada (Largura: %d, Altura: %d, Canais: %d, ID: %u)\n",
               nomeArquivo, largura, altura, numCanais, idTextura);
        return idTextura;
    } else {
        // Exibe erro se o carregamento falhar
        fprintf(stderr, "Erro ao carregar textura '%s': %s\n", nomeArquivo, stbi_failure_reason());
        return 0; // Retorna 0 para indicar falha
    }
}

void desenharDegrauArquibancada(float cx, float cy,
                                float rx_int, float ry_int, float rx_ext, float ry_ext,
                                float z_base, float z_topo,
                                float angulo_inicial_graus, float angulo_final_graus,
                                int num_segmentos_curva) {
    if (num_segmentos_curva <= 1) num_segmentos_curva = 2;
    float rad_inicial = GRAUS_PARA_RAD(angulo_inicial_graus);
    float rad_final = GRAUS_PARA_RAD(angulo_final_graus);
    float intervalo_rad = rad_final - rad_inicial;

    // --- Face Superior (Topo) ---
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= num_segmentos_curva; i++) {
        float fracao = (float)i / (float)num_segmentos_curva;
        float angulo_atual_rad = rad_inicial + fracao * intervalo_rad;
        float cos_a = cosf(angulo_atual_rad);
        float sin_a = sinf(angulo_atual_rad);
        float s_coord = fracao * 5.0f; // Repetição de textura ao longo do arco
        glTexCoord2f(s_coord, 1.0f); // Coord T = 1 (borda externa da textura?)
        glVertex3f(cx + rx_ext * cos_a, cy + ry_ext * sin_a, z_topo);
        glTexCoord2f(s_coord, 0.0f); // Coord T = 0 (borda interna da textura?)
        glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_topo);
    }
    glEnd();

    // --- Face Frontal (Vertical) ---
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= num_segmentos_curva; i++) {
        float fracao = (float)i / (float)num_segmentos_curva;
        float angulo_atual_rad = rad_inicial + fracao * intervalo_rad;
        float cos_a = cosf(angulo_atual_rad);
        float sin_a = sinf(angulo_atual_rad);
        float s_coord = fracao * 5.0f; // Repetição de textura ao longo do arco
        glTexCoord2f(s_coord, 1.0f); // Coord T = 1 (topo da face vertical)
        glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_topo);
        glTexCoord2f(s_coord, 0.0f); // Coord T = 0 (base da face vertical)
        glVertex3f(cx + rx_int * cos_a, cy + ry_int * sin_a, z_base);
    }
    glEnd();
}

void desenharParedeExterna(float cx, float cy,
                           float rx_base, float ry_base, float rx_topo, float ry_topo,
                           float z_baixo, float z_alto,
                           float angulo_inicial_graus, float angulo_final_graus,
                           int num_segmentos_curva) {
    if (num_segmentos_curva <= 1) num_segmentos_curva = 2;
    float rad_inicial = GRAUS_PARA_RAD(angulo_inicial_graus);
    float rad_final = GRAUS_PARA_RAD(angulo_final_graus);
    float intervalo_rad = rad_final - rad_inicial;

    // --- Face da Parede ---
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= num_segmentos_curva; i++) {
        float fracao = (float)i / (float)num_segmentos_curva;
        float angulo_atual_rad = rad_inicial + fracao * intervalo_rad;
        float cos_a = cosf(angulo_atual_rad);
        float sin_a = sinf(angulo_atual_rad);
        float s_coord = fracao * 10.0f; // Repetição de textura (talvez maior para paredes)
        glTexCoord2f(s_coord, 1.0f); // Coord T = 1 (topo da parede)
        glVertex3f(cx + rx_topo * cos_a, cy + ry_topo * sin_a, z_alto);
        glTexCoord2f(s_coord, 0.0f); // Coord T = 0 (base da parede)
        glVertex3f(cx + rx_base * cos_a, cy + ry_base * sin_a, z_baixo);
    }
    glEnd();
}

void desenharTampaLateral(float cx, float cy, float angulo_graus,
                          float rx_int_base, float ry_int_base, float z_int_base,
                          float rx_ext_base, float ry_ext_base, float z_ext_base,
                          float rx_ext_topo, float ry_ext_topo, float z_ext_topo,
                          float rx_int_topo, float ry_int_topo, float z_int_topo) {
    float angulo_rad = GRAUS_PARA_RAD(angulo_graus);
    float cos_a = cosf(angulo_rad);
    float sin_a = sinf(angulo_rad);

    // Vértices do quadrilátero da tampa
    float v1[3] = {cx + rx_int_base * cos_a, cy + ry_int_base * sin_a, z_int_base}; // Interno, Base
    float v2[3] = {cx + rx_ext_base * cos_a, cy + ry_ext_base * sin_a, z_ext_base}; // Externo, Base
    float v3[3] = {cx + rx_ext_topo * cos_a, cy + ry_ext_topo * sin_a, z_ext_topo}; // Externo, Topo
    float v4[3] = {cx + rx_int_topo * cos_a, cy + ry_int_topo * sin_a, z_int_topo}; // Interno, Topo

    // --- Tampa (Quadrilátero) ---
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3fv(v1); // Canto inf-int
        glTexCoord2f(1.0f, 0.0f); glVertex3fv(v2); // Canto inf-ext
        glTexCoord2f(1.0f, 1.0f); glVertex3fv(v3); // Canto sup-ext
        glTexCoord2f(0.0f, 1.0f); glVertex3fv(v4); // Canto sup-int
    glEnd();
}

void desenharMarquiseCobertura(float cx, float cy,
    float rx_base, float ry_base, float z_base, // Ponto de trás/base
    float rx_frente, float ry_frente, float z_frente, // Ponto da frente/projetado
    float angulo_inicial_graus, float angulo_final_graus,
    int num_segmentos_curva) {
    if (num_segmentos_curva <= 1) num_segmentos_curva = 2;

    float rad_inicial = GRAUS_PARA_RAD(angulo_inicial_graus);
    float rad_final = GRAUS_PARA_RAD(angulo_final_graus);
    float intervalo_rad = rad_final - rad_inicial;

    // Define a aparência (pode mudar cor/textura)
    glColor3f(0.7f, 0.7f, 0.75f); // Um cinza diferente
    // glBindTexture(GL_TEXTURE_2D, idTexturaMarquise); // Se tiver textura

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= num_segmentos_curva; i++) {
    float fracao = (float)i / (float)num_segmentos_curva;
    float angulo_atual_rad = rad_inicial + fracao * intervalo_rad;
    float cos_a = cosf(angulo_atual_rad);
    float sin_a = sinf(angulo_atual_rad);
    float s_coord = fracao; // Mapeamento de textura simples 0-1

    // Vértice de TRÁS/BASE (no topo da parede superior)
    glTexCoord2f(s_coord, 1.0f); // T=1 na parte de trás/base
    glVertex3f(cx + rx_base * cos_a, cy + ry_base * sin_a, z_base);

    // Vértice da FRENTE (projetado sobre a arquibancada)
    glTexCoord2f(s_coord, 0.0f); // T=0 na frente/borda
    glVertex3f(cx + rx_frente * cos_a, cy + ry_frente * sin_a, z_frente);
    }
    glEnd();
}

// --- Função de callback: Desenho ---
void display() {
    // Limpa os buffers de cor e profundidade
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- Configurações de Câmera e Rotação ---
    glMatrixMode(GL_MODELVIEW); // Define a matriz de ModelView como a matriz atual
    glLoadIdentity();           // Carrega a matriz identidade (reseta transformações)

    // Salva o estado da matriz antes de aplicar a transformação da câmera
   glPushMatrix();

   // Configuração da câmera
   glTranslatef(-camera_position[0], -camera_position[1], -camera_position[2]);
    
   // Cálculo do ângulo de rotação da câmera
   float rad = cameraAngle * M_PI / 180.0f;
   float lookX = camera_position[0] + sin(rad);
   float lookZ = camera_position[2] - cos(rad);

   // Define a posição da câmera
   gluLookAt(camera_position[0], camera_position[1], camera_position[2],
             lookX, 0.0f, lookZ,
             0.0f, 1.0f, 0.0f);


    // Aplica as rotações (a ordem importa!)
    glRotatef(anguloRotacaoX, 1.0f, 0.0f, 0.0f); // Rotação em torno do eixo X global
    glRotatef(anguloRotacaoY, 0.0f, 1.0f, 0.0f); // Rotação em torno do eixo Y global
    glRotatef(anguloRotacaoZ, 0.0f, 0.0f, 1.0f); // Rotação em torno do eixo Z global

   // --- 1. Desenhar o Chão ---
   glColor3f(0.8f, 0.8f, 0.8f);
   glBindTexture(GL_TEXTURE_2D, idTexturaTerra);
   float tamChao = 20.0f;
   float repTexturaChao = 15.0f;
   glBegin(GL_QUADS);
       glTexCoord2f(0.0f, 0.0f);                   glVertex3f(-tamChao, -tamChao, Z_CHAO);
       glTexCoord2f(repTexturaChao, 0.0f);         glVertex3f( tamChao, -tamChao, Z_CHAO);
       glTexCoord2f(repTexturaChao, repTexturaChao); glVertex3f( tamChao,  tamChao, Z_CHAO);
       glTexCoord2f(0.0f, repTexturaChao);         glVertex3f(-tamChao,  tamChao, Z_CHAO);
   glEnd();

   // --- Parâmetros Comuns & Seção Especial ---
   float centro_x = 0.0f;
   float centro_y = 0.0f;
   int segmentos_curva_degrau = 40;
   int segmentos_curva_parede = 60;
   // Raios e altura padrões
   const float raio_x_geral_int = 0.5f;
   const float raio_y_geral_int = 0.7f;
   const float raio_x_geral_ext = 0.8f;
   const float raio_y_geral_ext = 0.95f;
   const float largura_radial_x_padrao = raio_x_geral_ext - raio_x_geral_int;
   const float largura_radial_y_padrao = raio_y_geral_ext - raio_y_geral_int;
   const float altura_max_parede_padrao = ALTURA_MAX_ESC; // Altura padrão explícita
   const float rx_topo_parede_padrao = raio_x_geral_ext + INCLINACAO_PAREDE_OFFSET; // Topo padrão explícito
   const float ry_topo_parede_padrao = raio_y_geral_ext + INCLINACAO_PAREDE_OFFSET;
   // const float altura_arco_conexao = ALTURA_MIN_ESC; // Valor original
   const float altura_arco_conexao = ALTURA_MIN_ESC * 7; // Valor que estava no seu código
   

   // Arcos
   float arcos_principais[][2] = {
       {  0.0f,  60.0f}, {120.0f, 140.0f}, {140.0f, 220.0f}, {220.0f, 240.0f}, {300.0f, 360.0f}
   };
   int num_arcos_principais = sizeof(arcos_principais) / sizeof(arcos_principais[0]);
   float arcos_conexao[][2] = {
       { 60.0f, 120.0f}, {240.0f, 300.0f}
   };
   int num_arcos_conexao = sizeof(arcos_conexao) / sizeof(arcos_conexao[0]);

   // Parâmetros Seção Especial (Arquibancada 140-220)
   const int DEGRAUS_ADICIONAIS_SECAO_ESPECIAL = 8;
   const float FATOR_ALTURA_SECAO_ESPECIAL = 1.35f;
   const float FATOR_RAIO_EXTERNO_SECAO_ESPECIAL = 1.15f;
   const int num_degraus_secao_especial = NUM_DEGRAUS + DEGRAUS_ADICIONAIS_SECAO_ESPECIAL;
   const float altura_max_secao_especial = altura_max_parede_padrao * FATOR_ALTURA_SECAO_ESPECIAL;
   const float raio_x_ext_especial_seating = raio_x_geral_ext * FATOR_RAIO_EXTERNO_SECAO_ESPECIAL;
   const float raio_y_ext_especial_seating = raio_y_geral_ext * FATOR_RAIO_EXTERNO_SECAO_ESPECIAL;
   const float largura_radial_x_especial = raio_x_ext_especial_seating - raio_x_geral_int;
   const float largura_radial_y_especial = raio_y_ext_especial_seating - raio_y_geral_int;

   const float INCLINACAO_PAREDE_SUPERIOR_EXTRA = 0.15f;
   const float FATOR_ALTURA_PAREDE_SUPERIOR = 1.6f; 
   const float rx_base_parede_superior = rx_topo_parede_padrao;
   const float ry_base_parede_superior = ry_topo_parede_padrao;
   const float z_base_parede_superior = altura_max_parede_padrao;
   const float rx_topo_parede_superior = rx_base_parede_superior + INCLINACAO_PAREDE_SUPERIOR_EXTRA;
   const float ry_topo_parede_superior = ry_base_parede_superior + INCLINACAO_PAREDE_SUPERIOR_EXTRA;
   const float z_topo_parede_superior = altura_max_parede_padrao * FATOR_ALTURA_PAREDE_SUPERIOR;

   const float MARQUISE_PROJECAO_RADIAL = -0.3f; 
   const float MARQUISE_INCLINACAO_Z_OFFSET = 0.08f;
   const float rx_base_marquise = rx_topo_parede_superior; // Base (atrás) no topo da parede superior
   const float ry_base_marquise = ry_topo_parede_superior;
   const float z_base_marquise = z_topo_parede_superior;   // Base Z no topo da parede superior
   const float rx_frente_marquise = rx_base_marquise + MARQUISE_PROJECAO_RADIAL; // Frente (projetada) com raio menor
   const float ry_frente_marquise = ry_base_marquise + MARQUISE_PROJECAO_RADIAL;
   const float z_frente_marquise = z_base_marquise + MARQUISE_INCLINACAO_Z_OFFSET; // Frente com Z menor 

   // --- 2. Desenhar Arquibancadas (Degraus) ---
   glColor3f(1.0f, 1.0f, 1.0f);
   glBindTexture(GL_TEXTURE_2D, idTexturaConcreto);
   for (int i = 0; i < num_arcos_principais; ++i) {
       float ang_inicio_arco = arcos_principais[i][0];
       float ang_fim_arco = arcos_principais[i][1];
       float z_topo_anterior = Z_BASE_INICIAL;
       bool secao_especial = (fabs(ang_inicio_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE);

       // Define parâmetros para este arco (padrão ou especial)
       int num_degraus_atual = secao_especial ? num_degraus_secao_especial : NUM_DEGRAUS;
       // A altura máxima aqui refere-se à arquibancada
       float altura_max_atual_seating = secao_especial ? altura_max_secao_especial : altura_max_parede_padrao;
       float largura_radial_x_atual = secao_especial ? largura_radial_x_especial : largura_radial_x_padrao;
       float largura_radial_y_atual = secao_especial ? largura_radial_y_especial : largura_radial_y_padrao;

       // Desenha os degraus
       for (int k = 0; k < num_degraus_atual; ++k) {
           float z_topo_atual = Z_BASE_INICIAL + (altura_max_atual_seating - Z_BASE_INICIAL) * ((float)(k + 1) / num_degraus_atual);
           float z_base_atual = z_topo_anterior;

           float fracao_raio_int = (float)k / num_degraus_atual;
           float fracao_raio_ext = (float)(k + 1) / num_degraus_atual;
           float rx_int_k = raio_x_geral_int + largura_radial_x_atual * fracao_raio_int;
           float ry_int_k = raio_y_geral_int + largura_radial_y_atual * fracao_raio_int;
           float rx_ext_k = raio_x_geral_int + largura_radial_x_atual * fracao_raio_ext;
           float ry_ext_k = raio_y_geral_int + largura_radial_y_atual * fracao_raio_ext;

           desenharDegrauArquibancada(
               centro_x, centro_y,
               rx_int_k, ry_int_k, rx_ext_k, ry_ext_k,
               z_base_atual, z_topo_atual,
               ang_inicio_arco, ang_fim_arco,
               segmentos_curva_degrau
           );
           z_topo_anterior = z_topo_atual;
       }
   }


   // --- 3. Desenhar Paredes Externas e Tampas ---
   glColor3f(0.9f, 0.9f, 0.9f);
   glBindTexture(GL_TEXTURE_2D, idTexturaConcretoExterno);

    const float INCLINACAO_MARQUISE_EXTRA = 0.10f; // Inclinação *adicional* à base da marquise
    const float ALTURA_INICIO_MARQUISE_FATOR = 1.05f; // Começa 5% acima da parede superior
    const float ALTURA_FIM_MARQUISE_FATOR = 1.4f;
 
   // Desenha as Paredes Principais (PADRÃO - até altura_max_parede_padrao)
   for (int i = 0; i < num_arcos_principais; ++i) {
       float ang_inicio_arco = arcos_principais[i][0];
       float ang_fim_arco = arcos_principais[i][1];
       desenharParedeExterna(
           centro_x, centro_y,
           raio_x_geral_ext, raio_y_geral_ext,           // Base padrão
           rx_topo_parede_padrao, ry_topo_parede_padrao, // Topo padrão
           Z_BASE_INICIAL, altura_max_parede_padrao,     // Altura padrão
           ang_inicio_arco, ang_fim_arco, segmentos_curva_parede
       );
   }

   // Desenha as Paredes de Conexão
   for (int i = 0; i < num_arcos_conexao; ++i) {
       float ang_inicio_arco = arcos_conexao[i][0];
       float ang_fim_arco = arcos_conexao[i][1];
       desenharParedeExterna(
           centro_x, centro_y,
           raio_x_geral_ext, raio_y_geral_ext,           // Base padrão
           rx_topo_parede_padrao, ry_topo_parede_padrao, // Topo padrão
           Z_BASE_INICIAL, altura_arco_conexao,          // Altura de conexão (usando seu valor *7)
           ang_inicio_arco, ang_fim_arco, segmentos_curva_parede
       );
   }

   // Desenha a PAREDE SUPERIOR ADICIONAL (140-220) 
   desenharParedeExterna(
       centro_x, centro_y,
       rx_base_parede_superior, ry_base_parede_superior, // Base = Topo da parede padrão
       rx_topo_parede_superior, ry_topo_parede_superior, // Topo = Mais inclinado
       z_base_parede_superior, z_topo_parede_superior,   // Altura = Acima da parede padrão
       140.0f, 220.0f,                                   // Apenas neste ângulo
       segmentos_curva_parede
   );

   //Desenha Marquise
   desenharMarquiseCobertura(
    centro_x, centro_y,
    rx_base_marquise, ry_base_marquise, z_base_marquise,     // Base da marquise
    rx_frente_marquise, ry_frente_marquise, z_frente_marquise, // Frente da marquise
    180.0f, 220.0f,                                          // Ângulo (segunda metade da seção especial)
    segmentos_curva_parede / 2                               // Resolução (ajuste se necessário)
    );
    // Restaurar cor/textura da parede se necessário
    glColor3f(0.9f, 0.9f, 0.9f);
    glBindTexture(GL_TEXTURE_2D, idTexturaConcretoExterno);


   // Desenha as Tampas Laterais (pulando 0, 140, 220, 360)
   float altura_interna_tampa = altura_arco_conexao; // Altura interna da tampa é sempre a de conexão
   for (int i = 0; i < num_arcos_principais; ++i) {
       float ang_inicio_arco = arcos_principais[i][0];
       float ang_fim_arco = arcos_principais[i][1];

       // *** ALTERADO/CONFIRMADO: Lógica para pular tampas e ajustar adjacentes ***
       // Verifica se a tampa INICIAL deve ser desenhada
       bool desenhar_tampa_inicial =
              fabs(ang_inicio_arco -   0.0f) > FLOAT_COMPARISON_TOLERANCE
           && fabs(ang_inicio_arco - 140.0f) > FLOAT_COMPARISON_TOLERANCE
           && fabs(ang_inicio_arco - 220.0f) > FLOAT_COMPARISON_TOLERANCE;

       if (desenhar_tampa_inicial) {
            // Determina parâmetros do TOPO EXTERNO para a tampa INICIAL
            float rx_final_topo_ext = rx_topo_parede_padrao;      // Default
            float ry_final_topo_ext = ry_topo_parede_padrao;
            float z_final_topo_ext = altura_max_parede_padrao;

            // Se a tampa inicial for em 240 graus, ela encontra a parede superior que termina em 220
            if (fabs(ang_inicio_arco - 240.0f) < FLOAT_COMPARISON_TOLERANCE) {
                rx_final_topo_ext = rx_topo_parede_superior; // Usa topo da parede superior
                ry_final_topo_ext = ry_topo_parede_superior;
                z_final_topo_ext = z_topo_parede_superior;
            }

            desenharTampaLateral(
               centro_x, centro_y, ang_inicio_arco,
               raio_x_geral_int, raio_y_geral_int, Z_BASE_INICIAL,      // Base interna
               raio_x_geral_ext, raio_y_geral_ext, Z_BASE_INICIAL,      // Base externa
               rx_final_topo_ext, ry_final_topo_ext, z_final_topo_ext, // Topo externo (Calculado)
               raio_x_geral_int, raio_y_geral_int, altura_interna_tampa // Topo interno
            );
       }


       // Verifica se a tampa FINAL deve ser desenhada
       bool desenhar_tampa_final =
              fabs(ang_fim_arco - 360.0f) > FLOAT_COMPARISON_TOLERANCE
           && fabs(ang_fim_arco - 140.0f) > FLOAT_COMPARISON_TOLERANCE
           && fabs(ang_fim_arco - 220.0f) > FLOAT_COMPARISON_TOLERANCE;

       if (desenhar_tampa_final) {
            // Determina parâmetros do TOPO EXTERNO para a tampa FINAL
            float rx_final_topo_ext = rx_topo_parede_padrao;      // Default
            float ry_final_topo_ext = ry_topo_parede_padrao;
            float z_final_topo_ext = altura_max_parede_padrao;

            // Se a tampa final for em 120 graus, ela encontra a parede superior que começa em 140
            if (fabs(ang_fim_arco - 120.0f) < FLOAT_COMPARISON_TOLERANCE) {
                rx_final_topo_ext = rx_topo_parede_superior; // Usa topo da parede superior
                ry_final_topo_ext = ry_topo_parede_superior;
                z_final_topo_ext = z_topo_parede_superior;
            }

            desenharTampaLateral(
               centro_x, centro_y, ang_fim_arco,
               raio_x_geral_int, raio_y_geral_int, Z_BASE_INICIAL,      // Base interna
               raio_x_geral_ext, raio_y_geral_ext, Z_BASE_INICIAL,      // Base externa
               rx_final_topo_ext, ry_final_topo_ext, z_final_topo_ext, // Topo externo (Calculado)
               raio_x_geral_int, raio_y_geral_int, altura_interna_tampa // Topo interno
            );
       }
   } // Fim loop for (para tampas)

<<<<<<< HEAD
    glBindTexture(GL_TEXTURE_2D, 0);

    if (alphaFiltro > 0.0f) {
        glDisable(GL_DEPTH_TEST); 
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1, 0, 1);
    
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    
        for (int i = 0; i < 4; i++) {  
            glColor4f(0.0f, 0.0f, 0.0f, alphaFiltro);
            glBegin(GL_QUADS);
                glVertex2f(0.0f, 0.0f);
                glVertex2f(1.0f, 0.0f);
                glVertex2f(1.0f, 1.0f);
                glVertex2f(0.0f, 1.0f);
            glEnd();
        }
    
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
    }

    glutSwapBuffers();
=======
   glBindTexture(GL_TEXTURE_2D, 0);
   glutSwapBuffers();
>>>>>>> ffc216e1a86d9e4f9349550f3a3d9fd009e71059
}


void idle() {
    static int contador = 0;
    contador++;
    if (contador > 5000) {  // muda a cada x frames (~alguns segundos)
        modoNoite = !modoNoite;
        contador = 0;
    }
    glutPostRedisplay();
}


// --- Função de callback: Inicialização ---
void init() {
    // Define a cor de fundo da janela (RGBA) - um azul céu claro
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);

    // Habilita o uso de texturas 2D
    glEnable(GL_TEXTURE_2D);
    // Define como a cor da textura interage com a cor definida por glColor*
    // GL_MODULATE: multiplica a cor da textura pela cor do objeto (permite tingir texturas)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Habilita o teste de profundidade (Z-buffer) para que objetos mais próximos ocultem os mais distantes
    glEnable(GL_DEPTH_TEST);

    // Carrega as texturas necessárias
    printf("Carregando texturas...\n");
    idTexturaConcreto = carregarTextura("concreto.jpg");
    if (idTexturaConcreto == 0) { // Verifica se o carregamento falhou
        fprintf(stderr, "ERRO FATAL: Textura 'concreto.jpg' não carregada!\n");
        exit(1); // Aborta a execução se uma textura essencial não puder ser carregada
    }
    // Assume que o nome correto é terra.jpeg baseado na mensagem de erro original
    idTexturaTerra = carregarTextura("terra.jpeg");
    if (idTexturaTerra == 0) {
        fprintf(stderr, "ERRO FATAL: Textura 'terra.jpeg' não carregada!\n");
        exit(1);
    }
    idTexturaConcretoExterno = carregarTextura("concreto_externo.jpg");
     if (idTexturaConcretoExterno == 0) {
        fprintf(stderr, "ERRO FATAL: Textura 'concreto_externo.jpg' não carregada!\n");
        exit(1);
    }
    printf("Texturas carregadas com sucesso.\n");
}

// --- Função de callback: Redimensionamento da Janela ---
void reshape(int largura, int altura) {
    // Previne divisão por zero se a janela for minimizada
    if (altura == 0) {
        altura = 1;
    }
    // Calcula a proporção largura/altura da janela
    float proporcao = (float)largura / (float)altura;

    // Define a área de desenho (viewport) para ocupar toda a janela
    glViewport(0, 0, largura, altura);

    // Define a matriz de Projeção como a matriz atual
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reseta a matriz de projeção

    // Configura a projeção em perspectiva
    // Parâmetros: ângulo de visão Y, proporção, plano de corte próximo (near), plano de corte distante (far)
    gluPerspective(60.0, proporcao, 0.1, 100.0);

    // Retorna para a matriz de ModelView para as operações de desenho e câmera
    glMatrixMode(GL_MODELVIEW);
}

// teste para ver se a função keyboard funciona
// void keyboard(unsigned char key, int x, int y) {
//    printf("tecla: %c\n", key);
//  }

void keyboard(unsigned char key, int x, int y) {
    printf("Tecla: %c\n", key);

    float cameraSpeed = 0.5f; // Velocidade de movimento
    float rad = cameraAngle * M_PI / 180.0f;
    float dirX = sin(rad);
    float dirZ = -cos(rad);

    switch (key) {
        case 'a': // Rotacionar à esquerda
            cameraAngle -= 5.0f;
            break;
        case 'd': // Rotacionar à direita
            cameraAngle += 5.0f;
            break;
        case 'w': // Subir (eixo Y)
            camera_position[1] += cameraSpeed;
            break;
        case 's': // Descer (eixo Y)
            camera_position[1] -= cameraSpeed;
            break;
        case 'j': // Aproximar (zoom in)
            camera_position[0] += dirX * cameraSpeed;
            camera_position[2] += dirZ * cameraSpeed;
            break;
        case 'k': // Afastar (zoom out)
            camera_position[0] -= dirX * cameraSpeed;
            camera_position[2] -= dirZ * cameraSpeed;
            break;
        case 27: // ESC
            exit(0);
            break;
    }

    glutPostRedisplay();
}

// --- Função Principal ---
int main(int numArgumentos, char** argumentos) {
    // Inicializa a biblioteca GLUT
    glutInit(&numArgumentos, argumentos);

    // Define o modo de exibição inicial
    // GLUT_DOUBLE: Habilita double buffering (evita flickering)
    // GLUT_RGBA: Usa modo de cor RGBA
    // GLUT_DEPTH: Habilita buffer de profundidade
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Define o tamanho inicial da janela (largura, altura)
    glutInitWindowSize(1200, 800);

    // Define a posição inicial da janela na tela (canto superior esquerdo)
    glutInitWindowPosition(100, 100);

    // Cria a janela com o título especificado
    glutCreateWindow("Simulador de Estádio Elíptico com Texturas");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutTimerFunc(0, atualizarTransicao, 0);

    // Chama nossa função de inicialização (configura OpenGL, carrega texturas)
    init();

    // ---> REGISTRA AS FUNÇÕES DE CALLBACK <---
    // Estas funções serão chamadas pelo GLUT em resposta a eventos
    glutDisplayFunc(display);   // Função a ser chamada para desenhar a cena
    glutReshapeFunc(reshape);   // Função a ser chamada quando a janela é redimensionada
    glutKeyboardFunc(keyboard); // Função a ser chamada quando uma tecla é pressionada

    glutIdleFunc(idle);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);


    // Exibe as instruções de controle no console
    printf("Controles do Simulador:\n");
    printf("  W/S: Rotacionar Cima/Baixo (Eixo Y)\n");
    printf("  A/D: Rotacionar Esquerda/Direita (Eixo Z)\n");
    printf("  X/Z: Rotacionar Inclinação Lateral (Eixo X)\n");
    printf("  K/J: Zoom Out / Zoom In\n"); // Corrigido K/J
    printf("  ESC: Sair\n");

    // Inicia o loop principal do GLUT. A partir daqui, GLUT gerencia os eventos.
    glutMainLoop();

    // O código abaixo de glutMainLoop() normalmente não é alcançado,
    // pois o loop só termina quando exit() é chamado.
    return 0; // Retorno padrão indicando sucesso
}