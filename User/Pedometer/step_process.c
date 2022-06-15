#include "step_process.h"
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "usart.h"
#include "lptim.h"
#include "vcom.h"
#include "sys_func.h"
#include "bma253.h"

#if (PEDOMETER)

#define MAX_COUNT 3 // x.y.z
#define FILTER_NR 9
#define MIN_EFFECTIVE_STEPS 9

#define TASK_STEP_CLEAR_TIME_1 (1500 / 31.25)
#define TASK_STEP_CLEAR_TIME_2 (2200 / 31.25)

struct AccelDevice {
    int32_t max[MAX_COUNT];
    int32_t mid[MAX_COUNT];
    int32_t min[MAX_COUNT];
    uint32_t counter[MAX_COUNT];
    uint32_t iCounter[MAX_COUNT];
    uint32_t allCounter;
    uint32_t oCounter;
};

static struct AccelDevice g_accelDevice;
static struct bma2x2_accel_data_fifo g_accelFifo[FIFO_DEPTH];
static int32_t g_filterArr[MAX_COUNT][FILTER_NR];

static bool g_gsensorInt;
static bool g_stepStatus;
static uint16_t g_mTicks = 0;
static uint32_t g_counter;
static uint8_t g_taskStepClearCount;

static int32_t AccelFilter(uint8_t idx, int32_t value)
{
    uint8_t i;
    int32_t sum = 0;

    for (i = 0; i < FILTER_NR - 1; i++) {
        g_filterArr[idx][i] = g_filterArr[idx][i + 1];
        sum += g_filterArr[idx][i + 1];
    }

    g_filterArr[idx][FILTER_NR - 1] = value;
    sum += value;

    return (sum / FILTER_NR);
}

static int32_t FindMaxInt(int32_t *data, uint8_t len)
{
    uint8_t i;
    int32_t max = data[0];

    for (i = 1; i < len; i++) {
        if (data[i] > max) {
            max = data[i];
        }
    }

    return max;
}

static int32_t FindMinInt(int32_t *data, uint8_t len)
{
    uint8_t i;
    int32_t min = data[0];

    for (i = 1; i < len; i++) {
        if (data[i] < min) {
            min = data[i];
        }
    }

    return min;
}

static uint8_t CalcX(unsigned int value)
{
    static int lTicks = 0;
    uint32_t maxMin;
    static int32_t x[5];

    x[0] = x[1];
    x[1] = x[2];
    x[2] = x[3];
    x[3] = x[4];
    x[4] = value;

    /* Find the wave crest */
    if ((x[0] < x[1] && x[1] < x[2] && x[2] > x[3] && x[3] > x[4]) ||
        ((x[0] == x[1]) && (x[1] > x[2] && x[2] > x[3] && x[3] > x[4])) || (x[3] > x[2] && x[3] > x[4])) {
        // g_accelDevice.hightCnt[0]++;
        // g_accelDevice.max[0] = x[1];
        g_accelDevice.max[0] = FindMaxInt(x, 5);
        maxMin = g_accelDevice.max[0] - g_accelDevice.min[0];
        //g_accelDevice.mid[0] =  (int)(g_accelDevice.max[0] - maxMin * 0.2);

        if (((g_mTicks - lTicks) >= 200) && ((g_mTicks - lTicks) <= 2000)) {
        }

        //hTicks = g_mTicks;
    }

    /* Find the wave trough */
    if ((x[0] > x[1] && x[1] > x[2] && x[2] < x[3] && x[3] <= x[4]) ||
        ((x[0] == x[1]) && (x[1] < x[2] && x[2] < x[3] && x[3] <= x[4])) || (x[3] < x[2] && x[3] < x[4])) {
        // g_accelDevice.lowCnt[0]++;
        // g_accelDevice.min[0] = x[1];
        g_accelDevice.min[0] = FindMinInt(x, 5);
        maxMin = g_accelDevice.max[0] - g_accelDevice.min[0];
        //g_accelDevice.mid[0] =  (int)(g_accelDevice.min[0] + maxMin * 0.2);

        if ((((g_mTicks - lTicks) >= 200) && ((g_mTicks - lTicks) <= 2000)) && (maxMin > 35))
        //&& (maxMin > 50))
        {
            g_accelDevice.counter[0]++;
            g_accelDevice.iCounter[0]++;

            uint32_t max = (g_accelDevice.iCounter[1] > g_accelDevice.iCounter[2]) ? (g_accelDevice.iCounter[1]) :
                                                                            (g_accelDevice.iCounter[2]);

            if (g_accelDevice.iCounter[0] < max) {
                g_accelDevice.iCounter[0] = max;
                g_accelDevice.allCounter = max;
            } else {
                g_accelDevice.allCounter = g_accelDevice.iCounter[0];

                // Add for sleep
                //g_accelDevice.WaveCounter++;
                //g_accelDevice.WaveAmplitudeSum += maxMin;
            }
        }

        lTicks = g_mTicks;
    }

    return 0;
}

static uint8_t CalcY(unsigned int value)
{
    static int lTicks = 0;
    uint32_t maxMin;
    static int32_t y[5];

    y[0] = y[1];
    y[1] = y[2];
    y[2] = y[3];
    y[3] = y[4];
    y[4] = value;

    /* Find the wave crest */
    if ((y[0] < y[1] && y[1] < y[2] && y[2] > y[3] && y[3] > y[4]) ||
        ((y[0] == y[1]) && (y[1] > y[2] && y[2] > y[3] && y[3] > y[4])) || (y[3] > y[2] && y[3] > y[4])) {
        // g_accelDevice.hightCnt[1]++;
        // g_accelDevice.max[1] = y[1];
        g_accelDevice.max[1] = FindMaxInt(y, 5);
        maxMin = g_accelDevice.max[1] - g_accelDevice.min[1];
        //g_accelDevice.mid[1] =  (int)(g_accelDevice.max[1] - maxMin * 0.2);

        if (((g_mTicks - lTicks) >= 200) && ((g_mTicks - lTicks) <= 2000)) {
        }

        // hTicks = g_mTicks;
    }

    /* Find the wave trough */
    if ((y[0] > y[1] && y[1] > y[2] && y[2] < y[3] && y[3] <= y[4]) ||
        ((y[0] == y[1]) && (y[1] < y[2] && y[2] < y[3] && y[3] <= y[4])) || (y[3] < y[2] && y[3] < y[4])) {
        // g_accelDevice.lowCnt[1]++;
        // g_accelDevice.min[1] = y[1];
        g_accelDevice.min[1] = FindMinInt(y, 5);
        maxMin = g_accelDevice.max[1] - g_accelDevice.min[1];
        //g_accelDevice.mid[1] =  (int)(g_accelDevice.min[1] + maxMin * 0.2);

        if ((((g_mTicks - lTicks) >= 200) && ((g_mTicks - lTicks) <= 2000)) && (maxMin > 35))
        //&& (maxMin > 50))
        {
            g_accelDevice.counter[1]++;
            g_accelDevice.iCounter[1]++;

            uint32_t max = (g_accelDevice.iCounter[0] > g_accelDevice.iCounter[2]) ? (g_accelDevice.iCounter[0]) :
                                                                            (g_accelDevice.iCounter[2]);

            if (g_accelDevice.iCounter[1] < max) {
                g_accelDevice.iCounter[1] = max;
                g_accelDevice.allCounter = max;
            } else {
                g_accelDevice.allCounter = g_accelDevice.iCounter[1];

                // Add for sleep
                //g_accelDevice.WaveCounter++;
                //g_accelDevice.WaveAmplitudeSum += maxMin;
            }
        }

        lTicks = g_mTicks;
    }

    return 0;
}

static uint8_t CalcZ(unsigned int value)
{
    static int lTicks = 0;
    uint32_t maxMin;
    static int32_t z[5];

    z[0] = z[1];
    z[1] = z[2];
    z[2] = z[3];
    z[3] = z[4];
    z[4] = value;

    /* Find the wave crest */
    if ((z[0] < z[1] && z[1] < z[2] && z[2] > z[3] && z[3] > z[4]) ||
        ((z[0] == z[1]) && (z[1] > z[2] && z[2] > z[3] && z[3] > z[4])) || (z[3] > z[2] && z[3] > z[4])) {
        // g_accelDevice.hightCnt[2]++;
        // g_accelDevice.max[2] = z[1];
        g_accelDevice.max[2] = FindMaxInt(z, 5);
        maxMin = g_accelDevice.max[2] - g_accelDevice.min[2];
        //g_accelDevice.mid[2] =  (int)(g_accelDevice.max[2] - maxMin * 0.2);
        if (((g_mTicks - lTicks) >= 200) && ((g_mTicks - lTicks) <= 2000)) {
        }

        // hTicks = g_mTicks;
    }

    /* Find the wave trough */
    if ((z[0] > z[1] && z[1] > z[2] && z[2] < z[3] && z[3] <= z[4]) ||
        ((z[0] == z[1]) && (z[1] < z[2] && z[2] < z[3] && z[3] <= z[4])) || (z[3] < z[2] && z[3] < z[4])) {
        // g_accelDevice.lowCnt[2]++;
        // g_accelDevice.min[2] = z[1];
        g_accelDevice.min[2] = FindMinInt(z, 5);
        maxMin = g_accelDevice.max[2] - g_accelDevice.min[2];
        //g_accelDevice.mid[2] = (int)(g_accelDevice.min[2] + maxMin * 0.2);
        if ((((g_mTicks - lTicks) >= 200) && ((g_mTicks - lTicks) <= 2000)) && (maxMin > 35))
        //&& (maxMin > 50))
        {
            g_accelDevice.counter[2]++;
            g_accelDevice.iCounter[2]++;

            uint32_t max = (g_accelDevice.iCounter[1] > g_accelDevice.iCounter[0]) ? (g_accelDevice.iCounter[1]) :
                                                                                     (g_accelDevice.iCounter[0]);

            if (g_accelDevice.iCounter[2] < max) {
                g_accelDevice.iCounter[2] = max;
                g_accelDevice.allCounter = max;
            } else {
                g_accelDevice.allCounter = g_accelDevice.iCounter[2];

                // Add for sleep
                //g_accelDevice.WaveCounter++;
                //g_accelDevice.WaveAmplitudeSum += maxMin;
            }
        }

        lTicks = g_mTicks;
    }

    return 0;
}

static uint8_t StepThread(struct bma2x2_accel_data_fifo *accel)
{
    struct bma2x2_accel_data_fifo *pa;
    register int16_t val;

    pa = accel;

    val = pa->x + 512;
    val = AccelFilter(0, val);
    CalcX(val);

    val = pa->y + 512;
    val = AccelFilter(1, val);
    CalcY(val);

    val = pa->z + 512;
    val = AccelFilter(2, val);
    CalcZ(val);

    g_mTicks += 25;

    return 0;
}

static void AccelHandle(void)
{
    register uint8_t i;

#if (FIFO_DEPTH == 16)
    for (i = 0; i < FIFO_DEPTH; i += FIFO_DEPTH / 2) {
        StepThread(&g_accelFifo[i]);
        StepThread(&g_accelFifo[i + 1]);
        StepThread(&g_accelFifo[i + 2]);
        StepThread(&g_accelFifo[i + 3]);
        StepThread(&g_accelFifo[i + 4]);
        StepThread(&g_accelFifo[i + 5]);
        StepThread(&g_accelFifo[i + 6]);
        StepThread(&g_accelFifo[i + 7]);
    }
#elif (FIFO_DEPTH == 20)
    for (i = 0; i < FIFO_DEPTH; i += FIFO_DEPTH / 2) {
        StepThread(&g_accelFifo[i]);
        StepThread(&g_accelFifo[i + 1]);
        StepThread(&g_accelFifo[i + 2]);
        StepThread(&g_accelFifo[i + 3]);
        StepThread(&g_accelFifo[i + 4]);
        StepThread(&g_accelFifo[i + 5]);
        StepThread(&g_accelFifo[i + 6]);
        StepThread(&g_accelFifo[i + 7]);
        StepThread(&g_accelFifo[i + 8]);
        StepThread(&g_accelFifo[i + 9]);
    }
#elif (FIFO_DEPTH == 31)
    for (i = 0; i < FIFO_DEPTH; i += FIFO_DEPTH / 2) {
        StepThread(&g_accelFifo[i]);
        StepThread(&g_accelFifo[i + 1]);
        StepThread(&g_accelFifo[i + 2]);
        StepThread(&g_accelFifo[i + 3]);
        StepThread(&g_accelFifo[i + 4]);
        StepThread(&g_accelFifo[i + 5]);
        StepThread(&g_accelFifo[i + 6]);
        StepThread(&g_accelFifo[i + 7]);
        StepThread(&g_accelFifo[i + 8]);
        StepThread(&g_accelFifo[i + 9]);
    }
    StepThread(&g_accelFifo[30]);
#else
#error "FIFO DEPTH Error"
#endif
}

static void UploadPedometer(void)
{
    static uint32_t counter = 0;

    if (g_stepStatus == 0) {
        /* 至少2秒内要走出一步，且连续十步有效 */
        if ((g_accelDevice.allCounter - g_accelDevice.oCounter) > MIN_EFFECTIVE_STEPS) {
            g_stepStatus = 1;
            g_taskStepClearCount = 0x00;
            goto exit;
        }

        if (counter != g_accelDevice.allCounter) {
            counter = g_accelDevice.allCounter;

            g_taskStepClearCount = TASK_STEP_CLEAR_TIME_1;
            if (!LL_LPTIM_IsEnabled(LPTIM1)) {
                MX_LPTIM1_Init();
                LPTIM1_CounterStartIT();
            }
        }
        goto exit;
    }

    if (g_accelDevice.oCounter != g_accelDevice.allCounter) {
        g_accelDevice.oCounter = g_accelDevice.allCounter;
        g_counter = g_accelDevice.oCounter; /* update pedometer */
        PRINTF("Pedometer:%d\n",g_counter);

        g_taskStepClearCount = TASK_STEP_CLEAR_TIME_2;
        if (!LL_LPTIM_IsEnabled(LPTIM1)) {
            MX_LPTIM1_Init();
            LPTIM1_CounterStartIT();
        }
    }

exit:
    __asm("NOP");
}

static void accel_filter_init(void)
{
    uint8_t i;
    uint8_t j;

    for (i = 0; i < MAX_COUNT; i++) {
        for (j = 0; j < FILTER_NR; j++) {
            g_filterArr[i][j] = 0;
        }
    }
}

static void ReadFifoData(void)
{
    bool ret;
    uint8_t i;
    const char *buf;

    ret = accelerometer_fifo_data_read(g_accelFifo, sizeof(g_accelFifo));
    if (ret == false) {
        PRINTF("read accelerometer fifo error!\r\n");
        return;
    }

    for (i = 0; i < FIFO_DEPTH; i++) {
        buf = (char *)&g_accelFifo[i];

        g_accelFifo[i].x = (int16_t)((((int32_t)((int8_t)buf[1])) << C_BMA2x2_EIGHT_uint8_tX) |
                                    (buf[0] & BMA2x2_10_BIT_SHIFT));
        g_accelFifo[i].x = g_accelFifo[i].x >> C_BMA2x2_SIX_uint8_tX;

        g_accelFifo[i].y = (int16_t)((((int32_t)((int8_t)buf[3])) << C_BMA2x2_EIGHT_uint8_tX) |
                                    (buf[2] & BMA2x2_10_BIT_SHIFT));
        g_accelFifo[i].y = g_accelFifo[i].y >> C_BMA2x2_SIX_uint8_tX;

        g_accelFifo[i].z = (int16_t)((((int32_t)((int8_t)buf[5])) << C_BMA2x2_EIGHT_uint8_tX) |
                                    (buf[4] & BMA2x2_10_BIT_SHIFT));
        g_accelFifo[i].z = g_accelFifo[i].z >> C_BMA2x2_SIX_uint8_tX;
    }
}

static void TaskStepFunc(void)
{
    if (g_gsensorInt == false) {
        return;
    }
    g_gsensorInt = false;

    ReadFifoData();
    AccelHandle();
    UploadPedometer();
    accelerometer_rst_intr();
}

static void TaskStepClearCountdown(void)
{
    uint8_t i;

    if (Get32HzFlag() == false) {
        return;
    }
    Set32HzFlag(false);

    if (g_taskStepClearCount) {
        g_taskStepClearCount--;
        if (g_taskStepClearCount == 0x00) {
            g_accelDevice.allCounter = g_accelDevice.oCounter;
            for (i = 0; i < MAX_COUNT; i++) {
                g_accelDevice.counter[i] = g_accelDevice.oCounter;
                g_accelDevice.iCounter[i] = g_accelDevice.oCounter;
            }

            g_stepStatus = 0;
            LL_LPTIM_Disable(LPTIM1);
        }
    }
}

void SetGsensorIntFlag(bool flag)
{
    g_gsensorInt = flag;
}

uint32_t GetPedometer(void)
{
    return g_counter;
}

void TaskStepInit(void)
{
    memset(&g_accelDevice, 0, sizeof(struct AccelDevice));
    memset(&g_accelFifo, 0, sizeof(struct bma2x2_accel_data_fifo));
    accel_filter_init();
}

void TaskStep(void)
{
    TaskStepFunc();
    TaskStepClearCountdown();
}
#else

void SetGsensorIntFlag(bool flag)
{
}

uint32_t GetPedometer(void)
{
    return 0;
}

void TaskStepInit(void)
{
}

void TaskStep(void)
{
}

#endif
