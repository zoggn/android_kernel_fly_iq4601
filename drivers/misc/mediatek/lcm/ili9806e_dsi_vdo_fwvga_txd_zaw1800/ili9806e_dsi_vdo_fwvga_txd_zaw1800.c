#ifdef BUILD_LK
#include <stdio.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/kernel.h>
#if defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#endif
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (480)
#define FRAME_HEIGHT (854)
#define ILI9806E_LCM_ID (0x0604)

#define REGFLAG_DELAY 0xFE

#define REGFLAG_END_OF_TABLE 0xFFF  // END OF REGISTERS MARKER

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)                                  (lcm_util.set_reset_pin((v)))

#define UDELAY(n)                                         (lcm_util.udelay(n))
#define MDELAY(n)                                         (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)  lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)     lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                    lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                     lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)             lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table
{
  unsigned char cmd;
  unsigned char count;
  unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] =
{
// unk_C081D5DC
	{ 0xFF, 0x05, {0xFF, 0x98, 0x06, 0x04, 0x01}},
    { 0x08, 0x01, {0x10}},
    { 0x21, 0x01, {0x01}},
    { 0x30, 0x01, {0x01}},
    { 0x31, 0x01, {0x02}},
    { 0x40, 0x01, {0x15}},
    { 0x41, 0x01, {0x77}},
    { 0x42, 0x01, {0x02}},
    { 0x43, 0x01, {0x0D}},
    { 0x44, 0x01, {0x06}},
    { 0x45, 0x01, {0x66}},
    { 0x50, 0x01, {0x78}},
    { 0x51, 0x01, {0x78}},
    { 0x52, 0x01, {0x00}},
    { 0x53, 0x01, {0x3A}},
    { 0x57, 0x01, {0x50}},
    { 0x60, 0x01, {0x07}},
    { 0x61, 0x01, {0x01}},
    { 0x62, 0x01, {0x06}},
    { 0x63, 0x01, {0x02}},
    { 0xFF, 0x05, {0xFF, 0x98, 0x06, 0x04, 0x01}},
    { 0xA0, 0x01, {0x08}},
    { 0xA1, 0x01, {0x0A}},
    { 0xA2, 0x01, {0x23}},
    { 0xA3, 0x01, {0x10}},
    { 0xA4, 0x01, {0x0F}},
    { 0xA5, 0x01, {0x1A}},
    { 0xA6, 0x01, {0x0A}},
    { 0xA7, 0x01, {0x01}},
    { 0xA8, 0x01, {0x03}},
    { 0xA9, 0x01, {0x0D}},
    { 0xAA, 0x01, {0x02}},
    { 0xAB, 0x01, {0x09}},
    { 0xAC, 0x01, {0x07}},
    { 0xAD, 0x01, {0x2B}},
    { 0xAE, 0x01, {0x24}},
    { 0xAF, 0x01, {0x08}},
    { 0xC0, 0x01, {0x00}},
    { 0xC1, 0x01, {0x0F}},
    { 0xC2, 0x01, {0x15}},
    { 0xC3, 0x01, {0x11}},
    { 0xC4, 0x01, {0x04}},
    { 0xC5, 0x01, {0x17}},
    { 0xC6, 0x01, {0x08}},
    { 0xC7, 0x01, {0x0D}},
    { 0xC8, 0x01, {0x06}},
    { 0xC9, 0x01, {0x05}},
    { 0xCA, 0x01, {0x09}},
    { 0xCB, 0x01, {0x04}},
    { 0xCC, 0x01, {0x0E}},
    { 0xCD, 0x01, {0x24}},
    { 0xCE, 0x01, {0x23}},
    { 0xCF, 0x01, {0x00}},
    { 0xFF, 0x05, {0xFF, 0x98, 0x06, 0x04, 0x06}},
    { 0x21, 0x00, {0x00}},
    { 0x01, 0x01, {0x06}},
    { 0x02, 0x01, {0xA0}},
    { 0x03, 0x01, {0x02}},
    { 0x04, 0x01, {0x01}},
    { 0x05, 0x01, {0x01}},
    { 0x06, 0x01, {0x80}},
    { 0x07, 0x01, {0x04}},
    { 0x08, 0x01, {0x00}},
    { 0x09, 0x01, {0x80}},
    { 0x0A, 0x01, {0x00}},
    { 0x0B, 0x01, {0x00}},
    { 0x0C, 0x01, {0x2C}},
    { 0x0D, 0x01, {0x2C}},
    { 0x0E, 0x01, {0x1C}},
    { 0x0F, 0x01, {0x00}},
    { 0x10, 0x01, {0xFF}},
    { 0x11, 0x01, {0xF0}},
    { 0x12, 0x01, {0x00}},
    { 0x13, 0x01, {0xC0}},
    { 0x14, 0x01, {0x00}},
    { 0x15, 0x01, {0xC0}},
    { 0x16, 0x01, {0x08}},
    { 0x17, 0x01, {0x00}},
    { 0x18, 0x01, {0x00}},
    { 0x19, 0x01, {0x00}},
    { 0x1A, 0x01, {0x00}},
    { 0x1B, 0x01, {0x00}},
    { 0x1C, 0x01, {0x00}},
    { 0x1D, 0x01, {0x00}},
    { 0x20, 0x01, {0x01}},
    { 0x21, 0x01, {0x23}},
    { 0x22, 0x01, {0x45}},
    { 0x23, 0x01, {0x67}},
    { 0x24, 0x01, {0x01}},
    { 0x25, 0x01, {0x23}},
    { 0x26, 0x01, {0x45}},
    { 0x27, 0x01, {0x67}},
    { 0x30, 0x01, {0x12}},
    { 0x31, 0x01, {0x22}},
    { 0x32, 0x01, {0x22}},
    { 0x33, 0x01, {0x22}},
    { 0x34, 0x01, {0x87}},
    { 0x35, 0x01, {0x96}},
    { 0x36, 0x01, {0xAA}},
    { 0x37, 0x01, {0xDB}},
    { 0x38, 0x01, {0xCC}},
    { 0x39, 0x01, {0xBD}},
    { 0x3A, 0x01, {0x78}},
    { 0x3B, 0x01, {0x69}},
    { 0x3C, 0x01, {0x22}},
    { 0x3D, 0x01, {0x22}},
    { 0x3E, 0x01, {0x22}},
    { 0x3F, 0x01, {0x22}},
    { 0x40, 0x01, {0x22}},
    { 0x52, 0x01, {0x10}},
    { 0x53, 0x01, {0x10}},
    { 0xFF, 0x05, {0xFF, 0x98, 0x06, 0x04, 0x07}},
    { 0x18, 0x01, {0x1D}},
    { 0x17, 0x01, {0x32}},
    { 0x02, 0x01, {0x77}},
    { 0xE1, 0x01, {0x79}},
    { 0xFF, 0x05, {0xFF, 0x98, 0x06, 0x04, 0x00}},
    { 0x3A, 0x01, {0x77}},
    { 0x11, 0x01, {0x00}},
    { REGFLAG_DELAY, 0x78, {0x00}},
    { 0x29, 0x01, {0x00}},
    { REGFLAG_DELAY, 0x1E, {0x00}},
    { REGFLAG_END_OF_TABLE, 0x00, {0x00}}
};
    
static struct LCM_setting_table lcm_sleep_in_setting[] = {
// unk_C081D42C
    { 0xFF, 0x05, {0xFF, 0x98, 0x06, 0x04, 0x00}},
    { 0x28, 0x01, {0x00}},
    { REGFLAG_DELAY, 0x32, {0x00}},
    { 0x10, 0x01, {0x00}},
    { REGFLAG_DELAY, 0x78, {0x00}},
    { REGFLAG_END_OF_TABLE, 0x00, {0x00}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
  unsigned int i;

  for (i = 0; i < count; i++)
  {
    unsigned cmd;
    cmd = table[i].cmd;

    switch (cmd)
    {
    case REGFLAG_DELAY:
      MDELAY(table[i].count);
      break;

    case REGFLAG_END_OF_TABLE:
      break;

    default:
      dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
      MDELAY(10);
    }
  }
}

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
  memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
  memset(params, 0, sizeof(LCM_PARAMS));
  
  params->dbi.te_edge_polarity = 0;
  params->dsi.data_format.color_order = 0;
  params->dsi.data_format.trans_seq = 0;
  params->dsi.data_format.padding = 0;
  params->type = 2;
  params->dsi.word_count = 1440;
  params->width = 480;
  params->dsi.vertical_backporch = 28;
  params->height = 854;
  params->dsi.horizontal_sync_active = 10;
  params->dbi.te_mode = 1;
  params->dsi.horizontal_backporch = 50;
  params->dsi.mode = 1;
  params->dsi.horizontal_frontporch = 100;
  params->dsi.LANE_NUM = 2;
  params->dsi.horizontal_blanking_pixel = 120;
  params->dsi.data_format.format = 2;
  params->dsi.intermediat_buffer_num = 2;
  params->dsi.PS = 2;
  params->dsi.vertical_active_line = 854;
  params->dsi.vertical_sync_active = 2;
  params->dsi.vertical_frontporch = 2;
  params->dsi.horizontal_active_pixel = 480;
  params->dsi.pll_div1 = 1;
  params->dsi.pll_div2 = 1;
  params->dsi.fbk_sel = 1;
  params->dsi.fbk_div = 30;
  params->dsi.noncont_clock = 1;
  params->dsi.noncont_clock_period = 2;
}

static void lcm_init(void)
{
  SET_RESET_PIN(1);
  MDELAY(15);
  SET_RESET_PIN(0);
  MDELAY(25);
  SET_RESET_PIN(1);
  MDELAY(120);

  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
  push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
  lcm_init();
}

static unsigned int lcm_compare_id(void)
{
	return 1;
/*
  int lcm_adc; // [sp+4h] [bp-80h]
  unsigned char buffer[5]; // [sp+Bh] [bp-79h]
  int arr[4]; // [sp+20h] [bp-64h]
  unsigned int data_array[16]; // [sp+30h] [bp-54h]

  lcm_adc = 0;
  arr[0] = 0;
  arr[1] = 0;
  arr[2] = 0;
  arr[3] = 0;
  SET_RESER_PIN(1);
  SET_RESER_PIN(0);
  MDELAY(10);
  SET_RESER_PIN(1);
  MDELAY(120);
  data_array[0] = 0x43902;
  data_array[1] = 0x1698FFFF;
  dsi_set_cmdq(data_array, 2, 1);
  MDELAY(10);
  data_array[0] = 0x33700;
  dsi_set_cmdq(data_array, 1, 1);
  MDELAY(10);
  g_LCM_UTIL_FUNCS.dsi_dcs_read_lcm_reg_v2(0xD3, buffer, 3);
  wtf_f(1, arr, &lcm_adc);
  printk("ili9806 kernel buffer[1] == %0x\n", buffer[1]);
  printk("ili9806 kernel buffer[2] == %0x\n", buffer[2]);
  printk("ili9806 kernel lcm_adc txd== %0x\n", lcm_adc);
  return lcm_adc <= 2999;
*/
}

LCM_DRIVER ili9806e_dsi_vdo_fwvga_txd_zaw1800_lcm_drv =
{
  .name = "ili9806e_dsi_vdo_fwvga_txd_zaw1800",
  .set_util_funcs = lcm_set_util_funcs,
  .get_params = lcm_get_params,
  .init = lcm_init,
  .suspend = lcm_suspend,
  .resume = lcm_resume,
  .compare_id = lcm_compare_id
};
