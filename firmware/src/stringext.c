#include <stdbool.h>

bool strpartcmp(char* first, char* second)
{
	while(*first && *second && (*first == *second))
	{
		first++;
		second++;
	}
	return (!*first || !*second);
}
