#ifndef _NETCFG_H_
#define _NETCFG_H_
#include <sys/types.h>
#include <stdio.h>
#include <cdebconf/debconfclient.h>

#define ETC_DIR 	"/etc"
#define NETWORK_DIR 	"/etc/network"
#define DHCPCD_DIR 	"/etc/dhcpc"
#define INTERFACES_FILE "/etc/network/interfaces"
#define HOSTS_FILE      "/etc/hosts"
#define NETWORKS_FILE   "/etc/networks"
#define RESOLV_FILE     "/etc/resolv.conf"
#define DHCPCD_FILE     "/etc/dhcpc/config"
#define DHCLIENT_DIR	"/var/dhcp"

#define TEMPLATE_PREFIX	"debian-installer/netcfg/"

#ifndef _
#define _(x)  (x)
#endif

struct debconfclient *client;

struct interface_info
{
	char *interface;
	char *description;
	enum { PTP_NO, PTP_YES } pointopoint;
	enum { INET6_NO, INET6_YES, INET6_PART, INET6_ONLY } inet6;
};

struct interface_config
{
	char interface[10];

	const struct interface_info *info;

	int inet;
	int inet6;

	char domain[255];
	char nameservers[255];
};

char *my_debconf_input (const char *priority, const char *question);
void my_debconf_set (const char *question, const char *to);
char *my_debconf_interface_input (const char *interface, const char *priority, const char *questionprefix, const char *question);
void my_debconf_interface_ask (const char *interface, const char *priority, const char *questionprefix, const char *question);
char *my_debconf_interface_get (const char *interface, const char *questionprefix, const char *question);
void my_debconf_interface_set (const char *interface, const char *questionprefix, const char *question, const char *to);
void my_debconf_interface_register (const char *interface, const char *questionprefix, const char *question);
void my_debconf_interface_unregister (const char *interface, const char *questionprefix, const char *question);

void netcfg_die (void) __attribute__ ((noreturn));
void netcfg_die_cfg (void) __attribute__ ((noreturn));
int netcfg_common_interface_configured (const struct interface_config *config);
int netcfg_common_interface_configured_add (const struct interface_config *config);
void netcfg_common_get (struct interface_config *config);
void netcfg_common_get_hostname (struct interface_config *config);
int netcfg_common_get_interface (struct interface_config *config);
void netcfg_common_get_nameserver (struct interface_config *config);
void netcfg_common_set_type (const struct interface_config *config, const char *type);
int netcfg_common_activate (struct interface_config *config);
void netcfg_common_write_nameserver (struct interface_config *config);

#endif /* _NETCFG_H_ */
