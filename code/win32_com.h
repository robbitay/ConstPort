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
	switch (comIndex)
	{
		case ComPort_1:  return "COM1";
		case ComPort_2:  return "COM2";
		case ComPort_3:  return "COM3";
		case ComPort_4:  return "COM4";
		case ComPort_5:  return "COM5";
		case ComPort_6:  return "COM6";
		case ComPort_7:  return "COM7";
		case ComPort_8:  return "COM8";
		case ComPort_9:  return "COM9";
		case ComPort_10: return "COM10";
		case ComPort_11: return "COM11";
		case ComPort_12: return "COM12";
		case ComPort_13: return "COM13";
		case ComPort_14: return "COM14";
		case ComPort_15: return "COM15";
		case ComPort_16: return "COM16";
		case ComPort_17: return "COM17";
		case ComPort_18: return "COM18";
		case ComPort_19: return "COM19";
		case ComPort_20: return "COM20";
		case ComPort_21: return "COM21";
		case ComPort_22: return "COM22";
		case ComPort_23: return "COM23";
		case ComPort_24: return "COM24";
		
		default:         return "Unkwn";
	};
}

#endif // _WIN32_COM_PORT_H