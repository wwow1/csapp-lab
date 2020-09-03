/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define MAX(a,b) ((a) > (b) ? (a) : (b) )
#define WSIZE  4
#define DSIZE  8
#define CHUNKSIZE ((1<<12))
#define MIN_BLK 16

#define PACK(size,alloc) ((size) | (alloc))
#define GET(p)  (*(unsigned int*)(p))
#define PUT(p,val)  (*(unsigned int*)(p) = (val))
#define PUT_PTR(p,ptr)  ( *(unsigned int*)(p)=(unsigned int)(ptr) )
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

#define HDRP(bp)    ((char*)(bp)-WSIZE)
#define FTRP(bp)    ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

#define NEXT_LIST_BLKP(bp) (char*) (* ( (unsigned int*)(bp+WSIZE) ) )
#define PREV_LIST_BLKP(bp) (char*) (* ( (unsigned int*)(bp) ) )

#define NEXT_BLKP(bp) ((char*)(bp)+GET_SIZE(((char*)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE(((char*)(bp)-DSIZE)))

char* find_fit(size_t size);
void* extend_heap(size_t size);
void place(char* bp,size_t size);
void insert_into_list(char* bp,size_t size);
void* coalesce(char* bp);
void delete_from_list(char* bp);
int mm_check();

static char* heap_listp=NULL;     //指向空闲链表头(伪头节点)
static char* heap_table_p=NULL;   //指向链表数组的第一个元素
int request_no=0;
int free_no=0;
//分离适配+按空闲块大小排序+首次适应+立即合并
//当该块被分配后，指向后继节点和后继节点指针的内存也将作为有效载荷

/*
 * get_listp - 给定所需要的块的大小，返回对应大小的空闲链指针
 */

char* get_listp(size_t size){
    int index=0;
    if(size>=4096)
        return heap_table_p+(10*WSIZE);
    while(size){
        size>>=1;
        index++;
    }
    return heap_table_p+(index-3)*WSIZE;
}

/*
 * extend_heap - 扩展堆的大小，请求一个新的内存块,成功返回新的空闲块指针，失败返回NULL
 */
void* extend_heap(size_t size){
    char* newblk;
    if((newblk=mem_sbrk(size)) == (void*)-1){
        printf("error:mem_sbrk(%d) fails!\n",size);
        return NULL;
    }
    //printf("extend_heap: newblk=%lx\n",(unsigned long)newblk);
    
    PUT(HDRP(newblk),PACK(size,0)); //分配一页大小的空闲块
    PUT(FTRP(newblk),PACK(size,0));

    PUT(HDRP(NEXT_BLKP(newblk)),PACK(0,1)); //迁移结束块
    
    
    insert_into_list(newblk,size);

    return coalesce(newblk); //也许原堆的最后一块处于空闲状态，这样我们就能够与之合并
}

/* 
 * mm_init - initialize the malloc package.
 */
 //分配一个起始的大的空闲块
 //由于是地址排序的显式空闲链表，所以不需要终止块，脚部也不需要
 //但是为了方便，需要一个伪头节点
int mm_init(void) 
{
    if((heap_listp=mem_sbrk(14*WSIZE)) == (void*)-1){
        printf("error:mem_sbrk(%d) fails!\n",CHUNKSIZE);
        return 0;
    }

    PUT_PTR(heap_listp,NULL); //1-7
    PUT_PTR(heap_listp+WSIZE,NULL);  //8-15
    PUT_PTR(heap_listp+(2*WSIZE),NULL);  //16-31
    PUT_PTR(heap_listp+(3*WSIZE),NULL);  //32-63
    PUT_PTR(heap_listp+(4*WSIZE),NULL);  //64-127
    PUT_PTR(heap_listp+(5*WSIZE),NULL);  //128-255
    PUT_PTR(heap_listp+(6*WSIZE),NULL);  //256-511
    PUT_PTR(heap_listp+(7*WSIZE),NULL);  //512-1023
    PUT_PTR(heap_listp+(8*WSIZE),NULL);  //1024-2047
    PUT_PTR(heap_listp+(9*WSIZE),NULL);  //2048-4095
    PUT_PTR(heap_listp+(10*WSIZE),NULL); //4096---

    heap_table_p=heap_listp; //链表数组地址

    //序言块

    PUT(heap_listp+(11*WSIZE),PACK(DSIZE,1)); //头部
    PUT(heap_listp+(12*WSIZE),PACK(DSIZE,1)); //脚部
    PUT_PTR(heap_listp+(13*WSIZE),PACK(0,1)); //结束块
    
    heap_listp+=12*WSIZE;

    if(extend_heap(CHUNKSIZE)==NULL)
        return -1;
    //mm_check();
    return 0;
}

/*
 * find_fit - 遍历空闲链表，找到一个合适大小的空闲块并返回"指向其上一个空闲块的指针"，
    如果没有合适的空闲块则返回NULL
 */
char* find_fit(size_t size){
    char* base=get_listp(size);
    char* end=get_listp(10000);
    char* tra_listp;
    while(base<=end){
        tra_listp=(char*)(GET(base));
        while(tra_listp){
            size_t nowsize=GET_SIZE(HDRP(tra_listp));
            if(nowsize>=size){
                return tra_listp;
            }
            tra_listp=NEXT_LIST_BLKP(tra_listp);
        }
        base+=WSIZE;
    }
    return NULL;
}

/*
 *  delete_from_list - 将某个块从空闲链表中删除
 */
void delete_from_list(char* bp){
    char* preblk=PREV_LIST_BLKP(bp);
    char* nextblk=NEXT_LIST_BLKP(bp);
    if(preblk < heap_listp){  //pre是链表头指针，则需要修改头指针的指向
        PUT_PTR(preblk,nextblk);
    }else{
        PUT_PTR(preblk+WSIZE,nextblk);
    }
    if(nextblk != NULL)
        PUT_PTR(nextblk,preblk);
}

/*
 * place - 分割空闲块，将剩余的部分继续挂接到空闲链表上
 */
void place(char* bp,size_t size){
    char* newblk;
    size_t oldsize=GET_SIZE(HDRP(bp));

    if(oldsize-size<MIN_BLK){ //剩余部分无法作为空闲块
        size=oldsize;
        delete_from_list(bp);

        PUT(HDRP(bp),PACK(size,1));
        PUT(FTRP(bp),PACK(size,1));

    }else{
        delete_from_list(bp);

        PUT(HDRP(bp),PACK(size,1)); //修改头部和脚部的信息
        PUT(FTRP(bp),PACK(size,1));

        newblk=bp+size;
        PUT(HDRP(newblk),PACK(oldsize-size,0)); 
        PUT(FTRP(newblk),PACK(oldsize-size,0));

        insert_into_list(newblk,oldsize-size);  //剩余块挂到空闲链上
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    request_no++;
    if(size==0) return NULL;  //无意义请求
    char* bp;
    size_t extendsize;
    int newsize=MAX(ALIGN(size+DSIZE),MIN_BLK); //有效载荷必须大于等于16字节，否则无法装载2个指针
    //printf("call malloc newsize=%d size=%d request_no=%d\n",newsize,size,request_no);
    if((bp=find_fit(newsize)) != NULL){ //寻找合适的空闲块
        place(bp,newsize);  //找到了就分割并返回块指针
        //printf("return malloc request_no=%d ptr=%lx\n",request_no,(unsigned long)bp);
        return bp;
    }
    //没有找到合适的空闲块
    extendsize=MAX(newsize,CHUNKSIZE);
    if((bp=extend_heap(extendsize)) == (void*)-1){
        return NULL;
    }
    place(bp,newsize); //分割空闲块
    //printf("ask_new blk requestno=%d alloc bp=%lx size=%d\n",request_no,(unsigned long)bp,GET_SIZE(HDRP(bp)));
    //mm_check();
    //printf("return malloc request_no=%d ptr=%lx\n",request_no,(unsigned long)bp);
    return bp;
}

/*
 * insert_into_list - 将free的空闲块挂到空闲链表上
 */
void insert_into_list(char* bp,size_t size){  //重点审查

    char* linked_listp=get_listp(size);
    char* tra_listp=(char*)(GET(linked_listp));
    char* last_listp=linked_listp;
    while(tra_listp!=NULL){
        if(GET_SIZE(HDRP(tra_listp))>=size)
            break;
        last_listp=tra_listp;
        tra_listp=NEXT_LIST_BLKP(tra_listp);
    }
    PUT_PTR(bp,last_listp);
    PUT_PTR(bp+WSIZE,tra_listp);

    if(last_listp==linked_listp){  //链表为空
        PUT_PTR(last_listp,bp);
    }else{   //链表不为空
        PUT_PTR(last_listp+WSIZE,bp);
    }

    if(tra_listp != NULL){
        PUT_PTR(tra_listp,bp);
    }
}

/*
 * coalesce - 查看当前空闲块是否能与前后的块合并，如果能则合并并返回新块的指针
 * 如果不能则直接返回当前块的指针
 */
void* coalesce(char* bp){
    //printf("call coalesce bp=%lx\n",(unsigned long)bp);
    char* preblk=PREV_BLKP(bp);
    char* nextblk=NEXT_BLKP(bp);
    size_t size=GET_SIZE(HDRP(bp));
    int nextblk_size=0;
    int next_merge=0;
    int preblk_size=0;
    int pre_merge=0;

    if(preblk > heap_listp){
        preblk_size=GET_SIZE(HDRP(preblk));
        pre_merge=!GET_ALLOC(HDRP(preblk));
    }

    if(GET_SIZE(nextblk)!=0){
        nextblk_size=GET_SIZE(HDRP(nextblk));
        next_merge=!GET_ALLOC(HDRP(nextblk));
    }
    //printf("pre_merge=%d next_merge=%d\n",pre_merge,next_merge);
    if(!pre_merge && !next_merge) //无法合并
        return bp;
    if(pre_merge && !next_merge){ //与前面的块合并
        size+=preblk_size;

        delete_from_list(preblk);
        delete_from_list(bp);

        PUT(HDRP(preblk),PACK(size,0));
        PUT(FTRP(preblk),PACK(size,0));
        
        insert_into_list(preblk,size);

        return preblk;
    }
    else if(!pre_merge && next_merge){ //与后面的块合并
        size+=nextblk_size;
        delete_from_list(bp);

        delete_from_list(nextblk);


        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        insert_into_list(bp,size);

        return bp;
    }
    else{ //与前后的块一起合并
        size+=preblk_size+nextblk_size;

        delete_from_list(preblk);
        delete_from_list(bp);
        delete_from_list(nextblk);

        PUT(HDRP(preblk),PACK(size,0));
        PUT(FTRP(preblk),PACK(size,0));

        insert_into_list(preblk,size);

        return preblk;
    }
    printf("error:unreachable place!\n");
    return bp;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)  //遍历链表，寻找合适的位置插入
{
    free_no++;
    size_t size=GET_SIZE(HDRP(ptr));
    //printf("call free,ptr=%lx size=%d free_no=%d\n",(unsigned long)ptr,size,free_no);
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    insert_into_list(ptr,size);
    coalesce(ptr);
    //("free return free_no=%d\n",free_no);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
  
void *mm_realloc(void *ptr, size_t size)
{
    //printf("call realloc ptr=%lx size=%d\n",(unsigned long)ptr,size);
    if(ptr==NULL)   return mm_malloc(size);
    if(size==0){
        free(ptr);
        return NULL;
    }
    char *oldptr = ptr;
    char *newptr;
    char *nextblk=NEXT_BLKP(ptr);
    size_t copySize=0;
    size_t old_size=0;

    old_size = GET_SIZE(HDRP(oldptr));
    size=ALIGN(size+DSIZE);

    if(old_size>=size){  //所需块的大小<=已有块的大小，不用重新分配
        return ptr;
    }else if(GET_SIZE(HDRP(nextblk))==0){   
        //已有块是当前堆中最后一块，请求分配新的空间，并且这些新分配的空间与已有块是相连的
        if (extend_heap(MAX(CHUNKSIZE,size-old_size)) == NULL)
            return NULL;

        place(nextblk,size-old_size);
        PUT(HDRP(ptr),PACK(size,1));
        PUT(FTRP(ptr),PACK(size,1));
        return ptr;
    }else if(GET_ALLOC(HDRP(nextblk))==0 && old_size+GET_SIZE(HDRP(nextblk))>=size){
        //下一个块是空闲块，且已有块和下一个空闲块合并后大小满足要求
        place(nextblk,GET_SIZE(HDRP(nextblk)));
        PUT(HDRP(ptr),PACK(old_size+GET_SIZE(HDRP(nextblk)),1));
        PUT(FTRP(ptr),PACK(old_size+GET_SIZE(HDRP(nextblk)),1));
        return ptr;
    }
    else{ //以上都不满足的情况，重新malloc一块合适的空间
        newptr=mm_malloc(size);
        copySize = GET_SIZE(HDRP(newptr));
        if (old_size < copySize){
          copySize = old_size;
        }
        memcpy(newptr, oldptr, copySize-DSIZE);
        mm_free(oldptr);
        return newptr;
    }
    return NULL;
}

int mm_check(){
    char* base=heap_table_p;
    char* end=get_listp(10000);
    char* tra_listp;
    //int i=0;
    //int top_limit=7;
    //int down_limit=1;
    //int mark=0;
    char* heap_hi=mem_heap_hi();
    char* heap_lo=mem_heap_lo();
    /*
    while(base<=end){
        printf("----------------------\n");
        printf("list table %d addr=%lx  top=%d,down=%d\n",i,(unsigned long)(base),top_limit,down_limit);
        tra_listp=(char*)(GET(base));
        while(tra_listp){
            size_t nowsize=GET_SIZE(HDRP(tra_listp));
            printf("addr=%lx size=%d\n",(unsigned long)tra_listp,nowsize);
            if(nowsize>top_limit || nowsize<down_limit){
                mark=1;
            }
            tra_listp=NEXT_LIST_BLKP(tra_listp);
        }
        down_limit=top_limit+1;
        if(down_limit==4096)
            top_limit=0x7fffffff;
        else
            top_limit=2*down_limit-1;
        base+=WSIZE;
        i++;
    }
    printf("----------------------\n");
    if(mark){
        printf("error: block size don't match with list size!\n\n");
        exit(-1);
    }
    return 1;
    */
    while(base<=end){
        tra_listp=(char*)(GET(base));
        while(tra_listp){
            //size_t nowsize=GET_SIZE(HDRP(tra_listp));
            //printf("addr=%lx size=%d\n",(unsigned long)tra_listp,nowsize);
            /*
            if(nowsize>top_limit || nowsize<down_limit){
                mark=1;
            }*/
            if(tra_listp!=NULL && (tra_listp<heap_lo || tra_listp>heap_hi)){
                printf("addr=%lx out of bound\n",(unsigned long)tra_listp);
                exit(-1);
            }
            tra_listp=NEXT_LIST_BLKP(tra_listp);
        }
        base+=WSIZE;
    }
    return 1;
}













