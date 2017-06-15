#ifndef _WIN32_COM_PORT_H
#define _WIN32_COM_PORT_H

#define MAX_COM_PORT_NUM   12

typedef enum
{
	BaudRate_110 = 1,
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
} BaudRate_t;

typedef enum
{
	Parity_None = 0,
	Parity_Even,
	Parity_Odd,
	Parity_Mark,
	Parity_Space,
} Parity_t;

typedef enum
{
	StopBits_1 = 1,
	StopBits_1_5,
	StopBits_2,
} StopBits_t;

typedef enum
{
	ComError_Generic = -1,
	ComError_Other   = -2,
	
} ComError_t;

struct ComPort_t
{
	bool isOpen;
	
	BaudRate_t baudRate;
	Parity_t   parity;
	StopBits_t stopBits;
	
	HANDLE handle;
};

#define GetComPortList_DEFINITION(functionName) u32 functionName(char* arrayOut, u32 arrayOutWidth, u32 arrayOutHeight)
typedef GetComPortList_DEFINITION(GetComPortList_f);

#define OpenComPort_DEFINITION(functionName) ComPort_t functionName( \
	const char* portName, BaudRate_t baudRate, bool useFlowControl,  \
	bool parityEnabled, Parity_t parity,                             \
	u8 numBits, StopBits_t stopBits)
typedef OpenComPort_DEFINITION(OpenComPort_f);

#define CloseComPort_DEFINITION(functionName) void functionName(ComPort_t* comPortPntr)
typedef CloseComPort_DEFINITION(CloseComPort_f);

#define ReadComPort_DEFINITION(functionName) i32 functionName(ComPort_t* comPortPntr, void* outputBuffer, u32 outputBufferLength)
typedef ReadComPort_DEFINITION(ReadComPort_f);

#define WriteComPort_DEFINITION(functionName) void functionName(ComPort_t* comPortPntr, const char* newChars, u32 numChars)
typedef WriteComPort_DEFINITION(WriteComPort_f);

#endif // _WIN32_COM_PORT_H