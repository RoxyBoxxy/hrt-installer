/* 
   netcfg-static.c - Configure a static network for the debian-installer

   Copyright (C) 2000-2002  David Kimdon <dwhedon@debian.org>
                 2002  Bastian Blank <waldi@debian.org>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   
*/

#include <arpa/inet.h>
#include <ctype.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cdebconf/debconfclient.h>
#include <debian-installer.h>

#include "netcfg-static.h"

// convert a netmask (255.255.255.0) into the length (24)
static int inet_ptom (const char *src, short *dst)
{
	struct in_addr addr;
	in_addr_t mask, num;

	if (!inet_pton (AF_INET, src, &addr))
		return 0;

	mask = ntohl(addr.s_addr);

	for (num = mask; num & 1; num >>= 1);

	if (num != 0 && mask != 0)
	{
		for (num = ~mask; num & 1; num >>= 1);
		if (num)
			return 0;
	}

	for (num = 0; mask; mask <<= 1)
		num++;

	*dst = num;

	return 1;
}

// convert a length (24) into the netmask (255.255.255.0)
static int inet_mtop (short src, char *dst, size_t cnt)
{
	struct in_addr addr;
	in_addr_t mask = 0;

	for(; src; src--)
		mask |= 1 << (32 - src);

	addr.s_addr = htonl(mask);

	if (!inet_ntop (AF_INET, &addr, dst, cnt))
		return 0;

	return 1;
}

static void netcfg_static_get_loop_common (struct interface_config_static *config);
static void netcfg_static_get_loop_inet (struct interface_config_static *config, int af);

static int netcfg_static_get_common (struct interface_config_static *config);
static int netcfg_static_get_inet (struct interface_config_static *config, int af);

static void netcfg_static_register_common (struct interface_config_static *config);
static void netcfg_static_register_inet (struct interface_config_static *config, int af);

static void netcfg_static_get (struct interface_config_static *config)
{
	netcfg_common_get (&config->config_common);

	if (config->config_common.inet)
		netcfg_static_get_loop_inet (config, AF_INET);
	if (config->config_common.inet6)
		netcfg_static_get_loop_inet (config, AF_INET6);

	netcfg_static_get_loop_common (config);

	netcfg_common_interface_configured_add (&config->config_common);
}

static void netcfg_static_get_loop_common (struct interface_config_static *config)
{
	netcfg_static_register_common (config);

	while (1)
	{
		if (!netcfg_static_get_common (config))
			break;
	}
}

static void netcfg_static_get_loop_inet (struct interface_config_static *config, int af)
{
	netcfg_static_register_inet (config, af);

	while (1)
	{
		if (!netcfg_static_get_inet (config, af))
			break;
	}

	if (af == AF_INET)
		my_debconf_interface_set (config->config_common.interface, "common", "families/inet", "true");
	else
		my_debconf_interface_set (config->config_common.interface, "common", "families/inet6", "true");
}

static int netcfg_static_get_common (struct interface_config_static *config)
{
	char *ptr = NULL;

	if (config->config_common.info->pointopoint == PTP_YES)
	{
		my_debconf_interface_set (config->config_common.interface, "static", "common/pointopoint", "true");
		my_debconf_interface_ask (config->config_common.interface, "high", "static", "common/pointopoint_gateway");
	}

	client->command (client, "go", NULL);

	if (config->config_common.info->pointopoint == PTP_YES)
	{
		ptr = my_debconf_interface_get (config->config_common.interface, "static", "common/pointopoint_gateway");

		if (strstr (ptr, "true"))
			config->common.pointopoint_gateway = PTP_GATEWAY_YES;
		else
			config->common.pointopoint_gateway = PTP_GATEWAY_NO;

		client->command (client, "set", TEMPLATE_PREFIX "static/common/confirm_pointopoint", "true", NULL);

		client->command (client, "subst", TEMPLATE_PREFIX "static/common/confirm_pointopoint",
		                 "interface", config->config_common.interface, NULL);

		ptr = my_debconf_interface_get (config->config_common.interface, "static", "common/pointopoint_gateway");
		client->command (client, "subst", TEMPLATE_PREFIX "static/common/confirm_pointopoint",
		                 "gateway", ptr, NULL);

		ptr = my_debconf_input ("high", "static/common/confirm_pointopoint");
	}

	if (!ptr || strstr (ptr, "true"))
		return 0;

	return -1;
}

static int netcfg_static_get_inet (struct interface_config_static *config, int af)
{
	char *ptr;
	char addr[INET6_ADDRSTRLEN];

	if (af == AF_INET)
		my_debconf_interface_ask (config->config_common.interface, "high", "static", "inet/address");
	else
		my_debconf_interface_ask (config->config_common.interface, "high", "static", "inet6/address");

	if (config->config_common.info->pointopoint != PTP_YES)
	{
		if (af == AF_INET)
		{
			my_debconf_interface_ask (config->config_common.interface, "high", "static", "inet/netmask");
			my_debconf_interface_ask (config->config_common.interface, "high", "static", "inet/gateway");
		}
		else
		{
			my_debconf_interface_ask (config->config_common.interface, "high", "static", "inet6/netmask");
			my_debconf_interface_ask (config->config_common.interface, "high", "static", "inet6/gateway");
		}
	}

	client->command (client, "go", NULL);

	if (af == AF_INET)
	{
		ptr = my_debconf_interface_get (config->config_common.interface, "static", "inet/address");
		if (!inet_pton (AF_INET, ptr, &config->inet.address))
			netcfg_die ();
	}
	else
	{
		ptr = my_debconf_interface_get (config->config_common.interface, "static", "inet6/address");
		if (inet_pton (AF_INET6, ptr, &config->inet6.address) <= 0)
			netcfg_die ();
	}

	if (config->config_common.info->pointopoint == PTP_YES)
	{
		if (af == AF_INET)
			config->inet.netmask = 32;
		else
			config->inet6.netmask = 128;

		client->command (client, "set", TEMPLATE_PREFIX "static/confirm_pointopoint", "true", NULL);

		client->command (client, "subst", TEMPLATE_PREFIX "static/confirm_pointopoint",
		                 "interface", config->config_common.interface, NULL);

		if (af == AF_INET)
			inet_ntop (AF_INET, &config->inet.address, addr, INET_ADDRSTRLEN);
		else
			inet_ntop (AF_INET6, &config->inet6.address, addr, INET6_ADDRSTRLEN);

		client->command (client, "subst", TEMPLATE_PREFIX "static/confirm_pointopoint",
		                 "address", addr, NULL);

		ptr = my_debconf_input ("high", "static/confirm_pointopoint");
	}
	else
	{
		if (af == AF_INET)
		{
			ptr = my_debconf_interface_get (config->config_common.interface, "static", "inet/netmask");
			if (!inet_ptom (ptr, &config->inet.netmask))
				netcfg_die ();

			ptr = my_debconf_interface_get (config->config_common.interface, "static", "inet/gateway");
			if (!inet_pton (AF_INET, ptr, &config->inet.gateway))
				config->inet.no_gateway = 1;
		}
		else
		{
			ptr = my_debconf_interface_get (config->config_common.interface, "static", "inet6/netmask");
			config->inet6.netmask = (short) strtol (ptr, NULL, 10);

			ptr = my_debconf_interface_get (config->config_common.interface, "static", "inet6/gateway");
			if (inet_pton (AF_INET6, ptr, &config->inet6.gateway) <= 0)
				config->inet6.no_gateway = 1;
		}

		client->command (client, "set", TEMPLATE_PREFIX "static/confirm", "true", NULL);

		client->command (client, "subst", TEMPLATE_PREFIX "static/confirm",
		                 "interface", config->config_common.interface, NULL);

		if (af == AF_INET)
			inet_ntop (AF_INET, &config->inet.address, addr, INET_ADDRSTRLEN);
		else
			inet_ntop (AF_INET6, &config->inet6.address, addr, INET6_ADDRSTRLEN);
		client->command (client, "subst", TEMPLATE_PREFIX "static/confirm",
		                 "address", addr, NULL);

		if (af == AF_INET)
			inet_mtop (config->inet.netmask, addr, INET_ADDRSTRLEN);
		else
			snprintf (addr, INET6_ADDRSTRLEN, "%d", config->inet6.netmask);
		client->command (client, "subst", TEMPLATE_PREFIX "static/confirm",
		                 "netmask", addr, NULL);

		if (af == AF_INET)
		{
			if (config->inet.no_gateway)
				strcpy (addr, "-");
			else
				inet_ntop (AF_INET, &config->inet.gateway, addr, INET_ADDRSTRLEN);
		}
		else
		{
			if (config->inet6.no_gateway)
				strcpy (addr, "-");
			else
				inet_ntop (AF_INET6, &config->inet6.gateway, addr, INET6_ADDRSTRLEN);
		}
		client->command (client, "subst", TEMPLATE_PREFIX "static/confirm",
		                 "gateway", addr, NULL);

		ptr = my_debconf_input ("high", "static/confirm");
	}

	if (strstr (ptr, "true"))
		return 0;

	return -1;
}

static void netcfg_static_register_common (struct interface_config_static *config)
{
	my_debconf_interface_register (config->config_common.interface, "static", "common/pointopoint");

	if (config->config_common.info->pointopoint == PTP_YES)
		my_debconf_interface_register (config->config_common.interface, "static", "common/pointopoint_gateway");
}

static void netcfg_static_register_inet (struct interface_config_static *config, int af)
{
	if (af == AF_INET)
		my_debconf_interface_register (config->config_common.interface, "static", "inet/address");
	else
		my_debconf_interface_register (config->config_common.interface, "static", "inet6/address");

	if (config->config_common.info->pointopoint != PTP_YES)
	{
		if (af == AF_INET)
		{
			my_debconf_interface_register (config->config_common.interface, "static", "inet/netmask");
			my_debconf_interface_register (config->config_common.interface, "static", "inet/gateway");
		}
		else
		{
			my_debconf_interface_register (config->config_common.interface, "static", "inet6/netmask");
			my_debconf_interface_register (config->config_common.interface, "static", "inet6/gateway");
		}
	}
}

static int netcfg_static_activate (struct interface_config_static *config)
{
	int rv = 0;
	char buf[256], addr[INET6_ADDRSTRLEN], *ptr;
	int inet = 0, inet6 = 0;

	ptr = my_debconf_interface_get (config->config_common.interface, "common", "families/inet");
	if (strstr (ptr, "true"))
		inet = 1;
	ptr = my_debconf_interface_get (config->config_common.interface, "common", "families/inet6");
	if (strstr (ptr, "true"))
		inet6 = 1;

#ifdef __GNU__
	/* I had to do something like this ? */
	/*  di_execlog ("settrans /servers/socket/2 -fg");  */
	di_execlog ("settrans /servers/socket/2 --goaway");

	if (inet)
	{
		inet_ntop (AF_INET, config->inet.address, addr, INET6_ADDRSTRLEN);
		snprintf (buf, sizeof (buf),
				"settrans -fg /servers/socket/2 /hurd/pfinet --interface=%s --address=%s",
				config->config_common.interface, addr);

		if (config->info->pointopoint == PTP_YES)
			inet_mtop (32, addr, INET6_ADDRSTRLEN);
		else
			inet_mtop (config->inet.netmask, addr, INET6_ADDRSTRLEN);
		di_snprintfcat (buf, sizeof (buf), " --netmask=%s", addr);

		if (config->config_common.info->pointopoint != PTP_YES && !config->inet6.no_gateway)
		{
			inet_ntop (AF_INET, config->inet.gateway, addr, INET6_ADDRSTRLEN);
			di_snprintfcat (buf, sizeof (buf), " --gateway=%s", addr);
		}
		else if (config->common.pointopoint_gateway == PTP_GATEWAY_YES)
		{
			inet_ntop (AF_INET, config->inet.pointopoint, addr, INET6_ADDRSTRLEN);
			di_snprintfcat (buf, sizeof (buf), " --gateway=%s", addr);
		}

		rv |= di_execlog (buf);
	}
#else
	snprintf (buf, sizeof (buf), "/bin/ip link set %s up", config->config_common.interface);
	rv |= di_execlog (buf);

	if (inet)
	{
		inet_ntop (AF_INET, &config->inet.address, addr, INET6_ADDRSTRLEN);
		snprintf (buf, sizeof (buf), "/bin/ip addr add %s/%d dev %s", addr, config->inet.netmask, config->config_common.interface);
		rv |= di_execlog (buf);

		if (config->config_common.info->pointopoint != PTP_YES && !config->inet.no_gateway)
		{
			inet_ntop (AF_INET, &config->inet.gateway, addr, INET6_ADDRSTRLEN);
			snprintf (buf, sizeof (buf), "/bin/ip route add default via %s", addr);
			rv |= di_execlog (buf);
		}
		else if (config->common.pointopoint_gateway == PTP_GATEWAY_YES)
		{
			snprintf (buf, sizeof (buf), "/bin/ip route add default dev %s", config->config_common.interface);
			rv |= di_execlog (buf);
		}
	}
	if (inet6)
	{
		inet_ntop (AF_INET6, &config->inet6.address, addr, INET6_ADDRSTRLEN);
		snprintf (buf, sizeof (buf), "/bin/ip -f inet6 addr add %s/%d dev %s", addr, config->inet6.netmask, config->config_common.interface);
		rv |= di_execlog (buf);

		if (config->config_common.info->pointopoint != PTP_YES && !config->inet6.no_gateway)
		{
			inet_ntop (AF_INET6, &config->inet6.gateway, addr, INET6_ADDRSTRLEN);
			snprintf (buf, sizeof (buf), "/bin/ip -f inet6 route add default via %s", addr);
			rv |= di_execlog (buf);
		}
		else if (config->common.pointopoint_gateway == PTP_GATEWAY_YES)
		{
			snprintf (buf, sizeof (buf), "/bin/ip -f inet6 route add default dev %s", config->config_common.interface);
			rv |= di_execlog (buf);
		}
	}
#endif

	if (rv)
		return 1;
	return 0;
}

#if 0
static int netcfg_write_static()
{
        FILE *fp;

        if ((fp = file_open(NETWORKS_FILE))) {
                fprintf(fp, "localnet %s\n", num2dot(network));
                fclose(fp);
        } else
                goto error;

        if ((fp = file_open(INTERFACES_FILE))) {
                fprintf(fp,
                        "\n# This entry was created during the Debian installation\n");
                fprintf(fp,
                        "# (network, broadcast and gateway are optional)\n");
                fprintf(fp, "iface %s inet static\n", interface);
                fprintf(fp, "\taddress %s\n", num2dot(ipaddress));
                fprintf(fp, "\tnetmask %s\n", num2dot(netmask));
                fprintf(fp, "\tnetwork %s\n", num2dot(network));
                fprintf(fp, "\tbroadcast %s\n", num2dot(broadcast));
                if (gateway)
                        fprintf(fp, "\tgateway %s\n", num2dot(gateway));
                if (pointopoint)
                        fprintf(fp, "\tpointopoint %s\n",
                                num2dot(pointopoint));
                fclose(fp);
        } else
                goto error;

        return 0;
      error:
        return -1;
}
#endif

int main (int argc, char *argv[])
{
	static struct interface_config_static config;
	int need_nameserver = 1;

	client = debconfclient_new ();
	client->command (client, "title", "Static Network Configuration", NULL);

	netcfg_common_get_hostname (&config.config_common);

	while (1)
	{
		if (!netcfg_common_get_interface (&config.config_common))
			break;
		netcfg_common_set_type (&config.config_common, "static");
		netcfg_static_get (&config);

		if (need_nameserver)
			netcfg_common_get_nameserver (&config.config_common);

		if (netcfg_common_activate (&config.config_common))
			netcfg_die_cfg ();
		if (netcfg_static_activate (&config))
			netcfg_die_cfg ();

		need_nameserver = 0;
	}

	netcfg_common_write_nameserver (&config.config_common);
	//netcfg_static_write (&config);

	return 0;
}

