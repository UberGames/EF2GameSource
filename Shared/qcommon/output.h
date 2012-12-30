//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/Shared/qcommon/output.h                                  $
// $Revision:: 10                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//

class DocFileOutput;

#ifndef __DOCFILEOUTPUT_H__
#define __DOCFILEOUTPUT_H__

#if defined ( GAME_DLL )
// class.h behaves different when the GAME_DLL define is set, so
// we need to act differently to.  If we simply include class.h here,
// it includes g_local.h, which then includes game.h which fails to
// compile because class.h isn't fully read yet.  To prevent this, 
// we include g_local.h first, so that it can include things in the
// right order.
#include <game/g_local.h>
#else
#include <game/listener.h>
#include <game/class.h>
#endif

// Base Class
class DocFileOutput 
	{
	protected:
		FILE *fileptr;
		int typeFlag;

		int classCount;
		int eventCount;

	public:
		DocFileOutput();
		virtual ~DocFileOutput();

		virtual void OutputClasses(ClassDef *classlist);
		virtual void OutputClass(ClassDef *in_class);
		virtual void OutputEvents(ClassDef *in_class);
		virtual void OutputEvent(Event *ev);
		virtual void OutputArguments(Event *ev);
		virtual void OutputArgument(EventArgDef *evarg);

		virtual const char *GetExtension() { return "txt"; }

		void Write(const char *filename, ClassDef *classlist, int ptypeFlag);
	};

// HTML Output
class HTMLDocFileOutput : public DocFileOutput
	{
	public:
		HTMLDocFileOutput();
		~HTMLDocFileOutput();

		// Virtual Function Implementation
		void OutputClass(ClassDef *in_class);
		void OutputEvent(Event *ev);
		void OutputArgument(EventArgDef *evarg);

		// Special override
		void OutputClasses(ClassDef *classlist);

		void OutputRange(EventArgDef *evarg);

		const char *GetExtension() { return "html"; }
	};

// Tool Output -- Output to the specific parser format that
// the tools use.
class ToolDocFileOutput : public DocFileOutput
	{
	private:
		void PrintSubClasses(ClassDef *in_class, ClassDef *classlist); // recursive
		bool randFlag;
		bool colorFlag;

	public:
		ToolDocFileOutput();
		~ToolDocFileOutput();

		// Virtual Function Implementation
		void OutputClass(ClassDef *in_class);
		void OutputEvent(Event *ev);
		void OutputArgument(EventArgDef *evarg);

		// Special override
		void OutputClasses(ClassDef *classlist);

		void OutputRange(EventArgDef *evarg);

		const char *GetExtension() { return "txt"; }
	};

#endif // #ifndef __DOCFILEOUTPUT_H__
