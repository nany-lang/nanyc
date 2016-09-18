// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.io;




/*!
** \brief Try to mount a local folder
**
** \param path The target virtual folder
** \param localfolder A folder locally accessible by the current user
** \return True if the operation succeeded
*/
public func mount(cref path: string, cref localfolder: string): bool
	-> new bool(!!__nanyc_io_mount_local(path.m_cstr, path.size.pod, localfolder.m_cstr, localfolder.size.pod));






// -*- mode: nany;-*-
// vim: set filetype=nany:
