#ifndef _WIN32_COM_PORT_H
#define _WIN32_COM_PORT_H

#define MAX_COM_PORT_NUM   24
#define MAX_COM_PORT_NAME_LENGTH 5

//TODO: Is there ever a COM0?
typedef enum
{
	ComPort_1 = 0,
	ComPort_2,
	ComPort_3,
	ComPort_4,
	ComPort_5,
	ComPort_6,
	ComPort_7,
	ComPort_8,
	ComPort_9,
	ComPort_10,
	ComPort_11,
	ComPort_12,
	ComPort_13,
	ComPort_14,
	ComPort_15,
	ComPort_16,
	ComPort_17,
	ComPort_18,
	ComPort_19,
	ComPort_20,
	ComPort_21,
	ComPort_22,
	ComPort_23,
	ComPort_24,
	ComPort_25,
	ComPort_26,
	ComPort_27,
	ComPort_28,
	ComPort_29,
	ComPort_30,
	ComPort_31,
	ComPort_32,
	ComPort_33,
	ComPort_34,
	ComPort_35,
	ComPort_36,
	ComPort_37,
	ComPort_38,
	ComPort_39,
	ComPort_40,
	ComPort_41,
	ComPort_42,
	ComPort_43,
	ComPort_44,
	ComPort_45,
	ComPort_46,
	ComPort_47,
	ComPort_48,
	ComPort_49,
	ComPort_50,
	ComPort_51,
	ComPort_52,
	ComPort_53,
	ComPort_54,
	ComPort_55,
	ComPort_56,
	ComPort_57,
	ComPort_58,
	ComPort_59,
	ComPort_60,
	ComPort_61,
	ComPort_62,
	ComPort_63,
	ComPort_64,
	ComPort_65,
	ComPort_66,
	ComPort_67,
	ComPort_68,
	ComPort_69,
	ComPort_70,
	ComPort_71,
	ComPort_72,
	ComPort_73,
	ComPort_74,
	ComPort_75,
	ComPort_76,
	ComPort_77,
	ComPort_78,
	ComPort_79,
	ComPort_80,
	ComPort_81,
	ComPort_82,
	ComPort_83,
	ComPort_84,
	ComPort_85,
	ComPort_86,
	ComPort_87,
	ComPort_88,
	ComPort_89,
	ComPort_90,
	ComPort_91,
	ComPort_92,
	ComPort_93,
	ComPort_94,
	ComPort_95,
	ComPort_96,
	ComPort_97,
	ComPort_98,
	ComPort_99,
	ComPort_100,
	ComPort_101,
	ComPort_102,
	ComPort_103,
	ComPort_104,
	ComPort_105,
	ComPort_106,
	ComPort_107,
	ComPort_108,
	ComPort_109,
	ComPort_110,
	ComPort_111,
	ComPort_112,
	ComPort_113,
	ComPort_114,
	ComPort_115,
	ComPort_116,
	ComPort_117,
	ComPort_118,
	ComPort_119,
	ComPort_120,
	ComPort_121,
	ComPort_122,
	ComPort_123,
	ComPort_124,
	ComPort_125,
	ComPort_126,
	ComPort_127,
	ComPort_128,
	ComPort_129,
	ComPort_130,
	ComPort_131,
	ComPort_132,
	ComPort_133,
	ComPort_134,
	ComPort_135,
	ComPort_136,
	ComPort_137,
	ComPort_138,
	ComPort_139,
	ComPort_140,
	ComPort_141,
	ComPort_142,
	ComPort_143,
	ComPort_144,
	ComPort_145,
	ComPort_146,
	ComPort_147,
	ComPort_148,
	ComPort_149,
	ComPort_150,
	ComPort_151,
	ComPort_152,
	ComPort_153,
	ComPort_154,
	ComPort_155,
	ComPort_156,
	ComPort_157,
	ComPort_158,
	ComPort_159,
	ComPort_160,
	ComPort_161,
	ComPort_162,
	ComPort_163,
	ComPort_164,
	ComPort_165,
	ComPort_166,
	ComPort_167,
	ComPort_168,
	ComPort_169,
	ComPort_170,
	ComPort_171,
	ComPort_172,
	ComPort_173,
	ComPort_174,
	ComPort_175,
	ComPort_176,
	ComPort_177,
	ComPort_178,
	ComPort_179,
	ComPort_180,
	ComPort_181,
	ComPort_182,
	ComPort_183,
	ComPort_184,
	ComPort_185,
	ComPort_186,
	ComPort_187,
	ComPort_188,
	ComPort_189,
	ComPort_190,
	ComPort_191,
	ComPort_192,
	ComPort_193,
	ComPort_194,
	ComPort_195,
	ComPort_196,
	ComPort_197,
	ComPort_198,
	ComPort_199,
	ComPort_200,
	ComPort_201,
	ComPort_202,
	ComPort_203,
	ComPort_204,
	ComPort_205,
	ComPort_206,
	ComPort_207,
	ComPort_208,
	ComPort_209,
	ComPort_210,
	ComPort_211,
	ComPort_212,
	ComPort_213,
	ComPort_214,
	ComPort_215,
	ComPort_216,
	ComPort_217,
	ComPort_218,
	ComPort_219,
	ComPort_220,
	ComPort_221,
	ComPort_222,
	ComPort_223,
	ComPort_224,
	ComPort_225,
	ComPort_226,
	ComPort_227,
	ComPort_228,
	ComPort_229,
	ComPort_230,
	ComPort_231,
	ComPort_232,
	ComPort_233,
	ComPort_234,
	ComPort_235,
	ComPort_236,
	ComPort_237,
	ComPort_238,
	ComPort_239,
	ComPort_240,
	ComPort_241,
	ComPort_242,
	ComPort_243,
	ComPort_244,
	ComPort_245,
	ComPort_246,
	ComPort_247,
	ComPort_248,
	ComPort_249,
	ComPort_250,
	ComPort_251,
	ComPort_252,
	ComPort_253,
	ComPort_254,
	ComPort_255,
	
	NumComPorts,
} ComPortIndex_t;

typedef enum
{
	BaudRate_110 = 0,
	BaudRate_300,
	BaudRate_600,
	BaudRate_1200,
	BaudRate_2400,
	BaudRate_4800,
	BaudRate_9600,
	BaudRate_14400,
	BaudRate_19200,
	BaudRate_38400,
	BaudRate_57600,
	BaudRate_115200,
	BaudRate_128000,
	BaudRate_256000,
	
	NumBaudRates,
} BaudRate_t;

typedef enum
{
	Parity_None = 0,
	Parity_Even,
	Parity_Odd,
	Parity_Mark,
	Parity_Space,
	
	NumParityTypes,
} Parity_t;

typedef enum
{
	StopBits_1 = 0,
	StopBits_1_5,
	StopBits_2,
	
	NumStopBitTypes,
} StopBits_t;

typedef enum
{
	ComError_Generic = -1,
	ComError_Other   = -2,
	
} ComError_t;

struct ComSettings_t
{
	BaudRate_t baudRate;
	Parity_t parity;
	StopBits_t stopBits;
	u8 numBits;
	bool flowControlEnabled;
};

bool operator == (ComSettings_t left, ComSettings_t right)
{
	if (left.baudRate == right.baudRate &&
		left.parity == right.parity &&
		left.stopBits == right.stopBits &&
		left.numBits == right.numBits &&
		left.flowControlEnabled == right.flowControlEnabled)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool operator != (ComSettings_t left, ComSettings_t right)
{
	return !(left == right);
}

struct ComPort_t
{
	bool isOpen;
	char* name;
	ComSettings_t settings;
	
	HANDLE handle;
};

const char* GetBaudRateString(BaudRate_t baudRate)
{
	switch (baudRate)
	{
		case BaudRate_110:    return "110";
		case BaudRate_300:    return "300";
		case BaudRate_600:    return "600";
		case BaudRate_1200:   return "1200";
		case BaudRate_2400:   return "2400";
		case BaudRate_4800:   return "4800";
		case BaudRate_9600:   return "9600";
		case BaudRate_14400:  return "14400";
		case BaudRate_19200:  return "19200";
		case BaudRate_38400:  return "38400";
		case BaudRate_57600:  return "57600";
		case BaudRate_115200: return "115200";
		case BaudRate_128000: return "128000";
		case BaudRate_256000: return "256000";
		
		default:              return "Unknwn";
	};
}

const char* GetParityString(Parity_t parity)
{
	switch(parity)
	{
		case Parity_None:  return "None";
		case Parity_Even:  return "Even";
		case Parity_Odd:   return "Odd";
		case Parity_Mark:  return "Mark";
		case Parity_Space: return "Space";
		
		default:           return "Unkwn";
	};
}

const char* GetStopBitsString(StopBits_t stopBits)
{
	switch(stopBits)
	{
		case StopBits_1:   return "1";
		case StopBits_1_5: return "1.5";
		case StopBits_2:   return "2";
		
		default:           return "Unk";
	};
}

const char* GetComPortReadableName(ComPortIndex_t comIndex)
{
	static char returnBuffer[16] = {};
	
	snprintf(returnBuffer, ArrayCount(returnBuffer)-1, "COM%u", (u32)comIndex);
	returnBuffer[ArrayCount(returnBuffer)-1] = '\0';
	return returnBuffer;
}

#endif // _WIN32_COM_PORT_H