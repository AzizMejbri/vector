#include "vector.h"


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../defer/defer.h"

//TODO: bench mark with and without __builtin_expect

#ifdef __GNUC__
    #define LIKELY(x) __builtin_expect((x), 1)
    #define UNLIKELY(x) __builtin_expect((x), 0)
#else
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
#endif

#define handle_err(condition, msg, code)                \
    do{                                                 \
       if ( LIKELY(condition )){                        \
            fprintf(stderr, "\e[31m%s\e[m\n", msg);     \
            code;                                       \
        }                                               \
    } while (0);


struct vector{
    u64     capacity    ;
    u64     size        ;
    u8*     data        ;
    Arena   arena       ;
    u8      block_size  ;
}; 

Vector vec_init_(u64 def, u8 block_size){
    Vector v = (Vector)malloc(sizeof(struct vector));
    handle_err(
        v == NULL,
        "Error Allocating Space for the Vector! Aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    defer(v, free);
    v -> capacity = def == 0 ? 10 : def;
    v -> block_size = block_size;
    v -> data = (u8*)malloc(v -> capacity * block_size);
    v -> arena = NULL;
    handle_err(
        v -> data == NULL,
        "Error Allocating Space for the Vector! Aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    defer(v -> data, free);
    return v;
}

Vector vec_arena_(u64 def, u8 block_size, Arena arena){
    Vector v = (Vector)alloc_on_arena(arena, sizeof(struct vector));
    handle_err(
        v == NULL,
        "Error Allocating Space for the Vector! Aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    v -> capacity = def == 0 ? 10 : def;
    v -> block_size = block_size;
    v -> data = (u8*)alloc_on_arena(arena, v -> capacity * block_size);
    v -> arena = arena;
    handle_err(
        v -> data == NULL,
        "Error Allocating Space for the Vector! Aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    return v;

}
void vec_push(Vector v, void* x){                                                                   //TODO: Realloc for Arena
    handle_err(
            v == NULL || v -> data == NULL,
            "Null Vector passed as a parameter to push to ...",
            cleanup();
            exit(EXIT_FAILURE);
    )
    if ( v -> capacity == v -> size ){
        const u64 old_capa = v -> capacity;
        v -> capacity *= 2;
        v -> data = v -> arena == NULL ? (u8*)ds_realloc(v -> data, v -> capacity * v -> block_size ) : realloc_on_arena(v -> arena, v -> data, old_capa * v -> block_size, v -> capacity * v -> block_size);
        handle_err(
            v -> data == NULL,
            "Error resizing the Vector! Watch out the element was NOT pushed ...",
            cleanup();
            exit(EXIT_FAILURE);
        )
    }
    memcpy(v -> data + v -> size * v -> block_size, x, v -> block_size);
    v -> size ++;
}

void vec_pop_(Vector v, void* gottem){
    handle_err(
        v == NULL || v -> data == NULL,
        "Null Vector passed as a paramater to be popped from ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        v -> size == 0,
        "Illegal Popping operation on an empty vector ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    memcpy(gottem, v -> data + (--v -> size) * v -> block_size, v -> block_size);
}

void vec_dbg(const Vector v){
    handle_err(
        v == NULL,
        "NULL",
        return;
    )
    handle_err(
        v -> data == NULL,
        "(nullvec)",
        return;
    )
    if ( v -> size == 0 ) {
        fprintf(stdout, "[]");
        return;
    }
    fprintf(stdout, "[");
    for ( unsigned i = 0 ; i < v -> size - 1; i ++){
        for(unsigned j = 0; j < v -> block_size; j++){
            fprintf(stdout, "%X", v -> data[i * v -> block_size + j]);
        }
        fprintf(stdout, ", ");
    }
    for(unsigned j = 0; j < v -> block_size; j++){
        fprintf(stdout, "%X", v -> data[(v -> size - 1) * v -> block_size + j ]);
    }
    fprintf(stdout, "]\n");
}

void vec_print(const Vector v, void(*printer)(void*)){
    handle_err(
        v == NULL,
        "NULL",
        return ;
    )
    handle_err(
        v -> data == NULL,
        "(nullvec)",
        return ;
    )
    if ( v -> size == 0 ) {
        fprintf(stdout, "[]");
        return ;
    }
    fprintf(stdout, "[");
    for(unsigned i = 0; i < v -> size - 1; i++){
        printer(v -> data + i * v -> block_size);
        fprintf(stdout, ", "); 
    }
    printer(v -> data + v -> block_size * ( v -> size - 1 ));
    fprintf(stdout, "]");
}


void vec_get_(const Vector v, u64 index, void* gottem){
    handle_err(
            v == NULL || v -> data == NULL,
            "Null Pointer Error! Illegal access ...",
            cleanup() ;
            exit(EXIT_FAILURE);
    )
    handle_err(
        index >= v -> size,
        "Out Of Bounds Error ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    memcpy(gottem, v -> data + v -> block_size * index, v -> block_size);
}

static inline void inline_vec_push(Vector v, void* x){
    if ( v -> capacity == v -> size ){
        const u64 old_capa = v -> capacity;
        v -> capacity *= 2;
        v -> data = v -> arena == NULL ? (u8*)ds_realloc(v -> data, v -> capacity * v -> block_size ) : (u8*)realloc_on_arena(v -> arena, v -> data, old_capa * v -> block_size, v -> capacity * v -> block_size);
        handle_err(
            v -> data == NULL,
            "Error resizing the Vector! Watch out the element was NOT pushed ...",
            cleanup();
            exit(EXIT_FAILURE);
        )
    }
    memcpy(v -> data + v -> size * v -> block_size, x, v -> block_size);
    v -> size ++;
}


void vec_insert(Vector v, void* x, u64 index){
    handle_err(
        v == NULL || v -> data == NULL,
        "Error inserting into a null vector! aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        index > v -> size,
        "Error inserting into the vector! index out of bounds, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )

    if(index == v -> size){
        inline_vec_push(v, x);
        return;
    }
    if ( v -> capacity == v -> size ){
        const u64 old_capa = v -> capacity;
        v -> capacity *= 2;
        v -> data = v -> arena == NULL ? (u8*)ds_realloc(v -> data, v -> capacity * v -> block_size) : (u8*)realloc_on_arena(v -> arena, v -> data, old_capa * v -> block_size, v -> capacity * v -> block_size);
        handle_err(
            v -> data == NULL,
            "Error resizing the Vector! Watch out the element was NOT pushed ...",
            cleanup();
            exit(EXIT_FAILURE);
        )
    }
    memmove(v -> data + v -> block_size * (index+1),
        v -> data + v -> block_size * index,
        (v -> size - index) * v -> block_size
    ); 
    memcpy(v -> data + v -> block_size * index, x, v -> block_size);
    v -> size ++; 
}

void vec_set(Vector v, void* x, u64 index){
    handle_err(
        v == NULL || v -> data == NULL,
        "Error inserting into a null vector! aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        index >= v -> size,
        "Error setting a case of the vector out of bounds! aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    memcpy(v -> data + v -> block_size * index, x, v -> block_size);
}

void vec_remove_(Vector v, u64 index, void* gottem){
    handle_err(
        v == NULL || v -> data == NULL,
        "Error removing into a null vector! aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        index >= v -> size,
        "Error removing into the vector! index out of bounds, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    memcpy(
        gottem,
        v -> data + v -> block_size * index,
        v -> block_size
    );
    memmove(
       v -> data + index * v -> block_size,
       v -> data + (index + 1) * v -> block_size,
       v -> block_size * (v -> size - index - 1)
    );
    v -> size --;
   
}

bool vec_empty(const Vector v){ return v -> size == 0; }

u64  vec_len(const Vector v) { return v -> size; }

void vec_reverse(Vector src, Vector dest){
    handle_err(
        src == NULL || dest == NULL || src -> data == NULL || dest -> data == NULL,
        "Error in reverse method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        src -> block_size != dest -> block_size,
        "unresolvable difference in datatypes of source and destination of reverse ! aboring ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
   
    const u64 orig_size = src -> size;

    if ( UNLIKELY(src -> size == 0))
        return;

    if ( dest -> capacity < src -> size )
        dest -> capacity = src -> size; 

    u8 tmp_arr[orig_size * src -> block_size];
    u8 x[src -> block_size];

    for(u64 i = 0; i < orig_size; i++){
        vec_get_(src, i, x);
        memcpy(tmp_arr + (orig_size - 1 - i) * src -> block_size, x, src -> block_size); 
    }
    memmove(dest -> data, tmp_arr, orig_size * src -> block_size);
    dest -> size = orig_size;
}

void vec_join(Vector appendee, const Vector appended){
     handle_err(
        appendee == NULL || appended == NULL || appendee -> data == NULL || appended -> data == NULL,
        "Error in joining method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        appendee -> block_size != appended -> block_size,
        "unresolvable difference in datatypes of source and destination of join ! aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    u64 capacity_cap = appendee -> size + appended -> size;
    if ( appendee -> capacity < capacity_cap ){
        const u64 old_capa = appendee -> capacity;
        appendee -> data = appendee -> arena == NULL ?
            ds_realloc(appendee -> data, appendee -> block_size * (appendee -> size + appended -> size)):
            realloc_on_arena(appendee -> arena, appendee -> data, old_capa * appendee -> block_size, appendee -> block_size * (appendee -> size + appended -> size));
        appendee -> capacity = capacity_cap;    
    }
    memcpy(appendee -> data + appendee -> size * appendee -> block_size,
        appended -> data,
        appended -> size * appendee -> block_size
    );
    appendee -> size = capacity_cap;
}

bool vec_in(const Vector v, void* x){
    handle_err(
        v == NULL || v -> data == NULL,
        "Error searching in a null vector! aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    for(u64 i = 0; i < v -> size; i++)
        if ( memcmp(v -> data + i * v -> block_size, x, v -> block_size) == 0)
            return true;
    
    return false;

}

bool vec_eq(const Vector va, const Vector vb){
    handle_err(
        va == NULL || vb == NULL || va -> data == NULL || vb -> data == NULL,
        "Error in comparing method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        va -> block_size != vb -> block_size,
        "unresolvable difference in datatypes of source and destination of compare ! aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    if ( va -> size != vb -> size ) return false;
    return memcmp(va -> data, vb -> data, va -> size * va -> block_size) == 0;
}

void vec_copy(const Vector src, Vector dest){
     handle_err(
        src == NULL || dest == NULL || src -> data == NULL || dest -> data == NULL,
        "Error in copy method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        src -> block_size != dest -> block_size,
        "unresolvable difference in datatypes of source and destination of copy ! aboring ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    if ( dest -> capacity < src -> size ){
        const u64 old_capa = dest -> capacity;
        dest -> data = dest -> arena == NULL ?
            ds_realloc(dest -> data, src -> size * src -> block_size):
            realloc_on_arena(dest -> arena, dest -> data, old_capa * dest -> block_size, src -> size * src -> block_size );
        dest -> capacity = src -> size;
    }
    memcpy(dest -> data, src -> data, src -> block_size * src -> size );
    dest -> size = src -> size;
}

void vec_clear(const Vector v){ 
    handle_err(
        v == NULL || v -> data == NULL,
        "Error in clear method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    v -> size = 0;
}

u64 vec_count(const Vector v, void* x){
    handle_err(
        v == NULL || v -> data == NULL,
        "Error in count method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    u64 occurences = 0;
    for(u64 i = 0; i < v -> size; i++)
        if ( memcmp( v -> data + i * v -> block_size, x, v -> block_size ) == 0 )
            occurences ++;
    return occurences;
}

static inline u64 binary_search(Vector v, void* x, bool* found, int (*cmp)(void*, void*)){
    u64 left = 0;
    u64 right = v -> size - 1;
    u64 mid;
    int compare_output;
    while(left < right){
       mid = (left + right) / 2;
       compare_output = cmp(v -> data + mid * v -> block_size, x);
       if ( compare_output == 0 ){
           *found = true;
           return mid;
        }
       if ( compare_output < 0  )
           left = mid + 1;
       else
           right = mid - 1;
    }
    *found = false;
    return mid;
}

static inline u64 normal_search(Vector v, void* x, bool* found, int (*cmp)(void*, void*)){
    u64 i;
    for(i = 0; i < v -> size; i++){
        if ( cmp(v -> data + i * v -> block_size, x) == 0 ){
            *found = true;
            return i;
        }
    }
    *found = false;
    return i;
}

u64 vec_search(const Vector v, void* x, bool* sorted_found, int (*cmp)(void*, void*)){
    if ( *sorted_found == SORTED )
        return binary_search(v, x, sorted_found, cmp);
    return normal_search(v, x, sorted_found, cmp);
}

void vec_map_(const Vector input, Vector output, void* (*mapper)(void*), u8 input_size, u8 output_size){ 
    handle_err(
        input == NULL || output == NULL || input -> data == NULL || output -> data == NULL,
        "Error in map method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        input -> block_size != input_size || output -> block_size != output_size,
        "unresolvable difference in datatypes of input, output and function of map ! aboring ...",
        cleanup();
        exit(EXIT_FAILURE);
    )

    output -> size = 0;
    if ( output -> capacity < input -> size){
        const u64 old_capa = output -> capacity;
        output -> capacity = input -> size;
        output -> data = output -> arena == NULL? 
            ds_realloc(output -> data, output -> capacity * output -> block_size):
            realloc_on_arena(output -> arena, output -> data, old_capa * output -> block_size, output -> capacity * output -> block_size);
    }
    for(u64 i = 0; i < input -> size; i++)
        memcpy(
            output -> data + i * output -> block_size,
            mapper(input -> data + i * input -> block_size),
            output -> block_size
        );
}

static inline void swap(u8* a, u8* b, const u8 block_size){
    u8 tmp[block_size];
    memcpy(tmp, a, block_size);
    memcpy(a, b, block_size);
    memcpy(b, tmp, block_size);
}

static inline i64 partition(u8* arr, const i64 low, const i64 high, const u8 block_size, int (*cmp)(void*, void*)){
    u8 pivot[block_size];
    memcpy(pivot, arr + high * block_size, block_size);

    i64 i = low - 1;
    for (i64 j = low; j < high; j++) {
        if (cmp(arr + j * block_size, pivot) < 0) {
            i++;
            swap(arr + i * block_size, arr + j * block_size, block_size);
        }
    }
    swap(arr + (i+1) * block_size, arr + high * block_size, block_size);
    return i+1;
}

static void quick_sort(u8* arr, const i64 low, const i64 high, const u8 block_size, int (*cmp)(void*, void*)){
    if ( low < high ){
        const int64_t pi = partition(arr, low, high, block_size, cmp);

        quick_sort(arr, low, pi - 1, block_size, cmp);
        quick_sort(arr, pi + 1, high, block_size, cmp);
    }
}

// Public sorting function
void vec_sort(const Vector input, Vector output, int (*cmp)(void*, void*)) {
    handle_err(
        input == NULL || output == NULL || input -> data == NULL || output -> data == NULL,
        "Error in map method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        input -> block_size != output -> block_size,
        "unresolvable difference in datatypes of input, output of sort ! aboring ...",
        cleanup();
        exit(EXIT_FAILURE);
    )

    output -> size = input -> size;
    if ( output -> capacity < input -> size ){
        const u64 old_capa = output -> capacity;
        output -> capacity = input -> size;
        output -> data = output -> arena == NULL ?
            ds_realloc(output -> data, output -> capacity * output -> block_size):
            realloc_on_arena( output -> arena, output -> data, old_capa * output -> block_size, output -> capacity * output -> block_size ); 
    }
    if ( input -> size <= 1 )
    {
        memcpy(input -> data, output -> data, input -> block_size * input -> size);
        return;
    }
    memcpy(output -> data, input -> data, input -> block_size * input -> size);
    quick_sort(output -> data, 0, input -> size - 1, input -> block_size, cmp);
}

void vec_slice(const Vector input, Vector output, const u64 low, const u64 high){
    handle_err(
        input == NULL || output == NULL || input -> data == NULL || output -> data == NULL,
        "Error in slice method ! a null vector was passed, aborting now ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    handle_err(
        input -> block_size != output -> block_size,
        "unresolvable difference in datatypes of input, output of slice ! aboring ...",
        cleanup();
        exit(EXIT_FAILURE);
    )

    u64 len = high - low;
    if ( output -> capacity < len){
        const u64 old_capa = output -> capacity;
        output -> capacity = len;
        output -> data = output -> arena == NULL ? 
            ds_realloc(output -> data, len * output -> block_size):
            realloc_on_arena(output -> arena, output -> data, old_capa * output -> block_size, len * output -> block_size);
    }
    output -> size = len;
    memcpy(output -> data, input -> data + low * input -> block_size, input -> block_size * len);
}

void vec_fit(Vector v){
    const u64 old_capa = v -> capacity;
    v -> capacity = v -> size;
    v -> data = v -> arena == NULL ? 
        ds_realloc(v -> data, v -> block_size * v -> capacity):
        realloc_on_arena(v -> arena, v -> data, old_capa * v -> block_size, v -> block_size * v -> capacity); 
}

void vec_setcapacity(Vector v, u64 capa){
    if ( capa < v -> size)
        return;
    const u64 old_capa = v -> capacity;
    v -> capacity = capa;
    v -> data = v -> arena == NULL ?
        ds_realloc(v -> data, capa * v -> block_size):
        realloc_on_arena(v -> arena, v -> data, old_capa * v -> block_size, capa * v -> block_size);
}

u64 vec_capacity(const Vector v){
    handle_err(
        v == NULL || v -> data == NULL,
        "Error getting capacity for the null Vector! Aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    return v -> capacity;
}

u8 vec_blocksize(const Vector v){
    handle_err(
        v == NULL || v -> data == NULL,
        "Error getting datasize for the null Vector! Aborting ...",
        cleanup();
        exit(EXIT_FAILURE);
    )
    return v -> block_size;
}
