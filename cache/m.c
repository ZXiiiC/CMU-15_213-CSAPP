#include <stdbool.h>
#include <stdio.h>
union bit{
	int it;
	bool bl[32];
};
int main(){
	printf("size of bool = %ld\n",sizeof(bool));
	union bit b;
	b.it=10;
	for(int i=0;i<32;i++)
		printf("%d\n",b.bl[i]);
	return 0;
}
