#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

//#define DEBUG 0;

int hit_count=0, miss_count=0, eviction_count=0;
int sBit=0, eNum=0, bBit=0,setNum=0;
bool visTag=0;
char *filename=NULL;
int* valid=NULL;

extern int optind,opterr,optopt;
extern char *optarg;

void readArgs(int argc, char *argv[]);
void callHelp();
void argError();
int getopt(int argc,char *const argv[],const char *optstring);
void readInpt(FILE* fp,int* cache);
int* InitCache();
void Load(int* cache,int tag,int sAdr,int bAdr);
void Store(int* cache,int tag,int sAdr,int bAdr);
void Modify(int* cache,int tag,int sAdr,int bAdr);
//char*[] split(char* str,char* delim);
void parseAdr(long address,int* ptag,int* psAdr,int* pbAdr);
void memoryAccess(int* cache,int tag,int sAdr);
void orderSet(int* cache,int sAdr,int e);

int main(int argc, char *argv[])
{
	#ifdef DEBUG
    printf("argc=%d\n",argc);
	#endif
	readArgs(argc,argv);
	int*cache=InitCache();
	if(cache==NULL){
		printf("Init cache error!\n");
		exit(1);
	}
	FILE* fp=fopen(filename,"r");
	#ifdef DEBUG
		if(fp) printf("open file success fp = %p\n",fp);
		else   printf("open file error\n");
	#endif
	readInpt(fp,cache);	
	printSummary(hit_count, miss_count, eviction_count);

	return 0;
}

void readArgs(int argc, char *argv[]){

	#ifdef DEBUG
    printf("In funcRA argc=%d\n",argc);
	#endif
	if (argc!=2&&argc!=9&&argc!=10&&argc!=11){
		argError();
	}
	
	int c = 0; //用于接收选项
    /*循环处理参数*/
    while(EOF != (c = getopt(argc,argv,"hvs:E:b:t:")))
    {

		switch(c)
        {
            case 'h':
                callHelp();
				exit(0);
                break;
            case 'v':
				visTag=1;break; 
            case 's':
				sBit=atoi(optarg); setNum=1<<sBit; break;
			case 'E':
				eNum=atoi(optarg); break;
			case 'b':
				bBit=atoi(optarg); break;
			case 't':
				filename=optarg;   break;
            //表示选项不支持
            case '?':
				argError();        break;
            default:
                break;
        }    
    }
}

void callHelp(){
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\nOptions:\n  -h         Print this help message.\n  -v         Optional verbose flag.\n  -s <num>   Number of set index bits.\n  -E <num>   Number of lines per set.\n  -b <num>   Number of block offset bits.\n  -t <file>  Trace file.\n\nExamples:\n  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
	return ;
}

void argError(){
	printf("./csim-ref: Missing required command line argument\n");
	callHelp();	
	exit(1);
}

void readInpt(FILE* fp,int* cache){
	
	char operation;
    long address;
    int size;
	
	
	while (fscanf(fp," %c %lx,%d",&operation,&address,&size)>0)
	{
		#ifdef DEBUG
			printf("read line : %c %lx,%d\n",operation,address,size);
		#endif
		
		int tag=0,sAdr=0,bAdr=0;
		parseAdr(address,&tag,&sAdr,&bAdr);
		
		switch (operation)
		{
		case 'I':
			
			continue;
		case 'L':
			if(visTag) printf("%c %lx,%d",operation,address,size);
			//Load(cache,tag,sAdr,bAdr);
			memoryAccess(cache,tag,sAdr);  
			if(visTag) printf("\n"); 
			break;
		case 'S':
			if(visTag) printf("%c %lx,%d",operation,address,size);
			//Store(cache,tag,sAdr,bAdr); 
			memoryAccess(cache,tag,sAdr); 
			if(visTag) printf("\n"); 
			break;
		case 'M':
			if(visTag) printf("%c %lx,%d",operation,address,size);
			//Modify(cache,tag,sAdr,bAdr); 
			memoryAccess(cache,tag,sAdr);
			memoryAccess(cache,tag,sAdr);  
			if(visTag) printf("\n");
			break;
		default:
			printf("error operation : %c\n",operation);
			exit(1);
		}

	}
	
}

int* InitCache(){
	if(sBit==0||bBit==0||eNum==0) return NULL;
	int cacheSize=eNum*setNum;
	int* cache=malloc(sizeof(int)*cacheSize);
	valid=malloc(sizeof(int)*cacheSize);
	for(int i=0;i<cacheSize;i++){
		cache[i]=0;
		valid[i]=0;
	}
	#ifdef DEBUG
		printf("Init cache success, cache size = %d\n",cacheSize);
	#endif
	return cache;
}

void orderSet(int* cache,int sAdr,int e){
	if(eNum==1) return ;
	if(valid[sAdr+(eNum-1)*setNum]==eNum){
		int save=cache[sAdr+(eNum-1)*setNum];
		for(int i=eNum-1;i>0;i--)
			cache[i]=cache[i-1];
		cache[0]=save;
		return;
	}
	else{
		int save=cache[sAdr+e*setNum];
		while(valid[sAdr+(--e)*setNum]!=0){
			cache[sAdr+(e+1)*setNum]=cache[sAdr+e*setNum];
		}
		cache[sAdr+(e+1)*setNum]=save;
		return;
	}
}

void memoryAccess(int* cache,int tag,int sAdr){
	int e=eNum,curAdr;
	if(valid[sAdr+(eNum-1)*setNum]!=eNum){
		while(--e>=0){
			curAdr=setNum*e+sAdr;
			if(valid[curAdr]==0){//cold miss
				valid[curAdr]=1;
				cache[curAdr]=tag;
				if(visTag) printf(" miss");
				miss_count++;
				valid[sAdr+(eNum-1)*setNum]++;
				return ;
			}
			else if(cache[curAdr]==tag){//hit
				if(visTag) printf(" hit");
				hit_count++;
				orderSet(cache,sAdr,e);
				return ;
			}
		
		}
	}
	else{//eviction
		
		if(visTag) printf(" miss eviction");
		miss_count++;
		eviction_count++;
		cache[sAdr+(eNum-1)*setNum]=tag;
		orderSet(cache,sAdr,e);
		return ;
	}
}

void Load(int* cache,int tag,int sAdr,int bAdr){

	int e=0;
	int curAdr=0;
	while(e<eNum){
		curAdr=setNum*(e++)+sAdr;
		if(valid[curAdr]==0){
			valid[curAdr]=1;
			cache[curAdr]=tag;
			if(visTag) printf(" miss");
			miss_count++;
			return ;
		}
		else if(cache[curAdr]==tag){
			if(visTag) printf(" hit");
			hit_count++;
			return ;
		}
	}

	if(visTag) {
		printf(" miss");
		printf(" eviction");
	}
	cache[curAdr]=tag;
	eviction_count++;
	miss_count++;
}

void Store(int* cache,int tag,int sAdr,int bAdr){

	int e=0;
	int curAdr=0;
	while(e<eNum){
		curAdr=setNum*(e++)+sAdr;
		if(valid[curAdr]==0){
			valid[curAdr]=1;
			cache[curAdr]=tag;
			if(visTag) printf(" miss");
			miss_count++;
			return ;
		}
		else if(cache[curAdr]==tag){
			if(visTag) printf(" hit");
			hit_count++;
			return ;
		}
	}

	if(visTag) {
		printf(" miss");
		printf(" eviction");
	}
	cache[curAdr]=tag;
	eviction_count++;
	miss_count++;
}

void Modify(int* cache,int tag,int sAdr,int bAdr){
	Load(cache,tag,sAdr,bAdr);
	Store(cache,tag,sAdr,bAdr);
	
}

void parseAdr(long address,int* ptag,int* psAdr,int* pbAdr){

	*pbAdr=address%(1<<bBit);
	address>>=bBit;
	*psAdr=address%(1<<sBit);
	address>>=sBit;
	*ptag=address;
	#ifdef DEBUG
	//printf("parse address into tag = %d, sAdr = %d, bAdr = %d\n",*ptag,*psAdr,*pbAdr);
	#endif
}