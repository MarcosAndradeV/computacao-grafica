// dependencias:
//      Cargo: raylib (git = "https://github.com/MarcosAndradeV/raylib-rs")
//      Sistema: libX11-devel (Fedora) / libx11-dev (Debian/Ubuntu)
//      (raylib do sistema nao e necessario; o crate embute libraylib estatica)

use raylib::rlgl::*;
use raylib::*;

const LARGURA: i32 = 600;
const ALTURA: i32 = 600;

struct Projetil {
    x: f32,
    y: f32,
    dir_x: f32,
    dir_y: f32,
    ativo: bool,
}

struct Alvo {
    x: f32,
    y: f32,
    escala_base: f32,
    rot_angulo: f32,
    rot_vel: f32,
    ativo: bool,
}

fn desenhar_geometria_nave() {
    unsafe {
        rlBegin(RL_TRIANGLES as _);
        // Corpo principal
        rlColor3f(0.2, 0.8, 1.0);
        rlVertex3f(0.0, 0.8, 0.0);
        rlColor3f(0.1, 0.4, 0.8);
        rlVertex3f(-0.5, -0.6, 0.0);
        rlVertex3f(0.5, -0.6, 0.0);

        // Asa esquerda
        rlColor3f(0.9, 0.2, 0.2);
        rlVertex3f(-0.5, -0.6, 0.0);
        rlVertex3f(-0.8, -0.8, 0.0);
        rlVertex3f(-0.3, -0.2, 0.0);

        // Asa direita
        rlVertex3f(0.5, -0.6, 0.0);
        rlVertex3f(0.8, -0.8, 0.0);
        rlVertex3f(0.3, -0.2, 0.0);
        rlEnd();
    }
}

fn main() {
    init_window(LARGURA, ALTURA, "Transformacoes Geometricas 3D - Raylib");
    set_target_fps(60);

    let mut pos_x = 0.0;
    let mut pos_y = -3.5;
    let mut angulo_nave = 0.0;
    let escala_nave = 0.8;
    let mut tempo_animacao = 0.0;

    let mut tiros = vec![];
    let mut alvos = vec![
        Alvo {
            x: -3.0,
            y: 2.5,
            escala_base: 0.6,
            rot_angulo: 0.0,
            rot_vel: 1.5,
            ativo: true,
        },
        Alvo {
            x: -1.0,
            y: 3.0,
            escala_base: 0.7,
            rot_angulo: 0.0,
            rot_vel: -2.0,
            ativo: true,
        },
        Alvo {
            x: 1.0,
            y: 3.0,
            escala_base: 0.7,
            rot_angulo: 0.0,
            rot_vel: 2.5,
            ativo: true,
        },
        Alvo {
            x: 3.0,
            y: 2.5,
            escala_base: 0.6,
            rot_angulo: 0.0,
            rot_vel: -1.8,
            ativo: true,
        },
    ];

    while !window_should_close() {
        let dt = get_frame_time();
        tempo_animacao += dt * 2.0;

        // Entradas do Jogador
        if is_key_down(KEY_LEFT) {
            angulo_nave += 180.0 * dt;
        }
        if is_key_down(KEY_RIGHT) {
            angulo_nave -= 180.0 * dt;
        }
        if is_key_down(KEY_UP) {
            let rad = angulo_nave * (PI / 180.0);
            pos_x += -(rad).sin() * 4.0 * dt;
            pos_y += (rad).cos() * 4.0 * dt;
        }

        // Disparo
        if is_key_pressed(KEY_SPACE) {
            let rad = (angulo_nave + 90.0) * (PI / 180.0);
            tiros.push(Projetil {
                x: pos_x,
                y: pos_y,
                dir_x: (rad).cos(),
                dir_y: (rad).sin(),
                ativo: true,
            });
        }

        // Atualização dos Tiros e Colisões
        for tiro in &mut tiros {
            if !tiro.ativo {
                continue;
            }
            tiro.x += tiro.dir_x * 8.0 * dt;
            tiro.y += tiro.dir_y * 8.0 * dt;

            if tiro.x > 5.5 || tiro.x < -5.5 || tiro.y > 5.5 || tiro.y < -5.5 {
                tiro.ativo = false;
            }

            for alvo in &mut alvos {
                if !alvo.ativo {
                    continue;
                }
                let dx = tiro.x - alvo.x;
                let dy = tiro.y - alvo.y;
                if (dx * dx + dy * dy).sqrt() < (alvo.escala_base + 0.2) {
                    alvo.ativo = false;
                    tiro.ativo = false;
                    break;
                }
            }
        }

        // Rotação dos Alvos
        for alvo in &mut alvos {
            if alvo.ativo {
                alvo.rot_angulo += alvo.rot_vel * 60.0 * dt;
            }
        }

        // Renderização
        begin_drawing();
        clear_background(Color {
            r: 12,
            g: 12,
            b: 25,
            a: 255,
        });

        // Configuração da Projeção Ortográfica (-5 a 5)
        unsafe {
            rlMatrixMode(RL_PROJECTION as _);
            rlLoadIdentity();
            rlOrtho(-5.0, 5.0, -5.0, 5.0, -5.0, 5.0);

            rlMatrixMode(RL_MODELVIEW as _);
            rlLoadIdentity();

            // 1. Nave (T * R * S)
            rlPushMatrix();
            rlTranslatef(pos_x, pos_y, 0.0);
            rlRotatef(angulo_nave, 0.0, 0.0, 1.0);
            rlScalef(escala_nave, escala_nave, 1.0);
            desenhar_geometria_nave();
            rlPopMatrix();

            // 2. Tiros
            rlColor3f(1.0, 1.0, 0.0);
            for tiro in &tiros {
                if !tiro.ativo {
                    continue;
                }
                rlPushMatrix();
                rlTranslatef(tiro.x, tiro.y, 0.0);
                rlScalef(0.15, 0.3, 1.0);
                DrawCubeWires(Vector3::zero(), 1.0, 1.0, 1.0, YELLOW);
                rlPopMatrix();
            }

            // 3. Alvos (T * R * S com pulsação)
            for (i, alvo) in alvos.iter().enumerate() {
                if !alvo.ativo {
                    continue;
                }
                rlPushMatrix();
                rlTranslatef(alvo.x, alvo.y, 0.0);
                rlRotatef(alvo.rot_angulo, 0.0, 1.0, 0.0);
                rlRotatef(alvo.rot_angulo * 0.5, 0.0, 0.0, 1.0);

                let fator_pulso = 1.0 + 0.2 * (tempo_animacao + i as f32).sin();
                let esc = alvo.escala_base * fator_pulso;
                rlScalef(esc, esc, esc);

                DrawCubeWires(Vector3::zero(), 1.0, 1.0, 1.0, GREEN);
                rlPopMatrix();
            }
        }
        end_drawing();
    }

    close_window();
}
