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

A **Galeria de Arte 3D Interativa** é uma aplicação em tempo real desenvolvida em **C++17**, utilizando **OpenGL**, **Raylib** e Shaders GLSL (OpenGL 3.3 Core). O projeto implementa um museu virtual tridimensional com sala expositiva, iluminação dinâmica e quatro obras de arte dispostas sobre pedestais de mármore.

Todos os conceitos fundamentais exigidos pelo professor estão integralmente implementados:
1. **Transformações Geométricas 3D Compostas:** Translação, rotação e escala individualizadas para cada obra em seus três eixos cartesianos ($X, Y, Z$).
2. **Visualização Tridimensional (Viewing 3D):** Câmera orbital centrada na galeria, movimentação e rotação angular, aproximação/afastamento (*zoom*) e alternância dinâmica entre **Projeção Perspectiva** (`CAMERA_PERSPECTIVE`) e **Projeção Ortográfica** (`CAMERA_ORTHOGRAPHIC`).
3. **Modelo de Iluminação e Materiais de Phong/Blinn-Phong:** Implementado via shaders GLSL com:
   - Fonte de luz direcional distante (`uDirLightDir`).
   - Fonte de luz posicional próxima móvel com atenuação física quadrática (`uPosLightPos`).
   - Toggles independentes das componentes **Ambiente**, **Difusa** e **Especular**.
   - Propriedades de materiais clássicos: **Ouro**, **Rubi**, **Esmeralda** e **Cromo**.
4. **Identificação Visual Integrada (HUD 2D):** Painel informativo desenhado em tempo real sobre a tela com informações da entidade ativa, coordenadas, projeção, estado das luzes e guia de teclas.

---

## 📦 Estrutura e Organização do Código

A mesma estrutura modular foi preservada:

```text
computacao-grafica/
├── Makefile                # Script de compilação automatizada com Raylib
├── README.md               # Documentação resumida do projeto e manual de uso
└── src/
    ├── main.cpp            # Loop principal da janela Raylib e leitura de entradas
    ├── material.h          # Coeficientes ópticos dos materiais (Ouro, Rubi, Esmeralda, Cromo)
    ├── object.h / .cpp     # Classe ArtObject: malhas 3D e matrizes de transformações compostas
    ├── camera.h / .cpp     # Classe GalleryCamera: coordenadas esféricas, zoom, pan e projeção
    ├── lighting.h / .cpp   # Classe LightingManager: shader GLSL Phong, luzes e toggles F1-F5
    ├── scene.h / .cpp      # Classe Scene: arquitetura da sala, piso xadrez, paredes e pedestais
    └── hud.h / .cpp        # Classe HUD: overlay 2D em tempo real
```

---

## 🎨 Obras Expostas e Materiais

| Objeto | Primitiva / Geometria | Material | Propriedades Visuais Notáveis |
| :---: | :--- | :--- | :--- |
| **1** | **Nó Escultural (Trefoil Knot)** | **Ouro** | Alta reflexão especular com tonalidade amarela dourada e brilho metálico nobre. |
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
| **`1`** | Seleciona o **Objeto 1** (*Nó Escultural em Ouro*) |
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
| **`F1`** | Ativar / Desativar a **Fonte de Luz Direcional** |
| **`F2`** | Ativar / Desativar a **Fonte de Luz Posicional** |
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
Certifique-se de possuir a biblioteca `raylib` instalada:

```bash
# Ubuntu / Debian / Pop!_OS:
sudo apt install libraylib-dev

# Fedora:
sudo dnf install raylib-devel

# Arch Linux:
sudo pacman -S raylib
```

### Compilação
```bash
make
```

### Execução
```bash
make run
# ou:
./galeria_arte
```
