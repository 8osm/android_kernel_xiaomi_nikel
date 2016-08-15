/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 IMX386mipi_sensor.c
 *
 * Project:
 * --------
 *	 ALPS MT6797
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 *	

 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_typedef.h"
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "imx386sunnymipi_sensor.h"

/*===FEATURE SWITH===*/
 // #define FPTPDAFSUPPORT   //for pdaf switch
 // #define FANPENGTAO   //for debug log
 #define LOG_INF LOG_INF_LOD
 //#define NONCONTINUEMODE
 #define PDAF_FIX_WINDOW
/*===FEATURE SWITH===*/

/****************************Modify Following Strings for Debug****************************/
#define PFX "imx386_camera_sunny"
#define LOG_INF_LOD(format, args...)    pr_info(PFX "[%s] " format, __FUNCTION__, ##args)
#define LOG_1 LOG_INF("IMX386,MIPI 4LANE\n")
#define SENSORDB LOG_INF
/****************************   Modify end    *******************************************/

static DEFINE_SPINLOCK(imgsensor_drv_lock);

u8 *sunny_otp_buf;
#define	MAX_READ_WRITE_SIZE	8
#define	OTP_DATA_SIZE	2153
#define	OTP_START_ADDR	0x0000
#define	E2PROM_WRITE_ID	0xA0
#define SPC1_START_ADDR	0x7A7
#define SPC2_START_ADDR	0x7D7
#define SPC_DATA_SIZE	48
#define PDAF_CAL_DATA_SIZE	96
#define PDAF_CAL_DATA_OFFSET	0x807

struct imx386_write_buffer {
	u8 addr[2];
	u8 data[MAX_READ_WRITE_SIZE];
};

extern int main_module_id;
static kal_uint8 mode_change = 0;
static imgsensor_info_struct imgsensor_info = { 
	.sensor_id = IMX386SUNNY_SENSOR_ID & 0x0FFF,		//Sensor ID Value: 0x30C8//record sensor id defined in Kd_imgsensor.h
	
	.checksum_value =  0xe1b26f6c,		//checksum value for Camera Auto Test
	
	.pre = {
		.pclk = 233300000,				//record different mode's pclk
		.linelength  = 4296,				//record different mode's linelength
		.framelength = 1780,			//record different mode's framelength
		.startx= 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 2016,		//record different mode's width of grabwindow
		.grabwindow_height = 1508,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,	
	},
	.cap = {
		.pclk = 433300000,				//record different mode's pclk
		.linelength  = 4296,				//record different mode's linelength
		.framelength = 3300,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 4032,		//record different mode's width of grabwindow
		.grabwindow_height = 3016,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,	
	},
	.cap1 = {							//capture for PIP 24fps relative information, capture1 mode must use same framelength, linelength with Capture mode for shutter calculate
		.pclk = 400000000,				//record different mode's pclk
		.linelength  = 4704,				//record different mode's linelength
		.framelength = 3536,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 4208,		//record different mode's width of grabwindow
		.grabwindow_height = 3120,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 240,	
	},
	.custom1 = {
		.pclk = 300000000,				//record different mode's pclk
		.linelength  = 4296,				//record different mode's linelength
		.framelength = 2302,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 4032,		//record different mode's width of grabwindow
		.grabwindow_height = 2256,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 233300000,				//record different mode's pclk
		.linelength  = 4296,				//record different mode's linelength
		.framelength = 1780,			//record different mode's framelength
		.startx= 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 2016,		//record different mode's width of grabwindow
		.grabwindow_height = 1508,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,	
	}, 
	.hs_video = {
		.pclk = 600000000,				//record different mode's pclk
		.linelength  = 4296,				//record different mode's linelength
		.framelength = 1160,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 1296,		//record different mode's width of grabwindow
		.grabwindow_height = 736,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 1200,	
	},
	.slim_video = {
		.pclk = 600000000,				//record different mode's pclk
		.linelength  = 4296,				//record different mode's linelength
		.framelength = 4648,			//record different mode's framelength
		.startx = 0,					//record different mode's startx of grabwindow
		.starty = 0,					//record different mode's starty of grabwindow
		.grabwindow_width  = 1296,		//record different mode's width of grabwindow
		.grabwindow_height = 736,		//record different mode's height of grabwindow
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,	
	},

	.margin = 10,			//sensor framelength & shutter margin
	.min_shutter = 2,		//min shutter
	.max_frame_length = 0xFFFE,//REG0x0202 <=REG0x0340-5//max framelength by sensor register's limitation
	.ae_shut_delay_frame = 0,	//shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2
	.ae_sensor_gain_delay_frame = 0,//sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2
	.ae_ispGain_delay_frame = 2,//isp gain delay frame for AE cycle
	.ihdr_support = 0,	  //1, support; 0,not support
	.ihdr_le_firstline = 0,  //1,le first ; 0, se first
	.sensor_mode_num = 6,	  //support sensor mode num ,don't support Slow motion
	
	.cap_delay_frame = 0,		//enter capture delay frame num
	.pre_delay_frame = 0, 		//enter preview delay frame num
	.custom1_delay_frame = 0,		//enter capture delay frame num
	.video_delay_frame = 0,		//enter video delay frame num
	.hs_video_delay_frame = 0,	//enter high speed video  delay frame num
	.slim_video_delay_frame = 0,//enter slim video delay frame num

	.isp_driving_current = ISP_DRIVING_8MA, //mclk driving current
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,//sensor_interface_type
	.mipi_sensor_type = MIPI_OPHY_NCSI2, //0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2
	.mipi_settle_delay_mode = MIPI_SETTLEDELAY_MANUAL,//0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_R,//sensor output first pixel color
	.mclk = 24,//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
	.mipi_lane_num = SENSOR_MIPI_4_LANE,//mipi lane num
	.i2c_addr_table = {0x20, 0xff},//record sensor support all write id addr, only supprt 4must end with 0xff
};


static imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,				//mirrorflip information
	.sensor_mode = IMGSENSOR_MODE_INIT, //IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video,High Speed Video, Slim Video
	.shutter = 0x200,					//current shutter
	.gain = 0x200,						//current gain
	.dummy_pixel = 0,					//current dummypixel
	.dummy_line = 0,					//current dummyline
	.current_fps = 0,  //full size current fps : 24fps for PIP, 30fps for Normal or ZSD
	.autoflicker_en = KAL_FALSE,  //auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker
	.test_pattern = KAL_FALSE,		//test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,//current scenario id
	.ihdr_en = KAL_FALSE, //sensor need support LE, SE with HDR feature
	.i2c_write_id = 0x20,//record current sensor's i2c write id
};

/*VC1 for HDR(DT=0X35) , VC2 for PDAF(DT=0X36), unit : 10bit*/
static SENSOR_VC_INFO_STRUCT SENSOR_VC_INFO[3]=
{/* Preview mode setting */
 {0x02, 0x0a,	 0x00,	 0x08, 0x40, 0x00,
  0x00, 0x2b, 0x7E0, 0x05E4, 0x01, 0x00, 0x0000, 0x0000,
  0x00, 0x36, 0x09D8, 0x0001, 0x03, 0x00, 0x0000, 0x0000},
  /* Capture mode setting */
 {0x03, 0x0a,	 0x00,	 0x08, 0x40, 0x00,
  0x00, 0x2b, 0x0FC0, 0x0BC8, 0x00, 0x35, 0x0280, 0x0001,
  0x00, 0x36, 0x13B0, 0x0001, 0x03, 0x00, 0x0000, 0x0000},
  /* Video mode setting */
 {0x02, 0x0a,	 0x00,	 0x08, 0x40, 0x00,
  0x00, 0x2b, 0x7E0, 0x05E4, 0x00, 0x35, 0x0280, 0x0001,
  0x00, 0x36, 0x09D8, 0x0001, 0x03, 0x00, 0x0000, 0x0000},
    /*Slim Video1  mode setting */
 {0x02, 0x0a,	 0x00,	 0x08, 0x40, 0x00,
  0x00, 0x2b, 0x7E0, 0x05E4, 0x00, 0x35, 0x0280, 0x0001,
  0x00, 0x36, 0x0654, 0x0001, 0x03, 0x00, 0x0000, 0x0000},
    /* Slim Video2 mode setting */
 {0x02, 0x0a,	 0x00,	 0x08, 0x40, 0x00,
  0x00, 0x2b, 0x7E0, 0x05E4, 0x00, 0x35, 0x0280, 0x0001,
  0x00, 0x36, 0x09D8, 0x0001, 0x03, 0x00, 0x0000, 0x0000},
    /* Custom1 mode setting */
 {0x02, 0x0a,	 0x00,	 0x08, 0x40, 0x00,
  0x00, 0x2b, 0x0FC0, 0x08D0, 0x00, 0x35, 0x0280, 0x0001,//0x00, 0x2b, 0x7E0, 0x05E4, 0x00, 0x35, 0x0280, 0x0001,
  0x00, 0x36, 0x13B0, 0x0001, 0x03, 0x00, 0x0000, 0x0000},
};

/* Sensor output window information*/
static SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[6] =
{
 { 4032, 3016,	  0,  	0, 4032, 3016, 2016, 1508,   0,	0, 2016, 1508, 	 0, 0, 2016, 1508}, // Preview 
 { 4032, 3016,	  0,  	0, 4032, 3016, 4032, 3016,   0,	0, 4032, 3016, 	 0, 0, 4032, 3016}, // capture
 { 4032, 3016,	  0,  	0, 4032, 3016, 2016, 1508,   0,	0, 2016, 1508, 	 0, 0, 2016, 1508}, // normal video 
 { 4032, 3016,	720,  772, 2592, 1472, 1296, 736,    0, 0, 1296, 736,  0, 0, 1296, 736}, // high speed video
 { 4032, 3016,	720,  772, 2592, 1472, 1296, 736,    0, 0, 1296, 736,  0, 0, 1296, 736}, // slim video
 { 4032, 3016,	  0,  380, 4032, 2256, 4032, 2256,   0,	0, 4032, 2256, 	 0, 0, 4032, 2256}, // custom1
};

 static SET_PD_BLOCK_INFO_T imgsensor_pd_info =
 //for 2M8 non mirror flip
{
 
    .i4OffsetX = 28,
 
    .i4OffsetY = 31,
 
    .i4PitchX = 64,
 
    .i4PitchY = 64,
 
    .i4PairNum =16,
 
    .i4SubBlkW =16,
 
    .i4SubBlkH =16, 
 
    .i4PosL = {{28,31},{80,31},{44,35},{64,35},{32,51},{76,51},{48,55},{60,55},{48,63},{60,63},{32,67},{76,67},{44,83},{64,83},{28,87},{80,87}},
 
    .i4PosR = {{28,35},{80,35},{44,39},{64,39},{32,47},{76,47},{48,51},{60,51},{48,67},{60,67},{32,71},{76,71},{44,79},{64,79},{28,83},{80,83}},
 
};


// //for 2M8 mirror flip
//{
// 
//    .i4OffsetX = 31,
// 
//    .i4OffsetY = 24,
// 
//    .i4PitchX = 64,
// 
//    .i4PitchY = 64,
// 
//    .i4PairNum =16,
// 
//    .i4SubBlkW =16,
// 
//    .i4SubBlkH =16, 
// 
//    .i4PosL = {{31,28},{83,28},{47,32},{67,32},{35,40},{79,40},{51,44},{63,44},{51,60},{63,60},{35,64},{79,64},{47,72},{67,72},{31,76},{83,76}},
// 
//    .i4PosR = {{31,24},{83,24},{47,28},{67,28},{35,44},{79,44},{51,48},{63,48},{51,56},{63,56},{35,60},{79,60},{47,76},{67,76},{31,80},{83,80}},
// 
//};

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
    char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

    iReadRegI2C(pu_send_cmd, 2, (u8*)&get_byte, 1, imgsensor.i2c_write_id);
    return get_byte;
}

static void write_cmos_sensor_burst(kal_uint32 addr, u8 *reg_buf, kal_uint32 size)
{
	struct imx386_write_buffer buf;
	int i;
	int ret;

	for (i = 0; i < size; i += MAX_READ_WRITE_SIZE) {
		buf.addr[0] = (u8)(addr >> 8);
		buf.addr[1] = (u8)(addr & 0xFF);
		if ((i + MAX_READ_WRITE_SIZE) > size) {
			memcpy(buf.data, (reg_buf + i), (size - i));
			ret = iBurstWriteReg((u8 *)&buf, (size - i + 2), imgsensor.i2c_write_id);
		} else {
			memcpy(buf.data, (reg_buf + i), MAX_READ_WRITE_SIZE);
			ret = iBurstWriteReg((u8 *)&buf, (MAX_READ_WRITE_SIZE + 2), imgsensor.i2c_write_id);
		}

		if (ret < 0)
			LOG_INF("write burst reg into sensor failed!\n");

		addr += MAX_READ_WRITE_SIZE;
	}
}

static int imx386_read_otp(u16 addr, u8 *buf)
{
	int ret = 0;
	u8 pu_send_cmd[2] = {(u8)(addr >> 8), (u8)(addr & 0xFF)};

	ret = iReadRegI2C(pu_send_cmd, 2, (u8*)buf, 1, E2PROM_WRITE_ID);
	if (ret < 0)
		LOG_INF("read data from imx386 sunny otp e2prom failed!\n");

	return ret;
}

static void imx386_read_otp_burst(u16 addr, u8 *otp_buf)
{
	int i;
	int ret;
	u8 pu_send_cmd[2];

	for (i = 0; i < OTP_DATA_SIZE; i += MAX_READ_WRITE_SIZE) {
		pu_send_cmd[0] = (u8)(addr >> 8);
		pu_send_cmd[1] = (u8)(addr & 0xFF);

		if (i + MAX_READ_WRITE_SIZE > OTP_DATA_SIZE)
			ret = iReadRegI2C(pu_send_cmd, 2, (u8 *)(otp_buf + i), (OTP_DATA_SIZE - i), E2PROM_WRITE_ID);
		else
			ret = iReadRegI2C(pu_send_cmd, 2, (u8 *)(otp_buf + i), MAX_READ_WRITE_SIZE, E2PROM_WRITE_ID);

		if (ret < 0)
			LOG_INF("read lsc table from imx386 sunny otp e2prom failed!\n");

		addr += MAX_READ_WRITE_SIZE;
	}
}

static void read_imx386_pdaf_data(kal_uint16 addr, BYTE* data, kal_uint32 size)
{
	memcpy((void *)data,(void *)(sunny_otp_buf + PDAF_CAL_DATA_OFFSET), PDAF_CAL_DATA_SIZE);
}

static void write_cmos_sensor_w(kal_uint16 addr, kal_uint16 para)
{
    char pusendcmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para >> 8), (char)(para & 0xFF)};

    iWriteRegI2C(pusendcmd, 4, imgsensor.i2c_write_id);
}

static void write_cmos_sensor(kal_uint16 addr, kal_uint16 para)
{
    char pusendcmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};

    iWriteRegI2C(pusendcmd, 3, imgsensor.i2c_write_id);
}

static MUINT32 cur_startpos[2];
static MUINT32 cur_size[2];

static void imx386_set_pd_focus_area(MUINT32 startpos, MUINT32 size, MUINT32 Idx)
{
	UINT16 start_x_pos, start_y_pos, end_x_pos, end_y_pos;
	UINT16 focus_width, focus_height;

	if((cur_startpos[1] == startpos) && (cur_size[1] == size) && (Idx == 1))
	{
		LOG_INF("Not to need update focus area!\n");
		return;
	}
	else
	{
		switch(Idx)
		{
		case 0:
			cur_startpos[0] = startpos;
			cur_size[0] = size;
			break;
		case 1:
			cur_startpos[1] = startpos;
			cur_size[1] = size;
			break;			
		}
	}

	start_x_pos = (startpos >> 16) & 0xFFFF;
	start_y_pos = startpos & 0xFFFF;
	focus_width = (size >> 16) & 0xFFFF;
	focus_height = size & 0xFFFF;

	end_x_pos = start_x_pos + focus_width;
	end_y_pos = start_y_pos + focus_height;

		/* GC pre PDAF */
		/*PDAF*/
		/*PD_CAL_ENALBE*/
		write_cmos_sensor(0x3047,0x01);
		/*AREA MODE*/
		write_cmos_sensor(0x315D,0x02);// 8x6 output
		write_cmos_sensor(0x315E,0x01);// 8x6 output
		write_cmos_sensor(0x315F,0x01);// 8x6 output
		/*PD_OUT_EN=1*/
		write_cmos_sensor(0x3049,0x01);

		/*Fixed area mode*/
		if(Idx == 0)
		{
			write_cmos_sensor(0x3108,(start_x_pos >> 8) & 0xFF);
			write_cmos_sensor(0x3109,start_x_pos & 0xFF);// X start
			write_cmos_sensor(0x310a,(start_y_pos >> 8) & 0xFF);
			write_cmos_sensor(0x310b,start_y_pos & 0xFF);// Y start
			write_cmos_sensor(0x310c,(end_x_pos >> 8) & 0xFF);
			write_cmos_sensor(0x310d,end_x_pos & 0xFF);//X end 
			write_cmos_sensor(0x310e,(end_y_pos >> 8) & 0xFF);
			write_cmos_sensor(0x310f,end_y_pos & 0xFF);// Y end
		}
		else if(Idx == 1)
		{
			write_cmos_sensor(0x3110,(start_x_pos >> 8) & 0xFF);
			write_cmos_sensor(0x3111,start_x_pos & 0xFF);// X start
			write_cmos_sensor(0x3112,(start_y_pos >> 8) & 0xFF);
			write_cmos_sensor(0x3113,start_y_pos & 0xFF);// Y start
			write_cmos_sensor(0x3114,(end_x_pos >> 8) & 0xFF);
			write_cmos_sensor(0x3115,end_x_pos & 0xFF);//X end 
			write_cmos_sensor(0x3116,(end_y_pos >> 8) & 0xFF);
			write_cmos_sensor(0x3117,end_y_pos & 0xFF);// Y end
		}

	LOG_INF("Idx:%d, start_x_pos:%d, start_y_pos:%d, focus_width:%d, focus_height:%d, end_x_pos:%d, end_y_pos:%d\n", \
			Idx, start_x_pos, start_y_pos, focus_width, focus_height, end_x_pos, end_y_pos);

	return;
}

static void load_imx386_spc_data(void)
{
	kal_uint16 i;
	for(i = 0; i < SPC_DATA_SIZE; i++)
	{
		write_cmos_sensor(0x7D4C+i, sunny_otp_buf[SPC1_START_ADDR + i]);
	}
	for(i = 0; i < SPC_DATA_SIZE; i++)
	{
		write_cmos_sensor(0x7D80+i, sunny_otp_buf[SPC2_START_ADDR + i]);
	}
}

static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line, imgsensor.dummy_pixel);
	/* you can set dummy by imgsensor.dummy_line and imgsensor.dummy_pixel, or you can set dummy by imgsensor.frame_length and imgsensor.line_length */
	write_cmos_sensor_w(0x0340, imgsensor.frame_length & 0xFFFF);	  
//	write_cmos_sensor_w(0x0342, imgsensor.line_length & 0xFFFF);
}	/*	set_dummy  */


static void set_max_framerate(UINT16 framerate,kal_bool min_framelength_en)
{
	kal_int16 dummy_line;
	kal_uint32 frame_length = imgsensor.frame_length;
	//unsigned long flags;

	LOG_INF("framerate = %d, min framelength should enable(%d) \n", framerate,min_framelength_en);
   
	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length; 
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	//dummy_line = frame_length - imgsensor.min_frame_length;
	//if (dummy_line < 0)
		//imgsensor.dummy_line = 0;
	//else
		//imgsensor.dummy_line = dummy_line;
	//imgsensor.frame_length = frame_length + imgsensor.dummy_line;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
	{
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */

/*************************************************************************
* FUNCTION
*	set_shutter
*
* DESCRIPTION
*	This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*	iShutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
#define MAX_SHUTTER	12103350		/* 120s long exposure time */
static void set_shutter(kal_uint32 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	kal_uint32 frame_length = 0;
	kal_uint32 line_length=0;
	kal_uint16 long_exp_times = 0;
	kal_uint16 long_exp_shift = 0;

	/* limit max exposure time to be 120s */
	if (shutter > MAX_SHUTTER)
		shutter = MAX_SHUTTER;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);
	
	LOG_INF("enter shutter =%d \n", shutter);
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)		
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	
	/* Just should be called in capture case with long exposure */
        if(shutter == 1)
	{
		/*
		 * return to normal mode from long exposure mode.
		 */
		write_cmos_sensor(0x0100, 0x00);
		write_cmos_sensor(0x3004, 0x00);
		write_cmos_sensor_w(0x0342, imgsensor.line_length & 0xFFFF);
		write_cmos_sensor(0x0100, 0x01);
	}
        if(shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin))
	{
		long_exp_times = shutter / (imgsensor_info.max_frame_length - imgsensor_info.margin);
		if (shutter % (imgsensor_info.max_frame_length - imgsensor_info.margin))
			long_exp_times++;
		if (long_exp_times > 128)
			long_exp_times = 128;
		long_exp_shift = fls(long_exp_times) - 1;
		if (long_exp_times & (~(1 << long_exp_shift)))
			long_exp_shift++;

		long_exp_times = 1 << long_exp_shift;
		write_cmos_sensor(0x3004, long_exp_shift);
		shutter = shutter / long_exp_times;
		if(shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin))
		{
			line_length = shutter * 4296 / (imgsensor_info.max_frame_length - imgsensor_info.margin);
			line_length = (line_length + 1) / 2 * 2;
		}

		spin_lock(&imgsensor_drv_lock);
		if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)		
			imgsensor.frame_length = shutter + imgsensor_info.margin;
		else
			imgsensor.frame_length = imgsensor.min_frame_length;
		if (imgsensor.frame_length > imgsensor_info.max_frame_length)
			imgsensor.frame_length = imgsensor_info.max_frame_length;
		spin_unlock(&imgsensor_drv_lock);

		/* line_length range is 4296 <-> 32766 */
	        if(line_length > 32766)
        	    line_length = 32766;
	        if(line_length < 4296)
        	    line_length = 4296;
		write_cmos_sensor_w(0x0342, line_length & 0xFFFF);
	}

	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;
	if (imgsensor.autoflicker_en) { 
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305) {
			set_max_framerate(296,0);
            		write_cmos_sensor(0x0104, 0x01);
        	} else if(realtime_fps >= 237 && realtime_fps <= 245) {
            		set_max_framerate(236,0);
            		write_cmos_sensor(0x0104, 0x01);
		} else if(realtime_fps >= 147 && realtime_fps <= 150) {
			set_max_framerate(146,0);
            		write_cmos_sensor(0x0104, 0x01);
		} else {
			// Extend frame length
			write_cmos_sensor(0x0104, 0x01);
			write_cmos_sensor_w(0x0340, imgsensor.frame_length & 0xFFFF);
		}
	} else {
		// Extend frame length
        	write_cmos_sensor(0x0104, 0x01);
		write_cmos_sensor_w(0x0340, imgsensor.frame_length & 0xFFFF);
	}

	// Update Shutter
	write_cmos_sensor_w(0x0202, shutter & 0xFFFF);
	write_cmos_sensor(0x0104, 0x00);
	LOG_INF("Exit! shutter=%d, framelength=%d, long_exp_line_length=%d, long_exp_shift:%d\n", shutter,imgsensor.frame_length, line_length, long_exp_shift);
}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0000;
	/* gain = 64 = 1x real gain */
	reg_gain = 512 - (512 * 64 / gain);
	return (kal_uint16)reg_gain;
}

/*************************************************************************
* FUNCTION
*	set_gain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*	iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;
	if (gain < BASEGAIN || gain > 16 * BASEGAIN) {
		LOG_INF("Error gain setting");
		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 16 * BASEGAIN)
			gain = 16 * BASEGAIN;		 
	}

	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain; 
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

	write_cmos_sensor_w(0x0204, (reg_gain&0xFFFF));    
	/*
	 * WORKAROUND! stream on after set shutter/gain, which will get
	 * first valid capture frame.
	 */
	if (mode_change && (imgsensor.sensor_mode == IMGSENSOR_MODE_CAPTURE)) {
		write_cmos_sensor(0x0100, 0x01);
		mode_change = 0;
	}
	return gain;
}	/*	set_gain  */

//ihdr_write_shutter_gain not support for s5k2M8
static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
	LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n",le,se,gain);
	if (imgsensor.ihdr_en) {
		
		spin_lock(&imgsensor_drv_lock);
			if (le > imgsensor.min_frame_length - imgsensor_info.margin)		
				imgsensor.frame_length = le + imgsensor_info.margin;
			else
				imgsensor.frame_length = imgsensor.min_frame_length;
			if (imgsensor.frame_length > imgsensor_info.max_frame_length)
				imgsensor.frame_length = imgsensor_info.max_frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (le < imgsensor_info.min_shutter) le = imgsensor_info.min_shutter;
			if (se < imgsensor_info.min_shutter) se = imgsensor_info.min_shutter;
			
			
		// Extend frame length first
		write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);

		write_cmos_sensor(0x3502, (le << 4) & 0xFF);
		write_cmos_sensor(0x3501, (le >> 4) & 0xFF);	 
		write_cmos_sensor(0x3500, (le >> 12) & 0x0F);
		
		write_cmos_sensor(0x3512, (se << 4) & 0xFF); 
		write_cmos_sensor(0x3511, (se >> 4) & 0xFF);
		write_cmos_sensor(0x3510, (se >> 12) & 0x0F); 

		set_gain(gain);
	}

}



static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

	/********************************************************
	   *
	   *   0x3820[2] ISP Vertical flip
	   *   0x3820[1] Sensor Vertical flip
	   *
	   *   0x3821[2] ISP Horizontal mirror
	   *   0x3821[1] Sensor Horizontal mirror
	   *
	   *   ISP and Sensor flip or mirror register bit should be the same!!
	   *
	   ********************************************************/
	spin_lock(&imgsensor_drv_lock);
    imgsensor.mirror= image_mirror; 
    spin_unlock(&imgsensor_drv_lock);
	switch (image_mirror) {
		case IMAGE_NORMAL:
			write_cmos_sensor(0x0101,0X00); //GR
			break;
		case IMAGE_H_MIRROR:
			write_cmos_sensor(0x0101,0X01); //R
			break;
		case IMAGE_V_MIRROR:
			write_cmos_sensor(0x0101,0X02); //B	
			break;
		case IMAGE_HV_MIRROR:
			write_cmos_sensor(0x0101,0X03); //GB
			break;
		default:
			LOG_INF("Error image_mirror setting\n");
	}

}

/*************************************************************************
* FUNCTION
*	night_mode
*
* DESCRIPTION
*	This function night mode of sensor.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/ 
}	/*	night_mode	*/
static void sensor_init(void)
{
	LOG_INF("%s.\n", __func__);
	/* External Clock Setting */
	write_cmos_sensor(0x0136, 0x18);
	write_cmos_sensor(0x0137, 0x00);
	/* Register version */
	write_cmos_sensor(0x3A7D, 0x00);
	write_cmos_sensor(0x3A7E, 0x01);
	write_cmos_sensor(0x3A7F, 0x05);
	/* Global Setting */
	write_cmos_sensor(0x0101, 0x00);
	write_cmos_sensor(0x3100, 0x00);
	write_cmos_sensor(0x3101, 0x40);
	write_cmos_sensor(0x3102, 0x00);
	write_cmos_sensor(0x3103, 0x10);
	write_cmos_sensor(0x3104, 0x01);
	write_cmos_sensor(0x3105, 0xE8);
	write_cmos_sensor(0x3106, 0x01);
	write_cmos_sensor(0x3107, 0xF0);
	write_cmos_sensor(0x3150, 0x04);
	write_cmos_sensor(0x3151, 0x03);
	write_cmos_sensor(0x3152, 0x02);
	write_cmos_sensor(0x3153, 0x01);
	write_cmos_sensor(0x5A86, 0x00);
	write_cmos_sensor(0x5A87, 0x82);
	write_cmos_sensor(0x5D1A, 0x00);
	write_cmos_sensor(0x5D95, 0x02);
	write_cmos_sensor(0x5E1B, 0x00);
	write_cmos_sensor(0x5F5A, 0x00);
	write_cmos_sensor(0x5F5B, 0x04);
	write_cmos_sensor(0x682C, 0x31);
	write_cmos_sensor(0x6831, 0x31);
	write_cmos_sensor(0x6835, 0x0E);
	write_cmos_sensor(0x6836, 0x31);
	write_cmos_sensor(0x6838, 0x30);
	write_cmos_sensor(0x683A, 0x06);
	write_cmos_sensor(0x683B, 0x33);
	write_cmos_sensor(0x683D, 0x30);
	write_cmos_sensor(0x6842, 0x31);
	write_cmos_sensor(0x6844, 0x31);
	write_cmos_sensor(0x6847, 0x31);
	write_cmos_sensor(0x6849, 0x31);
	write_cmos_sensor(0x684D, 0x0E);
	write_cmos_sensor(0x684E, 0x32);
	write_cmos_sensor(0x6850, 0x31);
	write_cmos_sensor(0x6852, 0x06);
	write_cmos_sensor(0x6853, 0x33);
	write_cmos_sensor(0x6855, 0x31);
	write_cmos_sensor(0x685A, 0x32);
	write_cmos_sensor(0x685C, 0x33);
	write_cmos_sensor(0x685F, 0x31);
	write_cmos_sensor(0x6861, 0x33);
	write_cmos_sensor(0x6865, 0x0D);
	write_cmos_sensor(0x6866, 0x33);
	write_cmos_sensor(0x6868, 0x31);
	write_cmos_sensor(0x686B, 0x34);
	write_cmos_sensor(0x686D, 0x31);
	write_cmos_sensor(0x6872, 0x32);
	write_cmos_sensor(0x6877, 0x33);
	write_cmos_sensor(0x7FF0, 0x01);
	write_cmos_sensor(0x7FF4, 0x08);
	write_cmos_sensor(0x7FF5, 0x3C);
	write_cmos_sensor(0x7FFA, 0x01);
	write_cmos_sensor(0x7FFD, 0x00);
	write_cmos_sensor(0x831E, 0x00);
	write_cmos_sensor(0x831F, 0x00);
	write_cmos_sensor(0x9301, 0xBD);
	write_cmos_sensor(0x9B94, 0x03);
	write_cmos_sensor(0x9B95, 0x00);
	write_cmos_sensor(0x9B96, 0x08);
	write_cmos_sensor(0x9B97, 0x00);
	write_cmos_sensor(0x9B98, 0x0A);
	write_cmos_sensor(0x9B99, 0x00);
	write_cmos_sensor(0x9BA7, 0x18);
	write_cmos_sensor(0x9BA8, 0x18);
	write_cmos_sensor(0x9D04, 0x08);
	write_cmos_sensor(0x9D50, 0x8C);
	write_cmos_sensor(0x9D51, 0x64);
	write_cmos_sensor(0x9D52, 0x50);
	write_cmos_sensor(0x9E31, 0x04);
	write_cmos_sensor(0x9E32, 0x04);
	write_cmos_sensor(0x9E33, 0x04);
	write_cmos_sensor(0x9E34, 0x04);
	write_cmos_sensor(0xA200, 0x00);
	write_cmos_sensor(0xA201, 0x0A);
	write_cmos_sensor(0xA202, 0x00);
	write_cmos_sensor(0xA203, 0x0A);
	write_cmos_sensor(0xA204, 0x00);
	write_cmos_sensor(0xA205, 0x0A);
	write_cmos_sensor(0xA206, 0x01);
	write_cmos_sensor(0xA207, 0xC0);
	write_cmos_sensor(0xA208, 0x00);
	write_cmos_sensor(0xA209, 0xC0);
	write_cmos_sensor(0xA20C, 0x00);
	write_cmos_sensor(0xA20D, 0x0A);
	write_cmos_sensor(0xA20E, 0x00);
	write_cmos_sensor(0xA20F, 0x0A);
	write_cmos_sensor(0xA210, 0x00);
	write_cmos_sensor(0xA211, 0x0A);
	write_cmos_sensor(0xA212, 0x01);
	write_cmos_sensor(0xA213, 0xC0);
	write_cmos_sensor(0xA214, 0x00);
	write_cmos_sensor(0xA215, 0xC0);
	write_cmos_sensor(0xA300, 0x00);
	write_cmos_sensor(0xA301, 0x0A);
	write_cmos_sensor(0xA302, 0x00);
	write_cmos_sensor(0xA303, 0x0A);
	write_cmos_sensor(0xA304, 0x00);
	write_cmos_sensor(0xA305, 0x0A);
	write_cmos_sensor(0xA306, 0x01);
	write_cmos_sensor(0xA307, 0xC0);
	write_cmos_sensor(0xA308, 0x00);
	write_cmos_sensor(0xA309, 0xC0);
	write_cmos_sensor(0xA30C, 0x00);
	write_cmos_sensor(0xA30D, 0x0A);
	write_cmos_sensor(0xA30E, 0x00);
	write_cmos_sensor(0xA30F, 0x0A);
	write_cmos_sensor(0xA310, 0x00);
	write_cmos_sensor(0xA311, 0x0A);
	write_cmos_sensor(0xA312, 0x01);
	write_cmos_sensor(0xA313, 0xC0);
	write_cmos_sensor(0xA314, 0x00);
	write_cmos_sensor(0xA315, 0xC0);
	write_cmos_sensor(0xBC19, 0x01);
	write_cmos_sensor(0xBC1C, 0x0A);
	/* Image Tuning Setting */
	write_cmos_sensor(0x3035, 0x01);
	write_cmos_sensor(0x3051, 0x00);
	write_cmos_sensor(0x7F47, 0x00);
	write_cmos_sensor(0x7F78, 0x00);
	write_cmos_sensor(0x7F89, 0x00);
	write_cmos_sensor(0x7F93, 0x00);
	write_cmos_sensor(0x7FB4, 0x00);
	write_cmos_sensor(0x7FCC, 0x01);
	write_cmos_sensor(0x9D02, 0x00);
	write_cmos_sensor(0x9D44, 0x8C);
	write_cmos_sensor(0x9D62, 0x8C);
	write_cmos_sensor(0x9D63, 0x50);
	write_cmos_sensor(0x9D64, 0x1B);
	write_cmos_sensor(0x9E0D, 0x00);
	write_cmos_sensor(0x9E0E, 0x00);
	write_cmos_sensor(0x9E15, 0x0A);
	write_cmos_sensor(0x9F02, 0x00);
	write_cmos_sensor(0x9F03, 0x23);
	write_cmos_sensor(0x9F4E, 0x00);
	write_cmos_sensor(0x9F4F, 0x42);
	write_cmos_sensor(0x9F54, 0x00);
	write_cmos_sensor(0x9F55, 0x5A);
	write_cmos_sensor(0x9F6E, 0x00);
	write_cmos_sensor(0x9F6F, 0x10);
	write_cmos_sensor(0x9F72, 0x00);
	write_cmos_sensor(0x9F73, 0xC8);
	write_cmos_sensor(0x9F74, 0x00);
	write_cmos_sensor(0x9F75, 0x32);
	write_cmos_sensor(0x9FD3, 0x00);
	write_cmos_sensor(0x9FD4, 0x00);
	write_cmos_sensor(0x9FD5, 0x00);
	write_cmos_sensor(0x9FD6, 0x3C);
	write_cmos_sensor(0x9FD7, 0x3C);
	write_cmos_sensor(0x9FD8, 0x3C);
	write_cmos_sensor(0x9FD9, 0x00);
	write_cmos_sensor(0x9FDA, 0x00);
	write_cmos_sensor(0x9FDB, 0x00);
	write_cmos_sensor(0x9FDC, 0xFF);
	write_cmos_sensor(0x9FDD, 0xFF);
	write_cmos_sensor(0x9FDE, 0xFF);
	write_cmos_sensor(0xA002, 0x00);
	write_cmos_sensor(0xA003, 0x14);
	write_cmos_sensor(0xA04E, 0x00);
	write_cmos_sensor(0xA04F, 0x2D);
	write_cmos_sensor(0xA054, 0x00);
	write_cmos_sensor(0xA055, 0x40);
	write_cmos_sensor(0xA06E, 0x00);
	write_cmos_sensor(0xA06F, 0x10);
	write_cmos_sensor(0xA072, 0x00);
	write_cmos_sensor(0xA073, 0xC8);
	write_cmos_sensor(0xA074, 0x00);
	write_cmos_sensor(0xA075, 0x32);
	write_cmos_sensor(0xA0CA, 0x04);
	write_cmos_sensor(0xA0CB, 0x04);
	write_cmos_sensor(0xA0CC, 0x04);
	write_cmos_sensor(0xA0D3, 0x0A);
	write_cmos_sensor(0xA0D4, 0x0A);
	write_cmos_sensor(0xA0D5, 0x0A);
	write_cmos_sensor(0xA0D6, 0x00);
	write_cmos_sensor(0xA0D7, 0x00);
	write_cmos_sensor(0xA0D8, 0x00);
	write_cmos_sensor(0xA0D9, 0x18);
	write_cmos_sensor(0xA0DA, 0x18);
	write_cmos_sensor(0xA0DB, 0x18);
	write_cmos_sensor(0xA0DC, 0x00);
	write_cmos_sensor(0xA0DD, 0x00);
	write_cmos_sensor(0xA0DE, 0x00);
	write_cmos_sensor(0xBCB2, 0x01);

	load_imx386_spc_data();
	write_cmos_sensor(0x0100, 0x00);
}


static void preview_setting(void)
{
	LOG_INF("%s.\n", __func__);
	write_cmos_sensor(0x0100, 0x00);
	/*
	 * 1/2Binning@30fps
	 * H: 2016
	 * V: 1508
	 */
	/* Mode Setting */
	write_cmos_sensor(0x0112, 0x0A);
	write_cmos_sensor(0x0113, 0x0A);
	/* Clock Setting */
	write_cmos_sensor(0x0301, 0x06);
	write_cmos_sensor(0x0303, 0x02);
	write_cmos_sensor(0x0305, 0x0C);
	write_cmos_sensor(0x0306, 0x01);
	write_cmos_sensor(0x0307, 0x5E);
	write_cmos_sensor(0x0309, 0x0A);
	write_cmos_sensor(0x030B, 0x01);
	write_cmos_sensor(0x030D, 0x0C);
	write_cmos_sensor(0x030E, 0x01);
	write_cmos_sensor(0x030F, 0x5E);
	write_cmos_sensor(0x0310, 0x00);
	/* Output Size Setting */
	write_cmos_sensor(0x0342, 0x10);
	write_cmos_sensor(0x0343, 0xC8);
	/* Output Size Setting */
	write_cmos_sensor(0x0340, 0x06);
	write_cmos_sensor(0x0341, 0xF4);
	/* Output Size Setting */
	write_cmos_sensor(0x0344, 0x00);
	write_cmos_sensor(0x0345, 0x00);
	write_cmos_sensor(0x0346, 0x00);
	write_cmos_sensor(0x0347, 0x00);
	write_cmos_sensor(0x0348, 0x0F);
	write_cmos_sensor(0x0349, 0xBF);
	write_cmos_sensor(0x034A, 0x0B);
	write_cmos_sensor(0x034B, 0xC7);
	/* Output Size Setting */
	write_cmos_sensor(0x0385, 0x01);
	write_cmos_sensor(0x0387, 0x01);
	write_cmos_sensor(0x0900, 0x01);
	write_cmos_sensor(0x0901, 0x12);
	write_cmos_sensor(0x300D, 0x00);
	write_cmos_sensor(0x302E, 0x00);
	write_cmos_sensor(0x0401, 0x01);
	write_cmos_sensor(0x0404, 0x00);
	write_cmos_sensor(0x0405, 0x20);
	write_cmos_sensor(0x040C, 0x0F);
	write_cmos_sensor(0x040D, 0xC0);
	write_cmos_sensor(0x040E, 0x05);
	write_cmos_sensor(0x040F, 0xE4);
	write_cmos_sensor(0x034C, 0x07);
	write_cmos_sensor(0x034D, 0xE0);
	write_cmos_sensor(0x034E, 0x05);
	write_cmos_sensor(0x034F, 0xE4);
	/* Other Setting */
	write_cmos_sensor(0x0114, 0x03);
	write_cmos_sensor(0x0408, 0x00);
	write_cmos_sensor(0x0409, 0x00);
	write_cmos_sensor(0x040A, 0x00);
	write_cmos_sensor(0x040B, 0x00);
	write_cmos_sensor(0x0902, 0x00);
	write_cmos_sensor(0x3030, 0x00);
	write_cmos_sensor(0x3031, 0x01);
	write_cmos_sensor(0x3032, 0x00);
	write_cmos_sensor(0x30E6, 0x00);
	write_cmos_sensor(0x30E7, 0x00);
	write_cmos_sensor(0x4E25, 0x80);
	write_cmos_sensor(0x663A, 0x01);
	write_cmos_sensor(0x9311, 0x40);
	write_cmos_sensor(0xA0CD, 0x19);
	write_cmos_sensor(0xA0CE, 0x19);
	write_cmos_sensor(0xA0CF, 0x19);
	/* Integration Time Setting */
	write_cmos_sensor(0x0202, 0x06);
	write_cmos_sensor(0x0203, 0xEA);
	/* Gain Setting */
	write_cmos_sensor(0x0204, 0x00);
	write_cmos_sensor(0x0205, 0x00);
	write_cmos_sensor(0x020E, 0x01);
	write_cmos_sensor(0x020F, 0x00);
	write_cmos_sensor(0x0210, 0x01);
	write_cmos_sensor(0x0211, 0x00);
	write_cmos_sensor(0x0212, 0x01);
	write_cmos_sensor(0x0213, 0x00);
	write_cmos_sensor(0x0214, 0x01);
	write_cmos_sensor(0x0215, 0x00);
	/*pdaf setting*/
	write_cmos_sensor(0x3047, 0x01);/*PD_CAL_ENALBE*/
	write_cmos_sensor(0x3049, 0x01);/*PD_OUT_EN=1*/
#ifdef PDAF_FIX_WINDOW //fix window
	write_cmos_sensor(0x315D, 0x01);/*Area mode*/

	write_cmos_sensor(0x3100, 0x00);
	write_cmos_sensor(0x3101, 0x00);
	write_cmos_sensor(0x3102, 0x00);
	write_cmos_sensor(0x3103, 0x00);
	write_cmos_sensor(0x3104, 0x00);
	write_cmos_sensor(0x3105, 0xFB);//0x64);
	write_cmos_sensor(0x3106, 0x00);
	write_cmos_sensor(0x3107, 0xFC);//0x64);
#else//flexible window
	write_cmos_sensor(0x315D, 0x02);/*Area mode*/
	write_cmos_sensor(0x315E, 0x01);/*Area0*/
#endif

	write_cmos_sensor(0x0100, 0x01);
	cur_startpos[1] = cur_startpos[0] = 0;
	cur_size[1] = cur_size[0] = 0;

}	/* preview_setting */

static void capture_setting(kal_uint16 currefps)
{
	LOG_INF("%s.\n", __func__);
	write_cmos_sensor(0x0100, 0x00);
	/* Mode Setting */
	write_cmos_sensor(0x0112, 0x0A);
	write_cmos_sensor(0x0113, 0x0A);
	/* Clock Setting */
	write_cmos_sensor(0x0301, 0x06);
	write_cmos_sensor(0x0303, 0x02);
	write_cmos_sensor(0x0305, 0x0C);
	write_cmos_sensor(0x0306, 0x02);
	write_cmos_sensor(0x0307, 0x8A);
	write_cmos_sensor(0x0309, 0x0A);
	write_cmos_sensor(0x030B, 0x01);
	write_cmos_sensor(0x030D, 0x0C);
	write_cmos_sensor(0x030E, 0x02);
	write_cmos_sensor(0x030F, 0x8A);
	write_cmos_sensor(0x0310, 0x00);
	/* Output Size Setting */
	write_cmos_sensor(0x0342, 0x10);
	write_cmos_sensor(0x0343, 0xC8);
	write_cmos_sensor(0x0340, 0x0C);
	write_cmos_sensor(0x0341, 0xE4);
	write_cmos_sensor(0x0344, 0x00);
	write_cmos_sensor(0x0345, 0x00);
	write_cmos_sensor(0x0346, 0x00);
	write_cmos_sensor(0x0347, 0x00);
	write_cmos_sensor(0x0348, 0x0F);
	write_cmos_sensor(0x0349, 0xBF);
	write_cmos_sensor(0x034A, 0x0B);
	write_cmos_sensor(0x034B, 0xC7);
	write_cmos_sensor(0x0385, 0x01);
	write_cmos_sensor(0x0387, 0x01);
	write_cmos_sensor(0x0900, 0x00);
	write_cmos_sensor(0x0901, 0x11);
	write_cmos_sensor(0x300D, 0x00);
	write_cmos_sensor(0x302E, 0x00);
	write_cmos_sensor(0x0401, 0x00);
	write_cmos_sensor(0x0404, 0x00);
	write_cmos_sensor(0x0405, 0x10);
	write_cmos_sensor(0x040C, 0x0F);
	write_cmos_sensor(0x040D, 0xC0);
	write_cmos_sensor(0x040E, 0x0B);
	write_cmos_sensor(0x040F, 0xC8);
	write_cmos_sensor(0x034C, 0x0F);
	write_cmos_sensor(0x034D, 0xC0);
	write_cmos_sensor(0x034E, 0x0B);
	write_cmos_sensor(0x034F, 0xC8);
	/* Other Setting */
	write_cmos_sensor(0x0114, 0x03);
	write_cmos_sensor(0x0408, 0x00);
	write_cmos_sensor(0x0409, 0x00);
	write_cmos_sensor(0x040A, 0x00);
	write_cmos_sensor(0x040B, 0x00);
	write_cmos_sensor(0x0902, 0x00);
	write_cmos_sensor(0x3030, 0x00);
	write_cmos_sensor(0x3031, 0x01);
	write_cmos_sensor(0x3032, 0x00);
	write_cmos_sensor(0x30E6, 0x02);
	write_cmos_sensor(0x30E7, 0x59);
	write_cmos_sensor(0x4E25, 0x80);
	write_cmos_sensor(0x663A, 0x02);
	write_cmos_sensor(0x9311, 0x00);
	write_cmos_sensor(0xA0CD, 0x19);
	write_cmos_sensor(0xA0CE, 0x19);
	write_cmos_sensor(0xA0CF, 0x19);
	/* Integration Time Setting */
	write_cmos_sensor(0x0202, 0x0C);
	write_cmos_sensor(0x0203, 0xDA);
	/* Gain Setting */
	write_cmos_sensor(0x0204, 0x00);
	write_cmos_sensor(0x0205, 0x00);
	write_cmos_sensor(0x020E, 0x01);
	write_cmos_sensor(0x020F, 0x00);
	write_cmos_sensor(0x0210, 0x01);
	write_cmos_sensor(0x0211, 0x00);
	write_cmos_sensor(0x0212, 0x01);
	write_cmos_sensor(0x0213, 0x00);
	write_cmos_sensor(0x0214, 0x01);
	write_cmos_sensor(0x0215, 0x00);
	/*pdaf setting*/
	write_cmos_sensor(0x3047, 0x01);/*PD_CAL_ENALBE*/
	write_cmos_sensor(0x3049, 0x01);/*PD_OUT_EN=1*/
#ifdef PDAF_FIX_WINDOW //fix window
	write_cmos_sensor(0x315D, 0x01);/*Area mode*/

	write_cmos_sensor(0x3100, 0x00);
	write_cmos_sensor(0x3101, 0x40);
	write_cmos_sensor(0x3102, 0x00);
	write_cmos_sensor(0x3103, 0x10);
	write_cmos_sensor(0x3104, 0x01);
	write_cmos_sensor(0x3105, 0xE8);
	write_cmos_sensor(0x3106, 0x01);
	write_cmos_sensor(0x3107, 0xF0);
#else//flexible window
	write_cmos_sensor(0x315D, 0x02);/*Area mode*/
	write_cmos_sensor(0x315E, 0x01);/*Area0*/
#endif
	cur_startpos[1] = cur_startpos[0] = 0;
	cur_size[1] = cur_size[0] = 0;

}	/* capture setting */

static void hd_4k_setting(void)
{
	LOG_INF("%s.\n", __func__);
	write_cmos_sensor(0x0100, 0x00);
	/*
	 * Full-reso (16:9)@30fps
	 * H: 4032
	 * V: 2256
	 */
	/* Mode Setting */
	write_cmos_sensor(0x0112, 0x0A);
	write_cmos_sensor(0x0113, 0x0A);
	/* Clock Setting */
	write_cmos_sensor(0x0301, 0x06);
	write_cmos_sensor(0x0303, 0x02);
	write_cmos_sensor(0x0305, 0x0C);
	write_cmos_sensor(0x0306, 0x01);
	write_cmos_sensor(0x0307, 0xC2);
	write_cmos_sensor(0x0309, 0x0A);
	write_cmos_sensor(0x030B, 0x01);
	write_cmos_sensor(0x030D, 0x0C);
	write_cmos_sensor(0x030E, 0x01);
	write_cmos_sensor(0x030F, 0xC2);
	write_cmos_sensor(0x0310, 0x00);
	/* Output Size Setting */
	write_cmos_sensor(0x0342, 0x10);
	write_cmos_sensor(0x0343, 0xC8);
	write_cmos_sensor(0x0340, 0x08);
	write_cmos_sensor(0x0341, 0xFE);
	write_cmos_sensor(0x0344, 0x00);
	write_cmos_sensor(0x0345, 0x00);
	write_cmos_sensor(0x0346, 0x01);
	write_cmos_sensor(0x0347, 0x7C);
	write_cmos_sensor(0x0348, 0x0F);
	write_cmos_sensor(0x0349, 0xBF);
	write_cmos_sensor(0x034A, 0x0A);
	write_cmos_sensor(0x034B, 0x4B);
	write_cmos_sensor(0x0385, 0x01);
	write_cmos_sensor(0x0387, 0x01);
	write_cmos_sensor(0x0900, 0x00);
	write_cmos_sensor(0x0901, 0x11);
	write_cmos_sensor(0x300D, 0x00);
	write_cmos_sensor(0x302E, 0x00);
	write_cmos_sensor(0x0401, 0x00);
	write_cmos_sensor(0x0404, 0x00);
	write_cmos_sensor(0x0405, 0x10);
	write_cmos_sensor(0x040C, 0x0F);
	write_cmos_sensor(0x040D, 0xC0);
	write_cmos_sensor(0x040E, 0x08);
	write_cmos_sensor(0x040F, 0xD0);
	write_cmos_sensor(0x034C, 0x0F);
	write_cmos_sensor(0x034D, 0xC0);
	write_cmos_sensor(0x034E, 0x08);
	write_cmos_sensor(0x034F, 0xD0);
	/* Other Setting */
	write_cmos_sensor(0x0114, 0x03);
	write_cmos_sensor(0x0408, 0x00);
	write_cmos_sensor(0x0409, 0x00);
	write_cmos_sensor(0x040A, 0x00);
	write_cmos_sensor(0x040B, 0x00);
	write_cmos_sensor(0x0902, 0x00);
	write_cmos_sensor(0x3030, 0x00);
	write_cmos_sensor(0x3031, 0x01);
	write_cmos_sensor(0x3032, 0x00);
	write_cmos_sensor(0x30E6, 0x02);
	write_cmos_sensor(0x30E7, 0x59);
	write_cmos_sensor(0x4E25, 0x80);
	write_cmos_sensor(0x663A, 0x02);
	write_cmos_sensor(0x9311, 0x00);
	write_cmos_sensor(0xA0CD, 0x19);
	write_cmos_sensor(0xA0CE, 0x19);
	write_cmos_sensor(0xA0CF, 0x19);
	/* Integration Time Setting */
	write_cmos_sensor(0x0202, 0x08);
	write_cmos_sensor(0x0203, 0xF4);
	/* Gain Setting */
	write_cmos_sensor(0x0204, 0x00);
	write_cmos_sensor(0x0205, 0x00);
	write_cmos_sensor(0x020E, 0x01);
	write_cmos_sensor(0x020F, 0x00);
	write_cmos_sensor(0x0210, 0x01);
	write_cmos_sensor(0x0211, 0x00);
	write_cmos_sensor(0x0212, 0x01);
	write_cmos_sensor(0x0213, 0x00);
	write_cmos_sensor(0x0214, 0x01);
	write_cmos_sensor(0x0215, 0x00);
	/*pdaf setting*/
	write_cmos_sensor(0x3047, 0x01);/*PD_CAL_ENALBE*/
	write_cmos_sensor(0x3049, 0x01);/*PD_OUT_EN=1*/
#ifdef PDAF_FIX_WINDOW //fix window
	write_cmos_sensor(0x315D, 0x01);/*Area mode*/

	write_cmos_sensor(0x3100, 0x00);
	write_cmos_sensor(0x3101, 0x00);
	write_cmos_sensor(0x3102, 0x00);
	write_cmos_sensor(0x3103, 0x00);
	write_cmos_sensor(0x3104, 0x01);
	write_cmos_sensor(0x3105, 0x78);//0x64);
	write_cmos_sensor(0x3106, 0x01);
	write_cmos_sensor(0x3107, 0xF0);//0x64);
#else //flexible window
	write_cmos_sensor(0x315D, 0x02);/*Area mode*/
	write_cmos_sensor(0x315E, 0x01);/*Area0*/
#endif
	write_cmos_sensor(0x0100, 0x01);

	cur_startpos[1] = cur_startpos[0] = 0;
	cur_size[1] = cur_size[0] = 0;

}

static void normal_video_setting(kal_uint16 currefps)
{
	LOG_INF("%s.\n", __func__);
	write_cmos_sensor(0x0100, 0x00);
	/*
	 * 1/2Binning@30fps
	 * H: 2016
	 * V: 1508
	 */
	/* Mode Setting */
	write_cmos_sensor(0x0112, 0x0A);
	write_cmos_sensor(0x0113, 0x0A);
	/* Clock Setting */
	write_cmos_sensor(0x0301, 0x06);
	write_cmos_sensor(0x0303, 0x02);
	write_cmos_sensor(0x0305, 0x0C);
	write_cmos_sensor(0x0306, 0x01);
	write_cmos_sensor(0x0307, 0x5E);
	write_cmos_sensor(0x0309, 0x0A);
	write_cmos_sensor(0x030B, 0x01);
	write_cmos_sensor(0x030D, 0x0C);
	write_cmos_sensor(0x030E, 0x01);
	write_cmos_sensor(0x030F, 0x5E);
	write_cmos_sensor(0x0310, 0x00);
	/* Output Size Setting */
	write_cmos_sensor(0x0342, 0x10);
	write_cmos_sensor(0x0343, 0xC8);
	/* Output Size Setting */
	write_cmos_sensor(0x0340, 0x06);
	write_cmos_sensor(0x0341, 0xF4);
	/* Output Size Setting */
	write_cmos_sensor(0x0344, 0x00);
	write_cmos_sensor(0x0345, 0x00);
	write_cmos_sensor(0x0346, 0x00);
	write_cmos_sensor(0x0347, 0x00);
	write_cmos_sensor(0x0348, 0x0F);
	write_cmos_sensor(0x0349, 0xBF);
	write_cmos_sensor(0x034A, 0x0B);
	write_cmos_sensor(0x034B, 0xC7);
	/* Output Size Setting */
	write_cmos_sensor(0x0385, 0x01);
	write_cmos_sensor(0x0387, 0x01);
	write_cmos_sensor(0x0900, 0x01);
	write_cmos_sensor(0x0901, 0x12);
	write_cmos_sensor(0x300D, 0x00);
	write_cmos_sensor(0x302E, 0x00);
	write_cmos_sensor(0x0401, 0x01);
	write_cmos_sensor(0x0404, 0x00);
	write_cmos_sensor(0x0405, 0x20);
	write_cmos_sensor(0x040C, 0x0F);
	write_cmos_sensor(0x040D, 0xC0);
	write_cmos_sensor(0x040E, 0x05);
	write_cmos_sensor(0x040F, 0xE4);
	write_cmos_sensor(0x034C, 0x07);
	write_cmos_sensor(0x034D, 0xE0);
	write_cmos_sensor(0x034E, 0x05);
	write_cmos_sensor(0x034F, 0xE4);
	/* Other Setting */
	write_cmos_sensor(0x0114, 0x03);
	write_cmos_sensor(0x0408, 0x00);
	write_cmos_sensor(0x0409, 0x00);
	write_cmos_sensor(0x040A, 0x00);
	write_cmos_sensor(0x040B, 0x00);
	write_cmos_sensor(0x0902, 0x00);
	write_cmos_sensor(0x3030, 0x00);
	write_cmos_sensor(0x3031, 0x01);
	write_cmos_sensor(0x3032, 0x00);
	write_cmos_sensor(0x30E6, 0x00);
	write_cmos_sensor(0x30E7, 0x00);
	write_cmos_sensor(0x4E25, 0x80);
	write_cmos_sensor(0x663A, 0x01);
	write_cmos_sensor(0x9311, 0x40);
	write_cmos_sensor(0xA0CD, 0x19);
	write_cmos_sensor(0xA0CE, 0x19);
	write_cmos_sensor(0xA0CF, 0x19);
	/* Integration Time Setting */
	write_cmos_sensor(0x0202, 0x06);
	write_cmos_sensor(0x0203, 0xEA);
	/* Gain Setting */
	write_cmos_sensor(0x0204, 0x00);
	write_cmos_sensor(0x0205, 0x00);
	write_cmos_sensor(0x020E, 0x01);
	write_cmos_sensor(0x020F, 0x00);
	write_cmos_sensor(0x0210, 0x01);
	write_cmos_sensor(0x0211, 0x00);
	write_cmos_sensor(0x0212, 0x01);
	write_cmos_sensor(0x0213, 0x00);
	write_cmos_sensor(0x0214, 0x01);
	write_cmos_sensor(0x0215, 0x00);
	/*pdaf setting*/
	write_cmos_sensor(0x3047, 0x01);/*PD_CAL_ENALBE*/
	write_cmos_sensor(0x3049, 0x01);/*PD_OUT_EN=1*/
#ifdef PDAF_FIX_WINDOW //fix window
	write_cmos_sensor(0x315D, 0x01);/*Area mode*/

	write_cmos_sensor(0x3100, 0x00);
	write_cmos_sensor(0x3101, 0x00);
	write_cmos_sensor(0x3102, 0x00);
	write_cmos_sensor(0x3103, 0x00);
	write_cmos_sensor(0x3104, 0x00);
	write_cmos_sensor(0x3105, 0xFB);//0x64);
	write_cmos_sensor(0x3106, 0x00);
	write_cmos_sensor(0x3107, 0xFC);//0x64);
#else //flexible window
	write_cmos_sensor(0x315D, 0x02);/*Area mode*/
	write_cmos_sensor(0x315E, 0x01);/*Area0*/
#endif

	write_cmos_sensor(0x0100, 0x01);
	cur_startpos[1] = cur_startpos[0] = 0;
	cur_size[1] = cur_size[0] = 0;

}

static void hs_video_setting(void)
{
	LOG_INF("%s.\n", __func__);
	write_cmos_sensor(0x0100, 0x00);
	/*
	 * 1296X736@120fps
	 * H: 1296
	 * V: 736
	 */
	/* Mode Setting */
	write_cmos_sensor(0x0112, 0x0A);
	write_cmos_sensor(0x0113, 0x0A);
	/* Clock Setting */
	write_cmos_sensor(0x0301, 0x04);
	write_cmos_sensor(0x0303, 0x02);
	write_cmos_sensor(0x0305, 0x0C);
	write_cmos_sensor(0x0306, 0x02);
	write_cmos_sensor(0x0307, 0x58);
	write_cmos_sensor(0x0309, 0x0A);
	write_cmos_sensor(0x030B, 0x01);
	write_cmos_sensor(0x030D, 0x0C);
	write_cmos_sensor(0x030E, 0x02);
	write_cmos_sensor(0x030F, 0x58);
	write_cmos_sensor(0x0310, 0x00);
	/* Output Size Setting */
	write_cmos_sensor(0x0342, 0x10);
	write_cmos_sensor(0x0343, 0xC8);
	/* Output Size Setting */
	write_cmos_sensor(0x0340, 0x04);
	write_cmos_sensor(0x0341, 0x88);
	/* Output Size Setting */
	write_cmos_sensor(0x0344, 0x02);
	write_cmos_sensor(0x0345, 0xD0);
	write_cmos_sensor(0x0346, 0x03);
	write_cmos_sensor(0x0347, 0x04);
	write_cmos_sensor(0x0348, 0x0C);
	write_cmos_sensor(0x0349, 0xEF);
	write_cmos_sensor(0x034A, 0x08);
	write_cmos_sensor(0x034B, 0xC3);
	write_cmos_sensor(0x0385, 0x01);
	write_cmos_sensor(0x0387, 0x01);
	write_cmos_sensor(0x0900, 0x01);
	write_cmos_sensor(0x0901, 0x12);
	write_cmos_sensor(0x300D, 0x00);
	write_cmos_sensor(0x302E, 0x01);
	write_cmos_sensor(0x0401, 0x01);
	write_cmos_sensor(0x0404, 0x00);
	write_cmos_sensor(0x0405, 0x20);
	write_cmos_sensor(0x040C, 0x0A);
	write_cmos_sensor(0x040D, 0x20);
	write_cmos_sensor(0x040E, 0x02);
	write_cmos_sensor(0x040F, 0xE0);
	write_cmos_sensor(0x034C, 0x05);
	write_cmos_sensor(0x034D, 0x10);
	write_cmos_sensor(0x034E, 0x02);
	write_cmos_sensor(0x034F, 0xE0);
	/* Other Setting */
	write_cmos_sensor(0x0114, 0x03);
	write_cmos_sensor(0x0408, 0x00);
	write_cmos_sensor(0x0409, 0x00);
	write_cmos_sensor(0x040A, 0x00);
	write_cmos_sensor(0x040B, 0x00);
	write_cmos_sensor(0x0902, 0x00);
	write_cmos_sensor(0x3030, 0x00);
	write_cmos_sensor(0x3031, 0x01);
	write_cmos_sensor(0x3032, 0x00);
	write_cmos_sensor(0x30E6, 0x00);
	write_cmos_sensor(0x30E7, 0x00);
	write_cmos_sensor(0x4E25, 0x80);
	write_cmos_sensor(0x663A, 0x01);
	write_cmos_sensor(0x9311, 0x64);
	write_cmos_sensor(0xA0CD, 0x19);
	write_cmos_sensor(0xA0CE, 0x19);
	write_cmos_sensor(0xA0CF, 0x19);
	/* Integration Time Setting */
	write_cmos_sensor(0x0202, 0x04);
	write_cmos_sensor(0x0203, 0x7E);
	/* Gain Setting */
	write_cmos_sensor(0x0204, 0x00);
	write_cmos_sensor(0x0205, 0x00);
	write_cmos_sensor(0x020E, 0x01);
	write_cmos_sensor(0x020F, 0x00);
	write_cmos_sensor(0x0210, 0x01);
	write_cmos_sensor(0x0211, 0x00);
	write_cmos_sensor(0x0212, 0x01);
	write_cmos_sensor(0x0213, 0x00);
	write_cmos_sensor(0x0214, 0x01);
	write_cmos_sensor(0x0215, 0x00);
	/*pdaf setting*/
	write_cmos_sensor(0x3047, 0x01);/*PD_CAL_ENALBE*/
	write_cmos_sensor(0x3049, 0x01);/*PD_OUT_EN=1*/
#ifdef PDAF_FIX_WINDOW //fix window
	write_cmos_sensor(0x315D, 0x01);/*Area mode*/

	write_cmos_sensor(0x3100, 0x00);
	write_cmos_sensor(0x3101, 0x00);
	write_cmos_sensor(0x3102, 0x00);
	write_cmos_sensor(0x3103, 0x00);
	write_cmos_sensor(0x3104, 0x00);
	write_cmos_sensor(0x3105, 0x64);
	write_cmos_sensor(0x3106, 0x00);
	write_cmos_sensor(0x3107, 0x64);
#else //flexible window
	write_cmos_sensor(0x315D, 0x02);/*Area mode*/
	write_cmos_sensor(0x315E, 0x01);/*Area0*/
#endif
	write_cmos_sensor(0x0100, 0x01);
	cur_startpos[1] = cur_startpos[0] = 0;
	cur_size[1] = cur_size[0] = 0;

}

static void slim_video_setting(void)
{
	LOG_INF("%s.\n", __func__);
	write_cmos_sensor(0x0100, 0x00);
	/*
	 * 1296X736@30fps
	 * H: 1296
	 * V: 736
	 */
	/* Mode Setting */
	write_cmos_sensor(0x0112, 0x0A);
	write_cmos_sensor(0x0113, 0x0A);
	/* Clock Setting */
	write_cmos_sensor(0x0301, 0x04);
	write_cmos_sensor(0x0303, 0x02);
	write_cmos_sensor(0x0305, 0x0C);
	write_cmos_sensor(0x0306, 0x02);
	write_cmos_sensor(0x0307, 0x58);
	write_cmos_sensor(0x0309, 0x0A);
	write_cmos_sensor(0x030B, 0x01);
	write_cmos_sensor(0x030D, 0x0C);
	write_cmos_sensor(0x030E, 0x02);
	write_cmos_sensor(0x030F, 0x58);
	write_cmos_sensor(0x0310, 0x00);
	/* Output Size Setting */
	write_cmos_sensor(0x0342, 0x10);
	write_cmos_sensor(0x0343, 0xC8);
	/* Output Size Setting */
	write_cmos_sensor(0x0340, 0x12);
	write_cmos_sensor(0x0341, 0x28);
	/* Output Size Setting */
	write_cmos_sensor(0x0344, 0x02);
	write_cmos_sensor(0x0345, 0xD0);
	write_cmos_sensor(0x0346, 0x03);
	write_cmos_sensor(0x0347, 0x04);
	write_cmos_sensor(0x0348, 0x0C);
	write_cmos_sensor(0x0349, 0xEF);
	write_cmos_sensor(0x034A, 0x08);
	write_cmos_sensor(0x034B, 0xC3);
	write_cmos_sensor(0x0385, 0x01);
	write_cmos_sensor(0x0387, 0x01);
	write_cmos_sensor(0x0900, 0x01);
	write_cmos_sensor(0x0901, 0x12);
	write_cmos_sensor(0x300D, 0x00);
	write_cmos_sensor(0x302E, 0x01);
	write_cmos_sensor(0x0401, 0x01);
	write_cmos_sensor(0x0404, 0x00);
	write_cmos_sensor(0x0405, 0x20);
	write_cmos_sensor(0x040C, 0x0A);
	write_cmos_sensor(0x040D, 0x20);
	write_cmos_sensor(0x040E, 0x02);
	write_cmos_sensor(0x040F, 0xE0);
	write_cmos_sensor(0x034C, 0x05);
	write_cmos_sensor(0x034D, 0x10);
	write_cmos_sensor(0x034E, 0x02);
	write_cmos_sensor(0x034F, 0xE0);
	/* Other Setting */
	write_cmos_sensor(0x0114, 0x03);
	write_cmos_sensor(0x0408, 0x00);
	write_cmos_sensor(0x0409, 0x00);
	write_cmos_sensor(0x040A, 0x00);
	write_cmos_sensor(0x040B, 0x00);
	write_cmos_sensor(0x0902, 0x00);
	write_cmos_sensor(0x3030, 0x00);
	write_cmos_sensor(0x3031, 0x01);
	write_cmos_sensor(0x3032, 0x00);
	write_cmos_sensor(0x3047, 0x01);
	write_cmos_sensor(0x3049, 0x01);
	write_cmos_sensor(0x30E6, 0x00);
	write_cmos_sensor(0x30E7, 0x00);
	write_cmos_sensor(0x4E25, 0x80);
	write_cmos_sensor(0x663A, 0x01);
	write_cmos_sensor(0x9311, 0x64);
	write_cmos_sensor(0xA0CD, 0x19);
	write_cmos_sensor(0xA0CE, 0x19);
	write_cmos_sensor(0xA0CF, 0x19);
	/* Integration Time Setting */
	write_cmos_sensor(0x0202, 0x12);
	write_cmos_sensor(0x0203, 0x1E);
	/* Gain Setting */
	write_cmos_sensor(0x0204, 0x00);
	write_cmos_sensor(0x0205, 0x00);
	write_cmos_sensor(0x020E, 0x01);
	write_cmos_sensor(0x020F, 0x00);
	write_cmos_sensor(0x0210, 0x01);
	write_cmos_sensor(0x0211, 0x00);
	write_cmos_sensor(0x0212, 0x01);
	write_cmos_sensor(0x0213, 0x00);
	write_cmos_sensor(0x0214, 0x01);
	write_cmos_sensor(0x0215, 0x00);

	write_cmos_sensor(0x0100, 0x01);
	/*pdaf setting*/
	cur_startpos[1] = cur_startpos[0] = 0;
	cur_size[1] = cur_size[0] = 0;
}


static kal_uint32 return_sensor_id(void)
{
    return ((read_cmos_sensor(0x0016) << 8) | read_cmos_sensor(0x0017));
}

/*************************************************************************
* FUNCTION
*	get_imgsensor_id
*
* DESCRIPTION
*	This function get the sensor ID 
*
* PARAMETERS
*	*sensorID : return the sensor ID 
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
    kal_uint8 i = 0;
    kal_uint8 retry = 2;
    int ret = 0;
    u8 module_id = 0;
    int j;
    //sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
    while (imgsensor_info.i2c_addr_table[i] != 0xff) {
        spin_lock(&imgsensor_drv_lock);
        imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
        spin_unlock(&imgsensor_drv_lock);
        do {
            *sensor_id = return_sensor_id();
	    ret = imx386_read_otp(0x0001, &module_id);
	    if ((ret >= 0) && (*sensor_id == imgsensor_info.sensor_id) && (module_id == 0x07)) {
		LOG_INF("i2c write id: 0x%x, sensor id: 0x%x, module_id:0x%x\n", imgsensor.i2c_write_id,*sensor_id, module_id);
		*sensor_id = IMX386SUNNY_SENSOR_ID;
		main_module_id = IMX386SUNNY_SENSOR_ID;
		goto otp_read;
            }
	    LOG_INF("Read sensor id fail, addr: 0x%x, sensor_id:0x%x, module_id:0x%x\n", imgsensor.i2c_write_id,*sensor_id, module_id);
            retry--;
        } while(retry > 0);
        i++;
        retry = 1;
    }
    if ((ret < 0) || (*sensor_id != imgsensor_info.sensor_id)
	    || (module_id != 0x07)) {
        // if Sensor ID is not correct, Must set *sensor_id to 0xFFFFFFFF
        *sensor_id = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }

otp_read:
	/*
	 * read lsc calibration from OTP E2PROM.
	 */
	sunny_otp_buf = (u8 *)kzalloc(OTP_DATA_SIZE, GFP_KERNEL);

	/* read lsc calibration from E2PROM */
	imx386_read_otp_burst(OTP_START_ADDR, sunny_otp_buf);
	for (j = 0; j < OTP_DATA_SIZE; j++) {
		LOG_INF("========imx386_sunny_otp RegIndex-%d=====val:0x%x======\n", j, *(sunny_otp_buf + j));
	}

	return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*	open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
    //const kal_uint8 i2c_addr[] = {IMGSENSOR_WRITE_ID_1, IMGSENSOR_WRITE_ID_2};
    kal_uint8 i = 0;
    kal_uint8 retry = 2;
    kal_uint32 sensor_id = 0;
	LOG_1;
    //sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
    while (imgsensor_info.i2c_addr_table[i] != 0xff) {
        spin_lock(&imgsensor_drv_lock);
        imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
        spin_unlock(&imgsensor_drv_lock);
        do {
            sensor_id = return_sensor_id();
            if (sensor_id == imgsensor_info.sensor_id) {
                LOG_INF("2015_12_24 i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
                break;
            }
            LOG_INF("Read sensor id fail, id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
            retry--;
        } while(retry > 0);
        i++;
        if (sensor_id == imgsensor_info.sensor_id)
            break;
        retry = 1;
    }
    if (imgsensor_info.sensor_id != sensor_id)
        return ERROR_SENSOR_CONNECT_FAIL;

    /* initail sequence write in  */
    sensor_init();

    spin_lock(&imgsensor_drv_lock);

    imgsensor.autoflicker_en= KAL_FALSE;
    imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
    imgsensor.pclk = imgsensor_info.pre.pclk;
    imgsensor.frame_length = imgsensor_info.pre.framelength;
    imgsensor.line_length = imgsensor_info.pre.linelength;
    imgsensor.min_frame_length = imgsensor_info.pre.framelength;
    imgsensor.dummy_pixel = 0;
    imgsensor.dummy_line = 0;
    imgsensor.ihdr_en = KAL_FALSE;
    imgsensor.test_pattern = KAL_FALSE;
    imgsensor.current_fps = imgsensor_info.pre.max_framerate;
    spin_unlock(&imgsensor_drv_lock);

    return ERROR_NONE;
}   /*  open  */



/*************************************************************************
* FUNCTION
*	close
*
* DESCRIPTION
*	
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("%s.\n", __func__);

	/*No Need to implement this function*/ 
	
	return ERROR_NONE;
}	/*	close  */


/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	//imgsensor.video_mode = KAL_FALSE;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength; 
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();
	
	return ERROR_NONE;
}	/*	preview   */

/*************************************************************************
* FUNCTION
*	capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	mode_change = 1;
	if (imgsensor.current_fps == imgsensor_info.cap.max_framerate)
		LOG_INF("capture30fps: use cap30FPS's setting: %d fps!\n",imgsensor.current_fps/10);
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;  
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	capture_setting(imgsensor.current_fps); 

	return ERROR_NONE;
}	/* capture() */

static kal_uint32 hd_4k_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
	imgsensor.pclk = imgsensor_info.custom1.pclk;
	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;  
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hd_4k_setting();
	
	return ERROR_NONE;
}	/*	hd_4k_video   */

static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;  
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting(imgsensor.current_fps);
	
	return ERROR_NONE;
}	/*	normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength; 
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	
	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength; 
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	return ERROR_NONE;
}
	
static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("%s.\n", __func__);
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;
	
	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;		

	sensor_resolution->SensorCustom1Width = imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height = imgsensor_info.custom1.grabwindow_height;		

	sensor_resolution->SensorHighSpeedVideoWidth	 = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight	 = imgsensor_info.hs_video.grabwindow_height;
	
	sensor_resolution->SensorSlimVideoWidth	 = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight	 = imgsensor_info.slim_video.grabwindow_height;

	return ERROR_NONE;
}	/*	get_resolution	*/

static kal_uint32 get_info(MSDK_SCENARIO_ID_ENUM scenario_id,
					  MSDK_SENSOR_INFO_STRUCT *sensor_info,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	
	//sensor_info->SensorVideoFrameRate = imgsensor_info.normal_video.max_framerate/10; /* not use */
	//sensor_info->SensorStillCaptureFrameRate= imgsensor_info.cap.max_framerate/10; /* not use */
	//imgsensor_info->SensorWebCamCaptureFrameRate= imgsensor_info.v.max_framerate; /* not use */

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame; 
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame; 
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;
	
	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame; 		 /* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;	/* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;	
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	
	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num; 
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */
	
	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  // 0 is default 1x
	sensor_info->SensorHightSampling = 0;	// 0 is default 1x 
	sensor_info->SensorPacketECCOrder = 1;
	sensor_info->PDAF_Support = 2;

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;		
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			sensor_info->SensorGrabStartX = imgsensor_info.cap.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.cap.mipi_data_lp2hs_settle_dc; 

			break;	 
		case MSDK_SCENARIO_ID_CUSTOM1:
			
			sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;
	   
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom1.mipi_data_lp2hs_settle_dc; 

			break;	  
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			
			sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;
	   
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc; 

			break;	  
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:			
			sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc; 

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;
				  
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc; 

			break;
		default:			
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx; 
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;		
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}
	
	return ERROR_NONE;
}	/*	get_info  */


static kal_uint32 control(MSDK_SCENARIO_ID_ENUM scenario_id, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			preview(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			capture(image_window, sensor_config_data);
			break;	
		case MSDK_SCENARIO_ID_CUSTOM1:
			hd_4k_video(image_window, sensor_config_data);
			break;	  
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			normal_video(image_window, sensor_config_data);
			break;	  
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			hs_video(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			slim_video(image_window, sensor_config_data);
			break;	  
		default:
			LOG_INF("Error ScenarioId setting");
			preview(image_window, sensor_config_data);
			return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}	/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);
	// SetVideoMode Function should fix framerate
	if (framerate == 0)
		// Dynamic frame rate
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps,1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d \n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) //enable auto flicker	  
		imgsensor.autoflicker_en = KAL_TRUE;
	else //Cancel Auto flick
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate) 
{
	kal_uint32 frame_length;
  
	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if(framerate == 0)
				return ERROR_NONE;
			frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength) : 0;			
			imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			if(framerate == 0)
				return ERROR_NONE;
			frame_length = imgsensor_info.custom1.pclk / framerate * 10 / imgsensor_info.custom1.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.custom1.framelength) ? (frame_length - imgsensor_info.custom1.framelength) : 0;			
			imgsensor.frame_length = imgsensor_info.custom1.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			break;	
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength): 0;	
			imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		default:  //coding with  preview scenario by default
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			LOG_INF("error scenario_id = %d, we use preview scenario \n", scenario_id);
			break;
	}	
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate) 
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*framerate = imgsensor_info.pre.max_framerate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*framerate = imgsensor_info.normal_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*framerate = imgsensor_info.custom1.max_framerate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*framerate = imgsensor_info.cap.max_framerate;
			break;		
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*framerate = imgsensor_info.hs_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO: 
			*framerate = imgsensor_info.slim_video.max_framerate;
			break;
		default:
			break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("%s, enable: %d\n", __func__, enable);

	if (enable) {
	} else {
	}	 
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
                             UINT8 *feature_para,UINT32 *feature_para_len)
{
    UINT16 *feature_return_para_16=(UINT16 *) feature_para;
    UINT16 *feature_data_16=(UINT16 *) feature_para;
    UINT32 *feature_return_para_32=(UINT32 *) feature_para;
    UINT32 *feature_data_32=(UINT32 *) feature_para;
    unsigned long long *feature_data=(unsigned long long *) feature_para;
    unsigned long long *feature_return_para=(unsigned long long *) feature_para;

	SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	SENSOR_VC_INFO_STRUCT *pvcinfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data=(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;
	SET_PD_BLOCK_INFO_T *PDAFinfo;

    LOG_INF("feature_id = %d\n", feature_id);
    switch (feature_id) {
        case SENSOR_FEATURE_GET_PERIOD:
            *feature_return_para_16++ = imgsensor.line_length;
            *feature_return_para_16 = imgsensor.frame_length;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            *feature_return_para_32 = imgsensor.pclk;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            set_shutter(*feature_data);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            night_mode((BOOL) *feature_data);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            set_gain((UINT16) *feature_data);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *feature_return_para_32=LENS_DRIVER_ID_DO_NOT_CARE;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            set_video_mode(*feature_data);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            get_imgsensor_id(feature_return_para_32);
            break;
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            set_auto_flicker_mode((BOOL)*feature_data_16,*(feature_data_16+1));
            break;
        case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
            set_max_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*feature_data, *(feature_data+1));
            break;
        case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
            get_default_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*(feature_data), (MUINT32 *)(uintptr_t)(*(feature_data+1)));
            break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            set_test_pattern_mode((BOOL)*feature_data);
            break;
        case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: //for factory mode auto testing
            *feature_return_para_32 = imgsensor_info.checksum_value;
            *feature_para_len=4;
            break;
        case SENSOR_FEATURE_SET_FRAMERATE:
            LOG_INF("current fps :%d\n", (UINT32)*feature_data);
            spin_lock(&imgsensor_drv_lock);
            imgsensor.current_fps = *feature_data;
            spin_unlock(&imgsensor_drv_lock);
            break;
        case SENSOR_FEATURE_SET_HDR:
            LOG_INF("ihdr enable :%d\n", (BOOL)*feature_data);
            spin_lock(&imgsensor_drv_lock);
            imgsensor.ihdr_en = (BOOL)*feature_data;
            spin_unlock(&imgsensor_drv_lock);
            break;
        case SENSOR_FEATURE_GET_CROP_INFO:
            LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n", (UINT32)*feature_data);

            wininfo = (SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

            switch (*feature_data_32) {
                case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[1],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[2],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[3],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_SLIM_VIDEO:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[4],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_CUSTOM1:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[5],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
                case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
                default:
                    memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[0],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
                    break;
            }
			break;
        case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
            LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",(UINT16)*feature_data,(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
            ihdr_write_shutter_gain((UINT16)*feature_data,(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
            break;
        /******************** PDAF START >>> *********/
		case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
			LOG_INF("SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY scenarioId:%d\n", *feature_data);
			//PDAF capacity enable or not, 2p8 only full size support PDAF
			switch (*feature_data) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
					break;
				case MSDK_SCENARIO_ID_CUSTOM1:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1; // video & capture use same setting
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
					break;
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
					break;
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
					break;
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
					break;
				default:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
					break;
			}
			break;
		case SENSOR_FEATURE_GET_PDAF_INFO:
			LOG_INF("SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%d\n", *feature_data);
			PDAFinfo= (SET_PD_BLOCK_INFO_T *)(uintptr_t)(*(feature_data+1));
		
			switch (*feature_data) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				case MSDK_SCENARIO_ID_CUSTOM1:
					memcpy((void *)PDAFinfo,(void *)&imgsensor_pd_info,sizeof(SET_PD_BLOCK_INFO_T));
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				default:
					break;
			}
			break;
		case SENSOR_FEATURE_GET_PDAF_DATA:	
			LOG_INF("SENSOR_FEATURE_GET_PDAF_DATA\n");
			read_imx386_pdaf_data((kal_uint16 )(*feature_data),(char*)(uintptr_t)(*(feature_data+1)),(kal_uint32)(*(feature_data+2)));
			break;	
		case SENSOR_FEATURE_GET_VC_INFO:
			LOG_INF("SENSOR_FEATURE_GET_VC_INFO %d\n", (UINT16)*feature_data);
			pvcinfo = (SENSOR_VC_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));
			switch (*feature_data_32) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					memcpy((void *)pvcinfo,(void *)&SENSOR_VC_INFO[1],sizeof(SENSOR_VC_INFO_STRUCT));
				break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					memcpy((void *)pvcinfo,(void *)&SENSOR_VC_INFO[2],sizeof(SENSOR_VC_INFO_STRUCT));
				break;
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				default:
					memcpy((void *)pvcinfo,(void *)&SENSOR_VC_INFO[0],sizeof(SENSOR_VC_INFO_STRUCT));
				break;
			}
			break;
			case SENSOR_FEATURE_SET_PDFOCUS_AREA:
            		LOG_INF("SENSOR_FEATURE_SET_IMX386_PDFOCUS_AREA Start Pos=%d, Size=%d, Idx=%d\n",(UINT32)*feature_data,(UINT32)*(feature_data+1), (UINT32)*(feature_data+2));
			imx386_set_pd_focus_area(*feature_data,*(feature_data+1),*(feature_data+2));
			break;
		/******************** PDAF END   <<< *********/
		default:
			break;
    }

    return ERROR_NONE;
}    /*    feature_control()  */


static SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};


UINT32 IMX386_SUNNY_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&sensor_func;
	return ERROR_NONE;
}	/*	IMX386_SUNNY_MIPI_RAW_SensorInit	*/
