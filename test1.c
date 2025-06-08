#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
	const char* tmp1 = "96x192";
	int a=48;
	int b=48;
	for (int i=0; i<strlen(tmp1); ++i) {
		if (tmp1[i] == 'x') {
			char* tmp2 = (char*) calloc(i+1, sizeof(char));
			char* tmp3 = (char*) calloc(strlen(tmp1)-i, sizeof(char));	
			memcpy(tmp2, tmp1, i);
			for (int j=(i+1); j<strlen(tmp1); ++j) {
				tmp3[j-i-1] = tmp1[j];
			}
			
			a = atoi(tmp2);
			b = atoi(tmp3);
			break;
		}
	}
	
	printf("a = %d\n", a);
	printf("b = %d\n", b);
	
	return 0;
}
