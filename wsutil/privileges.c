/* privileges.c
 * Routines for handling privileges, e.g. set-UID and set-GID on UNIX.
 *
 * $Id$
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 2006 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(HAVE_SETRESUID) || defined(HAVE_SETREGUID)
#define _GNU_SOURCE /* Otherwise [sg]etres[gu]id won't be defined on Linux */
#endif

#include <glib.h>

#include "privileges.h"

#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
#include <tchar.h>

/*
 * Called when the program starts, to save whatever credential information
 * we'll need later.
 */
void
get_credential_info(void)
{
	npf_sys_is_running();
}

/*
 * For now, we say the program wasn't started with special privileges.
 * There are ways of running programs with credentials other than those
 * for the session in which it's run, but I don't know whether that'd be
 * done with Wireshark/TShark or not.
 */
gboolean
started_with_special_privs(void)
{
	return FALSE;
}

/*
 * For now, we say the program isn't running with special privileges.
 * There are ways of running programs with credentials other than those
 * for the session in which it's run, but I don't know whether that'd be
 * done with Wireshark/TShark or not.
 */
gboolean
running_with_special_privs(void)
{
	return FALSE;
}

/*
 * For now, we don't do anything when asked to relinquish special privileges.
 */
void
relinquish_special_privs_perm(void)
{
}

/*
 * Get the current username.  String must be g_free()d after use.
 */
gchar *
get_cur_username(void) {
	gchar *username;
	username = g_strdup("UNKNOWN");
	return username;
}

/*
 * Get the current group.  String must be g_free()d after use.
 */
gchar *
get_cur_groupname(void) {
	gchar *groupname;
	groupname = g_strdup("UNKNOWN");
	return groupname;
}

/*
 * If npf.sys is running, return TRUE.
 */
gboolean
npf_sys_is_running() {
	SC_HANDLE h_scm, h_serv;
	SERVICE_STATUS ss;

	h_scm = OpenSCManager(NULL, NULL, 0);
	if (!h_scm)
		return FALSE;

	h_serv = OpenService(h_scm, _T("npf"), SC_MANAGER_CONNECT|SERVICE_QUERY_STATUS);
	if (!h_serv)
		return FALSE;

	if (QueryServiceStatus(h_serv, &ss)) {
		if (ss.dwCurrentState & SERVICE_RUNNING)
			return TRUE;
	}
	return FALSE;
}


#else /* _WIN32 */

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#include <glib.h>
#include <string.h>
#include <errno.h>

static uid_t ruid, euid;
static gid_t rgid, egid;
static gboolean get_credential_info_called = FALSE;

/*
 * Called when the program starts, to save whatever credential information
 * we'll need later.
 * That'd be the real and effective UID and GID on UNIX.
 */
void
get_credential_info(void)
{
	ruid = getuid();
	euid = geteuid();
	rgid = getgid();
	egid = getegid();

	get_credential_info_called = TRUE;
}

/*
 * "Started with special privileges" means "started out set-UID or set-GID",
 * or run as the root user or group.
 */
gboolean
started_with_special_privs(void)
{
	g_assert(get_credential_info_called);
#ifdef HAVE_ISSETUGID
	return issetugid();
#else
	return (ruid != euid || rgid != egid || ruid == 0 || rgid == 0);
#endif
}

/*
 * Return TRUE if the real, effective, or saved (if we can check it) user
 * ID or group are 0.
 */
gboolean
running_with_special_privs(void)
{
#ifdef HAVE_SETRESUID
	uid_t ru, eu, su;
#endif
#ifdef HAVE_SETRESGID
	gid_t rg, eg, sg;
#endif

#ifdef HAVE_SETRESUID
	getresuid(&ru, &eu, &su);
	if (ru == 0 || eu == 0 || su == 0)
		return TRUE;
#else
	if (getuid() == 0 || geteuid() == 0)
		return TRUE;
#endif
#ifdef HAVE_SETRESGID
	getresgid(&rg, &eg, &sg);
	if (rg == 0 || eg == 0 || sg == 0)
		return TRUE;
#else
	if (getgid() == 0 || getegid() == 0)
		return TRUE;
#endif
	return FALSE;
}

/*
 * Permanently relinquish  set-UID and set-GID privileges.
 * Ignore errors for now - if we have the privileges, we should
 * be able to relinquish them.
 */

void
relinquish_special_privs_perm(void)
{
	/*
	 * If we were started with special privileges, set the
	 * real and effective group and user IDs to the original
	 * values of the real and effective group and user IDs.
	 * If we're not, don't bother - doing so seems to mung
	 * our group set, at least in OS X 10.5.
	 *
	 * (Set the effective UID last - that takes away our
	 * rights to set anything else.)
	 */
	if (started_with_special_privs()) {
#ifdef HAVE_SETRESGID
		setresgid(rgid, rgid, rgid);
#else
		setgid(rgid);
		setegid(rgid);
#endif

#ifdef HAVE_SETRESUID
		setresuid(ruid, ruid, ruid);
#else
		setuid(ruid);
		seteuid(ruid);
#endif
	}
}

/*
 * Get the current username.  String must be g_free()d after use.
 */
gchar *
get_cur_username(void) {
	gchar *username;
	struct passwd *pw = getpwuid(getuid());

	if (pw) {
		username = g_strdup(pw->pw_name);
	} else {
		username = g_strdup("UNKNOWN");
	}
	endpwent();
	return username;
}

/*
 * Get the current group.  String must be g_free()d after use.
 */
gchar *
get_cur_groupname(void) {
	gchar *groupname;
	struct group *gr = getgrgid(getgid());

	if (gr) {
		groupname = g_strdup(gr->gr_name);
	} else {
		groupname = g_strdup("UNKNOWN");
	}
	endgrent();
	return groupname;
}

#endif /* _WIN32 */

/*
 * Editor modelines
 *
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: tabs
 * End:
 *
 * ex: set shiftwidth=8 tabstop=8 noexpandtab
 * :indentSize=8:tabSize=8:noTabs=false:
 */
