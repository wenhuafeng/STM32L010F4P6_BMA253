
#ifndef __STEP_PROCESS_H__
#define __STEP_PROCESS_H__

struct AccelDevice
{
  int MaxThreshold[3], MinThreshold[3], MidleThreshold[3];
  //int32 hightCnt[3], lowCnt[3];
  int Counter[3], iCounter[3]/*, toMidleLines[3]*/;
  int allCounter, oCounter;
  //int WaveCounter, WaveAmplitudeSum;
};

extern struct AccelDevice accelDevice;

extern void AccelHandle(void);

#endif

