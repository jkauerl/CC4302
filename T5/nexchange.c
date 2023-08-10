#include "nthread-impl.h"

// Use los estados predefinidos WAIT_EXCHANGE y/o WAIT_EXCHANGE_TIMEOUT
// El descriptor de thread incluye el campo ptr para que Ud. lo use
// a su antojo.

// ... defina aca las estructuras que necesite ...

typedef struct {
  nThread thread;
} Message;

void* nExchange(nThread th, void *msg, int timeout) {
  START_CRITICAL
  nThread this_th = nSelf();
  Message message={th};
  this_th->ptr=&message;
  this_th->send.msg=msg;

  if(th->status == WAIT_EXCHANGE || th->status == WAIT_EXCHANGE_TIMEOUT) {
    if(((Message *)th->ptr)->thread == this_th) {
      if(th->status == WAIT_EXCHANGE_TIMEOUT) { 
        nth_cancelThread(th);
      }
      
      void *temp0= th->ptr;
      th->ptr=this_th->ptr;
      this_th->ptr=temp0;

      void *temp1=th->send.msg;
      th->send.msg=msg;
      this_th->send.msg=temp1;
      
      setReady(th);
    }
    else if(((Message *)th->ptr)->thread != this_th) {
      suspend(WAIT_EXCHANGE);
      schedule();
    }
  }
  else {
    if(timeout == -1 || timeout == 0) {
      suspend(WAIT_EXCHANGE);
    }
    else if(timeout > 0) {
      suspend(WAIT_EXCHANGE_TIMEOUT);
      nth_programTimer(timeout * 1000000, NULL);
    }
    schedule();
  }

  void *res= this_th->send.msg==msg ? NULL : this_th->send.msg;
  END_CRITICAL
  return res;
}
