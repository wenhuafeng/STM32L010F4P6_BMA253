#include "bma253.h"
#include "main.h"
#include "bma253_iic.h"
#include "vcom.h"

#if defined(PEDOMETER) && PEDOMETER

#define GET_ACC_VALUE(H, L)              ((int16_t)(((int16_t)((int8_t)(H))) << 8 | ((L)&0xf0))) >> 4
#define CHECK_SLOW_AUOT_OFFSET_TARGET(O) ((O) == -1 || (O) == 0 || (O) == 1)
#define BMA253_SET_BIT(R, O, Ret)                  \
    do {                                           \
        uint8_t _v;                                \
        if (bma253_read_byte((R), &_v) == false) { \
            (Ret) = false;                         \
            break;                                 \
        }                                          \
        _v |= (1 << (O));                          \
        if (bma253_write_byte((R), _v) == false) { \
            (Ret) = false;                         \
            break;                                 \
        }                                          \
        (Ret) = true;                              \
    } while (0)
#define BMA253_CLEAR_BIT(R, O, Ret)                \
    do {                                           \
        uint8_t _v;                                \
        if (bma253_read_byte((R), &_v) == false) { \
            (Ret) = false;                         \
            break;                                 \
        }                                          \
        _v &= ~(1 << (O));                         \
        if (bma253_write_byte((R), _v) == false) { \
            (Ret) = false;                         \
            break;                                 \
        }                                          \
        (Ret) = true;                              \
    } while (0)

#define BMA253_I2C_SLAVE_ADDRESS 0x18
//#define BMA253_I2C_SLAVE_ADDRESS 0x19

// Left Shit 1 bit
//#define BMA253_I2C_SLAVE_ADDRESS 0x30
//#define BMA253_I2C_SLAVE_ADDRESS 0x32

// b'1111'1010
#define BMA253_CHIP_ID (0xFA)

#define BMA253_EEP_OFFSET   (0x16)
#define BMA253_IMAGE_BASE   (0x38)
#define BMA253_IMAGE_LEN    (22)
#define BMA253_CHIP_ID_ADDR (0x00)
/* DATA ADDRESS DEFINITIONS */
#define BMA253_X_AXIS_LSB_ADDR (0x02)
#define BMA253_X_AXIS_MSB_ADDR (0x03)
#define BMA253_Y_AXIS_LSB_ADDR (0x04)
#define BMA253_Y_AXIS_MSB_ADDR (0x05)
#define BMA253_Z_AXIS_LSB_ADDR (0x06)
#define BMA253_Z_AXIS_MSB_ADDR (0x07)
#define BMA253_TEMP_ADDR       (0x08)
/* STATUS ADDRESS DEFINITIONS */
#define BMA253_STAT1_ADDR            (0x09)
#define BMA253_STAT2_ADDR            (0x0A)
#define BMA253_STAT_TAP_SLOPE_ADDR   (0x0B)
#define BMA253_STAT_ORIENT_HIGH_ADDR (0x0C)
#define BMA253_STAT_FIFO_ADDR        (0x0E)
/* STATUS ADDRESS DEFINITIONS */
#define BMA253_RANGE_SELECT_ADDR   (0x0F)
#define BMA253_BW_SELECT_ADDR      (0x10)
#define BMA253_MODE_CTRL_ADDR      (0x11)
#define BMA253_LOW_NOISE_CTRL_ADDR (0x12)
#define BMA253_DATA_CTRL_ADDR      (0x13)
#define BMA253_RST_ADDR            (0x14)
/* INTERUPT ADDRESS DEFINITIONS */
#define BMA253_INTR_ENABLE1_ADDR        (0x16)
#define BMA253_INTR_ENABLE2_ADDR        (0x17)
#define BMA253_INTR_SLOW_NO_MOTION_ADDR (0x18)
#define BMA253_INTR1_PAD_SELECT_ADDR    (0x19)
#define BMA253_INTR_DATA_SELECT_ADDR    (0x1A)
#define BMA253_INTR2_PAD_SELECT_ADDR    (0x1B)
#define BMA253_INTR_SOURCE_ADDR         (0x1E)
#define BMA253_INTR_SET_ADDR            (0x20)
#define BMA253_INTR_CTRL_ADDR           (0x21)
/* FEATURE ADDRESS DEFINITIONS */
#define BMA253_LOW_DURN_ADDR             (0x22)
#define BMA253_LOW_THRES_ADDR            (0x23)
#define BMA253_LOW_HIGH_HYST_ADDR        (0x24)
#define BMA253_HIGH_DURN_ADDR            (0x25)
#define BMA253_HIGH_THRES_ADDR           (0x26)
#define BMA253_SLOPE_DURN_ADDR           (0x27)
#define BMA253_SLOPE_THRES_ADDR          (0x28)
#define BMA253_SLOW_NO_MOTION_THRES_ADDR (0x29)
#define BMA253_TAP_PARAM_ADDR            (0x2A)
#define BMA253_TAP_THRES_ADDR            (0x2B)
#define BMA253_ORIENT_PARAM_ADDR         (0x2C)
#define BMA253_THETA_BLOCK_ADDR          (0x2D)
#define BMA253_THETA_FLAT_ADDR           (0x2E)
#define BMA253_FLAT_HOLD_TIME_ADDR       (0x2F)
#define BMA253_SELFTEST_ADDR             (0x32)
#define BMA253_EEPROM_CTRL_ADDR          (0x33)
#define BMA253_SERIAL_CTRL_ADDR          (0x34)
/* OFFSET ADDRESS DEFINITIONS */
#define BMA253_OFFSET_CTRL_ADDR   (0x36)
#define BMA253_OFFSET_PARAMS_ADDR (0x37)
#define BMA253_OFFSET_X_AXIS_ADDR (0x38)
#define BMA253_OFFSET_Y_AXIS_ADDR (0x39)
#define BMA253_OFFSET_Z_AXIS_ADDR (0x3A)
/* GP ADDRESS DEFINITIONS */
#define BMA253_GP0_ADDR (0x3B)
#define BMA253_GP1_ADDR (0x3C)
/* FIFO ADDRESS DEFINITIONS */
#define BMA253_FIFO_MODE_ADDR        (0x3E)
#define BMA253_FIFO_DATA_OUTPUT_ADDR (0x3F)
#define BMA253_FIFO_WML_TRIG         (0x30)

static bool bma253_read_byte(uint8_t addr, uint8_t *value)
{
    return BMA253_ReadByte(addr, value);
}
static bool bma253_write_byte(uint8_t addr, uint8_t value)
{
    return BMA253_WriteByte(addr, value);
}

void BMA_DelayMs(uint8_t MS)
{
    LL_mDelay(MS);
}

bool accelerometer_probe(void)
{
    uint8_t chip_id;
    if (true != bma253_read_byte(BMA253_CHIP_ID_ADDR, &chip_id)) {
        goto ERR;
    }
    if (BMA253_CHIP_ID != chip_id) {
        goto ERR;
    } else {
        ;
    }

    return true;
ERR:
    return false;
}

bool accelerometer_soft_reset(void)
{
    if (bma253_write_byte(BMA253_RST_ADDR, 0xb6) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_power_mode_set(acc_power_mode_t mode)
{
    uint8_t lpw_ctrl, low_power;

    if (bma253_read_byte(BMA253_LOW_NOISE_CTRL_ADDR, &low_power) == false)
        goto ERR;
    BMA_DelayMs(1);
    if (bma253_read_byte(BMA253_MODE_CTRL_ADDR, &lpw_ctrl) == false)
        goto ERR;

    lpw_ctrl &= ~0xe0;
    switch (mode) {
        case ACC_POWER_MODE_NORMAL:
            //lpw_ctrl |= 0x00;
            low_power &= ~0x40;
            break;
        case ACC_POWER_MODE_LOW_POWER1:
            lpw_ctrl |= 0x40;
            low_power &= ~0x40;
            break;
        case ACC_POWER_MODE_SUSPEND:
            lpw_ctrl |= 0x80;
            low_power &= ~0x40;
            break;
        case ACC_POWER_MODE_DEEP_SUSPEND:
            lpw_ctrl |= 0x20;
            break;
        case ACC_POWER_MODE_LOW_POWER2:
            lpw_ctrl |= 0x40;
            low_power |= 0x40;
            break;
        case ACC_POWER_MODE_STANDBY:
            lpw_ctrl |= 0x80;
            low_power |= 0x40;
        default:
            goto ERR;
    }

    if (bma253_write_byte(BMA253_LOW_NOISE_CTRL_ADDR, low_power) == false)
        goto ERR;
    BMA_DelayMs(1);
    if (bma253_write_byte(BMA253_MODE_CTRL_ADDR, lpw_ctrl) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_power_mode_get(acc_power_mode_t *mode)
{
    uint8_t lpw_ctrl, low_power;

    if (bma253_read_byte(BMA253_LOW_NOISE_CTRL_ADDR, &low_power) == false)
        goto ERR;
    BMA_DelayMs(1);
    if (bma253_read_byte(BMA253_MODE_CTRL_ADDR, &lpw_ctrl) == false)
        goto ERR;

    lpw_ctrl &= 0xe0;
    low_power &= 0x40;
    switch (lpw_ctrl) {
        case 0x00:
            if (low_power == 0x00) {
                *mode = ACC_POWER_MODE_NORMAL;
            }
            break;
        case 0x40:
            if (low_power == 0x00) {
                *mode = ACC_POWER_MODE_LOW_POWER1;
            } else if (low_power == 0x40) {
                *mode = ACC_POWER_MODE_LOW_POWER2;
            } else {
                goto ERR;
            }
            break;
        case 0x80:
            if (low_power == 0x00) {
                *mode = ACC_POWER_MODE_SUSPEND;
            } else if (low_power == 0x40) {
                *mode = ACC_POWER_MODE_STANDBY;
            } else {
                goto ERR;
            }
            break;
        case 0x20:
            *mode = ACC_POWER_MODE_DEEP_SUSPEND;
            break;
        default:
            goto ERR;
    }

    return true;
ERR:
    return false;
}

bool accelerometer_set_sleep_durn(acc_sleep_durn_t durn)
{
    uint8_t sleep_durn;

    if (bma253_read_byte(BMA253_MODE_CTRL_ADDR, &sleep_durn) == false)
        goto ERR;

    sleep_durn &= 0xe1;
    sleep_durn |= (durn);

    if (bma253_write_byte(BMA253_MODE_CTRL_ADDR, sleep_durn) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_get_sleep_durn(acc_sleep_durn_t *durn)
{
    uint8_t sleep_durn;

    if (bma253_read_byte(BMA253_MODE_CTRL_ADDR, &sleep_durn) == false)
        goto ERR;

    sleep_durn &= 0x1e;
    *durn = (acc_sleep_durn_t)sleep_durn;

    return true;
ERR:
    return false;
}

bool accelerometer_set_sleep_timer_mode(uint8_t en)
{
    uint8_t sleep_timer;

    if (bma253_read_byte(BMA253_LOW_NOISE_CTRL_ADDR, &sleep_timer) == false)
        goto ERR;

    if (en == 0x00) {
        sleep_timer &= ~0x20;
    } else {
        sleep_timer |= 0x20;
    }

    if (bma253_write_byte(BMA253_LOW_NOISE_CTRL_ADDR, sleep_timer) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_get_sleep_timer_mode(uint8_t *en)
{
    uint8_t sleep_timer;

    if (bma253_read_byte(BMA253_LOW_NOISE_CTRL_ADDR, &sleep_timer) == false)
        goto ERR;

    if (sleep_timer & 0x20) {
        *en = 1;
    } else {
        *en = 0;
    }

    return true;
ERR:
    return false;
}

bool accelerometer_accel_get(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t h, l;

    // Must read LSB first
    if (bma253_read_byte(BMA253_X_AXIS_LSB_ADDR, &l) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_X_AXIS_MSB_ADDR, &h) == false)
        goto ERR;
    *x = GET_ACC_VALUE(h, l);

    if (bma253_read_byte(BMA253_Y_AXIS_LSB_ADDR, &l) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_Y_AXIS_MSB_ADDR, &h) == false)
        goto ERR;
    *y = GET_ACC_VALUE(h, l);

    if (bma253_read_byte(BMA253_Z_AXIS_LSB_ADDR, &l) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_Z_AXIS_MSB_ADDR, &h) == false)
        goto ERR;
    *z = GET_ACC_VALUE(h, l);

    return true;
ERR:
    return false;
}

bool accelerometer_temperature_get(int8_t *temp)
{
    uint8_t data;

    if (bma253_read_byte(BMA253_TEMP_ADDR, &data) == false)
        goto ERR;

    // data = 0 = 23C = 296.15K, 0.5k/LSB
    *temp = 23 + (((int8_t)(data)) / 2);

    return true;
ERR:
    return false;
}

/*
 * target: -1, 0, 1
 */
bool accelerometer_offset_slow_auto_gen(int8_t x_en, int8_t y_en, int8_t z_en, int8_t cut_off_en)
{
    uint8_t data = 0;

    if (x_en) {
        data |= 0x01;
    }
    if (y_en) {
        data |= 0x02;
    }
    if (z_en) {
        data |= 0x04;
    }
    if (bma253_write_byte(BMA253_OFFSET_CTRL_ADDR, data) == false)
        goto ERR;

    data = 0;
    if (cut_off_en) {
        data |= 0x1;
    }
    if (bma253_write_byte(BMA253_OFFSET_PARAMS_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * axis: 0 = x, 1 = y, 2 = z
 */
bool accelerometer_offset_fast_auto_gen(int8_t axis, acc_offset_target_t target)
{
    uint8_t data = 0;

    if ((axis < 0 || axis > 2) || (target > ACC_OFFSET_TARGET_MAX)) {
        goto ERR;
    }

    // range must be 2g
    if (accelerometer_range_set(ACC_RANGE_2G) == false)
        goto ERR;

    data |= (target << (axis * 2));
    // set target
    if (bma253_write_byte(BMA253_OFFSET_PARAMS_ADDR, data) == false) {
        goto ERR;
    }

    // start
    data = 0;
    if (axis == 0) {
        data = 0x20;
    } else if (axis == 1) {
        data = 0x40;
    } else {
        data = 0x60;
    }
    if (bma253_write_byte(BMA253_OFFSET_CTRL_ADDR, data) == false) {
        goto ERR;
    }

    // wait completed
    while (1) {
        LL_mDelay(10);
        if (bma253_read_byte(BMA253_OFFSET_CTRL_ADDR, &data) == false) {
            goto ERR;
        }
        if ((data & 0x10) != 0) {
            break;
        }
    }
    return true;
ERR:
    return false;
}

bool accelerometer_offset_clear(void)
{
    if (bma253_write_byte(BMA253_OFFSET_CTRL_ADDR, 0x80) == false)
        return false;
    return true;
}
/*
 * mapping
 * +127 -> +0.992g
 * 0 -> 0 g
 * -128 -> -1 g
 * the scaling is independent of the selected g-range
 */
bool accelerometer_offset_get(int8_t *x, int8_t *y, int8_t *z)
{
    if (bma253_read_byte(BMA253_OFFSET_X_AXIS_ADDR, (uint8_t *)x) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_OFFSET_Y_AXIS_ADDR, (uint8_t *)y) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_OFFSET_Z_AXIS_ADDR, (uint8_t *)z) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * mapping
 * +127 -> +0.992g
 * 0 -> 0 g
 * -128 -> -1 g
 * the scaling is independent of the selected g-range
 */
bool accelerometer_offset_set(int8_t x, int8_t y, int8_t z)
{
    uint8_t data = 0;

    // Step 1: write new content to image registers
    if (bma253_write_byte(BMA253_OFFSET_X_AXIS_ADDR, (uint8_t)x) == false)
        goto ERR;
    if (bma253_write_byte(BMA253_OFFSET_Y_AXIS_ADDR, (uint8_t)y) == false)
        goto ERR;
    if (bma253_write_byte(BMA253_OFFSET_Z_AXIS_ADDR, (uint8_t)z) == false)
        goto ERR;

    // 3         2        1               0
    // nvm_load  nvm_rdy  nvm_prog_trig   nvm_prog_mode
    // Step 2: Unlock NVM
    if (bma253_write_byte(BMA253_EEPROM_CTRL_ADDR, 0x1) == false)
        goto ERR;
    // Step 3: trigger write process
    if (bma253_write_byte(BMA253_EEPROM_CTRL_ADDR, 0x3) == false)
        goto ERR;
    // wait until writing completed
    while (1) {
        if (bma253_read_byte(BMA253_EEPROM_CTRL_ADDR, &data) == false)
            goto ERR;
        if ((data & 0x04) != 0)
            break;
    }
    return true;
ERR:
    return false;
}
bool accelerometer_range_set(acc_accel_range_t acc_range_val)
{
    uint8_t range;

    switch (acc_range_val) {
        case ACC_RANGE_2G:
            range = 0x3;
            break;
        case ACC_RANGE_4G:
            range = 0x5;
            break;
        case ACC_RANGE_8G:
            range = 0x8;
            break;
        case ACC_RANGE_16G:
            range = 0xc;
            break;
        default:
            goto ERR;
    }
    if (bma253_write_byte(BMA253_RANGE_SELECT_ADDR, range) == false)
        goto ERR;
    return true;
ERR:
    return false;
}

bool accelerometer_range_get(acc_accel_range_t *acc_range_val)
{
    uint8_t range;

    if (bma253_read_byte(BMA253_RANGE_SELECT_ADDR, &range) == false)
        goto ERR;
    range = range & 0x0f;

    switch (range) {
        case 0x3:
            *acc_range_val = ACC_RANGE_2G;
            break;
        case 0x5:
            *acc_range_val = ACC_RANGE_4G;
            break;
        case 0x8:
            *acc_range_val = ACC_RANGE_8G;
            break;
        case 0xc:
            *acc_range_val = ACC_RANGE_16G;
            break;
        default:
            *acc_range_val = ACC_RANGE_UNKNOWN;
            break;
    }

    return true;
ERR:
    return false;
}

bool accelerometer_filter_enable_set(int8_t en)
{
    uint8_t data = 0;

    if (en == 0) {
        // unfiltered
        data |= 0x80;
    }

    if (bma253_write_byte(BMA253_DATA_CTRL_ADDR, data) == false)
        return false;
    return true;
}

bool accelerometer_filter_enable_get(int8_t *en)
{
    uint8_t data = 0;

    if (bma253_read_byte(BMA253_DATA_CTRL_ADDR, &data) == false)
        return false;

    if ((data & 0x80) != 0)
        *en = 0;
    else
        *en = 1;

    return true;
}

bool accelerometer_filter_bandwidth_set(acc_filter_bandwidth_t bw)
{
    uint8_t data[ACC_FILTER_BANDWIDTH_MAX] = { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    if (bw >= ACC_FILTER_BANDWIDTH_MAX)
        goto ERR;

    if (bma253_write_byte(BMA253_BW_SELECT_ADDR, data[bw]) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_filter_bandwidth_get(acc_filter_bandwidth_t *bw)
{
    uint8_t data;

    if (bma253_read_byte(BMA253_BW_SELECT_ADDR, &data) == false)
        goto ERR;
    data = data & 0x0f;
    *bw  = (acc_filter_bandwidth_t)(data - 0x08);

    return true;
ERR:
    return false;
}

bool accelerometer_fifo_mode_set(acc_fifo_mode_t mode, acc_fifo_data_type_t data_type)
{
    uint8_t data = 0;

    data = ((uint8_t)mode << 6) | (uint8_t)data_type;

    if (bma253_write_byte(BMA253_FIFO_MODE_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_fifo_mode_get(acc_fifo_mode_t *mode, acc_fifo_data_type_t *data_type)
{
    uint8_t data = 0;

    if (bma253_read_byte(BMA253_FIFO_MODE_ADDR, &data) == false)
        goto ERR;

    *mode      = (acc_fifo_mode_t)((data & 0xC0) >> 6);
    *data_type = (acc_fifo_data_type_t)(data & 0x02);

    return true;
ERR:
    return false;
}

/*
bool accelerometer_fifo_data_read(uint16_t *value)
{
    uint8_t *p_d = (uint8_t *)value;

    if (bma253_read_byte(BMA253_FIFO_MODE_ADDR, p_d) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_FIFO_MODE_ADDR, &p_d[1]) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool bma2x2_read_accel_xyz_fifo(struct bma2x2_accel_data_fifo *data_fifo, uint8_t len)
{
    return BMA253_ReadNBytes(BMA253_FIFO_DATA_OUTPUT_ADDR, (uint8_t *)data_fifo, len);
}
*/

bool accelerometer_fifo_data_read(struct bma2x2_accel_data_fifo *data_fifo, uint8_t len)
{
    return BMA253_ReadNBytes(BMA253_FIFO_DATA_OUTPUT_ADDR, (uint8_t *)data_fifo, len);
}

bool accelerometer_fifo_watermark_level_set(uint8_t level)
{
    if (level > 32)
        goto ERR;

    if (bma253_write_byte(BMA253_FIFO_WML_TRIG, level) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_fifo_watermark_level_get(uint8_t *level)
{
    if (bma253_read_byte(BMA253_FIFO_WML_TRIG, level) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_fifo_status_get(uint8_t *frame_count, uint8_t *over_run)
{
    uint8_t data;

    if (bma253_read_byte(BMA253_STAT_FIFO_ADDR, &data) == false)
        goto ERR;

    *over_run    = (data & 0x80) >> 7;
    *frame_count = data & 0x7f;

    return true;
ERR:
    return false;
}

// Interrupt configrations
bool accelerometer_int_mode_set(uint8_t clear_latched_int, acc_int_latch_type_t latch)
{
    uint8_t data;

    if (latch >= ACC_INT_LATCH_MAX)
        goto ERR;

    data = (clear_latched_int << 7) | ((uint8_t)latch);

    if (bma253_write_byte(BMA253_INTR_CTRL_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_int_mode_get(acc_int_latch_type_t *latch)
{
    if (bma253_read_byte(BMA253_INTR_CTRL_ADDR, latch) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

bool accelerometer_rst_intr(void)
{
    uint8_t latch;

    if (bma253_read_byte(BMA253_INTR_CTRL_ADDR, &latch) == false)
        goto ERR;
    latch |= 0x80; //clear any latch interrupts
    if (bma253_write_byte(BMA253_INTR_CTRL_ADDR, latch) == false)
        goto ERR;
    return true;
ERR:
    return false;
}

bool accelerometer_int_out_ctrl(acc_int_pin_t pin, acc_int_output_type_level_t intr_output_type_level)
{
    uint8_t data = 0;

    if (bma253_read_byte(BMA253_INTR_SET_ADDR, &data) == false)
        goto ERR;
    if (pin >= ACC_INT_PIN_MAX)
        goto ERR;
    if (pin == ACC_INT_PIN_1) {
        data &= 0xfc;
        data |= intr_output_type_level;
    } else {
        data &= 0xf3;
        data |= intr_output_type_level;
    }

    if (bma253_write_byte(BMA253_INTR_SET_ADDR, data) == false)
        goto ERR;
    return true;
ERR:
    return false;
}

bool accelerometer_int_out_ctrl_get(acc_int_pin_t pin, acc_int_output_type_level_t *intr_output_type_level)
{
    uint8_t output_type_level;

    if (bma253_read_byte(BMA253_INTR_SET_ADDR, &output_type_level) == false)
        goto ERR;

    *intr_output_type_level = (acc_int_output_type_level_t)output_type_level;
    if (pin == ACC_INT_PIN_1) {
        *intr_output_type_level &= ~0xfc;
    } else {
        *intr_output_type_level &= ~0xf3;
    }
    return true;
ERR:
    return false;
}

bool accelerometer_int_filter_enable(acc_int_src_type_t int_type, int8_t enable)
{
    int8_t ret = false;
    struct {
        uint8_t en_reg;
        uint8_t en_offset;
    } filter_offset[] = {
        {
                BMA253_INTR_SOURCE_ADDR,
        }, // ACC_INT_SRC_TYPE_FLAT
        {
                BMA253_INTR_SOURCE_ADDR,
        },                              // ACC_INT_SRC_TYPE_ORIENT
        { BMA253_INTR_SOURCE_ADDR, 4 }, // ACC_INT_SRC_TYPE_SIGNAL_TAP
        { BMA253_INTR_SOURCE_ADDR, 4 }, // ACC_INT_SRC_TYPE_DOUBLE_TAP
        { 0xff, 0 },                    // ACC_INT_SRC_TYPE_SLOPE_X
        { 0xff, 0 },                    // ACC_INT_SRC_TYPE_SLOPE_Y
        { 0xff, 0 },                    // ACC_INT_SRC_TYPE_SLOPE_Z
        { 0xff, 0 },                    // ACC_INT_SRC_TYPE_FIFO_WATERMARK
        { 0xff, 0 },                    // ACC_INT_SRC_TYPE_FIFO_FULL
        { BMA253_INTR_SOURCE_ADDR, 5 }, // ACC_INT_SRC_TYPE_DATA_READY
        { BMA253_INTR_SOURCE_ADDR, 0 }, // ACC_INT_SRC_TYPE_LOW_G
        { BMA253_INTR_SOURCE_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_X
        { BMA253_INTR_SOURCE_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_Y
        { BMA253_INTR_SOURCE_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_Z
        { BMA253_INTR_SOURCE_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION
        { BMA253_INTR_SOURCE_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_X
        { BMA253_INTR_SOURCE_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Y
        { BMA253_INTR_SOURCE_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Z
    };

    if (filter_offset[int_type].en_reg != 0xff) {
        if (enable) {
            BMA253_SET_BIT(filter_offset[int_type].en_reg, filter_offset[int_type].en_offset, ret);
        } else {
            BMA253_CLEAR_BIT(filter_offset[int_type].en_reg, filter_offset[int_type].en_offset, ret);
        }
    }

    return ret;
}

bool accelerometer_int_map_to_pin1(acc_int_src_type_t int_type, int8_t enable)
{
    int8_t ret;
    struct {
        uint8_t en_reg;
        uint8_t en_offset;
    } map_offset[] = {
        { BMA253_INTR1_PAD_SELECT_ADDR, 7 }, // ACC_INT_SRC_TYPE_FLAT
        { BMA253_INTR1_PAD_SELECT_ADDR, 6 }, // ACC_INT_SRC_TYPE_ORIENT
        { BMA253_INTR1_PAD_SELECT_ADDR, 5 }, // ACC_INT_SRC_TYPE_SIGNAL_TAP
        { BMA253_INTR1_PAD_SELECT_ADDR, 4 }, // ACC_INT_SRC_TYPE_DOUBLE_TAP
        { BMA253_INTR1_PAD_SELECT_ADDR, 2 }, // ACC_INT_SRC_TYPE_SLOPE_X
        { BMA253_INTR1_PAD_SELECT_ADDR, 2 }, // ACC_INT_SRC_TYPE_SLOPE_Y
        { BMA253_INTR1_PAD_SELECT_ADDR, 2 }, // ACC_INT_SRC_TYPE_SLOPE_Z
        { BMA253_INTR_DATA_SELECT_ADDR, 1 }, // ACC_INT_SRC_TYPE_FIFO_WATERMARK
        { BMA253_INTR_DATA_SELECT_ADDR, 2 }, // ACC_INT_SRC_TYPE_FIFO_FULL
        { BMA253_INTR_DATA_SELECT_ADDR, 0 }, // ACC_INT_SRC_TYPE_DATA_READY
        { BMA253_INTR1_PAD_SELECT_ADDR, 0 }, // ACC_INT_SRC_TYPE_LOW_G
        { BMA253_INTR1_PAD_SELECT_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_X
        { BMA253_INTR1_PAD_SELECT_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_Y
        { BMA253_INTR1_PAD_SELECT_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_Z
        { BMA253_INTR1_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION
        { BMA253_INTR1_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_X
        { BMA253_INTR1_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Y
        { BMA253_INTR1_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Z
    };

    if (enable) {
        BMA253_SET_BIT(map_offset[int_type].en_reg, map_offset[int_type].en_offset, ret);
    } else {
        BMA253_CLEAR_BIT(map_offset[int_type].en_reg, map_offset[int_type].en_offset, ret);
    }

    return ret;
}

bool accelerometer_int_map_to_pin2(acc_int_src_type_t int_type, int8_t enable)
{
    int8_t ret;
    struct {
        uint8_t en_reg;
        uint8_t en_offset;
    } map_offset[] = {
        { BMA253_INTR2_PAD_SELECT_ADDR, 7 }, // ACC_INT_SRC_TYPE_FLAT
        { BMA253_INTR2_PAD_SELECT_ADDR, 6 }, // ACC_INT_SRC_TYPE_ORIENT
        { BMA253_INTR2_PAD_SELECT_ADDR, 5 }, // ACC_INT_SRC_TYPE_SIGNAL_TAP
        { BMA253_INTR2_PAD_SELECT_ADDR, 4 }, // ACC_INT_SRC_TYPE_DOUBLE_TAP
        { BMA253_INTR2_PAD_SELECT_ADDR, 2 }, // ACC_INT_SRC_TYPE_SLOPE_X
        { BMA253_INTR2_PAD_SELECT_ADDR, 2 }, // ACC_INT_SRC_TYPE_SLOPE_Y
        { BMA253_INTR2_PAD_SELECT_ADDR, 2 }, // ACC_INT_SRC_TYPE_SLOPE_Z
        { BMA253_INTR_DATA_SELECT_ADDR, 6 }, // ACC_INT_SRC_TYPE_FIFO_WATERMARK
        { BMA253_INTR_DATA_SELECT_ADDR, 5 }, // ACC_INT_SRC_TYPE_FIFO_FULL
        { BMA253_INTR_DATA_SELECT_ADDR, 7 }, // ACC_INT_SRC_TYPE_DATA_READY
        { BMA253_INTR2_PAD_SELECT_ADDR, 0 }, // ACC_INT_SRC_TYPE_LOW_G
        { BMA253_INTR2_PAD_SELECT_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_X
        { BMA253_INTR2_PAD_SELECT_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_Y
        { BMA253_INTR2_PAD_SELECT_ADDR, 1 }, // ACC_INT_SRC_TYPE_HIGH_G_Z
        { BMA253_INTR2_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION
        { BMA253_INTR2_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_X
        { BMA253_INTR2_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Y
        { BMA253_INTR2_PAD_SELECT_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Z
    };

    if (enable) {
        BMA253_SET_BIT(map_offset[int_type].en_reg, map_offset[int_type].en_offset, ret);
    } else {
        BMA253_CLEAR_BIT(map_offset[int_type].en_reg, map_offset[int_type].en_offset, ret);
    }

    return ret;
}

bool accelerometer_int_enable(acc_int_src_type_t int_type, int8_t enable)
{
    int8_t ret;
    struct {
        uint8_t en_reg;
        uint8_t en_offset;
    } interrupt_config[] = {
        { BMA253_INTR_ENABLE1_ADDR, 7 },        // ACC_INT_SRC_TYPE_FLAT
        { BMA253_INTR_ENABLE1_ADDR, 6 },        // ACC_INT_SRC_TYPE_ORIENT
        { BMA253_INTR_ENABLE1_ADDR, 5 },        // ACC_INT_SRC_TYPE_SIGNAL_TAP
        { BMA253_INTR_ENABLE1_ADDR, 4 },        // ACC_INT_SRC_TYPE_DOUBLE_TAP
        { BMA253_INTR_ENABLE1_ADDR, 0 },        // ACC_INT_SRC_TYPE_SLOPE_X
        { BMA253_INTR_ENABLE1_ADDR, 1 },        // ACC_INT_SRC_TYPE_SLOPE_Y
        { BMA253_INTR_ENABLE1_ADDR, 2 },        // ACC_INT_SRC_TYPE_SLOPE_Z
        { BMA253_INTR_ENABLE2_ADDR, 6 },        // ACC_INT_SRC_TYPE_FIFO_WATERMARK
        { BMA253_INTR_ENABLE2_ADDR, 5 },        // ACC_INT_SRC_TYPE_FIFO_FULL
        { BMA253_INTR_ENABLE2_ADDR, 4 },        // ACC_INT_SRC_TYPE_DATA_READY
        { BMA253_INTR_ENABLE2_ADDR, 3 },        // ACC_INT_SRC_TYPE_LOW_G
        { BMA253_INTR_ENABLE2_ADDR, 0 },        // ACC_INT_SRC_TYPE_HIGH_G_X
        { BMA253_INTR_ENABLE2_ADDR, 1 },        // ACC_INT_SRC_TYPE_HIGH_G_Y
        { BMA253_INTR_ENABLE2_ADDR, 2 },        // ACC_INT_SRC_TYPE_HIGH_G_Z
        { BMA253_INTR_SLOW_NO_MOTION_ADDR, 3 }, // ACC_INT_SRC_TYPE_SLOW_MOTION
        { BMA253_INTR_SLOW_NO_MOTION_ADDR, 0 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_X
        { BMA253_INTR_SLOW_NO_MOTION_ADDR, 1 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Y
        { BMA253_INTR_SLOW_NO_MOTION_ADDR, 2 }, // ACC_INT_SRC_TYPE_SLOW_MOTION_Z
    };

    if (enable) {
        BMA253_SET_BIT(interrupt_config[int_type].en_reg, interrupt_config[int_type].en_offset, ret);
    } else {
        BMA253_CLEAR_BIT(interrupt_config[int_type].en_reg, interrupt_config[int_type].en_offset, ret);
    }

    return ret;
}

bool accelerometer_int_status(uint32_t *int_status)
{
    uint8_t int_stat_1, int_stat_2, int_stat_3, int_stat_4;

    if (bma253_read_byte(BMA253_STAT1_ADDR, &int_stat_1) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_STAT2_ADDR, &int_stat_2) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_STAT_TAP_SLOPE_ADDR, &int_stat_3) == false)
        goto ERR;
    if (bma253_read_byte(BMA253_STAT_ORIENT_HIGH_ADDR, &int_stat_4) == false)
        goto ERR;

    *int_status = ((uint32_t)(int_stat_4 & 0xff) << 24) | ((uint32_t)(int_stat_3 & 0xff) << 16) |
                  ((uint32_t)(int_stat_2 & 0xff) << 8) | ((uint32_t)(int_stat_1 & 0xff));

    return true;
ERR:
    return false;
}

/*
 * delay_time: actual delay = (delay_time + 1)*2ms
 * low_g_threashold: actual threadhold = low_g_threshold * 7.81mg
 * low_g_mode: 0 -> signal axis, 1 -> summing axis
 * low_hysteresis: actual low hysteresis = low_hysteresis * 125mg
 */
bool accelerometer_int_low_g_config_set(uint16_t delay_time, uint16_t low_g_threshold, uint8_t low_g_mode,
                                        uint8_t low_hysteresis)
{
    uint8_t data;

    if (bma253_write_byte(BMA253_LOW_DURN_ADDR, delay_time) == false)
        goto ERR;
    if (bma253_write_byte(BMA253_LOW_THRES_ADDR, low_g_threshold) == false)
        goto ERR;

    if (bma253_read_byte(BMA253_LOW_HIGH_HYST_ADDR, &data) == false)
        goto ERR;
    // clear 0-2 bits
    data &= 0xfc;
    data |= (((low_g_mode & 0x01) << 2) | low_hysteresis);
    if (bma253_write_byte(BMA253_LOW_HIGH_HYST_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * delay_time: actual delay = (delay_time + 1)*2ms
 * high_g_threshold: actual threadhold = low_g_threshold * 7.81mg (2-g range)
 *                                                         15.63 mg (4-g range),
 *                                                         31.25 mg (8-g range),
 *                                                         62.5 mg (16-g range)
 * high_hysteresis: actual low hysteresis = low_hysteresis * 125mg
 */
bool accelerometer_int_high_g_config_set(uint16_t delay_time, uint16_t high_g_threshold, uint8_t high_hysteresis)
{
    uint8_t data;

    if (bma253_write_byte(BMA253_HIGH_DURN_ADDR, delay_time) == false)
        goto ERR;
    if (bma253_write_byte(BMA253_HIGH_THRES_ADDR, high_g_threshold) == false)
        goto ERR;

    if (bma253_read_byte(BMA253_LOW_HIGH_HYST_ADDR, &data) == false)
        goto ERR;
    // clear 6-7 bits
    data &= 0x3f;
    data |= (high_hysteresis << 6);
    if (bma253_write_byte(BMA253_LOW_HIGH_HYST_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * slope_dur: slope interrupt triggers if slope_dur+1 consecutive slope data points
 *            are above the slope_threshold
 * slope_threshold: actual slope threshold = slope_threshold * 3.91 mg (2-g range)
 *                                                             7.81 mg (4-g range)
 *                                                             15.63 mg (8-g range)
 *                                                             31.25 mg (16-g range)
 */
bool accelerometer_int_slope_config_set(uint8_t slope_dur, uint8_t slope_threshold)
{
    uint8_t data;

    if (slope_dur > 3)
        goto ERR;

    if (bma253_read_byte(BMA253_SLOPE_DURN_ADDR, &data) == false)
        goto ERR;
    // clear 0-1 bits
    data &= 0x03;
    data |= slope_dur;
    if (bma253_write_byte(BMA253_SLOPE_DURN_ADDR, data) == false)
        goto ERR;

    if (bma253_write_byte(BMA253_SLOPE_THRES_ADDR, slope_threshold) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * slow_no_motion_threshold: actual slow/no motion threshold = slow_no_threshold * 3.91 mg (2-g range)
 *                                                                               7.81 mg (4-g range)
 *                                                                               15.63 mg (8-g range)
 *                                                                               31.25 mg (16-g range)
 */
bool accelerometer_int_slow_config_set(uint8_t slow_no_dur, uint8_t slow_no_threshold)
{
    uint8_t data;

    // TODO : slow no dur how to define
    if (slow_no_dur > 3)
        goto ERR;

    if (bma253_read_byte(BMA253_SLOPE_DURN_ADDR, &data) == false)
        goto ERR;
    // clear 2-7 bits
    data &= 0xfc;
    data |= (slow_no_dur << 2);
    if (bma253_write_byte(BMA253_SLOPE_DURN_ADDR, data) == false)
        goto ERR;

    if (bma253_write_byte(BMA253_SLOW_NO_MOTION_THRES_ADDR, slow_no_threshold) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * quiet: selects a tap quiet duration. 0 -> 30ms, 1 -> 20ms
 * shock: selects a tap shock duration. 0 -> 50ms, 1 -> 75ms
 * dur: selects the length of the time window for the second shock event for double tap detection.
 *      0 -> 50ms, 1 -> 100ms, 2 -> 150ms, 3 -> 200ms, 4 -> 250ms, 5 -> 375ms, 6 -> 500ms, 7 -> 700ms
 * samples: selects the number of samples that are processed after wake-up in the low-power mode
 *          0 -> 2 samples, 1 -> 4 samples, 2 -> 8 samples, 3 -> 16 samples.
 * threshold: threshold of the single/double-tap interrupt corresponding to an acceleration difference
 *            threshold * [62.5mg (2g-range), 125mg (4g- range), 250mg (8g-range), 500mg (16g- range)].
 */
bool accelerometer_int_tap_config_set(uint8_t quiet, uint8_t shock, uint8_t dur, uint8_t samples, uint8_t threshold)
{
    uint8_t data = 0;

    if (quiet > 1 || shock > 1 || dur > 7 || samples > 3 || threshold > 0xf)
        goto ERR;

    data |= (quiet << 7) | (shock << 6) | dur;
    if (bma253_write_byte(BMA253_TAP_PARAM_ADDR, data) == false)
        goto ERR;

    data = 0;
    data |= (samples << 6) | threshold;
    if (bma253_write_byte(BMA253_TAP_THRES_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * hysteresis: 1LSB = 62.5mg
 * blocking: 0 -> no blocking
 *           1 -> theta blocking or acceleration in any axis > 1.5g
 *           2 -> theta blocking or acceleration slope in any axis > 0.2 g or
 *                acceleration in any axis > 1.5g
 *           3 -> theta blocking or acceleration slope in any axis > 0.4 g or
 *                acceleration in any axis > 1.5g and value of orient is not stable for at least 100ms
 * mode: 0 -> symmetrical, 1 -> high-asymmetrical, 2 -> low-asymmetrical, 3 -> symmetrical
 * up_down_en: 1 enable, 0, disable
 * block_angle: blocking angle between 0� and 44.8�
 */
bool accelerometer_int_orient_config_set(uint8_t hysteresis, uint8_t blocking, uint8_t mode, uint8_t up_down_en,
                                         uint8_t block_angle)
{
    uint8_t data = 0;

    if (hysteresis > 7 || blocking > 3 || mode > 3 || up_down_en > 1 || block_angle > 0x3f)
        goto ERR;

    data |= (hysteresis << 4) | (blocking << 2) | mode;
    if (bma253_write_byte(BMA253_ORIENT_PARAM_ADDR, data) == false)
        goto ERR;

    data = 0;
    data |= (up_down_en << 6) | block_angle;
    if (bma253_write_byte(BMA253_THETA_BLOCK_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

/*
 * angle: angle between 0� and 44.8�
 * hold_time: 0 -> 0ms, 1 -> 512ms, 2 -> 1024ms, 3 -> 2048ms
 * hysteresis: 0 -> disable
 */
bool accelerometer_int_flat_config_set(uint8_t angle, uint8_t hold_time, uint8_t hysteresis)
{
    uint8_t data = 0;

    if (hold_time > 3 || hysteresis > 7 || angle > 0x3f)
        goto ERR;

    data |= angle;
    if (bma253_write_byte(BMA253_THETA_FLAT_ADDR, data) == false)
        goto ERR;

    data = 0;
    data |= (hold_time << 4) | hysteresis;
    if (bma253_write_byte(BMA253_FLAT_HOLD_TIME_ADDR, data) == false)
        goto ERR;

    return true;
ERR:
    return false;
}

#define READ_TIMES 10

static ErrorStatus BMA253_SetParameter(void)
{
    bool ret;
    uint8_t get;
    uint8_t error_cnt = 0;
    uint16_t cnt      = 0;

    BMA_DelayMs(10);
    if (accelerometer_soft_reset() == false) {
        PRINTF("bma253 soft reset error!\r\n");
    }
    BMA_DelayMs(10);

    if (accelerometer_probe() == false) {
        PRINTF("acc probe false!\r\n");
    } else {
        PRINTF("acc probe true!\r\n");
    }

    cnt = 0;
    acc_power_mode_t mode;
    do {
        accelerometer_power_mode_set(ACC_POWER_MODE_NORMAL);
        BMA_DelayMs(1);
        accelerometer_power_mode_get(&mode);
    } while (mode != ACC_POWER_MODE_NORMAL && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    acc_filter_bandwidth_t bw;
    do {
        accelerometer_filter_bandwidth_set(ACC_FILTER_BANDWIDTH_500_HZ);
        BMA_DelayMs(1);
        accelerometer_filter_bandwidth_get(&bw);
    } while (bw != ACC_FILTER_BANDWIDTH_500_HZ && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    acc_int_output_type_level_t level;
    do {
        accelerometer_int_out_ctrl(ACC_INT_PIN_1, ACC_INT_PIN1_PUSH_PULL_OUT_HIGH);
        BMA_DelayMs(1);
        accelerometer_int_out_ctrl_get(ACC_INT_PIN_1, &level);
    } while (level != ACC_INT_PIN1_PUSH_PULL_OUT_HIGH && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    do {
        ret = accelerometer_rst_intr();
    } while (ret != true && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    acc_int_latch_type_t latch;
    do {
        accelerometer_int_mode_set(1, ACC_INT_LATCH_DURN_1MS);
        BMA_DelayMs(1);
        accelerometer_int_mode_get(&latch);
    } while (latch != ACC_INT_LATCH_DURN_1MS && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    acc_accel_range_t accel;
    do {
        accelerometer_range_set(ACC_RANGE_2G);
        BMA_DelayMs(1);
        accelerometer_range_get(&accel);
    } while (accel != ACC_RANGE_2G && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    acc_fifo_mode_t fifo;
    acc_fifo_data_type_t type;
    do {
        accelerometer_fifo_mode_set(ACC_FIFO_MODE_STREAM, ACC_FIFO_DATA_TYPE_X_Y_Z);
        BMA_DelayMs(1);
        accelerometer_fifo_mode_get(&fifo, &type);
    } while (((fifo != ACC_FIFO_MODE_STREAM) || (type != ACC_FIFO_DATA_TYPE_X_Y_Z)) && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    do {
        accelerometer_fifo_watermark_level_set(FIFO_DEPTH);
        BMA_DelayMs(1);
        accelerometer_fifo_watermark_level_get(&get);
    } while (get != FIFO_DEPTH && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    do {
        ret = accelerometer_int_enable(ACC_INT_SRC_TYPE_FIFO_WATERMARK, 1);
    } while (ret != true && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    do {
        ret = accelerometer_int_map_to_pin1(ACC_INT_SRC_TYPE_FIFO_WATERMARK, 1);
    } while (ret != true && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    acc_sleep_durn_t durn;
    do {
        accelerometer_set_sleep_durn(ACC_SLEEP_DURN_25MS);
        BMA_DelayMs(1);
        accelerometer_get_sleep_durn(&durn);
    } while (durn != ACC_SLEEP_DURN_25MS && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    do {
        accelerometer_set_sleep_timer_mode(1); /* EST mode */
        BMA_DelayMs(1);
        accelerometer_get_sleep_timer_mode(&get);
    } while (get != 1 && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    cnt = 0;
    acc_power_mode_t powerMode;
    do {
        accelerometer_power_mode_set(ACC_POWER_MODE_LOW_POWER1);
        BMA_DelayMs(1);
        accelerometer_power_mode_get(&powerMode);
    } while (powerMode != ACC_POWER_MODE_LOW_POWER1 && (++cnt < READ_TIMES));
    if (cnt >= READ_TIMES) {
        error_cnt++;
    }

    PRINTF("error_count:%d\r\n", error_cnt);

    return (error_cnt != 0) ? (ERROR) : (SUCCESS);
}

void BMA253_Init(void)
{
    BMA253_I2cInit();
    if (BMA253_SetParameter() == ERROR) {
        PRINTF("bma253 init error!\r\n");
    } else {
        PRINTF("bma253 init success!\r\n");
    }
}

#endif
