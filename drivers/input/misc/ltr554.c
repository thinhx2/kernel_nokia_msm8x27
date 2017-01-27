/* Lite-On LTR-554ALS Android / Linux Driver
 *
 * Copyright (C) 2013 Lite-On Technology Corp (Singapore)
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/io.h>


/* LTR-554 Registers */
#define LTR554_ALS_CONTR	0x80
#define LTR554_PS_CONTR		0x81
#define LTR554_PS_LED		0x82
#define LTR554_PS_N_PULSES	0x83
#define LTR554_PS_MEAS_RATE	0x84
#define LTR554_ALS_MEAS_RATE	0x85
#define LTR554_PART_ID		0x86
#define LTR554_MANUFACTURER_ID	0x87
#define LTR554_ALS_DATA_CH1_0	0x88
#define LTR554_ALS_DATA_CH1_1	0x89
#define LTR554_ALS_DATA_CH0_0	0x8A
#define LTR554_ALS_DATA_CH0_1	0x8B
#define LTR554_ALS_PS_STATUS	0x8C
#define LTR554_PS_DATA_0	0x8D
#define LTR554_PS_DATA_1	0x8E
#define LTR554_INTERRUPT	0x8F
#define LTR554_PS_THRES_UP_0	0x90
#define LTR554_PS_THRES_UP_1	0x91
#define LTR554_PS_THRES_LOW_0	0x92
#define LTR554_PS_THRES_LOW_1	0x93
#define LTR554_PS_OFFSET_1	0x94
#define LTR554_PS_OFFSET_0	0x95
#define LTR554_ALS_THRES_UP_0	0x97
#define LTR554_ALS_THRES_UP_1	0x98
#define LTR554_ALS_THRES_LOW_0	0x99
#define LTR554_ALS_THRES_LOW_1	0x9A
#define LTR554_INTERRUPT_PRST	0x9E
/* LTR-554 Registers */


#define SET_BIT 1
#define CLR_BIT 0

#define ALS 0
#define PS 1
#define ALSPS 2
//#define PS_W_SATURATION_BIT	3

/* address 0x80 */
#define ALS_MODE_ACTIVE	(1 << 0)
#define ALS_MODE_STDBY		(0 << 0)
#define ALS_SW_RESET		(1 << 1)
#define ALS_SW_NRESET		(0 << 1)
#define ALS_GAIN_1x		(0 << 2)
#define ALS_GAIN_2x		(1 << 2)
#define ALS_GAIN_4x		(2 << 2)
#define ALS_GAIN_8x		(3 << 2)
#define ALS_GAIN_48x	(6 << 2)
#define ALS_GAIN_96x	(7 << 2)
#define ALS_MODE_RDBCK			0
#define ALS_SWRT_RDBCK			1
#define ALS_GAIN_RDBCK			2
#define ALS_CONTR_RDBCK		3

/* address 0x81 */
#define PS_MODE_ACTIVE		(3 << 0)
#define PS_MODE_STDBY		(0 << 0)
#define PS_GAIN_16x			(0 << 2)
#define PS_GAIN_32x			(2 << 2)
#define PS_GAIN_64x			(3 << 2)
#define PS_SATUR_INDIC_EN	(1 << 5)
#define PS_SATU_INDIC_DIS	(0 << 5)
#define PS_MODE_RDBCK		0
#define PS_GAIN_RDBCK		1
#define PS_SATUR_RDBCK		2
#define PS_CONTR_RDBCK		3

/* address 0x82 */
#define LED_CURR_5MA		(0 << 0)
#define LED_CURR_10MA		(1 << 0)
#define LED_CURR_20MA		(2 << 0)
#define LED_CURR_50MA		(3 << 0)
#define LED_CURR_100MA		(4 << 0)
#define LED_CURR_DUTY_25PC		(0 << 3)
#define LED_CURR_DUTY_50PC		(1 << 3)
#define LED_CURR_DUTY_75PC		(2 << 3)
#define LED_CURR_DUTY_100PC	(3 << 3)
#define LED_PUL_FREQ_30KHZ		(0 << 5)
#define LED_PUL_FREQ_40KHZ		(1 << 5)
#define LED_PUL_FREQ_50KHZ		(2 << 5)
#define LED_PUL_FREQ_60KHZ		(3 << 5)
#define LED_PUL_FREQ_70KHZ		(4 << 5)
#define LED_PUL_FREQ_80KHZ		(5 << 5)
#define LED_PUL_FREQ_90KHZ		(6 << 5)
#define LED_PUL_FREQ_100KHZ	(7 << 5)
#define LED_CURR_RDBCK			0
#define LED_CURR_DUTY_RDBCK	1
#define LED_PUL_FREQ_RDBCK		2
#define PS_LED_RDBCK			3

/* address 0x84 */
#define PS_MEAS_RPT_RATE_50MS		(0 << 0)
#define PS_MEAS_RPT_RATE_70MS		(1 << 0)
#define PS_MEAS_RPT_RATE_100MS	(2 << 0)
#define PS_MEAS_RPT_RATE_200MS	(3 << 0)
#define PS_MEAS_RPT_RATE_500MS	(4 << 0)
#define PS_MEAS_RPT_RATE_1000MS	(5 << 0)
#define PS_MEAS_RPT_RATE_2000MS	(6 << 0)
#define PS_MEAS_RPT_RATE_10MS		(8 << 0)

/* address 0x85 */
#define ALS_MEAS_RPT_RATE_50MS	(0 << 0)
#define ALS_MEAS_RPT_RATE_100MS	(1 << 0)
#define ALS_MEAS_RPT_RATE_200MS	(2 << 0)
#define ALS_MEAS_RPT_RATE_500MS	(3 << 0)
#define ALS_MEAS_RPT_RATE_1000MS	(4 << 0)
#define ALS_MEAS_RPT_RATE_2000MS	(5 << 0)
#define ALS_INTEG_TM_100MS		(0 << 3)
#define ALS_INTEG_TM_50MS			(1 << 3)
#define ALS_INTEG_TM_200MS		(2 << 3)
#define ALS_INTEG_TM_400MS		(3 << 3)
#define ALS_INTEG_TM_150MS		(4 << 3)
#define ALS_INTEG_TM_250MS		(5 << 3)
#define ALS_INTEG_TM_300MS		(6 << 3)
#define ALS_INTEG_TM_350MS		(7 << 3)
#define ALS_MEAS_RPT_RATE_RDBCK	0
#define ALS_INTEG_TM_RDBCK			1
#define ALS_MEAS_RATE_RDBCK		2

/* address 0x86 */
#define PART_NUM_ID_RDBCK		0
#define REVISION_ID_RDBCK		1
#define PART_ID_REG_RDBCK		2

/* address 0x8C */
#define PS_DATA_STATUS_RDBCK		0
#define PS_INTERR_STATUS_RDBCK	1
#define ALS_DATA_STATUS_RDBCK		2
#define ALS_INTERR_STATUS_RDBCK	3
#define ALS_GAIN_STATUS_RDBCK		4
#define ALS_VALID_STATUS_RDBCK	5
#define ALS_PS_STATUS_RDBCK		6

/* address 0x8F */
#define INT_MODE_00					(0 << 0)
#define INT_MODE_PS_TRIG			(1 << 0)
#define INT_MODE_ALS_TRIG			(2 << 0)
#define INT_MODE_ALSPS_TRIG		(3 << 0)
#define INT_POLAR_ACT_LO			(0 << 2)
#define INT_POLAR_ACT_HI			(1 << 2)
#define INT_MODE_RDBCK				0
#define INT_POLAR_RDBCK			1
#define INT_INTERRUPT_RDBCK		2

/* address 0x9E */
#define ALS_PERSIST_SHIFT	0
#define PS_PERSIST_SHIFT	4
#define ALS_PRST_RDBCK		0
#define PS_PRST_RDBCK		1
#define ALSPS_PRST_RDBCK	2

#define PON_DELAY		600

#define ALS_MIN_MEASURE_VAL	0
#define ALS_MAX_MEASURE_VAL	65535
#define ALS_VALID_MEASURE_MASK	ALS_MAX_MEASURE_VAL
#define PS_MIN_MEASURE_VAL	0
#define PS_MAX_MEASURE_VAL	2047
#define PS_VALID_MEASURE_MASK   PS_MAX_MEASURE_VAL
#define LO_LIMIT			0
#define HI_LIMIT			1
#define LO_N_HI_LIMIT	2
#define PS_OFFSET_MIN_VAL		0
#define PS_OFFSET_MAX_VAL		1023
#define PS_DEFAULT_LOW_THRESH   900
#define PS_DEFAULT_HIGH_THRESH  1200
#define FAR_VAL		PS_MAX_MEASURE_VAL
#define NEAR_VAL	PS_MIN_MEASURE_VAL
#define LTR554_ALS_THRESHOLD_HSYTERESIS 20
#define DRIVER_VERSION "1.13"
#define PARTID 0x90
#define PARTID_V5 0x92
#define MANUID 0x05
/*lux fac*/
#define AGC_UP_THRESHOLD		40000
#define AGC_DOWN_THRESHOLD  	5000
#define AGC_HYS					15
#define MAX_VAL					50000

#define I2C_RETRY 5

#define DEVICE_NAME "LTR554ALSPS"

#define ACT_INTERRUPT 1

#define LTR554_NV_BIN_FILE_NAME       "/persist/ps_als_sensor_nv.bin"

/*ALS work mode define*/
#define ALS_WORK_POLLING_MODE   1
#define ALS_WORK_INTERRUPT_MODE 2

#define ALS_WORK_MODE	ALS_WORK_POLLING_MODE

#define LTR554_ALS_POLL_SLOW          0 /*1 Hz (1s)*/
#define LTR554_ALS_POLL_MEDIUM        1 /* 10 Hz (100ms)*/
#define LTR554_ALS_POLL_FAST          2 /* 20 Hz (50ms)*/

/*
 * Magic Number
 * ============
 * Refer to file ioctl-number.txt for allocation
 */
#define LTR554_IOCTL_MAGIC      'c'

/* IOCTLs for ltr554 device */
#define LTR554_IOCTL_PS_ENABLE		_IOR(LTR554_IOCTL_MAGIC, 1, int *)
#define LTR554_IOCTL_PS_GET_ENABLED	_IOW(LTR554_IOCTL_MAGIC, 2, int *)
#define LTR554_IOCTL_ALS_ENABLE		_IOR(LTR554_IOCTL_MAGIC, 3, int *)
#define LTR554_IOCTL_ALS_GET_ENABLED	_IOW(LTR554_IOCTL_MAGIC, 4, int *)

//(Linux RTOS)>
#if 1
struct ltr554_platform_data {
	/* ALS */
	uint16_t pfd_levels[5];
	uint16_t pfd_als_lowthresh;
	uint16_t pfd_als_highthresh;
	int pfd_disable_als_on_suspend;

	/* PS */
	uint16_t pfd_ps_lowthresh;
	uint16_t pfd_ps_highthresh;
	int pfd_disable_ps_on_suspend;

	/* Interrupt */
	int pfd_gpio_int_no;
};
#endif
//(Linux RTOS)<


struct ltr554_data {
	/* Device */
	struct i2c_client *i2c_client;
	struct input_dev *als_input_dev;
	struct input_dev *ps_input_dev;
	struct delayed_work dwork;    /* for PS & ALS interrupt */
	struct delayed_work	als_dwork;	/* for ALS polling */
	struct early_suspend early_suspend;
	struct wake_lock ps_wake_lock;
	struct mutex bus_lock;

	atomic_t suspended;
	/* Device mode
	 * 0 = ALS
	 * 1 = PS
	 */
	uint8_t mode;

	/* ALS */
	uint8_t als_enable_flag;
	uint8_t als_suspend_enable_flag;
	uint8_t als_irq_flag;
	uint8_t als_opened;
	uint16_t als_lowthresh;
	uint16_t als_highthresh;
	uint16_t default_als_lowthresh;
	uint16_t default_als_highthresh;
	uint16_t *adc_levels;
	uint16_t als_poll_delay;	/* needed for light sensor polling : micro-second (us) */
	/* Flag to suspend ALS on suspend or not */
	uint8_t disable_als_on_suspend;
	uint16_t als_fac;

	/* PS */
	uint8_t ps_enable_flag;
	uint8_t ps_suspend_enable_flag;
	uint8_t ps_irq_flag;
	uint8_t ps_opened;
	uint16_t ps_lowthresh;
	uint16_t ps_highthresh;
	uint16_t default_ps_lowthresh;
	uint16_t default_ps_highthresh;
	/* Flag to suspend PS on suspend or not */
	uint8_t disable_ps_on_suspend;

	/* LED */
	int led_pulse_freq;
	int led_duty_cyc;
	int led_peak_curr;
	int led_pulse_count;

	/* Interrupt */
	int irq;
	int gpio_int_no;
	int is_suspend;

	/* For selftest */
	atomic_t ps_selftest_ongoing;
	atomic_t ps_selftest_int;
	wait_queue_head_t ps_selftest_wq;
};

/* Pay attention to this structure, it must be the same as Selftest HAL */
	struct PROX_SELFTEST_RESULT {
	bool i2c_status;
	bool interrupt_pin_support;
	bool interrupt_pin_status;
};

struct ltr554_nvitem_data {

	uint8_t  sensor_part_id;
	uint8_t  sensor_manufac_id;
	/* PS */
	uint16_t threshold_cross_talk;		/*interrupt threshold level*/
	uint16_t offset_factor;

	/*ALS*/
	uint16_t als_cal_fac;		/*ALS factor after calibration*/
	uint16_t winfac1_factor;
	uint16_t winfac2_factor;
	uint16_t winfac3_factor;
	uint8_t  nv_als_calibration_done;
};

static struct ltr554_data *sensor_info;
static struct workqueue_struct *ltr554_workqueue;

static uint16_t winfac1 = 174;
static uint16_t winfac2 = 174;
static uint16_t winfac3 = 27;
static uint8_t eqn_prev = 0;
static uint8_t ratio_old = 0;
static uint16_t ps_init_kept_data[8];
static uint8_t ps_grabData_stage = 0;
static uint16_t lux_val_prev = 0;
static uint8_t ps_kept_data_counter = 0;
static uint16_t ps_init_kept_data[8];
static uint16_t als_ch0_val = 0;
/*
* Management functions
*/

static void ltr554_wait_for_device_resume(struct ltr554_data *ltr554)
{
	dev_info(&ltr554->i2c_client->dev,"%s, ltr554_wait_for_device_resume\n", __func__);
	do {
		if (atomic_read(&ltr554->suspended) == 0)
			break;
		else{
			dev_dbg(&ltr554->i2c_client->dev,"%s, suspended state is %d\n", __func__, atomic_read(&ltr554->suspended));
			msleep(10);
		}
	} while(1);
}
/* I2C Read */
// take note ---------------------------------------
// for i2c read, need to send the register address follwed by buffer over to register.
// There should not be a stop in between register address and buffer.
// There should not be release of lock in between register address and buffer.
// take note ---------------------------------------
static int8_t I2C_Read(uint8_t *rxData, uint8_t length)
{
	int8_t index;
	struct ltr554_data *ltr554 = sensor_info;
	struct i2c_msg data[] = {
		{
			.addr = sensor_info->i2c_client->addr,
			.flags = 0,
			.len = 1,
			.buf = rxData,
		},
		{
			.addr = sensor_info->i2c_client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxData,
		},
	};
    //pr_info("%x: ltr554 I2C_Read \n", sensor_info->i2c_client->addr);
	for (index = 0; index < I2C_RETRY; index++) {
		if (i2c_transfer(sensor_info->i2c_client->adapter, data, 2) > 0)
			break;

		dev_info(&ltr554->i2c_client->dev,"%s, I2C_Read retry is %d\n", __func__, index);
		mdelay(10);
	}

	if (index >= I2C_RETRY) {
		pr_alert("%s I2C Read Fail !!!!\n",__func__);
		return -EIO;
	}

	return 0;
}


/* I2C Write */
static int8_t I2C_Write(uint8_t *txData, uint8_t length)
{
	int8_t index;
	struct ltr554_data *ltr554 = sensor_info;
	struct i2c_msg data[] = {
		{
			.addr = sensor_info->i2c_client->addr,
			.flags = 0,
			.len = length,
			.buf = txData,
		},
	};

	for (index = 0; index < I2C_RETRY; index++) {
		if (i2c_transfer(sensor_info->i2c_client->adapter, data, 1) > 0)
			break;
		dev_info(&ltr554->i2c_client->dev,"%s, I2C_Write retry is %d\n", __func__, index);
		mdelay(10);
	}

	if (index >= I2C_RETRY) {
		pr_alert("%s I2C Write Fail !!!!\n", __func__);
		return -EIO;
	}

	return 0;
}


/* Set register bit */
static int8_t _ltr554_set_bit(struct i2c_client *client, uint8_t set, uint8_t cmd, uint8_t data)
{
	uint8_t buffer[2];
	uint8_t value;
	int8_t ret = 0;

	buffer[0] = cmd;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	if (set)
		value |= data;
	else
		value &= ~data;

	buffer[0] = cmd;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return -EIO;
	}

	return ret;
}


static uint16_t lux_formula(uint16_t ch0_adc, uint16_t ch1_adc, uint8_t eqtn)
{
	uint32_t luxval = 0;
	uint32_t luxval_i = 0;
	uint32_t luxval_f = 0;
	uint16_t ch0_coeff_i = 0;
	uint16_t ch1_coeff_i = 0;
	uint16_t ch0_coeff_f = 0;
	uint16_t ch1_coeff_f = 0;
	int8_t ret;
	uint8_t gain = 1, als_int_fac;
	uint8_t buffer[2];
	uint16_t win_fac = 0;
	int8_t fac = 1;
	struct ltr554_data *ltr554 = sensor_info;

	dev_dbg(&ltr554->i2c_client->dev,
			"%s |  ch0_ch1_adc = 0x%x,0x%x\n", __func__, ch0_adc,ch1_adc);
	buffer[0] = LTR554_ALS_PS_STATUS;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		pr_alert("%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	gain = (buffer[0] & 0x70);
	gain >>= 4;

	if (gain == 0) {			//gain 1
		gain = 1;
	} else if (gain == 1) {		//gain 2
		gain = 2;
	} else if (gain == 2) {		//gain 4
		gain = 4;
	} else if (gain == 3) {		//gain 8
		gain = 8;
	} else if (gain == 6) {		//gain 48
		gain = 48;
	} else if (gain == 7) {		//gain 96
		gain = 96;
	}

	buffer[0] = LTR554_ALS_MEAS_RATE;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		pr_alert("%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}
	als_int_fac = buffer[0] & 0x38;
	als_int_fac >>= 3;

	if (als_int_fac == 0) {
		als_int_fac = 10;
	} else if (als_int_fac == 1) {
		als_int_fac = 5;
	} else if (als_int_fac == 2) {
		als_int_fac = 20;
	} else if (als_int_fac == 3) {
		als_int_fac = 40;
	} else if (als_int_fac == 4) {
		als_int_fac = 15;
	} else if (als_int_fac == 5) {
		als_int_fac = 25;
	} else if (als_int_fac == 6) {
		als_int_fac = 30;
	} else if (als_int_fac == 7) {
		als_int_fac = 35;
	}

	if (eqtn == 1) {
		ch0_coeff_i = 1;
		ch1_coeff_i = 1;
		ch0_coeff_f = 7743;
		ch1_coeff_f = 1059;
		fac = 1;
		win_fac = winfac1;
		luxval_i = ((ch0_adc * ch0_coeff_i) + (ch1_adc * ch1_coeff_i)) * win_fac;
		luxval_f = (((ch0_adc * ch0_coeff_f) + (ch1_adc * ch1_coeff_f)) / 100) * win_fac;
		//luxval = ((17743 * ch0_calc) + (11059 * ch1_adc));
		//luxval = ((1.7743 * ch0_calc) + (1.1059 * ch1_adc)) / (gain * (als_int_fac / 10));
	} else if (eqtn == 2) {
		ch0_coeff_i = 4;
		ch1_coeff_i = 1;
		ch0_coeff_f = 2785;
		ch1_coeff_f = 9548;
		win_fac = winfac2;
		if ((ch1_coeff_f * ch1_adc) < (ch0_adc * ch0_coeff_f)) {
			fac = 1;
			luxval_f = (((ch0_adc * ch0_coeff_f) - (ch1_adc * ch1_coeff_f)) / 100) * win_fac;
		} else {
			fac = -1;
			luxval_f = (((ch1_adc * ch1_coeff_f) - (ch0_adc * ch0_coeff_f)) / 100) * win_fac;
		}
		luxval_i = ((ch0_adc * ch0_coeff_i) - (ch1_adc * ch1_coeff_i)) * win_fac;
		//luxval = ((42785 * ch0_calc) - (10696 * ch1_adc));
		//luxval = ((4.2785 * ch0_calc) - (1.9548 * ch1_adc)) / (gain * (als_int_fac / 10));
	} else if (eqtn == 3) {
		ch0_coeff_i = 0;
		ch1_coeff_i = 0;
		ch0_coeff_f = 5926;
		ch1_coeff_f = 1185;
		fac = 1;
		win_fac = winfac3;
		luxval_i = ((ch0_adc * ch0_coeff_i) + (ch1_adc * ch1_coeff_i)) * win_fac;
		luxval_f = (((ch0_adc * ch0_coeff_f) + (ch1_adc * ch1_coeff_f)) / 100) * win_fac;
		//luxval = ((5926 * ch0_calc) + (1185 * ch1_adc));
		//luxval = ((0.5926 * ch0_calc) + (0.1185 * ch1_adc)) / (gain * (als_int_fac / 10));
	} else if (eqtn == 4) {
		ch0_coeff_i = 0;
		ch1_coeff_i = 0;
		ch0_coeff_f = 0;
		ch1_coeff_f = 0;
		fac = 1;
		luxval_i = 0;
		luxval_f = 0;
		//luxval = 0;
	}
	if( fac < 0 ){
		luxval = (luxval_i  - luxval_f / 100) / (gain * als_int_fac);
	}
	else{
		luxval = (luxval_i  + luxval_f / 100) / (gain * als_int_fac);
	}

	dev_dbg(&ltr554->i2c_client->dev,
			"%s |  luxval = 0x%x\n", __func__, luxval);
	return luxval;
}


static uint16_t ratioHysterisis (uint16_t ch0_adc, uint16_t ch1_adc)
{
#define	RATIO_HYSVAL	0
	int ratio;
	uint8_t buffer[2], eqn_now;
	int8_t ret;
	uint16_t ch0_calc;
	uint32_t luxval = 0;
	int abs_ratio_now_old;

	buffer[0] = LTR554_ALS_CONTR;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		pr_alert("%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	ch0_calc = ch0_adc;
	if ((buffer[0] & 0x20) == 0x20) {
		ch0_calc = ch0_adc - ch1_adc;
	}

	if ((ch1_adc + ch0_calc) == 0) {
		ratio = 100;
	} else {
		ratio = (ch1_adc*100) / (ch1_adc + ch0_calc);
	}

	if (ratio < 45) {
		eqn_now = 1;
	} else if ((ratio >= 45) && (ratio < 68)) {
		eqn_now = 2;
	} else if ((ratio >= 68) && (ratio < 97)) {
		eqn_now = 3;
	} else if (ratio >= 97) {
		eqn_now = 4;
	}

	if (eqn_prev == 0) {
		luxval = lux_formula(ch0_calc, ch1_adc, eqn_now);
		ratio_old = ratio;
		eqn_prev = eqn_now;
	} else {
		if (eqn_now == eqn_prev) {
			luxval = lux_formula(ch0_calc, ch1_adc, eqn_now);
			ratio_old = ratio;
			eqn_prev = eqn_now;
		} else {
			abs_ratio_now_old = ratio - ratio_old;
			if (abs_ratio_now_old < 0) {
				abs_ratio_now_old *= (-1);
			}
			if (abs_ratio_now_old >= RATIO_HYSVAL) {
				luxval = lux_formula(ch0_calc, ch1_adc, eqn_now);
				ratio_old = ratio;
				eqn_prev = eqn_now;
			} else {
				luxval = lux_formula(ch0_calc, ch1_adc, eqn_prev);
			}
		}
	}

	return luxval;
}


static uint16_t read_als_adc_value(struct ltr554_data *ltr554)
{

	int8_t ret = -99;
	uint16_t value = -99;
	int ch0_val;
	int ch1_val;
	uint8_t gain, value_temp, gain_chg_req = 0;
	uint8_t buffer[4], temp;



	//mutex_lock(&ltr554->bus_lock);

	/* ALS */
	buffer[0] = LTR554_ALS_DATA_CH1_0;

	/* read data bytes from data regs */
	ret = I2C_Read(buffer, 4);

	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}

	/* ALS Ch0 */
 	ch0_val = (uint16_t)buffer[2] | ((uint16_t)buffer[3] << 8);
		dev_dbg(&ltr554->i2c_client->dev,
			"%s | als_ch0 value = 0x%04X\n", __func__,
			ch0_val);

	if (ch0_val > ALS_MAX_MEASURE_VAL) {
		dev_err(&ltr554->i2c_client->dev,
		        "%s: ALS Value Error: 0x%X\n", __func__,
		        ch0_val);
	}
	ch0_val &= ALS_VALID_MEASURE_MASK;
	//input_report_abs(ltr554->als_input_dev, ABS_MISC, ch0_val);
	//input_sync(ltr554->als_input_dev);
	als_ch0_val = ch0_val;
	/* ALS Ch1 */
 	ch1_val = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
		dev_dbg(&ltr554->i2c_client->dev,
			"%s | als_ch1 value = 0x%04X\n", __func__,
			ch1_val);

	if (ch1_val > ALS_MAX_MEASURE_VAL) {
		dev_err(&ltr554->i2c_client->dev,
		        "%s: ALS Value Error: 0x%X\n", __func__,
		        ch1_val);
	}
	ch1_val &= ALS_VALID_MEASURE_MASK;
	//input_report_abs(ltr554->als_input_dev, ABS_MISC, ch1_val);
	//input_sync(ltr554->als_input_dev);

	buffer[0] = LTR554_ALS_PS_STATUS;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}

	value_temp = buffer[0];
	temp = buffer[0];
	gain = (value_temp & 0x70);
	gain >>= 4;

	if (gain == 0) {			//gain 1
		gain = 1;
	} else if (gain == 1) {		//gain 2
		gain = 2;
	} else if (gain == 2) {		//gain 4
		gain = 4;
	} else if (gain == 3) {		//gain 8
		gain = 8;
	} else if (gain == 6) {		//gain 48
		gain = 48;
	} else if (gain == 7) {		//gain 96
		gain = 96;
	}

	buffer[0] = LTR554_ALS_CONTR;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}
	value_temp = buffer[0];
	value_temp &= 0xE3;

	if ((ch0_val == 0) && (ch1_val > 50 )) {
		value = lux_val_prev;
	} else {
		if (gain == 1) {
			if ((ch0_val + ch1_val) < ((AGC_DOWN_THRESHOLD * 10) / AGC_HYS)) {
				value = ratioHysterisis(ch0_val, ch1_val);
				value_temp |= ALS_GAIN_8x;
				gain_chg_req = 1;
			} else {
				value = ratioHysterisis(ch0_val, ch1_val);
			}
		} else if (gain == 8) {
			if ((ch0_val + ch1_val) > AGC_UP_THRESHOLD) {
				value = ratioHysterisis(ch0_val, ch1_val);
				value_temp |= ALS_GAIN_1x;
				gain_chg_req = 1;
			} else {
				value = ratioHysterisis(ch0_val, ch1_val);
			}
		} else {
			value = ratioHysterisis(ch0_val, ch1_val);
		}
		if (gain_chg_req) {
			buffer[0] = LTR554_ALS_CONTR;
			buffer[1] = value_temp;
			ret = I2C_Write(buffer, 2);
			if (ret < 0) {
				dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);

				//mutex_unlock(&ltr554->bus_lock);

				return ret;
			}
		}

	}

	if ((value > MAX_VAL) || (((ch0_val + ch1_val) > MAX_VAL) && (temp & 0x80))) {
		value = MAX_VAL;
	}
	lux_val_prev = value;

	//mutex_unlock(&ltr554->bus_lock);
	dev_dbg(&ltr554->i2c_client->dev,
			"%s |  value = 0x%04X\n", __func__, value);
	return value;
}


static uint16_t read_ps_adc_value(struct ltr554_data *ltr554)
{
	int8_t ret = -99;
	uint16_t value = -99;
	uint16_t ps_val;
	uint8_t buffer[4];

	//mutex_lock(&ltr554->bus_lock);

	buffer[0] = LTR554_PS_DATA_0;

	/* read data bytes from data regs */
	ret = I2C_Read(buffer, 2);

	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}

	ps_val = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
		dev_dbg(&ltr554->i2c_client->dev,
			"%s | ps value = 0x%04X\n", __func__,
			ps_val);

	if (ps_val > PS_MAX_MEASURE_VAL) {
		dev_err(&ltr554->i2c_client->dev,
		        "%s: PS Value Error: 0x%X\n", __func__,
		        ps_val);
	}
	ps_val &= PS_VALID_MEASURE_MASK;

	value = ps_val;

	//mutex_unlock(&ltr554->bus_lock);

	return value;
}


static int read_ps_als_nvitem(struct ltr554_data *ltr554)
{
	int ret = 0;
	mm_segment_t old_fs;
	loff_t sz_file;
	char *pReadBuff = NULL;
	unsigned long magic;

	struct ltr554_nvitem_data *ltr554_nvitem;
	struct inode *inode;
	struct file *file;

	file = filp_open( LTR554_NV_BIN_FILE_NAME, O_RDONLY, 0 );
	if (IS_ERR(file)){
		dev_err(&ltr554->i2c_client->dev, "%s: NV file doesn't exist, use default setting \n", __func__);
		return -1;
	}
	dev_info(&ltr554->i2c_client->dev, "%s: open NV file sucess\n", __func__);
	inode=file->f_dentry->d_inode;
	magic=inode->i_sb->s_magic;
	sz_file=inode->i_size;

	dev_info(&ltr554->i2c_client->dev, "%s,nvitem file size = %x \n", __func__,(int)sz_file);


	if (sz_file >= sizeof(*ltr554_nvitem)){
		pReadBuff = kzalloc(sizeof(*ltr554_nvitem) + 1,GFP_KERNEL);
	}
	else{			/* skip when file is invaild*/
		filp_close(file,NULL);
		dev_err(&ltr554->i2c_client->dev, "%s: file size error...\n", __func__);
		return -1;
	}

	if (!pReadBuff){	/*mem alloc  failed*/
		dev_err(&ltr554->i2c_client->dev, "%s: Mem Alloc Fail...\n", __func__);
		filp_close(file,NULL);
		return -ENOMEM;
	}

	old_fs =get_fs();
	set_fs(KERNEL_DS);

	if(file->f_op->llseek(file, 0, SEEK_SET) != 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: lseek failure...\n", __func__);
		ret = -1;
		goto Exit;
	}

	ret = file->f_op->read( file, pReadBuff, sizeof(*ltr554_nvitem), &file->f_pos );
	if(!ret){
		goto Exit;
	}

	ltr554_nvitem = (struct ltr554_nvitem_data *)pReadBuff;
	if((ltr554_nvitem->sensor_part_id != PARTID) && (ltr554_nvitem->sensor_part_id != PARTID_V5)){
		dev_err(&ltr554->i2c_client->dev, "%s: Error nvitem file...\n", __func__);
		ret = -1;
		goto Exit;
	}
	ltr554->default_ps_highthresh = ltr554_nvitem->threshold_cross_talk;
	/*set threshold from FAR to NEAR as 300*/
	ltr554->default_ps_lowthresh =
			ltr554->default_ps_highthresh - (PS_DEFAULT_HIGH_THRESH - PS_DEFAULT_LOW_THRESH);

	if(ltr554_nvitem->nv_als_calibration_done == 1){
		ltr554->als_fac = ltr554_nvitem->als_cal_fac;
	}

Exit:
	/*free buffer and close file*/
	kfree(pReadBuff);
	filp_close(file,NULL);
	/*resume user space*/
	set_fs(old_fs);

	return ret;
}

static int8_t als_mode_setup (uint8_t alsMode_set_reset, struct ltr554_data *ltr554)
{
	int8_t ret = 0;

	ret = _ltr554_set_bit(ltr554->i2c_client, alsMode_set_reset, LTR554_ALS_CONTR, ALS_MODE_ACTIVE);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s ALS mode setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t als_sw_reset_setup(uint8_t alsSWReset_set_reset, struct ltr554_data *ltr554)
{
	int8_t ret = 0;

	ret = _ltr554_set_bit(ltr554->i2c_client, alsSWReset_set_reset, LTR554_ALS_CONTR, ALS_SW_RESET);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s ALS sw reset setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t als_gain_setup (uint8_t alsgain_range, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_ALS_CONTR;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0xE3;

	if (alsgain_range == 1) {
		value |= ALS_GAIN_1x;
	} else if (alsgain_range == 2) {
		value |= ALS_GAIN_2x;
	} else if (alsgain_range == 4) {
		value |= ALS_GAIN_4x;
	} else if (alsgain_range == 8) {
		value |= ALS_GAIN_8x;
	} else if (alsgain_range == 48) {
		value |= ALS_GAIN_48x;
	} else if (alsgain_range == 96) {
		value |= ALS_GAIN_96x;
	}

	buffer[0] = LTR554_ALS_CONTR;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s ALS gain setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t als_contr_setup(uint8_t als_contr_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3];

	buffer[0] = LTR554_ALS_CONTR;

	/* Default settings used for now. */
	buffer[1] = als_contr_val;
	buffer[1] &= 0x1F;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | ALS_CONTR (0x%02X) setup fail...", __func__, buffer[0]);
	}

	return ret;
}


static int8_t als_contr_readback (uint8_t rdbck_type, uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_ALS_CONTR;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	if (rdbck_type == ALS_MODE_RDBCK) {
		*retVal = value & 0x01;
	} else if (rdbck_type == ALS_SWRT_RDBCK) {
		*retVal = (value & 0x02) >> 1;
	} else if (rdbck_type == ALS_GAIN_RDBCK) {
		*retVal = (value & 0x1C) >> 2;
	} else if (rdbck_type == ALS_CONTR_RDBCK) {
		*retVal = value & 0x1F;
	}

	return ret;
}

static int8_t ps_mode_setup (uint8_t psMode_set_reset, struct ltr554_data *ltr554)
{
	int8_t ret = 0;

	ret = _ltr554_set_bit(ltr554->i2c_client, psMode_set_reset, LTR554_PS_CONTR, PS_MODE_ACTIVE);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS mode setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t ps_gain_setup (uint8_t psgain_range, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_PS_CONTR;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0xF3;

	if (psgain_range == 16) {
		value |= PS_GAIN_16x;
	} else if (psgain_range == 32) {
		value |= PS_GAIN_32x;
	} else if (psgain_range == 64) {
		value |= PS_GAIN_64x;
	}

	buffer[0] = LTR554_PS_CONTR;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS gain setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t ps_satu_indica_setup(uint8_t pssatuindica_enable, struct ltr554_data *ltr554)
{
	int8_t ret = 0;

	ret = _ltr554_set_bit(ltr554->i2c_client, pssatuindica_enable, LTR554_PS_CONTR, PS_SATUR_INDIC_EN);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS saturation indicator setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t ps_contr_setup(uint8_t ps_contr_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3];

	buffer[0] = LTR554_PS_CONTR;

	/* Default settings used for now. */
	buffer[1] = ps_contr_val;
	buffer[1] &= 0x2F;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | PS_CONTR (0x%02X) setup fail...", __func__, buffer[0]);
	}

	return ret;
}


static int8_t ps_contr_readback (uint8_t rdbck_type, uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_PS_CONTR;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	if (rdbck_type == PS_MODE_RDBCK) {
		*retVal = (value & 0x03);
	} else if (rdbck_type == PS_GAIN_RDBCK) {
		*retVal = (value & 0x0C) >> 2;
	} else if (rdbck_type == PS_SATUR_RDBCK) {
		*retVal = (value & 0x20) >> 5;
	} else if (rdbck_type == PS_CONTR_RDBCK) {
		*retVal = value & 0x2F;
	}

	return ret;
}


static int8_t ps_ledCurrent_setup (uint8_t psledcurr_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_PS_LED;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0xF8;

	if (psledcurr_val == 5) {
		value |= LED_CURR_5MA;
	} else if (psledcurr_val == 10) {
		value |= LED_CURR_10MA;
	} else if (psledcurr_val == 20) {
		value |= LED_CURR_20MA;
	} else if (psledcurr_val == 50) {
		value |= LED_CURR_50MA;
	} else if (psledcurr_val == 100) {
		value |= LED_CURR_100MA;
	}

	buffer[0] = LTR554_PS_LED;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS LED current setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t ps_ledCurrDuty_setup (uint8_t psleddutycycle_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_PS_LED;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0xE7;

	if (psleddutycycle_val == 25) {
		value |= LED_CURR_DUTY_25PC;
	} else if (psleddutycycle_val == 50) {
		value |= LED_CURR_DUTY_50PC;
	} else if (psleddutycycle_val == 75) {
		value |= LED_CURR_DUTY_75PC;
	} else if (psleddutycycle_val == 100) {
		value |= LED_CURR_DUTY_100PC;
	}

	buffer[0] = LTR554_PS_LED;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS LED current duty setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t ps_ledPulseFreq_setup (uint8_t pspulreq_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_PS_LED;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0x1F;

	if (pspulreq_val == 30) {
		value |= LED_PUL_FREQ_30KHZ;
	} else if (pspulreq_val == 40) {
		value |= LED_PUL_FREQ_40KHZ;
	} else if (pspulreq_val == 50) {
		value |= LED_PUL_FREQ_50KHZ;
	} else if (pspulreq_val == 60) {
		value |= LED_PUL_FREQ_60KHZ;
	} else if (pspulreq_val == 70) {
		value |= LED_PUL_FREQ_70KHZ;
	} else if (pspulreq_val == 80) {
		value |= LED_PUL_FREQ_80KHZ;
	} else if (pspulreq_val == 90) {
		value |= LED_PUL_FREQ_90KHZ;
	} else if (pspulreq_val == 100) {
		value |= LED_PUL_FREQ_100KHZ;
	}

	buffer[0] = LTR554_PS_LED;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS LED pulse frequency setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


/* LED Setup */
static int8_t ps_led_setup(uint8_t ps_led_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3];

	buffer[0] = LTR554_PS_LED;

	/* Default settings used for now. */
	buffer[1] = ps_led_val;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | PS_LED (0x%02X) setup fail...", __func__, buffer[0]);
	}

	return ret;
}


static int8_t ps_led_readback (uint8_t rdbck_type, uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_PS_LED;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	if (rdbck_type == LED_CURR_RDBCK) {
		*retVal = (value & 0x07);
	} else if (rdbck_type == LED_CURR_DUTY_RDBCK) {
		*retVal = (value & 0x18) >> 3;
	} else if (rdbck_type == LED_PUL_FREQ_RDBCK) {
		*retVal = (value & 0xE0) >> 5;
	} else if (rdbck_type == PS_LED_RDBCK) {
		*retVal = value;
	}

	return ret;
}


static int8_t ps_ledPulseCount_setup(uint8_t pspulsecount_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3];

	buffer[0] = LTR554_PS_N_PULSES;

	/* Default settings used for now. */
	if (pspulsecount_val > 15) {
		pspulsecount_val = 15;
	}
	buffer[1] = pspulsecount_val;
	buffer[1] &= 0x0F;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | PS_LED_COUNT (0x%02X) setup fail...", __func__, buffer[0]);
	}

	return ret;
}


static int8_t ps_ledPulseCount_readback (uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3], value;

	buffer[0] = LTR554_PS_N_PULSES;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	*retVal = value;

	return ret;
}


static int8_t ps_meas_rate_setup(uint16_t meas_rate_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	//mutex_lock(&ltr554->bus_lock);

	buffer[0] = LTR554_PS_MEAS_RATE;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}

	value = buffer[0];
	value &= 0xF0;

	if (meas_rate_val == 50) {
		value |= PS_MEAS_RPT_RATE_50MS;
	} else if (meas_rate_val == 70) {
		value |= PS_MEAS_RPT_RATE_70MS;
	} else if (meas_rate_val == 100) {
		value |= PS_MEAS_RPT_RATE_100MS;
	} else if (meas_rate_val == 200) {
		value |= PS_MEAS_RPT_RATE_200MS;
	} else if (meas_rate_val == 500) {
		value |= PS_MEAS_RPT_RATE_500MS;
	} else if (meas_rate_val == 1000) {
		value |= PS_MEAS_RPT_RATE_1000MS;
	} else if (meas_rate_val == 2000) {
		value |= PS_MEAS_RPT_RATE_2000MS;
	} else if (meas_rate_val == 10) {
		value |= PS_MEAS_RPT_RATE_10MS;
	}

	buffer[0] = LTR554_PS_MEAS_RATE;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS measurement rate setup fail...\n", __func__);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}

	//mutex_unlock(&ltr554->bus_lock);

	return ret;
}


static int8_t ps_meas_rate_readback (uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3], value;

	buffer[0] = LTR554_PS_MEAS_RATE;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	*retVal = (value & 0x0F);

	return ret;
}


static int8_t als_meas_rate_setup(uint16_t meas_rate_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_ALS_MEAS_RATE;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0xF8;

	if (meas_rate_val == 50) {
		value |= ALS_MEAS_RPT_RATE_50MS;
	} else if (meas_rate_val == 100) {
		value |= ALS_MEAS_RPT_RATE_100MS;
	} else if (meas_rate_val == 200) {
		value |= ALS_MEAS_RPT_RATE_200MS;
	} else if (meas_rate_val == 500) {
		value |= ALS_MEAS_RPT_RATE_500MS;
	} else if (meas_rate_val == 1000) {
		value |= ALS_MEAS_RPT_RATE_1000MS;
	} else if (meas_rate_val == 2000) {
		value |= ALS_MEAS_RPT_RATE_2000MS;
	}

	buffer[0] = LTR554_ALS_MEAS_RATE;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s ALS measurement rate setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t als_integ_time_setup(uint16_t integ_time_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_ALS_MEAS_RATE;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0xC7;

	if (integ_time_val == 100) {
		value |= ALS_INTEG_TM_100MS;
	} else if (integ_time_val == 50) {
		value |= ALS_INTEG_TM_50MS;
	} else if (integ_time_val == 200) {
		value |= ALS_INTEG_TM_200MS;
	} else if (integ_time_val == 400) {
		value |= ALS_INTEG_TM_400MS;
	} else if (integ_time_val == 150) {
		value |= ALS_INTEG_TM_150MS;
	} else if (integ_time_val == 250) {
		value |= ALS_INTEG_TM_250MS;
	} else if (integ_time_val == 300) {
		value |= ALS_INTEG_TM_300MS;
	} else if (integ_time_val == 350) {
		value |= ALS_INTEG_TM_350MS;
	}

	buffer[0] = LTR554_ALS_MEAS_RATE;
	buffer[1] = value;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s ALS integration time setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t als_meas_rate_reg_setup(uint8_t als_meas_rate_reg_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3];

	buffer[0] = LTR554_ALS_MEAS_RATE;

	buffer[1] = als_meas_rate_reg_val;
	buffer[1] &= 0x3F;
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | ALS_MEAS_RATE (0x%02X) setup fail...", __func__, buffer[0]);
	}

	return ret;
}


static int8_t als_meas_rate_readback (uint8_t rdbck_type, uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_ALS_MEAS_RATE;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	if (rdbck_type == ALS_MEAS_RPT_RATE_RDBCK) {
		*retVal = (value & 0x07);
	} else if (rdbck_type == ALS_INTEG_TM_RDBCK) {
		*retVal = (value & 0x38) >> 3;
	} else if (rdbck_type == ALS_MEAS_RATE_RDBCK) {
		*retVal = (value & 0x3F);
	}

	return ret;
}


static int8_t part_ID_reg_readback (uint8_t rdbck_type, uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[1], value;

	buffer[0] = LTR554_PART_ID;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	if (rdbck_type == PART_NUM_ID_RDBCK) {
		*retVal = (value & 0xF0) >> 4;
	} else if (rdbck_type == REVISION_ID_RDBCK) {
		*retVal = value & 0x0F;
	} else if (rdbck_type == PART_ID_REG_RDBCK) {
		*retVal = value;
	}

	return ret;
}


static int8_t manu_ID_reg_readback (uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[1], value;

	buffer[0] = LTR554_MANUFACTURER_ID;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	*retVal = value;

	return ret;
}


static int8_t als_ps_status_reg (uint8_t data_status_type, uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_ALS_PS_STATUS;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	if (data_status_type == PS_DATA_STATUS_RDBCK) {
		*retVal = (value & 0x01);
	} else if (data_status_type == PS_INTERR_STATUS_RDBCK) {
		*retVal = (value & 0x02) >> 1;
	} else if (data_status_type == ALS_DATA_STATUS_RDBCK) {
		*retVal = (value & 0x04) >> 2;
	} else if (data_status_type == ALS_INTERR_STATUS_RDBCK) {
		*retVal = (value & 0x08) >> 3;
	} else if (data_status_type == ALS_GAIN_STATUS_RDBCK) {
		*retVal = (value & 0x70) >> 4;
	} else if (data_status_type == ALS_VALID_STATUS_RDBCK) {
		*retVal = (value & 0x80) >> 7;
	} else if (data_status_type == ALS_PS_STATUS_RDBCK) {
		*retVal = value;
	}

	return ret;
}


static int8_t als_ch0ch1raw_calc_readback (uint16_t *retVal1, uint16_t *retVal2, uint16_t *retVal3, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[11];
	uint16_t value1, value2, value3;

	buffer[0] = LTR554_ALS_DATA_CH1_0;
	ret = I2C_Read(buffer, 4);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value1 = ((int)buffer[2]) + ((int)buffer[3] << 8); // CH0
	value2 = ((int)buffer[0]) + ((int)buffer[1] << 8); // CH1

	value3 = ratioHysterisis(value1, value2);

	*retVal1 = value1;
	*retVal2 = value2;
	*retVal3 = value3;

	return ret;
}

static int als_get_light_luminance(struct ltr554_data *ltr554, uint16_t *luminance)
{
	int ret;
	uint8_t rdback_val = 0;

	ret = als_contr_readback(ALS_CONTR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: light_luminance_show Fail...\n", __func__);
		return (-1);
	}

	als_contr_setup(rdback_val|ALS_MODE_ACTIVE, ltr554);
	msleep(20);

	*luminance = read_als_adc_value(ltr554);
	als_contr_setup(rdback_val, ltr554);
	return ret;
}
#if (ALS_WORK_MODE == ALS_WORK_INTERRUPT_MODE)
static int8_t als_interrupt_mode_setup (uint8_t interr_mode_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	
	ret = _ltr554_set_bit(ltr554->i2c_client, interr_mode_val, LTR554_INTERRUPT, INT_MODE_ALS_TRIG);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: als interrupt mode setup Fail...\n", __func__);
		return ret;
	}

	return ret;
}
#endif
static int8_t ps_interrupt_mode_setup (uint8_t interr_mode_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;

	ret = _ltr554_set_bit(ltr554->i2c_client, interr_mode_val, LTR554_INTERRUPT, INT_MODE_PS_TRIG);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: als interrupt mode setup Fail...\n", __func__);
		return ret;
	}

	return ret;
}

static int8_t interrupt_polarity_setup (uint8_t interr_polar_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_INTERRUPT;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value &= 0xFB;

	if (interr_polar_val == 0) {
		value |= INT_POLAR_ACT_LO;
	} else if (interr_polar_val == 1) {
		value |= INT_POLAR_ACT_HI;
	}

	buffer[0] = LTR554_INTERRUPT;
	buffer[1] = value;
	dev_dbg(&ltr554->i2c_client->dev, "%s: interrupt_polarity_setup value = 0x%x\n", __func__,value);
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s Interrupt polarity setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t interrupt_setup(uint8_t interrupt_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3];

	buffer[0] = LTR554_INTERRUPT;

	/* Default settings used for now. */
	buffer[1] = interrupt_val;
	buffer[1] &= 0x07;
	dev_dbg(&ltr554->i2c_client->dev, "%s: interrupt_setup value = 0x%x\n", __func__,buffer[1]);
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s |Interrupt (0x%02X) setup fail...", __func__, buffer[0]);
	}

	return ret;
}


static int8_t interrupt_readback (uint8_t rdbck_type, uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_INTERRUPT;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
    dev_dbg(&ltr554->i2c_client->dev, "%s: interrupt_readback value = 0x%x\n", __func__,value);
	if (rdbck_type == INT_MODE_RDBCK) {
		*retVal = (value & 0x03);
	} else if (rdbck_type == INT_POLAR_RDBCK) {
		*retVal = (value & 0x04) >> 2;
	} else if (rdbck_type == INT_INTERRUPT_RDBCK) {
		*retVal = (value & 0x07);
	}

	return ret;
}


static int8_t ps_offset_setup (uint16_t ps_offset_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[3];

	buffer[0] = LTR554_PS_OFFSET_1;
	buffer[1] = (ps_offset_val >> 8) & 0x03;
	buffer[2] = (ps_offset_val & 0xFF);

	ret = I2C_Write(buffer, 3);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s PS offset setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t ps_offset_readback (uint16_t *offsetval, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2];
	uint16_t value;

	buffer[0] = LTR554_PS_OFFSET_1;
	ret = I2C_Read(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];
	value <<= 8;
	value += buffer[1];

	*offsetval = value;

	return ret;
}


static int8_t interrupt_persist_setup (uint8_t interr_persist_val, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	value = interr_persist_val;

	buffer[0] = LTR554_INTERRUPT_PRST;
	buffer[1] = value;
	dev_dbg(&ltr554->i2c_client->dev, "%s: interrupt_persist_setup value = 0x%x\n", __func__,value);
	ret = I2C_Write(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s Interrupt persist setup fail...\n", __func__);
		return ret;
	}

	return ret;
}


static int8_t interrupt_prst_readback (uint8_t *retVal, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[2], value;

	buffer[0] = LTR554_INTERRUPT_PRST;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value = buffer[0];

	*retVal = value;

	return ret;
}


/* Set ALS range */
static int8_t set_als_range(uint16_t lt, uint16_t ht, uint8_t lo_hi)
{
	int8_t ret;
	uint8_t buffer[5], num_data = 0;

	if (lo_hi == LO_LIMIT) {
		buffer[0] = LTR554_ALS_THRES_LOW_0;
		buffer[1] = lt & 0xFF;
		buffer[2] = (lt >> 8) & 0xFF;
		num_data = 3;
	} else if (lo_hi == HI_LIMIT) {
		buffer[0] = LTR554_ALS_THRES_UP_0;
		buffer[1] = ht & 0xFF;
		buffer[2] = (ht >> 8) & 0xFF;
		num_data = 3;
	} else if (lo_hi == LO_N_HI_LIMIT) {
		buffer[0] = LTR554_ALS_THRES_UP_0;
		buffer[1] = ht & 0xFF;
		buffer[2] = (ht >> 8) & 0xFF;
		buffer[3] = lt & 0xFF;
		buffer[4] = (lt >> 8) & 0xFF;
		num_data = 5;
	}

	ret = I2C_Write(buffer, num_data);
	if (ret <0) {
		pr_alert("%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}
	dev_dbg(&sensor_info->i2c_client->dev, "%s Set als range:0x%04x"
	                                       " - 0x%04x\n", __func__, lt, ht);

	return ret;
}


static int8_t als_range_readback (uint16_t *lt, uint16_t *ht, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[5];
	uint16_t value_lo, value_hi;

	buffer[0] = LTR554_ALS_THRES_UP_0;
	ret = I2C_Read(buffer, 4);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}

	value_lo = buffer[3];
	value_lo <<= 8;
	value_lo += buffer[2];
	*lt = value_lo;

	value_hi = buffer[1];
	value_hi <<= 8;
	value_hi += buffer[0];
	*ht = value_hi;

	return ret;
}


/* Set PS range */
static int8_t set_ps_range(uint16_t lt, uint16_t ht, uint8_t lo_hi, struct ltr554_data *ltr554)
{
	int8_t ret;
	uint8_t buffer[5], num_data = 0;

	//mutex_lock(&ltr554->bus_lock);

	if (lo_hi == LO_LIMIT) {
		buffer[0] = LTR554_PS_THRES_LOW_0;
		buffer[1] = lt & 0xFF;
		buffer[2] = (lt >> 8) & 0x07;
		num_data = 3;
	} else if (lo_hi == HI_LIMIT) {
		buffer[0] = LTR554_PS_THRES_UP_0;
		buffer[1] = ht & 0xFF;
		buffer[2] = (ht >> 8) & 0x07;
		num_data = 3;
	} else if (lo_hi == LO_N_HI_LIMIT) {
		buffer[0] = LTR554_PS_THRES_UP_0;
		buffer[1] = ht & 0xFF;
		buffer[2] = (ht >> 8) & 0x07;
		buffer[3] = lt & 0xFF;
		buffer[4] = (lt >> 8) & 0x07;
		num_data = 5;
	}

	ret = I2C_Write(buffer, num_data);
	if (ret <0) {
		pr_alert("%s | 0x%02X", __func__, buffer[0]);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s Set ps range:0x%04x"
	                                       " - 0x%04x\n", __func__, lt, ht);

	//mutex_unlock(&ltr554->bus_lock);

	return ret;
}


static int8_t ps_range_readback (uint16_t *lt, uint16_t *ht, struct ltr554_data *ltr554)
{
	int8_t ret = 0;
	uint8_t buffer[5];
	uint16_t value_lo, value_hi;

	//mutex_lock(&ltr554->bus_lock);

	buffer[0] = LTR554_PS_THRES_UP_0;
	ret = I2C_Read(buffer, 4);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);

		//mutex_unlock(&ltr554->bus_lock);

		return ret;
	}

	value_lo = buffer[3];
	value_lo <<= 8;
	value_lo += buffer[2];
	*lt = value_lo;

	value_hi = buffer[1];
	value_hi <<= 8;
	value_hi += buffer[0];
	*ht = value_hi;

	//mutex_unlock(&ltr554->bus_lock);

	return ret;
}




static uint16_t findCT_Avg (uint16_t *ps_val)
{
	uint8_t i_ctr, min_Index, max_Index;
	uint16_t temp = 0, max_val, min_val;
	//struct ltr554_data *ltr554 = sensor_info;

	//mutex_lock(&ltr554->bus_lock);

	//findMinMaxVal_Index(ps_val, &min_val, &min_Index, &max_val, &max_Index);
	max_val = ps_val[3];
	max_Index = 3;
	min_val = ps_val[3];
	min_Index = 3;

	for (i_ctr = 3; i_ctr < 8; i_ctr++) {
		if (ps_val[i_ctr] > max_val) {
			max_val = ps_val[i_ctr];
			max_Index = i_ctr;
		}
	}

	for (i_ctr = 3; i_ctr < 8; i_ctr++) {
		if (ps_val[i_ctr] < min_val) {
			min_val = ps_val[i_ctr];
			min_Index = i_ctr;
		}
	}

	if (min_val == max_val) {
		// all values are the same
		for (i_ctr = 3; i_ctr < 6; i_ctr++) {
			temp += ps_val[i_ctr];
		}
	} else {
		for (i_ctr = 3; i_ctr < 8; i_ctr++) {
			if ((i_ctr != min_Index) && (i_ctr != max_Index)) {
				temp += ps_val[i_ctr];
			}
		}
	}

	temp = (temp / 3);

	//mutex_unlock(&ltr554->bus_lock);

	return temp;
}


// take note ------------------------------------------
// This function should be called in the function which is called when the CALL button is pressed.
// take note ------------------------------------------
static void setThrDuringCall (void)
{
	int8_t ret;
	struct ltr554_data *ltr554 = sensor_info;

	// set ps measurement rate to 10ms
	ret = ps_meas_rate_setup(10, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS MeasRate Setup Fail...\n", __func__);
	}

	ps_grabData_stage = 0;
	ps_kept_data_counter = 0;

	ret = set_ps_range(PS_MAX_MEASURE_VAL, PS_MIN_MEASURE_VAL, LO_N_HI_LIMIT, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s : PS thresholds setting Fail...\n", __func__);
	}

	ret = ps_contr_setup(0x03, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS Enable Fail...\n", __func__);
	}
}


/* Report ALS input event */
static void report_als_input_event(struct ltr554_data *ltr554)
{

	uint32_t adc_value;

	adc_value = ((read_als_adc_value (ltr554)) * ltr554->als_fac) / 100;
	if (adc_value > MAX_VAL) {
		adc_value = MAX_VAL;
	}
	input_report_rel(ltr554->als_input_dev, REL_MISC, adc_value);
	input_sync(ltr554->als_input_dev);
	dev_dbg(&ltr554->i2c_client->dev,"%s : %u\n", __func__, adc_value);
}

static int set_als_poll_delay(struct ltr554_data *ltr554, unsigned int val)
{
	int ret;
	uint8_t param;

	dev_err(&ltr554->i2c_client->dev,"%s : %d\n", __func__, val);

	if ((val != LTR554_ALS_POLL_SLOW) && (val != LTR554_ALS_POLL_MEDIUM) && (val != LTR554_ALS_POLL_FAST)) {
		dev_err(&ltr554->i2c_client->dev,"%s:invalid value=%d\n", __func__, val);
		return -1;
	}

	if (val == LTR554_ALS_POLL_FAST) {
		ltr554->als_poll_delay = 50;    // 50ms
		param = 0x8;
	}
	else if (val == LTR554_ALS_POLL_MEDIUM) {
		ltr554->als_poll_delay = 100;   // 100ms
		param = 0x8;
	}
	else {
		ltr554->als_poll_delay = 1000;    // 1000ms
		param = 0x3;
	}

	ret = als_meas_rate_reg_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: LTR554_Meas_Rate register Setup Fail...\n", __func__);
		return ret;
	}

	/*
	* If work is already scheduled then subsequent schedules will not
	* change the scheduled time that's why we have to cancel it first.
	*/
	cancel_delayed_work(&ltr554->als_dwork);
	queue_delayed_work(ltr554_workqueue, &ltr554->als_dwork, msecs_to_jiffies(ltr554->als_poll_delay));

	return 0;

}

/* ALS polling routine */
static void als_polling_work_handler(struct work_struct *work)
{
	struct ltr554_data *ltr554 = sensor_info;
	ltr554_wait_for_device_resume(ltr554);
	if(ltr554->als_suspend_enable_flag == 1){
		report_als_input_event(ltr554);
		queue_delayed_work(ltr554_workqueue, &ltr554->als_dwork, msecs_to_jiffies(ltr554->als_poll_delay));	// restart timer
	}

}

static void ltr554_ps_als_int_work_handler(struct work_struct *work)
{
	int8_t ret;
	uint8_t status;
	uint8_t	interrupt_stat, newdata;
	struct ltr554_data *ltr554 = sensor_info;
	uint8_t buffer[2];
	uint16_t adc_value;
	uint16_t thresh_hi, thresh_lo;

	dev_info(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler enter \n\n");
	ltr554_wait_for_device_resume(ltr554);
	buffer[0] = LTR554_ALS_PS_STATUS;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return;
	}
	adc_value = findCT_Avg(ps_init_kept_data);
	status = buffer[0];
	interrupt_stat = status & 0x0A;
	newdata = status & 0x05;
    dev_dbg(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler interrupt_stat = 0x%x, newdata = 0x%x\n\n",interrupt_stat,newdata);
	// PS interrupt and PS with new data
	if ((interrupt_stat & 0x02) && (newdata & 0x01)) {
		ltr554->ps_irq_flag = 1;

    	adc_value = read_ps_adc_value (ltr554);
    	dev_dbg(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler adc_value = 0x%x\n",adc_value);
    	ret = ps_range_readback(&thresh_lo, &thresh_hi, ltr554);
    	if (ret < 0) {
    		dev_err(&ltr554->i2c_client->dev, "%s: PS threshold range readback Fail...\n", __func__);
    	}
    	/*Force first interrupt FAR event to notice HAL */
		if ((thresh_lo == PS_MAX_MEASURE_VAL)&& (thresh_hi == PS_MIN_MEASURE_VAL )) {
			thresh_lo = PS_MIN_MEASURE_VAL;
			thresh_hi = ltr554->default_ps_highthresh;
			dev_info(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler FAR_VAL force \n");
			input_report_abs(ltr554->ps_input_dev, ABS_DISTANCE, FAR_VAL);
			input_sync(ltr554->ps_input_dev);

		}
    	/* Adjust measurement range using a crude filter to prevent interrupt jitter */
		/* FTN */
		else if (adc_value > ltr554->default_ps_highthresh) {
			if ((thresh_lo == PS_MIN_MEASURE_VAL) && (thresh_hi == ltr554->default_ps_highthresh)) {
				thresh_lo = ltr554->default_ps_lowthresh;
				thresh_hi = PS_MAX_MEASURE_VAL;
			}
			dev_info(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler NEAR_VAL \n");
			input_report_abs(ltr554->ps_input_dev, ABS_DISTANCE, NEAR_VAL);
			input_sync(ltr554->ps_input_dev);
		}
		/* FTN */

		/* NTF */
		else if (adc_value < ltr554->default_ps_lowthresh) {
			if ((thresh_lo == ltr554->default_ps_lowthresh) && (thresh_hi == PS_MAX_MEASURE_VAL)) {
				//thresh_lo = 0;
				thresh_lo = PS_MIN_MEASURE_VAL;
				thresh_hi = ltr554->default_ps_highthresh;
			}
			dev_info(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler FAR_VAL \n");
			input_report_abs(ltr554->ps_input_dev, ABS_DISTANCE, FAR_VAL);
			input_sync(ltr554->ps_input_dev);
		}
		/* NTF */

		/*High threshold > ps value > low threshold, invalid interrupt ,noise */
		else {
			dev_err(&ltr554->i2c_client->dev,"%s : invalid interrupt adc_value = %d\n", __func__, adc_value);
			if (thresh_lo == PS_MIN_MEASURE_VAL){
				thresh_hi = ltr554->default_ps_highthresh;
			}
			else if (thresh_hi == PS_MAX_MEASURE_VAL){
				thresh_lo = ltr554->default_ps_lowthresh;
			}
			else {
				thresh_lo = PS_MAX_MEASURE_VAL;
				thresh_hi = PS_MIN_MEASURE_VAL;
			}
		}
		ret = set_ps_range((uint16_t)thresh_lo, (uint16_t)thresh_hi, LO_N_HI_LIMIT, ltr554);
		if (ret < 0) {
			dev_err(&ltr554->i2c_client->dev, "%s : PS thresholds setting Fail...\n", __func__);
		}

		/*dummy read just to clear the PS interrupt bit */
		adc_value = read_ps_adc_value (ltr554);
		ltr554->ps_irq_flag = 0;
	}

	/* ALS interrupt and ALS with new data*/
	if ((interrupt_stat & 0x08) && (newdata & 0x04)) {
		ltr554->als_irq_flag = 1;
		report_als_input_event(ltr554);
		dev_dbg(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler als_ch0_val = %d\n",als_ch0_val);

		ltr554->als_lowthresh = (als_ch0_val * (100 - LTR554_ALS_THRESHOLD_HSYTERESIS) ) /100;
		ltr554->als_highthresh = (als_ch0_val * (100 + LTR554_ALS_THRESHOLD_HSYTERESIS) ) /100;
		dev_dbg(&ltr554->i2c_client->dev,"als_lowthresh  = %d\n",ltr554->als_lowthresh);
		dev_dbg(&ltr554->i2c_client->dev,"als_highthresh  = %d\n",ltr554->als_highthresh);
		ret = set_als_range(ltr554->als_lowthresh, ltr554->als_highthresh, LO_N_HI_LIMIT);
		if (ret < 0) {
			dev_err(&ltr554->i2c_client->dev, "%s : ALS Thresholds Write Fail...\n", __func__);
			return;
		}
		ltr554->als_irq_flag = 0;
	}

	enable_irq(ltr554->irq);
	dev_dbg(&ltr554->i2c_client->dev,"ltr554_ps_als_int_work_handler exit \n\n");

}
static void ltr554_reschedule_work(struct ltr554_data *ltr554,
    			unsigned long delay)
{

	if (unlikely(atomic_read(&ltr554->ps_selftest_ongoing) == 1)){

	    atomic_set(&ltr554->ps_selftest_int, 1);
	    wake_up_interruptible(&ltr554->ps_selftest_wq);
	    return;
	}

	queue_delayed_work(ltr554_workqueue, &ltr554->dwork, delay);

}

/* IRQ Handler */
static irqreturn_t ltr554_irq_handler(int irq, void *i2c_client)
{
	struct ltr554_data *ltr554 = i2c_get_clientdata(i2c_client);
    dev_dbg(&ltr554->i2c_client->dev,"ltr554_irq_handler enter \n\n");
	/* disable an irq without waiting */
	disable_irq_nosync(ltr554->irq);

	//schedule_work(&irq_workqueue);
    ltr554_reschedule_work(ltr554,0);
	return IRQ_HANDLED;
}



static int ltr554_gpio_irq(struct ltr554_data *ltr554)
{
	int rc = 0;

    int irq_no = of_get_named_gpio(ltr554->i2c_client->dev.of_node, "ltr554,irq-gpio", 0);
    if (!gpio_is_valid(irq_no)) {
        dev_err(&ltr554->i2c_client->dev,"Interrupt GPIO is not specified\n");
		return rc;
    }
    rc = gpio_request(irq_no, "ltr554_irq");
    if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev,"Request gpio failed, rc = %d\n",rc);
		return rc;
    }
    rc = gpio_tlmm_config(GPIO_CFG(
			irq_no, 0,
			GPIO_CFG_INPUT,
			GPIO_CFG_PULL_UP,
			GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev,"Unable to config tlmm = %d\n", rc);
		gpio_free(irq_no);
		return rc;
	}
    rc = gpio_direction_input(irq_no);
    if (rc < 0) {
        dev_err(&ltr554->i2c_client->dev,"Set direction for irq failed, rc = %d\n", rc);
        gpio_free(irq_no);
        return rc;
    }
    ltr554->irq = gpio_to_irq(irq_no);
	if (ltr554->irq <= 0) {
		dev_err(&ltr554->i2c_client->dev, "no IRQ\n");
		return -1;
	}
    dev_info(&ltr554->i2c_client->dev,"IRQ Number is %d\n", ltr554->irq);
	/* Configure an active low trigger interrupt for the device */
	//rc = request_irq(ltr554->irq, ltr554_irq_handler, IRQF_TRIGGER_FALLING, DEVICE_NAME, ltr554);
    if (request_irq(ltr554->irq, ltr554_irq_handler, IRQF_TRIGGER_LOW,DEVICE_NAME, (void *)ltr554->i2c_client) < 0 )
    {
    	rc = -1;
    	gpio_free(irq_no);
		dev_err(&ltr554->i2c_client->dev, "%s: Request IRQ (%d) for GPIO %d Fail (%d)\n", __func__, ltr554->irq,
		ltr554->gpio_int_no, rc);
        goto out1;
    }

	/* Allows this interrupt to wake the system */
	rc = irq_set_irq_wake(ltr554->irq, 1);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: IRQ-%d WakeUp Enable Fail...\n", __func__, ltr554->irq);
		goto out1;
	}

out1:

	return rc;

}
//(Linux RTOS)<


/* PS Enable */
static int8_t ps_enable_init(struct ltr554_data *ltr554)
{
	int8_t rc = 0;
	uint8_t buffer[1]; // for dummy read

	setThrDuringCall();

	if (ltr554->ps_enable_flag) {
		dev_dbg(&ltr554->i2c_client->dev, "%s: already enabled\n", __func__);
		return 0;
	}

	rc = ps_led_setup(0x7F, ltr554);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED Setup Fail...\n", __func__);
		return rc;
	}

	rc = ps_ledPulseCount_setup(0x08, ltr554);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED pulse count setup Fail...\n", __func__);
	}

	rc = ps_meas_rate_setup(10, ltr554);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS MeasRate Setup Fail...\n", __func__);
		return rc;
	}

	rc = ps_contr_setup(0x00, ltr554);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS Disable Fail...\n", __func__);
		return rc;
	}

	// dummy read
	buffer[0] = LTR554_PS_CONTR;
	I2C_Read(buffer, 1);
	// dumy read

	ltr554->ps_enable_flag = 1;

	return rc;
}


/* PS Disable */
static int8_t ps_disable(struct ltr554_data *ltr554)
{
	int8_t rc = 0;

	if (ltr554->ps_enable_flag == 0) {
		dev_info(&ltr554->i2c_client->dev, "%s: already disabled\n", __func__);
		return 0;
	}

	/* Don't allow this interrupt to wake the system anymore */
	rc = irq_set_irq_wake(ltr554->irq, 0);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: IRQ-%d WakeUp Disable Fail...\n", __func__, ltr554->irq);
		return rc;
	}

	rc = _ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_INTERRUPT, INT_MODE_PS_TRIG);
	dev_info(&ltr554->i2c_client->dev, "%s: ps_disable value = \n", __func__);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS Disable Fail...\n", __func__);
		return rc;
	}

	ltr554->ps_enable_flag = 0;

	return rc;
}


/* PS open fops */
ssize_t ps_open(struct inode *inode, struct file *file)
{
	struct ltr554_data *ltr554 = sensor_info;

	if (ltr554->ps_opened)
		return -EBUSY;

	ltr554->ps_opened = 1;

	return 0;
}


/* PS release fops */
ssize_t ps_release(struct inode *inode, struct file *file)
{
	struct ltr554_data *ltr554 = sensor_info;

	ltr554->ps_opened = 0;

	return ps_disable(ltr554);
}


/* PS IOCTL */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int ps_ioctl (struct inode *ino, struct file *file, unsigned int cmd, unsigned long arg)
#else
static long ps_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	int rc = 0, val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	pr_info("%s cmd %d\n", __func__, _IOC_NR(cmd));

	switch (cmd) {
		case LTR554_IOCTL_PS_ENABLE:
			if (get_user(val, (unsigned long __user *)arg)) {
				rc = -EFAULT;
				break;
			}
			rc = val ? ps_enable_init(ltr554) : ps_disable(ltr554);

			break;
		case LTR554_IOCTL_PS_GET_ENABLED:
			rc = put_user(ltr554->ps_enable_flag, (unsigned long __user *)arg);

			break;
		default:
			pr_err("%s: INVALID COMMAND %d\n", __func__, _IOC_NR(cmd));
			rc = -EINVAL;
	}

	return rc;
}

static const struct file_operations ps_fops = {
	.owner = THIS_MODULE,
	.open = ps_open,
	.release = ps_release,
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
	.ioctl = ps_ioctl
	#else
	.unlocked_ioctl = ps_ioctl
	#endif
};

struct miscdevice ps_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ltr554_ps",
	.fops = &ps_fops
};


static int8_t als_enable_init(struct ltr554_data *ltr554)
{
	int8_t rc = 0;
	uint8_t buffer[1]; // for dummy read

	/* if device not enabled, enable it */
	if (ltr554->als_enable_flag) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS already enabled...\n", __func__);
		return rc;
	}

	rc = als_meas_rate_reg_setup(0x03, ltr554);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_Meas_Rate register Setup Fail...\n", __func__);
		return rc;
	}

	/* Set minimummax thresholds where interrupt will *not* be generated */
#if ACT_INTERRUPT
	rc = set_als_range(ALS_MIN_MEASURE_VAL, ALS_MIN_MEASURE_VAL, LO_N_HI_LIMIT);
#else
	rc = set_als_range(ALS_MIN_MEASURE_VAL, ALS_MAX_MEASURE_VAL, LO_N_HI_LIMIT);
#endif
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s : ALS Thresholds Write Fail...\n", __func__);
		return rc;
	}

	rc = als_contr_setup(0x18, ltr554);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS Disable Fail...\n", __func__);
		return rc;
	}

	// dummy read
	buffer[0] = LTR554_ALS_CONTR;
	I2C_Read(buffer, 1);
	// dumy read
	ltr554->als_fac = 100;
	ltr554->als_enable_flag = 1;

	return rc;
}


static int8_t als_disable(struct ltr554_data *ltr554)
{
	int8_t rc = 0;

	if (ltr554->als_enable_flag == 0) {
		dev_err(&ltr554->i2c_client->dev, "%s : ALS already disabled...\n", __func__);
		return rc;
	}

	rc = _ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_ALS_CONTR, INT_MODE_ALS_TRIG);
	if (rc < 0) {
		dev_err(&ltr554->i2c_client->dev,"%s: ALS Disable Fail...\n", __func__);
		return rc;
	}
	ltr554->als_enable_flag = 0;

	return rc;
}


ssize_t als_open(struct inode *inode, struct file *file)
{
	struct ltr554_data *ltr554 = sensor_info;
	int8_t rc = 0;

	if (ltr554->als_opened) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS already Opened...\n", __func__);
		rc = -EBUSY;
	}
	ltr554->als_opened = 1;

	return rc;
}


ssize_t als_release(struct inode *inode, struct file *file)
{
	struct ltr554_data *ltr554 = sensor_info;

	ltr554->als_opened = 0;

	//return 0;
	return als_disable(ltr554);
}

static int8_t _check_part_id(struct ltr554_data *ltr554)
{


	int8_t ret;
	uint8_t buffer[2];



	buffer[0] = LTR554_MANUFACTURER_ID;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return -1;
	}
	pr_info("0x%x: MANUFACTURER_ID \n", buffer[0]);

	buffer[0] = LTR554_PART_ID;
	ret = I2C_Read(buffer, 1);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Read failure :0x%02X",
		        __func__, buffer[0]);
		return -1;
	}
	pr_info("0x%x: _check_part_id \n", buffer[0]);
	if ((buffer[0] != PARTID) && (buffer[0] != PARTID_V5)) {
		dev_err(&ltr554->i2c_client->dev, "%s: Part failure miscompare"
		        " act:0x%02x exp:0x%02x\n", __func__, buffer[0], PARTID);
		return -2;
	}

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int als_ioctl (struct inode *ino, struct file *file, unsigned int cmd, unsigned long arg)
#else
static long als_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	int rc = 0, val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	pr_debug("%s cmd %d\n", __func__, _IOC_NR(cmd));

	switch (cmd) {
		case LTR554_IOCTL_ALS_ENABLE:
			if (get_user(val, (unsigned long __user *)arg)) {
				rc = -EFAULT;
				break;
			}
			/*pr_info("%s value = %d\n", __func__, val);*/
			rc = val ? als_enable_init(ltr554) : als_disable(ltr554);

			break;
		case LTR554_IOCTL_ALS_GET_ENABLED:
			val = ltr554->als_enable_flag;
			/*pr_info("%s enabled %d\n", __func__, val);*/
			rc = put_user(val, (unsigned long __user *)arg);

			break;
		default:
			pr_err("%s: INVALID COMMAND %d\n", __func__, _IOC_NR(cmd));
			rc = -EINVAL;
	}

	return rc;
}


static const struct file_operations als_fops = {
	.owner = THIS_MODULE,
	.open = als_open,
	.release = als_release,
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
	.ioctl = als_ioctl
	#else
	.unlocked_ioctl = als_ioctl
	#endif
};

static struct miscdevice als_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ltr554_ls",
	.fops = &als_fops
};


static ssize_t als_adc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t value;
	int ret;
	struct ltr554_data *ltr554 = sensor_info;

	value = read_als_adc_value(ltr554);
	ret = sprintf(buf, "%d\n", value);

	return ret;
}

static DEVICE_ATTR(als_adc, 0666, als_adc_show, NULL);

static ssize_t light_luminance_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint32_t value;
	int ret;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ltr554_wait_for_device_resume(ltr554);
	read_ps_als_nvitem(ltr554);
	ret = als_contr_readback(ALS_CONTR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: light_luminance_show Fail...\n", __func__);
		return (-1);
	}

	als_contr_setup(rdback_val|ALS_MODE_ACTIVE, ltr554);
	msleep(200);

	value = ((read_als_adc_value (ltr554)) * ltr554->als_fac) / 100;
	als_contr_setup(rdback_val, ltr554);
	ret = sprintf(buf, "%d\n", value);

	return ret;
}

static DEVICE_ATTR(light_luminance_data, 0666, light_luminance_show, NULL);

static ssize_t ps_adc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t value;
	int ret;
	struct ltr554_data *ltr554 = sensor_info;

	//ltr554->mode = PS;
	//value = read_adc_value(ltr554);
	value = read_ps_adc_value(ltr554);
	//ret = sprintf(buf, "%d\n", value);
	ret = sprintf(buf, "%d\n", value);

	return ret;
}

static DEVICE_ATTR(ps_adc, 0666, ps_adc_show, NULL);

static ssize_t ps_rawdata_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t value;
	int ret;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;
	ltr554_wait_for_device_resume(ltr554);
	ret = ps_contr_readback(PS_CONTR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ps_rawdata_show Fail...\n", __func__);
		return (-1);
	}

	ps_contr_setup(rdback_val|PS_MODE_ACTIVE, ltr554);
	msleep(20);

	value = read_ps_adc_value(ltr554);

	ps_contr_setup(rdback_val, ltr554);

	ret = sprintf(buf, "%d\n", value);

	return ret;
}

static DEVICE_ATTR(proximity_raw_data, 0666, ps_rawdata_show, NULL);

static ssize_t psadcsaturationBit_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t value;
	uint8_t saturation_bit;
	int ret;
	uint8_t buffer[3];
	struct ltr554_data *ltr554 = sensor_info;

	buffer[0] = LTR554_PS_DATA_0;
	ret = I2C_Read(buffer, 2);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s | 0x%02X", __func__, buffer[0]);
		return ret;
	}
	value = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
	//ltr554->mode = PS_W_SATURATION_BIT;
	//value = read_adc_value(ltr554);
	saturation_bit = (value >> 15);
	value &= PS_VALID_MEASURE_MASK;
	ret = sprintf(buf, "%d %d\n", value, saturation_bit);

	return ret;
}

static DEVICE_ATTR(psadcsaturationBit, 0666, psadcsaturationBit_show, NULL);


static ssize_t ltr554help_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct ltr554_data *ltr554 = sensor_info;
    // address 0x 8D 8E
	dev_info(&ltr554->i2c_client->dev,"To show ALS value : cat als_adc\n");
	dev_info(&ltr554->i2c_client->dev,"To show PS value : cat ps_adc\n");
	dev_info(&ltr554->i2c_client->dev,"To show PS value with saturation bit : cat psadcsaturationBit\n\n");

	// address 0x80
	dev_info(&ltr554->i2c_client->dev,"Address 0x80 (ALS_CONTR)\n");
	dev_info(&ltr554->i2c_client->dev,"ALS active mode : echo 1 > alsmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS standby mode : echo 0 > alsmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS mode : cat alsmodesetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"ALS SW reset : echo 1 > alsswresetsetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS SW not reset : echo 0 > alsswresetsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS SW reset bit : cat alsswresetsetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"ALS gain 1x : echo 1 > alsgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS gain 2x : echo 2 > alsgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS gain 4x : echo 4 > alsgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS gain 8x : echo 8 > alsgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS gain 48x : echo 48 > alsgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS gain 96x : echo 96 > alsgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS gain : cat alsgainsetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"Write value to ALS_CONTR register (0x80) : echo [hexcode value] > alscontrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x0B : echo B > alscontrsetup or echo b > alscontrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x13 : echo 13 > alscontrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register ALS_CONTR (0x80) : cat alscontrsetup\n\n");
	// address 0x80

	// address 0x81
	dev_info(&ltr554->i2c_client->dev,"Address 0x81 (PS_CONTR)\n");
	dev_info(&ltr554->i2c_client->dev,"PS active mode : echo 1 > psmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS standby mode : echo 0 > psmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read PS mode : cat psmodesetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"PS gain x16 : echo 16 > psgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS gain x32 : echo 32 > psgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS gain x64 : echo 64 > psgainsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read PS gain : cat psgainsetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"PS saturation indicator enable : echo 1 > pssatuindicasetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS saturation indicator disable : echo 0 > pssatuindicasetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read back PS saturation indicator : cat pssatuindicasetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"Write value to PS_CONTR register (0x81) : echo [hexcode value] > pscontrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x0B : echo B > pscontrsetup or echo b > pscontrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x13 : echo 13 > pscontrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register PS_CONTR (0x81) : cat pscontrsetup\n\n");
	// address 0x81

	// address 0x82
	dev_info(&ltr554->i2c_client->dev,"Address 0x82 (PS_LED)\n");
	dev_info(&ltr554->i2c_client->dev,"LED current 5mA : echo 5 > psledcurrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED current 10mA : echo 10 > psledcurrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED current 20mA : echo 20 > psledcurrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED current 50mA : echo 50 > psledcurrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED current 100mA : echo 100 > psledcurrsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read LED current : cat psledcurrsetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"LED current duty 25%% : echo 25 > psledcurrduty\n");
	dev_info(&ltr554->i2c_client->dev,"LED current duty 50%% : echo 50 > psledcurrduty\n");
	dev_info(&ltr554->i2c_client->dev,"LED current duty 75%% : echo 75 > psledcurrduty\n");
	dev_info(&ltr554->i2c_client->dev,"LED current duty 100%% : echo 100 > psledcurrduty\n");
	dev_info(&ltr554->i2c_client->dev,"To read LED current duty : cat psledcurrduty\n\n");

	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 30kHz : echo 30 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 40kHz : echo 40 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 50kHz : echo 50 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 60kHz : echo 60 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 70kHz : echo 70 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 80kHz : echo 80 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 90kHz : echo 90 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"LED pulse freq 100kHz : echo 100 > psledpulsefreqsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read LED pulse freq : cat psledpulsefreqsetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"Write value to PS_LED register (0x82) : echo [hexcode value] > psledsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x0B : echo B > psledsetup or echo b > psledsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x13 : echo 13 > psledsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register PS_LED (0x82) : cat psledsetup\n\n");
	// address 0x82

	// address 0x83
	dev_info(&ltr554->i2c_client->dev,"Address 0x83 (PS_N_PULSES)\n");
	dev_info(&ltr554->i2c_client->dev,"To set PS LED pulse count (0x83) : echo [pulse count num] > psledpulsecountsetup\n");
	dev_info(&ltr554->i2c_client->dev,"[pulse count num] must be 0 to 15, inclusive\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to set 0 count : echo 0 > psledpulsecountsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to set 13 counts : echo 13 > psledpulsecountsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register PS_N_PULSES (0x83) : cat psledpulsecountsetup\n\n");
	// address 0x83

	// address 0x84
	dev_info(&ltr554->i2c_client->dev,"Address 0x84 (PS_MEAS_RATE)\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 50ms : echo 50 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 70ms : echo 70 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 100ms : echo 100 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 200ms : echo 200 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 500ms : echo 500 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 1000ms : echo 1000 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 2000ms : echo 2000 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"PS meas repeat rate 10ms : echo 10 > psmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register PS_MEAS_RATE (0x84) : cat psmeasratesetup\n\n");
	// address 0x84

	// address 0x85
	dev_info(&ltr554->i2c_client->dev,"Address 0x85 (ALS_MEAS_RATE)\n");
	dev_info(&ltr554->i2c_client->dev,"ALS meas repeat rate 50ms : echo 50 > alsmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS meas repeat rate 100ms : echo 100 > alsmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS meas repeat rate 200ms : echo 200 > alsmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS meas repeat rate 500ms : echo 500 > alsmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS meas repeat rate 1000ms : echo 1000 > alsmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS meas repeat rate 2000ms : echo 2000 > alsmeasratesetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS meas repeat rate : cat alsmeasratesetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"ALS integration time 100ms : echo 100 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS integration time 50ms : echo 50 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS integration time 200ms : echo 200 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS integration time 400ms : echo 400 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS integration time 150ms : echo 150 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS integration time 250ms : echo 250 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS integration time 300ms : echo 300 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"ALS integration time 350ms : echo 350 > alsintegtimesetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS integration time : cat alsintegtimesetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"Write value to ALS_MEAS register (0x85) : echo [hexcode value] > alsmeasrateregsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x0B : echo B > alsmeasrateregsetup or echo b > alsmeasrateregsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x13 : echo 13 > alsmeasrateregsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register ALS_MEAS (0x85) : cat alsmeasrateregsetup\n\n");
	// address 0x85

	// address 0x86
	dev_info(&ltr554->i2c_client->dev,"To read part ID : cat partid\n");
	dev_info(&ltr554->i2c_client->dev,"To read revision ID : cat revid\n");
	dev_info(&ltr554->i2c_client->dev,"To read PART_ID register (0x86) : cat partidreg\n\n");
	// address 0x86

	// address 0x87
	dev_info(&ltr554->i2c_client->dev,"To read manufacturing ID : cat manuid\n\n");
	// address 0x87

	// address 0x8C
	dev_info(&ltr554->i2c_client->dev,"Address 0x8C (ALS_PS_STATUS)\n");
	dev_info(&ltr554->i2c_client->dev,"To read PS data status : cat psdatastatus\n");
	dev_info(&ltr554->i2c_client->dev,"To read PS interrupt status : cat psinterruptstatus\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS data status : cat alsdatastatus\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS interrupt status : cat alsinterruptstatus\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS gain status : cat alsgainstatus\n");
	dev_info(&ltr554->i2c_client->dev,"To read ALS validity status : cat alsdatavaliditystatus\n");
	dev_info(&ltr554->i2c_client->dev,"To read register ALS_PS_STATUS (0x8C) : cat alspsstatusreg\n\n");
	// address 0x8C

	// address 0x88, 0x89, 0x8A, 0x8B
	dev_info(&ltr554->i2c_client->dev,"ALS raw and calculated data, address 0x88, 0x89, 0x8A, 0x8B\n");
	dev_info(&ltr554->i2c_client->dev,"To read raw and calculated ALS data : cat alsch0ch1rawcalc\n\n");
	// address 0x88, 0x89, 0x8A, 0x8B

	// address 0x94, 0x95
	dev_info(&ltr554->i2c_client->dev,"To set PS offset (0 ~ 1023) : echo [decimal value] > setpsoffset\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 55 : echo 55 > setpsoffset\n");
	dev_info(&ltr554->i2c_client->dev,"To read back the offset value : cat setpsoffset\n\n");
	// address 0x94, 0x95

	// address 0x8F
	dev_info(&ltr554->i2c_client->dev,"Address 0x8F (INTERRUPT)\n");
	dev_info(&ltr554->i2c_client->dev,"INT output pin inactive : echo 0 > interruptmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"Only PS triggers interrupt : echo 1 > interruptmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"Only ALS triggers interrupt : echo 2 > interruptmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"Both ALS PS trigger interrupt : echo 3 > interruptmodesetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read interrupt mode : cat interruptmodesetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"INT output pin active low : echo 0 > interruptpolarsetup\n");
	dev_info(&ltr554->i2c_client->dev,"INT output pin active high : echo 1 > interruptpolarsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read interrupt pin polarity : cat interruptpolarsetup\n\n");

	dev_info(&ltr554->i2c_client->dev,"Write value to INTERRUPT register (0x8F) : echo [hexcode value] > interruptsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x0B : echo B > interruptsetup or echo b > interruptsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x13 : echo 13 > interruptsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register INTERRUPT (0x8F) : cat interruptsetup\n\n");
	// address 0x8F

	// address 0x9E
	dev_info(&ltr554->i2c_client->dev,"Address 0x9E (INTERRUPT PERSIST)\n");
	dev_info(&ltr554->i2c_client->dev,"Write value to INTERRUPT register (0x9E) : echo [hexcode value] > interruptpersistsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x0B : echo B > interruptpersistsetup or echo b > interruptpersistsetup\n");
	dev_info(&ltr554->i2c_client->dev,"Example...to write 0x13 : echo 13 > interruptpersistsetup\n");
	dev_info(&ltr554->i2c_client->dev,"To read register INTERRUPT PERSIST (0x9E) : cat interruptpersistsetup\n\n");
	// address 0x9E

	// ALS threshold setting
	dev_info(&ltr554->i2c_client->dev,"ALS threshold setting 0x97, 0x98, 0x99, 0x9A\n");
	dev_info(&ltr554->i2c_client->dev,"To set ALS lo threshold : echo [lo limit in decimal] > setalslothrerange\n");
	dev_info(&ltr554->i2c_client->dev,"Example...To set 20 to lo threshold : echo 20 > setalslothrerange\n");
	dev_info(&ltr554->i2c_client->dev,"To set ALS hi threshold : echo [hi limit in decimal] > setalshithrerange\n");
	dev_info(&ltr554->i2c_client->dev,"Example...To set 999 to hi threshold : echo 999 > setalshithrerange\n");
	dev_info(&ltr554->i2c_client->dev,"To read the threshold values : cat dispalsthrerange\n\n");
	// ALS threshold setting

	// PS threshold setting
	dev_info(&ltr554->i2c_client->dev,"PS threshold setting 0x90, 0x91, 0x92, 0x93\n");
	dev_info(&ltr554->i2c_client->dev,"To set PS lo threshold : echo [lo limit in decimal] > setpslothrerange\n");
	dev_info(&ltr554->i2c_client->dev,"Example...To set 20 to lo threshold : echo 20 > setpslothrerange\n");
	dev_info(&ltr554->i2c_client->dev,"To set PS hi threshold : echo [hi limit in decimal] > setpshithrerange\n");
	dev_info(&ltr554->i2c_client->dev,"Example...To set 999 to hi threshold : echo 999 > setpshithrerange\n");
	dev_info(&ltr554->i2c_client->dev,"To read the threshold values : cat disppsthrerange\n\n");
	// PS threshold setting

	return 0;
}

static DEVICE_ATTR(ltr554help, 0666, ltr554help_show, NULL);

#if (ALS_WORK_MODE == ALS_WORK_INTERRUPT_MODE)
static ssize_t alsinterruptmodesetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;
	ltr554_wait_for_device_resume(ltr554);
	ret = als_contr_readback(ALS_MODE_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_MODE_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t alsinterruptmodesetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	int8_t ret;
	struct ltr554_data *ltr554 = sensor_info;

	sscanf(buf, "%d", &param);
	ltr554_wait_for_device_resume(ltr554);
	/*Note that when this register is to be set with values other than its default values,
	it must be set before device is in Active mode*/
	read_ps_als_nvitem(ltr554);

	ret = als_mode_setup(ALS_MODE_STDBY,ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS mode setup Fail...\n", __func__);
		return (-1);
	}

	/*set als interrupt mode*/
	ret = als_interrupt_mode_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: interrupt mode setup Fail...\n", __func__);
		return (-1);
	}
	/*set als operation mode*/
	ret = als_mode_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS mode setup Fail...\n", __func__);
		return (-1);
	}

	cancel_delayed_work(&ltr554->dwork);
	return count;
}

static DEVICE_ATTR(light_enable, 0666, alsinterruptmodesetup_show, alsinterruptmodesetup_store);
#endif

#if (ALS_WORK_MODE == ALS_WORK_POLLING_MODE)
static ssize_t alsmodesetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;
	ltr554_wait_for_device_resume(ltr554);
	ret = als_contr_readback(ALS_MODE_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_MODE_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static ssize_t alsmodesetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	int8_t ret;
	struct ltr554_data *ltr554 = sensor_info;

	sscanf(buf, "%d", &param);
	ltr554_wait_for_device_resume(ltr554);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	/*Note that when this register is to be set with values other than its default values,
	it must be set before device is in Active mode*/
	read_ps_als_nvitem(ltr554);

	ret = als_mode_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS mode setup Fail...\n", __func__);
		return (-1);
	}
	dev_info(&ltr554->i2c_client->dev,"alsmodesetup_store param = %d\n",param);

	if(param){
		cancel_delayed_work(&ltr554->als_dwork);
		ltr554->als_suspend_enable_flag = 1;
		queue_delayed_work(ltr554_workqueue, &ltr554->als_dwork, msecs_to_jiffies(ltr554->als_poll_delay));
	}
	else{
		cancel_delayed_work(&ltr554->als_dwork);
		ltr554->als_suspend_enable_flag = 0;
	}

	return count;

}

static DEVICE_ATTR(light_enable, 0666, alsmodesetup_show, alsmodesetup_store);
#endif

static ssize_t alsswresetsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_contr_readback(ALS_SWRT_RDBCK, &rdback_val, ltr554);

	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_SWRT_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);
	return ret;
}


static ssize_t alsswresetsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	int8_t ret;
    struct ltr554_data *ltr554 = sensor_info;

	sscanf(buf, "%d", &param);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = als_sw_reset_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS sw reset setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(alsswresetsetup, 0666, alsswresetsetup_show, alsswresetsetup_store);


static ssize_t alsgainsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_contr_readback(ALS_GAIN_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_GAIN_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t alsgainsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 1) {
		param_temp[0] = 0;
		param_temp[1] = 0;
	} else if (count == 2) {
		param_temp[0] -= 48;
		param_temp[1] = 0;

		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count >= 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
	}

	param = ((param_temp[0] * 10) + param_temp[1]);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = als_gain_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS gain setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(alsgainsetup, 0666, alsgainsetup_show, alsgainsetup_store);


static ssize_t alscontrsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_contr_readback(ALS_CONTR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_CONTR_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static ssize_t alscontrsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 1) {
		param_temp[0] = 48;
		param_temp[1] = 48;
	} else if (count == 2) {
		param_temp[1] = param_temp[0];
		param_temp[0] = 48;
	}


	if (param_temp[0] >= 65 && param_temp[0] <= 70) {
		param_temp[0] -= 55;
	} else if (param_temp[0] >= 97 && param_temp[0] <= 102) {
		param_temp[0] -= 87;
	} else if (param_temp[0] >= 48 && param_temp[0] <= 57) {
		param_temp[0] -= 48;
	} else {
		param_temp[0] = 0;
	}

	if (param_temp[1] >= 65 && param_temp[1] <= 70) {
		param_temp[1] -= 55;
	} else if (param_temp[1] >= 97 && param_temp[1] <= 102) {
		param_temp[1] -= 87;
	} else if (param_temp[1] >= 48 && param_temp[1] <= 57) {
		param_temp[1] -= 48;
	} else {
		param_temp[1] = 0;
	}

	param = ((param_temp[0] << 4) + (param_temp[1]));
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = als_contr_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS contr setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(alscontrsetup, 0666, alscontrsetup_show, alscontrsetup_store);


static ssize_t psmodesetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_contr_readback(PS_MODE_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS_MODE_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t psmodesetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	int8_t ret;
	struct ltr554_data *ltr554 = sensor_info;

	sscanf(buf, "%d", &param);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_mode_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS mode setup Fail...\n", __func__);
		return (-1);
	}

	return count;

}

static DEVICE_ATTR(psmodesetup, 0666, psmodesetup_show, psmodesetup_store);


static ssize_t psgainsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_contr_readback(PS_GAIN_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS_GAIN_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t psgainsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	uint8_t param;
	int8_t ret;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 2) {
		param_temp[0] = 0;
		param_temp[1] = 0;
	} else if (count >= 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
	}

	//for (i_ctr = 0; i_ctr < sizeof(param_temp); i_ctr++) {
	//	if ((param_temp[i_ctr] < 0) ||(param_temp[i_ctr] > 9)) {
	//		param_temp[i_ctr] = 0;
	//	}
	//}

	param = ((param_temp[0] * 10) + param_temp[1]);

	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_gain_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS gain setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(psgainsetup, 0666, psgainsetup_show, psgainsetup_store);


static ssize_t pssatuindicasetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_contr_readback(PS_SATUR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS_SATUR_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t pssatuindicasetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	int8_t ret;
	struct ltr554_data *ltr554 = sensor_info;

	sscanf(buf, "%d", &param);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_satu_indica_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS saturation indicator setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(pssatuindicasetup, 0666, pssatuindicasetup_show, pssatuindicasetup_store);


static ssize_t pscontrsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_contr_readback(PS_CONTR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS_CONTR_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t pscontrsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 1) {
		param_temp[0] = 48;
		param_temp[1] = 48;
	} else if (count == 2) {
		param_temp[1] = param_temp[0];
		param_temp[0] = 48;
	}

	if (param_temp[0] >= 65 && param_temp[0] <= 70) {
		param_temp[0] -= 55;
	} else if (param_temp[0] >= 97 && param_temp[0] <= 102) {
		param_temp[0] -= 87;
	} else if (param_temp[0] >= 48 && param_temp[0] <= 57) {
		param_temp[0] -= 48;
	} else {
		param_temp[0] = 0;
	}

	if (param_temp[1] >= 65 && param_temp[1] <= 70) {
		param_temp[1] -= 55;
	} else if (param_temp[1] >= 97 && param_temp[1] <= 102) {
		param_temp[1] -= 87;
	} else if (param_temp[1] >= 48 && param_temp[1] <= 57) {
		param_temp[1] -= 48;
	} else {
		param_temp[1] = 0;
	}

	param = ((param_temp[0] << 4) + (param_temp[1]));
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_contr_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS contr setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(pscontrsetup, 0666, pscontrsetup_show, pscontrsetup_store);


static ssize_t psledcurrsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_led_readback(LED_CURR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: LED_CURR_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}


static ssize_t psledcurrsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[3];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];

	if (count <=1) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
	} else if (count == 2) {
		param_temp[0] -= 48;
		param_temp[1] = 0;
		param_temp[2] = 0;

		param_temp[2] = param_temp[0];
		param_temp[0] = 0;
		param_temp[1] = 0;
	} else if (count == 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;

		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count > 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
	}

	//for (i_ctr = 0; i_ctr < sizeof(param_temp); i_ctr++) {
	//	if ((param_temp[i_ctr] < 0) ||(param_temp[i_ctr] > 9)) {
	//		param_temp[i_ctr] = 0;
	//	}
	//}

	param = ((param_temp[0] * 100) + (param_temp[1] * 10) + param_temp[2]);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_ledCurrent_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED current setup Fail...\n", __func__);
		return (-1);
	}

	return count;

}

static DEVICE_ATTR(psledcurrsetup, 0666, psledcurrsetup_show, psledcurrsetup_store);


static ssize_t psledcurrduty_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_led_readback(LED_CURR_DUTY_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: LED_CURR_DUTY_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}


static ssize_t psledcurrduty_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[3];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];

	if (count < 3) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
	} else if (count == 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;

		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count > 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
	}

	//for (i_ctr = 0; i_ctr < sizeof(param_temp); i_ctr++) {
	//	if ((param_temp[i_ctr] < 0) ||(param_temp[i_ctr] > 9)) {
	//		param_temp[i_ctr] = 0;
	//	}
	//}

	param = ((param_temp[0] * 100) + (param_temp[1] * 10) + param_temp[2]);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_ledCurrDuty_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED curent duty setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(psledcurrduty, 0666, psledcurrduty_show, psledcurrduty_store);


static ssize_t psledpulsefreqsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_led_readback(LED_PUL_FREQ_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: LED_PUL_FREQ_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}


static ssize_t psledpulsefreqsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[3];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];

	if (count < 3) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
	} else if (count == 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;

		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count > 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
	}

	//for (i_ctr = 0; i_ctr < sizeof(param_temp); i_ctr++) {
	//	if ((param_temp[i_ctr] < 0) ||(param_temp[i_ctr] > 9)) {
	//		param_temp[i_ctr] = 0;
	//	}
	//}

	param = ((param_temp[0] * 100) + (param_temp[1] * 10) + param_temp[2]);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_ledPulseFreq_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED pulse frequency setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(psledpulsefreqsetup, 0666, psledpulsefreqsetup_show, psledpulsefreqsetup_store);


static ssize_t psledsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_led_readback(PS_LED_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS_LED_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t psledsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 1) {
		param_temp[0] = 48;
		param_temp[1] = 48;
	} else if (count == 2) {
		param_temp[1] = param_temp[0];
		param_temp[0] = 48;
	}

	if (param_temp[0] >= 65 && param_temp[0] <= 70) {
		param_temp[0] -= 55;
	} else if (param_temp[0] >= 97 && param_temp[0] <= 102) {
		param_temp[0] -= 87;
	} else if (param_temp[0] >= 48 && param_temp[0] <= 57) {
		param_temp[0] -= 48;
	} else {
		param_temp[0] = 0;
	}

	if (param_temp[1] >= 65 && param_temp[1] <= 70) {
		param_temp[1] -= 55;
	} else if (param_temp[1] >= 97 && param_temp[1] <= 102) {
		param_temp[1] -= 87;
	} else if (param_temp[1] >= 48 && param_temp[1] <= 57) {
		param_temp[1] -= 48;
	} else {
		param_temp[1] = 0;
	}

	param = ((param_temp[0] << 4) + (param_temp[1]));
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_led_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(psledsetup, 0666, psledsetup_show, psledsetup_store);


static ssize_t psledpulsecountsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_ledPulseCount_readback(&rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED pulse count readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t psledpulsecountsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if ((count <= 1) || (count > 3)){
		param_temp[0] = 0;
		param_temp[1] = 0;
	} else if (count == 2) {
		param_temp[0] -= 48;
		param_temp[1] = 0;

		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count == 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
	}

	param = ((param_temp[0] * 10) + param_temp[1]);
	if (param > 15) {
		param = 15;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_ledPulseCount_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS LED pulse count setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(psledpulsecountsetup, 0666, psledpulsecountsetup_show, psledpulsecountsetup_store);


static ssize_t psmeasratesetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_meas_rate_readback(&rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS meas rate readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}


static ssize_t psmeasratesetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint16_t param;
	//int *param_temp = buf;
	int param_temp[4];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];
	param_temp[3] = buf[3];

	if (count <= 2) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
	} else if (count == 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[1];
		param_temp[2] = param_temp[0];
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 4) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] = 0;

		param_temp[3] = param_temp[2];
		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count > 4) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
	}

	param = ((param_temp[0] * 1000) + (param_temp[1] * 100) + (param_temp[2] * 10) + param_temp[3]);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = ps_meas_rate_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS measurement rate setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(psmeasratesetup, 0666, psmeasratesetup_show, psmeasratesetup_store);


static ssize_t alsmeasratesetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_meas_rate_readback(ALS_MEAS_RPT_RATE_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_MEAS_RPT_RATE_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}


static ssize_t alsmeasratesetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint16_t param;
	//int *param_temp = buf;
	int param_temp[4];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];
	param_temp[3] = buf[3];

	if (count <= 2) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
	} else if (count == 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[1];
		param_temp[2] = param_temp[0];
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 4) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] = 0;

		param_temp[3] = param_temp[2];
		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count > 4) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
	}

	//for (i_ctr = 0; i_ctr < sizeof(param_temp); i_ctr++) {
	//	if ((param_temp[i_ctr] < 0) ||(param_temp[i_ctr] > 9)) {
	//		param_temp[i_ctr] = 0;
	//	}
	//}

	param = ((param_temp[0] * 1000) + (param_temp[1] * 100) + (param_temp[2] * 10) + param_temp[3]);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = als_meas_rate_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS measurement rate setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(alsmeasratesetup, 0666, alsmeasratesetup_show, alsmeasratesetup_store);


static ssize_t alsintegtimesetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_meas_rate_readback(ALS_INTEG_TM_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_INTEG_TM_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}


static ssize_t alsintegtimesetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint16_t param;
	//int *param_temp = buf;

	int param_temp[3];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];

	if (count <= 2) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
	} else if (count == 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;

		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count > 3) {
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
	}

	//for (i_ctr = 0; i_ctr < sizeof(param_temp); i_ctr++) {
	//	if ((param_temp[i_ctr] < 0) ||(param_temp[i_ctr] > 9)) {
	//		param_temp[i_ctr] = 0;
	//	}
	//}

	param = ((param_temp[0] * 100) + (param_temp[1] * 10) + param_temp[2]);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = als_integ_time_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS integration time setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(alsintegtimesetup, 0666, alsintegtimesetup_show, alsintegtimesetup_store);


static ssize_t alsmeasrateregsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_meas_rate_readback(ALS_MEAS_RATE_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_MEAS_RATE_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}


static ssize_t alsmeasrateregsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 1) {
		param_temp[0] = 48;
		param_temp[1] = 48;
	} else if (count == 2) {
		param_temp[1] = param_temp[0];
		param_temp[0] = 48;
	}

	if (param_temp[0] >= 65 && param_temp[0] <= 70) {
		param_temp[0] -= 55;
	} else if (param_temp[0] >= 97 && param_temp[0] <= 102) {
		param_temp[0] -= 87;
	} else if (param_temp[0] >= 48 && param_temp[0] <= 57) {
		param_temp[0] -= 48;
	} else {
		param_temp[0] = 0;
	}

	if (param_temp[1] >= 65 && param_temp[1] <= 70) {
		param_temp[1] -= 55;
	} else if (param_temp[1] >= 97 && param_temp[1] <= 102) {
		param_temp[1] -= 87;
	} else if (param_temp[1] >= 48 && param_temp[1] <= 57) {
		param_temp[1] -= 48;
	} else {
		param_temp[1] = 0;
	}

	param = ((param_temp[0] << 4) + (param_temp[1]));
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = als_meas_rate_reg_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS meas rate register setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(alsmeasrateregsetup, 0666, alsmeasrateregsetup_show, alsmeasrateregsetup_store);


static ssize_t partid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = part_ID_reg_readback(PART_NUM_ID_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PART_NUM_ID_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(partid, 0666, partid_show, NULL);


static ssize_t revid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = part_ID_reg_readback(REVISION_ID_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: REVISION_ID_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(revid, 0666, revid_show, NULL);


static ssize_t partidreg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = part_ID_reg_readback(PART_ID_REG_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PART_ID_REG_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(partidreg, 0666, partidreg_show, NULL);


static ssize_t manuid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = manu_ID_reg_readback(&rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Manufacturing ID readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(manuid, 0666, manuid_show, NULL);


static ssize_t psdatastatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ps_status_reg(PS_DATA_STATUS_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS_DATA_STATUS_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(psdatastatus, 0666, psdatastatus_show, NULL);


static ssize_t psinterruptstatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ps_status_reg(PS_INTERR_STATUS_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS_INTERR_STATUS_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(psinterruptstatus, 0666, psinterruptstatus_show, NULL);


static ssize_t alsdatastatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ps_status_reg(ALS_DATA_STATUS_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_DATA_STATUS_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(alsdatastatus, 0666, alsdatastatus_show, NULL);


static ssize_t alsinterruptstatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ps_status_reg(ALS_INTERR_STATUS_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_INTERR_STATUS_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(alsinterruptstatus, 0666, alsinterruptstatus_show, NULL);


static ssize_t alsgainstatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ps_status_reg(ALS_GAIN_STATUS_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_GAIN_STATUS_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(alsgainstatus, 0666, alsgainstatus_show, NULL);


static ssize_t alsdatavaliditystatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ps_status_reg(ALS_VALID_STATUS_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_VALID_STATUS_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}

static DEVICE_ATTR(alsdatavaliditystatus, 0666, alsdatavaliditystatus_show, NULL);


static ssize_t alspsstatusreg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ps_status_reg(ALS_PS_STATUS_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_PS_STATUS_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;

}

static DEVICE_ATTR(alspsstatusreg, 0666, alspsstatusreg_show, NULL);


static ssize_t alsch0ch1rawcalc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint16_t rdback_val1 = 0, rdback_val2 = 0, rdback_val3 = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_ch0ch1raw_calc_readback(&rdback_val1, &rdback_val2, &rdback_val3, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS CH0 CH1 Calc reading readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d %d %d\n", rdback_val1, rdback_val2, rdback_val3);

	return ret;

}

static DEVICE_ATTR(alsch0ch1rawcalc, 0666, alsch0ch1rawcalc_show, NULL);


static ssize_t setpsoffset_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint16_t rdback_val;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_offset_readback(&rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS offset readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t setpsoffset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint16_t ps_offset = 0;
	uint8_t param_temp[4];
	struct ltr554_data *ltr554 = sensor_info;

	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];
	param_temp[3] = buf[3];

	if (count <= 1) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
	} else if (count == 2) { // 1 digit
		param_temp[0] -= 48;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[0];
		param_temp[2] = 0;
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 3) { // 2 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[1];
		param_temp[2] = param_temp[0];
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 4) { // 3 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] = 0;

		param_temp[3] = param_temp[2];
		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count >= 5) { // 4 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
	}

	ps_offset = ((param_temp[0] * 1000) + (param_temp[1] * 100) + (param_temp[2] * 10) + param_temp[3]);
	if (ps_offset > 1023) {
		ps_offset = 1023;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, ps_offset);

	ret = ps_offset_setup(ps_offset, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: set ps offset Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(setpsoffset, 0666, setpsoffset_show, setpsoffset_store);


static ssize_t psinterruptmodesetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;
	ltr554_wait_for_device_resume(ltr554);
	ret = interrupt_readback(INT_MODE_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: INT_MODE_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t psinterruptmodesetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	int8_t ret;
	uint8_t rdback_val = 0;

	struct ltr554_data *ltr554 = sensor_info;
	ltr554_wait_for_device_resume(ltr554);
	read_ps_als_nvitem(ltr554);

	sscanf(buf, "%d", &param);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);
	setThrDuringCall();

	/*Note that when this register is to be set with values other than its default values,
	it must be set before device is in Active mode*/

	/*readback als 0x80 register,set als operation to standby*/
	ret = als_contr_readback(ALS_MODE_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS_CONTR_RDBCK Fail...\n", __func__);
		return (-1);
	}
	ret = als_mode_setup(ALS_MODE_STDBY,ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS mode setup Fail...\n", __func__);
		return (-1);
	}
	/*set ps operation mode to standy*/
	ret = ps_mode_setup(PS_MODE_STDBY, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS mode setup Fail...\n", __func__);
		return (-1);
	}

	//set ps interrupt mode
	ret = ps_interrupt_mode_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: interrupt mode setup Fail...\n", __func__);
		return (-1);
	}
	/*set ps operation mode*/
	ret = ps_mode_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS mode setup Fail...\n", __func__);
		return (-1);
	}
	/*restore als mode*/
	ret = als_mode_setup(rdback_val,ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS mode setup Fail...\n", __func__);
		return (-1);
	}

	cancel_delayed_work(&ltr554->dwork);

	return count;
}

static DEVICE_ATTR(proximity_enable, 0666, psinterruptmodesetup_show, psinterruptmodesetup_store);


static ssize_t interruptpolarsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = interrupt_readback(INT_POLAR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: INT_POLAR_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t interruptpolarsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	int8_t ret;

	struct ltr554_data *ltr554 = sensor_info;

	sscanf(buf, "%d", &param);
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = interrupt_polarity_setup((uint8_t)param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: interrupt polarity setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(interruptpolarsetup, 0666, interruptpolarsetup_show, interruptpolarsetup_store);


static ssize_t interruptsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = interrupt_readback(INT_INTERRUPT_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: INT_INTERRUPT_RDBCK Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t interruptsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint8_t param;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	//sscanf(buf, "%d", param_temp);
	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 1) {
		param_temp[0] = 48;
		param_temp[1] = 48;
	} else if (count == 2) {
		param_temp[1] = param_temp[0];
		param_temp[0] = 48;
	}

	if (param_temp[0] >= 65 && param_temp[0] <= 70) {
		param_temp[0] -= 55;
	} else if (param_temp[0] >= 97 && param_temp[0] <= 102) {
		param_temp[0] -= 87;
	} else if (param_temp[0] >= 48 && param_temp[0] <= 57) {
		param_temp[0] -= 48;
	} else {
		param_temp[0] = 0;
	}

	if (param_temp[1] >= 65 && param_temp[1] <= 70) {
		param_temp[1] -= 55;
	} else if (param_temp[1] >= 97 && param_temp[1] <= 102) {
		param_temp[1] -= 87;
	} else if (param_temp[1] >= 48 && param_temp[1] <= 57) {
		param_temp[1] -= 48;
	} else {
		param_temp[1] = 0;
	}

	param = ((param_temp[0] << 4) + (param_temp[1]));
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, param);

	ret = interrupt_setup(param, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: interrupt setup Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(interruptsetup, 0666, interruptsetup_show, interruptsetup_store);


static ssize_t interruptpersistsetup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint8_t rdback_val = 0;
	struct ltr554_data *ltr554 = sensor_info;

	ret = interrupt_prst_readback(&rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Interrupt persist readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d\n", rdback_val);

	return ret;
}


static ssize_t interruptpersistsetup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret = 0;
	//uint8_t als_or_ps, prst_val;
	uint8_t prst_val;
	//int *param_temp = buf;
	int param_temp[2];

	struct ltr554_data *ltr554 = sensor_info;

	param_temp[0] = buf[0];
	param_temp[1] = buf[1];

	if (count <= 1) {
		param_temp[0] = 48;
		param_temp[1] = 48;
	} else if (count == 2) {
		param_temp[1] = param_temp[0];
		param_temp[0] = 48;
	}

	if (param_temp[0] >= 65 && param_temp[0] <= 70) {
		param_temp[0] -= 55;
	} else if (param_temp[0] >= 97 && param_temp[0] <= 102) {
		param_temp[0] -= 87;
	} else if (param_temp[0] >= 48 && param_temp[0] <= 57) {
		param_temp[0] -= 48;
	} else {
		param_temp[0] = 0;
	}

	if (param_temp[1] >= 65 && param_temp[1] <= 70) {
		param_temp[1] -= 55;
	} else if (param_temp[1] >= 97 && param_temp[1] <= 102) {
		param_temp[1] -= 87;
	} else if (param_temp[1] >= 48 && param_temp[1] <= 57) {
		param_temp[1] -= 48;
	} else {
		param_temp[1] = 0;
	}

	prst_val = ((param_temp[0] << 4) + (param_temp[1]));
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, prst_val);

	//ret = interrupt_persist_setup(als_or_ps, prst_val, ltr554);
	ret = interrupt_persist_setup(prst_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Interrupt persist setup Fail...\n", __func__);
		return (-1);
	}

	return count;

}

static DEVICE_ATTR(interruptpersistsetup, 0666, interruptpersistsetup_show, interruptpersistsetup_store);


static ssize_t setalslothrerange_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	int lo_thr = 0;
	uint8_t param_temp[5];
	struct ltr554_data *ltr554 = sensor_info;

	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];
	param_temp[3] = buf[3];
	param_temp[4] = buf[4];

	if (count <= 1) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
		param_temp[4] = 0;
	} else if (count == 2) { // 1 digit
		param_temp[0] -= 48;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
		param_temp[4] = 0;

		param_temp[4] = param_temp[0];
		param_temp[3] = 0;
		param_temp[2] = 0;
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 3) { // 2 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;
		param_temp[3] = 0;
		param_temp[4] = 0;

		param_temp[4] = param_temp[1];
		param_temp[3] = param_temp[0];
		param_temp[2] = 0;
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 4) { // 3 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] = 0;
		param_temp[4] = 0;

		param_temp[4] = param_temp[2];
		param_temp[3] = param_temp[1];
		param_temp[2] = param_temp[0];
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 5) { // 4 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
		param_temp[4] = 0;

		param_temp[4] = param_temp[3];
		param_temp[3] = param_temp[2];
		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count >= 6) { // 5 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
		param_temp[4] -= 48;
	}

	lo_thr = ((param_temp[0] * 10000) + (param_temp[1] * 1000) + (param_temp[2] * 100) + (param_temp[3] * 10) + param_temp[4]);
	if (lo_thr > 65535) {
		lo_thr = 65535;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, lo_thr);

	ret = set_als_range((uint16_t)lo_thr, 0, LO_LIMIT);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: set ALS lo threshold Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(setalslothrerange, 0666, NULL, setalslothrerange_store);


static ssize_t setalshithrerange_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	int hi_thr = 0;
	uint8_t param_temp[5];
	struct ltr554_data *ltr554 = sensor_info;

	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];
	param_temp[3] = buf[3];
	param_temp[4] = buf[4];

	if (count <= 1) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
		param_temp[4] = 0;
	} else if (count == 2) { // 1 digit
		param_temp[0] -= 48;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
		param_temp[4] = 0;

		param_temp[4] = param_temp[0];
		param_temp[3] = 0;
		param_temp[2] = 0;
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 3) { // 2 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;
		param_temp[3] = 0;
		param_temp[4] = 0;

		param_temp[4] = param_temp[1];
		param_temp[3] = param_temp[0];
		param_temp[2] = 0;
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 4) { // 3 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] = 0;
		param_temp[4] = 0;

		param_temp[4] = param_temp[2];
		param_temp[3] = param_temp[1];
		param_temp[2] = param_temp[0];
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 5) { // 4 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
		param_temp[4] = 0;

		param_temp[4] = param_temp[3];
		param_temp[3] = param_temp[2];
		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count >= 6) { // 5 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
		param_temp[4] -= 48;
	}

	hi_thr = ((param_temp[0] * 10000) + (param_temp[1] * 1000) + (param_temp[2] * 100) + (param_temp[3] * 10) + param_temp[4]);
	if (hi_thr > 65535) {
		hi_thr = 65535;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, hi_thr);

	ret = set_als_range(0, (uint16_t)hi_thr, HI_LIMIT);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: set ALS hi threshold Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(setalshithrerange, 0666, NULL, setalshithrerange_store);


static ssize_t dispalsthrerange_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint16_t rdback_lo, rdback_hi;
	struct ltr554_data *ltr554 = sensor_info;

	ret = als_range_readback(&rdback_lo, &rdback_hi, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS threshold range readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d %d\n", rdback_lo, rdback_hi);

	return ret;
}

static DEVICE_ATTR(dispalsthrerange, 0666, dispalsthrerange_show, NULL);


static ssize_t setpslothrerange_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint16_t lo_thr = 0;
	uint8_t param_temp[4];
	struct ltr554_data *ltr554 = sensor_info;

	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];
	param_temp[3] = buf[3];

	if (count <= 1) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
	} else if (count == 2) { // 1 digit
		param_temp[0] -= 48;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[0];
		param_temp[2] = 0;
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 3) { // 2 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[1];
		param_temp[2] = param_temp[0];
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 4) { // 3 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] = 0;

		param_temp[3] = param_temp[2];
		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count >= 5) { // 4 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
	}

	lo_thr = ((param_temp[0] * 1000) + (param_temp[1] * 100) + (param_temp[2] * 10) + param_temp[3]);
	if (lo_thr > 2047) {
		lo_thr = 2047;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, lo_thr);

	ret = set_ps_range(lo_thr, 0, LO_LIMIT, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: set PS lo threshold Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(setpslothrerange, 0666, NULL, setpslothrerange_store);


static ssize_t setpshithrerange_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int8_t ret;
	uint16_t hi_thr = 0;
	uint8_t param_temp[4];
	struct ltr554_data *ltr554 = sensor_info;

	param_temp[0] = buf[0];
	param_temp[1] = buf[1];
	param_temp[2] = buf[2];
	param_temp[3] = buf[3];

	if (count <= 1) {
		param_temp[0] = 0;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;
	} else if (count == 2) { // 1 digit
		param_temp[0] -= 48;
		param_temp[1] = 0;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[0];
		param_temp[2] = 0;
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 3) { // 2 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] = 0;
		param_temp[3] = 0;

		param_temp[3] = param_temp[1];
		param_temp[2] = param_temp[0];
		param_temp[1] = 0;
		param_temp[0] = 0;
	} else if (count == 4) { // 3 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] = 0;

		param_temp[3] = param_temp[2];
		param_temp[2] = param_temp[1];
		param_temp[1] = param_temp[0];
		param_temp[0] = 0;
	} else if (count >= 5) { // 4 digits
		param_temp[0] -= 48;
		param_temp[1] -= 48;
		param_temp[2] -= 48;
		param_temp[3] -= 48;
	}

	hi_thr = ((param_temp[0] * 1000) + (param_temp[1] * 100) + (param_temp[2] * 10) + param_temp[3]);
	if (hi_thr > 2047) {
		hi_thr = 2047;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s: store value = %d\n", __func__, hi_thr);

	ret = set_ps_range(0, hi_thr, HI_LIMIT, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: set PS hi threshold Fail...\n", __func__);
		return (-1);
	}

	return count;
}

static DEVICE_ATTR(setpshithrerange, 0666, NULL, setpshithrerange_store);


static ssize_t disppsthrerange_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int8_t ret = 0;
	uint16_t rdback_lo, rdback_hi;
	struct ltr554_data *ltr554 = sensor_info;

	ret = ps_range_readback(&rdback_lo, &rdback_hi, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS threshold range readback Fail...\n", __func__);
		return (-1);
	}

	ret = sprintf(buf, "%d %d\n", rdback_lo, rdback_hi);

	return ret;
}

static DEVICE_ATTR(disppsthrerange, 0666, disppsthrerange_show, NULL);

/* Proximity self test function */
static ssize_t ltr554_selftest_run_and_get_data(struct device *dev,
		struct device_attribute *attr, char *buf)
{

	struct ltr554_data *ltr554 = sensor_info;

	struct PROX_SELFTEST_RESULT* ltr554_prox_selftest_result = (struct PROX_SELFTEST_RESULT*)buf;
	uint16_t adc_value;
	int8_t ret = 0;
	uint8_t rdback_ps_int_status = 0;
	uint8_t rdback_ps_data_status = 0;

	ltr554_wait_for_device_resume(ltr554);
	dev_dbg(&ltr554->i2c_client->dev, "%s: enter\n", __func__);
	memset(ltr554_prox_selftest_result, 0, sizeof(*ltr554_prox_selftest_result));

	if (_check_part_id(ltr554) < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Part ID Read Fail after reset...\n", __func__);
		ltr554_prox_selftest_result->i2c_status = false;
		return sizeof(*ltr554_prox_selftest_result);
	}
	else {
		ltr554_prox_selftest_result->i2c_status = true;
	}

	ltr554_prox_selftest_result->interrupt_pin_support = true;
	ltr554_prox_selftest_result->interrupt_pin_status = false;

	if ( (ltr554_prox_selftest_result->i2c_status == true) &&
	(ltr554_prox_selftest_result->interrupt_pin_support == true) ){
		/*disable PS INT and operation mode*/
		_ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_PS_CONTR, PS_MODE_ACTIVE);
		_ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_ALS_CONTR, ALS_MODE_ACTIVE);
		_ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_INTERRUPT, INT_MODE_PS_TRIG);

		atomic_set(&ltr554->ps_selftest_int, 0);
		atomic_set(&ltr554->ps_selftest_ongoing, 1);

		ret = ps_interrupt_mode_setup((uint8_t)INT_MODE_PS_TRIG, ltr554);
		if (ret < 0) {
			dev_err(&ltr554->i2c_client->dev, "%s: interrupt mode setup Fail...\n", __func__);
			return sizeof(*ltr554_prox_selftest_result);
		}

		cancel_delayed_work(&ltr554->dwork);

		_ltr554_set_bit(ltr554->i2c_client, SET_BIT, LTR554_PS_CONTR, PS_MODE_ACTIVE);
		_ltr554_set_bit(ltr554->i2c_client, SET_BIT, LTR554_ALS_CONTR, ALS_MODE_ACTIVE);
		set_ps_range((uint16_t)PS_MAX_MEASURE_VAL,(uint16_t)PS_MIN_MEASURE_VAL, LO_N_HI_LIMIT, ltr554);

		wait_event_interruptible_timeout(ltr554->ps_selftest_wq, atomic_read(&ltr554->ps_selftest_int),
				msecs_to_jiffies(500));

		if (atomic_read(&ltr554->ps_selftest_int) == 1) {
			als_ps_status_reg(PS_INTERR_STATUS_RDBCK, &rdback_ps_int_status, ltr554);
			als_ps_status_reg(PS_DATA_STATUS_RDBCK, &rdback_ps_data_status, ltr554);
			dev_dbg(&ltr554->i2c_client->dev, "%s: rdback_ps_int_status = 0x%x\n", __func__,rdback_ps_int_status);
			dev_dbg(&ltr554->i2c_client->dev, "%s: rdback_ps_data_status = 0x%x\n", __func__,rdback_ps_data_status);
			if ((rdback_ps_int_status & rdback_ps_int_status & 0x1) == 0x1) {
				/*dummy read just to clear the PS interrupt bit*/
				adc_value = read_ps_adc_value (ltr554);
				/*dummy read just to clear the PS interrupt bit*/
				ltr554_prox_selftest_result->interrupt_pin_status = true;
			}
		}
		/*disable PS INT and operation mode*/
		_ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_PS_CONTR, PS_MODE_ACTIVE);
		_ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_ALS_CONTR, ALS_MODE_ACTIVE);
		_ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_INTERRUPT, INT_MODE_PS_TRIG);

		enable_irq(ltr554->irq);
		atomic_set(&ltr554->ps_selftest_ongoing, 0);
	}

	dev_dbg(&ltr554->i2c_client->dev,
	"%s: exit, i2c_status = %d, interrupt_pin_support = %d, interrupt_pin_status = %d\n", __func__,
	ltr554_prox_selftest_result->i2c_status, ltr554_prox_selftest_result->interrupt_pin_support,
	ltr554_prox_selftest_result->interrupt_pin_status);

	return sizeof(*ltr554_prox_selftest_result);
}
/* Proximity self test function */

static DEVICE_ATTR(proximity_selftest, 0666, ltr554_selftest_run_and_get_data, NULL);

static ssize_t ltr554_light_calibration_data_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	struct file *file;
	mm_segment_t old_fs;
	loff_t sz_file;
	int ret = 0;
	struct inode *inode;
	struct ltr554_nvitem_data *ltr554_nvitem;
	char *pReadBuff = NULL;
	unsigned long magic;
	struct ltr554_data *ltr554 = sensor_info;
	uint16_t luminance = 0;
	unsigned char file_exist = 1;

	sscanf(buf, "%d", &param);
	ltr554_wait_for_device_resume(ltr554);
	ret = als_get_light_luminance(ltr554,&luminance);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: als_get_light_luminance read Fail...\n", __func__);
		return -1;
	}

    if (luminance <= 0){
		dev_err(&ltr554->i2c_client->dev, "%s: Invalid light luminance...\n", __func__);
		return -1;
    }

    file = filp_open( LTR554_NV_BIN_FILE_NAME, O_RDWR, 0 );
	if (IS_ERR(file)){
		dev_err(&ltr554->i2c_client->dev,"%s: NV file doesn't exist, to creat new file\n", __func__);
		file = filp_open( LTR554_NV_BIN_FILE_NAME, O_RDWR|O_CREAT, 0777 );
		if (IS_ERR(file)){
			dev_err(&ltr554->i2c_client->dev,"%s: Create file error...\n", __func__);
			return -1;
		}
		file_exist = 0;
	}

	inode=file->f_dentry->d_inode;
	magic=inode->i_sb->s_magic;
	sz_file=inode->i_size;

	pReadBuff = kzalloc(sizeof(*ltr554_nvitem) + 1,GFP_KERNEL);

	if (!pReadBuff){	/*mem alloc  failed*/
		dev_err(&ltr554->i2c_client->dev, "%s: Mem Alloc Fail...\n", __func__);
		filp_close(file,NULL);
		return -ENOMEM;
	}

	old_fs =get_fs();
	set_fs(KERNEL_DS);

	if ( file_exist ){
		if(file->f_op->llseek(file, 0, SEEK_SET) != 0) {
			dev_err(&ltr554->i2c_client->dev, "%s: lseek failure...\n", __func__);
			ret = -1;
			goto Exit;
		}

		ret = file->f_op->read( file, pReadBuff, sizeof(*ltr554_nvitem), &file->f_pos );
		if(!ret){
			dev_err(&ltr554->i2c_client->dev, "%s: read file error...\n", __func__);
			goto Exit;
		}

		if(file->f_op->llseek(file, 0, SEEK_SET) != 0) {
			dev_err(&ltr554->i2c_client->dev, "%s: lseek failure...\n", __func__);
			ret = -1;
			goto Exit;
		}

	}


	ltr554_nvitem = (struct ltr554_nvitem_data*)pReadBuff;
	ltr554_nvitem->sensor_part_id = PARTID_V5;
	ltr554_nvitem->sensor_manufac_id = 0x5;
	ltr554_nvitem->als_cal_fac = param*100/luminance;
	ltr554_nvitem->nv_als_calibration_done = 1;
	dev_dbg(&ltr554->i2c_client->dev, "%s: als_cal_fac = 0x%x\n", __func__,ltr554_nvitem->als_cal_fac);
	ret = file->f_op->write(file, pReadBuff, sizeof(*ltr554_nvitem),&file->f_pos);
    if(!ret){
		dev_err(&ltr554->i2c_client->dev,"%s: file write error...\n", __func__);
		ret = -1;
		goto Exit;
    }
Exit:
	/*free buffer and close file*/
	kfree(pReadBuff);
	filp_close(file,NULL);
	/*resume user space*/
	set_fs(old_fs);

	if(!ret){
	dev_err(&ltr554->i2c_client->dev, "%s: write operation error...\n", __func__);
	return -1;
	}

	return count;
}

static DEVICE_ATTR(light_calibration_data, 0666, NULL, ltr554_light_calibration_data_store);


static ssize_t ltr554_show_light_poll_delay(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ltr554_data *ltr554 = sensor_info;

	return sprintf(buf, "%d\n", ltr554->als_poll_delay);
}

static ssize_t ltr554_store_light_poll_delay(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct ltr554_data *ltr554 = sensor_info;
	unsigned long val = simple_strtoul(buf, NULL, 10);
    unsigned int delay;
    int ret = 0;
	ltr554_wait_for_device_resume(ltr554);
	dev_err(&ltr554->i2c_client->dev,"%s: set als sensor poll delay ( %ld)\n", __func__ ,val);
#if (ALS_WORK_MODE == ALS_WORK_POLLING_MODE)
    if (val >= 1000000000) {
		delay = LTR554_ALS_POLL_SLOW;	/*1000ms*/
	}
	else if (val >= 200000000) {
		delay = LTR554_ALS_POLL_MEDIUM;	/*200 ms*/
	}
	else {
		delay = LTR554_ALS_POLL_FAST;	/*50 ms*/
	}
	ret = set_als_poll_delay(ltr554, delay);
	if (ret < 0) {
	dev_err(&ltr554->i2c_client->dev, "%s: register Setup Fail...\n", __func__);
	return -1;
	}
#else
	dev_err(&ltr554->i2c_client->dev,"Is not in NO_INT state\n", __func__);
#endif
	return count;
}

static DEVICE_ATTR(light_poll_delay, 0666, ltr554_show_light_poll_delay, ltr554_store_light_poll_delay);

static int ltr554_read_nvitem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct file *file;
	mm_segment_t old_fs;
	loff_t sz_file;
	struct inode *inode;
	char *pReadBuff = NULL;
	unsigned long magic;
	struct ltr554_nvitem_data *ltr554_nvitem;
	struct ltr554_data *ltr554 = sensor_info;

	file = filp_open( LTR554_NV_BIN_FILE_NAME, O_RDONLY, 0 );
	if (IS_ERR(file)){
		dev_err(&ltr554->i2c_client->dev, "%s: NV file doesn't exist\n", __func__);
		return -1;
	}
	inode=file->f_dentry->d_inode;
	magic=inode->i_sb->s_magic;
	sz_file=inode->i_size;

	if ((int)sz_file >= (int)sizeof(*ltr554_nvitem)){
		pReadBuff = kzalloc(sizeof(*ltr554_nvitem) + 1,GFP_KERNEL);
	}
	else{			/* skip when file is invaild*/
		filp_close(file,NULL);
		dev_err(&ltr554->i2c_client->dev, "%s: file size error...\n", __func__);
		return -1;
	}

	if (!pReadBuff){	/*mem alloc  failed*/
		dev_err(&ltr554->i2c_client->dev, "%s: Mem Alloc Fail...\n", __func__);
		filp_close(file,NULL);
		return -ENOMEM;
	}

	old_fs =get_fs();
	set_fs(KERNEL_DS);

	if(file->f_op->llseek(file, 0, SEEK_SET) != 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: lseek failure...\n", __func__);
		ret = -1;
		goto Exit;
	}

	ret = file->f_op->read( file, pReadBuff, sizeof(*ltr554_nvitem), &file->f_pos );
	if(!ret){
		dev_err(&ltr554->i2c_client->dev, "%s: read file error...\n", __func__);
		goto Exit;
	}

	ltr554_nvitem = (struct ltr554_nvitem_data *)pReadBuff;
	dev_dbg(&ltr554->i2c_client->dev,
		"%s: sensor_part_id = %x, threshold_cross_talk = %d, als_cal_fac = %d\n", __func__,
		ltr554_nvitem->sensor_part_id, ltr554_nvitem->threshold_cross_talk,ltr554_nvitem->als_cal_fac);

	ret = sprintf(buf, "%x %d\n", ltr554_nvitem->sensor_part_id,ltr554_nvitem->threshold_cross_talk);
Exit:
	/*free buffer and close file*/
	kfree(pReadBuff);
	filp_close(file,NULL);
	/*resume user space*/
	set_fs(old_fs);
	return ret;
}

static int ltr554_write_nvitem_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int param;
	struct file *file;
	mm_segment_t old_fs;
	loff_t sz_file;
	int ret = 0;
	struct inode *inode;
	struct ltr554_nvitem_data *ltr554_nvitem;
	char *pReadBuff = NULL;
	unsigned long magic;
	struct ltr554_data *ltr554 = sensor_info;
	unsigned char file_exist = 1;

	sscanf(buf, "%d", &param);

    file = filp_open( LTR554_NV_BIN_FILE_NAME, O_RDWR, 0 );
    if (IS_ERR(file)){
		dev_err(&ltr554->i2c_client->dev,"%s: NV file doesn't exist, to creat new file\n", __func__);
		file = filp_open( LTR554_NV_BIN_FILE_NAME, O_RDWR|O_CREAT, 0777 );
		if (IS_ERR(file)){
			dev_err(&ltr554->i2c_client->dev,"%s: Create file error...\n", __func__);
			return -1;
		}
		file_exist = 0;
    }

	inode=file->f_dentry->d_inode;
	magic=inode->i_sb->s_magic;
	sz_file=inode->i_size;

	pReadBuff = kzalloc(sizeof(*ltr554_nvitem) + 1,GFP_KERNEL);

	if (!pReadBuff){	/*mem alloc  failed*/
		dev_err(&ltr554->i2c_client->dev, "%s: Mem Alloc Fail...\n", __func__);
		filp_close(file,NULL);
		return -ENOMEM;
	}

	old_fs =get_fs();
	set_fs(KERNEL_DS);

	if ( file_exist ){
		if(file->f_op->llseek(file, 0, SEEK_SET) != 0) {
			dev_err(&ltr554->i2c_client->dev, "%s: lseek failure...\n", __func__);
			ret = -1;
			goto Exit;
		}

		ret = file->f_op->read( file, pReadBuff, sizeof(*ltr554_nvitem), &file->f_pos );
		if(!ret){
			dev_err(&ltr554->i2c_client->dev, "%s: read file error...\n", __func__);
			goto Exit;
		}

		if(file->f_op->llseek(file, 0, SEEK_SET) != 0) {
			dev_err(&ltr554->i2c_client->dev, "%s: lseek failure...\n", __func__);
			ret = -1;
			goto Exit;
		}

	}

	ltr554_nvitem = (struct ltr554_nvitem_data*)pReadBuff;
	ltr554_nvitem->sensor_part_id = PARTID_V5;
	ltr554_nvitem->sensor_manufac_id = 0x5;
	ltr554_nvitem->threshold_cross_talk= param;

	ret = file->f_op->write(file, pReadBuff, sizeof(*ltr554_nvitem),&file->f_pos);
    if(!ret){
		dev_err(&ltr554->i2c_client->dev,"%s: file write error...\n", __func__);
		ret = -1;
		goto Exit;
    }
Exit:
	/*free buffer and close file*/
	kfree(pReadBuff);
	filp_close(file,NULL);
	/*resume user space*/
	set_fs(old_fs);

	if(!ret){
	dev_err(&ltr554->i2c_client->dev, "%s: file operation error...\n", __func__);
	return -1;
	}

	return count;
}

static DEVICE_ATTR(proximity_crosstalk_data, 0666, ltr554_read_nvitem_show, ltr554_write_nvitem_store);

static ssize_t psabs_distance_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint16_t value;
	int ret;
	uint8_t rdback_val = 0;
	uint16_t abs_distance = 0;
	struct ltr554_data *ltr554 = sensor_info;
	ltr554_wait_for_device_resume(ltr554);
	ret = ps_contr_readback(PS_CONTR_RDBCK, &rdback_val, ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ps_rawdata_show Fail...\n", __func__);
		return (-1);
	}

	ps_contr_setup(rdback_val|PS_MODE_ACTIVE, ltr554);
	msleep(20);

	value = read_ps_adc_value(ltr554);
	if( value > ltr554->default_ps_highthresh ){
		abs_distance = PS_MAX_MEASURE_VAL;
	}
	else if( value < ltr554->default_ps_lowthresh ){
		abs_distance = PS_MIN_MEASURE_VAL;
	}

	ps_contr_setup(rdback_val, ltr554);

	ret = sprintf(buf, "%d\n", abs_distance);

	return ret;
}
static DEVICE_ATTR(proximity_distance_status, 0666, psabs_distance_show, NULL);

static void sysfs_register_device(struct i2c_client *client)
{
	int rc = 0;

	rc += device_create_file(&client->dev, &dev_attr_als_adc);
	rc += device_create_file(&client->dev, &dev_attr_ps_adc);
	//rc += device_create_file(&client->dev, &dev_attr_setwinfac1);
	//rc += device_create_file(&client->dev, &dev_attr_setwinfac2);
	//rc += device_create_file(&client->dev, &dev_attr_setwinfac3);
	rc += device_create_file(&client->dev, &dev_attr_psadcsaturationBit);
	rc += device_create_file(&client->dev, &dev_attr_ltr554help);
	rc += device_create_file(&client->dev, &dev_attr_light_enable);
	rc += device_create_file(&client->dev, &dev_attr_alsswresetsetup);
	rc += device_create_file(&client->dev, &dev_attr_alsgainsetup);
	rc += device_create_file(&client->dev, &dev_attr_alscontrsetup);
	rc += device_create_file(&client->dev, &dev_attr_psmodesetup);
	rc += device_create_file(&client->dev, &dev_attr_psgainsetup);
	rc += device_create_file(&client->dev, &dev_attr_pssatuindicasetup);
	rc += device_create_file(&client->dev, &dev_attr_pscontrsetup);
	rc += device_create_file(&client->dev, &dev_attr_psledcurrsetup);
	rc += device_create_file(&client->dev, &dev_attr_psledcurrduty);
	rc += device_create_file(&client->dev, &dev_attr_psledpulsefreqsetup);
	rc += device_create_file(&client->dev, &dev_attr_psledsetup);
	rc += device_create_file(&client->dev, &dev_attr_psledpulsecountsetup);
	rc += device_create_file(&client->dev, &dev_attr_psmeasratesetup);
	rc += device_create_file(&client->dev, &dev_attr_alsmeasratesetup);
	rc += device_create_file(&client->dev, &dev_attr_alsintegtimesetup);
	rc += device_create_file(&client->dev, &dev_attr_alsmeasrateregsetup);
	rc += device_create_file(&client->dev, &dev_attr_partid);
	rc += device_create_file(&client->dev, &dev_attr_revid);
	rc += device_create_file(&client->dev, &dev_attr_partidreg);
	rc += device_create_file(&client->dev, &dev_attr_manuid);
	rc += device_create_file(&client->dev, &dev_attr_psdatastatus);
	rc += device_create_file(&client->dev, &dev_attr_psinterruptstatus);
	rc += device_create_file(&client->dev, &dev_attr_alsdatastatus);
	rc += device_create_file(&client->dev, &dev_attr_alsinterruptstatus);
	rc += device_create_file(&client->dev, &dev_attr_alsgainstatus);
	rc += device_create_file(&client->dev, &dev_attr_alsdatavaliditystatus);
	rc += device_create_file(&client->dev, &dev_attr_alspsstatusreg);
	rc += device_create_file(&client->dev, &dev_attr_alsch0ch1rawcalc);
	rc += device_create_file(&client->dev, &dev_attr_setpsoffset);
	rc += device_create_file(&client->dev, &dev_attr_proximity_enable);
	rc += device_create_file(&client->dev, &dev_attr_interruptpolarsetup);
	rc += device_create_file(&client->dev, &dev_attr_interruptsetup);
	rc += device_create_file(&client->dev, &dev_attr_interruptpersistsetup);
	rc += device_create_file(&client->dev, &dev_attr_setalslothrerange);
	rc += device_create_file(&client->dev, &dev_attr_setalshithrerange);
	rc += device_create_file(&client->dev, &dev_attr_dispalsthrerange);
	rc += device_create_file(&client->dev, &dev_attr_setpslothrerange);
	rc += device_create_file(&client->dev, &dev_attr_setpshithrerange);
	rc += device_create_file(&client->dev, &dev_attr_disppsthrerange);
	rc += device_create_file(&client->dev, &dev_attr_proximity_selftest);
	rc += device_create_file(&client->dev, &dev_attr_proximity_crosstalk_data);
	rc += device_create_file(&client->dev, &dev_attr_proximity_raw_data);
	rc += device_create_file(&client->dev, &dev_attr_proximity_distance_status);
	rc += device_create_file(&client->dev, &dev_attr_light_luminance_data);
	rc += device_create_file(&client->dev, &dev_attr_light_calibration_data);
	rc += device_create_file(&client->dev, &dev_attr_light_poll_delay);

	if (rc) {
		dev_err(&client->dev, "%s Unable to create sysfs files\n", __func__);
	} else {
		dev_dbg(&client->dev, "%s Created sysfs files\n", __func__);
	}
}

static int als_setup(struct ltr554_data *ltr554)
{
	int ret;

	ltr554->als_input_dev = input_allocate_device();
	if (!ltr554->als_input_dev) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS Input Allocate Device Fail...\n", __func__);
		return -ENOMEM;
	}

    ltr554->als_input_dev->name = "light";
    ltr554->als_input_dev->id.bustype = BUS_I2C;
    ltr554->als_input_dev->dev.parent = &ltr554->i2c_client->dev;

	input_set_capability(ltr554->als_input_dev, EV_REL, REL_MISC);

	ret = input_register_device(ltr554->als_input_dev);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS Register Input Device Fail...\n", __func__);
		goto err_als_register_input_device;
	}

	ret = misc_register(&als_misc);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS Register Misc Device Fail...\n", __func__);
		goto err_als_register_misc_device;
	}

	return ret;

err_als_register_misc_device:
	input_unregister_device(ltr554->als_input_dev);
err_als_register_input_device:
	input_free_device(ltr554->als_input_dev);

	return ret;
}


static int ps_setup(struct ltr554_data *ltr554)
{
	int ret;

	ltr554->ps_input_dev = input_allocate_device();
	if (!ltr554->ps_input_dev) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS Input Allocate Device Fail...\n", __func__);
		return -ENOMEM;
	}
	ltr554->ps_input_dev->name = "proximity";
    ltr554->ps_input_dev->id.bustype = BUS_I2C;
    ltr554->ps_input_dev->dev.parent = &ltr554->i2c_client->dev;

    ret = set_ps_range((uint16_t)PS_MIN_MEASURE_VAL,(uint16_t)ltr554->default_ps_highthresh, LO_N_HI_LIMIT, ltr554);

	set_bit(EV_ABS, ltr554->ps_input_dev->evbit);
	input_set_abs_params(ltr554->ps_input_dev, ABS_DISTANCE, PS_MIN_MEASURE_VAL, PS_MAX_MEASURE_VAL, 0, 0);

	ret = input_register_device(ltr554->ps_input_dev);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS Register Input Device Fail...\n", __func__);
		goto err_ps_register_input_device;
	}

	ret = misc_register(&ps_misc);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS Register Misc Device Fail...\n", __func__);
		goto err_ps_register_misc_device;
	}

	return ret;

err_ps_register_misc_device:
	input_unregister_device(ltr554->ps_input_dev);
err_ps_register_input_device:
	input_free_device(ltr554->ps_input_dev);

	return ret;
}





static int ltr554_setup(struct ltr554_data *ltr554)
{
	int ret = 0;

	/* Reset the devices */
	ret = _ltr554_set_bit(ltr554->i2c_client, SET_BIT, LTR554_ALS_CONTR, ALS_SW_RESET);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS reset fail...\n", __func__);
		goto err_out1;
	}

	ret = _ltr554_set_bit(ltr554->i2c_client, CLR_BIT, LTR554_PS_CONTR, PS_MODE_ACTIVE);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS reset fail...\n", __func__);
		goto err_out1;
	}

	msleep(PON_DELAY);
	dev_dbg(&ltr554->i2c_client->dev, "%s: Reset ltr554 device\n", __func__);

	/* Do another part read to ensure we have exited reset */
	if (_check_part_id(ltr554) < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Part ID Read Fail after reset...\n", __func__);
		goto err_out1;
	}

	/* Set count of measurements outside data range before interrupt is generated */
	ret = _ltr554_set_bit(ltr554->i2c_client, SET_BIT, LTR554_INTERRUPT_PRST, 0x01);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: ALS Set Persist Fail...\n", __func__);
		goto err_out2;
	}

	ret = _ltr554_set_bit(ltr554->i2c_client, SET_BIT, LTR554_INTERRUPT_PRST, 0x10);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev,"%s: PS Set Persist Fail...\n", __func__);
		goto err_out2;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s: Set ltr554 persists\n", __func__);

	/* Enable interrupts on the device and clear only when status is read */
#if ACT_INTERRUPT
	ret = _ltr554_set_bit(ltr554->i2c_client, SET_BIT, LTR554_INTERRUPT, INT_MODE_00);
#else
	ret = _ltr554_set_bit(ltr554->i2c_client, SET_BIT, LTR554_INTERRUPT, INT_MODE_00);
#endif
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Enabled interrupts failed...\n", __func__);
		goto err_out2;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s Enabled interrupt to device\n", __func__);

	/* Turn on ALS and PS */

	ret = als_enable_init(ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s Unable to enable ALS", __func__);
		goto err_out2;
	}
	dev_info(&ltr554->i2c_client->dev, "%s Turned OFF ambient light sensor\n", __func__);

	ret = ps_enable_init(ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s Unable to enable PS", __func__);
		goto err_out2;
	}
	dev_info(&ltr554->i2c_client->dev, "%s Turned OFF proximity sensor\n", __func__);


	ret = ltr554_gpio_irq(ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: GPIO Request Fail...\n", __func__);
		goto err_out1;
	}
	dev_dbg(&ltr554->i2c_client->dev, "%s Requested interrupt\n", __func__);

	return ret;

err_out2:
	free_irq(ltr554->irq, ltr554);

err_out1:
	dev_err(&ltr554->i2c_client->dev, "%s Unable to setup device\n", __func__);

	return ret;
}


static int ltr554_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct ltr554_data *ltr554;

	ltr554 = kzalloc(sizeof(struct ltr554_data), GFP_KERNEL);
	if (!ltr554)
	{
		dev_err(&client->dev, "%s: Mem Alloc Fail...\n", __func__);
		return -ENOMEM;
	}

	/* Global pointer for this device */
	sensor_info = ltr554;

	/* Set initial defaults */
	ltr554->als_enable_flag = 0;
	ltr554->ps_enable_flag = 0;

	ltr554->i2c_client = client;

	i2c_set_clientdata(client, ltr554);


	ltr554->default_ps_lowthresh = PS_DEFAULT_LOW_THRESH;
	ltr554->default_ps_highthresh = PS_DEFAULT_HIGH_THRESH;

	atomic_set(&ltr554->ps_selftest_ongoing, 0);
	atomic_set(&ltr554->ps_selftest_int, 0);
	atomic_set(&ltr554->suspended, 0);

	if (_check_part_id(ltr554) < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Part ID Read Fail...\n", __func__);
		ret = -1;
		goto err_out;
	}
	/* Setup the input subsystem for the ALS */
	ret = als_setup(ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev,"%s: ALS Setup Fail...\n", __func__);
		goto err_out;
	}
	/* Setup the input subsystem for the PS */
	ret = ps_setup(ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: PS Setup Fail...\n", __func__);
		goto err_out;
	}

	/* Setup and configure both the ALS and PS on the ltr554 device */
	ret = ltr554_setup(ltr554);
	if (ret < 0) {
		dev_err(&ltr554->i2c_client->dev, "%s: Setup Fail...\n", __func__);
		goto err_out;
	}
	INIT_DELAYED_WORK(&ltr554->als_dwork, als_polling_work_handler);
	INIT_DELAYED_WORK(&ltr554->dwork, ltr554_ps_als_int_work_handler);
	init_waitqueue_head(&ltr554->ps_selftest_wq);
	/* Register the sysfs files */
	sysfs_register_device(client);

	dev_info(&ltr554->i2c_client->dev, "%s: probe complete\n", __func__);

	return ret;

err_out:
	kfree(ltr554);

	return ret;
}

#ifdef CONFIG_PM
static int ltr554_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct ltr554_data *ltr554 = sensor_info;

	dev_info(&ltr554->i2c_client->dev, "%s ltr554_suspend %d\n", __func__,atomic_read(&ltr554->suspended));
	atomic_set(&ltr554->suspended, 1);

	return 0;
}

static int ltr554_resume(struct i2c_client *client)
{
	struct ltr554_data *ltr554 = sensor_info;

	dev_info(&ltr554->i2c_client->dev, "%s ltr554_resume %d\n", __func__,atomic_read(&ltr554->suspended));
	atomic_set(&ltr554->suspended, 0);
	return 0;
}

#else

#define ltr554_suspend  NULL
#define ltr554_resume   NULL

#endif


static const struct of_device_id betamon_prox_match[] = {
    { .compatible = "liteon,betamon_prox", },
    { },
};

static const struct i2c_device_id ltr554_id[] = {
	{ DEVICE_NAME, 0 },
	{}
};

static struct i2c_driver ltr554_driver = {
	.probe = ltr554_probe,
	.id_table = ltr554_id,
	.suspend = ltr554_suspend,
	.resume = ltr554_resume,
	.driver = {
		.owner = THIS_MODULE,
		.name = DEVICE_NAME,
		.of_match_table = betamon_prox_match,
	},
};


static int __init ltr554_init(void)
{
	ltr554_workqueue = create_workqueue("proximity_als");

	if (!ltr554_workqueue)
		return -ENOMEM;

	return i2c_add_driver(&ltr554_driver);
}

static void __exit ltr554_exit(void)
{
	if (ltr554_workqueue)
		destroy_workqueue(ltr554_workqueue);

	ltr554_workqueue = NULL;
	i2c_del_driver(&ltr554_driver);
}


module_init(ltr554_init)
module_exit(ltr554_exit)

MODULE_AUTHOR("Lite-On Technology Corp");
MODULE_DESCRIPTION("LTR-554ALSPS Driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(DRIVER_VERSION);



