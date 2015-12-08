#include "mruby.h"

void mrb_init_network(mrb_state *mrb);
void mrb_init_gprs(mrb_state *mrb);
void mrb_init_wifi(mrb_state *mrb);
void mrb_init_ethernet(mrb_state *mrb);

#define DONE mrb_gc_arena_restore(mrb, 0)

void
mrb_mruby_pax_network_gem_init(mrb_state* mrb)
{
  mrb_init_network(mrb); DONE;
  mrb_init_gprs(mrb); DONE;
  mrb_init_wifi(mrb); DONE;
  mrb_init_ethernet(mrb); DONE;
}

void
mrb_mruby_pax_network_gem_final(mrb_state* mrb)
{
}