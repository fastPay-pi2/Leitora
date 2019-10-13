#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "reader_api.h"

int32_t main(void)
{
    readerconfig_t r1;
    int32_t val, arg, i;
    uint8_t version_rf[50], version_digital[50];
	
	
	val = Gen2ReaderCreate("192.168.5.10", &r1);
	if (val)
	{
		printf("Error creating reader session. %d\n", val);
		return val;
	}
	
	// while(1)
	// {
		val = GetVersion(&r1, &version_rf, &version_digital);
		if (val < 0)
		{
			printf("Operation failed! (%d)\n", val);
			return val;
		}
		else
		{
			printf("--- Firmware version ---\n");
			printf("RF: %s\n",version_rf);
			printf("Digital: %s\n", version_digital);
			// return 0;
		}
	// }
}
