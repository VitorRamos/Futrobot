#ifndef _TRANSMISSION_H_
#define _TRANSMISSION_H_

#define _TRANSMITION_BLUETOOTH_

/******************************************************/
/* Funções de comunicação com os robôs                */
/******************************************************/

#include "futdata.h"

#ifndef _SO_SIMULADO_
#ifndef _TRANSMITION_BLUETOOTH_
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#else  //#ifndef _TRANSMITION_BLUETOOTH_
#include <bluetoothAction.h>

#define ID_ROBO_0 "20:15:07:14:12:00"
#define ID_ROBO_1 "98:D3:31:B4:34:01"
#define ID_ROBO_2 "20:15:07:14:31:79"
 
#endif //#ifndef _TRANSMITION_BLUETOOTH_
#endif //#ifndef _SO_SIMULADO_


class Transmission :
  public virtual FutData
{
 
private:
#ifndef _SO_SIMULADO_
  #ifndef _TRANSMITION_BLUETOOTH_
  int serial_fd; //process ID info for forking and file descriptor for serial stream
  #else  //#ifndef _TRANSMITION_BLUETOOTH_
  BluetoothAction btAction;// = BluetoothAction;
  #endif //#ifndef _TRANSMITION_BLUETOOTH_
#endif //#ifndef _SO_SIMULADO_

public:
  Transmission(TEAM team, SIDE side, GAME_MODE mode);
  ~Transmission();
  bool transmission();

};

#endif //_TRANSMISSION_H_
