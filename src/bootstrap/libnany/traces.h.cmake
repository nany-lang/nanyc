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
namespace Traces
{

	//! Enable all traces (very verbose)
	static constexpr bool all = false;

	static constexpr bool recommended = all or false;




	//! Print all AST
	static constexpr bool ast = false;

	//! Print AST before normalization
	static constexpr bool printASTBeforeNormalize = ast or false;

	//! Print AST after normalization
	static constexpr bool printASTAfterNormalize  = ast or false;




	//! Print ATOM table
	static constexpr bool printAtomTable = all or recommended or false;

	//! Print ATOM table
	static constexpr bool printPreAtomTable = all or false;


	//! Print all types
	static constexpr bool printAllTypeDefinitions = all or false;

	//! Print classdef table
	static constexpr bool printClassdefTable = all or recommended or false;


	//! Print opcodes generated from AST
	static constexpr bool printSourceOpcodeSequence = all or recommended or false;

	//! Print opcodes after program instanciation
	static constexpr bool printGeneratedOpcodeSequence = all or recommended or false;



} // namespace Traces
} // namespace Config
} // namespace Nany

// vim: set ft=cpp:
