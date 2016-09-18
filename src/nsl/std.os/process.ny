// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// \file    process.ny
/// \ingroup std.os

namespace std.os;




/*!
** \brief Execute a command and wait for it
**
** \param cmd The command to execute (ex: "ls -l")
** \return True if the command has been executed and if the exit status is 0
*/
public func execute(cref cmd: string): ref
	-> new bool(!!__nanyc_os_execute(cmd.m_cstr, cmd.size.pod, 0__u32));


/*!
** \brief Execute a command and wait for it
**
** \param cmd The command to execute (ex: "ls -l")
** \param timeout Maximum execution time allowed for the command (in seconds - 0 means infinite)
** \return True if the command has been executed and if the exit status is 0
*/
public func execute(cref cmd: string, cref timeout: u32): ref
	-> new bool(!!__nanyc_os_execute(cmd.m_cstr, csm.size.pod, timeout.pod));
