#ifndef BORDER_CHARS_H
#define BORDER_CHARS_H

#include <wchar.h>
const wchar_t BASIC_BORDERCHARS[8] = {
	L'-',
	L'|',
	L'-',
	L'|',
	L'+',
	L'+',
	L'+',
	L'+'
};
const wchar_t INVIS_BORDERCHARS[8] = {
	L' ',
	L' ',
	L' ',
	L' ',
	L' ',
	L' ',
	L' ',
	L' '	
};
const wchar_t W1T1_BORDERCHARS[8] = {
	L'\x2500', 
	L'\x2502', 
	L'\x2500', 
	L'\x2502', 
	L'\x250C', 
	L'\x2510', 
	L'\x2518', 
	L'\x2514'
};
const wchar_t W1T2_BORDERCHARS[8] = {
	L'\x2501',
	L'\x2503',
	L'\x2501',
	L'\x2503',
	L'\x250F',
	L'\x2513',
	L'\x251B',
	L'\x2517'
};
const wchar_t W1T3_BORDERCHARS[8] = {
	L'\x2500',
	L'\x2502',
	L'\x2500',
	L'\x2502',
	L'\x256D',
	L'\x256E',
	L'\x256F',
	L'\x2570'
};
const wchar_t W2T1_BORDERCHARS[8] = {
	L'\x2550',
	L'\x2551',
	L'\x2550',
	L'\x2551',
	L'\x2554',
	L'\x2557',
	L'\x255D',
	L'\x255A'
};

const wchar_t BASIC_GRIDCHARS[13] = {
	L'-',
	L'|',
	L'-',
	L'|',
	L'+',
	L'+',
	L'+',
	L'+',
	L'+',
	L'+',
	L'+',
	L'+',
	L'+',
};
const wchar_t W1T1_GRIDCHARS[13] = {
	L'\x2500', 
	L'\x2502', 
	L'\x2500', 
	L'\x2502', 
	L'\x250C', 
	L'\x2510', 
	L'\x2518', 
	L'\x2514',
	L'\x252C',
	L'\x2524',
	L'\x2534',
	L'\x251C',
	L'\x253C'
};
const wchar_t W1T2_GRIDCHARS[13] = {
	L'\x2501',
	L'\x2503',
	L'\x2501',
	L'\x2503',
	L'\x250F',
	L'\x2513',
	L'\x251B',
	L'\x2517',
	L'\x2533',
	L'\x252B',
	L'\x253B',
	L'\x2523',
	L'\x254B'
};
const wchar_t W1T3_GRIDCHARS[13] = {
	L'\x2500',
	L'\x2502',
	L'\x2500',
	L'\x2502',
	L'\x256D',
	L'\x256E',
	L'\x256F',
	L'\x2570',
	L'\x252C',
	L'\x2524',
	L'\x2534',
	L'\x251C',
	L'\x253C'
};
const wchar_t W2T1_GRIDCHARS[13] = {
	L'\x2550',
	L'\x2551',
	L'\x2550',
	L'\x2551',
	L'\x2554',
	L'\x2557',
	L'\x255D',
	L'\x255A',
	L'\x2566',
	L'\x2563',
	L'\x2569',
	L'\x2560',
	L'\x256C'
};

#endif /* BORDER_CHARS_H */
