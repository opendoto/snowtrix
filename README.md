# ❄️ snowtrix

A lightweight and highly interactive **Truecolor terminal particle engine** for Linux terminals, powered by **termbox2**.

Inspired by **cmatrix** and classic terminal screensavers, **snowtrix** focuses on creating a smooth and immersive snowfall simulation with pseudo-physics, real-time interaction, dynamic weather effects, and efficient rendering—all directly inside your terminal.

---

## ❄️ Features

- Real-time snow particle simulation
- Interactive mouse physics
- Dynamic wind system with smooth transitions
- Progressive snow accumulation
- Automatic snow melting (Spring Mode)
- Truecolor (24-bit RGB) rendering
- Three depth layers with parallax effect
- Soft particle collisions
- Dynamic splash effects
- Mouse snow excavation
- Runtime color themes
- Psychedelic Party Mode 🌈
- Lightweight C99 implementation
- POSIX compatible
- No external assets required

---

## 📸 Showcase

| Dynamic Snowfall | Interactive Physics |
|:----------------:|:-------------------:|
| ![Snowtrix Demo 1](snowtrix1.gif) | ![Snowtrix Demo 2](snowtrix2.gif) |

---

## 🧰 Requirements

- Linux
- POSIX-compatible system
- GCC or Clang with C99 support
- GNU Make
- libm
- ncursesw

Everything else is bundled with the project, including **termbox2**.

---

## 🚀 Installation

Clone the repository:

```bash
git clone https://github.com/opendoto/snowtrix.git
cd snowtrix
```

Compile:

```bash
make
```

Run:

```bash
./snowtrix
```

Install system-wide (optional):

```bash
sudo make install
```

Remove:

```bash
sudo make uninstall
```

Clean build files:

```bash
make clean
```

---

## ⚙️ Command Line Options

```
-a
    Enable snow accumulation.

-p
    Start directly in Party Mode.

-c <RRGGBB>
    Set the foreground snow color.

-m <RRGGBB>
    Override the middle layer color.

-f <RRGGBB>
    Override the background layer color.

-C <color>
    Use a predefined color theme.

-s <speed>
    Set the initial falling speed.

-w <wind>
    Set the initial wind force.

-b <flakes>
    Set the number of active snowflakes.

-h, --help
    Display the help message.
```

---

## ⌨️ Runtime Controls

### ❄️ General

| Key | Action |
|------|--------|
| `q` / `ESC` / `Ctrl+C` | Quit |
| `a` | Toggle snow accumulation |
| `s` | Toggle Spring Mode |
| `p` | Toggle Party Mode |

---

### 🌬️ Wind

| Key | Action |
|------|--------|
| `>` or `.` | Increase wind to the right |
| `<` or `,` | Increase wind to the left |
| `w` | Calm the wind |

---

### ❄️ Snow

| Key | Action |
|------|--------|
| `+` / `=` | Increase falling speed |
| `-` / `_` | Decrease falling speed |
| `m` | Add 50 snowflakes |
| `l` | Remove 50 snowflakes |

---

### 🎨 Color Presets

| Key | Color |
|------|-------|
| `!` | Red |
| `@` | Green |
| `#` | Yellow |
| `$` | Blue |
| `%` | Magenta |
| `^` | Cyan |
| `&` | White |
| `)` | Dark Gray |

---

## 🖱️ Mouse Interaction

Move or click the mouse to interact with the simulation.

Features include:

- Repel nearby snowflakes
- Dig accumulated snow
- Launch dynamic snow splash effects
- Disturb falling particles naturally

---

## 🧊 Physics Engine

The simulation includes several lightweight pseudo-physics systems designed specifically for terminal environments.

Current features include:

- Frame-independent physics
- Semi-implicit Euler integration
- Dynamic terminal velocity
- Three-layer parallax system
- Smooth wind interpolation
- Sinusoidal particle flutter
- Ground accumulation
- Snow redistribution
- Heightmap smoothing
- Soft particle bouncing
- Mouse interaction forces
- Dynamic splash simulation
- Automatic thaw cycle
- Horizontal wind shadows
- Toroidal horizontal wrapping

---

## 🎨 Rendering

Snowtrix renders entirely inside the terminal using Unicode characters and Truecolor.

Rendering features include:

- UTF-8 snowflakes
- Unicode half-block accumulation
- Full 24-bit RGB color
- Layered depth rendering
- Dynamic Party Mode colors
- Resolution-independent rendering
- Efficient redraw pipeline

---

## 📁 Project Structure

```
.
├── Makefile
├── README.md
├── LICENSE
├── .gitignore
├── snowtrix.c
├── termbox.h
├── snowtrix1.gif
└── snowtrix2.gif
```

---

## 🛠️ Building

Default build:

```bash
make
```

Compile manually:

```bash
gcc -O2 -Wall -Wextra -std=c99 snowtrix.c -o snowtrix -lncursesw -lm
```

---

## 🎯 Project Goals

Snowtrix aims to provide:

- Beautiful snowfall inside the terminal
- Smooth and responsive interaction
- Realistic pseudo-physics
- Extremely low CPU usage
- Portable POSIX code
- Minimal dependencies
- Clean and maintainable C source

Rather than being a game, snowtrix is designed as a decorative desktop companion and an experiment in high-quality terminal graphics.

---

## 📜 License

This project is released under the MIT License.

See the `LICENSE` file for details.

---

## ⭐ Support

If you enjoy the project, consider giving it a ⭐ on GitHub.

Contributions, bug reports, feature suggestions, and pull requests are always welcome.

Enjoy the snowfall! ❄️
