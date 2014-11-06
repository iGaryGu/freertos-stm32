#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "stm32f429i_discovery_l3gd20.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ioe.h"
#include "fio.h"
#include "clib.h"

//block
int16_t width = LCD_PIXEL_WIDTH/4;
int16_t height = LCD_PIXEL_HEIGHT/4;
int16_t blockX = 0;
int16_t blockY = 0;

void init(){
	L3GD20_InitTypeDef L3GD20_InitStructure;
	L3GD20_InitStructure.Power_Mode = L3GD20_MODE_ACTIVE;
	L3GD20_InitStructure.Output_DataRate = L3GD20_OUTPUT_DATARATE_1;
	L3GD20_InitStructure.Axes_Enable = L3GD20_AXES_ENABLE;
	L3GD20_InitStructure.Band_Width = L3GD20_BANDWIDTH_4;
	L3GD20_InitStructure.BlockData_Update = L3GD20_BlockDataUpdate_Continous    ;
	L3GD20_InitStructure.Endianness = L3GD20_BLE_LSB;
	L3GD20_InitStructure.Full_Scale = L3GD20_FULLSCALE_250;
	L3GD20_Init(&L3GD20_InitStructure);
	L3GD20_FilterConfigTypeDef L3GD20_FilterStructure;
	L3GD20_FilterStructure.HighPassFilter_Mode_Selection = L3GD20_HPM_NORMAL_MODE_RES;
	L3GD20_FilterStructure.HighPassFilter_CutOff_Frequency = L3GD20_HPFCF_0;
	L3GD20_FilterConfig(&L3GD20_FilterStructure);
	L3GD20_FilterCmd(L3GD20_HIGHPASSFILTER_ENABLE);

}

void update(){
	uint8_t tmp[6] = {0};                                                   
	int16_t a[3] = {0};
	uint8_t tmpreg = 0;

	L3GD20_Read(&tmpreg, L3GD20_CTRL_REG4_ADDR, 1);
	L3GD20_Read(tmp, L3GD20_OUT_X_L_ADDR, 6);

	/* check in the control register 4 the data alignment (Big Endian or Lit    tle Endian)*/
	if (!(tmpreg & 0x40)) {
		for (int i = 0; i < 3; i++)
			a[i] = (int16_t)(((uint16_t)tmp[2 * i + 1] << 8) | (uint16_t)tmp[2 * i]);
	} else {
		for (int i = 0; i < 3; i++)
			a[i] = (int16_t)(((uint16_t)tmp[2 * i] << 8) | (uint16_t)tmp[2 *i + 1]);
	}

}

void render(){
	LCD_SetBackColor(LCD_COLOR_BLACK);
	LCD_SetTextColor( LCD_COLOR_WHITE );
	LCD_DrawFullRect( blockX, blockY, width, height );
}

uint32_t L3GD20_TIMEOUT_UserCallback(void)
{
	return 0;
}
