#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "stm32f429i_discovery_l3gd20.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ioe.h"
#include "stm32f4xx_rng.h"
#include "fio.h"
#include "clib.h"

//block
int16_t width = LCD_PIXEL_WIDTH/4;
int16_t height = LCD_PIXEL_WIDTH/4;
int block[4][4] = {0};
int blocktmp[4][4] = {0};
int layer = 1;
int pos1_x;
int pos1_y;
int pos2_x;
int pos2_y;
int score = 0;
int scoretmp = 0;
int flag = 0;
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

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG,ENABLE);
	RNG_Cmd(ENABLE);

}

void blockInit(){
	uint32_t rnd;
	int pos1;
	int pos2;

	while(!RNG_GetFlagStatus(RNG_FLAG_DRDY));
	rnd = RNG_GetRandomNumber();
	pos1 = rnd%16;

	while(!RNG_GetFlagStatus(RNG_FLAG_DRDY));
	rnd = RNG_GetRandomNumber();
	pos2 = rnd%16;
	
	int	pos1_x = pos1/4;
	int pos1_y = pos1%4;
	block[pos1_x][pos1_y] = 2;
	fio_printf(1,"pos1_x : %d pos1_y : %d\r\n",pos1_x,pos1_y);
	int pos2_x = pos2/4;
	int pos2_y = pos2%4;
	block[pos2_x][pos2_y] = 2;
	fio_printf(1,"pos2_x : %d pos2_y : %d\r\n",pos2_x,pos2_y);
	for(int i = 0 ;i < 4;i++){
		for(int j = 0 ; j < 4;j++){
			fio_printf(1,"%d ",block[i][j]);
			blocktmp[i][j] = block[i][j];
		}
		fio_printf(1,"\r\n");
	}
}

void random_gen_block(){
	int i;
	int j = 0;
	int k = 0;
	uint32_t rnd;
	uint32_t n;
	int arr[16];
	uint32_t n_x;
	uint32_t n_y;
	for(i = 0 ; i < 16;i++){
		arr[i] = 0;
	}
	for(i = 0 ; i < 4;i++){
		for(j = 0 ; j < 4;j++){
			fio_printf(1,"%d ",block[i][j]);
		}
		fio_printf(1,"\r\n");
	}
	//find the empty block
	for(i = 0 ;i < 4;i++){
		for(j = 0 ; j < 4;j++){
			if(block[i][j]==0){
				arr[k] = i*4+j;
				k++;
			}
		}
	}
	fio_printf(1,"k = %d\r\n",k);
	for(i = 0 ; i<16;i++){
		fio_printf(1,"%d ",arr[i]);
	}
	fio_printf(1,"\r\n");
	//get empty block and init
	while(!RNG_GetFlagStatus(RNG_FLAG_DRDY));
	rnd = RNG_GetRandomNumber();
	n = rnd % k;
	n_x = arr[n]/4;
	n_y = arr[n]%4;

	fio_printf(1,"rnd = %d \r\n",rnd);
	fio_printf(1,"n = %d \r\n",n);
	fio_printf(1,"n_x = %d n_y = %d \r\n",n_x,n_y);
	fio_printf(1,"block = %d\r\n",block[n_x][n_y]);
	if(n%2 == 0)
		block[n_x][n_y] = 2;
	else
		block[n_x][n_y] = 4;
}
static char* itoa1(int value, char* result, int base)
{
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}
	char *ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value     - value * base)];
	} while (value);

	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}


void draw_block(){
	LCD_SetBackColor(LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_RED);
	LCD_DisplayStringLine(LCD_LINE_0,"     2048");
	char str[16] = "   SCORE:";
	itoa1(score,str+9,10);
	LCD_DisplayStringLine(LCD_LINE_1,str);
		
	for(int i = 0 ; i < 4;i++){
		for(int j = 0 ; j < 4;j++){
			if(block[i][j]>0){
				LCD_SetBackColor(LCD_COLOR_WHITE);
				LCD_SetTextColor(LCD_COLOR_WHITE);
				LCD_DrawFullRect(i*60,j*60+80,width,height);
				LCD_SetTextColor(LCD_COLOR_RED);
				int tmp = block[i][j];
				int prefix = 15;
				int itr = 0;
				while(tmp>0){
					LCD_DisplayChar(j*60+20+80,i*60+40-prefix*itr,tmp%10+48);
					tmp/=10;
					itr++;
				}
				if(block[i][j]==128)
					flag = 1;
				else if(block[i][j]==1024)
					flag = 2;
				else if(block[i][j] == 2048)
					flag = 3;
			}
		}
	}
	if(flag == 1){
		LCD_SetBackColor(LCD_COLOR_BLACK);
		LCD_SetTextColor(LCD_COLOR_BLUE);
		LCD_DisplayStringLine(LCD_LINE_2,"keep going!");
	}else if(flag == 2){
		LCD_SetBackColor(LCD_COLOR_BLACK);
		LCD_SetTextColor(LCD_COLOR_BLUE);
		LCD_DisplayStringLine(LCD_LINE_2,"almost done!");
	}else if(flag ==3){
		LCD_SetBackColor(LCD_COLOR_BLACK);
		LCD_SetTextColor(LCD_COLOR_BLUE);
		LCD_DisplayStringLine(LCD_LINE_2,"ya good job!");
	}
	LCD_SetBackColor(LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_RED);
	for(int i = 0 ;i < 5;i++){
		LCD_DrawLine(0,i*60+80,LCD_PIXEL_WIDTH,LCD_DIR_HORIZONTAL);
		LCD_DrawLine(i*60,80,LCD_PIXEL_WIDTH,LCD_DIR_VERTICAL);
	}
}

void cal_block(int direction){
	int i;
	int j;
	int w;
	int x = 0;
	int k = 0;
	int tmp[4]={0};
	int new[4];
	int sum = 0;
	switch(direction){
		case 1://down
			for(i = 0;i<4;i++){
				for(j = 0;j < 4;j++){
					if(block[i][j]>0){
						tmp[k] = block[i][j];
						k++;
						block[i][j] = 0;
					}
				}
				for(w = k-1;w>=0;w--){
					if(w-1>=0 && tmp[w]==tmp[w-1]){
						new[x] = tmp[w]*2;
						score+= tmp[w]*2;
						w = w-1;
						x++;
					}else{
						new[x] = tmp[w];
						x++;
					}
				}
				int itr = 0;
				while(itr<x){
					block[i][3-itr] = new[itr];
					itr++;
				}
				x = 0;
				k = 0;
			}
			
			break;
		case 2://up
			for(i = 0;i<4;i++){
				for(j = 0;j < 4;j++){
					if(block[i][j]>0){
						tmp[k] = block[i][j];
						k++;
						block[i][j] = 0;
					}
				}
				for(w = 0;w < k;w++){
					if(w+1<=k && tmp[w]==tmp[w+1]){
						new[x] = tmp[w]*2;
						score += tmp[w]*2;
						w = w+1;
						x++;
					}else{
						new[x] = tmp[w];
						x++;
					}
				}
				int itr = 0;
				while(itr<x){
					block[i][itr] = new[itr];
					itr++;
				}
				x = 0;
				k = 0;
			}
			
			break;

		case 3://right
			for(j = 0;j<4;j++){
				for(i = 0;i < 4;i++){
					if(block[i][j]>0){
						tmp[k] = block[i][j];
						k++;
						block[i][j] = 0;
					}
				}
				for(w = k-1;w>=0;w--){
					if(w-1>=0 && tmp[w]==tmp[w-1]){
						new[x] = tmp[w]*2;
						score += tmp[w]*2;
						w = w-1;
						x++;
					}else{
						new[x] = tmp[w];
						x++;
					}
				}
				int itr = 0;
				while(itr<x){
					block[3-itr][j] = new[itr];
					itr++;
				}
				k = 0;
				x = 0;
			}
			
			break;

		case 4://left
			for(j = 0;j<4;j++){
				for(i = 0;i < 4;i++){
					if(block[i][j]>0){
						tmp[k] = block[i][j];
						k++;
						block[i][j] = 0;
					}
				}
				for(w =0;w < k;w++){
					if(w+1<=k && tmp[w]==tmp[w+1]){
						new[x] = tmp[w]*2;
						score += tmp[w]*2;
						w = w+1;
						x++;
					}else{
						new[x] = tmp[w];
						x++;
					}
				}
				int itr = 0;
				while(itr<x){
					block[itr][j] = new[itr];
					itr++;
				}
				k = 0;
				x = 0;
			}
			
			break;

	}
}
void undo(){
	for(int m = 0 ; m < 4;m++){
		for(int n = 0;n < 4;n++){
			block[m][n] = blocktmp[m][n];
			score = scoretmp;
		}
	}

}

void update(){

	uint8_t tmp[6] = {0};                                                   
	int16_t a[3] = {0};
	uint8_t tmpreg = 0;

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_DrawFullRect(0,0,240,320);
	L3GD20_Read(&tmpreg, L3GD20_CTRL_REG4_ADDR, 1);
	L3GD20_Read(tmp, L3GD20_OUT_X_L_ADDR, 6);

	/* check in the control register 4 the data alignment (Big Endian or Little Endian)*/
	if (!(tmpreg & 0x40)) {
		for (int i = 0; i < 3; i++){
			a[i] = (int16_t)(((uint16_t)tmp[2 * i + 1] << 8) | (uint16_t)tmp[2 * i]);
			a[i] /= 114.85f;
		}
	} else {
		for (int i = 0; i < 3; i++){
			a[i] = (int16_t)(((uint16_t)tmp[2 * i] << 8) | (uint16_t)tmp[2 *i + 1]);
			a[i] /= 114.85f;
		}
	}
	draw_block();
	if(a[0]>250){//down
		for(int m = 0;m<4;m++){
			for(int n = 0 ; n <4;n++){
				blocktmp[m][n] = block[m][n];
			}
		}
		scoretmp = score;
		fio_printf(1,"down!\r\n");
		cal_block(1);
		random_gen_block();
		draw_block();
		vTaskDelay(5);
	}
	if(a[0]<-250){ //up
		for(int m = 0;m<4;m++){
			for(int n = 0 ; n <4;n++){
				blocktmp[m][n] = block[m][n];
			}
		}
		scoretmp = score;
		fio_printf(1,"up!\r\n");
		cal_block(2);
		random_gen_block();
		draw_block();
		vTaskDelay(5);
	}
	if(a[1]>250){//right
		for(int m = 0;m<4;m++){
			for(int n = 0 ; n <4;n++){
				blocktmp[m][n] = block[m][n];
			}
		}
		scoretmp = score;
		fio_printf(1,"right!\r\n");
		cal_block(3);
		random_gen_block();
		draw_block();
		vTaskDelay(5);
	}
	if(a[1]<-250){//left
		for(int m = 0;m<4;m++){
			for(int n = 0 ; n <4;n++){
				blocktmp[m][n] = block[m][n];
			}
		}
		scoretmp = score;
		fio_printf(1,"left!\r\n");
		cal_block(4);
		random_gen_block();
		draw_block();
		vTaskDelay(5);
	}
	if(a[2] > 200){//undo
		fio_printf(1,"undo\r\n");
		undo();
		draw_block();
		vTaskDelay(5);
	}
	
}

void render(){
	if(layer == 1){
		LCD_SetTransparency(0xFF);
		LCD_SetLayer(LCD_BACKGROUND_LAYER);
		LCD_SetTransparency(0x00);
		layer = 2;
	}else{
		LCD_SetTransparency(0xFF);
		LCD_SetLayer(LCD_FOREGROUND_LAYER);
		LCD_SetTransparency(0x00);
		layer = 1;
	}
}

uint32_t L3GD20_TIMEOUT_UserCallback(void)
{
	return 0;
}
