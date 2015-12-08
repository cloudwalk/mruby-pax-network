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
mrb_ethernet_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=RET_OK;

  if (ret == RET_OK) OsNetSetRoute(NET_LINK_ETH);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_ethernet_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_fixnum_value(RET_OK);
}

static mrb_value
mrb_ethernet_connect(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(RET_OK);
}

static mrb_value
mrb_ethernet_connected_m(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(RET_OK);
}

static mrb_value
mrb_ethernet_disconnect(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(RET_OK);
}

void
mrb_init_ethernet(mrb_state *mrb)
{
  struct RClass *network, *ethernet;

  network  = mrb_class_get(mrb, "Network");
  ethernet = mrb_define_class_under(mrb, network, "Ethernet", mrb->object_class);

  mrb_define_class_method(mrb , ethernet , "_start"      , mrb_ethernet_start       , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , ethernet , "_power"      , mrb_ethernet_power       , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , ethernet , "_connect"    , mrb_ethernet_connect     , MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb , ethernet , "_connected?" , mrb_ethernet_connected_m , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , ethernet , "_disconnect" , mrb_ethernet_disconnect  , MRB_ARGS_NONE());
}

