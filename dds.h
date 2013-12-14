#ifndef _DDS_H_
#define _DDS_H_

#define F_DDS (125000000LL)
#define DDS_MOD (1LL << 32)
#define DELTA_DDS(f) ((DDS_MOD * (f)) / F_DDS)

void dds_init(void);
void dds_reset(void);
void dds_power_down(void);
void dds_set(uint32_t delta);

#endif // _DDS_H_
