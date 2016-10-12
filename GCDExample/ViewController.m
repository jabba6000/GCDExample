//
//  ViewController.m
//  GCDExample
//
//  Created by Uri Fuholichev on 10/12/16.
//  Copyright © 2016 Andrei Karpenia. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // ПРИМЕР: нужно подгружать из Интернета или выполнить ресурсоемкий процесс в бэкграунде, а потом выстрелить в MAIN
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
        // переводим работу с главного потока на глобальную очередь, но отсюда нельзя обновлять UI
        NSLog(@"1");
        dispatch_async(dispatch_get_main_queue(), ^{
            // А таперь, когда отыграет событие в главной очереди, переходим в главный поток и делаем
            // Отсюда можем например обновлять UI
            NSLog(@"2");
        });
    });

    // ПРИМЕР: вывод сообщения после трехсекундной задержки
    double delayInSeconds = 3.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC)); // 1
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){ // 2
        NSLog(@"Hello after delay");
    });
}


@end
