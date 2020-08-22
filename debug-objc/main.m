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
        objb.objA = obja;
        NSObject *obj = objb.objA;
        NSLog(@"Hello, %@",objb);
    }
    return 0;
}
