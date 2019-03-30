//
//  main.m
//  debug-objc
//
//  Created by Closure on 2018/12/4.
//

#import <Foundation/Foundation.h>
#import "MyObject.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        MyObject *my = [[MyObject alloc]init];
        
        NSLog(@"Hello, %@",my.name);
    }
    return 0;
}
