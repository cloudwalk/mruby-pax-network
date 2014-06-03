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
** Network.ping
**   NetPing();
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

#ifdef PAX
  #include "posapi.h"
#endif

static mrb_value
mrb_network_init(mrb_state *mrb, mrb_value klass)
{
  //mrb_value media, apn, user, pass, ary;
  mrb_value media, options, value;
  //const char *sAPN, *sUser, *sPwd;

  mrb_get_args(mrb, "o|o", &media, &options);

  mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@media"), media);

  if (mrb_hash_p(options)) {
    value = mrb_hash_get(mrb, options, mrb_symbol_value(mrb_intern_cstr(mrb, "apn")));
    //sAPN = mrb_str_to_cstr(mrb, value);
    mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@apn"), value);

    value = mrb_hash_get(mrb, options, mrb_symbol_value(mrb_intern_cstr(mrb, "user")));
    //sUser = mrb_str_to_cstr(mrb, value);
    mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@user"), value);

    value = mrb_hash_get(mrb, options, mrb_symbol_value(mrb_intern_cstr(mrb, "pass")));
    //sPwd = mrb_str_to_cstr(mrb, value);
    mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@pass"), value);

    // TODO Scalone: Implement auth
    //value = mrb_hash_get(mrb, options, mrb_symbol_value(mrb_intern_cstr(mrb, "apn")));
    //sAPN = mrb_str_to_cstr(mrb, value);
    //mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "apn"), value);

    // TODO Scalone: Implement Keep Alive Time
    //value = mrb_hash_get(mrb, options, mrb_symbol_value(mrb_intern_cstr(mrb, "apn")));
    //sAPN = mrb_str_to_cstr(mrb, value);
    //mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "apn"), value);
  }

#ifdef PAX
  // NULL could be a PIN to acess the SIM Card
  return mrb_fixnum_value(WlInit(NULL));
#else
  return mrb_fixnum_value(0);
#endif
}

static mrb_value
mrb_network_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int on;
  mrb_get_args(mrb, "i", &on);
#ifdef PAX
  WlSwitchPower(on);
#endif
  return mrb_true_value();
}

//TODO Scalone: Support blocking and non blocking connection with timeout modification
static mrb_value
mrb_network_connect(mrb_state *mrb, mrb_value klass)
{
  mrb_value apn, user, pass;
  const char *sAPN, *sUser, *sPass;
  int keep_alive=0, timeout=0, auth=0xff;

  apn = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@apn"));
  sAPN  = mrb_str_to_cstr(mrb, apn);

  user = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@user"));
  sUser = mrb_str_to_cstr(mrb, user);

  pass = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@pass"));
  sPass = mrb_str_to_cstr(mrb, pass);

#ifdef PAX
  WlPppLogin(sAPN, sUser, sPass, auth, keep_alive, timeout);
#endif

  return mrb_fixnum_value(0);
}

//   0 -> Sucess
//   1 -> In Progress
// < 0 -> Fail
static mrb_value
mrb_network_connected_m(mrb_state *mrb, mrb_value klass)
{
#ifdef PAX
  return mrb_fixnum_value(WlPppCheck());
#else
  return mrb_fixnum_value(-1);
#endif
}

static mrb_value
mrb_network_disconnect(mrb_state *mrb, mrb_value klass)
{
#ifdef PAX
  WlPppLogout();
#endif
  return mrb_true_value();
}

static mrb_value
mrb_network_ping(mrb_state *mrb, mrb_value klass)
{
  mrb_value host, ip;
  mrb_int timeout;

  mrb_get_args(mrb, "Si", &host, &timeout);

  ip = mrb_funcall(mrb, klass, "getaddress", 1, host);

#ifdef PAX
  return mrb_fixnum_value(NetPing(RSTRING_PTR(ip), timeout, 56));
#else
  return mrb_fixnum_value(-1);
#endif
}

void
mrb_init_network(mrb_state* mrb)
{
  struct RClass *network;

  network = mrb_define_class(mrb, "Network", mrb->object_class);
  mrb_define_class_method(mrb, network, "init", mrb_network_init, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, network, "power", mrb_network_power, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, network, "connect", mrb_network_connect, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, network, "connected?", mrb_network_connected_m, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, network, "disconnect", mrb_network_disconnect, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, network, "ping", mrb_network_ping, MRB_ARGS_REQ(2));
}

