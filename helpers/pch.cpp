// pch.cpp: файл исходного кода, соответствующий предварительно скомпилированному заголовочному файлу

#include "pch.h"
#include <QThread>
#include <time.h>


// При использовании предварительно скомпилированных заголовочных файлов необходим следующий файл исходного кода для выполнения сборки.

void Sleep(uint mSec) {
    QThread::msleep(mSec);
}

unsigned int GetTickCount()
{
    struct timespec ts;
    unsigned theTick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    theTick  = ts.tv_nsec / 1000000;
    theTick += ts.tv_sec * 1000;
    return theTick;
}

void splitpath(const CString &str,  CString &f_path, CString &f_name, CString &f_ext)
{

    int pos = str.lastIndexOf('/');

    if (-1 == pos) {
        f_path = "";
        f_name = str;
    } else {
        f_path = str.left(pos);
        f_name = str.right(str.size()-(pos+1));
    }

    // Extension
    int idx;

    idx = f_name.lastIndexOf('.');
    if(-1 == idx)
    {
        f_ext = "";
    }
    else
    {
        f_ext = f_name.right(f_name.size()-(idx+1));
        f_name = f_name.left(idx);
    }
}


const char *res_str[] = {
 "Код стр.",
 "Режим",
 "Base memory 11",
 "Base memory",
 "Config",
 "CPU registers",
 "Ext.16kb memory",
 "Ext.32kb memory",
 "Frame Data",
 "Memory map manager",
 "Не найден блок '%s'.",
 "Preview",
 "SMK512 memory",
 "System registers",
 "Tape",
 "Ошибка чтения данных блока '%s'.",
 "Ошибка записи данных блока '%s'.",
 "Ошибка чтения заголовка блока '%s'.",
 "Ошибка записи заголовка блока '%s'.",
 "Ошибка создания текущей конфигурации БК.",
 "",
 "",
 "Не удалось создать закрепляемые окна.",
 "",
 "",
 "",
 "",
 "",
 "Не удалось создать строку меню.",
 "Ошибка разбора командной строки.",
 "",
 "Не удалось создать строку состояния.",
 "",
 "",
 "Структура MSF файла повреждена.",
 "Это не MSF файл.",
 "Ошибка открытия файла.",
 "",
 "",
 "Ошибка чтения заголовка MSF файла.",
 "Ошибка позиционирования в файле.",
 "Ошибка записи заголовка MSF файла.",
 "Не удалось создать объект 'Осциллограф'.",
 "",
 "",
 "",
 "Окно CBKKbdButn не захотело создаваться.",
 "",
 "Не удалось создать объект 'Экран'.",
 "Не удалось инициализировать объект 'Экран'.",
 "",
 "Успешно.",
 "Недостаточно памяти.",
 "",
 "",
 "",
 "",
 "Не найдена DLL библиотека ",
 "Не тот файл подсовываете. Повторим?",
 "",
 "Режим отображения цветной / чёрно-белый",
 "Загрузить битмап (BMP, проч.) в эту страницу",
 "Сохранить страницу в битмап (BMP, проч.)",
 "Открыть отдельно окно просмотра страницы",
 "Невозможно открыть файл ""%s"" !\nОстановить CPU ? ",
 "\nКоманды:\n - /h или /? - эта справка.\n - /b путь - загрузка bin файла по заданному пути.\n - /m путь - загрузка msf файла по заданному пути.\n - /t путь - загрузка tape файла по заданному пути.\n - /s скрипт - запуск файла скрипта",
 "AY-3-8910",
 "YM2149F",
 "БК0010-01 + контроллер A16M",
 "БК0010-01 + доп. 32кб ОЗУ",
 "БК0010-01 + стандартный КНГМД",
 "БК0010-01 + блок Фокал-МСТД",
 "БК0010-01 + контроллер Samara",
 "БК0010-01 + контроллер SMK512",
 "БК0010-01",
 "БК0011 + контроллер A16M",
 "БК0011 + стандартный КНГМД",
 "БК0011М + контроллер A16M",
 "БК0011М + стандартный КНГМД",
 "БК0011М + контроллер Samara",
 "БК0011М + контроллер SMK512",
 "БК0011М + МСТД",
 "БК0011 + контроллер Samara",
 "БК0011 + контроллер SMK512",
 "БК0011 + МСТД",
 "Необходим DirectX 9.0 или выше!",
 "Адрес",
 "Аргументы",
 "Команда",
 "Машинный код",
 " ",
 "Окно отладки",
 "HDD AltPro",
 "ANDOS",
 "AODOS",
 "CSIDOS3",
 "Директория.",
 "DX-DOS",
 "Ошибка чтения",
 "MicroDOS",
 "MKDOS",
 "MS-DOS",
 "HC-DOS",
 "NORD",
 "RT-11",
 "HDD Samara",
 "Неизвестный формат",
 "Нет доступа.",
 "Системный",
 "Редактировать",
 "Эмулятор БК для Windows",
 "MSF старых версий, к сожалению, не поддерживается.",
 "Не поддерживаемый или повреждённый формат MSF файла.",
 "Возможно, внутренняя ошибка эмулятора. Эмулятор остановлен. ",
 "Внимание, отсутствует ROM модуль ",
 "Внимание, не задан ROM модуль для ",
 "Исключительная ситуация по адресу: %06o.\nНеверная инструкция по адресу: %06o.\nВозможная причина: ""%s"".\n""Прервать"" - отладка.\n""Повтор"" - продолжить выполнение. \n""Пропустить"" - больше не показывать это сообщение.",
 "Невозможно открыть файл.",
 "Проводник",
 ".bin",
 ".msf",
 ".rom",
 ".bkscript",
 ".tap",
 ".wav",
 "Бинарный файл эмулятора (*.bin *.BIN);; Все файлы (*.*)",
 "Образ жёсткого диска БК (*.hdi);; Все файлы (*.*)",
 "Образ диска БК (*.img *.bkd *.dsk);; Все файлы (*.*)",
 "Все файлы изображений",
 "Файл состояния памяти (*.msf);; Все файлы (*.*)",
 "",
 "",
 "Файл кассеты БК (*.tap);; Все файлы (*.*)",
 "Microsoft Wave (*.wav);; Все файлы (*.*)",
 "Start disasm address",
 "Start dump address",
 "Show dialog on access violation",
 "Tape auto start detection",
 "Tape auto end detection",
 "AY Channel A Pan Left",
 "AY Channel B Pan Left",
 "AY Channel C Pan Left",
 "AY Channel A Volume",
 "AY Channel B Volume",
 "AY Channel C Volume",
 "AY8910 enabled",
 "AY8910 DC Offset correct",
 "AY8910 filter enabled",
 "Big buttons for Instrumental Panel",
 "BK10 Optional MSTD",
 "BK10 Monitor",
 "BK10 Focal",
 "BK10 MSTD",
 "BK10 Basic1",
 "BK10 Basic2",
 "BK10 Basic3",
 "BK11M Monitor BOS",
 "BK11M Monitor EXT",
 "BK11M Basic1",
 "BK11M Basic2",
 "BK11M Basic3",
 "BK11M MSTD",
 "BK11 Basic1",
 "BK11 Basic2",
 "BK11 Basic3",
 "BK11 Monitor BOS",
 "BK11 Monitor EXT",
 "BK11 MSTD",
 "BK11 Optional Page12.1",
 "BK11 Optional Page12.2",
 "BK11 Optional Page13.1",
 "BK11 Optional Page13.2",
 "Joystick A Button",
 "Joystick AltFire",
 "Joystick B Button",
 "Joystick Down",
 "Joystick Fire",
 "Joystick Left",
 "Joystick Right",
 "Joystick Up",
 "Emulate BK keyboard",
 "BK model",
 "Adapt black & white mode",
 "Run in color mode",
 "Covox enabled",
 "Covox DC Offset correct",
 "Covox filter enabled",
 "Stereo covox",
 "CPU frequency",
 "CPU start address",
 "Drive A:",
 "Drive B:",
 "Drive C:",
 "Drive D:",
 "Emulate FDD IO",
 "Emulate load tape operations",
 "Emulate save tape operations",
 "FDD KNGMD253",
 "FDD A16M",
 "FDD KNGMD",
 "FDD Samara",
 "FDD SMK512",
 "FFMPEG Cmd Line",
 "bk.ini",
 "Run in fullscreen mode",
 "HDD0",
 "HDD1",
 "IC Load Block",
 "IMG directory",
 "Joystick enabled",
 "Use long Bin format",
 "Screen fade emulation",
 "Memory directory",
 "Menestrel DC Offset correct",
 "Menestrel filter enabled",
 "Menestrel enabled",
 "Emulate Mouse",
 "Origin screenshot size",
 "Oscilloscope render type",
 "Adapt BW Palette",
 "BW Palette",
 "Color Palette 00",
 "Color Palette 01",
 "Color Palette 02",
 "Color Palette 03",
 "Color Palette 04",
 "Color Palette 05",
 "Color Palette 06",
 "Color Palette 07",
 "Color Palette 08",
 "Color Palette 09",
 "Color Palette 10",
 "Color Palette 11",
 "Color Palette 12",
 "Color Palette 13",
 "Color Palette 14",
 "Color Palette 15",
 "Pause CPU after emulator start",
 "Program directory",
 "Registers dump interval",
 "Rom directory",
 "Use Saves directory as default",
 "User Saves directory",
 "Scripts directory",
 "Screen render type",
 "Directories",
 "Drives",
 "Joystick Parameters",
 "Main",
 "Options",
 "Palettes",
 "Parameters",
 "Rom modules",
 "Show performance string",
 "Smoothing Screen",
 "SoundChip Frequency",
 "SoundChip Model",
 "Sound Sample Rate",
 "Sound volume",
 "Speaker DC Offset correct",
 "Speaker filter enabled",
 "Speaker enabled",
 "Screenshots directory",
 "Screenshot number",
 "Tapes directory",
 "Tools directory",
 "Virtual Keyboard Type",
 "press key...",
 "unknown",
 "Ошибка",
 "BINARY",
 "MSF TAPE",
 "UNKNOWN",
 "WAVE",
 "BYTE",
 "Дамп памяти",
 "WORD",
 "177130|Чт",
 "177130|Зп",
 "177132|Чт",
 "177132|Зп",
 "177660",
 "177662|Чт",
 "177662|Зп",
 "177664",
 "177700",
 "177702",
 "177704",
 "177706",
 "177710",
 "177712",
 "177714|Чт",
 "177714|Зп",
 "177716|Чт",
 "177716|ЗпП",
 "177716|ЗпЗ",
 "PC",
 "PSW",
 "R0",
 "R1",
 "R2",
 "R3",
 "R4",
 "R5",
 "SP",
 "Стоп",
 "Продолжить",
 "BKEmu сообщает",
 "Неверный ID сообщения: %d.",
 "Осциллограф",
 "Консоль",
 "Градации Серого",
 "Россыпь Зелёного",
 "Искры Синего",
 "Оттенки Янтаря",
 "Ещё...",
 "Page %3i of %3i ",
 "Electronica BK",
 "DEC",
 "HEX",
 "Дамп регистров CPU",
 "Дамп регистров FDD",
 "Direct2D",
 "Direct3D",
 "VFW",
 "GDI+",
 "OpenGL",
 "Экран",
 "Кнопочная",
 "Плёночная",
 " Приводы и параметры",
 " Интерфейс эмулятора ",
 " Текущие опции",
 "1500000 (12МГц/8)",
 "1714286 (12МГц/7)",
 "1774400 (ZX)",
 "1789772 (12МГц/6.7)",
 "Панель 1",
 "Панель 2",
 "Управление чтением / записью лент",
 "Выберите папку для копирования...",
 "Адрес",
 "",
 "",
 "CRC",
 "Ошибка контрольной суммы в файле %s.",
 "err",
 "ок",
 "\tОбнаружен конец файла",
 "Устройство ввода по умолчанию",
 "Длина",
 "%s : адрес = %o, длина = %o",
 "Имя",
 "Устройства ввода отсутствуют.",
 "Путь файла",
 "Сохранить как...",
 "\t'%s' - ошибка. Невозможно записать файл.",
 "Сохранить выделение:",
 "\t'%s' - успешно.",
 "Время",
 "Тип",
 "неизвестно",
 "Длина Wave",
 "Настройка...",
 "Отладка",
 "Звук",
 "Стандартная",
 "Захват видео",
 "Виртуальная клавиатура",
 "Sys Break Mask"
};

