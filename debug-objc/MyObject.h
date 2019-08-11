//
//  MyObject.h
//  debug-objc
//
//  Created by candy on 2019/3/30.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MyObject : NSObject

@property (nonatomic) NSString *name;
@property (weak, nonatomic) NSObject *aweakOBj;

@end

NS_ASSUME_NONNULL_END
