#ifdef BUILD_LK
#include <string.h>
#include <mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <linux/string.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

#include "lcm_drv.h"
#include <cust_gpio_usage.h>
#if defined(BUILD_LK)
#define LCM_PRINT printf
#elif defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
// pixel
#if 1
#define FRAME_WIDTH  			(720)
#define FRAME_HEIGHT 			(1280)
#else
#define FRAME_WIDTH  			(540)
#define FRAME_HEIGHT 			(960)
#endif

// physical dimension
#if 1
#define PHYSICAL_WIDTH        (64)
#define PHYSICAL_HIGHT         (116)
#else
#define PHYSICAL_WIDTH        (70)
#define PHYSICAL_HIGHT         (122)
#endif

#define LCM_ID       (0xb9)
#define LCM_DSI_CMD_MODE		0

#define REGFLAG_DELAY 0xAB
#define REGFLAG_END_OF_TABLE 0xAA // END OF REGISTERS MARKER

//#define JDI_LCD_CUT_NUM 1 /* JDI 1st cut*/
#define JDI_LCD_CUT_NUM 2 /* JDI 2nd cut*/
//#define JDI_LCD_CUT_NUM 3 /* JDI 2nd + NVM cut*/

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V3(para_tbl, size, force_update)   	lcm_util.dsi_set_cmdq_V3(para_tbl, size, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static unsigned int need_set_lcm_addr = 1;

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

#if 0 //ndef LGE_LPWG_SUPPORT
/* Version 1 & No Gamma  */
static struct LCM_setting_table lcm_initialization_setting[] = {

	{0x51,  1,  {0xFF}},     //[1] Power On #1
	{0x53,  1,  {0x2C}},     //[1] Power On #2
	{0x55,  1,  {0x40}},     //[1] Power On #3
	{0xB0,  1,  {0x04}},	 // Test command
	{0xC1,  3,  {0x84, 0x61, 0x00}},	//test command
	{0xD6,  1,  {0x01}},	// test command

	{0x36,  1,  {0x00}},	// set address mode

	/*display on*/
	{0x29,	0,  {}},            //[5] set display on

	/* exit sleep*/
	{0x11,	0,  {}},            //[3] exit sleep mode
	{REGFLAG_DELAY, 120, {}},    //MDELAY(120)
	{REGFLAG_END_OF_TABLE, 0x00, {}},
};
#else
#if 1 /* Version 2 */
#if (JDI_LCD_CUT_NUM == 1)
/* R003 */
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0X51, 1, {0XFF}},	//Write_Display_Brightness
	{0X53, 1, {0X2C}},		//Write_CTRL_Display
	{0X55, 1, {0X40}},		//Write_CABC
	{0XB0, 1, {0X04}},		//Test command
	{0xC1, 3, {0x84,0x61,0x00}},
	{0xC7, 30,{0x00,0x0A,0x16,0x20,0x2C,0x39,0x43,0x52,0x36,0x3E,
				0x4B,0x58,0x5A,0x5F,0x67,0x00,0x0A,0x16,0x20,0x2C,
				0x39,0x43,0x52,0x36,0x3E,0x4B,0x58,0x5A,0x5F,0x67}},
	{0xC8,19,{0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00, 0x00,
					0x00,0xFC,0x00,0x00,0x00,0x00,0x00,0xFC,0x00}},
	{0XB8, 6, {0x07,0x90,0x1E,0x00,0x40,0x32}}, //Back Light Control 1
	{0XB9, 6, {0x07,0x8C,0x3C,0x20,0x2D,0x87}},
	{0XBA, 6, {0x07,0x82,0x3C,0x10,0x3C,0xB4}},
	{0XCE, 24, {0x7D,0x40,0x43,0x49,0x55,0x62,0x71,0x82,0x94,0xA8,
					0xB9,0xCB,0xDB,0xE9,0xF5,0xFC,0xFF,0x02,0x00,0x04,
					0x04,0x44,0x04,0x01}},
	{0XBB, 3, {0x01,0x1E,0x14}},
	{0XBC, 3, {0x01,0x50,0x32}},
	{0XBD, 3, {0x00,0xB4,0xA0}},
	{0XD6, 1, {0x01}},
	{0X36, 1, {0x00}},
	{0X29, 1, {0x00}},
	{0X11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},	 //MDELAY(120)
	{REGFLAG_END_OF_TABLE, 0x00, {}},
};
#elif (JDI_LCD_CUT_NUM == 2)
#if 1
/* R0091 */
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0X51, 1, {0XFF}},	//Write_Display_Brightness
	{0X53, 1, {0X0C}},		//Write_CTRL_Display
	{0X55, 1, {0X00}},		//Write_CABC
	{0XB0, 1, {0X04}},		//Test command

	{0xB3,7,{0x14,0x1A,0x00,0x00,0x00,0x00,0x00}},
	{0XB4,1,{0X0C}},
	{0xB6,2,{0x3A,0xB3}},	
	{0xC1,44,{0x84,0x61,0x00,0x52,0x4A,0x59,0x94,0x20,0x03,0x1C,
			0xD9,0xD8,0x82,0xCF,0xB9,0x07, 0x17, 0x6B, 0xD1, 0x80,
			0x41,0x82,0x94,0x52,0x4A,0x09,0x11,0x11,0x11,0x11,
			0xA0, 0x40, 0x42,0x20,0x12,0x10,0x22,0x00,0x15,0x00,
			0x01,0x00,0x00,0x00}},

	{0xC2,9,{0x31,0xF5,0x00,0x00,0x04,0x00,0x08,0x00,0x00}},
	{0xC4,15,{0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x06,0x16,0x16,0x16,0x01}},
	{0xC6,20,{0x54,0x10,0x10,0x04,0x49,0x01,0x01,0x01,0x01,0x01,0x01,0x45,0x0D,0x01,0x01,0x01,0x01,0x06,0x13,0x03}},
	//{0xCB,12,{0xF0,0xF7,0xFF,0x3F,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0xC0}},
	{0xCB,12,{0xF0,0xF7,0xFF,0x3F,0x30,0x00,0x00,0x00,0x00, 0x00,0x3C,0xCF}},
	{0xCC,1,{0x0D}},
	{0xD0,5,{0x10,0x91,0xBB,0x12,0x8E}},
	{0xD1,5,{0x25,0x00,0x19,0x61,0x06}},
	{0xD3,27,{0x0B,0x37,0x9F,0xBD,0xB7,0x33,0x33,0x17,0x00, 0x01,
				0x00,0xA0,0xD8,0xA0,0x0D,0x23,0x23,0x33,0x3B,0xF7,
				0x72,0x07,0x3D,0xBF,0x99,0x21,0xFA}},
	{0xD5,11,{0x06,0x00,0x00,0x01,0x28,0x01,0x28,0x00,0x00, 0x00,0x00}},
	{0xD9,12,{0x8A,0x23,0xAA,0xAA,0x0A,0xE4,0x00,0x06,0x07,0x00,0x00,0x00}},
	{0xEC,48,{0x29,0x00,0x2C,0x2B,0x01,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x09,0x31/*0x00*/,0x11,
				0x0F,0x00,0x00,0x00,0x00,0x50,0x06,0x00,0x00,0x00,
				0x00,0x05,0xFC,0x00,0x00,0x00,0x03,0x40,0x02,0x13,
				0x02,0x02,0x01,0x00,0x00,0x00,0x00,0x00}},
	{0xED,31,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,0x00}},
	{0xEE,31,{0x1D,0x02,0x02,0x03,0x00,0x10,0x00,0x02,0x00,0x00,
				0x00,0x00,0x00,0x03,0x00,0x00,0x01,0xC5,0x01,0xC4,
				0x0E,0x28,0x23,0x00,0x00,0x2A,0x00,0x00,0x00,0x00,0x00}},
	{0xEF,62,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,
				0x00,0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,
				0xF0,0x00,0x00,0x54,0x00,0x03,0x1E,0x21,0x00,0xDF,
				0x00,0xDD,0x06,0xF3,0x3F,0x00,0x15,0x00,0x00,0x00,
				0x00,0x00}},
	/*{0xC7,30,{0x00,0x07,0x10,0x18,0x25,0x32,0x3C,0x4C,0x31,0x3A,
				0x47,0x57,0x64,0x69,0x76,0x00,0x07,0x10,0x18,0x25,
				0x32,0x3C,0x4C,0x31,0x3A,0x47,0x57,0x64,0x69,0x76}},*/
	{0xC7,30,{0x00,0x0A,0x16,0x20,0x2C,0x39,0x43,0x52,0x36,0x3E,
				0x4B,0x58,0x5A,0x5F,0x67,0x00,0x0A,0x16,0x20,0x2C,
				0x39,0x43,0x52,0x36,0x3E,0x4B,0x58,0x5A,0x5F,0x67}},
//	{0xC8,19,{0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00, 0x00,
//				0x00,0xC6,0x00,0x00,0x00,0x00,0x00,0xBF,0x00}},
	{0xC8,19,{0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00, 0x00,
				0x00,0xFC,0x00,0x00,0x00,0x00,0x00,0xFC,0x00}},
	{0xCA,32,{0x01,0x70,0x9A,0x95,0xA2,0x98,0x95,0x8F,0x3F, 0x3F,
				0x80,0x80,0x08,0x80,0x08,0x3F,0x08,0x90,0x0C,0x0C,
				0x0A,0x06,0x04,0x04,0x00,0xC8,0x10,0x10,0x3F,0x3F,
				0x3F,0x3F}},
/*
	{0XB8, 6, {0x07,0x90,0x1E,0x00,0x40,0x32}}, //Back Light Control 1
	{0XB9, 6, {0x07,0x8C,0x3C,0x20,0x2D,0x87}},
	{0XBA, 6, {0x07,0x82,0x3C,0x10,0x3C,0xB4}},
	{0XCE, 24, {0x7D,0x40,0x43,0x49,0x55,
					0x62,0x71,0x82,0x94,0xA8,
					0xB9,0xCB,0xDB,0xE9,0xF5,
					0xFC,0xFF,0x02,0x00,0x04,
					0x04,0x44,0x04,0x01}},
	{0XBB, 3, {0x01,0x1E,0x14}},
	{0XBC, 3, {0x01,0x50,0x32}},
	{0XBD, 3, {0x00,0xB4,0xA0}},*/
	{0XD6, 1, {0x01}},
	{0X36, 1, {0x00}},
	{0X29, 1, {0x00}},
	{0X11, 1, {0x00}},
//	{REGFLAG_DELAY, 120, {}},	 //MDELAY(120)
	{REGFLAG_END_OF_TABLE, 0x00, {}},

};

#else
/* R0075 */
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0X51, 1, {0XFF}},	//Write_Display_Brightness
	{0X53, 1, {0X2C}},		//Write_CTRL_Display
	{0X55, 1, {0X40}},		//Write_CABC
	{0XB0, 1, {0X04}},		//Test command
				
	{0xB3,7,{0x14,0x1A,0x00,0x00,0x00,0x00,0x00}},
	{0XB4,1,{0X0C}},
	{0xB6,2,{0x3A,0xB3}},
	{0xC1,44,{0x84,0x61,0x00,0x52,0x4A,0x59,0x94,0x20,0x03,0x1C,
			0xD9,0xD8,0x82,0xCF,0xB9,0x07, 0x17, 0x6B, 0xD1, 0x80,
			0x41,0x82,0x94,0x52,0x4A,0x09,0x11,0x11,0x11,0x11,
			0xA0, 0x40, 0x42,0x20,0x12,0x10,0x22,0x00,0x15,0x00,
			0x01,0x00,0x00,0x00}},
	{0xC2,9,{0x31,0xF5,0x00,0x00,0x04,0x00,0x08,0x00,0x00}},
	{0xC4,15,{0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x06,0x16,0x16,0x16,0x01}},
	{0xC6,20,{0x54,0x10,0x10,0x04,0x49,0x01,0x01,0x01,0x01,0x01,0x01,0x45,0x0D,0x01,0x01,0x01,0x01,0x06,0x13,0x03}},
	{0xCB,12,{0xF0,0xF7,0xFF,0x3F,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0xC0}},
	{0xCC,1,{0x0D}},
	{0xD0,5,{0x10,0x91,0xBB,0x12,0x8E}},
	{0xD1,5,{0x25,0x00,0x19,0x61,0x06}},
	{0xD3,27,{0x0B,0x37,0x9F,0xBD,0xB7,0x33,0x33,0x17,0x00, 0x01,
				0x00,0xA0,0xD8,0xA0,0x0D,0x23,0x23,0x33,0x3B,0xF7,
				0x72,0x07,0x3D,0xBF,0x99,0x21,0xFA}},
	{0xD5,11,{0x06,0x00,0x00,0x01,0x28,0x01,0x28,0x00,0x00, 0x00,0x00}},
	{0xD9,12,{0x8A,0x23,0xAA,0xAA,0x0A,0xE4,0x00,0x06,0x07,0x00,0x00,0x00}},
	{0xEC,48,{0x29,0x00,0x2C,0x2B,0x01,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x09,0x31/*0x00*/,0x11,
				0x0F,0x00,0x00,0x00,0x00,0x50,0x06,0x00,0x00,0x00,
				0x00,0x05,0xFC,0x00,0x00,0x00,0x03,0x40,0x02,0x13,
				0x02,0x02,0x01,0x00,0x00,0x00,0x00,0x00}},
	{0xED,31,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,0x00}},
	{0xEE,31,{0x1D,0x02,0x02,0x03,0x00,0x10,0x00,0x02,0x00,0x00,
				0x00,0x00,0x00,0x03,0x00,0x00,0x01,0xC5,0x01,0xC4,
				0x0E,0x28,0x23,0x00,0x00,0x2A,0x00,0x00,0x00,0x00,0x00}},
	{0xEF,62,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,
				0x00,0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,
				0xF0,0x00,0x00,0x54,0x00,0x03,0x1E,0x21,0x00,0xDF,
				0x00,0xDD,0x06,0xF3,0x3F,0x00,0x15,0x00,0x00,0x00,
				0x00,0x00}},
	{0xC7,30,{0x00,0x07,0x10,0x18,0x25,0x32,0x3C,0x4C,0x31,0x3A,
				0x47,0x57,0x64,0x69,0x76,0x00,0x07,0x10,0x18,0x25,
				0x32,0x3C,0x4C,0x31,0x3A,0x47,0x57,0x64,0x69,0x76}},
	{0xC8,19,{0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00, 0x00,
				0x00,0xC6,0x00,0x00,0x00,0x00,0x00,0xBF,0x00}},
	{0XB8, 6, {0x07,0x90,0x1E,0x00,0x40,0x32}}, //Back Light Control 1
	{0XB9, 6, {0x07,0x8C,0x3C,0x20,0x2D,0x87}},
	{0XBA, 6, {0x07,0x82,0x3C,0x10,0x3C,0xB4}},
	{0XCE, 24, {0x7D,0x40,0x43,0x49,0x55,
					0x62,0x71,0x82,0x94,0xA8,
					0xB9,0xCB,0xDB,0xE9,0xF5,
					0xFC,0xFF,0x02,0x00,0x04,
					0x04,0x44,0x04,0x01}},
	{0XBB, 3, {0x01,0x1E,0x14}},
	{0XBC, 3, {0x01,0x50,0x32}},
	{0XBD, 3, {0x00,0xB4,0xA0}},
	{0XD6, 1, {0x01}},
	{0X36, 1, {0x00}},
	{0X29, 1, {0x00}},
	{0X11, 1, {0x00}},
//	{REGFLAG_DELAY, 120, {}},	 //MDELAY(120)
	{REGFLAG_END_OF_TABLE, 0x00, {}},

};
#endif
#elif (JDI_LCD_CUT_NUM == 3)
/* R0085 */
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0X51, 1, {0XFF}},	//Write_Display_Brightness
	{0X53, 1, {0X2C}},		//Write_CTRL_Display
	{0X55, 1, {0X40}},		//Write_CABC
	{0XB0, 1, {0X04}},		//Test command

	{0xB3,7,{0x14,0x1A,0x00,0x00,0x00,0x00,0x00}},
	{0XB4,1,{0X0C}},
	{0xB6,2,{0x3A,0xB3}},
	{0xC1,44,{0x84,0x61,0x00,0x52,0x4A,0x59,0x94,0x20,0x03,0x1C,
			0xD9,0xD8,0x82,0xCF,0xB9,0x07, 0x17, 0x6B, 0xD1, 0x80,
			0x41,0x82,0x94,0x52,0x4A,0x09,0x11,0x11,0x11,0x11,
			0xA0, 0x40, 0x42,0x20,0x12,0x10,0x22,0x00,0x15,0x00,
			0x01,0x00,0x00,0x00}},
	{0xC2,9,{0x31,0xF5,0x00,0x00,0x04,0x00,0x08,0x00,0x00}},
	{0xC4,15,{0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x06,0x16,0x16,0x16,0x01}},
	{0xC6,20,{0x54,0x10,0x10,0x04,0x49,0x01,0x01,0x01,0x01,0x01,0x01,0x45,0x0D,0x01,0x01,0x01,0x01,0x06,0x13,0x03}},
	{0xCB,12,{0xF0,0xF7,0xFF,0x3F,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0xC0}},

	{0xCC,1,{0x0D}},
	{0xD0,5,{0x10,0x91,0xBB,0x12,0x8E}},
	{0xD1,5,{0x25,0x00,0x19,0x61,0x06}},
	{0xD3,27,{0x0B,0x37,0x9F,0xBD,0xB7,0x33,0x33,0x17,0x00, 0x01,
				0x00,0xA0,0xD8,0xA0,0x0D,0x23,0x23,0x33,0x3B,0xF7,
				0x72,0x07,0x3D,0xBF,0x99,0x21,0xFA}},
	{0xD5,11,{0x06,0x00,0x00,0x01,0x28,0x01,0x28,0x00,0x00, 0x00,0x00}},
	{0xD9,12,{0x8A,0x23,0xAA,0xAA,0x0A,0xE4,0x00,0x06,0x07,0x00,0x00,0x00}},
	{0xEC,48,{0x29,0x00,0x2C,0x2B,0x01,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x09,0x31/*0x00*/,0x11,
				0x0F,0x00,0x00,0x00,0x00,0x50,0x06,0x00,0x00,0x00,
				0x00,0x05,0xFC,0x00,0x00,0x00,0x03,0x40,0x02,0x13,
				0x02,0x02,0x01,0x00,0x00,0x00,0x00,0x00}},
	{0xED,31,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,0x00}},
	{0xEE,31,{0x1D,0x02,0x02,0x03,0x00,0x10,0x00,0x02,0x00,0x00,
				0x00,0x00,0x00,0x03,0x00,0x00,0x01,0xC5,0x01,0xC4,
				0x0E,0x28,0x23,0x00,0x00,0x2A,0x00,0x00,0x00,0x00,0x00}},
	{0xEF,62,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,
				0x00,0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,
				0xF0,0x00,0x00,0x54,0x00,0x03,0x1E,0x21,0x00,0xDF,
				0x00,0xDD,0x06,0xF3,0x3F,0x00,0x15,0x00,0x00,0x00,
				0x00,0x00}},
	{0xC7,30,{0x00,0x07,0x10,0x18,0x25,0x32,0x3C,0x4C,0x31,0x3A,
				0x47,0x57,0x64,0x69,0x76,0x00,0x07,0x10,0x18,0x25,
				0x32,0x3C,0x4C,0x31,0x3A,0x47,0x57,0x64,0x69,0x76}},
	{0xC8,19,{0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00, 0x00,
				0x00,0xC6,0x00,0x00,0x00,0x00,0x00,0xBF,0x00}},
	{0XB8, 6, {0x07,0x90,0x1E,0x00,0x40,0x32}}, //Back Light Control 1
	{0XB9, 6, {0x07,0x8C,0x3C,0x20,0x2D,0x87}},
	{0XBA, 6, {0x07,0x82,0x3C,0x10,0x3C,0xB4}},
	{0XCE, 24, {0x7D,0x40,0x43,0x49,0x55,
					0x62,0x71,0x82,0x94,0xA8,
					0xB9,0xCB,0xDB,0xE9,0xF5,
					0xFC,0xFF,0x02,0x00,0x04,
					0x04,0x44,0x04,0x01}},
	{0XBB, 3, {0x01,0x1E,0x14}},
	{0XBC, 3, {0x01,0x50,0x32}},
	{0XBD, 3, {0x00,0xB4,0xA0}},
	{0XD6, 1, {0x01}},
	{0X36, 1, {0x00}},
	{0X29, 1, {0x00}},
	{0X11, 1, {0x00}},
//	{REGFLAG_DELAY, 120, {}},	 //MDELAY(120)
	{REGFLAG_END_OF_TABLE, 0x00, {}},
};
#else
 #error /* Need to declear */
	#endif

#else
/* Version 3 & without Gamma */
static LCM_setting_table_V3 lcm_initialization_setting_V3[] = {

	{0x39, 0X51, 1, {0XFF}},	//Write_Display_Brightness
	{0x39,0X53, 1, {0X2C}},		//Write_CTRL_Display
	{0x39,0X55, 1, {0X40}},		//Write_CABC
	{0x29,0XB0, 1, {0X04}},		//Test command

	{0x29, 0xB3,7,{0x14,0x1A,0x00,0x00,0x00,0x00,0x00}},
	{0x29, 0XB4,1,{0X0C}},
	{0x29, 0xB6,2,{0x3A,0xB3}},
	{0x29, 0xC1,44,{0x84,0x61,0x00,0x52,0x4A,0x59,0x94,0x20,0x03,0x1C,
			0xD9,0xD8,0x82,0xCF,0xB9,0x07, 0x17, 0x6B, 0xD1, 0x80,
			0x41,0x82,0x94,0x52,0x4A,0x09,0x11,0x11,0x11,0x11,
			0xA0, 0x40, 0x42,0x20,0x12,0x10,0x22,0x00,0x15,0x00,
			0x01,0x00,0x00,0x00}},
	{0x29, 0xC2,9,{0x31,0xF5,0x00,0x00,0x04,0x00,0x08,0x00,0x00}},
	{0x29, 0xC4,15,{0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x06,0x16,0x16,0x16,0x01}},
	{0x29, 0xC6,20,{0x54,0x10,0x10,0x04,0x49,0x01,0x01,0x01,0x01,0x01,0x01,0x45,0x0D,0x01,0x01,0x01,0x01,0x06,0x13,0x03}},
	{0x29, 0xCB,12,{0xF0,0xF7,0xFF,0x3F,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0xC0}},

	{0x29, 0xCC,1,{0x0D}},
	{0x29, 0xD0,5,{0x10,0x91,0xBB,0x12,0x8E}},
	{0x29, 0xD1,5,{0x25,0x00,0x19,0x61,0x06}},
	{0x29, 0xD3,27,{0x0B,0x37,0x9F,0xBD,0xB7,0x33,0x33,0x17,0x00, 0x01,
				0x00,0xA0,0xD8,0xA0,0x0D,0x23,0x23,0x33,0x3B,0xF7,
				0x72,0x07,0x3D,0xBF,0x99,0x21,0xFA}},
	{0x29, 0xD5,11,{0x06,0x00,0x00,0x01,0x28,0x01,0x28,0x00,0x00, 0x00,0x00}},
	{0x29, 0xD9,12,{0x8A,0x23,0xAA,0xAA,0x0A,0xE4,0x00,0x06,0x07,0x00,0x00,0x00}},
	{0x29, 0xEC,48,{0x29,0x00,0x2C,0x2B,0x01,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x09,0x31 /*0x00*/,0x11,
				0x0F,0x00,0x00,0x00,0x00,0x50,0x06,0x00,0x00,0x00,
				0x00,0x05,0xFC,0x00,0x00,0x00,0x03,0x40,0x02,0x13,
				0x02,0x02,0x01,0x00,0x00,0x00,0x00,0x00}},
	{0x29, 0xED,31,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,0x00}},
	{0x29, 0xEE,31,{0x1D,0x02,0x02,0x03,0x00,0x10,0x00,0x02,0x00,0x00,
				0x00,0x00,0x00,0x03,0x00,0x00,0x01,0xC5,0x01,0xC4,
				0x0E,0x28,0x23,0x00,0x00,0x2A,0x00,0x00,0x00,0x00,0x00}},
	{0x29, 0xEF,62,{0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,0x00,
				0x00,0x00,0x00,0x03,0x1E,0x21,0x00,0xDF,0x00,0xDD,
				0x06,0xF3,0x3F,0x00,0x00,0x15,0x00,0x00,0x00,0x00,
				0x00,0x3B,0x02,0x02,0x03,0x21,0x11,0x00,0x93,0x00,
				0xF0,0x00,0x00,0x54,0x00,0x03,0x1E,0x21,0x00,0xDF,
				0x00,0xDD,0x06,0xF3,0x3F,0x00,0x15,0x00,0x00,0x00,
				0x00,0x00}},
	{0x29, 0xC7,30,{0x00,0x07,0x10,0x18,0x25,0x32,0x3C,0x4C,0x31,0x3A,
				0x47,0x57,0x64,0x69,0x76,0x00,0x07,0x10,0x18,0x25,
				0x32,0x3C,0x4C,0x31,0x3A,0x47,0x57,0x64,0x69,0x76}},
	{0x29, 0xC8,19,{0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00, 0x00,
				0x00,0xC6,0x00,0x00,0x00,0x00,0x00,0xBF,0x00}},
	{0x29, 0XB8, 6, {0x07,0x90,0x1E,0x00,0x40,0x32}}, //Back Light Control 1
	{0x29, 0XB9, 6, {0x07,0x8C,0x3C,0x20,0x2D,0x87}},
	{0x29, 0XBA, 6, {0x07,0x82,0x3C,0x10,0x3C,0xB4}},
	{0x29, 0XCE, 24, {0x7D,0x40,0x43,0x49,0x55,
					0x62,0x71,0x82,0x94,0xA8,
					0xB9,0xCB,0xDB,0xE9,0xF5,
					0xFC,0xFF,0x02,0x00,0x04,
					0x04,0x44,0x04,0x01}},
	{0x29, 0XBB, 3, {0x01,0x1E,0x14}},
	{0x29, 0XBC, 3, {0x01,0x50,0x32}},
	{0x29, 0XBD, 3, {0x00,0xB4,0xA0}},
	{0x29, 0XD6, 1, {0x01}},
	{0x15, 0X36, 1, {0x00}},
	{0x05, 0X29, 1, {0x00}},
	{0x05, 0X11, 1, {0x00}},
//	{REGFLAG_DELAY, 120, 1,{}},	 //MDELAY(120)
	{REGFLAG_END_OF_TABLE, 0x00, 1,{}},
};
#endif
#endif
static struct LCM_setting_table lcm_sleep_setting[] = {
	{0X28, 0, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	{0X10, 0, {0x00}},
	{REGFLAG_DELAY, 80, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static LCM_setting_table_V3 lcm_sleep_setting_v3[] = {
	{0x05,0X28, 0, {0x00}},
	{REGFLAG_DELAY, 20,0, {}},
	{0x05,0X10, 0, {0x00}},
	{REGFLAG_DELAY, 80, 0,{}},
	{REGFLAG_END_OF_TABLE, 0x00,0,{}}
};

static struct LCM_setting_table __attribute__ ((unused)) lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for(i = 0; i < count; i++) {
		unsigned cmd;
		
		cmd = table[i].cmd;

		switch (cmd) {
		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
	LCM_PRINT("[LCD] push_table \n");
}
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy((void*)&lcm_util, (void*)util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS * params) 
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->physical_width=PHYSICAL_WIDTH;
	params->physical_height=PHYSICAL_HIGHT;

       params->dsi.mode   = SYNC_EVENT_VDO_MODE; //BURST_VDO_MODE;//SYNC_PULSE_VDO_MODE;
//	params->dsi.switch_mode = CMD_MODE;
//	params->dsi.switch_mode_enable = 0;
	 // enable tearing-free
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				    = LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order 	= LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      	= LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	params->dsi.packet_size=256;
	//video mode timing
	// Video mode setting
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	#if 0
	params->dsi.vertical_sync_active				= 1;//2;
	params->dsi.vertical_backporch					= 3;   // from Q driver
	params->dsi.vertical_frontporch					= 4;  // rom Q driver
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 5;//10;
	params->dsi.horizontal_backporch				= 40; // from Q driver
	params->dsi.horizontal_frontporch				= 140;  // from Q driver
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 210;//240; //this value must be in MTK suggested table

	#else
	params->dsi.vertical_sync_active = 1; 
	params->dsi.vertical_backporch = 3; 
	params->dsi.vertical_frontporch = 6; 
	params->dsi.vertical_active_line = FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 5;
	params->dsi.horizontal_backporch				= 60;
	params->dsi.horizontal_frontporch				= 140;
	params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;

	/* 20141029 */
	//params->dsi.PLL_CLOCK = 208;
	params->dsi.PLL_CLOCK = 234;
	#endif

}

static void init_lcm_registers(void)
{
	unsigned int data_array[32];

#if 1 
#if 1
	/* Version 2 */
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#else
	/* Version 1 */
	data_array[0] = 0x00023902; //Write_Display_Brightness
	data_array[1] = 0x0000ff51;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00023902; //Write_CTRL_Display
	data_array[1] = 0x00002c53; //Write_CTRL_Display
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00023902; //Write_CABC
	data_array[1] = 0x00004055; //Write_CABC
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00022902; //Test command
	data_array[1] = 0x000004B0; //Test command
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00082902;  //Test Command
	data_array[1] = 0x001A14B3;
	data_array[2] = 0x00000000;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00022902; //Test command
	data_array[1] = 0x00000CB4; //Test command
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00032902; //Test command
	data_array[1] = 0x00B33AB6;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x002D2902; //Test command
	data_array[1] = 0x006184C1;
	data_array[2] = 0x94594A52;
	data_array[3] = 0xD91C0320;
	data_array[4] = 0xB9CF82D8;
	data_array[5] = 0xD16B1707;
	data_array[6] = 0x94824180;
	data_array[7] = 0x11094A52;
	data_array[8] = 0x0A111111;
	data_array[9] = 0x12204240;
	data_array[10] = 0x15002210;
	data_array[11] = 0x00000100;
	data_array[12] = 0x00000000;
	dsi_set_cmdq(data_array, 13, 1);

	data_array[0] = 0x000A2902; //Test command
	data_array[1] = 0x00F531C2;
	data_array[2] = 0x08000400;
	data_array[3] = 0x00000000;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x00102902; //Test command
	data_array[1] = 0x000070C4;
	data_array[2] = 0x00000000;
	data_array[3] = 0x06000000;
	data_array[4] = 0x01161616;
	dsi_set_cmdq(data_array, 5, 1);

	data_array[0] = 0x00152902; //Test command
	data_array[1] = 0x101054C6;
	data_array[2] = 0x01014904;
	data_array[3] = 0x01010101;
	data_array[4] = 0x01010D45;
	data_array[5] = 0x13060101;
	data_array[6] = 0x00000003;
	dsi_set_cmdq(data_array, 7, 1);

	data_array[0] = 0x000D2902; //Test command
	data_array[1] = 0xFFF7F0CB;
	data_array[2] = 0x0000303F;
	data_array[3] = 0x00000000;
	data_array[4] = 0x000000C0;
	dsi_set_cmdq(data_array, 5, 1);

	data_array[0] = 0x00022902; //Test command
	data_array[1] = 0x00000DCC;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00062902; //Test command
	data_array[1] = 0xBB9110D0;
	data_array[2] = 0x00008E12;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00063902; //Test command
	data_array[1] = 0x190025D1;
	data_array[2] = 0x00000661;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x001C2902; //Test command
	data_array[1] = 0x9F370BD3;
	data_array[2] = 0x3333B7BD;
	data_array[3] = 0x00010017;
	data_array[4] = 0x0DA0D8A0;
	data_array[5] = 0x3B332323;
	data_array[6] = 0x3D0772F7;
	data_array[7] = 0xFA2199BF;
	dsi_set_cmdq(data_array, 8, 1);

	data_array[0] = 0x000B2902; //Test command
	data_array[1] = 0x000006D5;
	data_array[2] = 0x28012801;
	data_array[3] = 0x00000000;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x000B2902; //Test command
	data_array[1] = 0x000006D5;
	data_array[2] = 0x28012801;
	data_array[3] = 0x00000000;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x000D2902; //Test command
	data_array[1] = 0xAA238AD9;
	data_array[2] = 0x00E40AAA;
	data_array[3] = 0x00000706;
	data_array[4] = 0x00000000;
	dsi_set_cmdq(data_array, 5, 1);

	data_array[0] = 0x00312902; //Test command
	data_array[1] = 0x0000012B;
	data_array[2] = 0x00000000;
	data_array[3] = 0x00000000;
	data_array[4] = 0x31090700;
	data_array[5] = 0x00000F11;
	data_array[6] = 0x06500000;
	data_array[7] = 0x00000000;
	data_array[8] = 0x0000FC05;
	data_array[9] = 0x02400300;
	data_array[10] = 0x01020213;
	data_array[11] = 0x00000000;
	data_array[12] = 0x00000000;
	dsi_set_cmdq(data_array, 13, 1);

	data_array[0] = 0x00202902; //Test command
	data_array[1] = 0x02023BED;
	data_array[2] = 0x00112103;
	data_array[3] = 0x00000093;
	data_array[4] = 0x1E030000;
	data_array[5] = 0x00DF0021;
	data_array[6] = 0x3FF306DD;
	data_array[7] = 0x00150000;
	data_array[8] = 0x00000000;
	dsi_set_cmdq(data_array, 9, 1);

	data_array[0] = 0x00202902; //Test command
	data_array[1] = 0x02021DEE;
	data_array[2] = 0x00100003;
	data_array[3] = 0x00000002;
	data_array[4] = 0x00030000;
	data_array[5] = 0x01C50100;
	data_array[6] = 0x23280EC4;
	data_array[7] = 0x002A0000;
	data_array[8] = 0x00000000;
	dsi_set_cmdq(data_array, 9, 1);

	data_array[0] = 0x003F2902; //Test command
	data_array[1] = 0x02023BEF; 
	data_array[2] = 0x00112103;
	data_array[3] = 0x00000093;
	data_array[4] = 0x1E030000;
	data_array[5] = 0x00DF0021;
	data_array[6] = 0x3FF306DD;
	data_array[7] = 0x00150000;
	data_array[8] = 0x00000000;
	data_array[9] = 0x0302023B;
	data_array[10] = 0x93001121;
	data_array[11] = 0x00000F00;
	data_array[12] = 0x1E030054;
	data_array[13] = 0x00DF0021;
	data_array[14] = 0x3FF306DD;
	data_array[15] = 0x00001500;
	data_array[16] = 0x00000000;
	dsi_set_cmdq(data_array, 17, 1);

	data_array[0] = 0x001F2902; //Test command
	data_array[1] = 0x10070030; 
	data_array[2] = 0x3C322518;
	data_array[3] = 0x473A314C;
	data_array[4] = 0x76696457;
	data_array[5] = 0x18100700;
	data_array[6] = 0x4C3C3225;
	data_array[7] = 0x57473A31;
	data_array[8] = 0x00766964;
	dsi_set_cmdq(data_array, 9, 1);

	data_array[0] = 0x00142902; //Test command
	data_array[1] = 0x000000C8;
	data_array[2] = 0x00FC0000;
	data_array[3] = 0x00000000;
	data_array[4] = 0x000000C6;
	data_array[5] = 0x00BF0000;
	dsi_set_cmdq(data_array, 6, 1);

	data_array[0] = 0x00072902; //Test command
	data_array[1] = 0x1E9007B8;
	data_array[2] = 0x00324000; 
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00072902; //Test command
	data_array[1] = 0x3C8C07B9;
	data_array[2] = 0x00872D20;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00072902; //Test command
	data_array[1] = 0x3C8207BA;
	data_array[2] = 0x00B43C10;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00192902; //Test command
	data_array[1] = 0x43407DCE;
	data_array[2] = 0x71625549;
	data_array[3] = 0xB9A89482;
	data_array[4] = 0xF5E9DBCB;
	data_array[5] = 0x0002FFFC;
	data_array[6] = 0x04440404;
	data_array[7] = 0x00000001;
	dsi_set_cmdq(data_array, 8, 1);

	data_array[0] = 0x00042900; //test commad
	data_array[1] = 0x141E01BB;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00042900; //test commad
	data_array[1] = 0x325001BC;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00042900; //test commad
	data_array[1] = 0xA0B400BD;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00021500; //set address mode
	data_array[0] = 0x00000036; //set address mode
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00290500; //Display On
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00110500; //Sleep Out
	dsi_set_cmdq(data_array, 1, 1);

#endif
#else
	/* version 3 */
	dsi_set_cmdq_V3(lcm_initialization_setting_V3, sizeof(lcm_initialization_setting_V3) / sizeof(LCM_setting_table_V3), 1);
#if 0
	data_array[0] = 0x00290500;	//Display On
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00110500;	//Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
#endif	

	MDELAY(120);

#endif
	LCM_PRINT("[LCD] init_lcm_registers \n");
}

static void init_lcm_registers_sleep(void)
{
	unsigned int data_array[1];
#if 0
	//push_table(lcm_sleep_setting, sizeof(lcm_sleep_setting) / sizeof(struct LCM_setting_table), 1);
	dsi_set_cmdq_V3(lcm_sleep_setting_v3, sizeof(lcm_sleep_setting_v3) / sizeof(LCM_setting_table_V3), 1);
#else
	MDELAY(10);
	data_array[0] = 0x00280500; //Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00100500; //enter sleep
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(80);
#endif
	LCM_PRINT("[LCD] init_lcm_registers_sleep \n");
}


/* VGP1 1.8v LDO enable */
static void ldo_1v8io_on(void)
{
#ifdef BUILD_UBOOT 
	#error "not implemeted"
#elif defined(BUILD_LK) 	
	upmu_set_rg_vgp1_vosel(3);  // VGP1_SEL= 101 : 2.8V , 110 : 3.0V
	upmu_set_rg_vgp1_en(1);	
#else
	hwPowerOn(MT6323_POWER_LDO_VCAMD, VOL_1800, "1V8_LCD_VIO_MTK_S");	
#endif 
}

/* VGP1 1.8v LDO disable */
static void ldo_1v8io_off(void)
{
#ifdef BUILD_UBOOT 
#error "not implemeted"
#elif defined(BUILD_LK) 	
	upmu_set_rg_vgp1_en(0);

#else
	hwPowerDown(MT6323_POWER_LDO_VCAMD, "1V8_LCD_VIO_MTK_S");	
#endif 
}

/* VGP2 3.0v LDO enable */
static void ldo_3v0_on(void)
{
#if 1 //defined(TARGET_S7)
#ifdef BUILD_UBOOT 
	#error "not implemeted"
#elif defined(BUILD_LK)
	upmu_set_rg_vgp2_vosel(6);  // VGP2_SEL= 101 : 2.8V , 110 : 3.0V
	upmu_set_rg_vgp2_en(1);	


#else
	hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_3000, "3V0_LCD_VCC_MTK_S");	
#endif
#else
	mt_set_gpio_mode(GPIO_LCM_PWR, GPIO_LCM_PWR_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCM_PWR, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_PWR, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ONE);
#endif
}

/* VGP2 3.0v LDO disable */
static void ldo_3v0_off(void)
{
#if 1 //defined(TARGET_S7)
#ifdef BUILD_UBOOT 
	#error "not implemeted"
#elif defined(BUILD_LK)
	upmu_set_rg_vgp2_en(0);
#else
	hwPowerDown(MT6323_POWER_LDO_VGP2, "3V0_LCD_VCC_MTK_S");	
#endif
#else
	mt_set_gpio_mode(GPIO_LCM_PWR, GPIO_LCM_PWR_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCM_PWR, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_PWR, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ZERO);
#endif
}

/*
DSV power +5V,-5v
*/
static void ldo_p5m5_dsv_3v0_on(void)
{
	mt_set_gpio_mode(GPIO_DSV_EN, GPIO_DSV_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_DSV_EN, GPIO_OUT_ONE);
}

static void ldo_p5m5_dsv_3v0_off(void)
{
	mt_set_gpio_mode(GPIO_DSV_EN, GPIO_DSV_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_DSV_EN, GPIO_OUT_ZERO);
}

/*
DSV power +5V,-5v
*/
static void ldo_p5m5_dsv_5v5_on(void)
{
	mt_set_gpio_mode(GPIO_DSV_AVDD_EN, GPIO_DSV_AVDD_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_AVDD_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_AVDD_EN, GPIO_DIR_OUT);
	mt_set_gpio_mode(GPIO_DSV_AVEE_EN, GPIO_DSV_AVEE_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_AVEE_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_AVEE_EN, GPIO_DIR_OUT);
	
	mt_set_gpio_out(GPIO_DSV_AVDD_EN, GPIO_OUT_ONE);
	MDELAY(4);
	mt_set_gpio_out(GPIO_DSV_AVEE_EN, GPIO_OUT_ONE);
}

static void ldo_p5m5_dsv_5v5_off(void)
{
	mt_set_gpio_mode(GPIO_DSV_AVDD_EN, GPIO_DSV_AVDD_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_AVDD_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_AVDD_EN, GPIO_DIR_OUT);
	mt_set_gpio_mode(GPIO_DSV_AVEE_EN, GPIO_DSV_AVEE_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_AVEE_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_AVEE_EN, GPIO_DIR_OUT);
	
	mt_set_gpio_out(GPIO_DSV_AVDD_EN, GPIO_OUT_ZERO);
	MDELAY(1);
	mt_set_gpio_out(GPIO_DSV_AVEE_EN, GPIO_OUT_ZERO);
}

static void reset_lcd_module(unsigned char reset)
{
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_LCM_RST_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCM_RST, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);

   if(reset){
   	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
      MDELAY(50);	
   }else{
   	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
   }
}
   

static void lcm_init(void)
{
#if defined(BUILD_LK) 	
	ldo_p5m5_dsv_5v5_off();
#endif
	//SET_RESET_PIN(0);

	//TP_VCI 3.0v on
	ldo_3v0_on();
	//MDELAY(200); //20141117
	MDELAY(10);

	ldo_1v8io_on();

	//MDELAY(200); //20141117
	MDELAY(10);

	ldo_p5m5_dsv_5v5_on();

	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(2);
	SET_RESET_PIN(1);
	MDELAY(20);

	init_lcm_registers();	//SET EXTC ~ sleep out register

	//MDELAY(80); //20141117
	
	MDELAY(120);
//	init_lcm_registers_added();	//Display On
	need_set_lcm_addr = 1;
	LCM_PRINT("[SEOSCTEST] lcm_init \n");
	LCM_PRINT("[LCD] lcm_init \n");
}

static void lcm_suspend(void)
{
	init_lcm_registers_sleep();

#if 1 //if LPWG on, Need to block
	SET_RESET_PIN(0);
	MDELAY(20);

	ldo_p5m5_dsv_5v5_off();
	MDELAY(10);

	ldo_1v8io_off();
#endif

	LCM_PRINT("[LCD] lcm_suspend \n");
}

static void lcm_resume(void)
{
    lcm_init();
    need_set_lcm_addr = 1;
    LCM_PRINT("[LCD] lcm_resume \n");
}

static void lcm_esd_recover(void)
{
	lcm_suspend();
	lcm_resume();

	LCM_PRINT("[LCD] lcm_esd_recover \n");
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	// need update at the first time
	if(need_set_lcm_addr)
	{
		data_array[0]= 0x00053902;
		data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
		data_array[2]= (x1_LSB);
		dsi_set_cmdq(data_array, 3, 1);
		
		data_array[0]= 0x00053902;
		data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
		data_array[2]= (y1_LSB);
		dsi_set_cmdq(data_array, 3, 1);		
		need_set_lcm_addr = 0;
	}
	
	data_array[0]= 0x002c3909;
   dsi_set_cmdq(data_array, 1, 0);
	LCM_PRINT("[LCD] lcm_update \n");	
}

static unsigned int lcm_compare_id(void)
{
#if 0
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);//Must over 6 ms
	array[0]=0x00043902;
	array[1]=0x8983FFB9;// page enable
	dsi_set_cmdq(array, 2, 1);
	MDELAY(10);
	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; 
	LCM_PRINT("%s, id = 0x%08x\n", __func__, id);
	return (LCM_ID == id)?1:0;
#else
	LCM_PRINT("[SEOSCTEST] lcm_compare_id \n");	
	return 1;
#endif	
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER r69338_hd720_5in_dsi_vdo_jdi_dw8768_drv = {
	.name = "r69338_hd720_5in_dsi_vdo_jdi_dw8768",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
//	.compare_id = lcm_compare_id,
//	.update = lcm_update,
#if (!defined(BUILD_UBOOT) && !defined(BUILD_LK))
//	.esd_recover = lcm_esd_recover,
#endif
};
