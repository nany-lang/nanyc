#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/core/noncopyable.h>
#include "classdef-table.h"
#include "type.h"
#include "vardef.h"




namespace Nany
{


	class LocalVariables final : public Yuni::NonCopyable<LocalVariables>
	{
	public:
		//! \name Constructors
		//@{
		/*!
		** \brief Default constructor
		*/
		LocalVariables(ClassdefTable& classdefs, yuint32 groupid);
		//! Move constructor
		LocalVariables(LocalVariables&&) = default;
		//@}

		/*!
		** \brief Clear the container
		*/
		void clear();

		/*!
		** \brief Get if a varid is valid
		*/
		bool exists(LVID) const;

		/*!
		** \brief Get the variable definition
		*/
		Vardef& vardef(LVID);

		/*!
		** \brief Get the variable definition (const)
		*/
		const Vardef& vardef(LVID) const;


		/*!
		** \brief Make a target var ID share the same definition than a source var ID
		*/
		bool makeHardlink(LVID source, LVID target);

		/*!
		** \brief Resize to N local variables
		**
		** \note The corresponding classdefs won't be created
		*/
		void bulkCreate(yuint32 count);


	public:
		//! Reference to the classdef table, where all types are stored
		ClassdefTable& classdefs;


	private:
		LVID pAtomID;
		LVID pMaxVarID;
		std::vector<Vardef> pLocalVars;

	}; // class LocalVariables







} // namespace Nany

#include "local-variables.hxx"

