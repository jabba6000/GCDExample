//
//  NSObject_Guide.h
//  GCDExample
//
//  Created by Uri Fuholichev on 10/12/16.
//  Copyright © 2016 Andrei Karpenia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSObject ()

/*
 1) ВЫПОЛНЕНИЕ В БЭКГРАУНДЕ КАКОЙ-ТО ЗАДАЧИ, А ПОТОМ, КОГДА ДАННЫЕ ПРИХОДЯТ, ОНА ВЫСТРЕЛИВАЕТ В ГЛАВНЫЙ ПОТОК (обновляет UI)
 */

// ПРИМЕР: нужно подгружать из Интернета или выполнить ресурсоемкий процесс в бэкграунде, а потом выстрелить в MAIN
// после того как dispatch_async возвращает управление, отемнить ее в очереди уже нельзя.
dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
    // переводим работу с главного потока на глобальную очередь, но отсюда нельзя обновлять UI
    NSLog(@"1");
    dispatch_async(dispatch_get_main_queue(), ^{
        // Отсюда можем например обновлять UI
        NSLog(@"2");
    });
 });

/*
 2) ЕСЛИ НУЖНО ВЫПОЛНИТЬ КАКОЕ-ТО ДЕЙСТВИЕ ПОСЛЕ ОТСРОЧКИ
 */

// ПРИМЕР: хз. Суть сводится к тому, что dispatch_after это  то же, что и dispatch_async только после задержки
//задаем время в секундах
double delayInSeconds = 1.0;
//создаем GCD переменную для временной задержки
dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
//выполняем вывод сообщения в главном потоке после задержки popTime
dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
    NSLog(@"Hello after delay");
});

/*
3) СИНГЛТОН
*/

// ПРИМЕР: доступ к свойствам синглтона из разных контроллеров. Но: обычно НЕ потокобезопасны.
// ПРИМЕР: объект-контроллер, который тоже синглтон, также непотокобезопасен, поэтому
// dispatch_once_t - эта команда выполнит блок только один единственный раз и блок будет потокобезопасным
+ (Singleton *)sharedInstance {
    static dispatch_once_t pred;
    static Singleton *sharedInstance = nil;
    dispatch_once(&pred, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}
// пока в блоке dispatch_once есть поток, другие потоки туда не будут допущены, пока критическая секция не будет выполнена
// таким образом последующие проверки объекта синглтона на nil выдадут, что такой объект уже есть и другие потоки не смогут создать еще один объект Синглтона


@end
