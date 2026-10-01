# snowtrix

Snow falling in your terminal.

`snowtrix` is a small interactive particle engine written in C99 with `termbox2`. It brings falling snow, wind, accumulation, and mouse interaction to a terminal window without trying to be a full game or a complex graphics project.

The idea is simple: open a terminal, let the snow fall, and play with the simulation when you feel like it.

## Features

* **Interactive Snow:** Snowflakes fall with different speeds, movement, and depth layers.
* **Wind System:** Change the direction and strength of the wind while the simulation is running.
* **Snow Accumulation:** Let snow build up on the ground and slowly spread across the surface.
* **Mouse Physics:** Move or click the mouse to push snowflakes around and dig into accumulated snow.
* **Parallax Layers:** Three particle layers create a simple sense of depth.
* **Spring Mode:** Melt accumulated snow dynamically while the simulation continues.
* **Party Mode:** Switch from normal snow colors to animated RGB colors.
* **Color Presets:** Change the snow theme instantly while the program is running.
* **Lightweight:** Written in C99 with POSIX support and designed to run directly in the terminal.

## Showcase

|          Dynamic Snowfall         |        Interactive Physics        |
| :-------------------------------: | :-------------------------------: |
| ![Snowtrix Demo 1](snowtrix1.gif) | ![Snowtrix Demo 2](snowtrix2.gif) |

## Requirements

* Linux / POSIX system
* GCC or Clang with C99 support
* GNU Make
* `ncursesw` and `libm`
* `man-db` for viewing the installed manual page

`termbox2` is included directly in the source tree, so there is no separate termbox2 installation step.

## Installation

Clone the repository and build the program:

```bash
git clone https://github.com/opendoto/snowtrix.git
cd snowtrix
make
```

Run it directly from the project directory:

```bash
./snowtrix
```

To install it system-wide:

```bash
sudo make install
```

To remove the installed binary:

```bash
sudo make uninstall
```

To remove local build files:

```bash
make clean
```

## Command Line Options

```text
Usage: snowtrix [OPTIONS]

OPTIONS:
  -a           Enable snow accumulation on startup
  -p           Start directly in Party Mode
  -c <RRGGBB>  Set foreground snow color
  -m <RRGGBB>  Set middle layer color
  -f <RRGGBB>  Set background layer color
  -C <theme>   Use a predefined color theme
  -s <speed>   Set initial falling speed
  -w <wind>    Set initial wind force
  -b <flakes>  Set the number of active snowflakes
  -h, --help   Show help message
```

## Runtime Controls

### General

| Key         | Action                                 |
| ----------- | -------------------------------------- |
| `q` / `ESC` | Quit                                   |
| `a`         | Toggle snow accumulation               |
| `s`         | Toggle Spring Mode (automatic melting) |
| `p`         | Toggle Party Mode                      |

### Environment

| Key       | Action                     |
| --------- | -------------------------- |
| `>` / `.` | Increase wind to the right |
| `<` / `,` | Increase wind to the left  |
| `w`       | Calm the wind              |
| `+` / `=` | Increase falling speed     |
| `-` / `_` | Decrease falling speed     |
| `m`       | Add 50 snowflakes          |
| `l`       | Remove 50 snowflakes       |

### Color Presets

| Key | Color  | Key | Color     |
| --- | ------ | --- | --------- |
| `!` | Red    | `%` | Magenta   |
| `@` | Green  | `^` | Cyan      |
| `#` | Yellow | `&` | White     |
| `$` | Blue   | `)` | Dark Gray |

## Limitations & Known Issues

Keep in mind that `snowtrix` is a terminal-based particle engine, so some behavior depends on the terminal emulator being used:

* **Terminal Resizing:** Resizing the terminal while the simulation is running can cause small visual artifacts during the redraw.
* **Mouse Support:** Mouse interaction requires a terminal emulator with mouse tracking support.
* **Unicode Rendering:** Snowflake shapes may look different depending on the terminal font and Unicode support.


## AI Disclaimer

Full disclosure: parts of this codebase were developed with the assistance of an AI tool. I designed the project concept, feature direction, physics behavior, and overall requirements, using AI as a pair programmer to help with implementation, debugging, and code cleanup.

## License

MIT License. See `LICENSE` for details.
