/* This header must be included *only* from be_events_sdl.c */

#define emptyDOSKeyEvent {false, 0}

const emulatedDOSKeyEvent sdlKeyMappings[SDL_NUM_SCANCODES] = {
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    {false,  BE_ST_SC_A}, // SDL_SCANCODE_A
    {false,  BE_ST_SC_B}, // SDL_SCANCODE_B
    {false,  BE_ST_SC_C}, // SDL_SCANCODE_C
    {false,  BE_ST_SC_D}, // SDL_SCANCODE_D
    {false,  BE_ST_SC_E}, // SDL_SCANCODE_E
    {false,  BE_ST_SC_F}, // SDL_SCANCODE_F
    {false,  BE_ST_SC_G}, // SDL_SCANCODE_G
    {false,  BE_ST_SC_H}, // SDL_SCANCODE_H
    {false,  BE_ST_SC_I}, // SDL_SCANCODE_I
    {false,  BE_ST_SC_J}, // SDL_SCANCODE_J
    {false,  BE_ST_SC_K}, // SDL_SCANCODE_K
    {false,  BE_ST_SC_L}, // SDL_SCANCODE_L
    {false,  BE_ST_SC_M}, // SDL_SCANCODE_M
    {false,  BE_ST_SC_N}, // SDL_SCANCODE_N
    {false,  BE_ST_SC_O}, // SDL_SCANCODE_O
    {false,  BE_ST_SC_P}, // SDL_SCANCODE_P
    {false,  BE_ST_SC_Q}, // SDL_SCANCODE_Q
    {false,  BE_ST_SC_R}, // SDL_SCANCODE_R
    {false,  BE_ST_SC_S}, // SDL_SCANCODE_S
    {false,  BE_ST_SC_T}, // SDL_SCANCODE_T
    {false,  BE_ST_SC_U}, // SDL_SCANCODE_U
    {false,  BE_ST_SC_V}, // SDL_SCANCODE_V
    {false,  BE_ST_SC_W}, // SDL_SCANCODE_W
    {false,  BE_ST_SC_X}, // SDL_SCANCODE_X
    {false,  BE_ST_SC_Y}, // SDL_SCANCODE_Y
    {false,  BE_ST_SC_Z}, // SDL_SCANCODE_Z

    {false,  BE_ST_SC_1}, // SDL_SCANCODE_1
    {false,  BE_ST_SC_2}, // SDL_SCANCODE_2
    {false,  BE_ST_SC_3}, // SDL_SCANCODE_3
    {false,  BE_ST_SC_4}, // SDL_SCANCODE_4
    {false,  BE_ST_SC_5}, // SDL_SCANCODE_5
    {false,  BE_ST_SC_6}, // SDL_SCANCODE_6
    {false,  BE_ST_SC_7}, // SDL_SCANCODE_7
    {false,  BE_ST_SC_8}, // SDL_SCANCODE_8
    {false,  BE_ST_SC_9}, // SDL_SCANCODE_9
    {false,  BE_ST_SC_0}, // SDL_SCANCODE_0

    {false,  BE_ST_SC_ENTER}, // SDL_SCANCODE_RETURN
    {false,  BE_ST_SC_ESC}, // SDL_SCANCODE_ESCAPE
    {false,  BE_ST_SC_BSPACE}, // SDL_SCANCODE_BACKSPACE
    {false,  BE_ST_SC_TAB}, // SDL_SCANCODE_TAB
    {false,  BE_ST_SC_SPACE}, // SDL_SCANCODE_SPACE

    {false,  BE_ST_SC_MINUS}, // SDL_SCANCODE_MINUS
    {false,  BE_ST_SC_EQUALS}, // SDL_SCANCODE_EQUALS
    {false,  BE_ST_SC_LBRACKET}, // SDL_SCANCODE_LEFTBRACKET
    {false,  BE_ST_SC_RBRACKET}, // SDL_SCANCODE_RIGHTBRACKET
    {false,  BE_ST_SC_BACKSLASH}, // SDL_SCANCODE_BACKSLASH

    emptyDOSKeyEvent, // SDL_SCANCODE_NONUSHASH

    {false,  BE_ST_SC_SEMICOLON}, // SDL_SCANCODE_SEMICOLON
    {false,  BE_ST_SC_QUOTE}, // SDL_SCANCODE_APOSTROPHE
    {false,  BE_ST_SC_GRAVE}, // SDL_SCANCODE_GRAVE
    {false,  BE_ST_SC_COMMA}, // SDL_SCANCODE_COMMA
    {false,  BE_ST_SC_PERIOD}, // SDL_SCANCODE_PERIOD
    {false,  BE_ST_SC_SLASH}, // SDL_SCANCODE_SLASH

    {false,  BE_ST_SC_CAPSLOCK}, // SDL_SCANCODE_CAPSLOCK

    {false,  BE_ST_SC_F1}, // SDL_SCANCODE_F1
    {false,  BE_ST_SC_F2}, // SDL_SCANCODE_F2
    {false,  BE_ST_SC_F3}, // SDL_SCANCODE_F3
    {false,  BE_ST_SC_F4}, // SDL_SCANCODE_F4
    {false,  BE_ST_SC_F5}, // SDL_SCANCODE_F5
    {false,  BE_ST_SC_F6}, // SDL_SCANCODE_F6
    {false,  BE_ST_SC_F7}, // SDL_SCANCODE_F7
    {false,  BE_ST_SC_F8}, // SDL_SCANCODE_F8
    {false,  BE_ST_SC_F9}, // SDL_SCANCODE_F9
    {false,  BE_ST_SC_F10}, // SDL_SCANCODE_F10
    {false,  BE_ST_SC_F11}, // SDL_SCANCODE_F11
    {false,  BE_ST_SC_F12}, // SDL_SCANCODE_F12

    {false,  BE_ST_SC_PRINTSCREEN}, // SDL_SCANCODE_PRINTSCREEN
    {false,  BE_ST_SC_SCROLLLOCK}, // SDL_SCANCODE_SCROLLLOCK
    {false,  BE_ST_SC_PAUSE}, // SDL_SCANCODE_PAUSE
    {true,  BE_ST_SC_INSERT}, // SDL_SCANCODE_INSERT
    {true,  BE_ST_SC_HOME}, // SDL_SCANCODE_HOME
    {true,  BE_ST_SC_PAGEUP}, // SDL_SCANCODE_PAGEUP
    {true,  BE_ST_SC_DELETE}, // SDL_SCANCODE_DELETE
    {true,  BE_ST_SC_END}, // SDL_SCANCODE_END
    {true,  BE_ST_SC_PAGEDOWN}, // SDL_SCANCODE_PAGEDOWN
    {true,  BE_ST_SC_RIGHT}, // SDL_SCANCODE_RIGHT
    {true,  BE_ST_SC_LEFT}, // SDL_SCANCODE_LEFT
    {true,  BE_ST_SC_DOWN}, // SDL_SCANCODE_DOWN
    {true,  BE_ST_SC_UP}, // SDL_SCANCODE_UP

    {false,  BE_ST_SC_NUMLOCK}, // SDL_SCANCODE_NUMLOCKCLEAR
    {true,  BE_ST_SC_KP_DIVIDE}, // SDL_SCANCODE_KP_DIVIDE
    {false,  BE_ST_SC_KP_MULTIPLY}, // SDL_SCANCODE_KP_MULTIPLY
    {false,  BE_ST_SC_KP_MINUS}, // SDL_SCANCODE_KP_MINUS
    {false,  BE_ST_SC_KP_PLUS}, // SDL_SCANCODE_KP_PLUS
    {true,  BE_ST_SC_KP_ENTER}, // SDL_SCANCODE_KP_ENTER
    {false,  BE_ST_SC_KP_1}, // SDL_SCANCODE_KP_1
    {false,  BE_ST_SC_KP_2}, // SDL_SCANCODE_KP_2
    {false,  BE_ST_SC_KP_3}, // SDL_SCANCODE_KP_3
    {false,  BE_ST_SC_KP_4}, // SDL_SCANCODE_KP_4
    {false,  BE_ST_SC_KP_5}, // SDL_SCANCODE_KP_5
    {false,  BE_ST_SC_KP_6}, // SDL_SCANCODE_KP_6
    {false,  BE_ST_SC_KP_7}, // SDL_SCANCODE_KP_7
    {false,  BE_ST_SC_KP_8}, // SDL_SCANCODE_KP_8
    {false,  BE_ST_SC_KP_9}, // SDL_SCANCODE_KP_9
    {false,  BE_ST_SC_KP_0}, // SDL_SCANCODE_KP_0
    {false,  BE_ST_SC_KP_PERIOD}, // SDL_SCANCODE_KP_PERIOD

    {false,  BE_ST_SC_LESSTHAN}, // SDL_SCANCODE_NONUSBACKSLASH

    // SDL 2.0 scancodes 101 to 223 (either actually defined or not) follow...
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    {false,  BE_ST_SC_ENTER}, // SDL_SCANCODE_SELECT (for Android, d-pad press)
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,

    {false,  BE_ST_SC_LCTRL}, // SDL_SCANCODE_LCTRL
    {false,  BE_ST_SC_LSHIFT}, // SDL_SCANCODE_LSHIFT
    {false,  BE_ST_SC_LALT}, // SDL_SCANCODE_LALT

    emptyDOSKeyEvent, // SDL_SCANCODE_LGUI

    {true,  BE_ST_SC_RCTRL}, // SDL_SCANCODE_RCTRL
    {false,  BE_ST_SC_RSHIFT}, // SDL_SCANCODE_RSHIFT
    {true,  BE_ST_SC_RALT}, // SDL_SCANCODE_RALT

    // SDL 2.0 scancodes 231 to 269...

    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,
    emptyDOSKeyEvent,

    {false,  BE_ST_SC_ESC}, // SDL_SCANCODE_AC_BACK (for Android)

    // More SDL 2.0 scancodes follow, but are ignored
};
