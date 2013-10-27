/*
 * linuxkb module for Unified Remote, v0.1
 *
 * This library adds support for text and character input for Unified
 * Remote on Linux.  It has been tested with Ubuntu 13.04 and the
 * Linux (64-bit) version of the Unified Remote Server 3 DP4.
 *
 * Known issues:
 *
 * - This library simply assumes that the user is using the keyboard
 *   layout "English (US, international with dead keys)" in Ubuntu.
 *   When using other layouts, some keys will be mixed up.
 *   TODO solve: probably have to first convert char into keysim, then
 *   keysim to keycode(s) using some X libs (have to know keymap) or
 *   something like that (vague guess).
 *
 * - The function text(lua_State *L) here will parse the input string
 *   as an ASCII string, while Lua actually hands us an UTF8
 *   string. Hence Unicode characters are not (yet) supported.
 *   TODO solve: parse as UTF8 string instead and build in support for
 *   UTF8 characters.
 *
 * Changelog:
 *
 * v0.1 (24 Oct 2013)
 * - initial release
 *
 * Copyright (c) 2013 Malte Skoruppa <malte.skoruppa@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#define die(str, args...) do {				\
    fprintf(stderr, "linuxkb: error: %s\n",str);	\
    return -1;						\
  } while(0)

int                    fd;
struct input_event     ev;

// initialize uinput device
int uinput_init(void) {

  struct uinput_user_dev uidev;
  
  // open character device in write-only and non-blocking mode
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if(fd < 0)
    die("open");
  
  // inform the input subsystem we want to use EV_KEY events
  if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
    die("ioctl");
  // enable all keycodes
  int i;
  for( i = 0; i < 256; i++)
    if(ioctl(fd, UI_SET_KEYBIT, i) < 0)
      die("ioctl");

  // fill struct uidev with appropriate values
  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "linuxkb");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor  = 0x1;
  uidev.id.product = 0x1;
  uidev.id.version = 1;
  
  // write uidev to uinput file descriptor
  if(write(fd, &uidev, sizeof(uidev)) < 0)
    die("write");
  
  // request creation of device via ioctl request
  if(ioctl(fd, UI_DEV_CREATE) < 0)
    die("ioctl");

  return 0;
}

// convert ASCII chars to key codes
// atm we assume the keyboard layout to be "English (US, international with dead keys)" (in Ubuntu 13.04)
// see http://dry.sailingissues.com/us-international-keyboard-layout.html
// we then return the corresponding keycode according to <linux/input.h>
short char_to_keycode(char c) {

  short keycode;

  switch(c) {

  // these two are on many keyboard views on Android
  case ' ': keycode = KEY_SPACE; break;
  case '.': keycode = KEY_DOT; break;

  // normal keyboard
  case 'a': case 'A': keycode = KEY_A; break;
  case 'b': case 'B': keycode = KEY_B; break;
  case 'c': case 'C': keycode = KEY_C; break;
  case 'd': case 'D': keycode = KEY_D; break;
  case 'e': case 'E': keycode = KEY_E; break;
  case 'f': case 'F': keycode = KEY_F; break;
  case 'g': case 'G': keycode = KEY_G; break;
  case 'h': case 'H': keycode = KEY_H; break;
  case 'i': case 'I': keycode = KEY_I; break;
  case 'j': case 'J': keycode = KEY_J; break;
  case 'k': case 'K': keycode = KEY_K; break;
  case 'l': case 'L': keycode = KEY_L; break;
  case 'm': case 'M': keycode = KEY_M; break;
  case 'n': case 'N': keycode = KEY_N; break;
  case 'o': case 'O': keycode = KEY_O; break;
  case 'p': case 'P': keycode = KEY_P; break;
  case 'q': case 'Q': keycode = KEY_Q; break;
  case 'r': case 'R': keycode = KEY_R; break;
  case 's': case 'S': keycode = KEY_S; break;
  case 't': case 'T': keycode = KEY_T; break;
  case 'u': case 'U': keycode = KEY_U; break;
  case 'v': case 'V': keycode = KEY_V; break;
  case 'w': case 'W': keycode = KEY_W; break;
  case 'x': case 'X': keycode = KEY_X; break;
  case 'y': case 'Y': keycode = KEY_Y; break;
  case 'z': case 'Z': keycode = KEY_Z; break;

  // special chars on Android keyboard, page 1
  case '1': keycode = KEY_1; break;
  case '2': keycode = KEY_2; break;
  case '3': keycode = KEY_3; break;
  case '4': keycode = KEY_4; break;
  case '5': keycode = KEY_5; break;
  case '6': keycode = KEY_6; break;
  case '7': keycode = KEY_7; break;
  case '8': keycode = KEY_8; break;
  case '9': keycode = KEY_9; break;
  case '0': keycode = KEY_0; break;

  case '@': keycode = KEY_2; break; // with SHIFT
  case '#': keycode = KEY_3; break; // with SHIFT
  //case '€': keycode = KEY_5; break; // with ALTGR; not ASCII
  case '%': keycode = KEY_5; break; // with SHIFT
  case '&': keycode = KEY_7; break; // with SHIFT
  case '*': keycode = KEY_8; break; // with SHIFT; alternative is KEY_KPASTERISK
  case '-': keycode = KEY_MINUS; break; // alternative is KEY_KPMINUS
  case '+': keycode = KEY_EQUAL; break; // with SHIFT; alternative is KEY_KPPLUS
  case '(': keycode = KEY_9; break; // with SHIFT
  case ')': keycode = KEY_0; break; // with SHIFT

  case '!': keycode = KEY_1; break; // with SHIFT
  case '"': keycode = KEY_APOSTROPHE; break; // with SHIFT, dead key
  case '\'': keycode = KEY_APOSTROPHE; break; // dead key
  case ':': keycode = KEY_SEMICOLON; break; // with SHIFT
  case ';': keycode = KEY_SEMICOLON; break;
  case '/': keycode = KEY_SLASH; break;
  case '?': keycode = KEY_SLASH; break; // with SHIFT

  case ',': keycode = KEY_COMMA; break;

  // special chars on Android keyboard, page 2
  case '~': keycode = KEY_GRAVE; break; // with SHIFT, dead key
  case '`': keycode = KEY_GRAVE; break; // dead key
  case '|': keycode = KEY_BACKSLASH; break; // with SHIFT
  // missing because there's no ASCII code:  •, √, π, ÷, ×
  case '{': keycode = KEY_LEFTBRACE; break; // with SHIFT
  case '}': keycode = KEY_RIGHTBRACE; break; // with SHIFT

  // note: TAB key is handled elsewhere
  // missing because there's no ASCII code: £, ¥
  case '$': keycode = KEY_4; break; // with SHIFT
  // missing because there's no ASCII code: °
  case '^': keycode = KEY_6; break; // with SHIFT, dead key
  case '_': keycode = KEY_MINUS; break; // with SHIFT
  case '=': keycode = KEY_EQUAL; break;
  case '[': keycode = KEY_LEFTBRACE; break;
  case ']': keycode = KEY_RIGHTBRACE; break;

  // missing because there's no ASCII code:  ™, ®, ©, ¶
  case '\\': keycode = KEY_BACKSLASH; break;
  case '<': keycode = KEY_COMMA; break; // with SHIFT
  case '>': keycode = KEY_DOT; break; // with SHIFT

  // missing because there's no ASCII code:  „, …

  default: keycode = -1;
  }

  return keycode;
}

// triggers uinput event
// event is type of event (either EV_KEY or EV_SYN)
// https://www.kernel.org/doc/Documentation/input/event-codes.txt
// keycode is one of the KEY_* values in <linux/input.h>
// value is either 1 (press) or 0 (release)
int uinput_event(int event, short keycode, int value) {

  memset(&ev, 0, sizeof(struct input_event));
  ev.type = event;
  ev.code = keycode;
  ev.value = value;
  if(write(fd, &ev, sizeof(struct input_event)) < 0)
    die("write");

  return 0;
}

// type (i.e., press and release) a key
int type_key(short keycode) {

  // press, release and SYN
  if( uinput_event(EV_KEY,keycode,1)<0 || uinput_event(EV_KEY,keycode,0)<0 || uinput_event(EV_SYN,0,0)<0)
    die("could not type");

  return 0;
}

// takes a char and types it (on a US international keyboard with dead keys)
int type_char(char c) {

  // for chars in shift_chars, press SHIFT modifier
  const char* shift_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ@#%&*+()!\":?~|{}$^_<>";
  if( strchr(shift_chars, c) != NULL) {
#ifdef DEBUG
  fprintf(stderr,"linuxkb: DEBUG: pressing SHIFT modifier\n");
#endif
    if( uinput_event(EV_KEY,KEY_LEFTSHIFT,1)<0)
      die("could not press shift");
  }

  // type keycode according to char_to_keycode()
  short keycode = char_to_keycode(c);
  if( keycode < 0)
    die("invalid keycode");
#ifdef DEBUG
  fprintf(stderr,"linuxkb: DEBUG: typing %c\n",c);
#endif
  type_key(keycode);

  // for chars in shift_chars, release SHIFT modifier
  if( strchr(shift_chars, c) != NULL) {
#ifdef DEBUG
  fprintf(stderr,"linuxkb: DEBUG: releasing SHIFT modifier\n");
#endif
    if( uinput_event(EV_KEY,KEY_LEFTSHIFT,0)<0 || uinput_event(EV_SYN,0,0)<0)
      die("could not release shift");
  }

  // for chars in dead_chars, type a SPACE at the end
  const char* dead_chars = "\"'~`^";
  if( strchr(dead_chars, c) != NULL) {
#ifdef DEBUG
    fprintf(stderr,"linuxkb: DEBUG: was a dead key, typing a SPACE\n");
#endif
    type_key(KEY_SPACE);
  }

  return 0;
}

// external function exposed by the module and which takes a text to type.
// expects a string on top of Lua stack.
int text(lua_State *L) {

  // get text string from Lua
  const char *text = lua_tostring(L, -1);

#ifdef DEBUG
  fprintf(stderr,"linuxkb: DEBUG: text is %s\n",text);
  int textlen = lua_strlen(L, -1);
  fprintf(stderr,"linuxkb: DEBUG: lua_strlen(L,-1) is %i\n", textlen);
#endif

  // iterate over string and simulate typing
  int i;
  for( i = 0; text[i] != '\0'; i++) {
    // for ASCII chars:
    if(text[i] >= 0) {
#ifdef DEBUG
      fprintf(stderr,"linuxkb: DEBUG: text[%i] is %c, ASCII code %d\n",i,text[i],text[i]);
#endif
      if( type_char(text[i])<0)
	fprintf(stderr,"linuxkb: warning: could not type char\n");
    }

    // the following block takes care of UTF8 chars (not supported yet, except for Euro symbol)
    // mind: this code is very temporary and serves purely experimentation purposes.
    // for actual deployment use a lib that allows true parsing of UTF8 strings, e.g., see
    // http://userguide.icu-project.org/strings
    // (install package libicu-dev in Ubuntu)
    // TODO read up on how to use it :-)
    else {
#ifdef DEBUG
      fprintf(stderr,"linuxkb: DEBUG: UTF8 char: ");
#endif
      int j;
      for( j=i; text[j] < 0; j++) {
#ifdef DEBUG
	fprintf(stderr,"%x,",text[j]);
#endif
      }
#ifdef DEBUG
      fprintf(stderr,"\n");
      fprintf(stderr, "linuxkb: DEBUG: length: %d\n", j-i);
#endif
      // if UTF8 code is 0xffffffe2 0xffffff82 0xffffffac, type a Euro symbol, otherwise don't type anything
      if( j-i == 3 && text[i] == 0xffffffe2 && text[i+1] == 0xffffff82 && text[i+2] == 0xffffffac) {
	// press ALTGR aka level 3 shift and '5'
	if( uinput_event(EV_KEY,KEY_RIGHTALT,1)<0 ||
	    type_key(KEY_5)<0 ||
	    uinput_event(EV_KEY,KEY_RIGHTALT,0)<0 ||
	    uinput_event(EV_SYN,0,0)<0)
	  fprintf(stderr,"linuxkb: warning: could not type UTF8 char\n");
      }
      else
	fprintf(stderr,"linuxkb: warning: only ASCII chars and the Euro symbol are supported!\n");
      // set i to the last negative char, so that in next iteration we get next positive char
      i = j-1;
    }
  }

  return 0;
}

// external function exposed by the module for some special comands.
// expects a string on top of Lua stack.
int press(lua_State *L) {

  // get input string from Lua stack
  const char *input = lua_tostring(L, -1);

#ifdef DEBUG
  fprintf(stderr,"linuxkb: DEBUG: input is %s\n",input);
#endif

  // special commands
  if( strcmp(input,"BACK") == 0) {
    if( type_key(KEY_BACKSPACE)<0)
      fprintf(stderr,"linuxkb: warning: could not type char\n");
  }
  else if( strcmp(input,"RETURN") == 0) {
    if( type_key(KEY_ENTER)<0)
      fprintf(stderr,"linuxkb: warning: could not type char\n");
  }
  else if( strcmp(input,"TAB") == 0) {
    if( type_key(KEY_TAB)<0)
      fprintf(stderr,"linuxkb: warning: could not type char\n");
  }
  else
    fprintf(stderr,"linuxkb: warning: invalid command\n");

  return 0;
}

// close uinput device (unused)
int uinput_destroy(void) {
  
  if(ioctl(fd, UI_DEV_DESTROY) < 0)
    die("ioctl");
  
  close(fd);
  
  return 0;
}

// register the external functions for Lua

// Lua 5.1 code:

static const struct luaL_reg linuxkb_funs [] = {
  {"text", text},
  {"press", press},
  {NULL, NULL} // sentinel
};

int luaopen_linuxkb (lua_State *L) {
  uinput_init();

  //sleep(2);

  luaL_openlib(L, "linuxkb", linuxkb_funs, 0);
  return 1;
}

// Lua 5.2 code:
/*
static const struct luaL_Reg linuxkb_funs [] = {
  {"text", text},
  {"press", press},
  {NULL, NULL} // sentinel
};

int luaopen_linuxkb (lua_State *L) {
  uinput_init();

  //sleep(2);

  lua_newtable(L);
  luaL_setfuncs(L,linuxkb_funs,0);
  lua_pushvalue(L,-1);        // pluck these lines out if they offend you
  lua_setglobal(L,"linuxkb"); // for they clobber the Holy _G
  return 1;
}
*/
