/*
** socket.c - TCPSocket module
**
** TCPSocket.getaddress(hostname)
**   DnsResolve(char *name, char *result, int len);
** TCPSocket.open/new(ip, port)
**   fd=NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
**   SockAddrSet(&server_addr, sIP, ucPort);
**   iNCRet=NetConnect(fd,&server_addr,sizeof(server_addr));
** TCPSocket#close
**   NetCloseSocket(fd);
** TCPSocket#closed?
**   ioctl
** TCPSocket#send
**   iNCRet=NetSend(fd,ucTmpSend,iSize,0);
** TCPSocket#recv
**   iNCRet=NetRecv(fd,ucTmpRcv+iTemp,10*1000,0);
** TCPSocket#write
**   ruby
** TCPSocket#read wait number of bytes
**   ruby
** TCPSocket#timeout=
**   Netioctl(fd, CMD_TO_SET, 55000);
**
** Network.apn, user, pass, type, timeout
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

static mrb_int
socket_fd(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum(mrb_iv_get(mrb, klass, mrb_intern_cstr(mrb, "@fd")));
}


static mrb_value
mrb_socket_getaddress(mrb_state *mrb, mrb_value klass)
{
  mrb_value host, ip;

  mrb_get_args(mrb, "S", &host);

  ip = mrb_str_buf_new(mrb, 20);

#ifdef PAX
  if (mrb_string_p(host))
    DnsResolve(RSTRING_PTR(host), RSTRING_PTR(ip), RSTRING_LEN(host));
  else
    mrb_raise(mrb, E_TYPE_ERROR, "host must be String");
#endif

  if (strcmp(RSTRING_PTR(ip), "") == 0)
    return host;
  else
    return ip;
}

static mrb_value
mrb_socket_open(mrb_state *mrb, mrb_value klass)
{
  int iNCRet=-1;
#ifdef PAX
  int fd=-1;
  mrb_value ip, remote_host, timeout, local_host, local_port;
  mrb_int remote_port;
  // TODO Scalone maybe alloc and store net_sockaddr
  struct net_sockaddr host_addr;
  const char *port = NULL;

  mrb_get_args(mrb, "Si|Si", &remote_host, &remote_port, &local_host, &local_port);

  if (! mrb_string_p(remote_host)) {
    mrb_raise(mrb, E_TYPE_ERROR, "remote_host must be String");
  }

  fd = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
  mrb_iv_set(mrb, klass, mrb_intern_cstr(mrb, "@fd"), mrb_fixnum_value(fd));

  ip = mrb_funcall(mrb, klass, "getaddress", 1, remote_host);

  if (fd >= 0){
    SockAddrSet(&host_addr, RSTRING_PTR(ip), remote_port);
    mrb_funcall(mrb, klass, "timeout=", 0);
    iNCRet = NetConnect(fd, &host_addr, sizeof(host_addr));
  }
#endif
  if (iNCRet < 0)
    mrb_funcall(mrb, klass, "close", 0);

  return klass;
}

static mrb_value
mrb_socket_ioctl(mrb_state *mrb, mrb_value klass)
{
  mrb_int fd, cmd, miliseconds;

  mrb_get_args(mrb, "ii", &cmd, &miliseconds);

  fd = socket_fd(mrb, klass);

#ifdef PAX
  return mrb_fixnum_value(Netioctl(fd, cmd, miliseconds));
#else
  return mrb_fixnum_value(-1);
#endif
}

static mrb_value
mrb_socket_close(mrb_state *mrb, mrb_value klass)
{
  mrb_int fd;

  fd = socket_fd(mrb, klass);
  mrb_iv_set(mrb, klass, mrb_intern_cstr(mrb, "@fd"), mrb_fixnum_value(-1));

#ifdef PAX
  NetCloseSocket(fd);
#endif
  return mrb_nil_value();
}

static mrb_value
mrb_socket_send(mrb_state *mrb, mrb_value klass)
{
  mrb_int fd, flags = 0;
  mrb_value mesg, dest;
  int bytes=-1;

  mrb_get_args(mrb, "Si|S", &mesg, &flags, &dest);

  fd = socket_fd(mrb, klass);

#ifdef PAX
  if (mrb_string_p(mesg)) {
    bytes = NetSend(fd, RSTRING_PTR(mesg), RSTRING_LEN(mesg), flags);
  } else
    mrb_raise(mrb, E_TYPE_ERROR, "mesg must be String");
#endif

  return mrb_fixnum_value(bytes);
}

static mrb_value
mrb_socket_recv(mrb_state *mrb, mrb_value klass)
{
  mrb_int fd, maxlen, flags = 0;
  mrb_value mesg;
  int bytes=-1;

  mrb_get_args(mrb, "i|i", &maxlen, &flags);

  fd = socket_fd(mrb, klass);

#ifdef PAX
  mesg  = mrb_str_buf_new(mrb, maxlen);
  bytes = NetRecv(fd, RSTRING_PTR(mesg), maxlen, flags);

  if (bytes > 0)
    mrb_str_resize(mrb, mesg, bytes);
#endif

  return mesg;
}

void
mrb_init_socket(mrb_state* mrb)
{
  struct RClass *socket;

  socket = mrb_define_class(mrb, "TCPSocket", mrb->object_class);
  mrb_define_class_method(mrb, socket, "getaddress", mrb_socket_getaddress, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, socket, "initialize", mrb_socket_open, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));
  mrb_define_method(mrb, socket, "ioctl", mrb_socket_ioctl, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, socket, "close", mrb_socket_close, MRB_ARGS_NONE());
  mrb_define_method(mrb, socket, "send", mrb_socket_send, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));
  mrb_define_method(mrb, socket, "recv", mrb_socket_recv, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
  //mrb_define_method(mrb, socket, "write", mrb_socket_send, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
  //mrb_define_method(mrb, socket, "read", mrb_socket_read, MRB_ARGS_REQ(1));
}
