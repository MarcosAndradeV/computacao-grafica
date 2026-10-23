// dependencias:
//      sdl2-compat-devel ou SDL2-devel (Fedora) / libsdl2-dev (Debian/Ubuntu)
//      libglvnd-devel (Fedora) / libgl1-mesa-dev (Debian/Ubuntu)
//      flags/bibliotecas: -lSDL2 -lGL

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cmath>
#include <vector>

struct Projetil {
    float x, y;
    float dirX, dirY;
    bool ativo;
};

struct Alvo {
    float x, y;
    float escalaBase;
    float rotAngulo;
    float rotVel;
    bool ativo;
};

void desenharCuboArame(float tamanho) {
    float m = tamanho / 2.0f;
    glBegin(GL_LINE_LOOP);
        glVertex3f(-m, -m,  m); glVertex3f( m, -m,  m);
        glVertex3f( m,  m,  m); glVertex3f(-m,  m,  m);
    glEnd();
    glBegin(GL_LINE_LOOP);
        glVertex3f(-m, -m, -m); glVertex3f( m, -m, -m);
        glVertex3f( m,  m, -m); glVertex3f(-m,  m, -m);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(-m, -m,  m); glVertex3f(-m, -m, -m);
        glVertex3f( m, -m,  m); glVertex3f( m, -m, -m);
        glVertex3f( m,  m,  m); glVertex3f( m,  m, -m);
        glVertex3f(-m,  m,  m); glVertex3f(-m,  m, -m);
    glEnd();
}

void desenharNave() {
    glBegin(GL_TRIANGLES);
        glColor3f(0.2f, 0.8f, 1.0f);
        glVertex3f(0.0f,  0.8f, 0.0f);
        glColor3f(0.1f, 0.4f, 0.8f);
        glVertex3f(-0.5f, -0.6f, 0.0f);
        glVertex3f(0.5f, -0.6f, 0.0f);

        glColor3f(0.9f, 0.2f, 0.2f);
        glVertex3f(-0.5f, -0.6f, 0.0f);
        glVertex3f(-0.8f, -0.8f, 0.0f);
        glVertex3f(-0.3f, -0.2f, 0.0f);

        glVertex3f(0.5f, -0.6f, 0.0f);
        glVertex3f(0.8f, -0.8f, 0.0f);
        glVertex3f(0.3f, -0.2f, 0.0f);
    glEnd();
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* janela = SDL_CreateWindow("Transformacoes Geometricas 3D - SDL2 + OpenGL",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          600, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GLContext contexto = SDL_GL_CreateContext(janela);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-5.0, 5.0, -5.0, 5.0, -5.0, 5.0);

    float posX = 0.0f, posY = -3.5f;
    float anguloNave = 0.0f, escalaNave = 0.8f;
    float tempoAnimacao = 0.0f;

    std::vector<Projetil> tiros;
    std::vector<Alvo> alvos = {
        { -3.0f,  2.5f, 0.6f, 0.0f,  1.5f, true },
        { -1.0f,  3.0f, 0.7f, 0.0f, -2.0f, true },
        {  1.0f,  3.0f, 0.7f, 0.0f,  2.5f, true },
        {  3.0f,  2.5f, 0.6f, 0.0f, -1.8f, true }
    };

    bool rodando = true;
    SDL_Event evento;

    while (rodando) {
        tempoAnimacao += 0.016f;

        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) rodando = false;
            if (evento.type == SDL_KEYDOWN) {
                if (evento.key.keysym.sym == SDLK_ESCAPE) rodando = false;
                if (evento.key.keysym.sym == SDLK_SPACE) {
                    float rad = (anguloNave + 90.0f) * 3.14159f / 180.0f;
                    tiros.push_back({ posX, posY, std::cos(rad), std::sin(rad), true });
                }
            }
        }

        const Uint8* teclas = SDL_GetKeyboardState(NULL);
        if (teclas[SDL_SCANCODE_LEFT])  anguloNave += 3.0f;
        if (teclas[SDL_SCANCODE_RIGHT]) anguloNave -= 3.0f;
        if (teclas[SDL_SCANCODE_UP]) {
            float rad = anguloNave * 3.14159f / 180.0f;
            posX += -std::sin(rad) * 0.07f;
            posY +=  std::cos(rad) * 0.07f;
        }

        // Atualização de Tiros e Colisão
        for (auto& tiro : tiros) {
            if (!tiro.ativo) continue;
            tiro.x += tiro.dirX * 0.15f;
            tiro.y += tiro.dirY * 0.15f;
            if (tiro.y > 5.5f || tiro.y < -5.5f || tiro.x > 5.5f || tiro.x < -5.5f) {
                tiro.ativo = false;
            }
            for (auto& alvo : alvos) {
                if (!alvo.ativo) continue;
                float dx = tiro.x - alvo.x;
                float dy = tiro.y - alvo.y;
                if (std::sqrt(dx * dx + dy * dy) < (alvo.escalaBase + 0.2f)) {
                    alvo.ativo = false;
                    tiro.ativo = false;
                    break;
                }
            }
        }

        for (auto& alvo : alvos) {
            if (alvo.ativo) alvo.rotAngulo += alvo.rotVel;
        }

        // Renderização OpenGL
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // 1. Nave
        glPushMatrix();
            glTranslatef(posX, posY, 0.0f);
            glRotatef(anguloNave, 0.0f, 0.0f, 1.0f);
            glScalef(escalaNave, escalaNave, 1.0f);
            desenharNave();
        glPopMatrix();

        // 2. Tiros
        glColor3f(1.0f, 1.0f, 0.0f);
        for (const auto& tiro : tiros) {
            if (!tiro.ativo) continue;
            glPushMatrix();
                glTranslatef(tiro.x, tiro.y, 0.0f);
                glScalef(0.15f, 0.3f, 1.0f);
                desenharCuboArame(1.0f);
            glPopMatrix();
        }

        // 3. Alvos
        for (size_t i = 0; i < alvos.size(); ++i) {
            if (!alvos[i].ativo) continue;
            glPushMatrix();
                glTranslatef(alvos[i].x, alvos[i].y, 0.0f);
                glRotatef(alvos[i].rotAngulo, 0.0f, 1.0f, 0.0f);
                glRotatef(alvos[i].rotAngulo * 0.5f, 0.0f, 0.0f, 1.0f);

                float fatorPulso = 1.0f + 0.2f * std::sin(tempoAnimacao * 3.0f + (float)i);
                float esc = alvos[i].escalaBase * fatorPulso;
                glScalef(esc, esc, esc);

                glColor3f(0.2f, 1.0f, 0.3f);
                desenharCuboArame(1.0f);
            glPopMatrix();
        }

        SDL_GL_SwapWindow(janela);
        SDL_Delay(16);
    }

    SDL_GL_DeleteContext(contexto);
    SDL_DestroyWindow(janela);
    SDL_Quit();
    return 0;
}
