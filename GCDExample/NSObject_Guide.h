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
// dispatch_once() - эта команда выполнит блок только один единственный раз и блок будет потокобезопасным
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
// Так мы создали только безопасное создание объекта Синглтона, однако для его переменных потокобезопасность нужно настраивать отдельно
/*
4) ПОТОКОБЕЗОПАСНОСТЬ, БАРЬЕРЫ, WRITE-методы
*/

/*
 Список потокобезпасных и небезопасных классаов
 https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Multithreading/ThreadSafetySummary/ThreadSafetySummary.html
 GCD барьеры - все элементы, потсавленные в очередь до отправки в GCD-барьер, должны быть выполнены, прежде чем блок выполнится. Когда очередь блока приходит, барьер выполняет блок и следит, чтоьбы очередь не выполняла какой-либо еще блок, пока не выполнится блок внутри барьера
 -----\          /----
       ==БАРЬЕР==
 -----/          \----
 Итак, порядок такой: допустим, нам надо выполнить write метод и записать данные в массив photoArray. Для этого создаем массив-свойство и создаем свойство для кастомной очереди:
 @property (nonatomic,strong,readonly) NSMutableArray *photosArray;
 @property (nonatomic, strong) dispatch_queue_t concurrentPhotoQueue;
 ....
 Затем 	
 1) проверяем, есть ли фото, чтобы добавить его в массив
 2) создаем операцию write, используя кастомную очередь
 3) этот код добавляет объект в массив. Т.к. это барьерный блок, этот блок никогда не будет вызван синхронно с каким-либое еще блоком
 4) Наконец мы отправляем нотификацию о том, что добавили картинку. Посылаем нотификацию асинхронно, тк, нужно обновить UI
 */
 - (void)addPhoto:(Photo *)photo
 {
     if (photo) { // 1
        dispatch_barrier_async(self.concurrentPhotoQueue, ^{ // 2
            [_photosArray addObject:photo]; // 3
            dispatch_async(dispatch_get_main_queue(), ^{ // 4
                [self postContentAddedNotification];
            });
        });
     }
 }
/*
 5) ПОТОКОБЕЗОПАСНОСТЬ, READ-методы
 Допустим, нам дадо получить текущий массив фотографий. Для этого мы будем выполнять запрос в той же кастомной очереди, которую создали выше concurrentPhotoQueue. Однако асинхронный запрос здесь не очень. Поэтому синхронный запрос dispatch_sync(). Это используется, когда нужно например дождаться прихода данных для работы с ними. Но здесь высока опасность deadlock, когда вечно будет дожидаться конца блока, который не может быть выполнен.
 */
 - (NSArray *)photos
 {
     __block NSArray *array; // 1
         dispatch_sync(self.concurrentPhotoQueue, ^{ // 2
             array = [NSArray arrayWithArray:_photosArray]; // 3
         });
     return array;
 }
/*
 1) __block - создает мутабельный аналог объекта
 2) на нашей кастомной очереди concurentPhotoQueue команда будет выполнена синхронно
 3) возвращаем массив текущих фотографий
 НАКОНЕЦ, нужно внутри ранее созданного синглтона подтвердить создание concurenPhotoQueue очереди:
*/
+ (instancetype)sharedManager
{
    static PhotoManager *sharedPhotoManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedPhotoManager = [[PhotoManager alloc] init];
        sharedPhotoManager->_photosArray = [NSMutableArray array];
        
        // ADD THIS:
        sharedPhotoManager->_concurrentPhotoQueue = dispatch_queue_create("com.selander.GooglyPuff.photoQueue",
                                                                          DISPATCH_QUEUE_CONCURRENT);
    });
    
    return sharedPhotoManager;
}
/*
 Здесь важные моменты такие: 
 1) создание очереди посредством dispatch_queue_create()
 2) первый параметр - имя очереди по конвенции DNS.
 3) второй параметр - конкурентная или последовательная очередь (какую хотим сами). 0 или NULL - это последовательная очередь
 */

@end
