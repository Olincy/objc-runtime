//
//  main.m
//  debug-objc
//
//  Created by Closure on 2018/12/4.
//

#import <Foundation/Foundation.h>
#import "MyObject.h"

@interface MyObjectA : NSObject
@property (atomic) NSObject *objA;
@end

@implementation MyObjectA
@end

@interface MyObjectB: NSObject
@property (atomic) NSObject *objA;
@end

@implementation MyObjectB
@end

static inline uint32_t ptr_hash(uint64_t key)
{
    key ^= key >> 4;
    key *= 0x8a970be7488fda55;
    key ^= __builtin_bswap64(key);
    return (uint32_t)key;
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
//        MyObject *my = [[MyObject alloc]init];
//        my = nil;
//        __weak MyObject *wObj = my;
////        wObj = [MyObject new];
//        NSLog(@"Hello, %@",my.name);
//        NSLog(@"Hello, %@",wObj);
////        __weak MyObject *my = [MyObject new];
////        NSLog(@"Hello, %@",my);
        MyObjectA *obja = [[MyObjectA alloc]init];
        MyObjectB *objb = [[MyObjectB alloc]init];
//        objb.objA = obja;
//        NSObject *obj = objb.objA;
        NSLog(@"A before:%0x",&obja);
        NSLog(@"A after:%x",ptr_hash(&obja)&3);
        
        NSLog(@"B before:%0x",&objb);
        NSLog(@"B after:%x",ptr_hash(&objb)&3);
        NSLog(@"Hello, %@",objb);
    }
    return 0;
}
