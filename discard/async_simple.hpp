
#ifndef FAST_ASYNC_SIMPLE_DISABLE_COROUTINES
#define FAST_ASYNC_SIMPLE_DISABLE_COROUTINES


//#include "async_simple/coro/Collect.h"
//#include "async_simple/coro/ConditionVariable.h"
//#include "async_simple/coro/FutureAwaiter.h"
//#include "async_simple/coro/Latch.h"
#include "../thirdparty/async_simple/coro/Lazy.h"
#include "../thirdparty/async_simple/coro/Mutex.h"
#include "../thirdparty/async_simple/coro/Semaphore.h"
#include "../thirdparty/async_simple/coro/Sleep.h"
#include "../thirdparty/async_simple/coro/SyncAwait.h"

#ifndef ASYNC_SIMPLE_MODULES_HAS_NOT_UTHREAD
#include "../thirdparty/async_simple/uthread/Async.h"
#include "../thirdparty/async_simple/uthread/Await.h"
#include "../thirdparty/async_simple/uthread/Collect.h"
#include "../thirdparty/async_simple/uthread/Latch.h"
#include "../thirdparty/async_simple/uthread/Uthread.h"
#endif

#include "../thirdparty/async_simple/Collect.h"
#include "../thirdparty/async_simple/Future.h"

#include "../thirdparty/async_simple/util/Condition.h"

#endif