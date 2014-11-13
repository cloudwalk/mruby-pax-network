/*
** network.c - Network module
**
** Network.init(GPRS, apn: claro.com.br, user: claro.com.br, pass: claro.com.br)
**   WlSwitchPower(1);
**   WlInit(NULL);
** Network.power true/false
**   WlSwitchPower(1);
** Network.connect(block = true/false)
**   WlPppLogin(sAPN,sUser,sPwd,0xff,0,0);
** Network.connected?
**   WlPppCheck();
** Network.disconnect
**   WlPppLogout();
**   WlSwitchPower(0);
**
** Network.apn, user, pass, type
**
** See Copyright Notice in mruby.h
*/

#include "mruby.h"
#include <stddef.h>
#include <string.h>

#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#if MRUBY_RELEASE_NO < 10000
  #include "error.h"
#else
  #include "mruby/error.h"
#endif

#include "osal.h"

static mrb_value
mrb_wifi_start(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(OsWifiOpen());
}

//TODO Scalone: Check if Power is necessary before init
static mrb_value
mrb_wifi_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_fixnum_value(OsWifiSwitchPower(state));
}

//TODO Scalone: Support blocking and non blocking connection with timeout modification
static mrb_value
mrb_wifi_connect(mrb_state *mrb, mrb_value klass)
{
  mrb_value password, essid, bssid, channel, mode, authentication, cipher;
  const char *sPassword, *sEssid, *sBssid;

  ST_WifiApSet wifiSet;
  WPA_PSK_KEY psk;

  /*memset(psk, 0, sizeof(psk));*/
  /*memset(wifiSet, 0, sizeof(wifiSet));*/

  /*int AuthMode; [> Authentication modes <]*/
  authentication = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@authentication"));
  wifiSet.AuthMode = mrb_fixnum(authentication);

  password = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@password"));
  sPassword  = mrb_str_to_cstr(mrb, password);

  /*WPA_PSK_KEY PskKey; [> For wpa,wpa2-psk authentication <]*/
  if (wifiSet.AuthMode == AUTH_NONE_WEP && wifiSet.AuthMode == AUTH_NONE_WEP_SHARED) {
    strcpy((char *)&psk.Key, sPassword);
    psk.KeyLen = strlen(sPassword);
    wifiSet.KeyUnion.PskKey = psk;
  }

  /*char Essid[33]; [> AP name, it can support 32 bytes at most, and ending with '\0' <]*/
  essid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@essid"));
  sEssid  = mrb_str_to_cstr(mrb, essid);
  strcpy((char *)&wifiSet.Essid, sEssid);

  /*char Bssid[20]; [> MAC address, if there is no any APs with the same ESSID, Bssid can be "\0"<]*/
  bssid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@bssid"));
  sBssid = mrb_str_to_cstr(mrb, bssid);
  strcpy((char *)&wifiSet.Bssid, sBssid);

  /*int Channel;  [> Channel, 0:Auto set <]*/
  channel = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@channel"));
  wifiSet.Channel = mrb_fixnum(channel);

  /*int Mode; [> Connection mode, 0:Station; 1:IBSS <]*/
  mode = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@mode"));
  wifiSet.Mode = mrb_fixnum(mode);

  cipher = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@cipher"));
  wifiSet.SecMode = mrb_fixnum(cipher);

  /*WlPppLogin(sAPN, sUser, sPass, auth, timeout, keep_alive);*/
  /*OsWlLogin(sAPN, sUser, sPass, auth, timeout, keep_alive, NULL);*/

  return mrb_fixnum_value(OsWifiConnect(&wifiSet, 1000));
}

//   0 -> Sucess
//   1 -> In Progress
// < 0 -> Fail
static mrb_value
mrb_wifi_connected_m(mrb_state *mrb, mrb_value klass)
{
  char sEssid[32+1] = "                                \0";
  char sBssid[19+1] = "                   \0";
  mrb_int iRssi, ret;

  ret = OsWifiCheck(&sEssid, &sBssid, &iRssi);


  if (ret == RET_OK) {
    mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@essid"), mrb_str_new_cstr(mrb, sEssid));
    mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@bssid"), mrb_str_new_cstr(mrb, sBssid));
    mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@rssi"), mrb_fixnum_value(iRssi));
  }

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_wifi_disconnect(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(OsWifiDisconnect());
}

void
mrb_init_wifi(mrb_state* mrb)
{
  struct RClass *network, *wifi;

  network = mrb_define_class(mrb, "Network", mrb->object_class);
  network = mrb_class_get(mrb, "Network");
  wifi    = mrb_define_class(mrb, "Wifi", network);

  mrb_define_class_method(mrb, wifi, "start", mrb_wifi_start, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "power", mrb_wifi_power, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, wifi, "connect", mrb_wifi_connect, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, wifi, "connected?", mrb_wifi_connected_m, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "disconnect", mrb_wifi_disconnect, MRB_ARGS_NONE());
}

