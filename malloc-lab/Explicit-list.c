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
#define ALIGN_4(size) (((size) + 3) & ~0x3)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define MAX(a,b) ((a) > (b) ? (a) : (b) )
#define WSIZE  4
#define DSIZE  8
#define CHUNKSIZE ((1<<12))
#define MIN_BLK 24

#define PACK(size,alloc) ((size) | (alloc))
#define GET(p)  (*(unsigned int*)(p))
#define PUT(p,val)  (*(unsigned int*)(p) = (val))
#define PUT_PTR(p,ptr)  (*(char**)(p)=(ptr))
#define GET_SIZE(p) (GET(p) & ~0x3)
#define GET_ALLOC(p)    (GET(p) & 0x1)

#define HDRP(bp)    ((char*)(bp)-WSIZE)
//#define FTRP(bp)

#define NEXT_BLKP(bp) (char*) (* ( (char**)(bp+DSIZE) ) )
#define PREV_BLKP(bp) (char*) (* ( (char**)bp ) )

char* find_fit(size_t size);
void* extend_heap(size_t size);
void place(char* bp,size_t size);
void insert_into_list(char* bp);
void* coalesce(char* bp);
int mm_check();

static char* heap_listp=NULL;     //指向空闲链表头(伪头节点)
static char* heap_list_tail=NULL; //指向空闲链表尾
//第一次尝试: 显式空闲链表+地址排序+首次适应算法+立即合并

//单个空闲块的结构如下
// ----------------
// |块大小 a/f     |
// ----------------
// |指向后继节点指针|
// |指向前驱节点指针|
// | (有效载荷)    |
// ----------------
// |填充(可选)     |
// ----------------
//当该块被分配后，指向后继节点和后继节点指针的内存也将作为有效载荷


/*
 * extend_heap - 扩展堆的大小，请求一个新的内存块,成功返回新的空闲块指针，失败返回NULL
 */
void* extend_heap(size_t size){
    char* newblk;
    if((newblk=mem_sbrk(size)) == (void*)-1){
        printf("error:mem_sbrk(%d) fails!\n",size);
        return NULL;
    }
    //printf("extend_heap: heap_tail=%lx newblk=%lx\n",(unsigned long)heap_list_tail,(unsigned long)newblk);
    PUT(newblk,PACK(size,0)); //分配一页大小的空闲块
    PUT_PTR(newblk+WSIZE,heap_list_tail);        //前驱
    PUT_PTR(newblk+WSIZE+DSIZE,NULL);              //后继

    newblk+=WSIZE;
    PUT_PTR(heap_list_tail+DSIZE,newblk);    //修改原链表最后一个节点的后继指针

    //printf("extend heap:heap_list_tail=%lx size=%d\n",(unsigned long)NEXT_BLKP(heap_list_tail),size);
    
    heap_list_tail=newblk;

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
    if((heap_listp=mem_sbrk(MIN_BLK-WSIZE)) == (void*)-1){
        printf("error:mem_sbrk(%d) fails!\n",CHUNKSIZE);
        return 0;
    }
    //创建空闲链表的伪头节点
    PUT(heap_listp,PACK(MIN_BLK-WSIZE,1)); //填充一个无意义的头部,标记为1防止它被合并
    PUT_PTR(heap_listp+WSIZE,NULL); //指向上一个节点
    PUT_PTR(heap_listp+WSIZE+DSIZE,NULL); //指向下一个节点
    heap_listp+=WSIZE;
    //printf("heap_listp=%lx\n size=%d\n",(unsigned long)heap_listp,GET_SIZE(HDRP(heap_listp)));
    heap_list_tail=heap_listp;

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
    char* tra_listp=NEXT_BLKP(heap_listp);
    while(tra_listp){
        size_t nowsize=GET_SIZE(HDRP(tra_listp));
        if(nowsize>=size){
            return tra_listp;
        }
        tra_listp=NEXT_BLKP(tra_listp);
    }
    return NULL;
}

/*
 * place - 分割空闲块，将剩余的部分继续挂接到空闲链表上
 */
void place(char* bp,size_t size){
    char* preblk=PREV_BLKP(bp);
    char* nextblk=NEXT_BLKP(bp);
    char* newblk;
    size_t oldsize=GET_SIZE(HDRP(bp));

    if(oldsize-size<MIN_BLK){ //剩余部分无法作为空闲块
        size=oldsize;
        PUT(HDRP(bp),PACK(size,1));
        PUT_PTR(preblk+DSIZE,nextblk);
        if(nextblk != NULL)
            PUT_PTR(nextblk,preblk);

        if(heap_list_tail==bp)   //修改堆尾指针
            heap_list_tail=preblk;
        return;
    }else{
        PUT(HDRP(bp),PACK(size,1)); //修改头部的信息

        newblk=bp+size;
        PUT(HDRP(newblk),PACK(oldsize-size,0));  //将剩余的空闲块挂接到空闲链表中

        //printf("size=%d leave-size=%d\n",size,oldsize-size);
        PUT_PTR(newblk,preblk);
        PUT_PTR(newblk+DSIZE,nextblk);

        PUT_PTR(preblk+DSIZE,newblk);  //修改链表上前后节点的指针
        //if(targ){
        //    char* tra=heap_listp;
        //}
        if(nextblk != NULL)
            PUT_PTR(nextblk,newblk);

        if(heap_list_tail==bp){   //修改堆尾指针
            heap_list_tail=newblk;
        }
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if(size==0) return NULL;  //无意义请求
    char* bp;
    size_t extendsize;
    int newsize=MAX(ALIGN(size+DSIZE),MIN_BLK); //有效载荷必须大于等于16字节，否则无法装载2个指针
    //printf("call malloc newsize=%d size=%d\n",newsize,size);
    if((bp=find_fit(newsize)) != NULL){ //寻找合适的空闲块
        place(bp,newsize);  //找到了就分割并返回块指针
        return bp;
    }
    //没有找到合适的空闲块
    extendsize=MAX(CHUNKSIZE,newsize);
    if((bp=extend_heap(extendsize)) == (void*)-1){
        return NULL;
    }
    place(bp,newsize); //分割空闲块
    //mm_check();
    return bp;
}

/*
 * insert_into_list - 将free的空闲块挂到空闲链表上
 */
void insert_into_list(char* bp){
    char* tra_listp=NEXT_BLKP(heap_listp);
    char* last_listp=heap_listp;
    while(tra_listp!=NULL){   //寻找合适的位置（地址排序链表）
        if(tra_listp>bp)
            break;
        last_listp=tra_listp;
        tra_listp=NEXT_BLKP(tra_listp);
    }
    PUT_PTR(last_listp+DSIZE,bp);  //插入链表
    if(tra_listp!=NULL)
        PUT_PTR(tra_listp,bp);
    else{
        heap_list_tail=bp;
    }

    PUT_PTR(bp,last_listp);
    PUT_PTR(bp+DSIZE,tra_listp);
}

/*
 * coalesce - 查看当前空闲块是否能与前后的块合并，如果能则合并并返回新块的指针
 * 如果不能则直接返回当前块的指针
 */
void* coalesce(char* bp){
    //printf("call coalesce bp=%lx\n",(unsigned long)bp);
    //维护tail指针
    char* preblk=PREV_BLKP(bp);
    char* nextblk=NEXT_BLKP(bp);
    size_t size=GET_SIZE(HDRP(bp));
    char* next_next;
    int nextblk_size=0;
    int next_merge=0;

    int preblk_size=GET_SIZE(HDRP(preblk));
    int pre_merge=((preblk+preblk_size) == bp && preblk!=heap_listp);

    if(nextblk!=NULL){
        nextblk_size=GET_SIZE(HDRP(nextblk));
        next_merge=((bp+size) == nextblk);
    }

    //printf("pre_merge=%d next_merge=%d\n",pre_merge,next_merge);
    if(!pre_merge && !next_merge) //无法合并
        return bp;
    if(pre_merge && !next_merge){ //与前面的块合并
        size+=preblk_size;

        //printf("preblk=%lx aft-size=%d\n",(unsigned long)preblk,size);

        PUT(HDRP(preblk),PACK(size,0));
        PUT_PTR(preblk+DSIZE,nextblk);

        if(nextblk!=NULL)
            PUT_PTR(nextblk,preblk);
        if(heap_list_tail==bp){
            heap_list_tail=preblk;
        }
        return preblk;
    }
    else if(!pre_merge && next_merge){ //与后面的块合并
        next_next=NEXT_BLKP(nextblk);
        size+=nextblk_size;
        PUT(HDRP(bp),PACK(size,0));
        PUT_PTR(bp+DSIZE,next_next);

        if(next_next!=NULL)
            PUT_PTR(next_next,bp);

        if(heap_list_tail==nextblk){
            heap_list_tail=bp;
        }
        return bp;
    }
    else{ //与前后的块一起合并
        next_next=NEXT_BLKP(nextblk);
        size+=preblk_size+nextblk_size;
        PUT(HDRP(preblk),PACK(size,0));
        PUT_PTR(preblk+DSIZE,next_next);

        if(next_next!=NULL)
            PUT_PTR(next_next,preblk);
        if(heap_list_tail==nextblk){
            heap_list_tail=preblk;
        }
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
    size_t size=GET_SIZE(HDRP(ptr));
    //printf("call free,ptr=%lx size=%d\n",(unsigned long)ptr,size);
    PUT(HDRP(ptr),PACK(size,0));
    insert_into_list(ptr);
    coalesce(ptr);
    //mm_check();
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
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    //printf("newptr=%lx,size=%d",(unsigned long)newptr,GET_SIZE(HDRP(newptr)));
    if (newptr == NULL)
      return NULL;
    size = GET_SIZE(HDRP(oldptr));
    copySize = GET_SIZE(HDRP(newptr));
    if (size < copySize){
      copySize = size;
    }
    memcpy(newptr, oldptr, copySize-WSIZE);
    mm_free(oldptr);
    //printf("\n\n");
    return newptr;
}

/*
 * mm_check - 检测堆一致性，主要判断以下几点内容：
 * 1.是否空闲链表中的块都被标记为空闲状态
 * 2.是否有相邻两个空闲块没有合并
 * 3.是否每个空闲块都被挂接到空闲链表上
 * 4.块中的指针是否指向了堆外的内存地址
 */
int mm_check(){
    char* tra_listp=NEXT_BLKP(heap_listp);
    char* last_listp=heap_listp;
    char* heap_addrp=heap_listp;
    char* heap_hi=mem_heap_hi();
    char* heap_lo=mem_heap_lo();

    char* tmp_next;
    char* tmp_last;

    while(tra_listp){
        //printf("check:tra_listp=%lx size=%d\n",(unsigned long)tra_listp,GET_SIZE(HDRP(tra_listp)));
        tmp_last=PREV_BLKP(tra_listp);
        tmp_next=NEXT_BLKP(tra_listp);
        if(GET_ALLOC(HDRP(tra_listp)) == 1){
            //空闲链上存在标记位busy的节点
            printf("error:the block in the free list marked as busy!\n addr=%lx\n",(unsigned long)tra_listp);
            exit(-1);
        }
        if(last_listp != heap_listp && last_listp+GET_SIZE(HDRP(last_listp)) == tra_listp ){
            //存在相邻的两个空闲块没有被合并
            printf("error:contiguous free blocks escaped coalescing!\npre=%lx,next=%lx\n",(unsigned long)last_listp,(unsigned long)tra_listp);
            exit(-1);
        }
        if(tmp_last!=NULL && (tmp_last > heap_hi || tmp_last < heap_lo)){
            //前驱指针指向了堆外的地址
            printf("error:block pointer(prev_point) point to valid heap address!\nheap_hi=%lx heap_lo=%lx pointer=%lx\n",(unsigned long)heap_hi,\
            (unsigned long)heap_lo,(unsigned long)PREV_BLKP(tra_listp));
            exit(-1);
        }
        
        if(tmp_next!=NULL && (tmp_next > heap_hi || tmp_next < heap_lo)){
            //后继指针指向了堆外的地址
            printf("error:block pointer(next_point) point to valid heap address!\nheap_hi=%lx heap_lo=%lx pointer=%lx\n",(unsigned long)heap_hi,\
            (unsigned long)heap_lo,(unsigned long)NEXT_BLKP(tra_listp));
            exit(-1);
        }

        last_listp=tra_listp;
        tra_listp=NEXT_BLKP(tra_listp);
    }

    tra_listp=NEXT_BLKP(heap_listp);
    
    //地毯式搜索
    while(heap_addrp<heap_hi){
        if(heap_addrp+GET_SIZE(HDRP(heap_addrp))-WSIZE > heap_hi+1){
            //这个块内部存在非法空间
            printf("error:the last block out of bounds!\naddrp=%lx heap_hi=%lx\n",(unsigned long)heap_addrp,(unsigned long)heap_hi);
            exit(-1);
        }
        if(GET_ALLOC(HDRP(heap_addrp)) == 0){  //如果这个块是空闲块
            if(heap_addrp==tra_listp){  //与链表上的空闲块一一对应（由于链表是按地址顺序组织的）
                tra_listp=NEXT_BLKP(tra_listp);
            }else{
                //存在空闲块没有挂接在空闲链表上
                printf("error:there is a free block which is not in free list!\nfree_list=%lx,pointer=%lx\n",(unsigned long)tra_listp,(unsigned long)heap_addrp);
                exit(-1);
            }
        }

        heap_addrp+=GET_SIZE(HDRP(heap_addrp));//到相邻的下一个块
    }
    return 1;
}















