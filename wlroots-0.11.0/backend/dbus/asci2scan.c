#include <string.h>
#include <stdio.h>
#include "wlr/backend/dbus.h"

char* kbd_US [128] =
{
"",
"",
" 1 ! ",
" 2 @ ",
" 3 # ",
" 4 $ ",
" 5 % ",
" 6 ^ ",
" 7 & ",
" 8 * ",
" 9 ( ",
" 0 ) ",
" - _ ",
" = + ",
" Backspace ",
" Tab ",
" q Q ",
" w W ",
" e E ",
" r R ",
" t T ",
" y Y ",
" u U ",
" i I ",
" o O ",
" p P ",
" [ { ",
" ] } ",
" Enter ",
" Control ", /* <-- control key */
" a A ",
" s S ",
" d D ",
" f F ",
" g G ",
" h H ",
" j J ",
" k K ",
" l L ",
" ; : ",
"",
" ` \" ",
" Shift ",
" \\ | ",
" z Z ",
" x X ",
" c C ",
" v V ",
" b B ",
" n N ",
" m M ",
" , < ",
" . > ",
" / ? ",
"",
" * ",
" Alt ",  /* Alt */
" Space ",  /* Space bar */
" CapsLock ",  /* Caps lock */
" F1 ",  /* 59 - F1 key ... > */
" F2 ",
" F3 ",
" F4 ",
" F5 ",
" F6 ",
" F7 ",
" F8 ",
" F9 ",
" F10 ",  /* < ... F10 */
" NumLock ",  /* 69 - Num lock*/
" ScrollLock ",  /* Scroll Lock */
" Home ",  /* Home key */
" ArrowUp ",  /* Up Arrow */
" PageUp ",  /* Page Up */
" - ",
" ArrowLeft ",  /* Left Arrow */
"",
" ArrowRight ",  /* Right Arrow */
" + ",
" End ",  /* 79 - End key*/
" ArrowDown ",  /* Down Arrow */
" PageDown ",  /* Page Down */
" Insert ",  /* Insert Key */
" Delete ",  /* Delete Key */
"",
"",
"",
" F11 ",  /* F11 Key */
" F12 ",  /* F12 Key */
"",  /* All other keys are undefined */
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
};


int asci2scan(char* key) {
    char key2[32];
    sprintf(key2, " %s ", key);
    for (int code = 0; code < 128; code++) {
	if (strstr(kbd_US[code], key2) != NULL) return code;
    }
    return 57;
};
