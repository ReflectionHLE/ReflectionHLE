#ifndef BE_MISC_LPT_H
#define BE_MISC_LPT_H

#include "be_cross.h"

typedef uint16_t BE_ST_LPTPort;

#define BE_LPT1 0x378
#define BE_LPT2 0x278
#define BE_LPT3 0x3BC

typedef enum
{
	BE_LPT_DATA,
	BE_LPT_STATUS,
	BE_LPT_CONTROL,

	// EPP is unimplemented. These are defined for reference purposes.
	BE_LPT_EPP_ADDRESS,
	BE_LPT_EPP_DATA0,
	BE_LPT_EPP_DATA1,
	BE_LPT_EPP_DATA2,
	BE_LPT_EPP_DATA3,
} BE_ST_LPTRegister;

#define BE_LPT_REG_COUNT 8
#define BE_LPT_REG_MASK 7
#define BE_LPT_MAX 3

#define BE_LPT_CONTROL_STROBE 0x1
#define BE_LPT_CONTROL_AUTOFD 0x2
#define BE_LPT_CONTROL_INIT 0x4
#define BE_LPT_CONTROL_SELECT 0x8
#define BE_LPT_CONTROL_IRQACK 0x10
#define BE_LPT_CONTROL_BIDI 0x20

#define BE_LPT_STATUS_IRQ 0x4
#define BE_LPT_STATUS_ERROR 0x8
#define BE_LPT_STATUS_SELECT 0x10
#define BE_LPT_STATUS_PAPEROUT 0x20
#define BE_LPT_STATUS_ACK 0x40
#define BE_LPT_STATUS_BUSY 0x80

// NOTE: It is possible the floating value varies by hardware. This value is
// what was seen on a Thinkpad R500 on Windows 10 22H2 with InpOutx64.
#define BE_LPT_STATUS_DISCONNECTED 0x7F

static inline int BE_ST_GetLPTFromPort(BE_ST_LPTPort port)
{
	if(port >= BE_LPT1 && port < BE_LPT1+BE_LPT_REG_COUNT)
		return 1;
	if(port >= BE_LPT2 && port < BE_LPT2+BE_LPT_REG_COUNT)
		return 2;
	if(port >= BE_LPT3 && port < BE_LPT3+BE_LPT_REG_COUNT)
		return 3;
	return 0;
}

static inline BE_ST_LPTRegister BE_ST_GetLPTRegFromPort(BE_ST_LPTPort port)
{
	if (port >= BE_LPT3) // Only LPT3 is unaligned
		return (BE_ST_LPTRegister)((port-BE_LPT3)&BE_LPT_REG_MASK);
	return (BE_ST_LPTRegister)(port&BE_LPT_REG_MASK);
}

uint8_t BE_ST_LPTIn(BE_ST_LPTPort port);
void BE_ST_LPTOut(BE_ST_LPTPort port, uint8_t val);

// Hints to release any established claim on the associated port. No DOS equivalent.
void BE_ST_LPTRelease(BE_ST_LPTPort port);

#endif
