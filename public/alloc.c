#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_LEN 4

extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

unsigned int max_size = 0;

void make_memory_block(void *b, int len){   // 블록 header, tag 생성
    int32_t header = len;

}

void allocated_bit_set(void *p){
    int8_t ptr = (int8_t*)p;
    *(ptr + *(int32_t*)p - 1) |=1;  // allocated bit set tag
    *(int32_t*)p |= 1;        // allocated bit set header
    //*( p + HEADER_LEN + newsize) |= 1;    // allocate bit set tag
    //return (void*)( p + 1 )             // return memory block data pointer   
}

void *myalloc(size_t size)
{
    void * heap_start = sbrk(0) - max_size;     // find first heap block
    void * end = heap_start;
    int32_t * p;
    p = heap_start;
    int8_t * ptr;
    
    int newsize = ((size + 15 + HEADER_LEN*2) >> 4 ) << 4;   // 16의 배수 라운딩 , header & tag 자리 생성
    
    
    // search free block
    while((p < end) && ((*p & 1) || (*p <= newsize))){
        p = p + (*p & -2);
    }
    if(p == end){       // free된 영역들 중에 맞는 블록이 없는경우 
        p = (int32_t*)sbrk(newsize);                // 새로운 메모리 블록 생성
        *p = newsize;
        *(int32_t*)((void*)p + newsize) = newsize;
        allocated_bit_set((void*)p);
        allocated_bit_set((void*)((void*)p + newsize));
        end = (void*)p + newsize;
        max_size += newsize;
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
    debug("free(%p)\n", ptr);
}
