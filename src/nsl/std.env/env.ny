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
public func set(cref name: string, cref value: string)
	-> !!__nanyc_env_set(name.pod, value.pod);

/*!
** \brief Set an environment variable for any value convertible into a string
**
** \param name The environment variable name
** \param value The new value
*/
public func set(cref name: string, cref value)
	-> !!__nanyc_env_set(name.pod, ((new string()) += value).pod);

/*!
** \brief Set an environment variable flag
**
** \param name The environment variable name
** \param value The new value
*/
public func set(cref name: string, cref flag: bool)
	-> !!__nanyc_env_set(name.pod, (if flag then "1" else "0").pod);


/*!
** \brief Unset an environment variable
** \param name The environment variable name
*/
public func unset(cref name: string)
	-> !!__nanyc_env_unset(name.pod);


/*!
** \brief Read the value of an environment variable
**
** \param name The environment variable name
** \return The value of the env variable 'name' if it exists, an empty string otherwise
*/
public func read(cref name: string): ref string
	-> new string(pod: !!__nanyc_env_read(name.pod, null));


/*!
** \brief Read the value of an environment variable
**
** \param name The environment variable name
** \param default The default value to use if the variable is not set
** \return The value of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref name: string, cref default: string): ref string
	-> new string(pod: !!__nanyc_env_read(name.pod, default.pod));


/*!
** \brief Read the value of an environment variable as a flag
**
** \note The following values will be considered as 'true' (case insensitive, false otherwise):
**       "y", "1", "true", "on"
** \param flag The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, false otherwise
*/
public func read(cref flag: string): ref bool
	-> new bool(!!__nanyc_env_asbool(flag.pod, __false));


/*!
** \brief Read the value of an environment variable as a flag
**
** \note The following values will be considered as 'true' (case insensitive, false otherwise):
**       "y", "1", "true", "on"
** \param flag The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref flag: string, cref default: bool): ref bool
	-> new bool(!!__nanyc_env_asbool(flag.pod, default.pod));


/*!
** \brief Read the value of an environment variable as signed 64bits integer
**
** \param asi64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asi64: string): ref i64
	-> new i64(!!__nanyc_env_asi64(asi64.pod, 0__i64));

/*!
** \brief Read the value of an environment variable as signed 64bits integer
**
** \param asi64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asi64: string, cref default: i64): ref i64
	-> new i64(!!__nanyc_env_asi64(asi64.pod, default.pod));


/*!
** \brief Read the value of an environment variable as unsigned 64bits integer
**
** \param asu64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asu64: string): ref u64
	-> new u64(!!__nanyc_env_asu64(asu64.pod, 0__u64));

/*!
** \brief Read the value of an environment variable as unsigned 64bits integer
**
** \param asu64 The environment variable name
** \return The value as a flag of the env variable 'name' if it exists, 'default' otherwise
*/
public func read(cref asu64: string, cref default: u64): ref u64
	-> new u64(!!__nanyc_env_asu64(asu64.pod, default.pod));



/*!
** \brief Get if an environment variable is set
**
** \param name The environment variable name
** \return True if the variable is set (can be empty), false otherwise
*/
public func exists(cref name: string): ref bool
	-> new bool(!!__nanyc_env_exists(name.pod));
