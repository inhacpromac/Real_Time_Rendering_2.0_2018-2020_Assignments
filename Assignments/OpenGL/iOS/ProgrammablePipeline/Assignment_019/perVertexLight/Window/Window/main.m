//
//  main.m
//  Window
//
//  Created by ROHAN WAGHMODE on 10/03/20.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc]init];
    
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    
    int iRect = UIApplicationMain(argc, argv, nil, appDelegateClassName);
    [pPool release];
    return iRect;
}
