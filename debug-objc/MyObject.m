//
//  MyObject.m
//  debug-objc
//
//  Created by candy on 2019/3/30.
//

#import "MyObject.h"

@implementation MyObject

- (instancetype)init {
    if (self = [super init]) {
        self.aweakOBj = [NSObject new];
    }
    return self;
}

- (NSString *)name {
    return @"haha";
}
@end
