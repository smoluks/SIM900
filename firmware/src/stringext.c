#include <stdbool.h>

//сравнение строк с разными длинами
bool strpartcmp(char* first, char* second)
{
	while(*first && *second && (*first == *second))
	{
		first++;
		second++;
	}
	return (!*first || !*second);
}
