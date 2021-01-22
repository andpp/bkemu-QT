#pragma once

constexpr auto SCREEN_WIDTH  = 512;
constexpr auto SCREEN_HEIGHT = 256;

// условная переменная, чтобы использовать множитель текстуры
// при этом текстура увеличивается в размерах и становится чуть чётче
// в увеличенном виде за счёт более крупного пиксела
// но и работы при этом делается больше. Что лучше, пока не знаю.
#define BK_USE_TEXTURE_MULTIPLER 0

#if (BK_USE_TEXTURE_MULTIPLER)
// если уменьшить до 1, получается уж очень мутная текстура
// при этом оптимизирующий компилятор убирает вырожденные циклы
// но не везде и не всё оптимизирует
constexpr auto TEXTURE_MULTIPLER_X = 2;
constexpr auto TEXTURE_MULTIPLER_Y = 2;
#else
// чтобы очень уж много условных компиляций не делать
// зададим параметры по умолчанию
constexpr auto TEXTURE_MULTIPLER_X = 1;
constexpr auto TEXTURE_MULTIPLER_Y = 1;
#endif
