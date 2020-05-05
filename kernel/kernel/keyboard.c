#include <kernel/keyboard.h>

void kb_step1(uint8_t code)
{
	switch (code)
	{
		case 0x01:
			kb_event(Kb_Pressed, Kb_Escape);
			break;
		case 0x02:
			kb_event(Kb_Pressed, Kb_1);
			break;
		case 0x03:
			kb_event(Kb_Pressed, Kb_2);
			break;
		case 0x04:
			kb_event(Kb_Pressed, Kb_3);
			break;
		case 0x05:
			kb_event(Kb_Pressed, Kb_4);
			break;
		case 0x06:
			kb_event(Kb_Pressed, Kb_5);
			break;
		case 0x07:
			kb_event(Kb_Pressed, Kb_6);
			break;
		case 0x08:
			kb_event(Kb_Pressed, Kb_7);
			break;
		case 0x09:
			kb_event(Kb_Pressed, Kb_8);
			break;
		case 0x0A:
			kb_event(Kb_Pressed, Kb_9);
			break;
		case 0x0B:
			kb_event(Kb_Pressed, Kb_0);
			break;
		case 0x0C:
			kb_event(Kb_Pressed, Kb_Minus);
			break;
		case 0x0D:
			kb_event(Kb_Pressed, Kb_Equal);
			break;
		case 0x0E:
			kb_event(Kb_Pressed, Kb_Backspace);
			break;
		case 0x0F:
			kb_event(Kb_Pressed, Kb_Tab);
			break;
		case 0x10:
			kb_event(Kb_Pressed, Kb_q);
			break;
		case 0x11:
			kb_event(Kb_Pressed, Kb_w);
			break;
		case 0x12:
			kb_event(Kb_Pressed, Kb_e);
			break;
		case 0x13:
			kb_event(Kb_Pressed, Kb_r);
			break;
		case 0x14:
			kb_event(Kb_Pressed, Kb_t);
			break;
		case 0x15:
			kb_event(Kb_Pressed, Kb_y);
			break;
		case 0x16:
			kb_event(Kb_Pressed, Kb_u);
			break;
		case 0x17:
			kb_event(Kb_Pressed, Kb_i);
			break;
		case 0x18:
			kb_event(Kb_Pressed, Kb_o);
			break;
		case 0x19:
			kb_event(Kb_Pressed, Kb_p);
			break;
		case 0x1A:
			kb_event(Kb_Pressed, Kb_LBra);
			break;
		case 0x1B:
			kb_event(Kb_Pressed, Kb_RBra);
			break;
		case 0x1C:
			kb_event(Kb_Pressed, Kb_Enter);
			break;
		case 0x1D:
			kb_event(Kb_Pressed, Kb_LCtrl);
			break;
		case 0x1E:
			kb_event(Kb_Pressed, Kb_a);
			break;
		case 0x1F:
			kb_event(Kb_Pressed, Kb_s);
			break;
		case 0x20:
			kb_event(Kb_Pressed, Kb_d);
			break;
		case 0x21:
			kb_event(Kb_Pressed, Kb_f);
			break;
		case 0x22:
			kb_event(Kb_Pressed, Kb_g);
			break;
		case 0x23:
			kb_event(Kb_Pressed, Kb_h);
			break;
		case 0x24:
			kb_event(Kb_Pressed, Kb_j);
			break;
		case 0x25:
			kb_event(Kb_Pressed, Kb_k);
			break;
		case 0x26:
			kb_event(Kb_Pressed, Kb_l);
			break;
		case 0x27:
			kb_event(Kb_Pressed, Kb_Scol);
			break;
		case 0x28:
			kb_event(Kb_Pressed, Kb_Quote);
			break;
		case 0x29:
			kb_event(Kb_Pressed, Kb_Backtick);
			break;
		case 0x2A:
			kb_event(Kb_Pressed, Kb_LShift);
			break;
		case 0x2B:
			kb_event(Kb_Pressed, Kb_Antislash);
			break;
		case 0x2C:
			kb_event(Kb_Pressed, Kb_z);
			break;
		case 0x2D:
			kb_event(Kb_Pressed, Kb_x);
			break;
		case 0x2E:
			kb_event(Kb_Pressed, Kb_c);
			break;
		case 0x2F:
			kb_event(Kb_Pressed, Kb_v);
			break;
		case 0x30:
			kb_event(Kb_Pressed, Kb_b);
			break;
		case 0x31:
			kb_event(Kb_Pressed, Kb_n);
			break;
		case 0x32:
			kb_event(Kb_Pressed, Kb_m);
			break;
		case 0x33:
			kb_event(Kb_Pressed, Kb_Comma);
			break;
		case 0x34:
			kb_event(Kb_Pressed, Kb_Period);
			break;
		case 0x35:
			kb_event(Kb_Pressed, Kb_Slash);
			break;
		case 0x36:
			kb_event(Kb_Pressed, Kb_RShift);
			break;
		case 0x37:
			kb_event(Kb_Pressed, Kb_KP_Star);
			break;
		case 0x38:
			kb_event(Kb_Pressed, Kb_LAlt);
			break;
		case 0x39:
			kb_event(Kb_Pressed, Kb_Space);
			break;
		case 0x3A:
			kb_event(Kb_Pressed, Kb_CapsLock);
			break;
		case 0x3B:
			kb_event(Kb_Pressed, Kb_F1);
			break;
		case 0x3C:
			kb_event(Kb_Pressed, Kb_F2);
			break;
		case 0x3D:
			kb_event(Kb_Pressed, Kb_F3);
			break;
		case 0x3E:
			kb_event(Kb_Pressed, Kb_F4);
			break;
		case 0x3F:
			kb_event(Kb_Pressed, Kb_F5);
			break;
		case 0x40:
			kb_event(Kb_Pressed, Kb_F6);
			break;
		case 0x41:
			kb_event(Kb_Pressed, Kb_F7);
			break;
		case 0x42:
			kb_event(Kb_Pressed, Kb_F8);
			break;
		case 0x43:
			kb_event(Kb_Pressed, Kb_F9);
			break;
		case 0x44:
			kb_event(Kb_Pressed, Kb_F10);
			break;
		case 0x45:
			kb_event(Kb_Pressed, Kb_NumLock);
			break;
		case 0x46:
			kb_event(Kb_Pressed, Kb_ScrollLock);
			break;
		case 0x47:
			kb_event(Kb_Pressed, Kb_KP_7);
			break;
		case 0x48:
			kb_event(Kb_Pressed, Kb_KP_8);
			break;
		case 0x49:
			kb_event(Kb_Pressed, Kb_KP_9);
			break;
		case 0x4A:
			kb_event(Kb_Pressed, Kb_KP_Minus);
			break;
		case 0x4B:
			kb_event(Kb_Pressed, Kb_KP_4);
			break;
		case 0x4C:
			kb_event(Kb_Pressed, Kb_KP_5);
			break;
		case 0x4D:
			kb_event(Kb_Pressed, Kb_KP_6);
			break;
		case 0x4E:
			kb_event(Kb_Pressed, Kb_KP_Plus);
			break;
		case 0x4F:
			kb_event(Kb_Pressed, Kb_KP_1);
			break;
		case 0x50:
			kb_event(Kb_Pressed, Kb_KP_2);
			break;
		case 0x51:
			kb_event(Kb_Pressed, Kb_KP_3);
			break;
		case 0x52:
			kb_event(Kb_Pressed, Kb_KP_0);
			break;
		case 0x53:
			kb_event(Kb_Pressed, Kb_KP_Period);
			break;
		case 0x57:
			kb_event(Kb_Pressed, Kb_F11);
			break;
		case 0x58:
			kb_event(Kb_Pressed, Kb_F12);
			break;
		case 0x81:
			kb_event(Kb_Released, Kb_Escape);
			break;
		case 0x82:
			kb_event(Kb_Released, Kb_1);
			break;
		case 0x83:
			kb_event(Kb_Released, Kb_2);
			break;
		case 0x84:
			kb_event(Kb_Released, Kb_3);
			break;
		case 0x85:
			kb_event(Kb_Released, Kb_4);
			break;
		case 0x86:
			kb_event(Kb_Released, Kb_5);
			break;
		case 0x87:
			kb_event(Kb_Released, Kb_6);
			break;
		case 0x88:
			kb_event(Kb_Released, Kb_7);
			break;
		case 0x89:
			kb_event(Kb_Released, Kb_8);
			break;
		case 0x8A:
			kb_event(Kb_Released, Kb_9);
			break;
		case 0x8B:
			kb_event(Kb_Released, Kb_0);
			break;
		case 0x8C:
			kb_event(Kb_Released, Kb_Minus);
			break;
		case 0x8D:
			kb_event(Kb_Released, Kb_Equal);
			break;
		case 0x8E:
			kb_event(Kb_Released, Kb_Backspace);
			break;
		case 0x8F:
			kb_event(Kb_Released, Kb_Tab);
			break;
		case 0x90:
			kb_event(Kb_Released, Kb_q);
			break;
		case 0x91:
			kb_event(Kb_Released, Kb_w);
			break;
		case 0x92:
			kb_event(Kb_Released, Kb_e);
			break;
		case 0x93:
			kb_event(Kb_Released, Kb_r);
			break;
		case 0x94:
			kb_event(Kb_Released, Kb_t);
			break;
		case 0x95:
			kb_event(Kb_Released, Kb_y);
			break;
		case 0x96:
			kb_event(Kb_Released, Kb_u);
			break;
		case 0x97:
			kb_event(Kb_Released, Kb_i);
			break;
		case 0x98:
			kb_event(Kb_Released, Kb_o);
			break;
		case 0x99:
			kb_event(Kb_Released, Kb_p);
			break;
		case 0x9A:
			kb_event(Kb_Released, Kb_LBra);
			break;
		case 0x9B:
			kb_event(Kb_Released, Kb_RBra);
			break;
		case 0x9C:
			kb_event(Kb_Released, Kb_Enter);
			break;
		case 0x9D:
			kb_event(Kb_Released, Kb_LCtrl);
			break;
		case 0x9E:
			kb_event(Kb_Released, Kb_a);
			break;
		case 0x9F:
			kb_event(Kb_Released, Kb_s);
			break;
		case 0xA0:
			kb_event(Kb_Released, Kb_d);
			break;
		case 0xA1:
			kb_event(Kb_Released, Kb_f);
			break;
		case 0xA2:
			kb_event(Kb_Released, Kb_g);
			break;
		case 0xA3:
			kb_event(Kb_Released, Kb_h);
			break;
		case 0xA4:
			kb_event(Kb_Released, Kb_j);
			break;
		case 0xA5:
			kb_event(Kb_Released, Kb_k);
			break;
		case 0xA6:
			kb_event(Kb_Released, Kb_l);
			break;
		case 0xA7:
			kb_event(Kb_Released, Kb_Scol);
			break;
		case 0xA8:
			kb_event(Kb_Released, Kb_Quote);
			break;
		case 0xA9:
			kb_event(Kb_Released, Kb_Backtick);
			break;
		case 0xAA:
			kb_event(Kb_Released, Kb_LShift);
			break;
		case 0xAB:
			kb_event(Kb_Released, Kb_Antislash);
			break;
		case 0xAC:
			kb_event(Kb_Released, Kb_z);
			break;
		case 0xAD:
			kb_event(Kb_Released, Kb_x);
			break;
		case 0xAE:
			kb_event(Kb_Released, Kb_c);
			break;
		case 0xAF:
			kb_event(Kb_Released, Kb_v);
			break;
		case 0xB0:
			kb_event(Kb_Released, Kb_b);
			break;
		case 0xB1:
			kb_event(Kb_Released, Kb_n);
			break;
		case 0xB2:
			kb_event(Kb_Released, Kb_m);
			break;
		case 0xB3:
			kb_event(Kb_Released, Kb_Comma);
			break;
		case 0xB4:
			kb_event(Kb_Released, Kb_Period);
			break;
		case 0xB5:
			kb_event(Kb_Released, Kb_Slash);
			break;
		case 0xB6:
			kb_event(Kb_Released, Kb_RShift);
			break;
		case 0xB7:
			kb_event(Kb_Released, Kb_KP_Star);
			break;
		case 0xB8:
			kb_event(Kb_Released, Kb_LAlt);
			break;
		case 0xB9:
			kb_event(Kb_Released, Kb_Space);
			break;
		case 0xBA:
			kb_event(Kb_Released, Kb_CapsLock);
			break;
		case 0xBB:
			kb_event(Kb_Released, Kb_F1);
			break;
		case 0xBC:
			kb_event(Kb_Released, Kb_F2);
			break;
		case 0xBD:
			kb_event(Kb_Released, Kb_F3);
			break;
		case 0xBE:
			kb_event(Kb_Released, Kb_F4);
			break;
		case 0xBF:
			kb_event(Kb_Released, Kb_F5);
			break;
		case 0xC0:
			kb_event(Kb_Released, Kb_F6);
			break;
		case 0xC1:
			kb_event(Kb_Released, Kb_F7);
			break;
		case 0xC2:
			kb_event(Kb_Released, Kb_F8);
			break;
		case 0xC3:
			kb_event(Kb_Released, Kb_F9);
			break;
		case 0xC4:
			kb_event(Kb_Released, Kb_F10);
			break;
		case 0xC5:
			kb_event(Kb_Released, Kb_NumLock);
			break;
		case 0xC6:
			kb_event(Kb_Released, Kb_ScrollLock);
			break;
		case 0xC7:
			kb_event(Kb_Released, Kb_KP_7);
			break;
		case 0xC8:
			kb_event(Kb_Released, Kb_KP_8);
			break;
		case 0xC9:
			kb_event(Kb_Released, Kb_KP_9);
			break;
		case 0xCA:
			kb_event(Kb_Released, Kb_KP_Minus);
			break;
		case 0xCB:
			kb_event(Kb_Released, Kb_KP_4);
			break;
		case 0xCC:
			kb_event(Kb_Released, Kb_KP_5);
			break;
		case 0xCD:
			kb_event(Kb_Released, Kb_KP_6);
			break;
		case 0xCE:
			kb_event(Kb_Released, Kb_KP_Plus);
			break;
		case 0xCF:
			kb_event(Kb_Released, Kb_KP_1);
			break;
		case 0xD0:
			kb_event(Kb_Released, Kb_KP_2);
			break;
		case 0xD1:
			kb_event(Kb_Released, Kb_KP_3);
			break;
		case 0xD2:
			kb_event(Kb_Released, Kb_KP_0);
			break;
		case 0xD3:
			kb_event(Kb_Released, Kb_KP_Period);
			break;
		case 0xD7:
			kb_event(Kb_Released, Kb_F11);
			break;
		case 0xD8:
			kb_event(Kb_Released, Kb_F12);
			break;
		default:
			break;
	}
}

void init_keyboard()
{
	kb_callbacks = create_dynarray();
	kb_receive_scancode = kb_step1;
}

void kb_event(kb_event_type type, kb_event_key key)
{
	kb_event_t evt = {type, key};
	for (int i = 0 ; i < kb_callbacks->size ; i++)
	{
		void (*to_call)(kb_event_t) = (void(*)(kb_event_t)) kb_callbacks->array[i];
		if (to_call)
			to_call(evt);
	}
}

