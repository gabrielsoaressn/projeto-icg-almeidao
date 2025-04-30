# Simulação 3D do Estádio Almeidão (OpenGL)

## Descrição

Este projeto é uma simulação simplificada em 3D do Estádio José Américo de Almeida Filho, conhecido como "Almeidão", localizado em João Pessoa, Paraíba. O foco principal é a modelagem das características arquibancadas elípticas do estádio, utilizando C++, OpenGL (com GLUT e GLU), texturização básica, controle de câmera interativo e um ciclo dia/noite automático.

As dimensões utilizadas para as arquibancadas, degraus (batentes), paredes e marquise são **fictícias e escalonadas**, baseadas em conceitos gerais de estádios, e não nas medidas exatas do Almeidão real. O objetivo principal foi explorar técnicas de modelagem 3D, texturização, controle de câmera e efeitos visuais simples em OpenGL.

## Funcionalidades

*   **Modelagem 3D Detalhada:**
    *   Arquibancadas elípticas com seções principais e de conexão.
    *   Degraus (batentes) 3D individuais com altura progressiva.
    *   Seção especial (140°-220°) com mais degraus, maior altura e raio externo estendido.
    *   Parede externa superior adicional sobre a seção especial.
    *   Marquise (cobertura) com espessura sobre a segunda metade da seção especial (180°-220°).
    *   Gramado elíptico com folga para a arquibancada.
    *   Chão de terra externo.
*   **Texturização:** Aplicação de texturas de concreto, terra e grama (`concreto.jpg`, `terra.jpeg`, `grama.jpg`) nas superfícies correspondentes.
*   **Visualização 3D:** Projeção perspectiva (`gluPerspective`) e teste de profundidade (`GL_DEPTH_TEST`).
*   **Câmera Interativa:**
    *   Movimentação livre (cima/baixo 'W'/'S', frente/trás 'J'/'K') relativa à direção da câmera.
    *   Rotação da visão horizontal (esquerda/direita 'A'/'D' e arrastar mouse com botão esquerdo).
*   **Ciclo Dia/Noite Automático:**
    *   Transição visual suave entre dia (céu claro) e noite (cena escurecida).
    *   Realizada através de um filtro de cor semi-transparente desenhado sobre a tela, cuja opacidade (`alphaFiltro`) varia ao longo do tempo controlada por `glutTimerFunc`.
    *   O ciclo alterna automaticamente entre os modos dia e noite via `glutIdleFunc`.
*   **(Código Experimental)** Inclui código para cálculo de pontos em uma curva de Bézier cúbica, que poderia ser usado para um modo alternativo de movimentação da câmera (atualmente não ativado nos controles principais).

## Evolução do Desenvolvimento

O código evoluiu através das seguintes etapas principais:

1.  **Estrutura Base:** Desenho 2D inicial, transição para 3D com paredes e degraus básicos usando `GL_TRIANGLE_STRIP` e texturização.
2.  **Detalhes Geométricos:** Criação da seção especial da arquibancada, adição da parede superior e da marquise com espessura.
3.  **Ambiente:** Desenho do chão de terra e do gramado elíptico texturizado com folga.
4.  **Câmera e Interação Inicial:** Implementação de câmera 3D (`gluPerspective`, `gluLookAt`) e controles básicos de teclado para rotação/zoom do *objeto*.
5.  **Câmera Interativa Aprimorada:** Modificação dos controles de teclado (WASDJK) para movimentação da *câmera* e adição de rotação horizontal via mouse.
6.  **Ciclo Dia/Noite:** Implementação do sistema de transição automática dia/noite com filtro alfa.
7.  **(Exploração)** Adição de código para cálculo de curva de Bezier como uma alternativa potencial para a trajetória da câmera.
8.  **(Tentativa)** Exploração inicial de adição de fontes de luz OpenGL (`glLightfv`) para refletores (não funcional no estado atual devido à ausência de cálculo de vetores normais na geometria customizada).

## Pré-requisitos e Dependências

Para compilar e executar este projeto, você precisará ter instalado:

1.  **Compilador C++:** `g++` (parte do GCC ou ferramentas como MinGW/MSYS2 no Windows).
2.  **Bibliotecas de Desenvolvimento OpenGL:** Incluem os cabeçalhos (`GL/gl.h`, `GL/glu.h`) e bibliotecas para linkagem.
    *   **Linux (Debian/Ubuntu):** `sudo apt update && sudo apt install build-essential mesa-common-dev libglu1-mesa-dev freeglut3-dev`
    *   **Linux (Fedora):** `sudo dnf install gcc-c++ make mesa-libGL-devel mesa-libGLU-devel freeglut-devel`
    *   **macOS:** Instale o Xcode ou as "Command Line Tools for Xcode".
    *   **Windows (via MSYS2/MinGW):** `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-mesa mingw-w64-x86_64-freeglut`
3.  **Biblioteca de Desenvolvimento GLUT (ou FreeGLUT):** Usada para criar janelas e gerenciar eventos (`GL/glut.h`). FreeGLUT é a alternativa moderna recomendada. (Comandos acima já incluem `freeglut`).

## Texturas

*   O projeto utiliza a biblioteca `stb_image.h` (incluída diretamente no código fonte com `#define STB_IMAGE_IMPLEMENTATION`) para carregar as texturas. Nenhuma biblioteca de imagem externa adicional é necessária para compilar.
*   Os arquivos de textura (`concreto.jpg`, `terra.jpeg`, `grama.jpg`) **devem estar presentes no mesmo diretório do arquivo executável** quando você for rodar o programa.

## Como Compilar

Abra seu terminal ou prompt de comando, navegue até a pasta onde está o código-fonte (`.cpp`), `stb_image.h` e os arquivos de textura, e use o comando de compilação apropriado:

*   **Linux / macOS (com FreeGLUT):**
    ```bash
    g++ seu_arquivo.cpp -o almeidao_app -lGL -lGLU -lglut -lm
    ```
    *(Se linkar com FreeGLUT explicitamente: `g++ seu_arquivo.cpp -o almeidao_app -lGL -lGLU -lfreeglut -lm`)*

*   **Windows (usando terminal MinGW-w64 do MSYS2 com FreeGLUT):**
    ```bash
    g++ seu_arquivo.cpp -o almeidao_app.exe -lopengl32 -lglu32 -lfreeglut -lm
    ```

**Observações:**
*   Substitua `seu_arquivo.cpp` pelo nome real do seu arquivo C++.
*   `-o almeidao_app` define o nome do executável.
*   As flags `-l...` ou `-framework...` linkam as bibliotecas necessárias.

## Como Executar

Após a compilação bem-sucedida, execute o programa a partir do mesmo diretório no terminal:

*   **Linux / macOS:**
    ```bash
    ./almeidao_app
    ```

*   **Windows:**
    ```bash
    .\almeidao_app.exe
    ```

## Controles

*   **Mouse (Arrastar com Botão Esquerdo):** Gira a visão da câmera horizontalmente.
*   **W / S:** Move a câmera para cima / para baixo (no eixo Y global).
*   **A / D:** Gira a visão da câmera para esquerda / direita (alternativa ao mouse).
*   **J / K:** Move a câmera para frente / para trás (na direção que está olhando).
*   **ESC:** Fecha a janela e encerra o programa.

## Próximos Passos / Limitações

*   **Iluminação e Sombreamento:** A principal limitação é a ausência de um sistema de iluminação dinâmico funcional. Próximos passos incluiriam:
    *   Calcular e adicionar vetores normais (`glNormal3f`) a todas as superfícies desenhadas customizadas (paredes, degraus, marquise, tampas).
    *   Implementar fontes de luz (`glLightfv`) para simular o sol (dia) e os refletores (noite), ativando/desativando-as de acordo com o ciclo dia/noite.
    *   Explorar técnicas de sombreamento (como shadow mapping, se avançar para shaders) para maior realismo.
*   **Câmera de Bezier:** Ativar e integrar os controles para a movimentação da câmera ao longo da curva de Bezier pré-definida como um modo de visualização alternativo (ex: tour cinematográfico).
*   **Otimização:** Para cenas mais complexas, otimizar o desenho usando Vertex Arrays ou VBOs em vez de `glBegin/glEnd` (modo imediato).
*   **Detalhes Visuais:** Adicionar mais detalhes ao modelo (postes, placar, etc.) e usar texturas de maior resolução ou mais variadas.

---
