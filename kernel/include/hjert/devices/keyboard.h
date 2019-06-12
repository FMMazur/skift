#pragma once

typedef enum
{
    ERROR = 0,
    ESCAPE,
    NUM1,
    NUM2,
    NUM3,
    NUM4,
    NUM5,
    NUM6,
    NUM7,
    NUM8,
    NUM9,
    NUM0,
    MINUS,
    EQUAL,
    BACKSPACE,
    TAB,
    Q,
    W,
    E,
    R,
    T,
    Y,
    U,
    I,
    O,
    P,
    OPENB,
    CLOSEB,
    ENTER,
    LCONTROL,
    A,
    S,
    D,
    F,
    G,
    H,
    J,
    K,
    L,
    SCOLON,
    SQUOTE,
    BTICK,
    LSHIFT,
    BSLASH,
    Z,
    X,
    C,
    V,
    B,
    N,
    M,
    COMMA,
    POINT,
    SLASH,
    RSHIFT,
    KPMULTI,
    LALT,
    SPACE,
    CAPS_LOCK,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    NUM_LOCK,
    SCROLL_LOCK,
    KP7,
    KP8,
    KP9,
    KPMINUS,
    KP4,
    KP5,
    KP6,
    KPPLUS,
    KP1,
    KP2,
    KP3,
    KP0,
    KPPOINT,
    F11,
    F12,
    ANGLE_BRACKET,

    // SPECIAL KEYS
    UP,
    DOWN,
    LEFT,
    RIGHT,

    KPENTER,
    RCONTROL,
    RALT,

    SUPER,

    INSERT,
    DELETE,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,

    KEYBOAD_KEY_END
} keyboard_key_t;

#define KEYBOARD_DEVICE "/dev/kbd"

#define KEYBOARD_CHANNEL "#dev:keyboard"

#define KEYBOARD_KEYPRESSED "dev:keyboard.keypressed"
#define KEYBOARD_KEYRELEASED "dev:keyboard.keyreleased"
#define KEYBOARD_KEYTYPED "dev:keyboard.keytyped"

typedef struct
{
    char c;
    keyboard_key_t key;
} keyboard_event_t;