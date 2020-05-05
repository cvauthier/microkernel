#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Driver pour clavier PS/2 */

#include <stdint.h>
#include <kernel/utility.h>

typedef enum { Kb_Pressed, Kb_Released } kb_event_type;
typedef enum
{
	Kb_Escape, Kb_F1, Kb_F2, Kb_F3, Kb_F4, Kb_F5, Kb_F6, Kb_F7, Kb_F8, Kb_F9, Kb_F10, Kb_F11, Kb_F12,
	Kb_LCtrl, Kb_RCtrl, Kb_LAlt, Kb_RAlt, Kb_LShift, Kb_RShift,
	Kb_Tab, Kb_Backtick, Kb_Space, Kb_Backspace, Kb_Enter, Kb_CapsLock, Kb_NumLock, Kb_ScrollLock,
	Kb_a, Kb_b, Kb_c, Kb_d, Kb_e, Kb_f, Kb_g, Kb_h, Kb_i, Kb_j, Kb_k, Kb_l, Kb_m, 
	Kb_n, Kb_o, Kb_p, Kb_q, Kb_r, Kb_s, Kb_t, Kb_u, Kb_v, Kb_w, Kb_x, Kb_y, Kb_z,
	Kb_0, Kb_1, Kb_2, Kb_3, Kb_4, Kb_5, Kb_6, Kb_7, Kb_8, Kb_9,
	Kb_Period, Kb_Comma, Kb_Slash, Kb_Antislash, Kb_Scol, Kb_Minus, Kb_Quote, Kb_Equal,
	Kb_LBra, Kb_RBra,
	Kb_KP_Star, Kb_KP_Period, Kb_KP_Minus, Kb_KP_Plus,
	Kb_KP_0, Kb_KP_1, Kb_KP_2, Kb_KP_3, Kb_KP_4, Kb_KP_5, Kb_KP_6, Kb_KP_7, Kb_KP_8, Kb_KP_9, 
} kb_event_key;

struct kb_event_t
{
	kb_event_type type;
	kb_event_key  key;
};
typedef struct kb_event_t kb_event_t;

dynarray_t *kb_callbacks;
void (*kb_receive_scancode)(uint8_t);

void kb_step1(uint8_t code);

void init_keyboard();
void kb_event(kb_event_type type, kb_event_key key);

#endif
