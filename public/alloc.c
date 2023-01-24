#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_LEN 4

extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

unsigned int max_size = 0;

void allocated_bit_set(void *p){
    int8_t * ptr = (int8_t*)p;
    printf("allocated bit : %p\n",p);
    
    *(int32_t*)(ptr + *(int32_t*)p - 4) |=1;  // allocated bit set tag

    printf("allocated tag flag : %p %d\n",(int32_t*)(ptr + *(int32_t*)p - 4),*(int32_t*)(ptr + *(int32_t*)p - 4));
    *(int32_t*)p |= 1;        // allocated bit set header
    printf("allocated flag : %d\n",*(int32_t*)p);
    //*( p + HEADER_LEN + newsize) |= 1;    // allocate bit set tag
    //return (void*)( p + 1 )             // return memory block data pointer   
}

void *myalloc(size_t size)
{
    void * heap_start = 0;
    void * end = sbrk(0);
    heap_start = end - max_size;     // find first heap block

    int32_t * p;
    p = heap_start;
    int8_t * ptr;
    
    int newsize = ((size + 15 + HEADER_LEN*2) >> 4 ) << 4;   // 16의 배수 라운딩 , header & tag 자리 생성
    
    printf("myalloc : %d, newsize : %d, %p\n",size,newsize,p);
    // search free block
    while((p < end) && ((*p & 1) || (*p <= newsize))){
        printf("find free block p : %p, %d\n(*p & -2): %x : result : %p\n", p, *(int32_t*)p,(*p & -2),(void*)p + (*p & -2));
        p = (void*)p + (*p & -2);
        printf("find free block %p\n",p);
    }
    printf("find block p : %p end : %p\n",p,end);
    if(p == end){       // free된 영역들 중에 맞는 블록이 없는경우 
        printf("create new block\n");
        p = (int32_t*)sbrk(newsize);                // 새로운 메모리 블록 생성

        *p = newsize;                               // new header 
        *(int32_t*)((void*)p + newsize - 4) = newsize;  // new tag
        *(int32_t*)((void*)p + newsize) = 1;            // end block check

        printf("create new block size %d, newsize %d\n",size,newsize);
        printf("*p : %p %d sbrk(0): %p\n",p,*p,(int32_t*)sbrk(0));
        allocated_bit_set((void*)p);

        printf("create new block\n");

        //end = (int32_t*)sbrk(0) - newsize;
        max_size += newsize;
        // debug print

        debug("alloc(%u): %p\n", (unsigned int)newsize, p);
        debug("max: %u\n", max_size);
        printf("return point : %p\n", (void*)(p + 1));
        return (void*)(p + 1);
    }
    if(*p == newsize){     // 딱 맞는 사이즈인 경우 
        allocated_bit_set(p);
        /**
        ptr = (int8_t*)p;
        *(ptr + *p - 1) |=1;  // allocated bit set tag
        *p |= 1;        // allocated bit set header
        //*( p + HEADER_LEN + newsize) |= 1;    // allocate bit set tag
        return (void*)( p + 1 )             // return memory block data pointer
        */

        // debug print
        debug("alloc(%u): %p\n", (unsigned int)newsize, p);
        debug("max: %u\n", max_size);
        return (void*)(p + 1);

    }else{              // 사이즈가 작은경우
        int remain_memory = *p - newsize;
        ptr = (int8_t *)p;
        int oldsize = *p;
        *p = newsize;       //set new header
        memcpy(ptr + *p - 4 , p, sizeof(int32_t));  //set new tag

        *(ptr + *p) = remain_memory;    //set new header remain memory
        memcpy(ptr + oldsize - 4, ptr + *p, sizeof(int32_t));    //set new tag remain memory
        allocated_bit_set((void*)p);
        allocated_bit_set((void*)(ptr + *p));

        // debugprint 
        debug("alloc(%u): %p\n", (unsigned int)newsize, p);
        debug("max: %u\n", max_size);
        return (void*)(p + 1);
    }
    //allocate new block
    //void *p = sbrk(size);

    // make header & tag


    // debuging print
    debug("alloc(%u): %p\n", (unsigned int)size, p);
    max_size += size;
    debug("max: %u\n", max_size);
    return p;
}

void *myrealloc(void *ptr, size_t size)
{
    void *p = NULL;
    if (size != 0)
    {
        p = sbrk(size);
        if (ptr)
            memcpy(p, ptr, size);
        max_size += size;
        debug("max: %u\n", max_size);
    }
    debug("realloc(%p, %u): %p\n", ptr, (unsigned int)size, p);
    return p;
}

void myfree(void *ptr)
{
    int32_t * p;
    int32_t * next;
    int32_t * pre;
    p = (int32_t*)(ptr - HEADER_LEN);       // p = header pointer
    *p &= -2;                               // allocate flag clear
    next = (int32_t*)((void *)p + *p);      // next  block header pointer
    pre = (int32_t*)((void*)p - 4);
    pre = (int32_t*)((void*)p - (*pre & -2));    // pre block header ponter

    printf("free block pre, p, next : %p, %p, %p\n",pre, p, next);
    printf("free block data : %8d, %8d, %8d\n", *pre, *p, *next);

    
    if((*next &1) == 0){                    // 다음블록이 free된 블록인경우
        *p += *next;
        *(int32_t*)((void*)p + *p - 4) = *p;// p block tag 업데이트
    }
    if((*pre &1) == 0){                     // 이전 블록이 free 된 블록인경우
        *pre += *p;
        *(int32_t*)((void*)pre + *pre - 4) = *pre;  // pre block tag 업데이트 
    }
    // 마지막 블록을 free 하는경우 max_size를 줄어주어야한다. 
    if(*next == 1 && (*pre & 1)){                 // 마지막 블록이 and previous block is allocated
        // next 블록이 free된경우 같이 반환,

        // next 블록이 free가 아니라면 나만 반환
    }else{					// free last block and previous block is unallocated
    
    }
    debug("free(%p)\n", ptr);
}
