//
//  ViewController.h
//  GCDExample
//
//  Created by Uri Fuholichev on 10/12/16.
//  Copyright © 2016 Andrei Karpenia. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController

// ПОРЯДОК РАБОТЫ С GCD ВНУТРИ NSObject_Guide.h

/*
ОПРЕДЕЛЕНИЯ

 Serial vs. concurent
 Серийные задачи выполняются по одной за раз
 Конкрурентные (параллельные) задачи могут выполнятся по нескольку за раз
 
 Synchronous vs. Asynchronous
 Синхронные функции возвращают управление только после того, как выполняется задача, вызванная асинхронной функцией.
 Асинхронные функции - приказывают выполнять задачу, но сразу же возвращают управление. То есть текущий поток не блокируется
 
 Критическая секция - кусок кода в этой секции не может быть выполнен конкурентно, то есть из двух потоков за раз. Это уместно, когда коду правляется ресурсом, например, переменной, которая может быть повреждена, если до нее достучаться из разных потоков
 Deadlock - 2 или более потока ждут пока каждый из них ждет выполнения друг друга и в итоге они оба не могут получить доступ к коду (первый ждет второго, второй - первого)
 Потокобезопасный код - пример: NSDictionary - можно безопасно использовать из разных потоков. А вот NSMutableDictionary - нет, т.к. потоки могут изменять словарь и может получится дата коррапшн.
 Context Switch - процесс сохраниения или активации состояния выполнения, когда мы переключаемся между потоками при выполнении одного процесса.
 
 Concurrency vs Parallelism
Параллелизм - при нескольких ядрах запуск параллельно нескольких потоков.
 Конкуренси - при одном ядре, код якобы выполняется синхронно, на самом деле просто быстро переключатеся между потоками. Пример: бегает первый поток, потом срабатывает  context switch, переключается на второй процесс или поток, первый при этом заморожен. Скорее как имитация многопоточности
 
 Очереди
Dispatch queues - они потокобезопасны. Очередь гарантирует, что добавленные в нее задачи будут выполнены в FIFO порядке.
Serial Queues - выполняются по одной за раз, каждая задача начианется только после выполнения предыдущей. Выполняются в том же порядке, в каком были добавлены в очередь. Исключена возможность race conditions
 Concurrent Queues - старт выполнения задач будет осуществляться в том порядке, в каком задачи были добавлены в очередь, а вот заканчиватья они будут хз в каком порядке
 
 Типы очередей GCD
 Main queue - как и в серийных очередях, только по одной задаче за раз в этой очереди. Гарантирует, что задача будет выполнена в главном потоке (можно обновлять UI)
4 Global Dispatch Queues
 1) background
 2) low
 3) default
 4) high
 5) своя кастомная очередь
 */

@end

