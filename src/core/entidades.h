// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/core/entidades.h
//
// Descrição:
//   Define as estruturas de dados das entidades 3D, as constantes de
//   configuração (namespace `Config`), o estado completo da simulação
//   (`JogoEstado`) e as funções matemáticas/de inicialização compartilhadas
//   pelos demais módulos.
//
// Conceitos de CG relacionados:
//   - Posição, velocidade e orientação no espaço 3D (R³)
//   - Ângulos de Euler (Pitch / Arfagem, Yaw / Guinada, Roll / Rolagem)
//   - Câmera sintética (Matriz de Visualização e Projeção Perspectiva)
//   - Interseção segmento × esfera (colisão contínua)
// ============================================================================

#ifndef ENTIDADES_H
#define ENTIDADES_H

#include "raylib.h"
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// CONSTANTES DE CONFIGURAÇÃO
// Todos os "números mágicos" do jogo ficam aqui, agrupados por assunto.
// ----------------------------------------------------------------------------

namespace Config {
    // --- Janela ---
    constexpr int LARGURA_INICIAL = 1024;
    constexpr int ALTURA_INICIAL  = 720;
    constexpr int FPS_ALVO        = 60;
    constexpr float DT_MAXIMO     = 0.1f;   // Limite do passo de tempo (evita saltos ao arrastar a janela)

    // --- Arena ---
    constexpr float RAIO_MUNDO          = 220.0f; // Raio da cúpula que limita o espaço de combate
    constexpr float BORDA_INICIO_BRILHO = 0.55f;  // Fração do raio a partir da qual a cúpula começa a acender
    constexpr float BORDA_ALERTA        = 0.85f;  // Fração do raio que dispara o aviso de fronteira
    constexpr int   TOTAL_ESTRELAS      = 750;
    constexpr int   TOTAL_ANEIS         = 8;

    // --- Voo da nave ---
    constexpr float SENSIBILIDADE_VOO   = 95.0f;  // Velocidade angular de pitch/yaw (graus/s)
    constexpr float LIMITE_PITCH        = 85.0f;  // Evita o "gimbal lock" nos polos
    constexpr float BANKING_GUINADA     = 35.0f;  // Inclinação automática das asas ao virar (graus)
    constexpr float ROLL_MANUAL         = 45.0f;  // Inclinação adicional com Q/E (graus)
    constexpr float VELOCIDADE_CRUZEIRO = 14.0f;
    constexpr float VELOCIDADE_TURBO    = 32.0f;
    constexpr float VELOCIDADE_FREIO    = 5.0f;
    constexpr float VELOCIDADE_INICIAL  = 8.0f;

    // --- Suavização (taxas "k" de FatorSuavizacao: maior = resposta mais rápida) ---
    constexpr float SUAVIZ_ROLL       = 8.0f;
    constexpr float SUAVIZ_VELOCIDADE = 4.0f;
    constexpr float SUAVIZ_CAM_POS    = 12.0f;
    constexpr float SUAVIZ_CAM_ALVO   = 14.0f;
    constexpr float SUAVIZ_CAM_UP     = 10.0f;
    constexpr float SUAVIZ_CAM_ORBITAL = 8.0f;

    // --- Câmeras (View Matrix) ---
    constexpr float FOV_PADRAO           = 60.0f;  // Campo de visão vertical (graus)
    constexpr float CAM_CHASE_DISTANCIA  = 7.5f;   // Atrás da nave
    constexpr float CAM_CHASE_ALTURA     = 2.6f;   // Acima da nave
    constexpr float CAM_CHASE_FOCO       = 15.0f;  // Ponto observado à frente da nave
    constexpr float CAM_COCKPIT_FRENTE   = 0.3f;
    constexpr float CAM_COCKPIT_ALTURA   = 0.35f;
    constexpr float CAM_COCKPIT_FOCO     = 30.0f;
    constexpr float CAM_ORBITAL_ALTURA   = 32.0f;
    constexpr float CAM_ORBITAL_ATRAS    = 24.0f;

    // --- Tremor de câmera (screen shake) ---
    constexpr float TREMOR_AMPLITUDE   = 0.35f;
    constexpr float TREMOR_DECAIMENTO  = 1.6f;   // Trauma perdido por segundo
    constexpr float TREMOR_POR_TIRO    = 0.06f;
    constexpr float TREMOR_POR_ALVO    = 0.35f;

    // --- Armas ---
    constexpr float VELOCIDADE_TIRO         = 95.0f;
    constexpr float TEMPO_VIDA_TIRO         = 3.0f;
    constexpr float CADENCIA_TIRO           = 0.16f;  // Intervalo entre disparos contínuos (s)
    constexpr float CANHAO_AFASTAMENTO      = 1.85f;  // Distância lateral dos canhões (pontas das asas)
    constexpr float CANHAO_AVANCO           = 0.4f;   // Canhões ligeiramente à frente do centro
    // Distância (à frente da nave) em que os dois lasers se cruzam. A mira do HUD
    // é projetada nesse mesmo ponto, por isso as duas constantes são uma só.
    constexpr float DISTANCIA_CONVERGENCIA  = 45.0f;
    constexpr float FATOR_COLISAO_ALVO      = 1.15f;  // Raio de colisão = escala do alvo × fator

    // --- Pontuação ---
    constexpr int PONTOS_ALVO = 100;
    constexpr int PONTOS_ONDA = 500;
    constexpr int PONTOS_ANEL = 200;

    // --- Animação ---
    constexpr float VEL_FASE_JOGO = 2.0f;  // Velocidade da fase de animação durante a partida
    constexpr float VEL_FASE_MENU = 0.8f;  // ...e no menu (mais lenta)
}

// ----------------------------------------------------------------------------
// ENUMERAÇÕES
// ----------------------------------------------------------------------------

// Modos da Matriz de Visualização (View Matrix - LookAt)
enum class ModoCamera {
    Chase,    // 3ª pessoa: câmera de perseguição com suavização inercial
    Cockpit,  // 1ª pessoa: câmera no cockpit, acompanhando o vetor Up da nave
    Orbital   // Tática: câmera elevada para visão global
};

// Tipos de geometria procedural dos alvos
enum class TipoAlvo {
    DroneSentinela,       // Cubo com núcleo esférico e dois anéis orbitais
    AsteroideCristalino,  // Cubos entrelaçados rotacionados a 45°
    PrismaEnergia         // Pirâmides duplas giratórias
};

// ----------------------------------------------------------------------------
// ENTIDADES 3D
// ----------------------------------------------------------------------------

// Projétil laser disparado pelos canhões das asas
struct Projetil3D {
    Vector3 pos;  // Posição atual (espaço do mundo)
    Vector3 dir;  // Direção unitária de propagação
    float vida;   // Tempo restante (s)
    bool ativo;
};

// Fragmento de explosão
struct Particula3D {
    Vector3 pos;
    Vector3 vel;
    Vector3 rot;     // Ângulos de Euler atuais (graus)
    Vector3 rotVel;  // Velocidade angular por eixo (graus/s)
    float tamanho;
    Color cor;
    float vida;
    float vidaMax;   // Vida inicial: normaliza o canal alpha no fade-out
    bool ativo;
};

// Alvo sujeito à transformação composta T * R * S a cada quadro
struct Alvo3D {
    Vector3 pos;        // Translação (T)
    Vector3 rotAngulo;  // Rotação acumulada em X, Y, Z (R)
    Vector3 rotVel;     // Velocidade angular por eixo (graus/s)
    float escalaBase;   // Escala antes da pulsação harmônica (S)
    Color cor;
    TipoAlvo tipo;
    bool ativo;
};

// Portão de navegação (checkpoint). O disco fica num plano vertical que contém
// o eixo Y e é perpendicular à direção radial: a nave o "atravessa" voando
// em direção à borda (ou voltando dela).
struct AnelCheckpoint {
    Vector3 pos;      // Centro do anel
    Vector3 normal;   // Normal do plano do anel (unitária, horizontal)
    float rotY;       // Guinada que alinha o modelo à normal (graus)
    float raio;       // Raio de abertura: a travessia só conta dentro dele
    bool coletado;
};

// Ponto luminoso distante do fundo estelar
struct Estrela3D {
    Vector3 pos;
    Color cor;
    float tamanho;
    float faseBrilho;  // Fase inicial (rad) da cintilação senoidal
};

// Nave do jogador: translação 3D + três rotações de Euler (pitch, yaw, roll)
struct Nave3D {
    Vector3 pos;
    Vector3 vel;            // Frente × velocidadeAtual
    float pitch;            // Arfagem: rotação em X (nariz sobe/desce)
    float yaw;              // Guinada: rotação em Y (esquerda/direita)
    float roll;             // Rolagem: rotação em Z (inclinação das asas)
    float rollDesejado;     // Alvo do roll (banking suave via suavização)
    float escala;
    float velocidadeAtual;
    bool acelerando;        // Turbinas em modo acelerado (efeito visual)
    bool turbo;
    bool freando;
};

// ----------------------------------------------------------------------------
// ESTADO DA SIMULAÇÃO
// (O estado de menus/telas fica em `EstadoApp`, em core/estado.h)
// ----------------------------------------------------------------------------

struct JogoEstado {
    Nave3D nave;
    Camera3D camera;                  // Câmera "lógica" (sem tremor)
    Vector3 tremorCamera = {0, 0, 0}; // Deslocamento de tremor, aplicado só na hora de desenhar
    ModoCamera modoCamera = ModoCamera::Chase;

    bool inverterPitch = false;  // Padrão direto: W sobe o nariz
    bool mostrarHUD_CG = true;

    int pontuacao = 0;
    int ondaAtual = 1;

    float faseAnimacao       = 0.0f;  // Fase acumulada usada em pulsações/cintilações (não é tempo real)
    float alertaTemporizador = 0.0f;
    float cameraTrauma       = 0.0f;  // Intensidade do tremor (0..1)
    float cooldownTiro       = 0.0f;  // Tempo até o próximo disparo contínuo
    std::string alertaTexto;

    std::vector<Projetil3D> tiros;
    std::vector<Particula3D> particulas;
    std::vector<Alvo3D> alvos;
    std::vector<AnelCheckpoint> aneis;
    std::vector<Estrela3D> estrelas;
};

// ----------------------------------------------------------------------------
// MATEMÁTICA AUXILIAR (src/core/entidades.cpp)
// ----------------------------------------------------------------------------

// Número pseudoaleatório em [minVal, maxVal]
float AleatorioFloat(float minVal, float maxVal);

// Vetor em direção aleatória (coordenadas esféricas) com módulo `intensidade`
Vector3 VetorAleatorio3D(float intensidade);

// Base ortonormal da nave (Forward, Up, Right) a partir dos ângulos de Euler
void CalcularOrientacaoNave(const Nave3D &nave, Vector3 &outFwd, Vector3 &outUp, Vector3 &outRight);

// Atalho para quem só precisa do vetor Forward
Vector3 ObterFrenteNave(const Nave3D &nave);

// Fator de interpolação independente do FPS: 1 - e^(-k·dt), sempre em [0, 1).
// Usar no lugar de `k * dt` direto no Lerp, que ultrapassa o alvo quando dt é grande.
float FatorSuavizacao(float k, float dt);

// Distância da nave ao centro da arena, em frações de RAIO_MUNDO (1.0 = na cúpula)
float FracaoRaioMundo(const Nave3D &nave);

// O segmento [a, b] intercepta a esfera (centro, raio)? Evita o "tunneling":
// um projétil rápido não atravessa o alvo entre dois quadros.
bool SegmentoInterceptaEsfera(Vector3 a, Vector3 b, Vector3 centro, float raio);

// Câmera usada para desenhar: a câmera lógica + o tremor do quadro
Camera3D ObterCameraRender(const JogoEstado &jogo);

// ----------------------------------------------------------------------------
// INICIALIZAÇÃO
// ----------------------------------------------------------------------------

void InicializarEstrelas(JogoEstado &jogo);
void InicializarAlvos(JogoEstado &jogo);
void InicializarAneis(JogoEstado &jogo);

// Restaura nave, câmera, placar e mundo ao estado inicial de uma missão
void ReiniciarJogoCompleto(JogoEstado &jogo);

#endif // ENTIDADES_H
