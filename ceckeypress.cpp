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

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <linux/uinput.h>
//#include "input-event-codes.h"
#if defined(HAVE_CURSES_API)
  #include "curses/CursesControl.h"
#endif

using namespace CEC;
using namespace STD;
#include "cecloader.h"

static void PrintToStdOut(const char *strFormat, ...);

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


map<int, int>     keyMap;
int PressKey(const string json);

%int opendev;
%struct libevdev *keydev;
%struct libevdev_uinput *uidev;

void populateKeyMapDefault()
{
  cout << "Building Map" <<endl;
  keyMap[CEC_USER_CONTROL_CODE_PAGE_UP] = KEY_PAGEUP;
  keyMap[CEC_USER_CONTROL_CODE_PAGE_DOWN] = KEY_PAGEDOWN;
  keyMap[CEC_USER_CONTROL_CODE_CHANNEL_UP] = KEY_HOME;
  keyMap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN] = KEY_END;
  keyMap[CEC_USER_CONTROL_CODE_LEFT] = KEY_LEFT;
  keyMap[CEC_USER_CONTROL_CODE_RIGHT] = KEY_RIGHT;
  keyMap[CEC_USER_CONTROL_CODE_DOWN] = KEY_DOWN;
  keyMap[CEC_USER_CONTROL_CODE_UP] = KEY_UP;
  keyMap[CEC_USER_CONTROL_CODE_SELECT] = KEY_SPACE;
  keyMap[CEC_USER_CONTROL_CODE_EXIT] = KEY_RETURN;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER0] = KEY_0;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER1] = KEY_1;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER2] = KEY_2;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER3] = KEY_3;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER4] = KEY_4;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER5] = KEY_5;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER6] = KEY_6;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER7] = KEY_7;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER8] = KEY_8;
  keyMap[CEC_USER_CONTROL_CODE_NUMBER9] = KEY_9;
  keyMap[CEC_USER_CONTROL_CODE_RECORD] = KEY_R;
  keyMap[CEC_USER_CONTROL_CODE_PLAY] = KEY_P;
  keyMap[CEC_USER_CONTROL_CODE_FAST_FORWARD] = KEY_F;
  keyMap[CEC_USER_CONTROL_CODE_REWIND] = KEY_B;
  keyMap[CEC_USER_CONTROL_CODE_STOP] = KEY_Z;
  keyMap[CEC_USER_CONTROL_CODE_PAUSE] = KEY_X;
  keyMap[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE] = KEY_G;
  keyMap[CEC_USER_CONTROL_CODE_F1_BLUE] = KEY_F1;
  keyMap[CEC_USER_CONTROL_CODE_F2_RED] = KEY_F2;
  keyMap[CEC_USER_CONTROL_CODE_F3_GREEN] = KEY_F3;
  keyMap[CEC_USER_CONTROL_CODE_F4_YELLOW] = KEY_F4;
  keyMap[CEC_USER_CONTROL_CODE_AN_RETURN] = KEY_ENTER;
  keyMap[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL] = KEY_K;
  keyMap[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST] = KEY_L;
}





static void PrintToStdOut(const char *strFormat, ...)
{
  std::string strLog;

  va_list argList;
  va_start(argList, strFormat);
  strLog = StringUtils::FormatV(strFormat, argList);
  va_end(argList);

  CLockObject lock(g_outputMutex);
  std::cout << strLog << std::endl;
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

static cec_logical_address GetAddressFromInput(std::string& arguments)
{
  std::string strDev;
  if (GetWord(arguments, strDev))
  {
      unsigned long iDev = strtoul(strDev.c_str(), NULL, 16);
      if ((iDev >= CECDEVICE_TV) && (iDev <= CECDEVICE_BROADCAST))
        return (cec_logical_address)iDev;
  }
  return CECDEVICE_UNKNOWN;
}

void CecLogMessage(void *UNUSED(cbParam), const cec_log_message* message)
{
  if ((message->level & g_cecLogLevel) == message->level)
  {
    std::string strLevel;
    switch (message->level)
    {
    case CEC_LOG_ERROR:
      strLevel = "ERROR:   ";
      break;
    case CEC_LOG_WARNING:
      strLevel = "WARNING: ";
      break;
    case CEC_LOG_NOTICE:
      strLevel = "NOTICE:  ";
      break;
    case CEC_LOG_TRAFFIC:
      strLevel = "TRAFFIC: ";
      break;
    case CEC_LOG_DEBUG:
      strLevel = "DEBUG:   ";
      break;
    default:
      break;
    }

    std::string strFullLog;
    strFullLog = StringUtils::Format("%s[%16lld]\t%s", strLevel.c_str(), message->time, message->message);
    PrintToStdOut(strFullLog.c_str());

    if (g_logOutput.is_open())
    {
      if (g_bShortLog)
        g_logOutput << message->message << std::endl;
      else
        g_logOutput << strFullLog.c_str() << std::endl;
    }
  }
}

void CecKeyPress(void *UNUSED(cbParam), const cec_keypress* keyptr)
{
  {
  cec_keypress key = *keyptr;
  cout << "CeCKeyPress" << endl;
  if (key.duration == 0)
  {
    cout << "key " <<  key.keycode << endl;
    string json = "unmapped";
    if (keyMap.find(key.keycode) != keyMap.end())
    {
      json = keyMap[key.keycode];
      PressKey(json);
    }
   
    if (logEvents)
      cout << "remote key code: " << key.keycode << ", keyboard output: " << json << endl;
  }    
}

void CecCommand(void *UNUSED(cbParam), const cec_command*  commandptr)
{
  cec_command command = *commandptr;
  cout << "CeC Command" << endl;
  cout << "commandissued " << command.opcode << "versus" << CEC_OPCODE_DECK_CONTROL << " option " << int(command.parameters[0]) << int(CEC_USER_CONTROL_CODE_AN_RETURN) <<endl;
  switch (command.opcode)
  {
    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
      if (command.parameters.size ==1 &&
           command.initiator == CECDEVICE_TV)
        if (command.parameters[0] == CEC_USER_CONTROL_CODE_AN_RETURN)
        {
          string json = keyMap[CEC_USER_CONTROL_CODE_AN_RETURN];
          PressKey(json);
        }
        if (command.parameters[0] == CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST)
        {
          string json = keyMap[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST];
          PressKey(json);
        }
      cout << "button down" << endl;
      break;
    case CEC_OPCODE_USER_CONTROL_PRESSED:
    if (command.initiator == CECDEVICE_TV &&
      command.parameters.size ==1 &&
      command.parameters[0] == CEC_USER_CONTROL_CODE_STOP)
    {
    string json = keyMap[CEC_USER_CONTROL_CODE_STOP];
    PressKey(json);
    }
/*    case CEC_OPCODE_DECK_CONTROL:
    if (command.initiator == CECDEVICE_TV &&
      command.parameters.size ==1 &&
      command.parameters[0] == CEC_DECK_CONTROL_MODE_STOP)
    {
    string json = keyMap[CEC_USER_CONTROL_CODE_STOP];
    PressKey(json);
    }
    if (command.initiator == CECDEVICE_TV &&
       command.parameters.size ==1 &&
       command.parameters[0] == CEC_DECK_CONTROL_MODE_SKIP_FORWARD_WIND)
    {
      string json = keyMap[CEC_USER_CONTROL_CODE_PLAY];
      PressKey(json);
    }
    if (command.initiator == CECDEVICE_TV &&
      command.parameters.size ==1 &&
      command.parameters[0] == CEC_DECK_CONTROL_MODE_SKIP_REVERSE_REWIND)
    {
    string json = keyMap[CEC_USER_CONTROL_CODE_PAUSE];
    PressKey(json);
    }
    break;
*/
    default:
    cout << "CEC Command End: Nothing Special" << endl;
    break;
  }
}

int PressKey(const string json)
{
  cout << "Pressing key" <<endl;
  //libevdev_uinput_write_event(uidev, EV_KEY, KEY_A, 1);
  //libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
  //libevdev_uinput_write_event(uidev, EV_KEY, KEY_A, 0);
  //libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
  return 0;
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
    if (!CReconnect::Get().IsRunning())
    {
      PrintToStdOut("Connection lost - trying to reconnect\n");
      CReconnect::Get().CreateThread(false);
    }
    break;
  default:
    break;
  }
}





















void sighandler(int iSignal)
{
  PrintToStdOut("signal caught: %d - exiting", iSignal);
  g_bExit = 1;
}

int main (int argc, char *argv[])
{
  if (signal(SIGINT, sighandler) == SIG_ERR)
  {
    PrintToStdOut("can't register sighandler");
    return -1;
  }
  //Set up all the keypress events
  populateKeyMapDefault();

  g_config.Clear();
  g_callbacks.Clear();
  snprintf(g_config.strDeviceName, LIBCEC_OSD_NAME_SIZE, "MusicPi");
  g_config.clientVersion      = LIBCEC_VERSION_CURRENT;
  g_config.bActivateSource    = 0;
  g_callbacks.logMessage      = &CecLogMessage;
  g_callbacks.keyPress        = &CecKeyPress;
  g_callbacks.commandReceived = &CecCommand;
  g_callbacks.alert           = &CecAlert;
  g_callbacks.commandHandler  = &CecCommandHandler;
  g_config.callbacks          = &g_callbacks;
  g_strPort                   ="";
  g_cecLogLevel               = g_cecDefaultLogLevel;
  g_config.deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

  if (!ProcessCommandLineArguments(argc, argv))
    return 0;

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

  PrintToStdOut("opening a connection to the CEC adapter...");
  if (!g_parser->Open(g_strPort.c_str()))
  {
    PrintToStdOut("unable to open the device on port %s", g_strPort.c_str());
    UnloadLibCec(g_parser);
    return 1;
  } 
  PrintToStdOut("waiting for input");

  pause();

  g_parser->Close();
  UnloadLibCec(g_parser);
  libevdev_uinput_destroy(uidev);

  if (g_logOutput.is_open())
    g_logOutput.close();

  return 0;
