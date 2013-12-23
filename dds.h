#ifndef _DDS_H_
#define _DDS_H_

#define F_DDS (125000000LL)

void dds_init(void);
void dds_reset(void);
void dds_power_down(void);
void dds_set(uint32_t freq);

#endif // _DDS_H_
