
# Заметки по лабам

## ПУНКТ 1.1

thread ID нельзя сравнивать обычным способом, потому что ID может быть представлен разными способами - числом или структурой.

**man pthread_self NOTES:**
POSIX.1 allows an implementation wide freedom in choosing the type used to represent a thread ID; for example, representation using either an arithmetic type  or a structure is permitted. Therefore, variables of type pthread_t can't portably be compared using the C equality operator (==); use pthread_equal(3) instead.

## ПУНКТ 1.2

Завершения joinable потока можно дожидаться при помощи pthread_join, чтобы получить возвращаемое им значение и освободить ресурсы. Для detached потоков нельзя делать pthread_join. Их возвращаемое значение нельзя получить. Ресурсы освобождаются автоматически при завершении потока.

**man pthread_create NOTES (что такое joinable и detached потоки):**
A thread may either be joinable or detached. If a thread is joinable, then another thread can call pthread_join(3) to wait for the thread to terminate and fetch its exit status. Only when a terminated joinable thread has been joined are the last of its resources released back to the system. When a detached thread terminates, its resources are automatically released back to the system: it is not possible to join with the thread in order to obtain its exit status.Making a thread detached is useful for some types of daemon threads whose exit status the application does not need to care about. By default, a new thread is created in a joinable state...

**man pthread_self NOTES (про переиспользование thread ID):**
Thread IDs are guaranteed to be unique only within a process. A thread ID may be reused after a terminated thread has been joined, or a detached thread has terminated.

## ПУНКТ 1.3

Передача параметров в поточную функцию производится при помощи void *arg. Параметры надо хранить в той области памяти, которая остается валидной все время жизни потока.

Отсоединенные потоки не живут после того, как завершился main поток. При завершении программы завершаются все потоки.

**man pthread_detach NOTES:**
The detached attribute merely determines the behavior of the system when the thread terminates; it does not prevent the thread from being terminated if the  process terminates using exit(3) (or equivalently, if the main thread returns).

**man pthread_create DESCRIPTION:**
The new thread terminates in one of the following ways:
...
+ Any of the threads in the process calls exit(3), or the main thread performs a return from main(). This causes the termination of all threads in the process.

В целом можно хранить структуру в стековом кадре мейна, ведь если main завершится, то завершаются все потоки. Главное не хранить эту структуру на стеке какой-то другой функции, потому что она может завершиться раньше, чем detached поток, и данные будут не валидны. Безопаснее всего хранить в глобальной переменной или на куче (но тогда надо не забыть освободить память).

## ПУНКТ 1.4

У потока есть свойство cancelability state, которое определяет, можно ли прервать поток (по умолчанию можно). И есть свойство cancelability type, которое определяет, когда произойдет прерывание потока. По умолчанию это deferred (отложенное прерывание). Это значит, что поток будет прерван только тогда, когда дойдет до cancellation point. В качестве cancellation point выступают определенные функции, полный список в man 7 pthreads. Еще может быть asynchronous тип. Это значит, что поток может быть прерван в любое время (обычно сразу же, но не гарантированно). Для настройки используются функции pthread_setcancelstate(3) и pthread_setcanceltype(3). Подробнее в man pthread_cancel.

Если поток с отложенным прерыванием не вызывает функций, которые являются cancellation point, то его не получится прервать. Поэтому нужно установить асинхронный тип прерывания, чтобы поток можно было прервать в любое время.


При помощи clean-up handlers можно освобождать ресурсы в случае аварийного завершения потока. Они автоматически вызываются, когда мы делаем pthread_cancel или pthread_exit. С их помощью освобождаем память.

**man pthread_cleanup_push DESCRIPTION (про clean-up handlers):**
A clean-up handler is a function that is automatically executed when a thread is canceled (or in various other circumstances described below)...
A cancellation clean-up handler is popped from the stack and executed in the following circumstances:
1. When a thread is canceled, all of the stacked clean-up handlers are popped and executed in the reverse of the order in which they were pushed onto the stack.
2. When a thread terminates by calling pthread_exit(3), all clean-up handlers are executed as described in the preceding point. (Clean-up handlers are not called if the thread  terminates  by  performing a return from the thread start function.)
3. When a thread calls pthread_cleanup_pop() with a nonzero execute argument, the top-most clean-up handler is popped and executed.

pthread_cleanup_push и pthread_cleanup_pop реализованы как макрос и должны всегда использоваться в паре.

**man pthread_cleanup_push DESCRIPTION (про правильное использование push и pop в паре):**
POSIX.1 permits pthread_cleanup_push() and pthread_cleanup_pop() to be implemented as macros that expand to text containing '{' and '}', respectively. For this reason, the caller must ensure that calls to these functions are paired within the same function, and at the same lexical nesting level. (In other words, a clean-up handler is established only during the execution of a specified section of code.)

## ПУНКТ 1.5

**Выжимка из man 7 signal:**
Сигнал может быть обработан одним из трех способов:
+ действие по умолчанию (подробнее в man)
+ игнорирование сигнала
+ обработчик сигнала - пользовательская функция, которая автоматически вызывается при получении сигнала

Способ обработки конкретного сигнала можно устанавливать при помощи сисколов sigaction(2) или signal(2). Способ обработки сигнала - это атрибут, действующий в рамках всего процесса, он одинаков сразу для всех потоков. Сразу ответ - нельзя для каждого потока сделать свой обработчик.

Для отправки сигналов существует несколько сисколов и библиотченых функций, в т.ч.:
+ raise(3)        Отправляет сигнал вызывающему потоку
+ kill(2)         Отправляет сигнал указанному процессу/группе процессов/всем процессам в системе
+ pthread_kill(3) Отправляет сигнал указанному POSIX thread в том же процессе, что и вызывающий
+ tgkill(2)       Отправляет сигнал указанному потоку внутри указанного процесса (этот сискол используется в pthread_kill)
+ есть еще другие (подробнее в man)

Вместо того, чтобы асинхронно ловить сигнал в обработчике, можно принимать сигнал синхронно. Т.е. блокироваться до тех пор, пока сигнал не будет доставлен. Есть два способа, как это сделать:
+ sigwaitinfo(2), sigtimedwait(2), и sigwait(3) приостанавливают исполнение, пока один из указанных сигналов не будет доставлен. Эти функции возвращают информацию о доставленном сигнале
+ Можно еще через файловый дескриптор, но для лабы не актуально.

Сигнал может быть заблокирован. Это значит, что он не будет доставлен до тех пор, пока его не разблокируют. Во время между отправкой и доставкой сигнала он имеет стату pending. У каждого потока есть независимая signal mask, которая указывает, какие сигналы блокируются. Маской можно управлять при помощи pthread_sigmask(3). В однопоточной программе можно использовать sigprocmask(2).

Сигнал может быть адресован всему процессу или какому-то потоку отдельно (зависит от того, какой функцией отправлен сигнал, подробнее в man). Адресованный процессу может быть обработан любым потоком, не блокирующим этот сигнал.

Список стандартных сигналов и их номера можно посмотреть в man.

Есть еще real-time сигналы, но они нам сейчас не интересны.

**Полезные страницы man:**
+ signal(7) - про сигналы в целом
+ sigaction(2) - про добавления обработчика сигнала
+ sigprocmask(2), pthread_sigmask(3), sigsetops(3) - про настройку маски сигналов
+ sigwait(3) - про ожидание сигналов
+ pthread_kill(3) - про отправку сигналов POSIX потокам
