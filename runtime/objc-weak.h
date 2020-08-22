/*
 * Copyright (c) 2010-2011 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _OBJC_WEAK_H_
#define _OBJC_WEAK_H_

#include <objc/objc.h>
#include "objc-config.h"

__BEGIN_DECLS

/*
The weak table is a hash table governed by a single spin lock.
 weak table是一个由一个spin lock保护的哈希表
An allocated blob of memory, most often an object, but under GC any such 
allocation, may have its address stored in a __weak marked storage location 
through use of compiler generated write-barriers or hand coded uses of the 
register weak primitive.
 alloc开辟出来的一小块内存块（通常是一个对象），在GC下，（由编译器自动生成写屏障（write-barriers）或由手写代码注册，）将这种对象存储在一个__weak标记的内存地址中
 Associated with the registration can be a callback
block for the case when one of the allocated chunks of memory is reclaimed. 

 The table is hashed on the address of the allocated memory.  When __weak
marked memory changes its reference, we count on the fact that we can still 
see its previous reference.
 该table由对象的内存地址做hash，当__weak标记的内存的引用被修改时，我们任然可以获知它之前的引用。

So, in the hash table, indexed by the weakly referenced item, is a list of 
all locations where this address is currently being stored.
 因此，在哈希表中（由弱引用项索引）是当前存储该地址的所有位置的列表。
 
For ARC, we also keep track of whether an arbitrary object is being 
deallocated by briefly placing it in the table just prior to invoking 
dealloc, and removing it via objc_clear_deallocating just prior to memory 
reclamation.

*/

// The address of a __weak variable.
// These pointers are stored disguised so memory analysis tools
// don't see lots of interior pointers from the weak table into objects.
typedef DisguisedPtr<objc_object *> weak_referrer_t;

#if __LP64__
#define PTR_MINUS_2 62
#else
#define PTR_MINUS_2 30
#endif

/**
 * The internal structure stored in the weak references table. 
 * It maintains and stores
 * a hash set of weak references pointing to an object.
 * If out_of_line_ness != REFERRERS_OUT_OF_LINE then the set
 * is instead a small inline array.
 */
#define WEAK_INLINE_COUNT 4

// out_of_line_ness field overlaps with the low two bits of inline_referrers[1].
// inline_referrers[1] is a DisguisedPtr of a pointer-aligned address.
// The low two bits of a pointer-aligned DisguisedPtr will always be 0b00
// 不管是64位系统还是32位系统，由于地址值的最低的两个位值一定是0b00，因此这两个位可以用于存储其他有用的信息，这边用来存储out-of-line的状态
// (disguised nil or 0x80..00) or 0b11 (any other address).
// Therefore out_of_line_ness == 0b10 is used to mark the out-of-line state.
#define REFERRERS_OUT_OF_LINE 2

struct weak_entry_t {
    // DisguisedPtr<T> 实际上是T* 的封装，可以完全等同于T* ，将T* 伪装一下是的避免被类似leak的内存检测工具识别。
    DisguisedPtr<objc_object> referent;
    union {
        struct {
            weak_referrer_t *referrers; // typedef DisguisedPtr<objc_object *> weak_referrer_t; // 64bit * 1 -2bit
            uintptr_t        out_of_line_ness : 2; // 用于标记是否是out_of_line，0b10表示out_of_line
            // 如果out_of_line的话，数据存放在referrers数组中，否则的话，存放在inline_referrers这个内部小数组(长度只有4)
            uintptr_t        num_refs : PTR_MINUS_2;  // 62bit
            uintptr_t        mask; // 64bit * 1
            uintptr_t        max_hash_displacement; // 64bit * 1
        };
        struct {
            // out_of_line_ness field is low bits of inline_referrers[1]
            // inline_referrers[1]的低两位表示out_of_line_ness
            weak_referrer_t  inline_referrers[WEAK_INLINE_COUNT];// 64bit * 4
        };
    };

    bool out_of_line() {
        return (out_of_line_ness == REFERRERS_OUT_OF_LINE); //out_of_line_ness == 0b10表示out-of-line状态
    }

    weak_entry_t& operator=(const weak_entry_t& other) {
        memcpy(this, &other, sizeof(other));
        return *this;
    }
    
    // 构造函数后加冒号是初始化表达式，这边表示调用了成员类referent的构造函数
    weak_entry_t(objc_object *newReferent, objc_object **newReferrer)
        : referent(newReferent)
    {
        inline_referrers[0] = newReferrer;
        for (int i = 1; i < WEAK_INLINE_COUNT; i++) {
            inline_referrers[i] = nil;
        }
    }
};

/**
 * The global weak references table. Stores object ids as keys,
 * and weak_entry_t structs as their values.
 * 全局weak引用表，存储ids作为key，weak_entry_t作为value
 */
struct weak_table_t {
    weak_entry_t *weak_entries;
    size_t    num_entries;
    uintptr_t mask;
    uintptr_t max_hash_displacement;
};

/// Adds an (object, weak pointer) pair to the weak table.
id weak_register_no_lock(weak_table_t *weak_table, id referent, 
                         id *referrer, bool crashIfDeallocating);

/// Removes an (object, weak pointer) pair from the weak table.
void weak_unregister_no_lock(weak_table_t *weak_table, id referent, id *referrer);

#if DEBUG
/// Returns true if an object is weakly referenced somewhere.
bool weak_is_registered_no_lock(weak_table_t *weak_table, id referent);
#endif

/// Called on object destruction. Sets all remaining weak pointers to nil.
void weak_clear_no_lock(weak_table_t *weak_table, id referent);

__END_DECLS

#endif /* _OBJC_WEAK_H_ */
