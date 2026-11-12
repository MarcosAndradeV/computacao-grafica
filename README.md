# Galeria de Arte 3D Interativa — Computação Gráfica

**Componente Curricular:** Computação Gráfica  
**Docente:** José Humberto Góes Alves  
**Curso:** Engenharia de Computação — Período 2026.2  
**Atividade Avaliativa:** Desenvolvimento de uma Galeria de Arte 3D Interativa (BF3)

---

##  Integrantes

- David Ryan
- Kaique Maia 
- Tácio Vinícius 
- Marcos Vinícius 

---

## 🏛️ Descrição da Aplicação

A **Galeria de Arte 3D Interativa** é uma aplicação em tempo real desenvolvida em **C++17**, utilizando **OpenGL**, **GLU** e **FreeGLUT**. O projeto implementa um museu virtual tridimensional com sala expositiva, iluminação dinâmica e quatro obras de arte dispostas sobre pedestais de mármore.

A aplicação utiliza a **pipeline fixa do OpenGL**, demonstrando de forma prática e interativa os fundamentos da disciplina:
1. **Transformações Geométricas 3D Compostas:** Translação, rotação e escala individualizadas para cada obra em seus três eixos cartesianos ($X, Y, Z$).
2. **Visualização Tridimensional (Viewing 3D):** Câmera orbital centrada na galeria, movimentação e rotação angular, aproximação/afastamento (*zoom*) e alternância dinâmica entre **Projeção Perspectiva** (`gluPerspective`) e **Projeção Ortográfica** (`glOrtho`).
3. **Modelo de Iluminação e Materiais de Phong/Blinn-Phong:** Fonte de luz direcional distante (`GL_LIGHT0`), fonte de luz posicional próxima móvel (`GL_LIGHT1`), toggles independentes das componentes **Ambiente**, **Difusa** e **Especular**, além de materiais com propriedades de reflexão e brilho (*shininess*) característicos (Ouro, Rubi, Esmeralda e Cromo).
4. **Identificação Visual Integrada (HUD 2D):** Painel informativo desenhado diretamente na viewport OpenGL com informações sobre a entidade selecionada, coordenadas, matriz de projeção, estado das lâmpadas e guia rápido de teclas.

---

## 📦 Estrutura e Organização do Código

O código foi projetado de forma modular, com separação clara de responsabilidades conforme exigido no Item 5 do documento de avaliação:

```text
computacao-grafica/
├── galeria_arte.cpp        # CÓDIGO FONTE CONCATENADO EM ARQUIVO ÚNICO (Para revisão/perguntas do professor)
├── Makefile                # Script de compilação automatizada
├── README.md               # Documentação resumida do projeto e manual de uso
└── src/                    # Versão modular do projeto
    ├── main.cpp            # Ponto de entrada, inicialização FreeGLUT e despacho de callbacks
    ├── material.h          # Definições de materiais ópticos (Ouro, Rubi, Esmeralda, Cromo)
    ├── object.h / .cpp     # Classe ArtObject: geometria das obras e transformações 3D locais
    ├── camera.h / .cpp     # Classe Camera: controle orbital, zoom, pan e matrizes de projeção
    ├── lighting.h / .cpp   # Classe LightingManager: fontes de luz GL_LIGHT0/1 e toggles
    ├── scene.h / .cpp      # Classe Scene: arquitetura da sala, piso xadrez, paredes e pedestais
    └── hud.h / .cpp        # Classe HUD: overlay gráfico 2D na tela (gluOrtho2D)
```

---

## 🎨 Obras Expostas e Materiais

| Objeto | Primitiva / Geometria | Material | Propriedades Visuais Notáveis |
| :---: | :--- | :--- | :--- |
| **1** | **Utah Teapot** | **Ouro** | Alta reflexão especular com tonalidade amarela dourada e brilho metálico nobre. |
| **2** | **Esfera Polida** | **Rubi** | Tom avermelhado vítreo clássico com destaque especular pontual bem concentrado. |
| **3** | **Toro Geométrico** | **Esmeralda** | Tonalidade verde translúcida com componente difusa e reflexão especular acentuada. |
| **4** | **Cone Escultural** | **Cromo / Prata** | Metal prateado polido de altíssimo brilho (*shininess* elevado) e reflexão difusa balanceada. |
| **0** | **Lâmpada Posicional** | **Esfera Emissiva** | Marcador amarelo emissivo no teto/espaço da sala indicando a posição exata da luz posicional. |

---

## ⌨️ Relação Completa de Teclas e Controles

Todas as funcionalidades do projeto são acessíveis interativamente pelo teclado:

### 1. Seleção de Entidades
| Tecla | Ação |
| :---: | :--- |
| **`1`** | Seleciona o **Objeto 1** (*Utah Teapot em Ouro*) |
| **`2`** | Seleciona o **Objeto 2** (*Esfera Polida em Rubi*) |
| **`3`** | Seleciona o **Objeto 3** (*Toro Geométrico em Esmeralda*) |
| **`4`** | Seleciona o **Objeto 4** (*Cone Escultural em Cromo*) |
| **`0`** | Seleciona a **Luz Posicional** para movimentação espacial |

### 2. Transformações Geométricas 3D (no Objeto Selecionado ou Luz)
| Tecla | Ação |
| :---: | :--- |
| **`W` / `S`** | Transladar no **Eixo Z** (frente / trás) |
| **`A` / `D`** | Transladar no **Eixo X** (esquerda / direita) |
| **`Q` / `E`** | Transladar no **Eixo Y** (baixo / cima) |
| **`I` / `K`** | Rotacionar em torno do **Eixo X** |
| **`J` / `L`** | Rotacionar em torno do **Eixo Y** |
| **`U` / `O`** | Rotacionar em torno do **Eixo Z** |
| **`+` / `-`** | Aumentar ou diminuir a **Escala** |

### 3. Câmera e Visualização 3D (Viewing 3D)
| Tecla | Ação |
| :---: | :--- |
| **Setas Direcionais** | Rotacionar a câmera ao redor do centro da galeria (azimute e elevação) |
| **`Page Up` / `Page Down`** | Aproximar ou afastar a visualização (*Zoom in / Zoom out*) |
| **`Shift` + Setas** | Deslocar o ponto de observação (*Pan* da câmera nos eixos X e Y) |
| **`P`** | Alternar entre **Projeção Perspectiva** e **Projeção Ortográfica** |

### 4. Iluminação e Componentes da Luz
| Tecla | Ação |
| :---: | :--- |
| **`F1`** | Ativar / Desativar a **Fonte de Luz Direcional** (`GL_LIGHT0`) |
| **`F2`** | Ativar / Desativar a **Fonte de Luz Posicional** (`GL_LIGHT1`) |
| **`F3`** | Ativar / Desativar a componente de **Iluminação Ambiente** |
| **`F4`** | Ativar / Desativar a componente de **Iluminação Difusa** |
| **`F5`** | Ativar / Desativar a componente de **Iluminação Especular** |

### 5. Controles Gerais
| Tecla | Ação |
| :---: | :--- |
| **`R`** | **Restaurar configurações iniciais** (reseta posições, luzes, rotações, escalas e câmera) |
| **`H`** | Exibir / Ocultar o guia de atalhos expandido no HUD 2D |
| **`Esc`** | Sair da aplicação |

---

## 🛠️ Como Compilar e Executar

### Pré-requisitos
No Linux (Ubuntu/Debian/Fedora/Arch), certifique-se de possuir o compilador C++ e as bibliotecas de desenvolvimento do OpenGL e FreeGLUT:

```bash
# Ubuntu / Debian / Pop!_OS:
sudo apt update
sudo apt install build-essential freeglut3-dev libglu1-mesa-dev mesa-common-dev

# Fedora / RHEL:
sudo dnf install gcc-c++ freeglut-devel mesa-libGLU-devel

# Arch Linux:
sudo pacman -S base-devel freeglut glu
```

### Compilação

**1. Compilação direta do arquivo único (`galeria_arte.cpp`):**
```bash
g++ -Wall -Wextra -O2 -std=c++17 galeria_arte.cpp -lglut -lGLU -lGL -lm -o galeria_arte
```

**2. Compilação via Makefile:**
```bash
make          # Compila o arquivo único concatenado (galeria_arte.cpp)
make modular  # Alternativa: compila a versão modular (src/)
```
O executável binário `galeria_arte` será gerado automaticamente.

### Execução
Para iniciar o programa:
```bash
make run
# ou diretamente:
./galeria_arte
```

### Limpeza
Para remover os arquivos objeto e o executável compilado:
```bash
make clean
```

---

## 🎬 Roteiro Recomendado para o Vídeo Demonstrativo

Para atender com excelência ao requisito do vídeo avaliativo, sugerimos a seguinte sequência de gravação:

1. **Apresentação Inicial (0:00 - 0:20):**
   - Apresentar o ambiente geral da galeria: sala com piso xadrez, paredes iluminadas e os 4 pedestais de exposição.
2. **Transformações Geométricas 3D (0:20 - 1:00):**
   - Selecionar o Objeto 1 (pressionando `1`) e demonstrar translação em X/Y/Z (`A/D`, `Q/E`, `W/S`).
   - Demonstrar rotação nos três eixos (`I/K`, `J/L`, `U/O`).
   - Demonstrar aumento e redução de escala (`+` e `-`).
   - Repetir rapidamente com outro objeto (ex: `2` ou `3`).
3. **Visualização 3D e Projeção (1:00 - 1:40):**
   - Rotacionar a câmera utilizando as **setas direcionais**.
   - Demonstrar o zoom in e zoom out via **Page Up** e **Page Down**.
   - Deslocar a câmera com **Shift + Setas**.
   - Pressionar **`P`** para alternar para a **Projeção Ortográfica** e retornar para a **Projeção Perspectiva**.
4. **Iluminação e Materiais (1:40 - 2:40):**
   - Pressionar **`0`** para selecionar a **Luz Posicional** e movimentá-la pela sala (`A/D`, `Q/E`, `W/S`), evidenciando como os reflexos especulares deslizam sobre o Ouro, Rubi, Esmeralda e Cromo.
   - Alternar as fontes: desligar a luz direcional (**`F1`**) e posicional (**`F2`**).
   - Demonstrar o isolamento das componentes: desligar a componente ambiente (**`F3`**), difusa (**`F4`**) e especular (**`F5`**), mostrando o efeito de cada uma nos objetos.
   - Pressionar **`R`** para demonstrar o reset completo da cena para as configurações iniciais.
