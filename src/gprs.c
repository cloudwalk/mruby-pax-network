/*
** network.c - Network module
**
** Network.init(GPRS, apn: claro.com.br, user: claro.com.br, password: claro.com.br)
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
** Network.apn, user, password, type
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
mrb_gprs_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=RET_OK;

  ret = OsWlLock();
  if (ret == RET_OK) ret = OsWlInit(NULL);
  if (ret == RET_OK) OsNetSetRoute(NET_LINK_WL);

  return mrb_fixnum_value(ret);
}

//TODO Scalone: Check if Power is necessary before init
static mrb_value
mrb_gprs_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int on;
  mrb_get_args(mrb, "i", &on);

  OsWlSwitchPower(on);

  return mrb_true_value();
}

//TODO Scalone: Support blocking and non blocking connection with timeout modification
static mrb_value
mrb_gprs_connect(mrb_state *mrb, mrb_value klass)
{
  mrb_value apn, user, password;
  const char *sAPN, *sUser, *sPass;
  int keep_alive=0, timeout=0, auth=0xff;

  apn = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@apn"));
  sAPN  = mrb_str_to_cstr(mrb, apn);

  user = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@user"));
  sUser = mrb_str_to_cstr(mrb, user);

  password = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@password"));
  sPass = mrb_str_to_cstr(mrb, password);

  OsWlLogin(sAPN, sUser, sPass, auth, timeout, keep_alive, NULL);

  return mrb_fixnum_value(0);
}

//   0 -> Sucess
//   1 -> In Progress
// < 0 -> Fail
static mrb_value
mrb_gprs_connected_m(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(OsWlCheck());
}

static mrb_value
mrb_gprs_disconnect(mrb_state *mrb, mrb_value klass)
{
  OsWlLogout();
  return mrb_true_value();
}

void
mrb_init_gprs(mrb_state* mrb)
{
  struct RClass *network, *gprs;

  network = mrb_class_get(mrb, "Network");
  gprs    = mrb_define_class(mrb, "Gprs", network);

  mrb_define_class_method(mrb, gprs, "start", mrb_gprs_start, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, gprs, "power", mrb_gprs_power, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, gprs, "connect", mrb_gprs_connect, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, gprs, "connected?", mrb_gprs_connected_m, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, gprs, "disconnect", mrb_gprs_disconnect, MRB_ARGS_NONE());
}

