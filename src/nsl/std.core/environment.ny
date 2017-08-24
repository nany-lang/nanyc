// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// \file    env.ny
// \brief   Environment variables
// \ingroup std.env

namespace std.env;


/*!
** \brief Set an environment variable
**
** \param name The environment variable name
** \param value The new value
*/
public func set(cref name: string, cref value: string): void
	-> !!__nanyc_env_set(name.data, name.size.pod, value.data, value.size.pod);

/*!
** \brief Set an environment variable for any value convertible into a string
**
** \param name The environment variable name
** \param value The new value
*/
public func set(cref name: string, cref value): void
	-> std.env.set(name: name, value: ((new string()) += value));

/*!
** \brief Set an environment variable flag
**
** \param name The environment variable name
** \param value The new value
*/
public func set(cref name: string, cref flag: bool): void
	-> std.env.set(name: name, value: (if flag then "1" else "0"));

/*!
** \brief Unset an environment variable
** \param name The environment variable name
*/
public func unset(cref name: string): void
	-> !!__nanyc_env_unset(name.data, name.size.pod);

/*!
** \brief Read the value of an environment variable
**
** \param name The environment variable name
** \return The value of the env variable 'name' if it exists, an empty string otherwise
*/
public func read(cref name: string): ref string
	-> std.memory.nanyc_internal_create_string(!!__nanyc_env_read(name.data, name.size.pod, null, 0__u32));

/*!
** \brief Read the value of an environment variable
**
** \param name The environment variable name
** \param default The default value to use if the variable is not set
** \return The value of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref name: string, cref default: string): ref string
	-> std.memory.nanyc_internal_create_string(!!__nanyc_env_read(name.data, name.size.pod, default.data, default.size.pod));

/*!
** \brief Read the value of an environment variable as a flag
**
** \note The following values will be considered as 'true' (case insensitive, false otherwise):
**       "y", "1", "true", "on"
** \param flag The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, false otherwise
*/
public func read(cref flag: string): bool
	-> std.env.read(flag: flag, default: false);

/*!
** \brief Read the value of an environment variable as a flag
**
** \note The following values will be considered as 'true' (case insensitive, false otherwise):
**       "y", "1", "yes", "on", "true"
** \param flag The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref flag: string, cref default: bool): bool {
	ref value = read(name: flag);
	if not value.empty then {
		if value.size == 1u then
			return value == "1" or value == "y" or value == "Y";
		if value.size == 2u then
			return value == "on" or value == "ON";
		if value.size == 3u then
			return value == "yes" or value == "YES";
		if value.size == 4u then
			return value == "true" or value == "TRUE";
		return false;
	}
	return default;
}

/*!
** \brief Read the value of an environment variable as signed 64bits integer
**
** \param asi64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asi64: string): ref i64
	-> new i64(!!__nanyc_env_asi64(asi64.data, asi64.size.pod, 0__i64));

/*!
** \brief Read the value of an environment variable as signed 64bits integer
**
** \param asi64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asi64: string, cref default: i64): ref i64
	-> new i64(!!__nanyc_env_asi64(asi64.data, asi64.size.pod, default.pod));

/*!
** \brief Read the value of an environment variable as unsigned 64bits integer
**
** \param asu64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asu64: string): ref u64
	-> new u64(!!__nanyc_env_asu64(asu64.data, asu64.size.pod, 0__u64));

/*!
** \brief Read the value of an environment variable as unsigned 64bits integer
**
** \param asu64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asu64: string, cref default: u64): ref u64
	-> new u64(!!__nanyc_env_asu64(asu64.data, asu64.size.pod, default.pod));

/*!
** \brief Get if an environment variable is set
**
** \param name The environment variable name
** \return True if the variable is set (can be empty), false otherwise
*/
public func exists(cref name: string): ref bool
	-> new bool(!!__nanyc_env_exists(name.data, name.size.pod));
