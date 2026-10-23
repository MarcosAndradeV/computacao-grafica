# Space Combat 3D - Computação Gráfica

Jogo de combate espacial 3D desenvolvido em **C++** utilizando a biblioteca **Raylib** e **OpenGL (rlgl)** para a disciplina de Computação Gráfica.

Este projeto expande o protótipo 2D ortográfico original para um ambiente espacial tridimensional completo, aplicando conceitos teóricos e práticos fundamentais de computação gráfica.

---

## 🚀 Como Compilar e Executar

### Pré-requisitos
- Compilador C++ com suporte a C++17 (`g++` ou `clang++`)
- Biblioteca **Raylib** instalada no sistema:
  - **Fedora:** `sudo dnf install raylib-devel`
  - **Debian / Ubuntu:** `sudo apt install libraylib-dev`
  - **Arch Linux:** `sudo pacman -S raylib`

### Comandos de Compilação

Para compilar ambas as versões (3D e a 2D original):
```bash
make
```

Para executar a versão 3D diretamente:
```bash
make run
# ou
./main_3d
```

Para executar a versão 2D original:
```bash
make run-2d
# ou
./main_raylib
```

Limpar arquivos binários compilados:
```bash
make clean
```

---

## 🎮 Controles da Versão 3D

| Ação | Teclas | Descrição |
|---|---|---|
| **Arfagem (Pitch)** | `W` / `S` ou `▲` / `▼` | Inclina o nariz da nave para cima/baixo (Eixo X) |
| **Guinada (Yaw)** | `A` / `D` ou `◀` / `▶` | Gira a nave para a esquerda/direita (Eixo Y) com banking dinâmico |
| **Rolagem (Roll)** | `Q` / `E` | Inclina manualmente as asas para os lados (Eixo Z) |
| **Canhões Laser** | `ESPAÇO` ou Botão Esquerdo do Mouse | Dispara lasers duplos 3D das pontas das asas |
| **Aceleração / Turbo** | `SHIFT ESQUERDO` | Aumenta velocidade e acentua chamas das turbinas de plasma |
| **Freio Espacial** | `CTRL ESQUERDO` | Desacelera a nave |
| **Alternar Câmera** | `C` ou `TAB` | Alterna entre **3ª Pessoa (Chase)**, **1ª Pessoa (Cockpit)** e **Tática (Orbital)** |
| **Inverter Eixo Y** | `I` | Alterna entre manche direto e manche de aviação invertido |
| **Painel de CG** | `H` | Exibe/oculta o painel de transformações e métricas na tela |
| **Reiniciar Jogo** | `R` | Reposiciona a nave, reseta pontuação e alvos |

---

## ℹ️ Compatibilidade de Texto no Raylib
O HUD e as mensagens em tela utilizam exclusivamente caracteres padrão ASCII (`A-Z`, `0-9`, pontuação básica), garantindo renderização nítida sem artefatos ou caracteres `?`, já que a fonte padrão do Raylib não carrega glifos acentuados UTF-8.

---

## 📐 Conceitos de Computação Gráfica Implementados

1. **Pipeline de Transformações Geométricas ($M = T \times R_y \times R_x \times R_z \times S$):**
   - Utilização das funções de pilha de matrizes do OpenGL/rlgl (`rlPushMatrix()`, `rlPopMatrix()`).
   - Aplicação hierárquica de **Translação 3D** (`rlTranslatef`), **Rotações de Euler** em 3 eixos (`rlRotatef`) e **Escala Tridimensional** (`rlScalef`).

2. **Modelagem Geométrica 3D com Vértices e Cores:**
   - Nave espacial 3D com geometria aerodinâmica personalizada (fuselagem em prisma diedral, asas anguladas com canhões nas pontas, canopy translúcida e leme vertical).
   - Efeito visual de propulsão com jatos de plasma 3D animados senoidalmente e bocais de exaustão.

3. **Câmera 3D e Matriz de Visualização (View Matrix):**
   - **Modo 3ª Pessoa (Chase Cam):** Câmera dinâmica com interpolação inercial (`Lerp`), acompanhando a rotação da nave e roll dinâmico.
   - **Modo 1ª Pessoa (Cockpit View):** Visão imersiva com horizonte artificial (pitch ladder) e bússola angular (heading tape).
   - **Modo Visão Tática (Orbital):** Câmera elevada para inspeção global do campo de batalha espacial.
   - **Trauma de Câmera (Screen Shake):** Vibração suave da matriz de visualização no disparo de lasers e detonação de alvos.

4. **Matriz de Projeção Perspectiva:**
   - Projeção tridimensional com campo de visão (FOV 60°), plano de corte próximo e distante.
   - Projeção de coordenadas do mundo para a tela (`GetWorldToScreen`) para retículo de mira e caixas de mira holográficas (*bounding brackets*) nos alvos.

5. **Alvos e Transformações Compostas Dinâmicas:**
   - Alvos tridimensionais (Drones de defesa com anéis em rotações cruzadas, Asteroides facetados e Cristais de energia).
   - Escala harmônica pulsante: $S(t) = S_{\text{base}} \cdot (1 + A \cdot \sin(\omega t + \phi))$.

6. **Sistema de Partículas 3D:**
   - Explosões volumétricas ao destruir alvos, com fragmentos com vetores de velocidade tridimensionais aleatórios, velocidade angular própria e decaimento temporal (*fade-out*).

7. **Ambiente Espacial:**
   - Campo estelar infinito gerado em coordenadas esféricas 3D.
   - Grade holográfica de referência espacial para ancoragem de profundidade e paralaxe.
   - Anéis de checkpoint espaciais para navegação e pontuação extra.
