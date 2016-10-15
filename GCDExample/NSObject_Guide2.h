//
//  NSObject_Guide2.h
//  GCDExample
//
//  Created by Uri Fuholichev on 10/15/16.
//  Copyright © 2016 Andrei Karpenia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSObject ()

/*
 1) DISPATCH GROUP - используется, когда нужно дождаться выполнения всех асинхроных команд в очереди. Команды могут быть синхронными и асинхронными, могут быть из разных очередей. Уведомление о выполнении всех команд может прийти также в асинхронном порядке (главный поток не заблокирован) или синхронном (то есть пока не отыграют все асинхронные команды и не придет уведомление об этом, UI будет заблокирован)
 */

// DISPATCH_GROUP_WAIT

- (void)downloadPhotosWithCompletionBlock:(BatchPhotoDownloadingCompletionBlock)completionBlock
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{ // 1
        
        __block NSError *error;
        dispatch_group_t downloadGroup = dispatch_group_create(); // 2
        
        for (NSInteger i = 0; i < 3; i++) {
            NSURL *url;
            switch (i) {
                case 0:
                    url = [NSURL URLWithString:kOverlyAttachedGirlfriendURLString];
                    break;
                case 1:
                    url = [NSURL URLWithString:kSuccessKidURLString];
                    break;
                case 2:
                    url = [NSURL URLWithString:kLotsOfFacesURLString];
                    break;
                default:
                    break;
            }
            
            dispatch_group_enter(downloadGroup); // 3
            Photo *photo = [[Photo alloc] initwithURL:url
                                  withCompletionBlock:^(UIImage *image, NSError *_error) {
                                      if (_error) {
                                          error = _error;
                                      }
                                      dispatch_group_leave(downloadGroup); // 4
                                  }];
            
            [[PhotoManager sharedManager] addPhoto:photo];
        }
        dispatch_group_wait(downloadGroup, DISPATCH_TIME_FOREVER); // 5
        dispatch_async(dispatch_get_main_queue(), ^{ // 6
            if (completionBlock) { // 7
                completionBlock(error);
            }
        });
    });
}
/*
 1) чтобы не блокировать главный поток  выполняем задачу в баграунде
 2) Создаем новую dispatch группу - это что-то вроде счетчика незавершенным задачам
 3) вручную уведомляем группу, что задача началась
 4) вручную уведомляем группу, что задача завершена (для каждого group_enter должно быть group_leave инач)
 5) dispatch_group_wait будет ждать, пока или все задачи не выполнятся или пока не закончится время. В нашем случае DISPATCH_TIME_FOREVER означает, что будет ждать, пока не выполнятся все задачи
 6) к этому моменту гарантировано либо выйдет время ожидания, либо все задачи будут выполнены и мы делаем колбэк в главную очередь и запускаем completion block
 7) проверка блока завершения на nil, если есть, то запустить.
 */

// DISPATCH_GROUP_NOTIFY
- (void)downloadPhotosWithCompletionBlock:(BatchPhotoDownloadingCompletionBlock)completionBlock
{
    // 1
    __block NSError *error;
    dispatch_group_t downloadGroup = dispatch_group_create();
    
    for (NSInteger i = 0; i < 3; i++) {
        NSURL *url;
        switch (i) {
            case 0:
                url = [NSURL URLWithString:kOverlyAttachedGirlfriendURLString];
                break;
            case 1:
                url = [NSURL URLWithString:kSuccessKidURLString];
                break;
            case 2:
                url = [NSURL URLWithString:kLotsOfFacesURLString];
                break;
            default:
                break;
        }
        
        dispatch_group_enter(downloadGroup); // 2
        Photo *photo = [[Photo alloc] initwithURL:url
                              withCompletionBlock:^(UIImage *image, NSError *_error) {
                                  if (_error) {
                                      error = _error;
                                  }
                                  dispatch_group_leave(downloadGroup); // 3
                              }];
        
        [[PhotoManager sharedManager] addPhoto:photo];
    }
    
    dispatch_group_notify(downloadGroup, dispatch_get_main_queue(), ^{ // 4
        if (completionBlock) {
            completionBlock(error);
        }
    });
}
/*
 1) Тот же код. НО! здесь не надо указывать, чтобы выполнялся асинхронно - это делается по умолчанию
 2) Такой же вход в группу
 3) Такой же выход из группы как в примере выше
 4) dispatch_group_notify - это асинхронный блок, котоый выполняется, когда завершены все задачи в группе. Вторым аргументом у него служит очередь, на которой нужно отыграть блок выполнения.
 */

// DISPATCH_APPLY для ускорения итерации
- (void)downloadPhotosWithCompletionBlock:(BatchPhotoDownloadingCompletionBlock)completionBlock
{
    __block NSError *error;
    dispatch_group_t downloadGroup = dispatch_group_create();
    
    dispatch_apply(3, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^(size_t i) {//1
        
        NSURL *url;
        switch (i) {
            case 0:
                url = [NSURL URLWithString:kOverlyAttachedGirlfriendURLString];
                break;
            case 1:
                url = [NSURL URLWithString:kSuccessKidURLString];
                break;
            case 2:
                url = [NSURL URLWithString:kLotsOfFacesURLString];
                break;
            default:
                break;
        }
        
        dispatch_group_enter(downloadGroup);
        Photo *photo = [[Photo alloc] initwithURL:url
                              withCompletionBlock:^(UIImage *image, NSError *_error) {
                                  if (_error) {
                                      error = _error;
                                  }
                                  dispatch_group_leave(downloadGroup);
                              }];
        
        [[PhotoManager sharedManager] addPhoto:photo];
    });
    
    dispatch_group_notify(downloadGroup, dispatch_get_main_queue(), ^{
        if (completionBlock) {
            completionBlock(error);
        }
    });
}
/*
 1)  dispatch_apply использован здесь вместо цикла for. Он также как и for синхронный, то есть возвращает управление только по выполнению, но зато быстрее. В качестве параметров: сначала количество итераций, потом очередь, на которой выполнять, наконец действие блока в качестве третьего параметра.
 */

//SEMAPHORE
- (void)downloadImageURLWithString:(NSString *)URLString
{
    // 1
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    
    NSURL *url = [NSURL URLWithString:URLString];
    __unused Photo *photo = [[Photo alloc]
                             initwithURL:url
                             withCompletionBlock:^(UIImage *image, NSError *error) {
                                 if (error) {
                                     XCTFail(@"%@ failed. %@", URLString, error);
                                 }
                                 
                                 // 2
                                 dispatch_semaphore_signal(semaphore);
                             }];
    
    // 3
    dispatch_time_t timeoutTime = dispatch_time(DISPATCH_TIME_NOW, kDefaultTimeoutLengthInNanoSeconds);
    if (dispatch_semaphore_wait(semaphore, timeoutTime)) {
        XCTFail(@"%@ timed out", URLString);
    }
}
/*
 1) создаем семофор. параметр - величина, с которой стартует семофор. Это количество штуковин, что могут получить доступ к семафору не вызывая чего-либо, чтобы увеличить его сначала
 2) Сообщаем семаформу, что больше не нуждаемся в ресурсе. Это увеличивает счет семафора и сигнализирует, что семафор свободен  для других ресурсов, что хотят его
 3) Здесь блокируется текущий поток, пока семафор ен просигнализирует
 */

@end
