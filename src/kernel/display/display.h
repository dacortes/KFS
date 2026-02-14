// SPDX-License-Identifier: GPL-2.0

/**
 * @file display.h
 * @brief Display management for the kernel
 *
 * This module provides functions to manage display-related tasks, such as
 * rendering text and graphics on the screen. It also defines the display
 * structure used to represent display properties and state.
 */
#pragma once

#ifndef DISPLAY_H
#define DISPLAY_H 25
#endif

#ifndef DISPLAY_W
#define DISPLAY_W 80
#endif

#ifndef CHAR_SIZE
#define CHAR_SIZE 2
#endif

#ifndef VIDEO_MEM_ADDR
#define VIDEO_MEM_ADDR 0xb8000
#endif

#ifndef WHITE_ON_BLACK
#define WHITE_ON_BLACK 0x07
#endif

/**
 * Display object - represents a display device
 *
 * Encapsulates the state and methods for managing a display device,
 * including video memory access and text rendering operations.
 */
struct display {
	char *videomemptr;
	unsigned int width;
	unsigned int height;
	unsigned int char_size;
	unsigned char color;
	void (*clear)(struct display *display);
	void (*write_string)(struct display *display, const char *string);
};

typedef struct display display_t;

/**
 * Initialize a display struct with default values and function pointers.
 *
 * @param display Pointer to the display struct to initialize
 *
 * @note Sets function pointers to the appropriate functions.
 */
void display_init(display_t *display);
