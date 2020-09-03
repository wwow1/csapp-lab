#include "cachelab.h"
#include <getopt.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
struct _cacheInfo_{
    int S;  //组数
    int E;  //行数
    int B;  //块大小（字节）
    int m;  //物理地址位数，64
    int s_bit;  //组索引位数
    int b_bit;  //块偏移位数
    int t_bit;  //标记位数
}cacheInfo;
int verbose_flag=0;
FILE* fp;
int hit=0;
int miss=0;
int evicts=0;
long** cache;
bool** mark;
int** time_tag;
int time=0;

void print_help_info(){
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\nOptions:\n  -h         Print this help message.\n  -v         Optional verbose flag.\n  -s <num>   Number of set index bits.\n  -E <num>   Number of lines per set.\n  -b <num>   Number of block offset bits.\n  -t <file>  Trace file.\n\nExamples:\n  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}
/*
void printSummary(int hit,int miss,int evicts){
    printf("hits:%d misses=%d evictions=%d\n",hit,miss,evicts);
}
*/

long tolong(char c){
    if(isdigit(c))
        return c-'0';
    else
        return c-'a'+10;
}

long get_addr(char* s){
    int i=0;
    long addr=0;
    while(s[i]!=','){
        addr<<=4;
        addr+=tolong(s[i]);
        i++;
    }
    return addr;
}

void LRU_replacement(long saddr,long taddr,char* cacheState){
    for(int i=0;i<cacheInfo.E;i++){ 
        if(!mark[saddr][i]){
            cache[saddr][i]=taddr;
            mark[saddr][i]=true;
            time_tag[saddr][i]=time++;  //时间标记，在驱逐的时候会用到它
            return;
        }
    }
    int minTime=0x7fffffff;
    int minIndex=0;
    for(int i=0;i<cacheInfo.E;i++){   //cache已满，需要驱逐
        int stamp=time_tag[saddr][i];
        if(stamp<minTime){
            minTime=stamp;
            minIndex=i;
        }
    }
    cache[saddr][minIndex]=taddr;
    time_tag[saddr][minIndex]=time++;
    evicts++;
    strcat(cacheState," evicts");
}

void process_request(){
    char buffer[100];
    char type;
    long addr;
    char cacheState[100]={0};
    while(fgets(buffer,100,fp)!=NULL){
        buffer[strlen(buffer)-1]='\0';
        //printf("%s\n",buffer);
        int i=0;
        strcpy(cacheState,buffer);
        if(buffer[0]!=' ')  continue;
        type=buffer[1];
        addr=get_addr(buffer+3);
        //printf("addr=%ld   ",addr);
        long saddr=(((long)1)<<(cacheInfo.b_bit+cacheInfo.s_bit))-1;
        long taddr=~saddr;
        saddr=(saddr & addr)>>cacheInfo.b_bit;  //取得组索引
        taddr=(taddr & addr)>>(cacheInfo.b_bit+cacheInfo.s_bit); //取得标记位
        //printf("saddr=%ld taddr=%ld  ",saddr,taddr);
        for(i=0;i<cacheInfo.E;i++){  
            if(mark[saddr][i] && cache[saddr][i]==taddr){  //对应数据已经在cache中
                hit++;
                time_tag[saddr][i]=time++;  //更新时间戳
                strcat(cacheState," hit");
                break;
            }
        }
        if(i==cacheInfo.E){   //数据不在cache中，替换算法使用LRU
            //printf("buf=%s LRU ",buffer);
            miss++;
            strcat(cacheState," miss");
            LRU_replacement(saddr,taddr,cacheState);
        }
        if(type=='M'){
            hit++;
            strcat(cacheState," hit");
        }

        if(verbose_flag){  //输出详细信息
            printf("%s\n",cacheState);
        }
    }
}


int main(int argc,char* argv[])
{
    int ch;
    cacheInfo.m=64;
    while((ch=getopt(argc,argv,"hvs:E:b:t:"))!=-1){
        switch(ch){
            case 'h':
                print_help_info();
                return 0;
            case 'v':
                verbose_flag=1;
                break;
            case 's':
                cacheInfo.s_bit=atoi(optarg);
                cacheInfo.S=1<<cacheInfo.s_bit;
                break;
            case 'E':
                cacheInfo.E=atoi(optarg);
                break;
            case 'b':
                cacheInfo.b_bit=atoi(optarg);
                cacheInfo.B=1<<cacheInfo.b_bit;
                break;
            case 't':
                fp=fopen(optarg,"r");
                break;
            default:
                printf("Unknown option: %c\n",(char)optopt);
                break;
        }
    }
    cacheInfo.t_bit=cacheInfo.m-(cacheInfo.s_bit+cacheInfo.b_bit);
    printf("E=%d,S=%d\n",cacheInfo.E,cacheInfo.S);
    cache=(long **)malloc(sizeof(long*)*cacheInfo.S);  //分配地址空间模拟cache
    mark=(bool **)malloc(sizeof(bool*)*cacheInfo.S);   //对应每一组内每一行的有效位
    time_tag=(int **)malloc(sizeof(int*)*cacheInfo.S); //时间标记
    for(int i=0;i<cacheInfo.S;i++){                    //初始化
        cache[i]=(long*)malloc(sizeof(long)*cacheInfo.E);
        mark[i]=(bool*)malloc(sizeof(bool)*cacheInfo.E);
        time_tag[i]=(int*)malloc(sizeof(int)*cacheInfo.E);
    }
    for(int i=0;i<cacheInfo.S;i++){
        for(int j=0;j<cacheInfo.E;j++)
            mark[i][j]=false;
    }
    process_request();      //处理内存操作
    printSummary(hit, miss, evicts);
    free(cache);
    free(mark);
    free(time_tag);
    fclose(fp);
    return 0;
}
