#include <linux/io.h>
#include "tdmb.h"

#define TDMB_EBI2_CS_PA 0x8C000000 // EBI2_CS1(0x8C000000) on MSM7627A 
void *tdmb_ebi2_cs_va;

int tdmb_init_bus(void)
{
	tdmb_ebi2_cs_va = ioremap(TDMB_EBI2_CS_PA, PAGE_SIZE);
	DPRINTK("TDMB EBI2 Init tdmb_ebi2_cs_va(0x%x)\n", tdmb_ebi2_cs_va);
	return 0;
}

void tdmb_exit_bus(void)
{
	tdmb_ebi2_cs_va = NULL;
}
