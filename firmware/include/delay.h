#pragma once

#define uS 9

#define delay(us) \
	({  \
	uint32_t i = us * uS;  \
	while ((i--) != 0);  \
	})
