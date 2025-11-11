#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <arena.h>                // personal arena lib

#define UNSORTED    0
#define SORTED      1


typedef uint64_t u64;
typedef int64_t  i64;
typedef uint32_t u32;
typedef uint8_t  u8;

typedef struct vector* Vector;

Vector vec_init_(u64 def, u8 block_size);
Vector vec_arena_(u64 def, u8 block_size, Arena arena);
void   vec_push(Vector, void*) __attribute__((nonnull(1,2)));
void   vec_pop_(Vector, void*) __attribute__((nonnull(1)));
void   vec_get_(const Vector, u64 index, void* gottem)  __attribute__((nonnull(1, 3)));;
void   vec_insert(Vector, void*, u64) __attribute__((nonnull(1,2)));
void   vec_set(Vector, void*, u64) __attribute__((nonnull(1,2))); 
void   vec_remove_(Vector, u64, void* gottem) __attribute__((nonnull(1)));
void   vec_dbg(const Vector) __attribute__((nonnull(1)));
void   vec_print(Vector, void(*)(void*)) __attribute__((nonnull(1,2)));
void   vec_sort(Vector in, Vector out, int (*cmp)(void*, void *)) __attribute__((nonnull(1,2,3)));
void   vec_reverse(Vector in, Vector out) __attribute__((nonnull(1,2)));
void   vec_join(Vector, const Vector) __attribute__((nonnull(1,2)));
bool   vec_empty(const Vector) __attribute__((nonnull(1)));
bool   vec_in(const Vector, void*) __attribute__((nonnull(1,2)));
bool   vec_eq(const Vector, const Vector) __attribute__((nonnull(1,2)));
u64    vec_len(const Vector) __attribute__((nonnull(1)));
void   vec_copy(const Vector src, Vector dest) __attribute__((nonnull(1,2)));
u64    vec_count(const Vector, void*) __attribute__((nonnull(1,2)));
void   vec_clear(const Vector) __attribute__((nonnull(1)));
u64    vec_search(const Vector, void* x, bool* sorted_found, int (*cmp)(void*, void*)) __attribute__((nonnull(1,2,3,4)));
void   vec_slice(const Vector, Vector, const u64 low, const u64 high) __attribute__((nonnull(1,2)));
// the third argument is a pointer to a boolean value (sorted or unsorted)
// since if the vector is already sorted searching can be optimized 
// after the search if the element is found or not is written to that same 
// variable
void  vec_map_(const Vector input, Vector output, void* (*mapper)(void*),
        u8 input_size, u8 output_size) __attribute__((nonnull(1,2,3)));
void  vec_fit(Vector v) __attribute__((nonnull(1)));
void  vec_setcapacity(Vector v, u64 capa) __attribute__((nonnull(1)));
u64   vec_capacity(const Vector v) __attribute__((nonnull(1)));
u8    vec_blocksize(const Vector v) __attribute__((nonnull(1)));

#define vec_init(T, n, a)                                                             \
    a == NULL ? vec_init_(n, sizeof(T)) : vec_arena_(n, sizeof(T), a);                \

#define DEFINE_VECPRINT(T)                                                            \
    void vec_print_##T(const Vector v, void(*printer)(T)){                            \
        const void tmp(void* x){                                                      \
            printer(*(T*){x});                                                        \
        }                                                                             \
        vec_print((v), (tmp));                                                        \
    }

#define vec_get(v, i, T)                                                              \
    ({                                                                                \
        T odfijoijqewofjoqwej;                                                        \
        vec_get_((v), (i), &odfijoijqewofjoqwej);                                     \
        odfijoijqewofjoqwej;                                                          \
    })

#define vec_pop(v, T)                                                                 \
    ({                                                                                \
        T jf90f4190j13212903r;                                                        \
        vec_pop_((v), &jf90f4190j13212903r);                                          \
        jf90f4190j13212903r;                                                          \
    })

#define vec_remove(v, i, T)                                                           \
    ({                                                                                \
        T f12983u4021jf2190uj;                                                        \
        vec_remove_((v), (i), &f12983u4021jf2190uj);                                  \
        f12983u4021jf2190uj;                                                          \
    })

#define vec_map(in, out, f, T, U)                                                     \
    do{                                                                               \
        vec_map_((in), (out), (f), sizeof(T), sizeof(U));                             \
    } while(0)

/*
#define vec(T, ...)                                                                   \
    ({                                                                                \
        T args[] = { __VA_ARGS__ };                                                   \
        u64 length = sizeof(args) / sizeof(T);                                        \
        Vector v = vec_init(T, length, NULL);                                         \
        for(u64 i = 0; i < length; i++)                                               \
            vec_push(v, args + i);                                                    \
        v;                                                                            \
    })
*/

#define vec(T, a, ...)                                                                \
    ({                                                                                \
        T args[] = { __VA_ARGS__ };                                                   \
        u64 length = sizeof(args) / sizeof(T);                                        \
        Vector v = vec_init(T, length, a);                                            \
        for(u64 i = 0; i < length; i++)                                               \
            vec_push(v, args + i);                                                    \
        v;                                                                            \
    })

#endif
