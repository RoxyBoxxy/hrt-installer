/* 
   netcfg.c - Shared functions used to configure the network for 
	   the debian-installer.

   Copyright (C) 2000-2002  David Kimdon <dwhedon@debian.org>
   
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

#include <ctype.h>
#include <net/if.h>
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

#include "netcfg.h"

struct interface_info interface_info[] =
{
	{ "eth", "Ethernet or Fast Ethernet", PTP_NO, INET6_YES },
	{ "pcmcia", "PC-Card (PCMCIA) Ethernet or Token Ring", PTP_NO, INET6_YES },
	{ "tr", "Token Ring", PTP_NO, INET6_NO },
	{ "arc", "Arcnet", PTP_NO, INET6_NO },
	{ "slip", "Serial-line IP", PTP_YES, INET6_NO },
	{ "plip", "Parallel-line IP", PTP_YES, INET6_NO },
	{ "ctc", "Channel-to-channel", PTP_YES, INET6_PART },
	{ "escon", "Real channel-to-channel", PTP_YES, INET6_PART },
	{ "hsi", "Hypersocket", PTP_NO, INET6_PART },
	{ "iucv", "Inter-user communication vehicle", PTP_YES, INET6_YES },
	{ "sit", "IPv6-in-IPv4 tunnel", PTP_YES, INET6_ONLY },
	{ "gre", "GRE tunnel", PTP_YES, INET6_YES },
	{ NULL, "unknown interface", PTP_NO, INET6_NO }
};

char *my_debconf_input (const char *priority, const char *question)
{
	char template[256];
	snprintf (template, 256, TEMPLATE_PREFIX "%s", question);

	client->command (client, "fset", template, "seen", "false", NULL);
	client->command (client, "input", priority, template, NULL);
	client->command (client, "go", NULL);
	client->command (client, "get", template, NULL);
	return client->value;
}

void my_debconf_set (const char *question, const char *to)
{
	char template[256];
	snprintf (template, 256, TEMPLATE_PREFIX "%s", question);

	client->command (client, "set", template, to, NULL);
}

static void my_debconf_interface_build (const char *interface, const char *questionprefix, const char *question, char *template, size_t size)
{
	snprintf (template, size, TEMPLATE_PREFIX "interfaces/%s/%s/%s", questionprefix, interface, question);
}

char *my_debconf_interface_input (const char *interface, const char *priority, const char *questionprefix, const char *question)
{
	char template[256];
	my_debconf_interface_build (interface, questionprefix, question, template, 256);

	client->command (client, "fset", template, "seen", "false", NULL);
	client->command (client, "input", priority, template, NULL);
	client->command (client, "go", NULL);
	client->command (client, "get", template, NULL);
	return client->value;
}

void my_debconf_interface_ask (const char *interface, const char *priority, const char *questionprefix, const char *question)
{
	char template[256];
	my_debconf_interface_build (interface, questionprefix, question, template, 256);

	client->command (client, "fset", template, "seen", "false", NULL);
	client->command (client, "input", priority, template, NULL);
}

char *my_debconf_interface_get (const char *interface, const char *questionprefix, const char *question)
{
	char template[256];
	my_debconf_interface_build (interface, questionprefix, question, template, 256);

	client->command (client, "get", template, NULL);
	return client->value;
}

void my_debconf_interface_set (const char *interface, const char *questionprefix, const char *question, const char *to)
{
	char template[256];
	my_debconf_interface_build (interface, questionprefix, question, template, 256);

	client->command (client, "set", template, to, NULL);
}

void my_debconf_interface_register (const char *interface, const char *questionprefix, const char *question)
{
	char template1[256];
	char template2[256];
	snprintf (template1, 256, TEMPLATE_PREFIX "%s/%s", questionprefix, question);
	my_debconf_interface_build (interface, questionprefix, question, template2, 256);

	client->command (client, "register", template1, template2, NULL);
}

void my_debconf_interface_unregister (const char *interface, const char *questionprefix, const char *question)
{
	char template[256];
	my_debconf_interface_build (interface, questionprefix, question, template, 256);

	client->command (client, "unregister", template, NULL);
}

static int getif_is_interface_up(char *inter)
{
        struct ifreq ifr;
        int sfd = -1, ret = -1;

        if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
                goto int_up_done;

        strncpy(ifr.ifr_name, inter, sizeof(ifr.ifr_name));

        if (ioctl(sfd, SIOCGIFFLAGS, &ifr) < 0)
                goto int_up_done;

        ret = (ifr.ifr_flags & IFF_UP) ? 1 : 0;

      int_up_done:
        if (sfd != -1)
                close(sfd);
        return ret;
}

static void getif_get_name(char *name, char *p)
{
        while (isspace(*p))
                p++;
        while (*p) {
                if (isspace(*p))
                        break;
                if (*p == ':') {        /* could be an alias */
                        char *dot = p, *dotname = name;
                        *name++ = *p++;
                        while (isdigit(*p))
                                *name++ = *p++;
                        if (*p != ':') {        /* it wasn't, backup */
                                p = dot;
                                name = dotname;
                        }
                        if (*p == '\0')
                                return;
                        p++;
                        break;
                }
                *name++ = *p++;
        }
        *name++ = '\0';
        return;
}

static FILE *ifs = NULL;
static char ibuf[512];

static void getif_start (void)
{
        if (ifs != NULL) {
                fclose(ifs);
                ifs = NULL;
        }
        if ((ifs = fopen("/proc/net/dev", "r")) != NULL) {
                fgets(ibuf, sizeof(ibuf), ifs); /* eat header */
                fgets(ibuf, sizeof(ibuf), ifs); /* ditto */
        }
        return;
}

static char *getif (int all)
{
        char rbuf[512];
        if (ifs == NULL)
                return NULL;

        if (fgets(rbuf, sizeof(rbuf), ifs) != NULL) {
                getif_get_name(ibuf, rbuf);
                if (!strcmp(ibuf, "lo"))        /* ignore the loopback */
                        return getif(all);      /* seriously doubt there is an infinite number of lo devices */
                if (all || getif_is_interface_up(ibuf) == 1)
                        return ibuf;
        }
        return NULL;
}

static void getif_end (void)
{
        if (ifs != NULL) {
                fclose(ifs);
                ifs = NULL;
        }
        return;
}

static const struct interface_info *const netcfg_common_interface_info (const char *interface)
{
	int i;

	for (i = 0; interface_info[i].interface != NULL; i++)
		if (!strncmp(interface, interface_info[i].interface,
					strlen(interface_info[i].interface)))
			return &interface_info[i];

	return &interface_info[i];
}

void netcfg_die (void)
{
	client->command (client, "fset", TEMPLATE_PREFIX "common/error", "seen", "false", NULL);
	client->command (client, "input", "critical", TEMPLATE_PREFIX "common/error", NULL);
	client->command (client, "go", NULL);
	exit (1);
}

void netcfg_die_cfg (void)
{
	client->command (client, "fset", TEMPLATE_PREFIX "common/error_cfg", "seen", "false", NULL);
	client->command (client, "input", "critical", TEMPLATE_PREFIX "common/error_cfg", NULL);
	client->command (client, "go", NULL);
	exit (1);
}

int netcfg_common_interface_configured (const struct interface_config *config)
{
	char *ptr;

	client->command (client, "get", TEMPLATE_PREFIX "interfaces/configured", NULL);

	if (strlen (client->value))
	{
		ptr = strtok (client->value, ",");
		do
		{
			if (!strcmp(ptr, config->interface))
				return 1;
		}
		while ((ptr = strtok (NULL, ",")));
	}

	return 0;
}

int netcfg_common_interface_configured_add (const struct interface_config *config)
{
	char *ptr;

	client->command (client, "get", TEMPLATE_PREFIX "interfaces/configured", NULL);

	if (strlen (client->value))
	{
		ptr = strtok (client->value, ",");
		do
		{
			if (!strcmp(ptr, config->interface))
				return 0;
		}
		while ((ptr = strtok (NULL, ",")));
	}

	ptr = malloc (strlen (client->value) + strlen (config->interface) + 2);
	if (!ptr)
		netcfg_die ();
	*ptr = '\0';
	if (strlen (client->value))
	{
		strcat (ptr, client->value);
		strcat (ptr, ",");
	}
	strcat (ptr, config->interface);

	client->command (client, "set", TEMPLATE_PREFIX "interfaces/configured", ptr, NULL);

	free (ptr);
	return 1;
}

#if 0
void
netcfg_write_common(u_int32_t ipaddress, char *domain, char *hostname, 
		u_int32_t nameservers[])
{
        FILE *fp;


        if ((fp = file_open(HOSTS_FILE))) {
                fprintf(fp, "127.0.0.1\tlocalhost\n");
                if (ipaddress) {
                        //if (domain)
                                //fprintf(fp, "%s\t%s.%s\t%s\n",
                                //        num2dot(ipaddress), hostname,
				//	domain, hostname);
                        //else
                                //fprintf(fp, "%s\t%s\n", num2dot(ipaddress),
				//		hostname);
                }

                fclose(fp);
        }

        if ((fp = file_open(RESOLV_FILE))) {
                int i = 0;
                if (domain)
                        fprintf(fp, "search %s\n", domain);

                //while (nameservers[i])
                //        fprintf(fp, "nameserver %s\n",
                //                num2dot(nameservers[i++]));

                fclose(fp);
        }
}
#endif

static void netcfg_common_register (struct interface_config *config);

void netcfg_common_get (struct interface_config *config)
{
	netcfg_common_register(config);

	config->inet = 0;
	config->inet6 = 0;

#ifndef __GNU__
	if (config->info->inet6 == INET6_NO)
#endif /* __GNU__ */
		config->inet = 1;
#ifndef __GNU__
	else if (config->info->inet6 == INET6_ONLY)
		config->inet6 = 1;
	else
	{
		char *ptr, *ptr2;

		if (netcfg_common_interface_configured (config))
		{
			int inet = 0, inet6 = 0;

			ptr = my_debconf_interface_get (config->interface, "common", "families/inet");
			if (strstr (ptr, "true"))
				inet = 1;
			ptr = my_debconf_interface_get (config->interface, "common", "families/inet6");
			if (strstr (ptr, "true"))
				inet6 = 1;

			if (inet && inet6)
				ptr = "INET+INET6: IPv4 and IPv6";
			else if (inet6)
				ptr = "INET6: IPv6 only";
			else
				ptr = "INET: IPv4 only";
		}
		else
			ptr = "INET: IPv4 only";

		if (config->info->inet6 == INET6_PART)
			ptr2 = "common/families/choose_inet6_part";
		else
			ptr2 = "common/families/choose";

		my_debconf_set (ptr2, ptr);
		ptr = my_debconf_input ("medium", ptr2);

		ptr2 = strchr (ptr, ':');
		*ptr2 = '\0';

		if (!strcmp ("INET+INET6", ptr))
		{
			config->inet = 1;
			config->inet6 = 1;
		}
		else if (!strcmp ("INET6", ptr))
			config->inet6 = 1;
		else
			config->inet = 1;
	}
#endif /* __GNU__ */
}

static void netcfg_common_register (struct interface_config *config)
{
	my_debconf_interface_register (config->interface, "common", "families/inet");
	my_debconf_interface_register (config->interface, "common", "families/inet6");
}

void netcfg_common_get_hostname (struct interface_config *config)
{
	char *ptr;
	static const char *valid_chars =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
	size_t len;

	while (1)
	{
		ptr = my_debconf_input ("high", "common/hostname");
		len = strlen(ptr);

		/* Check the hostname for RFC 1123 compliance.  */
		if (len < 2 ||
				len > 63 ||
				strspn(ptr, valid_chars) != len ||
				ptr[len - 1] == '-' ||
				ptr[0] == '-') {
			client->command (client, "subst", TEMPLATE_PREFIX "common/invalid_hostname", "hostname", ptr, NULL);
			my_debconf_input ("high", "common/invalid_hostname");
		}
		else
			break;

	}
}

int netcfg_common_get_interface (struct interface_config *config)
{
	char *inter;
	int len;
	int newchars;
	char *ptr;
	int num_interfaces = 0;
	const struct interface_info *info;

	if (!(ptr = malloc (128)))
		goto error;

	len = 128;
	*ptr = '\0';

	getif_start();
	while ((inter = getif (1)) != NULL)
	{
		info = netcfg_common_interface_info (inter);
		newchars = strlen(inter) + strlen(info->description) + 5;
		if (len < (strlen(ptr) + newchars)) {
			if (!(ptr = realloc(ptr, len + newchars + 128)))
				goto error;
			len += newchars + 128;
		}

		di_snprintfcat(ptr, len, "%s: %s, ", inter, info->description);
		num_interfaces++;
	}
	getif_end();

	if (num_interfaces == 0)
	{
		my_debconf_input ("high", "common/no_interfaces");
		exit(1);
	}
	else 
	{
		client->command(client, "subst", TEMPLATE_PREFIX "common/choose_interface",
				"ifchoices", ptr, NULL);
		free(ptr);
		inter = my_debconf_input ("high", "common/choose_interface");

		if (!inter)
			goto error;
	}

	if (!strcmp (inter, "Quit"))
		return 0;

	ptr = strchr (inter, ':');
	if (ptr == NULL)
		goto error;
	*ptr = '\0';

	strncpy (config->interface, inter, 10);
	config->info = netcfg_common_interface_info (inter);

	return 1;

error:
	if (ptr)
		free (ptr);

	netcfg_die ();
}

void netcfg_common_get_nameserver (struct interface_config *config)
{
	char *ptr;

	ptr = my_debconf_input ("medium", "common/domain");
	strncpy (config->domain, ptr, 255);
	ptr = my_debconf_input ("medium", "common/nameservers");
	strncpy (config->nameservers, ptr, 255);
}

void netcfg_common_set_type (const struct interface_config *config, const char *type)
{
	my_debconf_interface_register (config->interface, "common", "type");
	my_debconf_interface_set (config->interface, "common", "type", type);
}

int netcfg_common_activate (struct interface_config *config)
{
	di_execlog ("modprobe ipv6");
	return 0;
}

void netcfg_common_write_nameserver (struct interface_config *config)
{
	FILE *f;

        f = fopen(RESOLV_FILE, "w");

        if (f)
	{
		if (strlen (config->domain))
			fprintf (f, "search %s\n", config->domain);

		if (strlen (config->nameservers))
		{
			char *ptr;

			ptr = strtok (config->nameservers, " ,");
			do
				fprintf (f, "nameserver %s\n", ptr);
			while ((ptr = strtok (NULL, " ,")));
		}

		fclose (f);
	}
}


