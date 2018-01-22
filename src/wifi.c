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
  mrb_int ret=RET_OK;

  OsWlSwitchPower(0);
  OsWlUnLock();

  ret = OsWifiOpen();
  if (ret == RET_OK) OsNetSetRoute(NET_LINK_WIFI);

  return mrb_fixnum_value(ret);
}

/*TODO Scalone: Check if Power is necessary before init*/
static mrb_value
mrb_wifi_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int state=1, ret=0;
  mrb_get_args(mrb, "i", &state);

  ret = OsWifiSwitchPower(state);
  if (state == 0) OsWifiClose();

  return mrb_fixnum_value(ret);
}

static int
isWEP(int AuthMode)
{
  if (AuthMode == AUTH_NONE_WEP || AuthMode == AUTH_NONE_WEP_SHARED)
    return 1;
  else
    return 0;
}

static int
isPSK(int AuthMode)
{
  if (AuthMode == AUTH_WPA_PSK || AuthMode == AUTH_WPA_WPA2_PSK || AuthMode == AUTH_WPA2_PSK)
    return 1;
  else
    return 0;
}

/*TODO Scalone: Support blocking and non blocking connection with timeout modification*/
/*TODO Scalone: Check if parameters is string*/
/*TODO Scalone: Add more authentications mode*/
static mrb_value
mrb_wifi_connect(mrb_state *mrb, mrb_value klass)
{
  mrb_value password, essid, bssid, channel, mode, authentication, cipher;
  const char *sPassword, *sEssid, *sBssid, *sChannel, *sCipher, *sMode, *sAuthentication;

  ST_WifiApSet wifiSet;
  WPA_PSK_KEY psk;
  WEP_SEC_KEY wep;
  WPA_EAP_KEY eap;

  memset(&psk, 0, sizeof(psk));
  memset(&eap, 0, sizeof(eap));
  memset(&wep, 0, sizeof(wep));
  memset(&wifiSet, 0, sizeof(wifiSet));

  /*char Essid[33]; [> AP name, it can support 32 bytes at most, and ending with '\0' <]*/
  essid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@essid"));
  sEssid = mrb_str_to_cstr(mrb, essid);
  strcpy((char *)&wifiSet.Essid, sEssid);
  /*DEBUG*/
  /*display("essid %s", wifiSet.Essid);*/

  /*char Bssid[20]; [> MAC address, if there is no any APs with the same ESSID, Bssid can be "\0"<]*/
  bssid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@bssid"));
  sBssid = mrb_str_to_cstr(mrb, bssid);
  strcpy((char *)&wifiSet.Bssid, sBssid);

  /*int Channel;  [> Channel, 0:Auto set <]*/
  channel = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@channel"));
  sChannel = mrb_str_to_cstr(mrb, channel);
  wifiSet.Channel = atoi(sChannel);
  /*DEBUG*/
  /*display("channel %d", atoi(sChannel));*/

  /*int Mode; [> Connection mode, 0:Station; 1:IBSS <]*/
  mode = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@mode"));
  sMode = mrb_str_to_cstr(mrb, mode);
  wifiSet.Mode = atoi(sMode);
  /*DEBUG*/
  /*display("mode %d", atoi(sMode));*/

  /*int AuthMode; [> Authentication modes <]*/
  authentication = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@authentication"));
  sAuthentication = mrb_str_to_cstr(mrb, authentication);
  wifiSet.AuthMode = atoi(sAuthentication);
  /*DEBUG*/
  /*display("authmode %d", atoi(sAuthentication));*/

  password = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@password"));
  sPassword = mrb_str_to_cstr(mrb, password);

  if (isPSK(wifiSet.AuthMode)) {
    /*WPA_PSK_KEY PskKey; [> For wpa,wpa2-psk authentication <]*/
    strcpy((char *)&psk.Key, sPassword);
    psk.KeyLen = strlen(sPassword);
    wifiSet.KeyUnion.PskKey = psk;
  } else if (isWEP(wifiSet.AuthMode)) {
    strcpy((char *)&wep.Key, sPassword);
    wep.KeyLen = strlen(sPassword);
    wifiSet.KeyUnion.WepKey = wep;
  }
  /*DEBUG*/
  /*display("len %d key %s", wifiSet.KeyUnion.PskKey.KeyLen, wifiSet.KeyUnion.PskKey.Key);*/

  cipher = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@cipher"));
  sCipher = mrb_str_to_cstr(mrb, cipher);
  wifiSet.SecMode = (int)sCipher[0];
  /*DEBUG*/
  /*display("cipher %d", (int)sCipher[0]);*/

  /*TODO Scalone timeout must come from DaFunk.*/
  return mrb_fixnum_value(OsWifiConnect(&wifiSet, 0));
}

/*0   -> Success*/
/*1   -> In Progress*/
/*< 0 -> Fail*/
static mrb_value
mrb_wifi_connected_m(mrb_state *mrb, mrb_value klass)
{
  char sEssid[32+1] = "                                \0";
  char sBssid[19+1] = "                   \0";
  mrb_int iRssi=0, ret;

  ret = OsWifiCheck((char *)&sEssid, (char *)&sBssid, &iRssi);

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

static mrb_value
mrb_wifi__scan(mrb_state *mrb, mrb_value klass)
{
  ST_WifiApInfo * aps;
  mrb_int ret = 0, i = 0;

  ret = OsWifiScan(&aps);

  if (ret < 0) return mrb_fixnum_value(ret);

  for (i=0;i < ret;i++) {
    mrb_funcall(mrb, klass, "ap", 7,
        mrb_str_new_cstr(mrb, aps[i].Essid),
        mrb_str_new_cstr(mrb, aps[i].Bssid),
        mrb_fixnum_value(aps[i].Channel),
        mrb_fixnum_value(aps[i].Mode),
        mrb_fixnum_value(aps[i].Rssi),
        mrb_fixnum_value(aps[i].AuthMode),
        mrb_fixnum_value(aps[i].SecMode)
        );
  }
  return mrb_fixnum_value(RET_OK);
}

void
mrb_init_wifi(mrb_state *mrb)
{
  struct RClass *network, *wifi;

  network = mrb_class_get(mrb, "Network");
  wifi    = mrb_define_class_under(mrb, network, "Wifi", mrb->object_class);

  mrb_define_class_method(mrb, wifi, "start", mrb_wifi_start, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "power", mrb_wifi_power, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, wifi, "connect", mrb_wifi_connect, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, wifi, "connected?", mrb_wifi_connected_m, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "disconnect", mrb_wifi_disconnect, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "_scan", mrb_wifi__scan, MRB_ARGS_NONE());
}

