#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <reg24le1.h>
#include <nrfutils.h>

#include "i2c.h"
#include "mpu_regs.h"

/*
// the MPU-6050 registers
#define REG_rate_div       0x19
#define REG_lpf            0x1A
#define REG_prod_id        0x0C
#define REG_user_ctrl      0x6A
#define REG_fifo_en        0x23
#define REG_gyro_cfg       0x1B
#define REG_accel_cfg      0x1C
#define REG_motion_thr     0x1F
#define REG_motion_dur     0x20
#define REG_fifo_count_h   0x72
#define REG_fifo_r_w       0x74
#define REG_raw_gyro       0x43
#define REG_raw_accel      0x3B
#define REG_temp           0x41
#define REG_int_enable     0x38
#define REG_dmp_int_status 0x39
#define REG_int_status     0x3A
#define REG_pwr_mgmt_1     0x6B
#define REG_pwr_mgmt_2     0x6C
#define REG_int_pin_cfg    0x37
#define REG_mem_r_w        0x6F
#define REG_accel_offs     0x06
#define REG_i2c_mst        0x24
#define REG_bank_sel       0x6D
#define REG_mem_start_addr 0x6E
#define REG_prgm_start_h   0x70
*/

#define BIT_I2C_MST_VDDIO   (0x80)
#define BIT_FIFO_EN         (0x40)
#define BIT_DMP_EN          (0x80)
#define BIT_FIFO_RST        (0x04)
#define BIT_DMP_RST         (0x08)
#define BIT_FIFO_OVERFLOW   (0x10)
#define BIT_DATA_RDY_EN     (0x01)
#define BIT_DMP_INT_EN      (0x02)
#define BIT_MOT_INT_EN      (0x40)
#define BITS_FSR            (0x18)
#define BITS_LPF            (0x07)
#define BITS_HPF            (0x07)
#define BITS_CLK            (0x07)
#define BIT_FIFO_SIZE_1024  (0x40)
#define BIT_FIFO_SIZE_2048  (0x80)
#define BIT_FIFO_SIZE_4096  (0xC0)
#define BIT_RESET           (0x80)
#define BIT_SLEEP           (0x40)
#define BIT_S0_DELAY_EN     (0x01)
#define BIT_S2_DELAY_EN     (0x04)
#define BITS_SLAVE_LENGTH   (0x0F)
#define BIT_SLAVE_BYTE_SW   (0x40)
#define BIT_SLAVE_GROUP     (0x10)
#define BIT_SLAVE_EN        (0x80)
#define BIT_I2C_READ        (0x80)
#define BITS_I2C_MASTER_DLY (0x1F)
#define BIT_AUX_IF_EN       (0x20)
#define BIT_ACTL            (0x80)
#define BIT_LATCH_EN        (0x20)
#define BIT_ANY_RD_CLR      (0x10)
#define BIT_BYPASS_EN       (0x02)
#define BITS_WOM_EN         (0xC0)
#define BIT_LPA_CYCLE       (0x20)
#define BIT_STBY_XA         (0x20)
#define BIT_STBY_YA         (0x10)
#define BIT_STBY_ZA         (0x08)
#define BIT_STBY_XG         (0x04)
#define BIT_STBY_YG         (0x02)
#define BIT_STBY_ZG         (0x01)
#define BIT_STBY_XYZA       (BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA)
#define BIT_STBY_XYZG       (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)

bool dmp_on = false;

bool mpu_write_byte(uint8_t reg_addr, uint8_t val)
{
	return i2c_write(reg_addr, 1, &val);
}

/*
uint8_t mpu_read_byte(uint8_t reg_addr, uint8_t* val)
{
	uint8_t result;
	result = i2c_read(reg_addr, 1, val);
	return result ? 0 : 0xff;
}*/

#define FIFO_ENABLE_VAL		0x71
#define PACKET_LENGTH		28
#define MAX_FIFO			1024

bool mpu_init(void)
{
	// reset device
	if (!mpu_write_byte(PWR_MGMT_1, BIT_RESET))
		return false;

	delay_ms(100);

	// wake the chip up
	if (!mpu_write_byte(PWR_MGMT_1, 0x00))
		return false;

	mpu_write_byte(GYRO_CONFIG, 0x18);		// +- 2000deg/sec range
	mpu_write_byte(ACCEL_CONFIG, 0x00);		// +- 2g range
	mpu_write_byte(CONFIG, 3);				// digital lowpass filter 42Hz
	//mpu_write_byte(CONFIG, 2);				// digital lowpass filter gyro=98Hz, accel=94Hz
	mpu_write_byte(SMPLRT_DIV, 4);			// 1000 / 200Hz - 1
	mpu_write_byte(FIFO_EN, FIFO_ENABLE_VAL);	// gyro xyz and accel xyz
	mpu_write_byte(PWR_MGMT_1, 1);			// clock source PLL from X axis gyro 
	
	return true;
}

#define DMP_CODE_SIZE	3062

const uint8_t __code dmp_memory[DMP_CODE_SIZE] =
{
    // bank # 0
    0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x65, 0x00, 0x54, 0xff, 0xef, 0x00, 0x00, 0xfa, 0x80, 0x00, 0x0b, 0x12, 0x82, 0x00, 0x01,
    0x03, 0x0c, 0x30, 0xc3, 0x0e, 0x8c, 0x8c, 0xe9, 0x14, 0xd5, 0x40, 0x02, 0x13, 0x71, 0x0f, 0x8e,
    0x38, 0x83, 0xf8, 0x83, 0x30, 0x00, 0xf8, 0x83, 0x25, 0x8e, 0xf8, 0x83, 0x30, 0x00, 0xf8, 0x83,
    0xff, 0xff, 0xff, 0xff, 0x0f, 0xfe, 0xa9, 0xd6, 0x24, 0x00, 0x04, 0x00, 0x1a, 0x82, 0x79, 0xa1,
    0x00, 0x00, 0x00, 0x3c, 0xff, 0xff, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x38, 0x83, 0x6f, 0xa2,
    0x00, 0x3e, 0x03, 0x30, 0x40, 0x00, 0x00, 0x00, 0x02, 0xca, 0xe3, 0x09, 0x3e, 0x80, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,
    0x00, 0x0c, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x6e, 0x00, 0x00, 0x06, 0x92, 0x0a, 0x16, 0xc0, 0xdf,
    0xff, 0xff, 0x02, 0x56, 0xfd, 0x8c, 0xd3, 0x77, 0xff, 0xe1, 0xc4, 0x96, 0xe0, 0xc5, 0xbe, 0xaa,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x0b, 0x2b, 0x00, 0x00, 0x16, 0x57, 0x00, 0x00, 0x03, 0x59,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0xfa, 0x00, 0x02, 0x6c, 0x1d, 0x00, 0x00, 0x00, 0x00,
    0x3f, 0xff, 0xdf, 0xeb, 0x00, 0x3e, 0xb3, 0xb6, 0x00, 0x0d, 0x22, 0x78, 0x00, 0x00, 0x2f, 0x3c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x42, 0xb5, 0x00, 0x00, 0x39, 0xa2, 0x00, 0x00, 0xb3, 0x65,
    0xd9, 0x0e, 0x9f, 0xc9, 0x1d, 0xcf, 0x4c, 0x34, 0x30, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x3b, 0xb6, 0x7a, 0xe8, 0x00, 0x64, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* bank # 1 */
    0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0xfa, 0x92, 0x10, 0x00, 0x22, 0x5e, 0x00, 0x0d, 0x22, 0x9f,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0xff, 0x46, 0x00, 0x00, 0x63, 0xd4, 0x00, 0x00,
    0x10, 0x00, 0x00, 0x00, 0x04, 0xd6, 0x00, 0x00, 0x04, 0xcc, 0x00, 0x00, 0x04, 0xcc, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x72, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x06, 0x00, 0x02, 0x00, 0x05, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x64, 0x00, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x32, 0xf8, 0x98, 0x00, 0x00, 0xff, 0x65, 0x00, 0x00, 0x83, 0x0f, 0x00, 0x00,
    0xff, 0x9b, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0xb2, 0x6a, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x01, 0xfb, 0x83, 0x00, 0x68, 0x00, 0x00, 0x00, 0xd9, 0xfc, 0x00, 0x7c, 0xf1, 0xff, 0x83,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x64, 0x03, 0xe8, 0x00, 0x64, 0x00, 0x28,
    0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x16, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0x00, 0x00, 0x10, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf4, 0x00, 0x00, 0x10, 0x00,
    /* bank # 2 */
    0x00, 0x28, 0x00, 0x00, 0xff, 0xff, 0x45, 0x81, 0xff, 0xff, 0xfa, 0x72, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x00, 0x05, 0x00, 0x05, 0xba, 0xc6, 0x00, 0x47, 0x78, 0xa2,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x14,
    0x00, 0x00, 0x25, 0x4d, 0x00, 0x2f, 0x70, 0x6d, 0x00, 0x00, 0x05, 0xae, 0x00, 0x0c, 0x02, 0xd0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x64, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x0e,
    0x00, 0x00, 0x0a, 0xc7, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0xff, 0xff, 0xff, 0x9c,
    0x00, 0x00, 0x0b, 0x2b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64,
    0xff, 0xe5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* bank # 3 */
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x24, 0x26, 0xd3,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x10, 0x00, 0x96, 0x00, 0x3c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0c, 0x0a, 0x4e, 0x68, 0xcd, 0xcf, 0x77, 0x09, 0x50, 0x16, 0x67, 0x59, 0xc6, 0x19, 0xce, 0x82,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0xd7, 0x84, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0x93, 0x8f, 0x9d, 0x1e, 0x1b, 0x1c, 0x19,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x18, 0x85, 0x00, 0x00, 0x40, 0x00,
    0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x67, 0x7d, 0xdf, 0x7e, 0x72, 0x90, 0x2e, 0x55, 0x4c, 0xf6, 0xe6, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* bank # 4 */
    0xd8, 0xdc, 0xb4, 0xb8, 0xb0, 0xd8, 0xb9, 0xab, 0xf3, 0xf8, 0xfa, 0xb3, 0xb7, 0xbb, 0x8e, 0x9e,
    0xae, 0xf1, 0x32, 0xf5, 0x1b, 0xf1, 0xb4, 0xb8, 0xb0, 0x80, 0x97, 0xf1, 0xa9, 0xdf, 0xdf, 0xdf,
    0xaa, 0xdf, 0xdf, 0xdf, 0xf2, 0xaa, 0xc5, 0xcd, 0xc7, 0xa9, 0x0c, 0xc9, 0x2c, 0x97, 0xf1, 0xa9,
    0x89, 0x26, 0x46, 0x66, 0xb2, 0x89, 0x99, 0xa9, 0x2d, 0x55, 0x7d, 0xb0, 0xb0, 0x8a, 0xa8, 0x96,
    0x36, 0x56, 0x76, 0xf1, 0xba, 0xa3, 0xb4, 0xb2, 0x80, 0xc0, 0xb8, 0xa8, 0x97, 0x11, 0xb2, 0x83,
    0x98, 0xba, 0xa3, 0xf0, 0x24, 0x08, 0x44, 0x10, 0x64, 0x18, 0xb2, 0xb9, 0xb4, 0x98, 0x83, 0xf1,
    0xa3, 0x29, 0x55, 0x7d, 0xba, 0xb5, 0xb1, 0xa3, 0x83, 0x93, 0xf0, 0x00, 0x28, 0x50, 0xf5, 0xb2,
    0xb6, 0xaa, 0x83, 0x93, 0x28, 0x54, 0x7c, 0xf1, 0xb9, 0xa3, 0x82, 0x93, 0x61, 0xba, 0xa2, 0xda,
    0xde, 0xdf, 0xdb, 0x81, 0x9a, 0xb9, 0xae, 0xf5, 0x60, 0x68, 0x70, 0xf1, 0xda, 0xba, 0xa2, 0xdf,
    0xd9, 0xba, 0xa2, 0xfa, 0xb9, 0xa3, 0x82, 0x92, 0xdb, 0x31, 0xba, 0xa2, 0xd9, 0xba, 0xa2, 0xf8,
    0xdf, 0x85, 0xa4, 0xd0, 0xc1, 0xbb, 0xad, 0x83, 0xc2, 0xc5, 0xc7, 0xb8, 0xa2, 0xdf, 0xdf, 0xdf,
    0xba, 0xa0, 0xdf, 0xdf, 0xdf, 0xd8, 0xd8, 0xf1, 0xb8, 0xaa, 0xb3, 0x8d, 0xb4, 0x98, 0x0d, 0x35,
    0x5d, 0xb2, 0xb6, 0xba, 0xaf, 0x8c, 0x96, 0x19, 0x8f, 0x9f, 0xa7, 0x0e, 0x16, 0x1e, 0xb4, 0x9a,
    0xb8, 0xaa, 0x87, 0x2c, 0x54, 0x7c, 0xba, 0xa4, 0xb0, 0x8a, 0xb6, 0x91, 0x32, 0x56, 0x76, 0xb2,
    0x84, 0x94, 0xa4, 0xc8, 0x08, 0xcd, 0xd8, 0xb8, 0xb4, 0xb0, 0xf1, 0x99, 0x82, 0xa8, 0x2d, 0x55,
    0x7d, 0x98, 0xa8, 0x0e, 0x16, 0x1e, 0xa2, 0x2c, 0x54, 0x7c, 0x92, 0xa4, 0xf0, 0x2c, 0x50, 0x78,
    /* bank # 5 */
    0xf1, 0x84, 0xa8, 0x98, 0xc4, 0xcd, 0xfc, 0xd8, 0x0d, 0xdb, 0xa8, 0xfc, 0x2d, 0xf3, 0xd9, 0xba,
    0xa6, 0xf8, 0xda, 0xba, 0xa6, 0xde, 0xd8, 0xba, 0xb2, 0xb6, 0x86, 0x96, 0xa6, 0xd0, 0xf3, 0xc8,
    0x41, 0xda, 0xa6, 0xc8, 0xf8, 0xd8, 0xb0, 0xb4, 0xb8, 0x82, 0xa8, 0x92, 0xf5, 0x2c, 0x54, 0x88,
    0x98, 0xf1, 0x35, 0xd9, 0xf4, 0x18, 0xd8, 0xf1, 0xa2, 0xd0, 0xf8, 0xf9, 0xa8, 0x84, 0xd9, 0xc7,
    0xdf, 0xf8, 0xf8, 0x83, 0xc5, 0xda, 0xdf, 0x69, 0xdf, 0x83, 0xc1, 0xd8, 0xf4, 0x01, 0x14, 0xf1,
    0xa8, 0x82, 0x4e, 0xa8, 0x84, 0xf3, 0x11, 0xd1, 0x82, 0xf5, 0xd9, 0x92, 0x28, 0x97, 0x88, 0xf1,
    0x09, 0xf4, 0x1c, 0x1c, 0xd8, 0x84, 0xa8, 0xf3, 0xc0, 0xf9, 0xd1, 0xd9, 0x97, 0x82, 0xf1, 0x29,
    0xf4, 0x0d, 0xd8, 0xf3, 0xf9, 0xf9, 0xd1, 0xd9, 0x82, 0xf4, 0xc2, 0x03, 0xd8, 0xde, 0xdf, 0x1a,
    0xd8, 0xf1, 0xa2, 0xfa, 0xf9, 0xa8, 0x84, 0x98, 0xd9, 0xc7, 0xdf, 0xf8, 0xf8, 0xf8, 0x83, 0xc7,
    0xda, 0xdf, 0x69, 0xdf, 0xf8, 0x83, 0xc3, 0xd8, 0xf4, 0x01, 0x14, 0xf1, 0x98, 0xa8, 0x82, 0x2e,
    0xa8, 0x84, 0xf3, 0x11, 0xd1, 0x82, 0xf5, 0xd9, 0x92, 0x50, 0x97, 0x88, 0xf1, 0x09, 0xf4, 0x1c,
    0xd8, 0x84, 0xa8, 0xf3, 0xc0, 0xf8, 0xf9, 0xd1, 0xd9, 0x97, 0x82, 0xf1, 0x49, 0xf4, 0x0d, 0xd8,
    0xf3, 0xf9, 0xf9, 0xd1, 0xd9, 0x82, 0xf4, 0xc4, 0x03, 0xd8, 0xde, 0xdf, 0xd8, 0xf1, 0xad, 0x88,
    0x98, 0xcc, 0xa8, 0x09, 0xf9, 0xd9, 0x82, 0x92, 0xa8, 0xf5, 0x7c, 0xf1, 0x88, 0x3a, 0xcf, 0x94,
    0x4a, 0x6e, 0x98, 0xdb, 0x69, 0x31, 0xda, 0xad, 0xf2, 0xde, 0xf9, 0xd8, 0x87, 0x95, 0xa8, 0xf2,
    0x21, 0xd1, 0xda, 0xa5, 0xf9, 0xf4, 0x17, 0xd9, 0xf1, 0xae, 0x8e, 0xd0, 0xc0, 0xc3, 0xae, 0x82,
    /* bank # 6 */
    0xc6, 0x84, 0xc3, 0xa8, 0x85, 0x95, 0xc8, 0xa5, 0x88, 0xf2, 0xc0, 0xf1, 0xf4, 0x01, 0x0e, 0xf1,
    0x8e, 0x9e, 0xa8, 0xc6, 0x3e, 0x56, 0xf5, 0x54, 0xf1, 0x88, 0x72, 0xf4, 0x01, 0x15, 0xf1, 0x98,
    0x45, 0x85, 0x6e, 0xf5, 0x8e, 0x9e, 0x04, 0x88, 0xf1, 0x42, 0x98, 0x5a, 0x8e, 0x9e, 0x06, 0x88,
    0x69, 0xf4, 0x01, 0x1c, 0xf1, 0x98, 0x1e, 0x11, 0x08, 0xd0, 0xf5, 0x04, 0xf1, 0x1e, 0x97, 0x02,
    0x02, 0x98, 0x36, 0x25, 0xdb, 0xf9, 0xd9, 0x85, 0xa5, 0xf3, 0xc1, 0xda, 0x85, 0xa5, 0xf3, 0xdf,
    0xd8, 0x85, 0x95, 0xa8, 0xf3, 0x09, 0xda, 0xa5, 0xfa, 0xd8, 0x82, 0x92, 0xa8, 0xf5, 0x78, 0xf1,
    0x88, 0x1a, 0x84, 0x9f, 0x26, 0x88, 0x98, 0x21, 0xda, 0xf4, 0x1d, 0xf3, 0xd8, 0x87, 0x9f, 0x39,
    0xd1, 0xaf, 0xd9, 0xdf, 0xdf, 0xfb, 0xf9, 0xf4, 0x0c, 0xf3, 0xd8, 0xfa, 0xd0, 0xf8, 0xda, 0xf9,
    0xf9, 0xd0, 0xdf, 0xd9, 0xf9, 0xd8, 0xf4, 0x0b, 0xd8, 0xf3, 0x87, 0x9f, 0x39, 0xd1, 0xaf, 0xd9,
    0xdf, 0xdf, 0xf4, 0x1d, 0xf3, 0xd8, 0xfa, 0xfc, 0xa8, 0x69, 0xf9, 0xf9, 0xaf, 0xd0, 0xda, 0xde,
    0xfa, 0xd9, 0xf8, 0x8f, 0x9f, 0xa8, 0xf1, 0xcc, 0xf3, 0x98, 0xdb, 0x45, 0xd9, 0xaf, 0xdf, 0xd0,
    0xf8, 0xd8, 0xf1, 0x8f, 0x9f, 0xa8, 0xca, 0xf3, 0x88, 0x09, 0xda, 0xaf, 0x8f, 0xcb, 0xf8, 0xd8,
    0xf2, 0xad, 0x97, 0x8d, 0x0c, 0xd9, 0xa5, 0xdf, 0xf9, 0xba, 0xa6, 0xf3, 0xfa, 0xf4, 0x12, 0xf2,
    0xd8, 0x95, 0x0d, 0xd1, 0xd9, 0xba, 0xa6, 0xf3, 0xfa, 0xda, 0xa5, 0xf2, 0xc1, 0xba, 0xa6, 0xf3,
    0xdf, 0xd8, 0xf1, 0xba, 0xb2, 0xb6, 0x86, 0x96, 0xa6, 0xd0, 0xca, 0xf3, 0x49, 0xda, 0xa6, 0xcb,
    0xf8, 0xd8, 0xb0, 0xb4, 0xb8, 0xd8, 0xad, 0x84, 0xf2, 0xc0, 0xdf, 0xf1, 0x8f, 0xcb, 0xc3, 0xa8,
    /* bank # 7 */
    0xb2, 0xb6, 0x86, 0x96, 0xc8, 0xc1, 0xcb, 0xc3, 0xf3, 0xb0, 0xb4, 0x88, 0x98, 0xa8, 0x21, 0xdb,
    0x71, 0x8d, 0x9d, 0x71, 0x85, 0x95, 0x21, 0xd9, 0xad, 0xf2, 0xfa, 0xd8, 0x85, 0x97, 0xa8, 0x28,
    0xd9, 0xf4, 0x08, 0xd8, 0xf2, 0x8d, 0x29, 0xda, 0xf4, 0x05, 0xd9, 0xf2, 0x85, 0xa4, 0xc2, 0xf2,
    0xd8, 0xa8, 0x8d, 0x94, 0x01, 0xd1, 0xd9, 0xf4, 0x11, 0xf2, 0xd8, 0x87, 0x21, 0xd8, 0xf4, 0x0a,
    0xd8, 0xf2, 0x84, 0x98, 0xa8, 0xc8, 0x01, 0xd1, 0xd9, 0xf4, 0x11, 0xd8, 0xf3, 0xa4, 0xc8, 0xbb,
    0xaf, 0xd0, 0xf2, 0xde, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xd8, 0xf1, 0xb8, 0xf6,
    0xb5, 0xb9, 0xb0, 0x8a, 0x95, 0xa3, 0xde, 0x3c, 0xa3, 0xd9, 0xf8, 0xd8, 0x5c, 0xa3, 0xd9, 0xf8,
    0xd8, 0x7c, 0xa3, 0xd9, 0xf8, 0xd8, 0xf8, 0xf9, 0xd1, 0xa5, 0xd9, 0xdf, 0xda, 0xfa, 0xd8, 0xb1,
    0x85, 0x30, 0xf7, 0xd9, 0xde, 0xd8, 0xf8, 0x30, 0xad, 0xda, 0xde, 0xd8, 0xf2, 0xb4, 0x8c, 0x99,
    0xa3, 0x2d, 0x55, 0x7d, 0xa0, 0x83, 0xdf, 0xdf, 0xdf, 0xb5, 0x91, 0xa0, 0xf6, 0x29, 0xd9, 0xfb,
    0xd8, 0xa0, 0xfc, 0x29, 0xd9, 0xfa, 0xd8, 0xa0, 0xd0, 0x51, 0xd9, 0xf8, 0xd8, 0xfc, 0x51, 0xd9,
    0xf9, 0xd8, 0x79, 0xd9, 0xfb, 0xd8, 0xa0, 0xd0, 0xfc, 0x79, 0xd9, 0xfa, 0xd8, 0xa1, 0xf9, 0xf9,
    0xf9, 0xf9, 0xf9, 0xa0, 0xda, 0xdf, 0xdf, 0xdf, 0xd8, 0xa1, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xac,
    0xde, 0xf8, 0xad, 0xde, 0x83, 0x93, 0xac, 0x2c, 0x54, 0x7c, 0xf1, 0xa8, 0xdf, 0xdf, 0xdf, 0xf6,
    0x9d, 0x2c, 0xda, 0xa0, 0xdf, 0xd9, 0xfa, 0xdb, 0x2d, 0xf8, 0xd8, 0xa8, 0x50, 0xda, 0xa0, 0xd0,
    0xde, 0xd9, 0xd0, 0xf8, 0xf8, 0xf8, 0xdb, 0x55, 0xf8, 0xd8, 0xa8, 0x78, 0xda, 0xa0, 0xd0, 0xdf,
    /* bank # 8 */
    0xd9, 0xd0, 0xfa, 0xf8, 0xf8, 0xf8, 0xf8, 0xdb, 0x7d, 0xf8, 0xd8, 0x9c, 0xa8, 0x8c, 0xf5, 0x30,
    0xdb, 0x38, 0xd9, 0xd0, 0xde, 0xdf, 0xa0, 0xd0, 0xde, 0xdf, 0xd8, 0xa8, 0x48, 0xdb, 0x58, 0xd9,
    0xdf, 0xd0, 0xde, 0xa0, 0xdf, 0xd0, 0xde, 0xd8, 0xa8, 0x68, 0xdb, 0x70, 0xd9, 0xdf, 0xdf, 0xa0,
    0xdf, 0xdf, 0xd8, 0xf1, 0xa8, 0x88, 0x90, 0x2c, 0x54, 0x7c, 0x98, 0xa8, 0xd0, 0x5c, 0x38, 0xd1,
    0xda, 0xf2, 0xae, 0x8c, 0xdf, 0xf9, 0xd8, 0xb0, 0x87, 0xa8, 0xc1, 0xc1, 0xb1, 0x88, 0xa8, 0xc6,
    0xf9, 0xf9, 0xda, 0x36, 0xd8, 0xa8, 0xf9, 0xda, 0x36, 0xd8, 0xa8, 0xf9, 0xda, 0x36, 0xd8, 0xa8,
    0xf9, 0xda, 0x36, 0xd8, 0xa8, 0xf9, 0xda, 0x36, 0xd8, 0xf7, 0x8d, 0x9d, 0xad, 0xf8, 0x18, 0xda,
    0xf2, 0xae, 0xdf, 0xd8, 0xf7, 0xad, 0xfa, 0x30, 0xd9, 0xa4, 0xde, 0xf9, 0xd8, 0xf2, 0xae, 0xde,
    0xfa, 0xf9, 0x83, 0xa7, 0xd9, 0xc3, 0xc5, 0xc7, 0xf1, 0x88, 0x9b, 0xa7, 0x7a, 0xad, 0xf7, 0xde,
    0xdf, 0xa4, 0xf8, 0x84, 0x94, 0x08, 0xa7, 0x97, 0xf3, 0x00, 0xae, 0xf2, 0x98, 0x19, 0xa4, 0x88,
    0xc6, 0xa3, 0x94, 0x88, 0xf6, 0x32, 0xdf, 0xf2, 0x83, 0x93, 0xdb, 0x09, 0xd9, 0xf2, 0xaa, 0xdf,
    0xd8, 0xd8, 0xae, 0xf8, 0xf9, 0xd1, 0xda, 0xf3, 0xa4, 0xde, 0xa7, 0xf1, 0x88, 0x9b, 0x7a, 0xd8,
    0xf3, 0x84, 0x94, 0xae, 0x19, 0xf9, 0xda, 0xaa, 0xf1, 0xdf, 0xd8, 0xa8, 0x81, 0xc0, 0xc3, 0xc5,
    0xc7, 0xa3, 0x92, 0x83, 0xf6, 0x28, 0xad, 0xde, 0xd9, 0xf8, 0xd8, 0xa3, 0x50, 0xad, 0xd9, 0xf8,
    0xd8, 0xa3, 0x78, 0xad, 0xd9, 0xf8, 0xd8, 0xf8, 0xf9, 0xd1, 0xa1, 0xda, 0xde, 0xc3, 0xc5, 0xc7,
    0xd8, 0xa1, 0x81, 0x94, 0xf8, 0x18, 0xf2, 0xb0, 0x89, 0xac, 0xc3, 0xc5, 0xc7, 0xf1, 0xd8, 0xb8,
    /* bank # 9 */
    0xb4, 0xb0, 0x97, 0x86, 0xa8, 0x31, 0x9b, 0x06, 0x99, 0x07, 0xab, 0x97, 0x28, 0x88, 0x9b, 0xf0,
    0x0c, 0x20, 0x14, 0x40, 0xb0, 0xb4, 0xb8, 0xf0, 0xa8, 0x8a, 0x9a, 0x28, 0x50, 0x78, 0xb7, 0x9b,
    0xa8, 0x29, 0x51, 0x79, 0x24, 0x70, 0x59, 0x44, 0x69, 0x38, 0x64, 0x48, 0x31, 0xf1, 0xbb, 0xab,
    0x88, 0x00, 0x2c, 0x54, 0x7c, 0xf0, 0xb3, 0x8b, 0xb8, 0xa8, 0x04, 0x28, 0x50, 0x78, 0xf1, 0xb0,
    0x88, 0xb4, 0x97, 0x26, 0xa8, 0x59, 0x98, 0xbb, 0xab, 0xb3, 0x8b, 0x02, 0x26, 0x46, 0x66, 0xb0,
    0xb8, 0xf0, 0x8a, 0x9c, 0xa8, 0x29, 0x51, 0x79, 0x8b, 0x29, 0x51, 0x79, 0x8a, 0x24, 0x70, 0x59,
    0x8b, 0x20, 0x58, 0x71, 0x8a, 0x44, 0x69, 0x38, 0x8b, 0x39, 0x40, 0x68, 0x8a, 0x64, 0x48, 0x31,
    0x8b, 0x30, 0x49, 0x60, 0x88, 0xf1, 0xac, 0x00, 0x2c, 0x54, 0x7c, 0xf0, 0x8c, 0xa8, 0x04, 0x28,
    0x50, 0x78, 0xf1, 0x88, 0x97, 0x26, 0xa8, 0x59, 0x98, 0xac, 0x8c, 0x02, 0x26, 0x46, 0x66, 0xf0,
    0x89, 0x9c, 0xa8, 0x29, 0x51, 0x79, 0x24, 0x70, 0x59, 0x44, 0x69, 0x38, 0x64, 0x48, 0x31, 0xa9,
    0x88, 0x09, 0x20, 0x59, 0x70, 0xab, 0x11, 0x38, 0x40, 0x69, 0xa8, 0x19, 0x31, 0x48, 0x60, 0x8c,
    0xa8, 0x3c, 0x41, 0x5c, 0x20, 0x7c, 0x00, 0xf1, 0x87, 0x98, 0x19, 0x86, 0xa8, 0x6e, 0x76, 0x7e,
    0xa9, 0x99, 0x88, 0x2d, 0x55, 0x7d, 0xd8, 0xb1, 0xb5, 0xb9, 0xa3, 0xdf, 0xdf, 0xdf, 0xae, 0xd0,
    0xdf, 0xaa, 0xd0, 0xde, 0xf2, 0xab, 0xf8, 0xf9, 0xd9, 0xb0, 0x87, 0xc4, 0xaa, 0xf1, 0xdf, 0xdf,
    0xbb, 0xaf, 0xdf, 0xdf, 0xb9, 0xd8, 0xb1, 0xf1, 0xa3, 0x97, 0x8e, 0x60, 0xdf, 0xb0, 0x84, 0xf2,
    0xc8, 0xf8, 0xf9, 0xd9, 0xde, 0xd8, 0x93, 0x85, 0xf1, 0x4a, 0xb1, 0x83, 0xa3, 0x08, 0xb5, 0x83,
    /* bank # 10 */
    0x9a, 0x08, 0x10, 0xb7, 0x9f, 0x10, 0xd8, 0xf1, 0xb0, 0xba, 0xae, 0xb0, 0x8a, 0xc2, 0xb2, 0xb6,
    0x8e, 0x9e, 0xf1, 0xfb, 0xd9, 0xf4, 0x1d, 0xd8, 0xf9, 0xd9, 0x0c, 0xf1, 0xd8, 0xf8, 0xf8, 0xad,
    0x61, 0xd9, 0xae, 0xfb, 0xd8, 0xf4, 0x0c, 0xf1, 0xd8, 0xf8, 0xf8, 0xad, 0x19, 0xd9, 0xae, 0xfb,
    0xdf, 0xd8, 0xf4, 0x16, 0xf1, 0xd8, 0xf8, 0xad, 0x8d, 0x61, 0xd9, 0xf4, 0xf4, 0xac, 0xf5, 0x9c,
    0x9c, 0x8d, 0xdf, 0x2b, 0xba, 0xb6, 0xae, 0xfa, 0xf8, 0xf4, 0x0b, 0xd8, 0xf1, 0xae, 0xd0, 0xf8,
    0xad, 0x51, 0xda, 0xae, 0xfa, 0xf8, 0xf1, 0xd8, 0xb9, 0xb1, 0xb6, 0xa3, 0x83, 0x9c, 0x08, 0xb9,
    0xb1, 0x83, 0x9a, 0xb5, 0xaa, 0xc0, 0xfd, 0x30, 0x83, 0xb7, 0x9f, 0x10, 0xb5, 0x8b, 0x93, 0xf2,
    0x02, 0x02, 0xd1, 0xab, 0xda, 0xde, 0xd8, 0xf1, 0xb0, 0x80, 0xba, 0xab, 0xc0, 0xc3, 0xb2, 0x84,
    0xc1, 0xc3, 0xd8, 0xb1, 0xb9, 0xf3, 0x8b, 0xa3, 0x91, 0xb6, 0x09, 0xb4, 0xd9, 0xab, 0xde, 0xb0,
    0x87, 0x9c, 0xb9, 0xa3, 0xdd, 0xf1, 0xb3, 0x8b, 0x8b, 0x8b, 0x8b, 0x8b, 0xb0, 0x87, 0xa3, 0xa3,
    0xa3, 0xa3, 0xb2, 0x8b, 0xb6, 0x9b, 0xf2, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3,
    0xa3, 0xf1, 0xb0, 0x87, 0xb5, 0x9a, 0xa3, 0xf3, 0x9b, 0xa3, 0xa3, 0xdc, 0xba, 0xac, 0xdf, 0xb9,
    0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3, 0xa3,
    0xd8, 0xd8, 0xd8, 0xbb, 0xb3, 0xb7, 0xf1, 0xaa, 0xf9, 0xda, 0xff, 0xd9, 0x80, 0x9a, 0xaa, 0x28,
    0xb4, 0x80, 0x98, 0xa7, 0x20, 0xb7, 0x97, 0x87, 0xa8, 0x66, 0x88, 0xf0, 0x79, 0x51, 0xf1, 0x90,
    0x2c, 0x87, 0x0c, 0xa7, 0x81, 0x97, 0x62, 0x93, 0xf0, 0x71, 0x71, 0x60, 0x85, 0x94, 0x01, 0x29,
    /* bank # 11 */
    0x51, 0x79, 0x90, 0xa5, 0xf1, 0x28, 0x4c, 0x6c, 0x87, 0x0c, 0x95, 0x18, 0x85, 0x78, 0xa3, 0x83,
    0x90, 0x28, 0x4c, 0x6c, 0x88, 0x6c, 0xd8, 0xf3, 0xa2, 0x82, 0x00, 0xf2, 0x10, 0xa8, 0x92, 0x19,
    0x80, 0xa2, 0xf2, 0xd9, 0x26, 0xd8, 0xf1, 0x88, 0xa8, 0x4d, 0xd9, 0x48, 0xd8, 0x96, 0xa8, 0x39,
    0x80, 0xd9, 0x3c, 0xd8, 0x95, 0x80, 0xa8, 0x39, 0xa6, 0x86, 0x98, 0xd9, 0x2c, 0xda, 0x87, 0xa7,
    0x2c, 0xd8, 0xa8, 0x89, 0x95, 0x19, 0xa9, 0x80, 0xd9, 0x38, 0xd8, 0xa8, 0x89, 0x39, 0xa9, 0x80,
    0xda, 0x3c, 0xd8, 0xa8, 0x2e, 0xa8, 0x39, 0x90, 0xd9, 0x0c, 0xd8, 0xa8, 0x95, 0x31, 0x98, 0xd9,
    0x0c, 0xd8, 0xa8, 0x09, 0xd9, 0xff, 0xd8, 0x01, 0xda, 0xff, 0xd8, 0x95, 0x39, 0xa9, 0xda, 0x26,
    0xff, 0xd8, 0x90, 0xa8, 0x0d, 0x89, 0x99, 0xa8, 0x10, 0x80, 0x98, 0x21, 0xda, 0x2e, 0xd8, 0x89,
    0x99, 0xa8, 0x31, 0x80, 0xda, 0x2e, 0xd8, 0xa8, 0x86, 0x96, 0x31, 0x80, 0xda, 0x2e, 0xd8, 0xa8,
    0x87, 0x31, 0x80, 0xda, 0x2e, 0xd8, 0xa8, 0x82, 0x92, 0xf3, 0x41, 0x80, 0xf1, 0xd9, 0x2e, 0xd8,
    0xa8, 0x82, 0xf3, 0x19, 0x80, 0xf1, 0xd9, 0x2e, 0xd8, 0x82, 0xac, 0xf3, 0xc0, 0xa2, 0x80, 0x22,
    0xf1, 0xa6, 0x2e, 0xa7, 0x2e, 0xa9, 0x22, 0x98, 0xa8, 0x29, 0xda, 0xac, 0xde, 0xff, 0xd8, 0xa2,
    0xf2, 0x2a, 0xf1, 0xa9, 0x2e, 0x82, 0x92, 0xa8, 0xf2, 0x31, 0x80, 0xa6, 0x96, 0xf1, 0xd9, 0x00,
    0xac, 0x8c, 0x9c, 0x0c, 0x30, 0xac, 0xde, 0xd0, 0xde, 0xff, 0xd8, 0x8c, 0x9c, 0xac, 0xd0, 0x10,
    0xac, 0xde, 0x80, 0x92, 0xa2, 0xf2, 0x4c, 0x82, 0xa8, 0xf1, 0xca, 0xf2, 0x35, 0xf1, 0x96, 0x88,
    0xa6, 0xd9, 0x00, 0xd8, 0xf1, 0xff
};

bool mpu_write_mem(uint16_t mem_addr, uint16_t length, const uint8_t* data2write)
{
    uint8_t tmp[2];

    tmp[0] = (uint8_t)(mem_addr >> 8);
    tmp[1] = (uint8_t)(mem_addr & 0xFF);

    if (!i2c_write(BANK_SEL, 2, tmp))
        return false;
		
    if (!i2c_write(MEM_R_W, length, data2write))
        return false;

    return true;
}

bool mpu_read_mem(uint16_t mem_addr, uint16_t length, uint8_t* data2read)
{
    uint8_t tmp[2];

    tmp[0] = (uint8_t)(mem_addr >> 8);
    tmp[1] = (uint8_t)(mem_addr & 0xFF);

    if (!i2c_write(BANK_SEL, 2, tmp))
        return false;

    if (!i2c_read(MEM_R_W, length, data2read))
        return false;

    return true;
}

bool dmp_load_firmware(void)
{
#define LOAD_CHUNK  	16
#define START_ADDR		0x0400
#define MAX_RETRY		5

    uint16_t ii, this_write;
	
    uint8_t cur[LOAD_CHUNK], tmp[2];

    for (ii = 0; ii < DMP_CODE_SIZE; ii += this_write)
	{
        this_write = DMP_CODE_SIZE - ii;
		if (this_write > LOAD_CHUNK)
			this_write = LOAD_CHUNK;

		if (!mpu_write_mem(ii, this_write, dmp_memory + ii))
			return false;

		if (!mpu_read_mem(ii, this_write, cur))
			return false;

        if (memcmp(dmp_memory + ii, cur, this_write))
            return false;
    }
	
    // Set program start address. 
    tmp[0] = START_ADDR >> 8;
    tmp[1] = START_ADDR & 0xFF;
    if (!i2c_write(PRGM_START_H, 2, tmp))
        return false;

    return true;
}

#define ORIENTATION_0	0b10001000		// Z Up X Forward
#define ORIENTATION_1	0b10000101		// X right
#define ORIENTATION_2	0b10101100		// X Back
#define ORIENTATION_3	0b10100001		// X Left

#define ORIENTATION		ORIENTATION_1

#define FCFG_3			1088
#define FCFG_2			1066
#define FCFG_1			1062
#define FCFG_7			1073

bool dmp_set_orientation(void)
{
	uint8_t gyro_regs[3], accel_regs[3];
	const uint8_t __code gyro_axes[3] = {0x4C, 0xCD, 0x6C};
	const uint8_t __code accel_axes[3] = {0x0C, 0xC9, 0x2C};
	const uint8_t __code gyro_sign[3] = {0x36, 0x56, 0x76};
	const uint8_t __code accel_sign[3] = {0x26, 0x46, 0x66};

	gyro_regs[0] = gyro_axes[ORIENTATION & 3];
	gyro_regs[1] = gyro_axes[(ORIENTATION >> 3) & 3];
	gyro_regs[2] = gyro_axes[(ORIENTATION >> 6) & 3];
	accel_regs[0] = accel_axes[ORIENTATION & 3];
	accel_regs[1] = accel_axes[(ORIENTATION >> 3) & 3];
	accel_regs[2] = accel_axes[(ORIENTATION >> 6) & 3];

	// Chip-to-body, axes only.
	if (!mpu_write_mem(FCFG_1, 3, gyro_regs))
		return false;

	if (!mpu_write_mem(FCFG_2, 3, accel_regs))
		return false;

	memcpy(gyro_regs, gyro_sign, 3);
	memcpy(accel_regs, accel_sign, 3);

	//if (ORIENTATION & 4)
	//{
		gyro_regs[0] |= 1;
		accel_regs[0] |= 1;
	//}

	//if (ORIENTATION & 0x20)
	//{
	//	gyro_regs[1] |= 1;
	//	accel_regs[1] |= 1;
	//}

	//if (ORIENTATION & 0x100)
	//{
	//	gyro_regs[2] |= 1;
	//	accel_regs[2] |= 1;
	//}

	// Chip-to-body, sign only
	if (!mpu_write_mem(FCFG_3, 3, gyro_regs))
		return false;

	if (!mpu_write_mem(FCFG_7, 3, accel_regs))
		return false;

	return true;
}

bool mpu_reset_fifo(void)
{
	if (!mpu_write_byte(INT_ENABLE, 0))
		return false;

	if (!mpu_write_byte(FIFO_EN, 0))
		return false;

	if (!mpu_write_byte(USER_CTRL, 0))
		return false;

	//if (dmp_on)
	//{
		if (!mpu_write_byte(USER_CTRL, BIT_FIFO_RST | BIT_DMP_RST))
			return false;
		
		delay_ms(50);
		
		if (!mpu_write_byte(USER_CTRL, BIT_DMP_EN | BIT_FIFO_EN))
			return false;
		
		if (!mpu_write_byte(INT_ENABLE, BIT_DMP_INT_EN))
			return false;
			
		if (!mpu_write_byte(FIFO_EN, 0))
			return false;
	//} else {
	//	if (!mpu_write_byte(USER_CTRL, BIT_FIFO_RST))
	//		return false;
    //
	//	if (!mpu_write_byte(USER_CTRL, BIT_FIFO_EN | BIT_AUX_IF_EN))
	//		return false;
	//		
	//	delay_ms(50);
	//	
	//	if (!mpu_write_byte(INT_ENABLE, BIT_DATA_RDY_EN))
	//		return false;
    //
	//	if (!mpu_write_byte(FIFO_EN, FIFO_ENABLE_VAL))
	//		return false;
	//}

	return true;
}

bool mpu_set_gyro_bias_reg(long* gyro_bias)
{
	uint8_t data[6] = {0, 0, 0, 0, 0, 0};
	uint8_t i;
	for (i = 0; i < 3; i++)
		gyro_bias[i] = -gyro_bias[i];

	data[0] = (gyro_bias[0] >> 8) & 0xff;
	data[1] = (gyro_bias[0]) & 0xff;
	data[2] = (gyro_bias[1] >> 8) & 0xff;
	data[3] = (gyro_bias[1]) & 0xff;
	data[4] = (gyro_bias[2] >> 8) & 0xff;
	data[5] = (gyro_bias[2]) & 0xff;

	if (!i2c_write(0x13, 2, &data[0]))
		return false;
	if (!i2c_write(0x15, 2, &data[2]))
		return false;
	if (!i2c_write(0x17, 2, &data[4]))
		return false;

	return true;
}

bool mpu_read_6050_accel_bias(long* accel_bias)
{
	uint8_t data[6];
	if (!i2c_read(0x06, 2, &data[0]))
		return false;
	if (!i2c_read(0x08, 2, &data[2]))
		return false;
	if (!i2c_read(0x0A, 2, &data[4]))
		return false;
		
	accel_bias[0] = ((long)data[0] << 8) | data[1];
	accel_bias[1] = ((long)data[2] << 8) | data[3];
	accel_bias[2] = ((long)data[4] << 8) | data[5];

	return true;
}

void mpu_set_accel_bias_6050_reg(const long *accel_bias, uint8_t relative)
{
	uint8_t data[2] = {0, 0};
	long accel_reg_bias[3] = {0, 0, 0};
	long mask = 0x0001;
	uint8_t mask_bit[3] = {0, 0, 0};
	uint8_t i;

	mpu_read_6050_accel_bias(accel_reg_bias);

	// bit 0 of the 2 byte bias is for temp comp
	// calculations need to compensate for this and not change it
	for (i = 0; i < 3; i++) 
	{
		if (accel_reg_bias[i] & mask)
			mask_bit[i] = 0x01;

		if (relative == 1)
			accel_reg_bias[i] -= accel_bias[i];
		else	// just dump the value in
			accel_reg_bias[i] = accel_bias[i];

		data[0] = (accel_reg_bias[i] >> 8) & 0xff;
		data[1] = (accel_reg_bias[i]) & 0xff;
		data[1] = data[1] | mask_bit[i];
		
		i2c_write(0x06 + i * 2, 2, data);
	}
}

bool start_dmp_state(void)
{
	// Remove FIFO elements.
	return mpu_write_byte(0x23, 0)  &&  mpu_reset_fifo();
}

#define DMP_SAMPLE_RATE		200
//#define GYRO_SF				(46850825LL * 200 / DMP_SAMPLE_RATE)

#define DMP_FEATURE_TAP					0x001
#define DMP_FEATURE_ANDROID_ORIENT		0x002
#define DMP_FEATURE_LP_QUAT				0x004
#define DMP_FEATURE_PEDOMETER			0x008
#define DMP_FEATURE_6X_LP_QUAT			0x010
#define DMP_FEATURE_GYRO_CAL			0x020
#define DMP_FEATURE_SEND_RAW_ACCEL		0x040
#define DMP_FEATURE_SEND_RAW_GYRO		0x080
#define DMP_FEATURE_SEND_CAL_GYRO		0x100
#define DMP_FEATURE_SEND_ANY_GYRO		(DMP_FEATURE_SEND_RAW_GYRO | DMP_FEATURE_SEND_CAL_GYRO)

#define GYRO_SF		0x02CAE309

#define D_0_104					104
#define D_0_22					(22+512)
#define CFG_6                   2753
#define CFG_8                   2718
#define CFG_15					2727
#define CFG_20					2224
#define CFG_27					2742
#define CFG_MOTION_BIAS			1208
#define CFG_ANDROID_ORIENT_INT	1853
#define CFG_LP_QUAT				2712
#define CFG_GYRO_RAW_DATA		2722

bool dmp_enable_gyro_cal(void)
{
    //if (enable) {
        uint8_t __code regs[9] = {0xb8, 0xaa, 0xb3, 0x8d, 0xb4, 0x98, 0x0d, 0x35, 0x5d};
        return mpu_write_mem(CFG_MOTION_BIAS, 9, regs);
    //} else {
    //    uint8_t regs[9] = {0xb8, 0xaa, 0xaa, 0xaa, 0xb0, 0x88, 0xc3, 0xc5, 0xc7};
    //    return mpu_write_mem(CFG_MOTION_BIAS, 9, regs);
    //}
}

bool dmp_enable_lp_quat(void /*uint8_t enable*/)
{
    uint8_t regs[4];
    //if (enable) {
    //    regs[0] = DINBC0;
    //    regs[1] = DINBC2;
    //    regs[2] = DINBC4;
    //    regs[3] = DINBC6;
    //}
    //else
        memset(regs, 0x8B, 4);

    return mpu_write_mem(CFG_LP_QUAT, 4, regs);

    //return mpu_reset_fifo();
}

bool dmp_enable_6x_lp_quat(void /*uint8_t enable*/)
{
    uint8_t regs[4];
    //if (enable) {
    //    regs[0] = DINA20;
    //    regs[1] = DINA28;
    //    regs[2] = DINA30;
    //    regs[3] = DINA38;
    //} else
        memset(regs, 0xA3, 4);

    return mpu_write_mem(CFG_8, 4, regs);

    //return mpu_reset_fifo();
}

bool dmp_enable_feature(void)
{
    uint8_t tmp[10];

	// DMP_FEATURE_6X_LP_QUAT
	// DMP_FEATURE_SEND_RAW_ACCEL
	// DMP_FEATURE_SEND_CAL_GYRO
	// DMP_FEATURE_GYRO_CAL
	// DMP_FEATURE_SEND_ANY_GYRO
	
    // Set integration scale factor.
    tmp[0] = (uint8_t)((GYRO_SF >> 24) & 0xFF);
    tmp[1] = (uint8_t)((GYRO_SF >> 16) & 0xFF);
    tmp[2] = (uint8_t)((GYRO_SF >> 8) & 0xFF);
    tmp[3] = (uint8_t)(GYRO_SF & 0xFF);
    mpu_write_mem(D_0_104, 4, tmp);

    // Send sensor data to the FIFO.
    tmp[0] = 0xA3;
    //if (mask & DMP_FEATURE_SEND_RAW_ACCEL)
	//{
        tmp[1] = 0xC0;
        tmp[2] = 0xC8;
        tmp[3] = 0xC2;
    //} else {
    //    tmp[1] = 0xA3;
    //    tmp[2] = 0xA3;
    //    tmp[3] = 0xA3;
    //}
	
    //if (mask & DMP_FEATURE_SEND_ANY_GYRO)
	//{
        tmp[4] = 0xC4;
        tmp[5] = 0xCC;
        tmp[6] = 0xC6;
    //} else {
    //    tmp[4] = 0xA3;
    //    tmp[5] = 0xA3;
    //    tmp[6] = 0xA3;
    //}
    tmp[7] = 0xA3;
    tmp[8] = 0xA3;
    tmp[9] = 0xA3;
    mpu_write_mem(CFG_15, 10, tmp);

    // Send gesture data to the FIFO.
    //if (mask & (DMP_FEATURE_TAP | DMP_FEATURE_ANDROID_ORIENT))
    //    tmp[0] = DINA20;
    //else
        tmp[0] = 0xD8;
    mpu_write_mem(CFG_27, 1, tmp);

    //if (mask & DMP_FEATURE_GYRO_CAL)
        dmp_enable_gyro_cal();
    //else
    //    dmp_enable_gyro_cal(0);

    //if (mask & DMP_FEATURE_SEND_ANY_GYRO)
	//{
        //if (mask & DMP_FEATURE_SEND_CAL_GYRO)
		//{
            tmp[0] = 0xB2;
            tmp[1] = 0x8B;
            tmp[2] = 0xB6;
            tmp[3] = 0x9B;
        //} else {
        //    tmp[0] = DINAC0;
        //    tmp[1] = DINA80;
        //    tmp[2] = DINAC2;
        //    tmp[3] = DINA90;
        //}
        mpu_write_mem(CFG_GYRO_RAW_DATA, 4, tmp);
    //}

    //if (mask & DMP_FEATURE_TAP)
	//{
    //    // Enable tap.
    //    tmp[0] = 0xF8;
    //    mpu_write_mem(CFG_20, 1, tmp);
    //    dmp_set_tap_thresh(TAP_XYZ, 250);
    //    dmp_set_tap_axes(TAP_XYZ);
    //    dmp_set_tap_count(1);
    //    dmp_set_tap_time(100);
    //    dmp_set_tap_time_multi(500);
    //
    //    dmp_set_shake_reject_thresh(GYRO_SF, 200);
    //    dmp_set_shake_reject_time(40);
    //    dmp_set_shake_reject_timeout(10);
    //} else {
        tmp[0] = 0xD8;
        mpu_write_mem(CFG_20, 1, tmp);
    //}

    //if (mask & DMP_FEATURE_ANDROID_ORIENT)
    //    tmp[0] = 0xD9;
    //else
        tmp[0] = 0xD8;
    mpu_write_mem(CFG_ANDROID_ORIENT_INT, 1, tmp);

    //if (mask & DMP_FEATURE_LP_QUAT)
    //    dmp_enable_lp_quat(1);
    //else
        dmp_enable_lp_quat();

    //if (mask & DMP_FEATURE_6X_LP_QUAT)
        dmp_enable_6x_lp_quat();
    //else
    //    dmp_enable_6x_lp_quat(0);

    // Pedometer is always enabled. */
    // mpu_reset_fifo();

    return true;
}

bool dmp_set_fifo_rate(void /*uint16_t rate*/)
{
	const uint8_t __code regs_end[12]
		= {0xFE, 0xF2, 0xAB, 0xc4, 0xAA, 0xF1, 0xDF, 0xDF, 0xBB, 0xAF, 0xDF, 0xDF};
	const uint8_t __code tmp[2] = {0, 0};

	if (!mpu_write_mem(D_0_22, 2, tmp))
		return false;

	if (!mpu_write_mem(CFG_6, 12, regs_end))
		return false;

	return true;
}

bool mpu_read_fifo_stream(uint16_t length, uint8_t* data, uint8_t* more)
{
	uint8_t tmp[2];
	uint16_t fifo_count;

	if (!i2c_read(FIFO_COUNT_H, 2, tmp))
		return false;

	fifo_count = (tmp[0] << 8) | tmp[1];

	if (fifo_count < length)
	{
		more[0] = 0;
		return false;
	}

	if (fifo_count > (MAX_FIFO >> 1))
	{
		// FIFO is 50% full, better check overflow bit
		if (!i2c_read(INT_STATUS, 1, tmp))
			return false;

		if (tmp[0] & BIT_FIFO_OVERFLOW)
		{
			mpu_reset_fifo();
			return false;
		}
	}

	if (!i2c_read(FIFO_R_W, length, data))
		return false;

	more[0] = fifo_count / length - 1;

	return true;
}

#define FIFO_CORRUPTION_CHECK
#ifdef FIFO_CORRUPTION_CHECK
# define QUAT_ERROR_THRESH			(1L<<24)
# define QUAT_MAG_SQ_NORMALIZED		(1L<<28)
# define QUAT_MAG_SQ_MIN			(QUAT_MAG_SQ_NORMALIZED - QUAT_ERROR_THRESH)
# define QUAT_MAG_SQ_MAX			(QUAT_MAG_SQ_NORMALIZED + QUAT_ERROR_THRESH)
#endif

bool dmp_read_fifo(int16_t* gyro, int16_t* accel, int32_t* quat, uint8_t* more)
{
	uint8_t fifo_data[PACKET_LENGTH];
	uint8_t ii = 0;

	// Get a packet.
	if (!mpu_read_fifo_stream(PACKET_LENGTH, fifo_data, more))
		return false;

	// Parse DMP packet.
	//if (dmp.feature_mask & (DMP_FEATURE_LP_QUAT | DMP_FEATURE_6X_LP_QUAT))
	{
#ifdef FIFO_CORRUPTION_CHECK
		int32_t quat_q14[4], quat_mag_sq;
#endif
		quat[0] = ((long)fifo_data[0] << 24) | ((long)fifo_data[1] << 16) |
					((long)fifo_data[2] << 8) | fifo_data[3];
		quat[1] = ((long)fifo_data[4] << 24) | ((long)fifo_data[5] << 16) |
					((long)fifo_data[6] << 8) | fifo_data[7];
		quat[2] = ((long)fifo_data[8] << 24) | ((long)fifo_data[9] << 16) |
					((long)fifo_data[10] << 8) | fifo_data[11];
		quat[3] = ((long)fifo_data[12] << 24) | ((long)fifo_data[13] << 16) |
					((long)fifo_data[14] << 8) | fifo_data[15];
		ii += 16;
#ifdef FIFO_CORRUPTION_CHECK
		// We can detect a corrupted FIFO by monitoring the quaternion data and
		// ensuring that the magnitude is always normalized to one. This
		// shouldn't happen in normal operation, but if an I2C error occurs,
		// the FIFO reads might become misaligned.
		//
		// Let's start by scaling down the quaternion data to avoid long long
		// math.
		quat_q14[0] = quat[0] >> 16;
		quat_q14[1] = quat[1] >> 16;
		quat_q14[2] = quat[2] >> 16;
		quat_q14[3] = quat[3] >> 16;
		quat_mag_sq = quat_q14[0] * quat_q14[0] + quat_q14[1] * quat_q14[1] +
						quat_q14[2] * quat_q14[2] + quat_q14[3] * quat_q14[3];

		if (quat_mag_sq < QUAT_MAG_SQ_MIN  ||  quat_mag_sq > QUAT_MAG_SQ_MAX)
		{
			// Quaternion is outside of the acceptable threshold.
			mpu_reset_fifo();
			return false;
		}
#endif
	}

	//if (dmp.feature_mask & DMP_FEATURE_SEND_RAW_ACCEL) {
	accel[0] = ((short)fifo_data[ii+0] << 8) | fifo_data[ii+1];
	accel[1] = ((short)fifo_data[ii+2] << 8) | fifo_data[ii+3];
	accel[2] = ((short)fifo_data[ii+4] << 8) | fifo_data[ii+5];
	ii += 6;
	//}

	//if (dmp.feature_mask & DMP_FEATURE_SEND_ANY_GYRO) {
	gyro[0] = ((short)fifo_data[ii+0] << 8) | fifo_data[ii+1];
	gyro[1] = ((short)fifo_data[ii+2] << 8) | fifo_data[ii+3];
	gyro[2] = ((short)fifo_data[ii+4] << 8) | fifo_data[ii+5];
	ii += 6;
	//}

	//get_ms(timestamp);

	return true;
}

bool dmp_init(void)
{
	long bias[3];
	
	if (!dmp_load_firmware())
	{
		puts("dmp_load_firmware FAILED!!!");
		return false;
	}

	if (!dmp_set_orientation())
	{
		puts("dmp_set_orientation FAILED!!!");
		return false;
	}
	
	if (!dmp_enable_feature())
	{
		puts("dmp_enable_feature FAILED!!!");
		return false;
	}
	
	if (!dmp_set_fifo_rate())
	{
		puts("dmp_set_fifo_rate FAILED!!!");
		return false;
	}
	
	bias[0] = 0x1234;
	bias[1] = 0x1234;
	bias[2] = 0x1234;
	
	mpu_set_gyro_bias_reg(bias);
	mpu_set_accel_bias_6050_reg(bias, true);
	
	if (!start_dmp_state())
	{
		puts("start_dmp_state() FAILED...");
		return false;
	}
	
	return true;
}

/*
dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL);
dmp_load_motion_driver_firmware();
dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
dmp_set_fifo_rate(DEFAULT_MPU_HZ);
dmp_set_fifo_rate(DEFAULT_MPU_HZ/2);
dmp_set_orientation(gyro_orients[orientation]);
mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
mpu_get_compass_reg(mag, &timestamp);
mpu_init();
mpu_set_accel_bias_6050_reg(aBias, true);
mpu_set_accel_fsr(2);
mpu_set_dmp_state(0);
mpu_set_dmp_state(1);
mpu_set_dmp_state(1);  // This enables the DMP; at this point, interrupts should commence
mpu_set_gyro_bias_reg(gBias);
mpu_set_gyro_fsr(2000);
mpu_set_lpf(42);
mpu_set_sample_rate(DEFAULT_MPU_HZ);
mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
*/