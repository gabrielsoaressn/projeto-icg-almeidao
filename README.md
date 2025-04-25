
# Simulação 3D do Estádio Almeidão (OpenGL)

## Descrição

Este projeto é uma simulação simplificada em 3D do Estádio José Américo de Almeida Filho, conhecido como "Almeidão", localizado em João Pessoa, Paraíba. O foco principal é a modelagem das características arquibancadas do estádio, utilizando C++, OpenGL (com GLUT e GLU) e texturização.

As dimensões utilizadas para as arquibancadas e seus degraus (batentes) são **fictícias e escalonadas**, baseadas em conceitos gerais de estádios, e não nas medidas exatas do Almeidão real. O objetivo principal foi explorar técnicas de modelagem 3D, texturização e controle de câmera em OpenGL.

## Funcionalidades

*   **Modelagem das Arquibancadas:** Representadas como seções parciais de elipses.
*   **Estrutura de Degraus:** Arquibancadas compostas por 15 degraus (batentes) 3D, com altura progressiva (mais baixos na parte interna, mais altos na externa).
*   **Texturização:** Aplicação de uma textura de concreto (`concreto.jpg`) em todas as superfícies das arquibancadas e degraus.
*   **Visualização 3D:** Utilização de projeção perspectiva para visualização em três dimensões.
*   **Controle de Câmera Interativo:**
    *   Rotação em torno dos eixos X, Y e Z.
    *   Controle de zoom (aproximação/afastamento).

## Evolução do Desenvolvimento

O código evoluiu através das seguintes etapas principais:

1.  **Elipse 2D Parcial:** Começou com o desenho simples do contorno de uma elipse, omitindo certos intervalos angulares, usando `GL_LINE_STRIP`.
2.  **Engrossamento do Contorno:** Tentativas de tornar a linha mais espessa, primeiro desenhando múltiplos contornos (interno e externo) e depois conectando suas pontas com `GL_LINES`.
3.  **Faixa Preenchida e Texturizada:** A abordagem foi alterada para desenhar diretamente a "parede" da arquibancada como uma área preenchida (usando `GL_TRIANGLE_STRIP` ou `GL_QUAD_STRIP`), permitindo a aplicação de textura.
4.  **Conversão para 3D:** O sistema de coordenadas foi mudado de 2D (`gluOrtho2D`) para 3D com perspectiva (`gluPerspective`), e uma câmera virtual foi posicionada com `gluLookAt`. O teste de profundidade (`GL_DEPTH_TEST`) foi habilitado.
5.  **Batentes 3D Progressivos:** A lógica de desenho foi refeita para construir a arquibancada degrau por degrau. Cada degrau é um prisma curvo 3D com altura calculada progressivamente, baseado em uma altura mínima (40cm) e máxima (6m) escalonadas. A função `desenharDegrauArquibancada` foi criada para modelar cada degrau.
6.  **Controles de Câmera:** Foram implementadas rotações nos eixos X, Y e Z e um controle de zoom (distância da câmera) através de callbacks do teclado (`glutKeyboardFunc`).

## Pré-requisitos e Dependências

Para compilar e executar este projeto, você precisará ter instalado:

1.  **Compilador C++:** `g++` (parte do GCC ou ferramentas como MinGW/MSYS2 no Windows).
2.  **Bibliotecas de Desenvolvimento OpenGL:** Incluem os cabeçalhos e bibliotecas para linkagem.
    *   **Linux (Debian/Ubuntu):** `sudo apt update && sudo apt install build-essential mesa-common-dev libglu1-mesa-dev`
    *   **Linux (Fedora):** `sudo dnf install gcc-c++ make mesa-libGL-devel mesa-libGLU-devel`
    *   **macOS:** Instale o Xcode ou as "Command Line Tools for Xcode" (geralmente já incluem OpenGL/GLU).
    *   **Windows (via MSYS2/MinGW):** `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-mesa`
3.  **Biblioteca de Desenvolvimento GLUT (ou FreeGLUT):** Usada para criar janelas e gerenciar eventos. FreeGLUT é a alternativa moderna mais comum.
    *   **Linux (Debian/Ubuntu):** `sudo apt install freeglut3-dev`
    *   **Linux (Fedora):** `sudo dnf install freeglut-devel`
    *   **macOS:** Incluído no Xcode/Command Line Tools.
    *   **Windows (via MSYS2/MinGW):** `pacman -S mingw-w64-x86_64-freeglut`

## Textura

*   O projeto utiliza a biblioteca `stb_image.h` (incluída diretamente no código fonte) para carregar a textura. Nenhuma biblioteca de imagem externa adicional é necessária para compilar.
*   O arquivo de textura `concreto.jpg` **deve estar presente no mesmo diretório do arquivo executável** quando você for rodar o programa.

## Como Compilar

Abra seu terminal ou prompt de comando, navegue até a pasta onde você clonou/baixou este repositório (onde estão o arquivo `.cpp`, `stb_image.h` e `concreto.jpg`), e use o comando de compilação apropriado para seu sistema:

*   **Linux:**
    ```bash
    g++ almeidao.cpp -o almeidao_app -lGL -lGLU -lglut -lm
    ```
    *(Substitua `-lglut` por `-lfreeglut` se você instalou o freeglut)*

*   **macOS:**
    ```bash
    g++ almeidao.cpp -o almeidao_app -framework OpenGL -framework GLUT -lm
    ```

*   **Windows (usando terminal MinGW-w64 do MSYS2):**
    ```bash
    g++ almeidao.cpp -o almeidao_app.exe -lopengl32 -lglu32 -lfreeglut -lm
    ```
    *(Substitua `-lfreeglut` por `-lglut32` se estiver usando o GLUT original)*

**Observações:**
*   `-o almeidao_app` define o nome do executável de saída.
*   `-lGL`, `-lGLU`, `-lglut`/`-lfreeglut`, `-lm` linkam as bibliotecas necessárias.
*   `-framework` é a forma de linkar no macOS.
*   `-lopengl32`, `-lglu32`, etc., são os nomes comuns das bibliotecas no Windows.

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

Use o teclado para interagir com a simulação:

*   **A / D:** Girar a visualização em torno do eixo Z.
*   **W / S:** Girar a visualização em torno do eixo Y.
*   **X / Z:** Girar a visualização em torno do eixo X.
*   **J:** Afastar a câmera (aumentar zoom out).
*   **K:** Aproximar a câmera (aumentar zoom in).
*   **ESC:** Fechar a janela e sair do programa.

---
