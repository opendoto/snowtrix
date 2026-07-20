/* 
 * snowtrix: A lightweight, interactive truecolor terminal particle engine.
 * Powered by termbox2. Controlled via keyboard & mouse physics.
 * Inspired by cmatrix and snowmachine by sontek.
 */

#define _DEFAULT_SOURCE
#define TB_IMPL
#define TB_OPT_TRUECOLOR

#include "termbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MIN_FLAKES 10
#define MAX_FLAKES 5000
#define SINE_LUT_SIZE 360

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float base_vy;  
    float mass;       
    int layer;        
    const char *shape;
    uint32_t color;   
    
    float phase;      
    float freq;       
    float amplitude;  
    int bounces;      
} Flake;

typedef struct {
    const char *name;
    char key;
    uint32_t color;
} ColorPreset;

// Engine state
static int running = 1;
static int max_x = 1;
static int max_y = 1;
static int nflakes = 500;
static float speedMult = 2.5f;    
static float target_wind = 0.0f;  
static float current_wind = 0.0f; 
static short accumulate = 0;   
static short party_mode = 0;
static short spring_mode = 0;
static short global_thaw = 0; 
static int weather_profile = 0; // 0=Custom, 1=Calm, 2=Blizzard, 3=Gentle

// Keep track of what the user passed via cli
static int user_set_speed = 0;
static int user_set_wind = 0;
static int user_set_flakes = 0;

static const char *custom_particle = NULL;
static char safe_custom_particle[8] = {0};

static int *ground = NULL;    
static int *base_ground = NULL; 
static Flake flakes[MAX_FLAKES] = {0};
static float sin_lut[SINE_LUT_SIZE]; 

static struct tb_event event = {0};
static struct timespec time_last;

static int mouse_btn = 0;
static int mouse_x = -1;
static int mouse_y = -1;

// Default theme
static uint32_t color_bg = 0x4C4C4C;
static uint32_t color_mid = 0x999999;
static uint32_t color_fg = 0xFFFFFF;
static uint32_t color_snow = 0xFFFFFF;

static const ColorPreset presets[] = {
    {"red", '!', 0xFF5555},
    {"green", '@', 0x55FF55},
    {"yellow", '#', 0xFFFF55},
    {"blue", '$', 0x5555FF},
    {"magenta", '%', 0xFF55FF},
    {"cyan", '^', 0x55FFFF},
    {"white", '&', 0xFFFFFF},
    {"black", ')', 0x777777} 
};
static const int num_presets = sizeof(presets) / sizeof(ColorPreset);

void init_params(void);
void reset_ground(int new_x, int new_y);
void update_physics(float dt);
void render_frame(uint32_t *layer_colors);
void event_handler(void);
void smooth_snow_physics(void);
void print_help(const char *prog_name);
uint32_t parse_hex_color(const char *hex_str, uint32_t default_color);
uint32_t dim_color(uint32_t color, float factor);
void apply_color_theme(uint32_t c);
void set_preset_color(char color_char);
void set_preset_color_name(const char *name);
void recolor_all_flakes(void);

// Grab a clean delta time to keep physics tied to time, not frame rate
float get_delta_time(void) {
    struct timespec time_now;
    clock_gettime(CLOCK_MONOTONIC, &time_now);
    
    float dt = (time_now.tv_sec - time_last.tv_sec) + 
               (time_now.tv_nsec - time_last.tv_nsec) * 1e-9f;
    time_last = time_now;
    
    // Clamp wildly high spikes (e.g. if the user suspends the process)
    if (dt < 0.0f || dt > 0.1f) return 0.016f; 
    return dt;
}

uint32_t parse_hex_color(const char *hex_str, uint32_t default_color) {
    if (!hex_str) return default_color;
    if (hex_str[0] == '#') hex_str++; 
    
    unsigned int color; 
    if (sscanf(hex_str, "%x", &color) == 1) {
        return (uint32_t)color;
    }
    return default_color;
}

uint32_t dim_color(uint32_t color, float factor) {
    uint8_t r = (uint8_t)(((color >> 16) & 0xFF) * factor);
    uint8_t g = (uint8_t)(((color >> 8) & 0xFF) * factor);
    uint8_t b = (uint8_t)((color & 0xFF) * factor);
    return (r << 16) | (g << 8) | b;
}

void recolor_all_flakes(void) {
    uint32_t pal[] = {0xFF5555, 0x55FF55, 0x5555FF, 0xFFFF55, 0xFF55FF, 0x55FFFF};
    uint32_t layer_colors[3] = {color_bg, color_mid, color_fg};
    
    for (int i = 0; i < nflakes; i++) {
        if (party_mode) {
            flakes[i].color = pal[random() % 6];
        } else {
            flakes[i].color = layer_colors[flakes[i].layer];
        }
    }
}

void apply_color_theme(uint32_t c) {
    color_fg = c;
    color_mid = dim_color(c, 0.6f);
    color_bg = dim_color(c, 0.3f);
    color_snow = c;
    party_mode = 0; 
}

void set_preset_color(char color_char) {
    for (int i = 0; i < num_presets; i++) {
        if (presets[i].key == color_char) {
            apply_color_theme(presets[i].color);
            recolor_all_flakes();
            return;
        }
    }
}

void set_preset_color_name(const char *name) {
    for (int i = 0; i < num_presets; i++) {
        if (strcmp(presets[i].name, name) == 0) {
            apply_color_theme(presets[i].color);
            return;
        }
    }
}

void print_help(const char *prog_name) {
    printf("snowtrix - A lightweight truecolor terminal particle engine.\n\n");
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("OPTIONS:\n");
    printf("  -a                 Enable snow accumulation on the ground.\n");
    printf("  -P <char>          Inject a custom snowflake character (e.g. -P \"*\").\n");
    printf("  -W <profile>       Load weather preset (Calm, Blizzard, Gentle).\n");
    printf("  -c <RRGGBB>        Set custom foreground snowflake color.\n");
    printf("  -m <RRGGBB>        Set custom mid-layer snowflake color.\n");
    printf("  -f <RRGGBB>        Set custom background snowflake color.\n");
    printf("  -C <color>         Use a predefined color (green, red, blue, white, black...).\n");
    printf("  -s <SPEED>         Set the initial falling speed (1.0 to 20.0, default 2.5).\n");
    printf("  -w <WIND>          Set the initial wind force (-5.0 left, 5.0 right).\n");
    printf("  -b <FLAKES>        Set the initial number of snowflakes (10 to 5000).\n");
    printf("  -p                 Start in TRUE RAINBOW MODE (Psychedelic particles).\n");
    printf("  -h, --help         Show this detailed help message.\n\n");
    printf("RUNTIME CONTROLS:\n");
    printf("  [Mouse Left Click] Dig snow / Repel falling snowflakes dynamically.\n");
    printf("  [q, ESC, Ctrl+C]   Quit the simulation.\n");
    printf("  [a]                Toggle snow accumulation.\n");
    printf("  [s]                Toggle spring mode (accelerated melting).\n");
    printf("  [p]                Toggle psychedelic party mode.\n");
    printf("  [w]                Reset wind force to zero.\n");
    printf("  [+, -]             Increase or decrease falling speed.\n");
    printf("  [<, >]             Change wind direction (left/right).\n");
    printf("  [m, l]             Increase or decrease amount of snowflakes.\n");
    printf("  [!, @, #, $, %%, ^, &, )] Change color themes instantly.\n\n");
}

int main(int argc, char *argv[]) {
    const char *arg_c = NULL, *arg_m = NULL, *arg_f = NULL, *arg_preset = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) accumulate = 1;
        else if (strcmp(argv[i], "-p") == 0) party_mode = 1;
        else if (strcmp(argv[i], "-P") == 0 && i + 1 < argc) {
            // Cap custom particles at 4 bytes (1 utf-8 char) to prevent visual chaos
            strncpy(safe_custom_particle, argv[++i], 4);
            safe_custom_particle[4] = '\0';
            custom_particle = safe_custom_particle;
        }
        else if (strcmp(argv[i], "-W") == 0 && i + 1 < argc) {
            const char *prof = argv[++i];
            if (strcmp(prof, "Calm") == 0 || strcmp(prof, "calm") == 0) weather_profile = 1;
            else if (strcmp(prof, "Blizzard") == 0 || strcmp(prof, "blizzard") == 0) weather_profile = 2;
            else if (strcmp(prof, "Gentle") == 0 || strcmp(prof, "gentle") == 0) weather_profile = 3;
        }
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) arg_c = argv[++i];
        else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) arg_m = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) arg_f = argv[++i];
        else if (strcmp(argv[i], "-C") == 0 && i + 1 < argc) arg_preset = argv[++i];
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            speedMult = atof(argv[++i]);
            user_set_speed = 1;
        }
        else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            target_wind = atof(argv[++i]);
            current_wind = target_wind; 
            user_set_wind = 1;
        }
        else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            nflakes = atoi(argv[++i]);
            if (nflakes > MAX_FLAKES) nflakes = MAX_FLAKES;
            if (nflakes < MIN_FLAKES) nflakes = MIN_FLAKES;
            user_set_flakes = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        }
    }

    if (arg_preset) set_preset_color_name(arg_preset);
    if (arg_c) apply_color_theme(parse_hex_color(arg_c, color_fg));
    if (arg_m) color_mid = parse_hex_color(arg_m, color_mid);
    if (arg_f) color_bg = parse_hex_color(arg_f, color_bg);

    srandom((unsigned)time(NULL));

    if (tb_init() != 0) {
        fprintf(stderr, "Fatal: Termbox2 initialization failed.\n");
        return 1;
    }
    
    tb_set_output_mode(TB_OUTPUT_TRUECOLOR);
    tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE); 
    tb_hide_cursor();

    init_params();
    clock_gettime(CLOCK_MONOTONIC, &time_last);

    float total_time = 0.0f;
    uint32_t frame_colors[3] = {color_bg, color_mid, color_fg};

    // Main loop
    while (running) {
        float dt = get_delta_time();
        total_time += dt;

        while (tb_peek_event(&event, 0) == TB_OK) {
            event_handler();
        }

        int current_w = tb_width();
        int current_h = tb_height();
        if (current_w != max_x || current_h != max_y) {
            reset_ground(current_w, current_h);
        }

        update_physics(dt);
        
        if (accumulate) {
            smooth_snow_physics();
            if (global_thaw || spring_mode) {
                smooth_snow_physics(); 
            }
        }

        if (party_mode) {
            for (int l = 0; l < 3; l++) {
                uint8_t r = (uint8_t)((sinf(total_time * 3.0f + l) * 127) + 128);
                uint8_t g = (uint8_t)((sinf(total_time * 2.5f + l + 2) * 127) + 128);
                uint8_t b = (uint8_t)((sinf(total_time * 4.0f + l + 4) * 127) + 128);
                frame_colors[l] = (r << 16) | (g << 8) | b;
            }
        } else {
            frame_colors[0] = color_bg;
            frame_colors[1] = color_mid;
            frame_colors[2] = color_fg;
        }

        tb_clear();
        render_frame(frame_colors); 
        tb_present();
        
        struct timespec req = {0, 16000000L}; 
        nanosleep(&req, NULL);
    }

    tb_shutdown();
    if (ground) free(ground);
    if (base_ground) free(base_ground);
    return 0;
}

void update_physics(float dt) {
    if (max_x <= 0 || max_y <= 0 || !ground || !base_ground) return;

    current_wind += (target_wind - current_wind) * 2.0f * dt;

    int ground_limit;
    
    // Simulate snow melting during spring
    if (spring_mode && accumulate) {
        int melt_rate = (max_x / 4) + 5; 
        for(int m = 0; m < melt_rate; m++) {
            int r_idx = random() % max_x;
            if (ground[r_idx] > base_ground[r_idx]) {
                ground[r_idx] -= (random() % 2) + 1; 
                if (ground[r_idx] < base_ground[r_idx]) ground[r_idx] = base_ground[r_idx];
            }
        }
        target_wind *= 0.99f; 
    }

    if (accumulate && !spring_mode) {
        if (!global_thaw) {
            int threshold = (int)(max_y * 1.7f); 
            for (int k = 0; k < max_x; k++) {
                if (ground[k] > threshold) {
                    global_thaw = 1; break;
                }
            }
        } else {
            int still_high = 0;
            int low_threshold = (int)(max_y * 0.25f); 
            for (int k = 0; k < max_x; k++) {
                if (ground[k] > base_ground[k]) {
                    if ((random() % 100) < 60) {
                        ground[k] -= (random() % 3) + 1; 
                        if (ground[k] < base_ground[k]) ground[k] = base_ground[k];
                    }
                }
                if (ground[k] > low_threshold) still_high = 1;
            }
            if (!still_high) global_thaw = 0;
        }
    }

    int safe_y_bound = (max_y / 2) > 0 ? (max_y / 2) : 1; 
    int safe_x_bound = (max_x / 2) > 0 ? (max_x / 2) : 1;

    for (int i = 0; i < nflakes; i++) {
        if (isnan(flakes[i].x) || isnan(flakes[i].y) || isnan(flakes[i].vx) || isnan(flakes[i].vy)) {
            flakes[i].x = (float)((random() % safe_x_bound) * 2);
            flakes[i].y = -(float)(random() % safe_y_bound) - 2.0f; 
            flakes[i].vx = 0.0f; flakes[i].vy = flakes[i].base_vy;
            flakes[i].bounces = 0;
            continue;
        }

        // Push flakes away from the mouse if left click is held down
        if (mouse_btn == 1 && mouse_x != -1 && mouse_y != -1) {
            float dx = flakes[i].x - mouse_x;
            float dy = flakes[i].y - mouse_y;
            float dist_sq = (dx * dx) + (dy * dy);
            
            if (dist_sq < 225.0f && dist_sq > 0.01f) { 
                float dist = sqrtf(dist_sq) + 0.001f;
                float inv_dist = 1.0f / dist; 
                float force = (15.0f - dist) * 0.15f; 
                
                float jitter = ((random() % 100) - 50) * 0.01f;
                if (fabsf(dx) < 0.5f) dx += jitter; 

                flakes[i].vx += (dx * inv_dist) * force;
                flakes[i].vy += (dy * inv_dist) * force - 0.2f; 
            }
        }

        flakes[i].vy += (flakes[i].base_vy - flakes[i].vy) * 0.05f;
        flakes[i].vx += ((random() % 101) - 50) * 0.0005f; 
        flakes[i].vx *= 0.92f; 
        
        flakes[i].phase += flakes[i].freq * dt * 90.0f; 
        if (flakes[i].phase >= 360.0f) flakes[i].phase -= 360.0f;
        
        int lut_idx = (int)flakes[i].phase;
        if (lut_idx < 0) lut_idx = 0;
        if (lut_idx >= SINE_LUT_SIZE) lut_idx = SINE_LUT_SIZE - 1;
        
        float wobble = sin_lut[lut_idx] * flakes[i].amplitude;
        
        float local_wind = current_wind;
        int ix_wind = (int)(flakes[i].x + 0.5f);
        if (ix_wind < 0) ix_wind = 0;
        if (ix_wind >= max_x) ix_wind = max_x - 1;

        if (accumulate && ground) {
            if (current_wind > 0.5f && ix_wind > 0) { 
                float peak_y = max_y - (ground[ix_wind - 1] * 0.5f);
                if (flakes[i].y > peak_y) local_wind *= 0.2f; 
            } else if (current_wind < -0.5f && ix_wind < max_x - 1) { 
                float peak_y = max_y - (ground[ix_wind + 1] * 0.5f);
                if (flakes[i].y > peak_y) local_wind *= 0.2f; 
            }
        }

        float wind_effect = (local_wind * 3.0f) / flakes[i].mass;
        
        flakes[i].x += (flakes[i].vx + wind_effect + wobble) * dt * speedMult;
        flakes[i].y += flakes[i].vy * flakes[i].mass * dt * speedMult;

        flakes[i].x = fmodf(flakes[i].x, (float)max_x);
        if (flakes[i].x < 0.0f) flakes[i].x += max_x;

        int ix = (int)(flakes[i].x + 0.5f);
        if (ix < 0) ix = 0;
        if (ix >= max_x) ix = max_x - 1;

        ground_limit = (max_y * 2) - (accumulate ? ground[ix] : base_ground[ix]);
        int half_y = (int)(flakes[i].y * 2.0f);

        // Fix: make sure flakes are actually falling down before registering a collision. 
        // Otherwise, particles we launch upwards with the shovel would get trapped invisibly.
        if ((half_y >= ground_limit && flakes[i].vy > 0.0f) || flakes[i].y > max_y + 5.0f) {
            if (flakes[i].bounces == 0 && flakes[i].vy > 2.0f && half_y < max_y * 2) {
                flakes[i].vy = -flakes[i].vy * 0.35f; 
                flakes[i].vx += local_wind * 0.2f;    
                flakes[i].bounces++;
            } else {
                if (accumulate && ground[ix] < max_y * 2 && !spring_mode && half_y >= ground_limit) {
                    ground[ix]++;
                }
                
                // Recycle the particle at the top
                flakes[i].y = -(float)(random() % safe_y_bound) - 2.0f; 
                flakes[i].x = (float)((random() % safe_x_bound) * 2);
                flakes[i].vx = 0.0f;
                flakes[i].vy = flakes[i].base_vy;
                flakes[i].bounces = 0;
                
                if (party_mode) {
                    uint32_t pal[] = {0xFF5555, 0x55FF55, 0x5555FF, 0xFFFF55, 0xFF55FF, 0x55FFFF};
                    flakes[i].color = pal[random() % 6];
                } else {
                    uint32_t layer_colors[3] = {color_bg, color_mid, color_fg};
                    flakes[i].color = layer_colors[flakes[i].layer];
                }
            }
        }
    }

    // Shovel out snow using the mouse
    if (mouse_btn == 1 && accumulate && ground && mouse_x != -1 && mouse_y != -1) {
        int radius = 4; 
        for (int x = mouse_x - radius; x <= mouse_x + radius; x++) {
            if (x >= 0 && x < max_x && ground[x] > base_ground[x]) {
                float snow_y = max_y - (ground[x] * 0.5f);
                float dx = x - mouse_x;
                float dy = snow_y - mouse_y;
                float dist_sq_g = (dx * dx) + (dy * dy);
                
                if (dist_sq_g < 36.0f) { 
                    float dist = sqrtf(dist_sq_g) + 0.001f;
                    
                    int dig_amount = 1; 
                    if (dist < 2.0f) dig_amount = 2; 
                    
                    if (ground[x] - base_ground[x] >= dig_amount) {
                        ground[x] -= dig_amount;
                    } else {
                        dig_amount = ground[x] - base_ground[x];
                        ground[x] = base_ground[x];
                    }
                    
                    for (int d = 0; d < dig_amount; d++) {
                        int fi = random() % nflakes;
                        flakes[fi].x = (float)x + ((random() % 100) - 50) * 0.02f;
                        flakes[fi].y = snow_y - 1.0f;
                        
                        float dir_x = (dx > 0) ? 1.0f : (dx < 0 ? -1.0f : (((random()%2)*2)-1.0f));
                        float spread_force = 1.0f + ((random() % 100) / 50.0f);
                        
                        flakes[fi].vx = dir_x * spread_force + current_wind * 0.3f;
                        flakes[fi].vy = -1.2f - ((random() % 100) / 100.0f); 
                        flakes[fi].bounces = 1; 
                    }
                }
            }
        }
    }
}

void smooth_snow_physics() {
    if (max_x <= 1 || !ground || !base_ground) return;
    
    static int sweep_dir = 1;
    sweep_dir = -sweep_dir;
    
    int start = (sweep_dir > 0) ? 0 : max_x - 1;
    int end = (sweep_dir > 0) ? max_x : -1;
    
    float wind_bias = current_wind * 0.4f; 
    int thresh_left = (int)(2.0f + wind_bias);
    int thresh_right = (int)(2.0f - wind_bias);
    
    if (thresh_left < 1) thresh_left = 1;
    if (thresh_right < 1) thresh_right = 1;

    for (int i = start; i != end; i += sweep_dir) {
        if (ground[i] > base_ground[i]) {
            if (i > 0 && ground[i] > ground[i-1] + thresh_left) {
                if (ground[i] - 1 >= base_ground[i]) {
                    ground[i]--; ground[i-1]++;
                }
            }
            else if (i < max_x - 1 && ground[i] > ground[i+1] + thresh_right) {
                if (ground[i] - 1 >= base_ground[i]) {
                    ground[i]--; ground[i+1]++;
                }
            }
        }
    }
}

void render_frame(uint32_t *layer_colors) { 
    if (max_x <= 0 || max_y <= 0 || !ground || !base_ground) return;

    if (accumulate) {
        uint32_t current_snow_color = party_mode ? layer_colors[2] : color_snow;
        for (int x = 0; x < max_x; x++) {
            int h = ground[x];
            
            if (h > base_ground[x]) {
                int empty_half_blocks = (max_y * 2) - h;
                
                for (int y = 0; y < max_y; y++) {
                    int top_hb = y * 2;
                    int bot_hb = y * 2 + 1;
                    
                    if (top_hb >= empty_half_blocks) {
                        tb_printf(x, y, current_snow_color, TB_DEFAULT, "█");
                    } else if (bot_hb >= empty_half_blocks) {
                        tb_printf(x, y, current_snow_color, TB_DEFAULT, "▄");
                    }
                }
            }
        }
    }

    // Draw flakes back-to-front for fake depth
    for (int layer = 0; layer <= 2; layer++) {
        for (int i = 0; i < nflakes; i++) {
            if (flakes[i].layer != layer) continue;

            int ix = (int)(flakes[i].x + 0.5f);
            int iy = (int)(flakes[i].y + 0.5f);
            
            if (ix >= 0 && ix < max_x && iy >= 0 && iy < max_y) {
                int limit = accumulate ? (max_y * 2) - ground[ix] : (max_y * 2) - base_ground[ix];
                if ((iy * 2) < limit) {
                    tb_printf(ix, iy, flakes[i].color, TB_DEFAULT, "%s", flakes[i].shape);
                }
            }
        }
    }
}

void reset_ground(int new_x, int new_y) {
    if (new_x < 1) new_x = 1;
    if (new_y < 1) new_y = 1;
    
    if (!ground || !base_ground || new_x != max_x || new_y != max_y) {
        int *new_ground = calloc(new_x, sizeof(int));
        int *new_base = calloc(new_x, sizeof(int));
        
        // Bail out cleanly if we only managed to allocate one of the buffers
        if (!new_ground || !new_base) {
            if (new_ground) free(new_ground);
            if (new_base) free(new_base);
            return; 
        }

        if (ground && base_ground) {
            int min_w = (max_x < new_x) ? max_x : new_x;
            for (int i = 0; i < min_w; i++) {
                int snow_h = ground[i] - base_ground[i];
                if (snow_h < 0) snow_h = 0;
                new_ground[i] = new_base[i] + snow_h;
            }
            free(ground);
            free(base_ground);
        } else {
            for (int i = 0; i < new_x; i++) {
                new_ground[i] = new_base[i];
            }
        }
        
        ground = new_ground;
        base_ground = new_base;
        
        float scale_x = (max_x > 0) ? ((float)new_x / (float)max_x) : 1.0f;
        max_x = new_x;
        
        for (int i = 0; i < MAX_FLAKES; i++) {
            flakes[i].x *= scale_x;
            if (flakes[i].x >= max_x || flakes[i].x < 0 || isnan(flakes[i].x)) {
                flakes[i].x = (float)((random() % (max_x / 2 > 0 ? max_x / 2 : 1)) * 2);
            } else {
                flakes[i].x += ((random() % 100) - 50) * 0.01f;
            }
        }
    }
    max_y = new_y; 
}

void init_params() {
    for(int i = 0; i < SINE_LUT_SIZE; i++) {
        sin_lut[i] = sinf((float)i * (M_PI / 180.0f));
    }

    // Only apply preset defaults if the user didn't override them via flags
    if (weather_profile == 1) { 
        if (!user_set_wind) target_wind = 0.0f;
        if (!user_set_speed) speedMult = 1.0f;
        if (!user_set_flakes) nflakes = 200;
    } else if (weather_profile == 2) { 
        if (!user_set_wind) target_wind = 4.5f;
        if (!user_set_speed) speedMult = 5.0f;
        if (!user_set_flakes) nflakes = 4000;
    } else if (weather_profile == 3) { 
        if (!user_set_wind) target_wind = 0.5f;
        if (!user_set_speed) speedMult = 1.5f;
        if (!user_set_flakes) nflakes = 500;
    }
    current_wind = target_wind;

    reset_ground(tb_width(), tb_height());
    
    const char* fg_shapes[] = {"❆", "❅", "❄", "✱", "❇"};
    const char* mid_shapes[] = {"*", "₊", "°", "✶", "·"};
    const char* bg_shapes[] = {".", "·", "`", ","};

    int safe_x_bound = (max_x / 2 > 0) ? (max_x / 2) : 1;

    for (int i = 0; i < MAX_FLAKES; i++) {
        flakes[i].x = (float)((random() % safe_x_bound) * 2);
        flakes[i].y = -(float)(random() % max_y) - 2.0f; 
        flakes[i].vx = 0.0f;
        flakes[i].vy = 0.0f;
        flakes[i].bounces = 0;
        
        flakes[i].phase = (float)(random() % 360); 
        flakes[i].freq  = 0.2f + ((random() % 100) * 0.005f);        
        
        int r = random() % 100;
        
        if (r < 15) { 
            flakes[i].shape = custom_particle ? custom_particle : fg_shapes[random() % 5];
            flakes[i].mass = 1.8f + ((random() % 50) * 0.01f);
            flakes[i].vy = 1.0f; flakes[i].base_vy = 1.0f;
            flakes[i].layer = 2;
            flakes[i].amplitude = 0.6f;   
        } 
        else if (r < 50) { 
            flakes[i].shape = custom_particle ? custom_particle : mid_shapes[random() % 5];
            flakes[i].mass = 0.9f + ((random() % 30) * 0.01f);
            flakes[i].vy = 0.7f; flakes[i].base_vy = 0.7f;
            flakes[i].layer = 1;
            flakes[i].amplitude = 0.3f;   
        } 
        else { 
            flakes[i].shape = custom_particle ? custom_particle : bg_shapes[random() % 4];
            flakes[i].mass = 0.4f + ((random() % 20) * 0.01f);
            flakes[i].vy = 0.4f; flakes[i].base_vy = 0.4f;
            flakes[i].layer = 0;
            flakes[i].amplitude = 0.15f;   
        }

        if (weather_profile == 1) { 
            flakes[i].mass *= 0.5f;
        } else if (weather_profile == 2) { 
            flakes[i].mass *= 1.5f;
            flakes[i].amplitude *= 0.5f;
        } else if (weather_profile == 3) { 
            flakes[i].amplitude *= 2.0f;
        }
    }

    recolor_all_flakes();
}

void event_handler() {
    if (event.type == TB_EVENT_MOUSE) {
        mouse_x = event.x;
        mouse_y = event.y;

        if (event.key == TB_KEY_MOUSE_LEFT) {
            mouse_btn = 1;
        } else if (event.key == TB_KEY_MOUSE_RELEASE) {
            mouse_btn = 0;
        }
    } else if (event.type == TB_EVENT_KEY) {
        mouse_btn = 0; 
        if (event.key == TB_KEY_CTRL_C || event.key == TB_KEY_ESC || event.ch == 'q' || event.ch == 'Q') {
            running = 0;
            return;
        }

        if (event.ch && event.ch < 128 && strchr("!@#$%^&)", (char)event.ch)) {
            set_preset_color((char)event.ch);
            return;
        }

        switch (event.ch) {
            case '-': case '_': if (speedMult > 1.0f) speedMult -= 0.5f; break;
            case '+': case '=': if (speedMult < 20.0f) speedMult += 0.5f; break;
            case 'm': case 'M': if (nflakes + 50 <= MAX_FLAKES) nflakes += 50; break;
            case 'l': case 'L': if (nflakes - 50 >= MIN_FLAKES) nflakes -= 50; break;
            case '>': case '.': target_wind += 1.5f; break;
            case '<': case ',': target_wind -= 1.5f; break;
            case 'w': case 'W': target_wind = 0.0f; break;
            case 'p': case 'P': 
                party_mode = !party_mode; 
                recolor_all_flakes();
                break;
            case 's': case 'S': spring_mode = !spring_mode; break;
            case 'a': case 'A': 
                accumulate = !accumulate;
                if (!accumulate && ground && base_ground) {
                    for (int i = 0; i < max_x; i++) ground[i] = base_ground[i];
                }
                break;
        }
    }
}
