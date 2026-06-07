#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "fbsplash.h"
#include "svg_parser.h"
#include "svg_renderer.h"
#include "dt_rotation.h"

/*
 * SVG path data for rendering the VELOVE logo
 * Canvas: 1284 x 500 (same as original ROCKNIX)
 * Letters Y range: 140 ~ 360 (height 220, same as original)
 * Stroke width: ~27 (same as original)
 *
 * Index meaning:
 * 0: "V" in logo (red)
 * 1: "E" in logo (red)
 * 2: "L" in logo (white)
 * 3: "O" in logo (white)
 * 4: "V" in logo (white)
 * 5: "E" in logo (white)
 */
const char *svg_paths[] = {
    /* V (1st) */
    "M 112 140.106 L 192.5 359.628 L 219.638 359.628 L 300 140.106 L 272.86 140.106 L 206.069 325 L 139.14 140.106 L 112 140.106 Z",
    /* E (1st) */
    "M 330 140.106 L 330 359.628 L 470 359.628 L 470 337.809 L 357.14 337.809 L 357.14 260.106 L 455 260.106 L 455 238.287 L 357.14 238.287 L 357.14 161.925 L 470 161.925 L 470 140.106 L 330 140.106 Z",
    /* L */
    "M 500 140.106 L 500 359.628 L 640 359.628 L 640 337.809 L 527.14 337.809 L 527.14 140.106 L 500 140.106 Z",
    /* O */
    "M 643 250 C 643 313.329 692.232 363.886 753.698 363.886 C 815.164 363.886 864.388 313.329 864.388 250 C 864.388 186.671 815.164 136.116 753.698 136.116 C 692.232 136.116 643 186.671 643 250 Z M 672.27 250 C 672.27 200.375 707.525 162.325 753.698 162.325 C 799.863 162.325 835.118 200.375 835.118 250 C 835.118 299.625 799.863 337.676 753.698 337.676 C 707.525 337.676 672.27 299.625 672.27 250 Z",
    /* V (2nd) */
    "M 894 140.106 L 974.5 359.628 L 1001.638 359.628 L 1082 140.106 L 1054.86 140.106 L 988.069 325 L 921.14 140.106 L 894 140.106 Z",
    /* E (2nd) */
    "M 1112 140.106 L 1112 359.628 L 1252 359.628 L 1252 337.809 L 1139.14 337.809 L 1139.14 260.106 L 1237 260.106 L 1237 238.287 L 1139.14 238.287 L 1139.14 161.925 L 1252 161.925 L 1252 140.106 L 1112 140.106 Z"
};

/* Color definitions for each path component
 * First 2 paths (VE) are red (brand color)
 * Last 4 paths (LOVE) are white
 */
const char *svg_colors[] = {
    "rgb(255,50,50)",    // V - Red
    "rgb(255,50,50)",    // E - Red
    "rgb(255,255,255)",  // L - White
    "rgb(255,255,255)",  // O - White
    "rgb(255,255,255)",  // V - White
    "rgb(255,255,255)"   // E - White
};

#define NUM_PATHS (sizeof(svg_paths) / sizeof(svg_paths[0]))

int main(void) {
    const char *fb_device = "/dev/fb0";

    int rotation = get_display_rotation();

    if (access(fb_device, R_OK | W_OK) != 0) {
        fprintf(stderr, "Cannot access %s: %s\n", fb_device, strerror(errno));
        return 1;
    }

    Framebuffer *fb = fb_init(fb_device);
    if (!fb) {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        return 1;
    }

    DisplayInfo *display_info = calculate_display_info(fb);
    if (!display_info) {
        fprintf(stderr, "Failed to calculate display information\n");
        fb_cleanup(fb);
        return 1;
    }

    for (uint32_t y = 0; y < fb->vinfo.yres; y++) {
        for (uint32_t x = 0; x < fb->vinfo.xres; x++) {
            set_pixel(fb, x, y, 0x00000000);
        }
    }

    for (size_t i = 0; i < NUM_PATHS; i++) {
        SVGPath *svg = parse_svg_path(svg_paths[i], svg_colors[i]);
        if (!svg) {
            fprintf(stderr, "Failed to parse SVG path %zu\n", i);
            continue;
        }

        if (rotation)
            rotate_svg_path(svg, rotation);

        render_svg_path(fb, svg, display_info);
        free_svg_path(svg);
    }

    fb_flush(fb);

    free(display_info);
    fb_cleanup(fb);

    return 0;
}
