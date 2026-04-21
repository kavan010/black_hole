"""Python port of the 3D black hole visualizer."""

import ctypes
import math
import os
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path

if "WAYLAND_DISPLAY" in os.environ and "GLFW_PLATFORM" not in os.environ:
    os.environ["GLFW_PLATFORM"] = "x11"

import glfw
import numpy as np
from OpenGL.GL import *


C = 299792458.0
G = 6.67430e-11
GRID_SIZE = 25
GRID_SPACING = 1e10
MAX_OBJECTS = 16
CAMERA_UBO_SIZE = 80
DISK_UBO_SIZE = 16
OBJECTS_UBO_SIZE = 16 + MAX_OBJECTS * 16 * 2
GRAVITY_ENABLED = False


def normalize(vec):
    length = np.linalg.norm(vec)
    if length == 0.0:
        return vec
    return vec / length


def perspective(fov_y_radians, aspect, near, far):
    f = 1.0 / math.tan(fov_y_radians / 2.0)
    return np.array(
        [
            [f / aspect, 0.0, 0.0, 0.0],
            [0.0, f, 0.0, 0.0],
            [0.0, 0.0, (far + near) / (near - far), (2.0 * far * near) / (near - far)],
            [0.0, 0.0, -1.0, 0.0],
        ],
        dtype=np.float32,
    )


def look_at(eye, target, up):
    forward = normalize(target - eye)
    right = normalize(np.cross(forward, up))
    camera_up = np.cross(right, forward)

    return np.array(
        [
            [right[0], right[1], right[2], -np.dot(right, eye)],
            [camera_up[0], camera_up[1], camera_up[2], -np.dot(camera_up, eye)],
            [-forward[0], -forward[1], -forward[2], np.dot(forward, eye)],
            [0.0, 0.0, 0.0, 1.0],
        ],
        dtype=np.float32,
    )


@dataclass
class Camera:
    target: np.ndarray = field(default_factory=lambda: np.array([0.0, 0.0, 0.0], dtype=np.float32))
    radius: float = 6.34194e10
    min_radius: float = 1e10
    max_radius: float = 1e12
    azimuth: float = 0.0
    elevation: float = math.pi / 2.0
    orbit_speed: float = 0.01
    zoom_speed: float = 25e9
    dragging: bool = False
    panning: bool = False
    moving: bool = False
    last_x: float = 0.0
    last_y: float = 0.0

    def position(self):
        elevation = max(0.01, min(math.pi - 0.01, self.elevation))
        return np.array(
            [
                self.radius * math.sin(elevation) * math.cos(self.azimuth),
                self.radius * math.cos(elevation),
                self.radius * math.sin(elevation) * math.sin(self.azimuth),
            ],
            dtype=np.float32,
        )

    def update(self):
        self.target = np.array([0.0, 0.0, 0.0], dtype=np.float32)
        self.moving = self.dragging or self.panning

    def process_mouse_move(self, x, y):
        dx = float(x - self.last_x)
        dy = float(y - self.last_y)
        if self.dragging and not self.panning:
            self.azimuth += dx * self.orbit_speed
            self.elevation -= dy * self.orbit_speed
            self.elevation = max(0.01, min(math.pi - 0.01, self.elevation))
        self.last_x = x
        self.last_y = y
        self.update()

    def process_mouse_button(self, button, action, window):
        global GRAVITY_ENABLED
        if button in (glfw.MOUSE_BUTTON_LEFT, glfw.MOUSE_BUTTON_MIDDLE):
            if action == glfw.PRESS:
                self.dragging = True
                self.panning = False
                self.last_x, self.last_y = glfw.get_cursor_pos(window)
            elif action == glfw.RELEASE:
                self.dragging = False
                self.panning = False
        if button == glfw.MOUSE_BUTTON_RIGHT:
            if action == glfw.PRESS:
                GRAVITY_ENABLED = True
            elif action == glfw.RELEASE:
                GRAVITY_ENABLED = False
        self.update()

    def process_scroll(self, _xoffset, yoffset):
        self.radius -= yoffset * self.zoom_speed
        self.radius = max(self.min_radius, min(self.max_radius, self.radius))
        self.update()

    def process_key(self, key, action):
        global GRAVITY_ENABLED
        if action == glfw.PRESS and key == glfw.KEY_G:
            GRAVITY_ENABLED = not GRAVITY_ENABLED
            print(f"[INFO] Gravity turned {'ON' if GRAVITY_ENABLED else 'OFF'}")


@dataclass
class BlackHole:
    position: np.ndarray
    mass: float

    def __post_init__(self):
        self.r_s = 2.0 * G * self.mass / (C * C)


@dataclass
class ObjectData:
    pos_radius: np.ndarray
    color: np.ndarray
    mass: float
    velocity: np.ndarray = field(default_factory=lambda: np.zeros(3, dtype=np.float32))


class Engine:
    def __init__(self):
        if not glfw.init():
            raise RuntimeError("GLFW init failed")

        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 4)
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
        glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

        self.width = 800
        self.height = 600
        self.low_compute_width = 200
        self.low_compute_height = 150
        self.high_compute_width = 400
        self.high_compute_height = 300
        self.texture_width = 0
        self.texture_height = 0

        self.window = glfw.create_window(self.width, self.height, "Black Hole Python", None, None)
        if not self.window:
            glfw.terminate()
            raise RuntimeError("Failed to create GLFW window")

        glfw.make_context_current(self.window)
        glfw.swap_interval(1)

        version = glGetString(GL_VERSION)
        print("OpenGL", version.decode("utf-8"))

        root = Path(__file__).resolve().parent
        self.shader_program = self.create_screen_program()
        self.grid_program = self.create_program(root / "grid.vert", root / "grid.frag")
        self.compute_program = self.create_compute_program(root / "geodesic.comp")

        self.camera_ubo = glGenBuffers(1)
        glBindBuffer(GL_UNIFORM_BUFFER, self.camera_ubo)
        glBufferData(GL_UNIFORM_BUFFER, CAMERA_UBO_SIZE, None, GL_DYNAMIC_DRAW)
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, self.camera_ubo)

        self.disk_ubo = glGenBuffers(1)
        glBindBuffer(GL_UNIFORM_BUFFER, self.disk_ubo)
        glBufferData(GL_UNIFORM_BUFFER, DISK_UBO_SIZE, None, GL_DYNAMIC_DRAW)
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, self.disk_ubo)

        self.objects_ubo = glGenBuffers(1)
        glBindBuffer(GL_UNIFORM_BUFFER, self.objects_ubo)
        glBufferData(GL_UNIFORM_BUFFER, OBJECTS_UBO_SIZE, None, GL_DYNAMIC_DRAW)
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, self.objects_ubo)

        self.quad_vao, self.texture = self.create_fullscreen_quad()
        self.grid_vao = 0
        self.grid_vbo = 0
        self.grid_ebo = 0
        self.grid_index_count = 0

        glfw.set_framebuffer_size_callback(self.window, self.on_resize)

    def on_resize(self, _window, width, height):
        self.width = max(1, width)
        self.height = max(1, height)
        glViewport(0, 0, self.width, self.height)

    def read_text(self, path):
        return Path(path).read_text(encoding="utf-8")

    def compile_shader(self, shader_type, source):
        shader = glCreateShader(shader_type)
        glShaderSource(shader, source)
        glCompileShader(shader)
        if not glGetShaderiv(shader, GL_COMPILE_STATUS):
            raise RuntimeError(glGetShaderInfoLog(shader).decode("utf-8"))
        return shader

    def link_program(self, shaders):
        program = glCreateProgram()
        for shader in shaders:
            glAttachShader(program, shader)
        glLinkProgram(program)
        if not glGetProgramiv(program, GL_LINK_STATUS):
            raise RuntimeError(glGetProgramInfoLog(program).decode("utf-8"))
        for shader in shaders:
            glDeleteShader(shader)
        return program

    def create_program(self, vert_path, frag_path):
        vert = self.compile_shader(GL_VERTEX_SHADER, self.read_text(vert_path))
        frag = self.compile_shader(GL_FRAGMENT_SHADER, self.read_text(frag_path))
        return self.link_program([vert, frag])

    def create_screen_program(self):
        vert_source = """
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
        """
        frag_source = """
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoord);
        }
        """
        vert = self.compile_shader(GL_VERTEX_SHADER, vert_source)
        frag = self.compile_shader(GL_FRAGMENT_SHADER, frag_source)
        return self.link_program([vert, frag])

    def create_compute_program(self, comp_path):
        comp = self.compile_shader(GL_COMPUTE_SHADER, self.read_text(comp_path))
        return self.link_program([comp])

    def create_fullscreen_quad(self):
        quad_vertices = np.array(
            [
                -1.0, 1.0, 0.0, 1.0,
                -1.0, -1.0, 0.0, 0.0,
                1.0, -1.0, 1.0, 0.0,
                -1.0, 1.0, 0.0, 1.0,
                1.0, -1.0, 1.0, 0.0,
                1.0, 1.0, 1.0, 1.0,
            ],
            dtype=np.float32,
        )
        vao = glGenVertexArrays(1)
        vbo = glGenBuffers(1)
        glBindVertexArray(vao)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glBufferData(GL_ARRAY_BUFFER, quad_vertices.nbytes, quad_vertices, GL_STATIC_DRAW)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, ctypes.c_void_p(8))
        glEnableVertexAttribArray(1)

        texture = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, texture)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            self.low_compute_width,
            self.low_compute_height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            None,
        )
        self.texture_width = self.low_compute_width
        self.texture_height = self.low_compute_height
        return vao, texture

    def generate_grid(self, objects):
        vertices = []
        indices = []

        for z_index in range(GRID_SIZE + 1):
            for x_index in range(GRID_SIZE + 1):
                world_x = (x_index - GRID_SIZE / 2) * GRID_SPACING
                world_z = (z_index - GRID_SIZE / 2) * GRID_SPACING
                y = 0.0
                for obj in objects:
                    obj_pos = obj.pos_radius[:3]
                    r_s = 2.0 * G * obj.mass / (C * C)
                    dx = world_x - obj_pos[0]
                    dz = world_z - obj_pos[2]
                    dist = math.sqrt(dx * dx + dz * dz)
                    if dist > r_s:
                        y += 2.0 * math.sqrt(r_s * (dist - r_s)) - 3e10
                    else:
                        y += 2.0 * math.sqrt(r_s * r_s) - 3e10
                vertices.extend([world_x, y, world_z])

        for z_index in range(GRID_SIZE):
            for x_index in range(GRID_SIZE):
                base = z_index * (GRID_SIZE + 1) + x_index
                indices.extend([base, base + 1, base, base + GRID_SIZE + 1])

        vertices_np = np.array(vertices, dtype=np.float32)
        indices_np = np.array(indices, dtype=np.uint32)

        if self.grid_vao == 0:
            self.grid_vao = glGenVertexArrays(1)
        if self.grid_vbo == 0:
            self.grid_vbo = glGenBuffers(1)
        if self.grid_ebo == 0:
            self.grid_ebo = glGenBuffers(1)

        glBindVertexArray(self.grid_vao)
        glBindBuffer(GL_ARRAY_BUFFER, self.grid_vbo)
        glBufferData(GL_ARRAY_BUFFER, vertices_np.nbytes, vertices_np, GL_DYNAMIC_DRAW)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.grid_ebo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_np.nbytes, indices_np, GL_STATIC_DRAW)
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, ctypes.c_void_p(0))
        self.grid_index_count = indices_np.size
        glBindVertexArray(0)

    def draw_grid(self, view_proj):
        glUseProgram(self.grid_program)
        location = glGetUniformLocation(self.grid_program, "viewProj")
        glUniformMatrix4fv(location, 1, GL_TRUE, view_proj)
        glBindVertexArray(self.grid_vao)
        glDisable(GL_DEPTH_TEST)
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glDrawElements(GL_LINES, self.grid_index_count, GL_UNSIGNED_INT, None)
        glBindVertexArray(0)
        glEnable(GL_DEPTH_TEST)

    def upload_camera_ubo(self, camera):
        pos = camera.position()
        forward = normalize(camera.target - pos)
        up = np.array([0.0, 1.0, 0.0], dtype=np.float32)
        right = normalize(np.cross(forward, up))
        up = np.cross(right, forward)
        payload = struct.pack(
            "<16f2f2i",
            pos[0], pos[1], pos[2], 0.0,
            right[0], right[1], right[2], 0.0,
            up[0], up[1], up[2], 0.0,
            forward[0], forward[1], forward[2], 0.0,
            math.tan(math.radians(60.0 * 0.5)),
            float(self.width) / float(self.height),
            int(camera.moving),
            0,
        )
        glBindBuffer(GL_UNIFORM_BUFFER, self.camera_ubo)
        glBufferSubData(GL_UNIFORM_BUFFER, 0, len(payload), payload)

    def upload_disk_ubo(self, black_hole):
        payload = struct.pack(
            "<4f",
            black_hole.r_s * 2.2,
            black_hole.r_s * 5.2,
            2.0,
            1e9,
        )
        glBindBuffer(GL_UNIFORM_BUFFER, self.disk_ubo)
        glBufferSubData(GL_UNIFORM_BUFFER, 0, len(payload), payload)

    def upload_objects_ubo(self, objects):
        count = min(len(objects), MAX_OBJECTS)
        payload = bytearray(OBJECTS_UBO_SIZE)
        struct.pack_into("<i", payload, 0, count)

        pos_offset = 16
        color_offset = pos_offset + MAX_OBJECTS * 16

        for index, obj in enumerate(objects[:count]):
            struct.pack_into("<4f", payload, pos_offset + index * 16, *obj.pos_radius)
            struct.pack_into("<4f", payload, color_offset + index * 16, *obj.color)

        glBindBuffer(GL_UNIFORM_BUFFER, self.objects_ubo)
        glBufferSubData(GL_UNIFORM_BUFFER, 0, len(payload), payload)

    def dispatch_compute(self, camera, black_hole, objects):
        compute_width = self.low_compute_width if camera.moving else self.high_compute_width
        compute_height = self.low_compute_height if camera.moving else self.high_compute_height

        if compute_width != self.texture_width or compute_height != self.texture_height:
            glBindTexture(GL_TEXTURE_2D, self.texture)
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA8,
                compute_width,
                compute_height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                None,
            )
            self.texture_width = compute_width
            self.texture_height = compute_height

        glUseProgram(self.compute_program)
        self.upload_camera_ubo(camera)
        self.upload_disk_ubo(black_hole)
        self.upload_objects_ubo(objects)
        glBindImageTexture(0, self.texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8)
        groups_x = math.ceil(compute_width / 16.0)
        groups_y = math.ceil(compute_height / 16.0)
        glDispatchCompute(groups_x, groups_y, 1)
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)

    def draw_fullscreen_quad(self):
        glUseProgram(self.shader_program)
        glBindVertexArray(self.quad_vao)
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, self.texture)
        glUniform1i(glGetUniformLocation(self.shader_program, "screenTexture"), 0)
        glDisable(GL_DEPTH_TEST)
        glDrawArrays(GL_TRIANGLES, 0, 6)
        glEnable(GL_DEPTH_TEST)

    def shutdown(self):
        glfw.destroy_window(self.window)
        glfw.terminate()


class BlackHoleApp:
    def __init__(self):
        self.camera = Camera()
        self.black_hole = BlackHole(np.array([0.0, 0.0, 0.0], dtype=np.float32), 8.54e36)
        self.objects = self.build_scene_objects()
        self.engine = Engine()
        self.grid_dirty = True
        self.last_time = glfw.get_time()
        self.world_up = np.array([0.0, 1.0, 0.0], dtype=np.float32)
        self.register_callbacks()

    def build_scene_objects(self):
        return [
            ObjectData(np.array([4e11, 0.0, 0.0, 4e10], dtype=np.float32), np.array([1.0, 1.0, 0.0, 1.0], dtype=np.float32), 1.98892e30),
            ObjectData(np.array([0.0, 0.0, 4e11, 4e10], dtype=np.float32), np.array([1.0, 0.0, 0.0, 1.0], dtype=np.float32), 1.98892e30),
            ObjectData(np.array([0.0, 0.0, 0.0, self.black_hole.r_s], dtype=np.float32), np.array([0.0, 0.0, 0.0, 1.0], dtype=np.float32), self.black_hole.mass),
        ]

    def register_callbacks(self):
        glfw.set_mouse_button_callback(self.engine.window, self.mouse_button_callback)
        glfw.set_cursor_pos_callback(self.engine.window, self.cursor_pos_callback)
        glfw.set_scroll_callback(self.engine.window, self.scroll_callback)
        glfw.set_key_callback(self.engine.window, self.key_callback)

    def mouse_button_callback(self, window, button, action, _mods):
        self.camera.process_mouse_button(button, action, window)

    def cursor_pos_callback(self, _window, xpos, ypos):
        self.camera.process_mouse_move(xpos, ypos)

    def scroll_callback(self, _window, xoffset, yoffset):
        self.camera.process_scroll(xoffset, yoffset)

    def key_callback(self, window, key, _scancode, action, _mods):
        if key == glfw.KEY_ESCAPE and action == glfw.PRESS:
            glfw.set_window_should_close(window, True)
            return
        self.camera.process_key(key, action)

    def integrate_gravity(self, dt):
        if not GRAVITY_ENABLED:
            return

        self.grid_dirty = True
        for obj in self.objects:
            total_acc = np.zeros(3, dtype=np.float32)
            for other in self.objects:
                if obj is other:
                    continue
                delta = other.pos_radius[:3] - obj.pos_radius[:3]
                distance = np.linalg.norm(delta)
                if distance <= 0.0:
                    continue
                direction = delta / distance
                force = (G * obj.mass * other.mass) / (distance * distance)
                total_acc += direction * (force / obj.mass)
            obj.velocity += total_acc * dt

        for obj in self.objects:
            obj.pos_radius[:3] += obj.velocity * dt

    def render_frame(self):
        glClearColor(0.0, 0.0, 0.0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        now = glfw.get_time()
        dt = min(max(now - self.last_time, 0.0), 1.0 / 30.0)
        self.last_time = now

        self.integrate_gravity(dt)

        if self.grid_dirty:
            self.engine.generate_grid(self.objects)
            self.grid_dirty = False

        eye = self.camera.position()
        view = look_at(eye, self.camera.target, self.world_up)
        proj = perspective(math.radians(60.0), float(self.engine.width) / float(self.engine.height), 1e9, 1e14)
        view_proj = proj @ view

        glViewport(0, 0, self.engine.width, self.engine.height)
        self.engine.dispatch_compute(self.camera, self.black_hole, self.objects)
        self.engine.draw_fullscreen_quad()
        self.engine.draw_grid(view_proj)

        glfw.swap_buffers(self.engine.window)
        glfw.poll_events()

    def run(self):
        try:
            while not glfw.window_should_close(self.engine.window):
                self.render_frame()
        finally:
            self.engine.shutdown()


def main():
    BlackHoleApp().run()


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"Failed to start Python black hole app: {exc}", file=sys.stderr)
        sys.exit(1)
