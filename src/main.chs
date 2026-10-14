module main

fn main() {
    var argv, argc = (null, 0)
    glutInit(&argc, argv);
    glutCreateWindow("Test".data);
    glutDisplayFunc(draw_func);
    glutMainLoop();
}

fn draw_func() {
    glClearColor(1.0, 0.5, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

library GL {
    link_name = "GL",
    kind = "dynlib"
}
library GLU {
    link_name = "GLU",
    kind = "dynlib"
}
library glut {
    link_name = "glut",
    kind = "dynlib"
}

type GLbitfield u32

const GL_COLOR_BUFFER_BIT = 0x00004000

fn glutInit(pargc: ^int, argv: ^^u8) #foreign GL
fn glutCreateWindow( title: ^u8 ) #foreign GL
fn glutMainLoop() #foreign GL
fn glutDisplayFunc(func: fn()) #foreign GL
fn glFlush() #foreign GL
fn glClearColor(r: f32, g: f32, b: f32, a: f32) #foreign GL
fn glClear(mask: GLbitfield) #foreign GL
