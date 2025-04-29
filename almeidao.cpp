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

// --- Função de callback: Desenho ---
void display() {
    // Limpa os buffers de cor e profundidade
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- Configurações de Câmera e Rotação ---
    glMatrixMode(GL_MODELVIEW); // Define a matriz de ModelView como a matriz atual
    glLoadIdentity();           // Carrega a matriz identidade (reseta transformações)

    // Configura a câmera usando gluLookAt: Posição do olho, Ponto para onde olha, Vetor 'up'
    gluLookAt(0.0, 0.0, cameraDistanciaZ,  // Posição da câmera (controlada pelo zoom)
              0.0, 0.0, 0.0,              // Ponto de foco (origem)
              0.0, 1.0, 0.0);             // Vetor que aponta para cima (eixo Y)

    // Aplica as rotações (a ordem importa!)
    glRotatef(anguloRotacaoX, 1.0f, 0.0f, 0.0f); // Rotação em torno do eixo X global
    glRotatef(anguloRotacaoY, 0.0f, 1.0f, 0.0f); // Rotação em torno do eixo Y global
    glRotatef(anguloRotacaoZ, 0.0f, 0.0f, 1.0f); // Rotação em torno do eixo Z global

    // --- 1. Desenhar o Chão ---
    glColor3f(0.8f, 0.8f, 0.8f);            // Cor cinza para o chão
    glBindTexture(GL_TEXTURE_2D, idTexturaTerra); // Ativa a textura da terra
    float tamChao = 20.0f;                  // Metade do tamanho do lado do chão
    float repTexturaChao = 15.0f;           // Fator de repetição da textura no chão
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);                   glVertex3f(-tamChao, -tamChao, Z_CHAO);
        glTexCoord2f(repTexturaChao, 0.0f);         glVertex3f( tamChao, -tamChao, Z_CHAO);
        glTexCoord2f(repTexturaChao, repTexturaChao); glVertex3f( tamChao,  tamChao, Z_CHAO);
        glTexCoord2f(0.0f, repTexturaChao);         glVertex3f(-tamChao,  tamChao, Z_CHAO);
    glEnd();

    // --- Parâmetros Comuns para Arquibancadas e Paredes ---
    float centro_x = 0.0f;
    float centro_y = 0.0f;
    int segmentos_curva_degrau = 40;
    int segmentos_curva_parede = 60;
    // Raio interno padrão
    const float raio_x_geral_int = 0.5f;
    const float raio_y_geral_int = 0.7f;
    // Raio externo padrão (usado para paredes e seções normais)
    const float raio_x_geral_ext = 0.8f;
    const float raio_y_geral_ext = 0.95f;
    // Largura radial padrão (baseada nos raios padrão)
    const float largura_radial_x_padrao = raio_x_geral_ext - raio_x_geral_int;
    const float largura_radial_y_padrao = raio_y_geral_ext - raio_y_geral_int;
    
    

    // Arcos principais (sem alterações na definição)
    float arcos_principais[][2] = {
        {  0.0f,  60.0f}, {120.0f, 140.0f}, {140.0f, 220.0f}, {220.0f, 240.0f}, {300.0f, 360.0f}
    };
    int num_arcos_principais = sizeof(arcos_principais) / sizeof(arcos_principais[0]);

    // Arcos de conexão (sem alterações)
    float arcos_conexao[][2] = {
        { 60.0f, 120.0f}, {240.0f, 300.0f}
    };
    int num_arcos_conexao = sizeof(arcos_conexao) / sizeof(arcos_conexao[0]);
    float altura_arco_conexao = ALTURA_MIN_ESC*7;

    const int DEGRAUS_ADICIONAIS_SECAO_ESPECIAL = 8;
    const float FATOR_ALTURA_SECAO_ESPECIAL = 1.35f; // 35% mais alta
    const float FATOR_RAIO_EXTERNO_SECAO_ESPECIAL = 1.15f; // 15% maior raio externo

    // *** Calcula parâmetros absolutos da seção especial ***
    const int num_degraus_secao_especial = NUM_DEGRAUS + DEGRAUS_ADICIONAIS_SECAO_ESPECIAL;
    const float altura_max_secao_especial = ALTURA_MAX_ESC * FATOR_ALTURA_SECAO_ESPECIAL;
    // Raio externo *maior* para a seção especial
    const float raio_x_ext_especial = raio_x_geral_ext * FATOR_RAIO_EXTERNO_SECAO_ESPECIAL;
    const float raio_y_ext_especial = raio_y_geral_ext * FATOR_RAIO_EXTERNO_SECAO_ESPECIAL;
    // Largura radial *nova* para a seção especial (baseada no raio externo maior e interno padrão)
    const float largura_radial_x_especial = raio_x_ext_especial - raio_x_geral_int;
    const float largura_radial_y_especial = raio_y_ext_especial - raio_y_geral_int;


    // --- 2. Desenhar Arquibancadas (Degraus) ---
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, idTexturaConcreto);

    for (int i = 0; i < num_arcos_principais; ++i) {
        float ang_inicio_arco = arcos_principais[i][0];
        float ang_fim_arco = arcos_principais[i][1];
        float z_topo_anterior = Z_BASE_INICIAL;

        bool secao_especial = false;
        if (fabs(ang_inicio_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE) {
            secao_especial = true;
        }

        // Define parâmetros PARA ESTE ARCO (padrão ou especial)
        int num_degraus_atual = NUM_DEGRAUS;
        float altura_max_atual = ALTURA_MAX_ESC;
        // Usa larguras radiais padrão por default
        float largura_radial_x_atual = largura_radial_x_padrao;
        float largura_radial_y_atual = largura_radial_y_padrao;

        if (secao_especial) {
            num_degraus_atual = num_degraus_secao_especial;
            altura_max_atual = altura_max_secao_especial;
            // *** USA AS LARGURAS RADIAIS ESPECIAIS (maiores) ***
            largura_radial_x_atual = largura_radial_x_especial;
            largura_radial_y_atual = largura_radial_y_especial;
            // printf("Aplicando parâmetros especiais ao arco %.1f-%.1f\n", ang_inicio_arco, ang_fim_arco);
        }

        // Desenha os degraus
        for (int k = 0; k < num_degraus_atual; ++k) {
            float z_topo_atual = Z_BASE_INICIAL + (altura_max_atual - Z_BASE_INICIAL) * ((float)(k + 1) / num_degraus_atual);
            float z_base_atual = z_topo_anterior;

            // Calcula raios interno/externo para degrau k
            // Ambos começam do raio interno PADRÃO (raio_x/y_geral_int)
            // e usam a largura radial ATUAL (padrão ou especial)
            float fracao_raio_int = (float)k / num_degraus_atual;
            float fracao_raio_ext = (float)(k + 1) / num_degraus_atual;
            float rx_int_k = raio_x_geral_int + largura_radial_x_atual * fracao_raio_int;
            float ry_int_k = raio_y_geral_int + largura_radial_y_atual * fracao_raio_int;
            float rx_ext_k = raio_x_geral_int + largura_radial_x_atual * fracao_raio_ext;
            float ry_ext_k = raio_y_geral_int + largura_radial_y_atual * fracao_raio_ext;

            // *** REMOVE O CLAMPING: Permite que rx/y_ext_k excedam raio_x/y_geral_ext ***
            // if (rx_ext_k > raio_x_geral_ext) rx_ext_k = raio_x_geral_ext; // REMOVIDO
            // if (ry_ext_k > raio_y_geral_ext) ry_ext_k = raio_y_geral_ext; // REMOVIDO

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

    // Cálculos da parede (SEM ALTERAÇÕES - usam raios/altura padrão)
    float rx_base_parede = raio_x_geral_ext;
    float ry_base_parede = raio_y_geral_ext;
    float rx_topo_parede = rx_base_parede + INCLINACAO_PAREDE_OFFSET;
    float ry_topo_parede = ry_base_parede + INCLINACAO_PAREDE_OFFSET;

    // Desenha as Paredes Principais (SEM ALTERAÇÕES - altura padrão)
    for (int i = 0; i < num_arcos_principais; ++i) {
        float ang_inicio_arco = arcos_principais[i][0];
        float ang_fim_arco = arcos_principais[i][1];
        desenharParedeExterna(
            centro_x, centro_y,
            rx_base_parede, ry_base_parede, rx_topo_parede, ry_topo_parede,
            Z_BASE_INICIAL, ALTURA_MAX_ESC, // Usa altura PADRÃO
            ang_inicio_arco, ang_fim_arco, segmentos_curva_parede
        );
    }

    // Desenha as Paredes de Conexão (SEM ALTERAÇÕES)
    for (int i = 0; i < num_arcos_conexao; ++i) {
        
        float ang_inicio_arco = arcos_conexao[i][0];
        float ang_fim_arco = arcos_conexao[i][1];
        desenharParedeExterna(
            centro_x, centro_y,
            rx_base_parede, ry_base_parede, rx_topo_parede, ry_topo_parede,
            Z_BASE_INICIAL, altura_arco_conexao,
            ang_inicio_arco, ang_fim_arco, segmentos_curva_parede
        );
    }

    // Desenha as Tampas Laterais
    float altura_interna_tampa = altura_arco_conexao; // Altura interna da tampa é sempre a de conexão
    for (int i = 0; i < num_arcos_principais; ++i) {
        float ang_inicio_arco = arcos_principais[i][0];
        float ang_fim_arco = arcos_principais[i][1];

        // --- Lógica para determinar parâmetros do topo EXTERNO da tampa ---
        // Por padrão, usa os parâmetros da parede normal
        float rx_topo_ext_tampa = rx_topo_parede;
        float ry_topo_ext_tampa = ry_topo_parede;
        float z_topo_ext_tampa = ALTURA_MAX_ESC;

        // Se o *início* deste arco for 140 ou 220 (adjacente à seção especial)
        // OU se o *fim* do arco *anterior* for 140 ou 220 (também adjacente)
        // --> O topo externo da tampa deve usar os parâmetros da seção especial

        // Verifica se o início ou fim deste arco coincide com os limites da seção especial
        bool inicio_na_juncao = (fabs(ang_inicio_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_inicio_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE);
        bool fim_na_juncao = (fabs(ang_fim_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_fim_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE);

        // Se a tampa for em 140 ou 220 graus, use os parâmetros externos da seção especial
        if (inicio_na_juncao) {
             rx_topo_ext_tampa = raio_x_ext_especial; // Usa raio externo maior
             ry_topo_ext_tampa = raio_y_ext_especial; // Usa raio externo maior
             z_topo_ext_tampa = altura_max_secao_especial; // Usa altura maior
             // printf("Tampa inicial em %.1f usando params especiais\n", ang_inicio_arco); // Debug
        }
         if (fim_na_juncao) {
             // Note: A tampa no *fim* de um arco também usa os parâmetros da seção especial se o ângulo for 140/220
             //       pois ela precisa "encontrar" a seção especial que começa/termina ali.
             rx_topo_ext_tampa = raio_x_ext_especial;
             ry_topo_ext_tampa = raio_y_ext_especial;
             z_topo_ext_tampa = altura_max_secao_especial;
             // printf("Tampa final em %.1f usando params especiais\n", ang_fim_arco); // Debug
        }


        // Desenha a tampa INICIAL (pulando 0 graus)
        if (fabs(ang_inicio_arco - 0.0f) > FLOAT_COMPARISON_TOLERANCE) {
             // Determina os parâmetros corretos para o topo externo desta tampa específica
             float rx_final_tampa_ini = (fabs(ang_inicio_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_inicio_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE) ? raio_x_ext_especial : rx_topo_parede;
             float ry_final_tampa_ini = (fabs(ang_inicio_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_inicio_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE) ? raio_y_ext_especial : ry_topo_parede;
             float z_final_tampa_ini  = (fabs(ang_inicio_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_inicio_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE) ? altura_max_secao_especial : ALTURA_MAX_ESC;

             desenharTampaLateral(
                centro_x, centro_y, ang_inicio_arco,
                raio_x_geral_int, raio_y_geral_int, Z_BASE_INICIAL,      // Base interna (padrão)
                rx_base_parede, ry_base_parede, Z_BASE_INICIAL,          // Base externa (padrão)
                rx_final_tampa_ini, ry_final_tampa_ini, z_final_tampa_ini, // Topo externo (pode ser especial)
                raio_x_geral_int, raio_y_geral_int, altura_interna_tampa // Topo interno (padrão)
             );
        }

        // Desenha a tampa FINAL (pulando 360 graus)
        if (fabs(ang_fim_arco - 360.0f) > FLOAT_COMPARISON_TOLERANCE) {
            // Determina os parâmetros corretos para o topo externo desta tampa específica
             float rx_final_tampa_fim = (fabs(ang_fim_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_fim_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE) ? raio_x_ext_especial : rx_topo_parede;
             float ry_final_tampa_fim = (fabs(ang_fim_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_fim_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE) ? raio_y_ext_especial : ry_topo_parede;
             float z_final_tampa_fim  = (fabs(ang_fim_arco - 140.0f) < FLOAT_COMPARISON_TOLERANCE || fabs(ang_fim_arco - 220.0f) < FLOAT_COMPARISON_TOLERANCE) ? altura_max_secao_especial : ALTURA_MAX_ESC;

             desenharTampaLateral(
                centro_x, centro_y, ang_fim_arco,
                raio_x_geral_int, raio_y_geral_int, Z_BASE_INICIAL,      // Base interna (padrão)
                rx_base_parede, ry_base_parede, Z_BASE_INICIAL,          // Base externa (padrão)
                rx_final_tampa_fim, ry_final_tampa_fim, z_final_tampa_fim, // Topo externo (pode ser especial)
                raio_x_geral_int, raio_y_geral_int, altura_interna_tampa // Topo interno (padrão)
             );
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glutSwapBuffers();
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

// --- Função de callback: Teclado ---
void keyboard(unsigned char key, int x, int y) {
    float incrementoRot = 5.0f;  // Quantos graus rotacionar por tecla pressionada
    float incrementoZoom = 0.2f; // Quanto mudar a distância da câmera por tecla
    float zoomMin = 0.5f;        // Distância mínima permitida da câmera
    float zoomMax = 20.0f;       // Distância máxima permitida da câmera

    switch (key) {
        // Rotação em torno do Eixo Z (A/D)
        case 'a': case 'A':
            anguloRotacaoZ += incrementoRot;
            break;
        case 'd': case 'D':
            anguloRotacaoZ -= incrementoRot;
            break;

        // Rotação em torno do Eixo Y (W/S) - Inclinação vertical
        case 'w': case 'W':
            anguloRotacaoY += incrementoRot;
            break;
        case 's': case 'S':
            anguloRotacaoY -= incrementoRot;
            break;

        // Rotação em torno do Eixo X (X/Z) - Inclinação lateral
        case 'x': case 'X':
            anguloRotacaoX += incrementoRot;
            break;
        case 'z': case 'Z': // Remapeado 'z' para controlar X negativo
            anguloRotacaoX -= incrementoRot;
            break;

        // Controle de Zoom (J/K) - Distância da câmera
        case 'j': case 'J': // Zoom In (aproximar) -> Diminuir distância
            cameraDistanciaZ -= incrementoZoom;
            if (cameraDistanciaZ < zoomMin) { // Limita o zoom mínimo
                cameraDistanciaZ = zoomMin;
            }
            break;
        case 'k': case 'K': // Zoom Out (afastar) -> Aumentar distância
            cameraDistanciaZ += incrementoZoom;
             if (cameraDistanciaZ > zoomMax) { // Limita o zoom máximo
                cameraDistanciaZ = zoomMax;
            }
            break;

        // Sair da aplicação
        case 27: // Tecla ESC
            exit(0);
            break;
    }
    // Solicita ao GLUT para redesenhar a janela na próxima iteração do loop
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

    // Chama nossa função de inicialização (configura OpenGL, carrega texturas)
    init();

    // ---> REGISTRA AS FUNÇÕES DE CALLBACK <---
    // Estas funções serão chamadas pelo GLUT em resposta a eventos
    glutDisplayFunc(display);   // Função a ser chamada para desenhar a cena
    glutReshapeFunc(reshape);   // Função a ser chamada quando a janela é redimensionada
    glutKeyboardFunc(keyboard); // Função a ser chamada quando uma tecla é pressionada

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