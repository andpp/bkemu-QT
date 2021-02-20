#pragma once

#define IDB_BITMAP_SOFT 0
#define IDB_BITMAP_PLEN 1

enum res_codes : int {
IDS_ALTPRO_CODE = 0,
IDS_ALTPRO_MODE,
IDS_BK_ERROR_BLKBASEMEM11,
IDS_BK_ERROR_BLKBASEMEM,
IDS_BK_ERROR_BLKCONFIG,
IDS_BK_ERROR_BLKCPUREGS,
IDS_BK_ERROR_BLKEXT16MEM,
IDS_BK_ERROR_BLKEXT32MEM,
IDS_BK_ERROR_BLKFRAMEDATA,
IDS_BK_ERROR_BLKMEMMAPMGR,
IDS_BK_ERROR_BLKNOTFOUND,
IDS_BK_ERROR_BLKPREVIEW,
IDS_BK_ERROR_BLKSMK512MEM,
IDS_BK_ERROR_BLKSYSREGS,
IDS_BK_ERROR_BLKTAPE,
IDS_BK_ERROR_DTABLKRDERR,
IDS_BK_ERROR_DTABLKWRERR,
IDS_BK_ERROR_HDRBLKRDERR,
IDS_BK_ERROR_HDRBLKWRERR,
IDS_BK_ERROR_MFBKBOARDERROR,
IDS_BK_ERROR_MFDBGTOOLBARERROR,
IDS_BK_ERROR_MFDOCKDEBUGVIEWERROR,
IDS_BK_ERROR_MFDOCKINGBARERROR,
IDS_BK_ERROR_MFDOCKMEMDUMPVIEWERROR,
IDS_BK_ERROR_MFDOCKOSCVIEWERROR,
IDS_BK_ERROR_MFDOCKREGDUMPVIEWERROR,
IDS_BK_ERROR_MFDOCKTAPECTRLVIEWERROR,
IDS_BK_ERROR_MFDOCKVKBDVIEWERROR,
IDS_BK_ERROR_MFMENUBARERROR,
IDS_BK_ERROR_MFPARSECMDLNERROR,
IDS_BK_ERROR_MFSNDTOOLBARERROR,
IDS_BK_ERROR_MFSTATUSBARERROR,
IDS_BK_ERROR_MFSTDTOOLBARERROR,
IDS_BK_ERROR_MFVCAPTOOLBARERROR,
IDS_BK_ERROR_MSFCORRUPTMSF,
IDS_BK_ERROR_MSFNOMSFFILE,
IDS_BK_ERROR_MSFOPENFILEERROR,
IDS_BK_ERROR_MSFPREVIEWBLK_DIB,
IDS_BK_ERROR_MSFPREVIEWBLK_OBJ,
IDS_BK_ERROR_MSFREADHEADERRROR,
IDS_BK_ERROR_MSFSEEKERROR,
IDS_BK_ERROR_MSFWRITEHEADERRROR,
IDS_BK_ERROR_NOCBKOSCSCRCREATE,
IDS_BK_ERROR_NOCBKOSCSCRINIT,
IDS_BK_ERROR_NOCDISASMDLGINIT,
IDS_BK_ERROR_NOCMEMDUMPDLGINIT,
IDS_BK_ERROR_NOCREATEVKBD,
IDS_BK_ERROR_NOCREGDUMPDLGINIT,
IDS_BK_ERROR_NOCSCREENCREATE,
IDS_BK_ERROR_NOCSCREENINIT,
IDS_BK_ERROR_NOCTAPECTRLDLGINIT,
IDS_BK_ERROR_NOERRORS,
IDS_BK_ERROR_NOTENMEMR,
IDS_BK_ERROR_SCRCOLORTABLEERR,
IDS_BK_ERROR_SCRDLLFUNCERR,
IDS_BK_ERROR_SCRDLLFUNCPTRERR,
IDS_BK_ERROR_SCRDLLINITERR,
IDS_BK_ERROR_SCRLLNOTLOAD,
IDS_BK_ERROR_WRONGFILE,
IDS_BUTTON_ADAPTIVEBWMODE_STR,
IDS_BUTTON_COLORMODE_STR,
IDS_BUTTON_LOAD_STR,
IDS_BUTTON_SAVE_STR,
IDS_BUTTON_SPRITE_STR,
IDS_CANT_OPEN_FILE_S,
IDS_COMMAND_PROMPT_INFO,
IDS_CONFAY_AY38910,
IDS_CONFAY_YM2149F,
IDS_CONFNAME_BK_0010_01_A16M,
IDS_CONFNAME_BK_0010_01_EXT32RAM,
IDS_CONFNAME_BK_0010_01_FDD,
IDS_CONFNAME_BK_0010_01_MSTD,
IDS_CONFNAME_BK_0010_01_SAMARA,
IDS_CONFNAME_BK_0010_01_SMK512,
IDS_CONFNAME_BK_0010_01,
IDS_CONFNAME_BK_0011_A16M,
IDS_CONFNAME_BK_0011_FDD,
IDS_CONFNAME_BK_0011M_A16M,
IDS_CONFNAME_BK_0011M_FDD,
IDS_CONFNAME_BK_0011M_SAMARA,
IDS_CONFNAME_BK_0011M_SMK512,
IDS_CONFNAME_BK_0011M,
IDS_CONFNAME_BK_0011_SAMARA,
IDS_CONFNAME_BK_0011_SMK512,
IDS_CONFNAME_BK_0011,
IDS_DDRAW_NEEDED,
IDS_DEBUG_ADDRESS,
IDS_DEBUG_ARGUMENTS,
IDS_DEBUG_COMMAND,
IDS_DEBUG_COMMENTS,
IDS_DEBUG_MARK,
IDS_DEBUG_WND,
IDS_DISK_FORMAT_ALTPRO,
IDS_DISK_FORMAT_ANDOS,
IDS_DISK_FORMAT_AODOS,
IDS_DISK_FORMAT_CSIDOS,
IDS_DISK_FORMAT_DIR,
IDS_DISK_FORMAT_DXDOS,
IDS_DISK_FORMAT_ERROR,
IDS_DISK_FORMAT_MICRODOS,
IDS_DISK_FORMAT_MKDOS,
IDS_DISK_FORMAT_MSDOS,
IDS_DISK_FORMAT_NCDOS,
IDS_DISK_FORMAT_NORD,
IDS_DISK_FORMAT_RT11,
IDS_DISK_FORMAT_SAMARA,
IDS_DISK_FORMAT_UNKNOWN,
IDS_DISK_NOACCESS,
IDS_DISK_SYSTEM,
IDS_EDIT_MENU,
IDS_EMUL_TITLE,
IDS_ERRMSF_OLD,
IDS_ERRMSF_WRONG,
IDS_ERRMSG_INTERNAL,
IDS_ERRMSG_NOTROM,
IDS_ERRMSG_ROM_NOT_DEFINED,
IDS_ERRMSG_ROM,
IDS_ERROR_CANTOPENFILE,
IDS_EXPLORER,
IDS_FILEEXT_BINARY,
IDS_FILEEXT_MEMORYSTATE,
IDS_FILEEXT_ROM,
IDS_FILEEXT_SCRIPT,
IDS_FILEEXT_TAPE,
IDS_FILEEXT_WAVE,
IDS_FILEFILTER_BIN,
IDS_FILEFILTER_BKHDDIMG,
IDS_FILEFILTER_BKIMG,
IDS_FILEFILTER_IMGLOAD,
IDS_FILEFILTER_MSF,
IDS_FILEFILTER_TAPE_LOAD,
IDS_FILEFILTER_TAPE_SAVE,
IDS_FILEFILTER_TAP_ONLY,
IDS_FILEFILTER_WAV,
IDS_INI_ADDR_DISASM,
IDS_INI_ADDR_DUMP,
IDS_INI_ASK_FOR_BREAK,
IDS_INI_AUTO_BEG_TAPE,
IDS_INI_AUTO_END_TAPE,
IDS_INI_AY1PANAL,
IDS_INI_AY1PANBL,
IDS_INI_AY1PANCL,
IDS_INI_AY1VOLA,
IDS_INI_AY1VOLB,
IDS_INI_AY1VOLC,
IDS_INI_AY8910,
IDS_INI_AY8910_FILTER,
IDS_INI_BIGBUTTONS,
IDS_INI_BK10_RE2_017_MONITOR,
IDS_INI_BK10_RE2_018_FOCAL,
IDS_INI_BK10_RE2_019_MSTD,
IDS_INI_BK10_RE2_106_BASIC1,
IDS_INI_BK10_RE2_107_BASIC2,
IDS_INI_BK10_RE2_108_BASIC3,
IDS_INI_BK11M_RE2_324_BOS,
IDS_INI_BK11M_RE2_325_EXT,
IDS_INI_BK11M_RE2_327_BASIC1,
IDS_INI_BK11M_RE2_328_BASIC2,
IDS_INI_BK11M_RE2_329_BASIC3,
IDS_INI_BK11M_RE2_330_MSTD,
IDS_INI_BK11_RE2_198_BASIC1,
IDS_INI_BK11_RE2_199_BASIC2,
IDS_INI_BK11_RE2_200_BASIC3,
IDS_INI_BK11_RE2_201_BOS,
IDS_INI_BK11_RE2_202_EXT,
IDS_INI_BK11_RE2_203_MSTD,
IDS_INI_BK11_RE_OPT_PG12_1,
IDS_INI_BK11_RE_OPT_PG12_2,
IDS_INI_BK11_RE_OPT_PG13_1,
IDS_INI_BK11_RE_OPT_PG13_2,
IDS_INI_BKJOY_A,
IDS_INI_BKJOY_ALTFIRE,
IDS_INI_BKJOY_B,
IDS_INI_BKJOY_DOWN,
IDS_INI_BKJOY_FIRE,
IDS_INI_BKJOY_LEFT,
IDS_INI_BKJOY_RIGHT,
IDS_INI_BKJOY_UP,
IDS_INI_BKKEYBOARD,
IDS_INI_BKMODEL,
IDS_INI_BLACK_WHITE,
IDS_INI_COLOR_MODE,
IDS_INI_COVOX,
IDS_INI_COVOX_FILTER,
IDS_INI_COVOX_STEREO,
IDS_INI_CPU_FREQUENCY,
IDS_INI_CPU_RUN_ADDR,
IDS_INI_DRIVEA,
IDS_INI_DRIVEB,
IDS_INI_DRIVEC,
IDS_INI_DRIVED,
IDS_INI_EMULATE_FDDIO,
IDS_INI_EMUL_LOAD_TAPE,
IDS_INI_EMUL_SAVE_TAPE,
IDS_INI_FDR253,
IDS_INI_FDR_A16M,
IDS_INI_FDR,
IDS_INI_FDR_SAMARA,
IDS_INI_FDR_SMK512,
IDS_INI_FFMPEGCMDLINE,
IDS_INI_FILENAME,
IDS_INI_FULL_SCREEN,
IDS_INI_HDD0,
IDS_INI_HDD1,
IDS_INI_ICLBLOCK,
IDS_INI_IMG_DIRECTORY,
IDS_INI_JOYSTICK,
IDS_INI_LONGBIN,
IDS_INI_LUMINOFOREMODE,
IDS_INI_MEM_DIRECTORY,
IDS_INI_MOUSEM,
IDS_INI_ORIG_SCRNSHOT_SIZE,
IDS_INI_OSCRENDER_TYPE,
IDS_INI_PALADAPTBW,
IDS_INI_PALBW,
IDS_INI_PALCOL00,
IDS_INI_PALCOL01,
IDS_INI_PALCOL02,
IDS_INI_PALCOL03,
IDS_INI_PALCOL04,
IDS_INI_PALCOL05,
IDS_INI_PALCOL06,
IDS_INI_PALCOL07,
IDS_INI_PALCOL08,
IDS_INI_PALCOL09,
IDS_INI_PALCOL10,
IDS_INI_PALCOL11,
IDS_INI_PALCOL12,
IDS_INI_PALCOL13,
IDS_INI_PALCOL14,
IDS_INI_PALCOL15,
IDS_INI_PAUSE_CPU,
IDS_INI_PROGRAM_DIRECTORY,
IDS_INI_REGSDUMP_INTERVAL,
IDS_INI_ROM_DIRECTORY,
IDS_INI_SAVES_DEFAULT,
IDS_INI_SAVES_DIRECTORY,
IDS_INI_SCRIPTS_DIRECTORY,
IDS_INI_SCRRENDER_TYPE,
IDS_INI_SECTIONNAME_DIRECTORIES,
IDS_INI_SECTIONNAME_DRIVES,
IDS_INI_SECTIONNAME_JOYSTICK,
IDS_INI_SECTIONNAME_MAIN,
IDS_INI_SECTIONNAME_OPTIONS,
IDS_INI_SECTIONNAME_PALETTES,
IDS_INI_SECTIONNAME_PARAMETERS,
IDS_INI_SECTIONNAME_ROMMODULES,
IDS_INI_SHOW_PERFORMANCE_STRING,
IDS_INI_SMOOTHING,
IDS_INI_SOUNDCHIPFREQ,
IDS_INI_SOUNDCHIPMODEL,
IDS_INI_SOUND_SAMPLE_RATE,
IDS_INI_SOUNDVOLUME,
IDS_INI_SPEAKER_FILTER,
IDS_INI_SPEAKER,
IDS_INI_SSHOT_DIRECTORY,
IDS_INI_SSHOT_NUM,
IDS_INI_TAPES_DIRECTORY,
IDS_INI_TOOLS_DIRECTORY,
IDS_INI_VKBD_TYPE,
IDS_JOYEDIT_PRESSKEY,
IDS_JOYEDIT_UNKNOWN,
IDS_LOADTAPE_ERRORVALUE,
IDS_LOADTAPE_TYPE_BIN,
IDS_LOADTAPE_TYPE_MSF,
IDS_LOADTAPE_TYPE_UNKNOWN,
IDS_LOADTAPE_TYPE_WAVE,
IDS_MEMDUMP_BYTE,
IDS_MEMDUMP_WND,
IDS_MEMDUMP_WORD,
IDS_MEMORY_177130IN,
IDS_MEMORY_177130OUT,
IDS_MEMORY_177132IN,
IDS_MEMORY_177132OUT,
IDS_MEMORY_177660,
IDS_MEMORY_177662IN,
IDS_MEMORY_177662OUT,
IDS_MEMORY_177664,
IDS_MEMORY_177700,
IDS_MEMORY_177702,
IDS_MEMORY_177704,
IDS_MEMORY_177706,
IDS_MEMORY_177710,
IDS_MEMORY_177712,
IDS_MEMORY_177714IN,
IDS_MEMORY_177714OUT,
IDS_MEMORY_177716IN,
IDS_MEMORY_177716OUT_MEM,
IDS_MEMORY_177716OUT_TAP,
IDS_MEMORY_PC,
IDS_MEMORY_PSW,
IDS_MEMORY_R0,
IDS_MEMORY_R1,
IDS_MEMORY_R2,
IDS_MEMORY_R3,
IDS_MEMORY_R4,
IDS_MEMORY_R5,
IDS_MEMORY_SP,
IDS_MENU_DEBUG_BREAK,
IDS_MENU_DEBUG_CONTINUE,
IDS_MSGBOX_CAPTION,
IDS_MSGBOX_ERRMSG,
IDS_OSCILLATOR_WND,
IDS_OUTPUT_WND,
IDS_PALETTE_BWPRESET0,
IDS_PALETTE_BWPRESET1,
IDS_PALETTE_BWPRESET2,
IDS_PALETTE_BWPRESET3,
IDS_PALETTE_MORE,
IDS_PRINT_PAGES,
IDS_PRINT_TITLE,
IDS_REGDUMPCPU_DEC,
IDS_REGDUMPCPU_HEX,
IDS_REGDUMP_CPU_WND,
IDS_REGDUMP_FDD_WND,
IDS_RENDER_D2D,
IDS_RENDER_D3D,
IDS_RENDER_DIB,
IDS_RENDER_GDI,
IDS_RENDER_OGL,
IDS_SCREENVIEW_WND,
IDS_SETT_KBDVIEW0,
IDS_SETT_KBDVIEW1,
IDS_SETT_TABTITLE0,
IDS_SETT_TABTITLE1,
IDS_SETT_TABTITLE2,
IDS_SNDCHPFRQ_1500000,
IDS_SNDCHPFRQ_1714286,
IDS_SNDCHPFRQ_1774400,
IDS_SNDCHPFRQ_1789772,
IDS_STATUS_PANE1,
IDS_STATUS_PANE2,
IDS_TAPECTRL_WND,
IDS_TAPEMANAGER_BROWSESELECT,
IDS_TAPEMNGR_ADDR,
IDS_TAPEMNGR_BADCRC_SAVE,
IDS_TAPEMNGR_CAPTURE_ERROR,
IDS_TAPEMNGR_CRC,
IDS_TAPEMNGR_CRC_ERROR,
IDS_TAPEMNGR_CRC_FAIL,
IDS_TAPEMNGR_CRC_OK,
IDS_TAPEMNGR_END_OF_FILE,
IDS_TAPEMNGR_INPUT_DEVS,
IDS_TAPEMNGR_LENGTH,
IDS_TAPEMNGR_NAME_FORMAT,
IDS_TAPEMNGR_NAME,
IDS_TAPEMNGR_NO_INPUTDEVS,
IDS_TAPEMNGR_PATH,
IDS_TAPEMNGR_SAVE_AS,
IDS_TAPEMNGR_SAVE_ERROR,
IDS_TAPEMNGR_SAVE_MESSAGE,
IDS_TAPEMNGR_SAVE_SUCCEED,
IDS_TAPEMNGR_TIME,
IDS_TAPEMNGR_TYPE,
IDS_TAPEMNGR_UNKNOWN,
IDS_TAPEMNGR_WAVELENGTH,
IDS_TOOLBAR_CUSTOMIZE,
IDS_TOOLBAR_DEBUG,
IDS_TOOLBAR_SOUND,
IDS_TOOLBAR_STANDARD,
IDS_TOOLBAR_VCAPT,
IDS_VKBD_WND
};

extern const char *res_str[];
/*
#define IDS_ALTPRO_CODE  "Код стр."
#define IDS_ALTPRO_MODE  "Режим"
#define IDS_BK_ERROR_BLKBASEMEM11  "Base memory 11"
#define IDS_BK_ERROR_BLKBASEMEM  "Base memory"
#define IDS_BK_ERROR_BLKCONFIG  "Config"
#define IDS_BK_ERROR_BLKCPUREGS  "CPU registers"
#define IDS_BK_ERROR_BLKEXT16MEM  "Ext.16kb memory"
#define IDS_BK_ERROR_BLKEXT32MEM  "Ext.32kb memory"
#define IDS_BK_ERROR_BLKFRAMEDATA  "Frame Data"
#define IDS_BK_ERROR_BLKMEMMAPMGR  "Memory map manager"
#define IDS_BK_ERROR_BLKNOTFOUND  "Не найден блок '%s'."
#define IDS_BK_ERROR_BLKPREVIEW  "Preview"
#define IDS_BK_ERROR_BLKSMK512MEM  "SMK512 memory"
#define IDS_BK_ERROR_BLKSYSREGS  "System registers"
#define IDS_BK_ERROR_BLKTAPE  "Tape"
#define IDS_BK_ERROR_DTABLKRDERR  "Ошибка чтения данных блока '%s'."
#define IDS_BK_ERROR_DTABLKWRERR  "Ошибка записи данных блока '%s'."
#define IDS_BK_ERROR_HDRBLKRDERR  "Ошибка чтения заголовка блока '%s'."
#define IDS_BK_ERROR_HDRBLKWRERR  "Ошибка записи заголовка блока '%s'."
#define IDS_BK_ERROR_MFBKBOARDERROR  "Ошибка создания текущей конфигурации БК."
#define IDS_BK_ERROR_MFDBGTOOLBARERROR  
#define IDS_BK_ERROR_MFDOCKDEBUGVIEWERROR  
#define IDS_BK_ERROR_MFDOCKINGBARERROR  "Не удалось создать закрепляемые окна."
#define IDS_BK_ERROR_MFDOCKMEMDUMPVIEWERROR  
#define IDS_BK_ERROR_MFDOCKOSCVIEWERROR  
#define IDS_BK_ERROR_MFDOCKREGDUMPVIEWERROR  
#define IDS_BK_ERROR_MFDOCKTAPECTRLVIEWERROR  
#define IDS_BK_ERROR_MFDOCKVKBDVIEWERROR  
#define IDS_BK_ERROR_MFMENUBARERROR  "Не удалось создать строку меню."
#define IDS_BK_ERROR_MFPARSECMDLNERROR  "Ошибка разбора командной строки."
#define IDS_BK_ERROR_MFSNDTOOLBARERROR  
#define IDS_BK_ERROR_MFSTATUSBARERROR  "Не удалось создать строку состояния."
#define IDS_BK_ERROR_MFSTDTOOLBARERROR  
#define IDS_BK_ERROR_MFVCAPTOOLBARERROR  
#define IDS_BK_ERROR_MSFCORRUPTMSF  "Структура MSF файла повреждена."
#define IDS_BK_ERROR_MSFNOMSFFILE  "Это не MSF файл."
#define IDS_BK_ERROR_MSFOPENFILEERROR  "Ошибка открытия файла."
#define IDS_BK_ERROR_MSFPREVIEWBLK_DIB  
#define IDS_BK_ERROR_MSFPREVIEWBLK_OBJ  
#define IDS_BK_ERROR_MSFREADHEADERRROR  "Ошибка чтения заголовка MSF файла."
#define IDS_BK_ERROR_MSFSEEKERROR  "Ошибка позиционирования в файле."
#define IDS_BK_ERROR_MSFWRITEHEADERRROR  "Ошибка записи заголовка MSF файла."
#define IDS_BK_ERROR_NOCBKOSCSCRCREATE  "Не удалось создать объект 'Осциллограф'."
#define IDS_BK_ERROR_NOCBKOSCSCRINIT  
#define IDS_BK_ERROR_NOCDISASMDLGINIT  
#define IDS_BK_ERROR_NOCMEMDUMPDLGINIT  
#define IDS_BK_ERROR_NOCREATEVKBD  "Окно CBKKbdButn не захотело создаваться."
#define IDS_BK_ERROR_NOCREGDUMPDLGINIT  
#define IDS_BK_ERROR_NOCSCREENCREATE  "Не удалось создать объект 'Экран'."
#define IDS_BK_ERROR_NOCSCREENINIT  "Не удалось инициализировать объект 'Экран'."
#define IDS_BK_ERROR_NOCTAPECTRLDLGINIT  
#define IDS_BK_ERROR_NOERRORS  "Успешно."
#define IDS_BK_ERROR_NOTENMEMR  "Недостаточно памяти."
#define IDS_BK_ERROR_SCRCOLORTABLEERR  
#define IDS_BK_ERROR_SCRDLLFUNCERR  
#define IDS_BK_ERROR_SCRDLLFUNCPTRERR  
#define IDS_BK_ERROR_SCRDLLINITERR  
#define IDS_BK_ERROR_SCRLLNOTLOAD  "Не найдена DLL библиотека "
#define IDS_BK_ERROR_WRONGFILE  "Не тот файл подсовываете. Повторим?"
#define IDS_BUTTON_ADAPTIVEBWMODE_STR  
#define IDS_BUTTON_COLORMODE_STR  "Режим отображения цветной / чёрно-белый"
#define IDS_BUTTON_LOAD_STR  "Загрузить битмап (BMP, проч.) в эту страницу"
#define IDS_BUTTON_SAVE_STR  "Сохранить страницу в битмап (BMP, проч.)"
#define IDS_BUTTON_SPRITE_STR  "Открыть отдельно окно просмотра страницы"
#define IDS_CANT_OPEN_FILE_S  "Невозможно открыть файл ""%s"" !\nОстановить CPU ? "
#define IDS_COMMAND_PROMPT_INFO  "\nКоманды:\n - /h или /? - эта справка.\n - /b путь - загрузка bin файла по заданному пути.\n - /m путь - загрузка msf файла по заданному пути.\n - /t путь - загрузка tape файла по заданному пути.\n - /s скрипт - запуск файла скрипта"
#define IDS_CONFAY_AY38910  "AY-3-8910"
#define IDS_CONFAY_YM2149F  "YM2149F"
#define IDS_CONFNAME_BK_0010_01_A16M  "БК0010-01 + контроллер A16M"
#define IDS_CONFNAME_BK_0010_01_EXT32RAM  "БК0010-01 + доп. 32кб ОЗУ"
#define IDS_CONFNAME_BK_0010_01_FDD  "БК0010-01 + стандартный КНГМД"
#define IDS_CONFNAME_BK_0010_01_MSTD  "БК0010-01 + блок Фокал-МСТД"
#define IDS_CONFNAME_BK_0010_01_SAMARA  "БК0010-01 + контроллер Samara"
#define IDS_CONFNAME_BK_0010_01_SMK512  "БК0010-01 + контроллер SMK512"
#define IDS_CONFNAME_BK_0010_01  "БК0010-01"
#define IDS_CONFNAME_BK_0011_A16M  "БК0011 + контроллер A16M"
#define IDS_CONFNAME_BK_0011_FDD  "БК0011 + стандартный КНГМД"
#define IDS_CONFNAME_BK_0011M_A16M  "БК0011М + контроллер A16M"
#define IDS_CONFNAME_BK_0011M_FDD  "БК0011М + стандартный КНГМД"
#define IDS_CONFNAME_BK_0011M_SAMARA  "БК0011М + контроллер Samara"
#define IDS_CONFNAME_BK_0011M_SMK512  "БК0011М + контроллер SMK512"
#define IDS_CONFNAME_BK_0011M  "БК0011М + МСТД"
#define IDS_CONFNAME_BK_0011_SAMARA  "БК0011 + контроллер Samara"
#define IDS_CONFNAME_BK_0011_SMK512  "БК0011 + контроллер SMK512"
#define IDS_CONFNAME_BK_0011  "БК0011 + МСТД"
#define IDS_DDRAW_NEEDED  "Необходим DirectX 9.0 или выше!"
#define IDS_DEBUG_ADDRESS  "Адрес"
#define IDS_DEBUG_ARGUMENTS  "Аргументы"
#define IDS_DEBUG_COMMAND  "Команда"
#define IDS_DEBUG_COMMENTS  "Машинный код"
#define IDS_DEBUG_MARK  " "
#define IDS_DEBUG_WND  "Окно отладки"
#define IDS_DISK_FORMAT_ALTPRO  "HDD AltPro"
#define IDS_DISK_FORMAT_ANDOS  "ANDOS"
#define IDS_DISK_FORMAT_AODOS  "AODOS"
#define IDS_DISK_FORMAT_CSIDOS  "CSIDOS3"
#define IDS_DISK_FORMAT_DIR  "Директория."
#define IDS_DISK_FORMAT_DXDOS  "DX-DOS"
#define IDS_DISK_FORMAT_ERROR  "Ошибка чтения"
#define IDS_DISK_FORMAT_MICRODOS  "MicroDOS"
#define IDS_DISK_FORMAT_MKDOS  "MKDOS"
#define IDS_DISK_FORMAT_MSDOS  "MS-DOS"
#define IDS_DISK_FORMAT_NCDOS  "HC-DOS"
#define IDS_DISK_FORMAT_NORD  "NORD"
#define IDS_DISK_FORMAT_RT11  "RT-11"
#define IDS_DISK_FORMAT_SAMARA  "HDD Samara"
#define IDS_DISK_FORMAT_UNKNOWN  "Неизвестный формат"
#define IDS_DISK_NOACCESS  "Нет доступа."
#define IDS_DISK_SYSTEM  "Системный"
#define IDS_EDIT_MENU  "Редактировать"
#define IDS_EMUL_TITLE  "Эмулятор БК для Windows"
#define IDS_ERRMSF_OLD  "MSF старых версий, к сожалению, не поддерживается."
#define IDS_ERRMSF_WRONG  "Не поддерживаемый или повреждённый формат MSF файла."
#define IDS_ERRMSG_INTERNAL  "Возможно, внутренняя ошибка эмулятора. Эмулятор остановлен. "
#define IDS_ERRMSG_NOTROM  "Внимание, отсутствует ROM модуль "
#define IDS_ERRMSG_ROM_NOT_DEFINED  "Внимание, не задан ROM модуль для "
#define IDS_ERRMSG_ROM  "Исключительная ситуация по адресу: %06o.\nНеверная инструкция по адресу: %06o.\nВозможная причина: ""%s"".\n""Прервать"" - отладка.\n""Повтор"" - продолжить выполнение. \n""Пропустить"" - больше не показывать это сообщение."
#define IDS_ERROR_CANTOPENFILE  "Невозможно открыть файл."
#define IDS_EXPLORER  "Проводник"
#define IDS_FILEEXT_BINARY  ".bin"
#define IDS_FILEEXT_MEMORYSTATE  ".msf"
#define IDS_FILEEXT_ROM  ".rom"
#define IDS_FILEEXT_SCRIPT  ".bkscript"
#define IDS_FILEEXT_TAPE  ".tap"
#define IDS_FILEEXT_WAVE  ".wav"
#define IDS_FILEFILTER_BIN  "Бинарный файл эмулятора (*.bin);; Все файлы (*.*)"
#define IDS_FILEFILTER_BKHDDIMG  "Образ жёсткого диска БК (*.hdi);; Все файлы (*.*)"
#define IDS_FILEFILTER_BKIMG  "Образ диска БК (*.img *.bkd *.dsk);; Все файлы (*.*)"
#define IDS_FILEFILTER_IMGLOAD  "Все файлы изображений"
#define IDS_FILEFILTER_MSF  "Файл состояния памяти (*.msf);; Все файлы (*.*)"
#define IDS_FILEFILTER_TAPE_LOAD  
#define IDS_FILEFILTER_TAPE_SAVE  
#define IDS_FILEFILTER_TAP_ONLY  "Файл кассеты БК (*.tap);; Все файлы (*.*)"
#define IDS_FILEFILTER_WAV  "Microsoft Wave (*.wav) ;;Все файлы (*.*)"
#define IDS_INI_ADDR_DISASM  "Start disasm address"
#define IDS_INI_ADDR_DUMP  "Start dump address"
#define IDS_INI_ASK_FOR_BREAK  "Show dialog on access violation"
#define IDS_INI_AUTO_BEG_TAPE  "Tape auto start detection"
#define IDS_INI_AUTO_END_TAPE  "Tape auto end detection"
#define IDS_INI_AY1PANAL  "AY Channel A Pan Left"
#define IDS_INI_AY1PANBL  "AY Channel B Pan Left"
#define IDS_INI_AY1PANCL  "AY Channel C Pan Left"
#define IDS_INI_AY1VOLA  "AY Channel A Volume"
#define IDS_INI_AY1VOLB  "AY Channel B Volume"
#define IDS_INI_AY1VOLC  "AY Channel C Volume"
#define IDS_INI_AY8910  "AY8910 enabled"
#define IDS_INI_AY8910_FILTER  "AY8910 filter enabled"
#define IDS_INI_BIGBUTTONS  "Big buttons for Instrumental Panel"
#define IDS_INI_BK10_RE2_017_MONITOR  "BK10 Monitor"
#define IDS_INI_BK10_RE2_018_FOCAL  "BK10 Focal"
#define IDS_INI_BK10_RE2_019_MSTD  "BK10 MSTD"
#define IDS_INI_BK10_RE2_106_BASIC1  "BK10 Basic1"
#define IDS_INI_BK10_RE2_107_BASIC2  "BK10 Basic2"
#define IDS_INI_BK10_RE2_108_BASIC3  "BK10 Basic3"
#define IDS_INI_BK11M_RE2_324_BOS  "BK11M Monitor BOS"
#define IDS_INI_BK11M_RE2_325_EXT  "BK11M Monitor EXT"
#define IDS_INI_BK11M_RE2_327_BASIC1  "BK11M Basic1"
#define IDS_INI_BK11M_RE2_328_BASIC2  "BK11M Basic2"
#define IDS_INI_BK11M_RE2_329_BASIC3  "BK11M Basic3"
#define IDS_INI_BK11M_RE2_330_MSTD  "BK11M MSTD"
#define IDS_INI_BK11_RE2_198_BASIC1  "BK11 Basic1"
#define IDS_INI_BK11_RE2_199_BASIC2  "BK11 Basic2"
#define IDS_INI_BK11_RE2_200_BASIC3  "BK11 Basic3"
#define IDS_INI_BK11_RE2_201_BOS  "BK11 Monitor BOS"
#define IDS_INI_BK11_RE2_202_EXT  "BK11 Monitor EXT"
#define IDS_INI_BK11_RE2_203_MSTD  "BK11 MSTD"
#define IDS_INI_BK11_RE_OPT_PG12_1  "BK11 Optional Page12.1"
#define IDS_INI_BK11_RE_OPT_PG12_2  "BK11 Optional Page12.2"
#define IDS_INI_BK11_RE_OPT_PG13_1  "BK11 Optional Page13.1"
#define IDS_INI_BK11_RE_OPT_PG13_2  "BK11 Optional Page13.2"
#define IDS_INI_BKJOY_A  "Joystick A Button"
#define IDS_INI_BKJOY_ALTFIRE  "Joystick AltFire"
#define IDS_INI_BKJOY_B  "Joystick B Button"
#define IDS_INI_BKJOY_DOWN  "Joystick Down"
#define IDS_INI_BKJOY_FIRE  "Joystick Fire"
#define IDS_INI_BKJOY_LEFT  "Joystick Left"
#define IDS_INI_BKJOY_RIGHT  "Joystick Right"
#define IDS_INI_BKJOY_UP  "Joystick Up"
#define IDS_INI_BKKEYBOARD  "Emulate BK keyboard"
#define IDS_INI_BKMODEL  "BK model"
#define IDS_INI_BLACK_WHITE  "Adapt black & white mode"
#define IDS_INI_COLOR_MODE  "Run in color mode"
#define IDS_INI_COVOX  "Covox enabled"
#define IDS_INI_COVOX_FILTER  "Covox filter enabled"
#define IDS_INI_COVOX_STEREO  "Stereo covox"
#define IDS_INI_CPU_FREQUENCY  "CPU frequency"
#define IDS_INI_CPU_RUN_ADDR  "CPU start address"
#define IDS_INI_DRIVEA  "Drive A:"
#define IDS_INI_DRIVEB  "Drive B:"
#define IDS_INI_DRIVEC  "Drive C:"
#define IDS_INI_DRIVED  "Drive D:"
#define IDS_INI_EMULATE_FDDIO  "Emulate FDD IO"
#define IDS_INI_EMUL_LOAD_TAPE  "Emulate load tape operations"
#define IDS_INI_EMUL_SAVE_TAPE  "Emulate save tape operations"
#define IDS_INI_FDR253  "FDD KNGMD253"
#define IDS_INI_FDR_A16M  "FDD A16M"
#define IDS_INI_FDR  "FDD KNGMD"
#define IDS_INI_FDR_SAMARA  "FDD Samara"
#define IDS_INI_FDR_SMK512  "FDD SMK512"
#define IDS_INI_FFMPEGCMDLINE  "FFMPEG Cmd Line"
#define IDS_INI_FILENAME  "bk.ini"
#define IDS_INI_FULL_SCREEN  "Run in fullscreen mode"
#define IDS_INI_HDD0  "HDD0"
#define IDS_INI_HDD1  "HDD1"
#define IDS_INI_ICLBLOCK  "IC Load Block"
#define IDS_INI_IMG_DIRECTORY  "IMG directory"
#define IDS_INI_JOYSTICK  "Joystick enabled"
#define IDS_INI_LONGBIN  "Use long Bin format"
#define IDS_INI_LUMINOFOREMODE  "Screen fade emulation"
#define IDS_INI_MEM_DIRECTORY  "Memory directory"
#define IDS_INI_MOUSEM  "Emulate Mouse"
#define IDS_INI_ORIG_SCRNSHOT_SIZE  "Origin screenshot size"
#define IDS_INI_OSCRENDER_TYPE  "Oscilloscope render type"
#define IDS_INI_PALADAPTBW  "Adapt BW Palette"
#define IDS_INI_PALBW  "BW Palette"
#define IDS_INI_PALCOL00  "Color Palette 00"
#define IDS_INI_PALCOL01  "Color Palette 01"
#define IDS_INI_PALCOL02  "Color Palette 02"
#define IDS_INI_PALCOL03  "Color Palette 03"
#define IDS_INI_PALCOL04  "Color Palette 04"
#define IDS_INI_PALCOL05  "Color Palette 05"
#define IDS_INI_PALCOL06  "Color Palette 06"
#define IDS_INI_PALCOL07  "Color Palette 07"
#define IDS_INI_PALCOL08  "Color Palette 08"
#define IDS_INI_PALCOL09  "Color Palette 09"
#define IDS_INI_PALCOL10  "Color Palette 10"
#define IDS_INI_PALCOL11  "Color Palette 11"
#define IDS_INI_PALCOL12  "Color Palette 12"
#define IDS_INI_PALCOL13  "Color Palette 13"
#define IDS_INI_PALCOL14  "Color Palette 14"
#define IDS_INI_PALCOL15  "Color Palette 15"
#define IDS_INI_PAUSE_CPU  "Pause CPU after emulator start"
#define IDS_INI_PROGRAM_DIRECTORY  "Program directory"
#define IDS_INI_REGSDUMP_INTERVAL  "Registers dump interval"
#define IDS_INI_ROM_DIRECTORY  "Rom directory"
#define IDS_INI_SAVES_DEFAULT  "Use Saves directory as default"
#define IDS_INI_SAVES_DIRECTORY  "User Saves directory"
#define IDS_INI_SCRIPTS_DIRECTORY  "Scripts directory"
#define IDS_INI_SCRRENDER_TYPE  "Screen render type"
#define IDS_INI_SECTIONNAME_DIRECTORIES  "Directories"
#define IDS_INI_SECTIONNAME_DRIVES  "Drives"
#define IDS_INI_SECTIONNAME_JOYSTICK  "Joystick Parameters"
#define IDS_INI_SECTIONNAME_MAIN  "Main"
#define IDS_INI_SECTIONNAME_OPTIONS  "Options"
#define IDS_INI_SECTIONNAME_PALETTES  "Palettes"
#define IDS_INI_SECTIONNAME_PARAMETERS  "Parameters"
#define IDS_INI_SECTIONNAME_ROMMODULES  "Rom modules"
#define IDS_INI_SHOW_PERFORMANCE_STRING  "Show performance string"
#define IDS_INI_SMOOTHING  "Smoothing Screen"
#define IDS_INI_SOUNDCHIPFREQ  "SoundChip Frequency"
#define IDS_INI_SOUNDCHIPMODEL  "SoundChip Model"
#define IDS_INI_SOUND_SAMPLE_RATE  "Sound Sample Rate"
#define IDS_INI_SOUNDVOLUME  "Sound volume"
#define IDS_INI_SPEAKER_FILTER  "Speaker filter enabled"
#define IDS_INI_SPEAKER  "Speaker enabled"
#define IDS_INI_SSHOT_DIRECTORY  "Screenshots directory"
#define IDS_INI_SSHOT_NUM  "Screenshot number"
#define IDS_INI_TAPES_DIRECTORY  "Tapes directory"
#define IDS_INI_TOOLS_DIRECTORY  "Tools directory"
#define IDS_INI_VKBD_TYPE  "Virtual Keyboard Type"
#define IDS_JOYEDIT_PRESSKEY  "press key..."
#define IDS_JOYEDIT_UNKNOWN  "unknown"
#define IDS_LOADTAPE_ERRORVALUE  "Ошибка"
#define IDS_LOADTAPE_TYPE_BIN  "BINARY"
#define IDS_LOADTAPE_TYPE_MSF  "MSF TAPE"
#define IDS_LOADTAPE_TYPE_UNKNOWN  "UNKNOWN"
#define IDS_LOADTAPE_TYPE_WAVE  "WAVE"
#define IDS_MEMDUMP_BYTE  "BYTE"
#define IDS_MEMDUMP_WND  "Дамп памяти"
#define IDS_MEMDUMP_WORD  "WORD"
#define IDS_MEMORY_177130IN  "177130|Чт"
#define IDS_MEMORY_177130OUT  "177130|Зп"
#define IDS_MEMORY_177132IN  "177132|Чт"
#define IDS_MEMORY_177132OUT  "177132|Зп"
#define IDS_MEMORY_177660  "177660"
#define IDS_MEMORY_177662IN  "177662|Чт"
#define IDS_MEMORY_177662OUT  "177662|Зп"
#define IDS_MEMORY_177664  "177664"
#define IDS_MEMORY_177700  "177700"
#define IDS_MEMORY_177702  "177702"
#define IDS_MEMORY_177704  "177704"
#define IDS_MEMORY_177706  "177706"
#define IDS_MEMORY_177710  "177710"
#define IDS_MEMORY_177712  "177712"
#define IDS_MEMORY_177714IN  "177714|Чт"
#define IDS_MEMORY_177714OUT  "177714|Зп"
#define IDS_MEMORY_177716IN  "177716|Чт"
#define IDS_MEMORY_177716OUT_MEM  "177716|ЗпП"
#define IDS_MEMORY_177716OUT_TAP  "177716|ЗпЗ"
#define IDS_MEMORY_PC  "PC"
#define IDS_MEMORY_PSW  "PSW"
#define IDS_MEMORY_R0  "R0"
#define IDS_MEMORY_R1  "R1"
#define IDS_MEMORY_R2  "R2"
#define IDS_MEMORY_R3  "R3"
#define IDS_MEMORY_R4  "R4"
#define IDS_MEMORY_R5  "R5"
#define IDS_MEMORY_SP  "SP"
#define IDS_MENU_DEBUG_BREAK  "Стоп"
#define IDS_MENU_DEBUG_CONTINUE  "Продолжить"
#define IDS_MSGBOX_CAPTION  "BKEmu сообщает"
#define IDS_MSGBOX_ERRMSG  "Неверный ID сообщения: %d."
#define IDS_OSCILLATOR_WND  "Осциллограф"
#define IDS_OUTPUT_WND  "Консоль"
#define IDS_PALETTE_BWPRESET0  "Градации Серого"
#define IDS_PALETTE_BWPRESET1  "Россыпь Зелёного"
#define IDS_PALETTE_BWPRESET2  "Искры Синего"
#define IDS_PALETTE_BWPRESET3  "Оттенки Янтаря"
#define IDS_PALETTE_MORE  "Ещё..."
#define IDS_PRINT_PAGES  "Page %3i of %3i "
#define IDS_PRINT_TITLE  "Electronica BK"
#define IDS_REGDUMPCPU_DEC  "DEC"
#define IDS_REGDUMPCPU_HEX  "HEX"
#define IDS_REGDUMP_CPU_WND  "Дамп регистров CPU"
#define IDS_REGDUMP_FDD_WND  "Дамп регистров FDD"
#define IDS_RENDER_D2D  "Direct2D"
#define IDS_RENDER_D3D  "Direct3D"
#define IDS_RENDER_DIB  "VFW"
#define IDS_RENDER_GDI  "GDI+"
#define IDS_RENDER_OGL  "OpenGL"
#define IDS_SCREENVIEW_WND  "Экран"
#define IDS_SETT_KBDVIEW0  "Кнопочная"
#define IDS_SETT_KBDVIEW1  "Плёночная"
#define IDS_SETT_TABTITLE0  " Приводы и параметры"
#define IDS_SETT_TABTITLE1  " Интерфейс эмулятора "
#define IDS_SETT_TABTITLE2  " Текущие опции"
#define IDS_SNDCHPFRQ_1500000  "1500000 (12МГц/8)"
#define IDS_SNDCHPFRQ_1714286  "1714286 (12МГц/7)"
#define IDS_SNDCHPFRQ_1774400  "1774400 (ZX)"
#define IDS_SNDCHPFRQ_1789772  "1789772 (12МГц/6.7)"
#define IDS_STATUS_PANE1  "Панель 1"
#define IDS_STATUS_PANE2  "Панель 2"
#define IDS_TAPECTRL_WND  "Управление чтением / записью лент"
#define IDS_TAPEMANAGER_BROWSESELECT  "Выберите папку для копирования..."
#define IDS_TAPEMNGR_ADDR  "Адрес"
#define IDS_TAPEMNGR_BADCRC_SAVE  
#define IDS_TAPEMNGR_CAPTURE_ERROR  
#define IDS_TAPEMNGR_CRC  "CRC"
#define IDS_TAPEMNGR_CRC_ERROR  "Ошибка контрольной суммы в файле %s."
#define IDS_TAPEMNGR_CRC_FAIL  "err"
#define IDS_TAPEMNGR_CRC_OK  "ок"
#define IDS_TAPEMNGR_END_OF_FILE  "\tОбнаружен конец файла"
#define IDS_TAPEMNGR_INPUT_DEVS  "Устройство ввода по умолчанию"
#define IDS_TAPEMNGR_LENGTH  "Длина"
#define IDS_TAPEMNGR_NAME_FORMAT  "%s : адрес = %o, длина = %o"
#define IDS_TAPEMNGR_NAME  "Имя"
#define IDS_TAPEMNGR_NO_INPUTDEVS  "Устройства ввода отсутствуют."
#define IDS_TAPEMNGR_PATH  "Путь файла"
#define IDS_TAPEMNGR_SAVE_AS  "Сохранить как..."
#define IDS_TAPEMNGR_SAVE_ERROR  "\t'%s' - ошибка. Невозможно записать файл."
#define IDS_TAPEMNGR_SAVE_MESSAGE  "Сохранить выделение:"
#define IDS_TAPEMNGR_SAVE_SUCCEED  "\t'%s' - успешно."
#define IDS_TAPEMNGR_TIME  "Время"
#define IDS_TAPEMNGR_TYPE  "Тип"
#define IDS_TAPEMNGR_UNKNOWN  "неизвестно"
#define IDS_TAPEMNGR_WAVELENGTH  "Длина Wave"
#define IDS_TOOLBAR_CUSTOMIZE  "Настройка..."
#define IDS_TOOLBAR_DEBUG  "Отладка"
#define IDS_TOOLBAR_SOUND  "Звук"
#define IDS_TOOLBAR_STANDARD  "Стандартная"
#define IDS_TOOLBAR_VCAPT  "Захват видео"
#define IDS_VKBD_WND  "Виртуальная клавиатура"
*/

#define MAKEINTRESOURCE(a) res_str[a]
