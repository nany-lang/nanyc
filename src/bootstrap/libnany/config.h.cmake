/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

/* NOTE: this file is generated by cmake */



namespace Nany
{
namespace Config
{

	//! The maximum number of nested namespaces
	static constexpr uint32_t maxNamespaceDepth = 32;

	//! Maximum number of parameters when declaring a function
	static constexpr uint32_t maxFuncDeclParameterCount = 7;

	//! Maximum length for a symbol name
	static constexpr uint32_t maxSymbolNameLength = 64 - 1 /*zero-terminated*/;

	//! Maxmimum number of pushed parameters for calling a function
	static constexpr uint32_t maxPushedParameters = 32;



	//! Remove redundant dbg info (line,offset) in opcode programs
	static constexpr bool removeRedundantDbgOffset = true;


	//! Import the NSL
	static constexpr bool importNSL = true;




} // namespace Config
} // namespace Nany
/* vim: set ft=cpp: */
