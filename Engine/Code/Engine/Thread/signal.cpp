#include "Engine/Thread/signal.h"

Signal::Signal()
{
   os_event = ::CreateEvent( nullptr, // security attributes, not needed
      TRUE,                           // Manual reset - do we manually reset, or auto reset after this is hit.
      FALSE,                          // initial state of the signal
      NULL );                         // name, used for cross-process communication
}

//------------------------------------------------------------------------
Signal::~Signal()
{
   ::CloseHandle( os_event );
   os_event = NULL;
   // nothing needed since we're destructing.
}

//------------------------------------------------------------------------
void Signal::signal_all()
{
   ::SetEvent( os_event );
}

//------------------------------------------------------------------------
void Signal::wait()
{
   DWORD result = ::WaitForSingleObject( os_event, INFINITE );
   if (result == WAIT_OBJECT_0) {
      // great, we succeeded!  [you can wait for multiple objects,
      // hence the WAIT_OBJECT_0 - meaning our the first object was
      // the one who caused us to activate.

      // this means everyone who is waiting is going to reset, but oh well - they already
      // got woken up. 
      ::ResetEvent(os_event);
   }
}


bool Signal::wait_for(unsigned int ms) 
{
   DWORD result = ::WaitForSingleObject( os_event, ms );
   if (result == WAIT_OBJECT_0) {
      // great, we succeeded!  [you can wait for multiple objects,
      // hence the WAIT_OBJECT_0 - meaning our the first object was
      // the one who caused us to activate.

      // this means everyone who is waiting is going to reset, but oh well - they already
      // got woken up. 
      ::ResetEvent(os_event);
      return true;
   }

   return false;
}