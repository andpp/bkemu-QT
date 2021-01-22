// сопоставление стандартных имён клавиш стандартным же кодам


#include "pch.h"
#include "vkeycodes.h"

UINT getKeyValue(CString& str)
{
	for (int i = 0; i < 256; ++i)
	{
		if (g_VKeyNames[i] == str)
		{
			return i;
		}
	}

	return -1;
}



// наоборот, массив, чтобы по коду определить имя виртуальной клавиши

CString g_VKeyNames[256] = {
	_T("vk_nullable"),      //0x00
/*
 * Virtual Keys, Standard Set
 */
	_T("VK_LBUTTON"),        //0x01
	_T("VK_RBUTTON"),        //0x02
	_T("VK_CANCEL"),         //0x03
	_T("VK_MBUTTON"),        //0x04    /* NOT contiguous with L & RBUTTON */
	_T("VK_XBUTTON1"),       //0x05    /* NOT contiguous with L & RBUTTON */
	_T("VK_XBUTTON2"),       //0x06    /* NOT contiguous with L & RBUTTON */
    _T("vk_reserved_0x07"),	//0x07 : reserved
	_T("VK_BACK"),           //0x08
	_T("VK_TAB"),            //0x09
	_T("vk_reserved_0x0A"),	//0x0A : reserved
	_T("vk_reserved_0x0B"),	//0x0B : reserved
	_T("VK_CLEAR"),          //0x0C
	_T("VK_RETURN"),         //0x0D
	_T("vk_unassigned_0x0E"),	//0x0E: unassigned
	_T("vk_unassigned_0x0F"),	//0x0F: unassigned
	_T("VK_SHIFT"),          //0x10
	_T("VK_CONTROL"),        //0x11
	_T("VK_MENU"),           //0x12
	_T("VK_PAUSE"),          //0x13
	_T("VK_CAPITAL"),        //0x14
	_T("VK_KANA"),           //0x15	_T("VK_HANGUL"),         //0x15
	_T("vk_unassigned_0x16"),	// 0x16 : unassigned
	_T("VK_JUNJA"),          //0x17
	_T("VK_FINAL"),          //0x18
	_T("VK_HANJA"),          //0x19	_T("VK_KANJI"),          //0x19
	_T("vk_unassigned_0x1A"),	// 0x1A : unassigned
	_T("VK_ESCAPE"),         //0x1B
	_T("VK_CONVERT"),        //0x1C
	_T("VK_NONCONVERT"),     //0x1D
	_T("VK_ACCEPT"),         //0x1E
	_T("VK_MODECHANGE"),     //0x1F
	_T("VK_SPACE"),          //0x20
	_T("VK_PRIOR"),          //0x21
	_T("VK_NEXT"),           //0x22
	_T("VK_END"),            //0x23
	_T("VK_HOME"),           //0x24
	_T("VK_LEFT"),           //0x25
	_T("VK_UP"),             //0x26
	_T("VK_RIGHT"),          //0x27
	_T("VK_DOWN"),           //0x28
	_T("VK_SELECT"),         //0x29
	_T("VK_PRINT"),          //0x2A
	_T("VK_EXECUTE"),        //0x2B
	_T("VK_SNAPSHOT"),       //0x2C
	_T("VK_INSERT"),         //0x2D
	_T("VK_DELETE"),         //0x2E
	_T("VK_HELP"),           //0x2F
 // VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 	_T("VK_0"),				//0x30
	_T("VK_1"),				//0x31
	_T("VK_2"),				//0x32
	_T("VK_3"),				//0x33
	_T("VK_4"),				//0x34
	_T("VK_5"),				//0x35
	_T("VK_6"),				//0x36
	_T("VK_7"),				//0x37
	_T("VK_8"),				//0x38
	_T("VK_9"),				//0x39
 // 0x3A - 0x40 : unassigned
	_T("vk_unassigned_0x3A"),	// 0x3A
	_T("vk_unassigned_0x3B"),	// 0x3B
	_T("vk_unassigned_0x3C"),	// 0x3C
	_T("vk_unassigned_0x3D"),	// 0x3D
	_T("vk_unassigned_0x3E"),	// 0x3E
	_T("vk_unassigned_0x3F"),	// 0x3F
	_T("vk_unassigned_0x40"),	// 0x40
 // VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 	_T("VK_A"),				//0x41
	_T("VK_B"),				//0x42
	_T("VK_C"),				//0x43
	_T("VK_D"),				//0x44
	_T("VK_E"),				//0x45
	_T("VK_F"),				//0x46
	_T("VK_G"),				//0x47
	_T("VK_H"),				//0x48
	_T("VK_I"),				//0x49
	_T("VK_J"),				//0x4A
	_T("VK_K"),				//0x4B
	_T("VK_L"),				//0x4C
	_T("VK_M"),				//0x4D
	_T("VK_N"),				//0x4E
	_T("VK_O"),				//0x4F
	_T("VK_P"),				//0x50
	_T("VK_Q"),				//0x51
	_T("VK_R"),				//0x52
	_T("VK_S"),				//0x53
	_T("VK_T"),				//0x54
	_T("VK_U"),				//0x55
	_T("VK_V"),				//0x56
	_T("VK_W"),				//0x57
	_T("VK_X"),				//0x58
	_T("VK_Y"),				//0x59
	_T("VK_Z"),				//0x5A
	_T("VK_LWIN"),           //0x5B
	_T("VK_RWIN"),           //0x5C
	_T("VK_APPS"),           //0x5D
	_T("vk_reserved_0x5E"),	//0x5E : reserved
	_T("VK_SLEEP"),          //0x5F
	_T("VK_NUMPAD0"),        //0x60
	_T("VK_NUMPAD1"),        //0x61
	_T("VK_NUMPAD2"),        //0x62
	_T("VK_NUMPAD3"),        //0x63
	_T("VK_NUMPAD4"),        //0x64
	_T("VK_NUMPAD5"),        //0x65
	_T("VK_NUMPAD6"),        //0x66
	_T("VK_NUMPAD7"),        //0x67
	_T("VK_NUMPAD8"),        //0x68
	_T("VK_NUMPAD9"),        //0x69
	_T("VK_MULTIPLY"),       //0x6A
	_T("VK_ADD"),            //0x6B
	_T("VK_SEPARATOR"),      //0x6C
	_T("VK_SUBTRACT"),       //0x6D
	_T("VK_DECIMAL"),        //0x6E
	_T("VK_DIVIDE"),         //0x6F
	_T("VK_F1"),             //0x70
	_T("VK_F2"),             //0x71
	_T("VK_F3"),             //0x72
	_T("VK_F4"),             //0x73
	_T("VK_F5"),             //0x74
	_T("VK_F6"),             //0x75
	_T("VK_F7"),             //0x76
	_T("VK_F8"),             //0x77
	_T("VK_F9"),             //0x78
	_T("VK_F10"),            //0x79
	_T("VK_F11"),            //0x7A
	_T("VK_F12"),            //0x7B
	_T("VK_F13"),            //0x7C
	_T("VK_F14"),            //0x7D
	_T("VK_F15"),            //0x7E
	_T("VK_F16"),            //0x7F
	_T("VK_F17"),            //0x80
	_T("VK_F18"),            //0x81
	_T("VK_F19"),            //0x82
	_T("VK_F20"),            //0x83
	_T("VK_F21"),            //0x84
	_T("VK_F22"),            //0x85
	_T("VK_F23"),            //0x86
	_T("VK_F24"),            //0x87
/*
 * 0x88 - 0x8F : UI navigation
 */
	_T("VK_NAVIGATION_VIEW"),     //0x88 // reserved
	_T("VK_NAVIGATION_MENU"),     //0x89 // reserved
	_T("VK_NAVIGATION_UP"),       //0x8A // reserved
	_T("VK_NAVIGATION_DOWN"),     //0x8B // reserved
	_T("VK_NAVIGATION_LEFT"),     //0x8C // reserved
	_T("VK_NAVIGATION_RIGHT"),    //0x8D // reserved
	_T("VK_NAVIGATION_ACCEPT"),   //0x8E // reserved
	_T("VK_NAVIGATION_CANCEL"),   //0x8F // reserved
	_T("VK_NUMLOCK"),        //0x90
	_T("VK_SCROLL"),         //0x91
/*
 * NEC PC-9800 kbd definitions
 */
	_T("VK_OEM_NEC_EQUAL"),  //0x92   // '=' key on numpad
/*
 * Fujitsu/OASYS kbd definitions
 */
	//_T("VK_OEM_FJ_JISHO"),   //0x92   // 'Dictionary' key
	_T("VK_OEM_FJ_MASSHOU"), //0x93   // 'Unregister word' key
	_T("VK_OEM_FJ_TOUROKU"), //0x94   // 'Register word' key
	_T("VK_OEM_FJ_LOYA"),    //0x95   // 'Left OYAYUBI' key
	_T("VK_OEM_FJ_ROYA"),    //0x96   // 'Right OYAYUBI' key
	_T("vk_unassigned_0x97"),	// 0x97 : unassigned
	_T("vk_unassigned_0x98"),	// 0x98 : unassigned
	_T("vk_unassigned_0x9A"),	// 0x9A : unassigned
	_T("vk_unassigned_0x9B"),	// 0x9B : unassigned
	_T("vk_unassigned_0x9C"),	// 0x9C : unassigned
	_T("vk_unassigned_0x9D"),	// 0x9D : unassigned
	_T("vk_unassigned_0x9E"),	// 0x9E : unassigned
	_T("vk_unassigned_0x9F"),	// 0x9F : unassigned

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
	_T("VK_LSHIFT"),         //0xA0
	_T("VK_RSHIFT"),         //0xA1
	_T("VK_LCONTROL"),       //0xA2
	_T("VK_RCONTROL"),       //0xA3
	_T("VK_LMENU"),          //0xA4
	_T("VK_RMENU"),          //0xA5
	_T("VK_BROWSER_BACK"),        //0xA6
	_T("VK_BROWSER_FORWARD"),     //0xA7
	_T("VK_BROWSER_REFRESH"),     //0xA8
	_T("VK_BROWSER_STOP"),        //0xA9
	_T("VK_BROWSER_SEARCH"),      //0xAA
	_T("VK_BROWSER_FAVORITES"),   //0xAB
	_T("VK_BROWSER_HOME"),        //0xAC
	_T("VK_VOLUME_MUTE"),         //0xAD
	_T("VK_VOLUME_DOWN"),         //0xAE
	_T("VK_VOLUME_UP"),           //0xAF
	_T("VK_MEDIA_NEXT_TRACK"),    //0xB0
	_T("VK_MEDIA_PREV_TRACK"),    //0xB1
	_T("VK_MEDIA_STOP"),          //0xB2
	_T("VK_MEDIA_PLAY_PAUSE"),    //0xB3
	_T("VK_LAUNCH_MAIL"),         //0xB4
	_T("VK_LAUNCH_MEDIA_SELECT"), //0xB5
	_T("VK_LAUNCH_APP1"),         //0xB6
	_T("VK_LAUNCH_APP2"),         //0xB7
	_T("vk_reserved_0xB8"),	//0xB8 : reserved
	_T("vk_reserved_0xB9"),	//0xB9 : reserved
	_T("VK_OEM_1"),          //0xBA   // ';:' for US
	_T("VK_OEM_PLUS"),       //0xBB   // '+' any country
	_T("VK_OEM_COMMA"),      //0xBC   // ',' any country
	_T("VK_OEM_MINUS"),      //0xBD   // '-' any country
	_T("VK_OEM_PERIOD"),     //0xBE   // '.' any country
	_T("VK_OEM_2"),          //0xBF   // '/?' for US
	_T("VK_OEM_3"),          //0xC0   // '`~' for US
	_T("vk_reserved_0xC1"),	//0xC1 : reserved
	_T("vk_reserved_0xC2"),	//0xC2 : reserved
/*
 * 0xC3 - 0xDA : Gamepad input
 */
	_T("VK_GAMEPAD_A"),                         //0xC3 // reserved
	_T("VK_GAMEPAD_B"),                         //0xC4 // reserved
	_T("VK_GAMEPAD_X"),                         //0xC5 // reserved
	_T("VK_GAMEPAD_Y"),                         //0xC6 // reserved
	_T("VK_GAMEPAD_RIGHT_SHOULDER"),            //0xC7 // reserved
	_T("VK_GAMEPAD_LEFT_SHOULDER"),             //0xC8 // reserved
	_T("VK_GAMEPAD_LEFT_TRIGGER"),              //0xC9 // reserved
	_T("VK_GAMEPAD_RIGHT_TRIGGER"),             //0xCA // reserved
	_T("VK_GAMEPAD_DPAD_UP"),                   //0xCB // reserved
	_T("VK_GAMEPAD_DPAD_DOWN"),                 //0xCC // reserved
	_T("VK_GAMEPAD_DPAD_LEFT"),                 //0xCD // reserved
	_T("VK_GAMEPAD_DPAD_RIGHT"),                //0xCE // reserved
	_T("VK_GAMEPAD_MENU"),                      //0xCF // reserved
	_T("VK_GAMEPAD_VIEW"),                      //0xD0 // reserved
	_T("VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON"),    //0xD1 // reserved
	_T("VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON"),   //0xD2 // reserved
	_T("VK_GAMEPAD_LEFT_THUMBSTICK_UP"),        //0xD3 // reserved
	_T("VK_GAMEPAD_LEFT_THUMBSTICK_DOWN"),      //0xD4 // reserved
	_T("VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT"),     //0xD5 // reserved
	_T("VK_GAMEPAD_LEFT_THUMBSTICK_LEFT"),      //0xD6 // reserved
	_T("VK_GAMEPAD_RIGHT_THUMBSTICK_UP"),       //0xD7 // reserved
	_T("VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN"),     //0xD8 // reserved
	_T("VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT"),    //0xD9 // reserved
	_T("VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT"),     //0xDA // reserved
	_T("VK_OEM_4"),          //0xDB  //  '[{' for US
	_T("VK_OEM_5"),          //0xDC  //  '\|' for US
	_T("VK_OEM_6"),          //0xDD  //  ']}' for US
	_T("VK_OEM_7"),          //0xDE  //  ''"' for US
	_T("VK_OEM_8"),          //0xDF
	_T("vk_reserved_0xE0"),	//0xE0 : reserved

/*
 * Various extended or enhanced keyboards
 */
	_T("VK_OEM_AX"),         //0xE1  //  'AX' key on Japanese AX kbd
	_T("VK_OEM_102"),        //0xE2  //  "<>" or "\|" on RT 102-key kbd.
	_T("VK_ICO_HELP"),       //0xE3  //  Help key on ICO
	_T("VK_ICO_00"),         //0xE4  //  00 key on ICO
	_T("VK_PROCESSKEY"),     //0xE5
	_T("VK_ICO_CLEAR"),      //0xE6
	_T("VK_PACKET"),         //0xE7
	_T("vk_unassigned_0xE8"),	// 0xE8 : unassigned
/*
 * Nokia/Ericsson definitions
 */
	_T("VK_OEM_RESET"),      //0xE9
	_T("VK_OEM_JUMP"),       //0xEA
	_T("VK_OEM_PA1"),        //0xEB
	_T("VK_OEM_PA2"),        //0xEC
	_T("VK_OEM_PA3"),        //0xED
	_T("VK_OEM_WSCTRL"),     //0xEE
	_T("VK_OEM_CUSEL"),      //0xEF
	_T("VK_OEM_ATTN"),       //0xF0
	_T("VK_OEM_FINISH"),     //0xF1
	_T("VK_OEM_COPY"),       //0xF2
	_T("VK_OEM_AUTO"),       //0xF3
	_T("VK_OEM_ENLW"),       //0xF4
	_T("VK_OEM_BACKTAB"),    //0xF5

	_T("VK_ATTN"),           //0xF6
	_T("VK_CRSEL"),          //0xF7
	_T("VK_EXSEL"),          //0xF8
	_T("VK_EREOF"),          //0xF9
	_T("VK_PLAY"),           //0xFA
	_T("VK_ZOOM"),           //0xFB
	_T("VK_NONAME"),         //0xFC
	_T("VK_PA1"),            //0xFD
	_T("VK_OEM_CLEAR"),      //0xFE
	_T("vk_reserved_0xFF")	//0xFF : reserved
};

