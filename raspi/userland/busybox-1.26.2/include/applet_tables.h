/* This is a generated file, don't edit */

#define NUM_APPLETS 23
#define KNOWN_APPNAME_OFFSETS 0

const char applet_names[] ALIGN1 = ""
"ash" "\0"
"bash" "\0"
"cat" "\0"
"echo" "\0"
"httpd" "\0"
"ifconfig" "\0"
"init" "\0"
"kill" "\0"
"ls" "\0"
"mkdir" "\0"
"mkfifo" "\0"
"mount" "\0"
"nc" "\0"
"ping" "\0"
"ps" "\0"
"pwd" "\0"
"rdate" "\0"
"route" "\0"
"sh" "\0"
"sleep" "\0"
"uname" "\0"
"uptime" "\0"
"vi" "\0"
;

#define APPLET_NO_ash 0
#define APPLET_NO_bash 1
#define APPLET_NO_cat 2
#define APPLET_NO_echo 3
#define APPLET_NO_httpd 4
#define APPLET_NO_ifconfig 5
#define APPLET_NO_init 6
#define APPLET_NO_kill 7
#define APPLET_NO_ls 8
#define APPLET_NO_mkdir 9
#define APPLET_NO_mkfifo 10
#define APPLET_NO_mount 11
#define APPLET_NO_nc 12
#define APPLET_NO_ping 13
#define APPLET_NO_ps 14
#define APPLET_NO_pwd 15
#define APPLET_NO_rdate 16
#define APPLET_NO_route 17
#define APPLET_NO_sh 18
#define APPLET_NO_sleep 19
#define APPLET_NO_uname 20
#define APPLET_NO_uptime 21
#define APPLET_NO_vi 22

#ifndef SKIP_applet_main
int (*const applet_main[])(int argc, char **argv) = {
ash_main,
ash_main,
cat_main,
echo_main,
httpd_main,
ifconfig_main,
init_main,
kill_main,
ls_main,
mkdir_main,
mkfifo_main,
mount_main,
nc_main,
ping_main,
ps_main,
pwd_main,
rdate_main,
route_main,
ash_main,
sleep_main,
uname_main,
uptime_main,
vi_main,
};
#endif

