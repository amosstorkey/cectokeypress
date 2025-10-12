 /*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */
#include "env.h"
#include "cec.h"

#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>


#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/uinput.h>
//#include "input-event-codes.h"
#include "cecloader.h"
using namespace CEC;

ICECCallbacks         g_callbacks;
libcec_configuration  g_config;
int                   g_cecLogLevel(-1);
int                   g_cecDefaultLogLevel(CEC_LOG_ALL);
std::ofstream         g_logOutput;
bool                  g_bShortLog(false);
std::string           g_strPort;
bool                  g_bSingleCommand(false);
volatile sig_atomic_t g_bExit(0);
bool                  g_bHardExit(false);
ICECAdapter*          g_parser;
int                   keyList[40];
int                   keynumber;
std::map<int, int>    keyMap;
int PressKey(const int keydown);
void emit(int fd, int type, int code, int val)
struct uinput_setup usetup;
int fd;
//int opendev;
//struct libevdev *keydev;
//struct libevdev_uinput *uidev;

void populateKeyMapDefault()
{
  std::cout << "Building Map" <<std::endl;
  keyMap[CEC_USER_CONTROL_CODE_PAGE_UP] = KEY_PAGEUP; keyList[0] = KEY_PAGEUP;
  keyMap[CEC_USER_CONTROL_CODE_PAGE_DOWN] = KEY_PAGEDOWN; keyList[1] = KEY_PAGEDOWN;
  keyMap[CEC_USER_CONTROL_CODE_CHANNEL_UP] = KEY_HOME; keyList[2] = KEY_HOME;
  keyMap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN] = KEY_END; keyList[3] = KEY_END;
  keyMap[CEC_USER_CONTROL_CODE_LEFT] = KEY_LEFT; keyList[4] = KEY_LEFT;
  keyMap[CEC_USER_CONTROL_CODE_RIGHT] = KEY_RIGHT; keyList[5] = KEY_RIGHT;
  keyMap[CEC_USER_CONTROL_CODE_DOWN] = KEY_DOWN; keyList[6] = KEY_DOWN;
  keyMap[CEC_USER_CONTROL_CODE_UP] = KEY_UP; keyList[7] = KEY_UP;
  keyMap[CEC_USER_CONTROL_CODE_SELECT] = KEY_SPACE; keyList[8] = KEY_SPACE;
  keyMap[CEC_USER_CONTROL_CODE_EXIT] = KEY_ENTER; keyList[9] = KEY_ENTER;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER0] = KEY_0; keyList[10] = KEY_0;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER1] = KEY_1; keyList[11] = KEY_1;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER2] = KEY_2; keyList[12] = KEY_2;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER3] = KEY_3; keyList[13] = KEY_3;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER4] = KEY_4; keyList[14] = KEY_4;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER5] = KEY_5; keyList[15] = KEY_5;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER6] = KEY_6; keyList[16] = KEY_6;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER7] = KEY_7; keyList[17] = KEY_7;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER8] = KEY_8; keyList[18] = KEY_8;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER9] = KEY_9; keyList[19] = KEY_9;
  keyMap[CEC_USER_CONTROL_CODE_RECORD] = KEY_R; keyList[20] = KEY_R;
  keyMap[CEC_USER_CONTROL_CODE_PLAY] = KEY_P; keyList[21] = KEY_P;
  keyMap[CEC_USER_CONTROL_CODE_FAST_FORWARD] = KEY_F; keyList[22] = KEY_F;
  keyMap[CEC_USER_CONTROL_CODE_REWIND] = KEY_B; keyList[23] = KEY_B;
  keyMap[CEC_USER_CONTROL_CODE_STOP] = KEY_Z; keyList[24] = KEY_Z;
  keyMap[CEC_USER_CONTROL_CODE_PAUSE] = KEY_X; keyList[25] = KEY_X;
  keyMap[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE] = KEY_G; keyList[26] = KEY_G;
  keyMap[CEC_USER_CONTROL_CODE_F1_BLUE] = KEY_F1; keyList[27] = KEY_F1;
  keyMap[CEC_USER_CONTROL_CODE_F2_RED] = KEY_F2; keyList[28] = KEY_F2;
  keyMap[CEC_USER_CONTROL_CODE_F3_GREEN] = KEY_F3; keyList[29] = KEY_F3;
  keyMap[CEC_USER_CONTROL_CODE_F4_YELLOW] = KEY_F4; keyList[30] = KEY_F4;
  keyMap[CEC_USER_CONTROL_CODE_AN_RETURN] = KEY_ENTER; keyList[31] = KEY_ENTER;
  keyMap[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL] = KEY_K; keyList[32] = KEY_K;
  keyMap[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST] = KEY_L; keyList[33] = KEY_L;
  keynumber = 34;
}


inline bool HexStrToInt(const std::string& data, uint8_t& value)
{
  int iTmp(0);
  if (sscanf(data.c_str(), "%x", &iTmp) == 1)
  {
    if (iTmp > 256)
      value = 255;
	  else if (iTmp < 0)
      value = 0;
    else
      value = (uint8_t) iTmp;

    return true;
  }

  return false;
}


void CecKeyPress(void *UNUSED(cbParam), const cec_keypress* keyptr)
{
  cec_keypress key = *keyptr;
  std::cout << "CeCKeyPress" << std::endl;
  if (key.duration == 0)
  {
    std::cout << "key " <<  key.keycode << std::endl;
    int keydown = -1;
    if (keyMap.find(key.keycode) != keyMap.end())
    {
      keydown = keyMap[key.keycode];
      PressKey(keydown);
    }
    std::cout << "remote key code: " << key.keycode << ", keyboard output: " << keydown << std::endl;
  }    
}

void CecCommand(void *UNUSED(cbParam), const cec_command* commandptr)
{
  cec_command command = *commandptr;
  std::cout << "CeC Command" << std::endl;
  std::cout << "commandissued " << command.opcode << "versus" << CEC_OPCODE_DECK_CONTROL << " option " << int(command.parameters[0]) << int(CEC_USER_CONTROL_CODE_AN_RETURN) <<std::endl;
  switch (command.opcode)
  {
    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
      if (command.parameters.size ==1 &&
           command.initiator == CECDEVICE_TV)
        if (command.parameters[0] == CEC_USER_CONTROL_CODE_AN_RETURN)
        {
          int keydown = keyMap[CEC_USER_CONTROL_CODE_AN_RETURN];
          PressKey(keydown);
        }
        if (command.parameters[0] == CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST)
        {
          int keydown = keyMap[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST];
          PressKey(keydown);
        }
      std::cout << "button down" << std::endl;
      break;
    case CEC_OPCODE_USER_CONTROL_PRESSED:
    if (command.initiator == CECDEVICE_TV &&
      command.parameters.size ==1 &&
      command.parameters[0] == CEC_USER_CONTROL_CODE_STOP)
    {
    int keydown = keyMap[CEC_USER_CONTROL_CODE_STOP];
    PressKey(keydown);
    }
/*    case CEC_OPCODE_DECK_CONTROL:
    if (command.initiator == CECDEVICE_TV &&
      command.parameters.size ==1 &&
      command.parameters[0] == CEC_DECK_CONTROL_MODE_STOP)
    {
    int keydown = keyMap[CEC_USER_CONTROL_CODE_STOP];
    PressKey(keydown);
    }
    if (command.initiator == CECDEVICE_TV &&
       command.parameters.size ==1 &&
       command.parameters[0] == CEC_DECK_CONTROL_MODE_SKIP_FORWARD_WIND)
    {
      int keydown = keyMap[CEC_USER_CONTROL_CODE_PLAY];
      PressKey(keydown);
    }
    if (command.initiator == CECDEVICE_TV &&
      command.parameters.size ==1 &&
      command.parameters[0] == CEC_DECK_CONTROL_MODE_SKIP_REVERSE_REWIND)
    {
    int keydown = keyMap[CEC_USER_CONTROL_CODE_PAUSE];
    PressKey(keydown);
    }
    break;
*/
    default:
    std::cout << "CEC Command End: Nothing Special" << std::endl;
    break;
  }
}

int PressKey(const int keydown)
{
  std::cout << "Pressing key" <<std::endl;
 emit(fd, EV_KEY, keydown, 1);
   emit(fd, EV_SYN, SYN_REPORT, 0);
   emit(fd, EV_KEY, keydown, 0);
   emit(fd, EV_SYN, SYN_REPORT, 0);

  return 0;
}

void emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}


int CecCommandHandler(void *UNUSED(cbParam), const cec_command* UNUSED(command))
{
  return 0;
}
void CecAlert(void *UNUSED(cbParam), const libcec_alert type, const libcec_parameter UNUSED(param))
{
  switch (type)
  {
  case CEC_ALERT_CONNECTION_LOST:
	std::cout<<"Connection lost"<<std::endl;
    break;
  default:
    break;
  }
}





















void sighandler(int iSignal)
{
 std::cout<<std::endl<< "signal caught:" << iSignal << " - exiting" <<std::endl;
  g_bExit = 1;
}

int main (int argc, char *argv[])
{
  if (signal(SIGINT, sighandler) == SIG_ERR)
  {
    std::cout<<std::endl<<"can't register sighandler"<<std::endl;
    return -1;
  }
  //Set up all the keypress events
  populateKeyMapDefault();
  
  g_config.Clear();
  g_callbacks.Clear();
  snprintf(g_config.strDeviceName, LIBCEC_OSD_NAME_SIZE, "MusicPi");
  g_config.clientVersion      = LIBCEC_VERSION_CURRENT;
  g_config.baseDevice         = CECDEVICE_AUDIOSYSTEM;
  g_config.bActivateSource    = 1;
  g_callbacks.keyPress        = &CecKeyPress;
  g_callbacks.commandReceived = &CecCommand;
  g_callbacks.alert           = &CecAlert;
  g_callbacks.commandHandler  = &CecCommandHandler;
  g_config.callbacks          = &g_callbacks;
  g_strPort                   ="";
  g_cecLogLevel               = g_cecDefaultLogLevel;
  g_config.deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

  g_parser = LibCecInitialise(&g_config);
  if (!g_parser)
  {
    std::cout << "Cannot load libcec.so" << std::endl;
    if (g_parser)
      UnloadLibCec(g_parser);

    return 1;
  }

  // init video on targets that need this
  g_parser->InitVideoStandalone();

  // Set up fake keyboard
  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
   /*
    * The ioctls below will enable the device that is about to be
    * created, to pass key events, in this case the space key.
    */
   for (int ii=0;ii<keynumber;ii++)
   {
     ioctl(fd, UI_SET_EVBIT, EV_KEY);
     ioctl(fd, UI_SET_KEYBIT, keyList[ii]);
   }
   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x1234; /* sample vendor */
   usetup.id.product = 0x5678; /* sample product */
   strcpy(usetup.name, "Example device");

   ioctl(fd, UI_DEV_SETUP, &usetup);
   ioctl(fd, UI_DEV_CREATE);

  //keydev = libevdev_new();
  //libevdev_set_name(keydev, "fake keyboard device");

  //libevdev_enable_event_type(dev, EV_KEY);
  //libevdev_enable_event_code(dev, EV_KEY, KEY_A, NULL);

  //opendev = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
  //if (opendev != 0)
  //{
  //  std::cout << "trouble making fake keyboard" << std::endl;
  //  return opendev;
  //}

  if (g_strPort.empty())
  {
    std::cout << "no serial port given. trying autodetect: ";
    cec_adapter_descriptor devices[10];
    uint8_t iDevicesFound = g_parser->DetectAdapters(devices, 10, NULL, true);
    if (iDevicesFound <= 0)
	{
      std::cout << "autodetect ";
      std::cout << "FAILED" << std::endl;
      UnloadLibCec(g_parser);
      return 1;
    }
    else
    { 
      std::cout << std::endl << " path:     " << devices[0].strComPath << std::endl <<
            " com port: " << devices[0].strComName << std::endl << std::endl;
      g_strPort = devices[0].strComName;
    }
  }

  std::cout<<"opening a connection to the CEC adapter..."<<std::endl;
  if (!g_parser->Open(g_strPort.c_str()))
  {
    std::cout<< "unable to open the device on port" << g_strPort << std::endl;
    UnloadLibCec(g_parser);
    return 1;
  } 
  std::cout << std::endl << "waiting for input" << std::endl;

  pause();

  g_parser->Close();
  UnloadLibCec(g_parser);
  //libevdev_uinput_destroy(uidev);

  if (g_logOutput.is_open())
    g_logOutput.close();

  return 0;
}
