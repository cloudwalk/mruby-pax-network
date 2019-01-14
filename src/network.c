#include "mruby.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>

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

static int
get_mac_addr(char *ifname, char *mac)
{
  int fd, rtn;
  struct ifreq ifr;

  if( !ifname || !mac ) return -1;

  fd = socket(AF_INET, SOCK_DGRAM, 0 );
  if ( fd < 0 ) return -1;

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, (const char *)ifname, IFNAMSIZ - 1);

  if ((rtn = ioctl(fd, SIOCGIFHWADDR, &ifr)) == 0)
    memcpy(mac, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);

  close(fd);

  return rtn;
}

static mrb_value
mrb_network__ping(mrb_state *mrb, mrb_value klass)
{
  mrb_value ip;
  mrb_int timeout, ret;
  char sIp[16]={0x00};

  mrb_get_args(mrb, "Si", &ip, &timeout);

  strncpy((char *)&sIp, RSTRING_PTR(ip), RSTRING_LEN(ip));

  ret = OsNetPing(sIp, (int)timeout);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_wifi_dhcp_client_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int net;
  mrb_get_args(mrb, "i", &net);

  return mrb_fixnum_value(OsNetStartDhcp(net));
}

static mrb_value
mrb_wifi_dhcp_client_check(mrb_state *mrb, mrb_value klass)
{
  mrb_int net;
  mrb_get_args(mrb, "i", &net);

  return mrb_fixnum_value(OsNetCheckDhcp(net));
}

static mrb_value
mrb_network_send_recv_timeout(mrb_state *mrb, mrb_value klass)
{
  struct timeval tTimeout_recv, tTimeout_send;
  mrb_int sockfd, timeout_recv=0, timeout_send=0;

  mrb_get_args(mrb, "iii", &sockfd, &timeout_send, &timeout_recv);

  tTimeout_recv.tv_sec = timeout_recv;
  tTimeout_recv.tv_usec = 0;

  tTimeout_send.tv_sec = timeout_send;
  tTimeout_send.tv_usec = 0;

  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tTimeout_recv, sizeof(tTimeout_recv)) < 0)
    return mrb_false_value();

  if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tTimeout_send, sizeof(tTimeout_send)) < 0)
    return mrb_false_value();

  return mrb_true_value();
}

static mrb_value
mrb_network__mac_address(mrb_state *mrb, mrb_value klass)
{
  mrb_value ifname;
  char mac[8];

  mrb_get_args(mrb, "S", &ifname);

  if (get_mac_addr(RSTRING_PTR(ifname), mac) >= 0)
    return mrb_str_new(mrb, mac, 6);

  return mrb_nil_value();
}

void
mrb_init_network(mrb_state* mrb)
{
  struct RClass *network;

  network = mrb_define_class(mrb, "Network", mrb->object_class);

  mrb_define_class_method(mrb, network, "_ping", mrb_network__ping, MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb, network, "_dhcp_client_start", mrb_wifi_dhcp_client_start, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, network, "_dhcp_client_check", mrb_wifi_dhcp_client_check, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, network, "set_socket_timeout", mrb_network_send_recv_timeout, MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb, network, "_mac_address", mrb_network__mac_address, MRB_ARGS_REQ(1));
}

