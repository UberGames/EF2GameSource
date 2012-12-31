//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/Shared/qcommon/output.cpp                                $
// $Revision:: 14                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/06/02 7:10p                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//


#include "output.h"
#include <game/str.h> // Was uilib/str.h

//================================================================
// Name:				DocFileOutput
// Class:			DocFileOutput
//
// Description:	Constructor
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
DocFileOutput::DocFileOutput()
{
	typeFlag = 0;
	fileptr = NULL;
	classCount = 0;
	eventCount = 0;
}


//================================================================
// Name:				~DocFileOutput
// Class:			DocFileOutput
//
// Description:	Destructor
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
DocFileOutput::~DocFileOutput()
{
	
}


//================================================================
// Name:				OutputClasses
// Class:			DocFileOutput
//
// Description:	Loops through all the classes and calls OutputClass on each
//						(which will usually be a derived classes' version of it).
//
// Parameters:		ClassDef *classlist -- The list of classes to loop through
//
// Returns:			None
//
//================================================================
void DocFileOutput::OutputClasses(ClassDef *classlist)
{
	ClassDef *c;
	int num = 0;
	
	for( c = classlist->next; c != classlist; c = c->next )
	{
		if ( num < MAX_CLASSES )
		{
			OutputClass(c);
			num++;
		}
	}
}

//================================================================
// Name:				OutputClass
// Class:			DocFileOutput
//
// Description:	This function is normally overriden by a subclass.
//						The subclass's OutputClass will call this function
//						when its done with its own version.
//
// Parameters:		ClassDef *in_class -- The class to output
//
// Returns:			None
//
//================================================================
void DocFileOutput::OutputClass(ClassDef *in_class)
{
	OutputEvents(in_class);
	classCount++;
}

//================================================================
// Name:				OutputEvents
// Class:			DocFileOutput
//
// Description:	Loops through all the events and calls OutputEvent on each
//						(which will usually be a derived classes' version of it).
//						This function also filters out the events based on the typeFlag
//
// Parameters:		ClassDef in_class -- The class whose events we will loop through
//
// Returns:			None
//
//================================================================
void DocFileOutput::OutputEvents(ClassDef *in_class)
{
	ResponseDef<Class> *r;
	int			ev, i, num;
	Event       **events;
	
	num = Event::NumEventCommands();
	
	events = new Event *[num];
	memset( events, 0, sizeof( Event * ) * num );
	
	// gather event responses for this class
	r = in_class->responses;
	if ( r )
	{
		for( i = 0; r[i].event != NULL; i++ )
		{
			ev = ( int )*r[i].event;
			if ( r[i].response )
			{
				events[ev] = r[i].event;
			}
		}
	}
	
	for( i = 1; i < num; i++ )
	{
		int index;
		
		index = Event::MapSortedEventIndex( i );
		if ( events[index] )
		{
			// Filtering
			
			if ( events[index]->GetFlags() != 0 )
			{
				// Event is not default
				
				if ( ( events[index]->GetFlags() & EV_TIKIONLY ) && typeFlag == EVENT_SCRIPT_ONLY && !( events[index]->GetFlags() & EV_SCRIPTONLY ) )
					continue;
				
				if ( ( events[index]->GetFlags() & EV_SCRIPTONLY ) && typeFlag == EVENT_TIKI_ONLY && !( events[index]->GetFlags() & EV_TIKIONLY ) )
					continue;
				
				if ( ( events[index]->GetFlags() & EV_CODEONLY ) && ( typeFlag == EVENT_TIKI_ONLY || typeFlag == EVENT_SCRIPT_ONLY ) )
					continue;
				
				if ( ( events[index]->GetFlags() & EV_CHEAT ) && ( typeFlag == EVENT_TIKI_ONLY ) && !( events[index]->GetFlags() & EV_TIKIONLY ) )
					continue;
				
				if ( ( events[index]->GetFlags() & EV_CHEAT ) && ( typeFlag == EVENT_SCRIPT_ONLY ) && !( events[index]->GetFlags() & EV_SCRIPTONLY ) )
					continue;
				
				if ( ( events[index]->GetFlags() & EV_CONSOLE ) && ( typeFlag == EVENT_TIKI_ONLY || typeFlag == EVENT_SCRIPT_ONLY ) )
					continue;
			}
			
			OutputEvent( events[index] );
		}
	}
	
	delete[] events;
}

//================================================================
// Name:				OutputEvent
// Class:			DocFileOutput
//
// Description:	This function is normally overriden by a subclass.
//						The subclass's OutputEvent will call this function
//						when its done with its own version.
//
// Parameters:		Event *ev -- The event to output
//
// Returns:			None
//
//================================================================
void DocFileOutput::OutputEvent(Event *ev)
{
	OutputArguments(ev);
	eventCount++;
}

//================================================================
// Name:				OutputArguments
// Class:			DocFileOutput
//
// Description:	Loops through all the args and calls OutputArgument on each
//						(which will usually be a derived classes' version of it).
//
// Parameters:		Event *ev -- The event whose arguments we will loop through
//
// Returns:			None
//
//================================================================
void DocFileOutput::OutputArguments(Event *ev)
{
	int i;
	for( i = 1; i <= ev->getNumArgDefs(); i++ )
	{
		EventArgDef *evarg = ev->getArgDef(i);
		OutputArgument(evarg);
	}
}

//================================================================
// Name:				OutputArgument
// Class:			DocFileOutput
//
// Description:	This function is normally overriden by a subclass.
//						The subclass's OutputArgument will call this function
//						when its done with its own version.
//				
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
void DocFileOutput::OutputArgument(EventArgDef *evarg)
{
	// Function does nothing.  This is the last level of nesting, and there
	// is nothing else to go print from here.
}

//================================================================
// Name:				Write
// Class:			DocFileOutput
//	
// Description:	Writes the file to disk
//
// Parameters:		char *filename -- the filename to create
//
// Returns:			None
//
//================================================================
void DocFileOutput::Write(const char *filename, ClassDef *classlist, int ptypeFlag)
{
	char realname[255];
	sprintf(realname,"%s.%s",filename,GetExtension());
	fileptr = fopen(realname,"w");
	if ( !fileptr )
		return;
	
	// Store the type flag privately so we don't have to pass it around
	typeFlag = ptypeFlag;
	
	// Start the writing process
	OutputClasses(classlist);
	
	fclose(fileptr);
}


// ************************************************************************
// *                                                                      *
// *                                                                      *
// *							HTMLDocFileOutput Class                           *
// *                                                                      *
// *                                                                      *
// ************************************************************************




//================================================================
// Name:				HTMLDocFileOutput	
// Class:			HTMLDocFileOutput
//
// Description:	Constructor
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
HTMLDocFileOutput::HTMLDocFileOutput()
{
	
}

//================================================================
// Name:				HTMLDocFileOutput	
// Class:			~HTMLDocFileOutput
//
// Description:	Destructor
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
HTMLDocFileOutput::~HTMLDocFileOutput()
{
	
}

//================================================================
// Name:				OutputClasses
// Class:			HTMLDocFileOutput
//
// Description:	Override of the base class version.  We do this
//						so we can print header and footer information.
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
void HTMLDocFileOutput::OutputClasses(ClassDef *classlist)
{
	str class_title;
#if defined( GAME_DLL )
	class_title = "Game Module";
#elif defined( CGAME_DLL )
	class_title = "Client Game Module";
#else
	class_title = "Client Module";
#endif
	
	// Header Info
	fprintf(fileptr, "<HTML>\n");
	fprintf(fileptr, "<HEAD>\n");
	fprintf(fileptr, "<Title>%s Classes</Title>\n", class_title.c_str() );
	fprintf(fileptr, "</HEAD>\n");
	fprintf(fileptr, "<BODY>\n");
	fprintf(fileptr, "<H1>\n");
	fprintf(fileptr, "<center>%s Classes</center>\n", class_title.c_str() );
	fprintf(fileptr, "</H1>\n");
	
#if defined( GAME_DLL )
	fprintf(fileptr, "<h2>" );
	fprintf(fileptr, "<a href=\"#Actor\">Actor</a>, " );
	fprintf(fileptr, "<a href=\"#Animate\">Animate</a>, " );
	fprintf(fileptr, "<a href=\"#Entity\">Entity</a>, " );
	fprintf(fileptr, "<a href=\"#ScriptSlave\">ScriptSlave</a>, " );
	fprintf(fileptr, "<a href=\"#Sentient\">Sentient</a>, " );
	fprintf(fileptr, "<a href=\"#Trigger\">Trigger</a>, " );
	fprintf(fileptr, "<a href=\"#World\">World</a>" );
	fprintf(fileptr, "</h2>" );
#endif
	
	// Print the body
	DocFileOutput::OutputClasses(classlist);
	
	// Footer
	fprintf(fileptr, "<H2>\n");
	fprintf(fileptr, "%d %s Classes.<BR>%d Events.\n", classCount, class_title.c_str(), eventCount );
	fprintf(fileptr, "</H2>\n");
	fprintf(fileptr, "</BODY>\n");
	fprintf(fileptr, "</HTML>\n");
}

//================================================================
// Name:				OutputClass
// Class:			HTMLDocFileOutput
//
// Description:	Write the class output for this type of file
//
// Parameters:		ClassDef *in_class -- Class to write out
//
// Returns:			None
//
//================================================================
void HTMLDocFileOutput::OutputClass(ClassDef *in_class)
{
	ClassDef *savedClass;
	savedClass = in_class;
	
	fprintf(fileptr, "\n");
	if ( in_class->classID[ 0 ] )
	{
		fprintf(fileptr, "<h2> <a name=\"%s\">%s (<i>%s</i>)</a>", in_class->classname, in_class->classname, in_class->classID );
	}
	else
	{
		fprintf(fileptr, "<h2> <a name=\"%s\">%s</a>", in_class->classname, in_class->classname );
	}
	
	// print out lineage
	for( in_class = in_class->super; in_class != NULL; in_class = in_class->super )
	{
		fprintf(fileptr, " -> <a href=\"#%s\">%s</a>", in_class->classname, in_class->classname );
	}
	fprintf(fileptr, "</h2>\n");
	fprintf(fileptr, "<BLOCKQUOTE>\n");
	
	// Events
	DocFileOutput::OutputClass(savedClass);
	   
	fprintf(fileptr, "</BLOCKQUOTE>\n");
}

//================================================================
// Name:				OutputEvent
// Class:			HTMLDocFileOutput
//
// Description:	Write the event output for this type of file
//
// Parameters:		Event *ev -- Event to write out
//
// Returns:			None
//
//================================================================
void HTMLDocFileOutput::OutputEvent(Event *ev)
{
	int numargs;
	const char *text;
	
	fprintf(fileptr, "\n<P><tt><B>%s</B>", ev->getName() );
	
	numargs = ev->getNumArgDefs();
	
	if ( numargs )
	{
		fprintf(fileptr, "( <i>" );
		DocFileOutput::OutputEvent(ev);
		fprintf(fileptr, " </i>)</tt><BR>\n" );
	}
	else
	{
		// No arguments, do not call base class OutputEvent
		fprintf(fileptr, "</tt><BR>\n" );
	}
	
	text = ev->GetDocumentation();
	
	// Build a new documentation string, replaces the newlines with <BR>\n
	if ( text )
	{
		char new_doc[1024];
		unsigned old_index;
		int new_index = 0;
		
		for ( old_index = 0 ; old_index < strlen ( text ) ; old_index++ )
		{
			if ( text[old_index] == '\n' )
			{
				new_doc[new_index 	] = '<';
				new_doc[new_index + 1] = 'B';
				new_doc[new_index + 2] = 'R';
				new_doc[new_index + 3] = '>';
				new_doc[new_index + 4] = '\n';
				new_index += 5;
			}
			else
			{
				new_doc[new_index] = text[old_index];
				new_index++;
			}
		}
		
		new_doc[new_index] = 0;
		
		fprintf(fileptr, "<ul>%s</ul>\n", new_doc );
	}
}

//================================================================
// Name:				OutputArgument
// Class:			HTMLDocFileOutput
//
// Description:	Write the argument output for this type of file
//
// Parameters:		EventArgDef *evarg -- EventArgs to write out
//
// Returns:			None
//
//================================================================
void HTMLDocFileOutput::OutputArgument(EventArgDef *evarg)
{
	if ( evarg->isOptional() )
	{
		fprintf(fileptr, "[ " );
	}
	
	switch( evarg->getType() )
	{
	case IS_ENTITY:
		fprintf(fileptr, "Entity " );
		break;
	case IS_VECTOR:
		fprintf(fileptr, "Vector " );
		break;
	case IS_INTEGER:
		fprintf(fileptr, "Integer " );
		break;
	case IS_FLOAT:
		fprintf(fileptr, "Float " );
		break;
	case IS_STRING:
		fprintf(fileptr, "String " );
		break;
	case IS_BOOLEAN:
		fprintf(fileptr, "Boolean " );
		break;
	}
	fprintf(fileptr, "%s", evarg->getName() );
	
	// print the range of the argument
	OutputRange(evarg);
	
	if ( evarg->isOptional() )
	{
		fprintf(fileptr, " ]" );
	}
	
	DocFileOutput::OutputArgument(evarg);
}

//================================================================
// Name:				OutputRange
// Class:			HTMLDocFileOutput
//
// Description:	Writes the range of the event arg passed if any
//
// Parameters:		EventArgDef *evarg -- Argument to check for a range
//
// Returns:			None
//
//================================================================
void HTMLDocFileOutput::OutputRange(EventArgDef *evarg)
{
	qboolean integer, single;
	int numRanges, i;
	
	single = false;
	integer = true;
	numRanges = 1;
	switch( evarg->getType() )
	{
	case IS_VECTOR:
		integer = false;
		numRanges = 3;
		break;
	case IS_FLOAT:
		integer = false;
		break;
	case IS_STRING:
		single = true;
		break;
	}
	for( i = 0; i < numRanges; i++ )
	{
		if ( single )
		{
			if ( !evarg->GetMinRangeDefault(i) )
            {
				if ( integer )
				{
					fprintf(fileptr, "<%d>", ( int )evarg->GetMinRange(i) );
				}
				else
				{
					fprintf(fileptr, "<%.2f>", evarg->GetMinRange(i) );
				}
            }
		}
		else
		{
			// both non-default
			if ( !evarg->GetMinRangeDefault(i) && !evarg->GetMaxRangeDefault(i) )
            {
				if ( integer )
				{
					fprintf(fileptr, "<%d...%d>", ( int )evarg->GetMinRange(i), ( int )evarg->GetMaxRange(i) );
				}
				else
				{
					fprintf(fileptr, "<%.2f...%.2f>", evarg->GetMinRange(i), evarg->GetMaxRange(i) );
				}
            }
			// max default
			else if ( !evarg->GetMinRangeDefault(i) && evarg->GetMaxRangeDefault(i) )
            {
				if ( integer )
				{
					fprintf(fileptr, "<%d...max_integer>", ( int )evarg->GetMinRange(i) );
				}
				else
				{
					fprintf(fileptr, "<%.2f...max_float>", evarg->GetMinRange(i) );
				}
            }
			// min default
			else if ( evarg->GetMinRangeDefault(i) && !evarg->GetMaxRangeDefault(i) )
            {
				if ( integer )
				{
					fprintf(fileptr, "<min_integer...%d>", ( int )evarg->GetMaxRange(i) );
				}
				else
				{
					fprintf(fileptr, "<min_float...%.2f>", evarg->GetMaxRange(i) );
				}
            }
		}
	}
}



// ************************************************************************
// *                                                                      *
// *                                                                      *
// *							ToolDocFileOutput Class                           *
// *                                                                      *
// *                                                                      *
// ************************************************************************




//================================================================
// Name:				ToolDocFileOutput	
// Class:			ToolDocFileOutput
//
// Description:	Constructor
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
ToolDocFileOutput::ToolDocFileOutput()
{
	randFlag = false;
	colorFlag = false;
}

//================================================================
// Name:				ToolDocFileOutput	
// Class:			~ToolDocFileOutput
//
// Description:	Destructor
//
// Parameters:		None
//
// Returns:			None
//
//================================================================
ToolDocFileOutput::~ToolDocFileOutput()
{
	
}

//================================================================
// Name:				OutputClass
// Class:			ToolDocFileOutput
//
// Description:	Write the class output for this type of file
//
// Parameters:		ClassDef *in_class -- Class to write out
//
// Returns:			None
//
//================================================================
void ToolDocFileOutput::OutputClass(ClassDef *in_class)
{
	ClassDef *savedClass;
	savedClass = in_class;
	
	fprintf(fileptr, "\n");
	fprintf(fileptr, "%s\n", in_class->classname );
	fprintf(fileptr, "{\n");
	
	// Events
	DocFileOutput::OutputClass(savedClass);
	if ( in_class->super != NULL )
		fprintf(fileptr, "\n\tincludes %s\n",in_class->super->classname);
	
	fprintf(fileptr, "}\n");
}

//================================================================
// Name:				OutputEvent
// Class:			ToolDocFileOutput
//
// Description:	Write the event output for this type of file
//
// Parameters:		Event *ev -- Event to write out
//
// Returns:			None
//
//================================================================
void ToolDocFileOutput::OutputEvent(Event *ev)
{
	int numargs;
	const char *text;
	unsigned i;
	
	fprintf(fileptr, "\n");
	fprintf(fileptr, "\t%s\n", ev->getName() );
	fprintf(fileptr, "\t{\n");
	
	numargs = ev->getNumArgDefs();
	
	if ( numargs )
	{
		DocFileOutput::OutputEvent(ev);
	}
	
	fprintf(fileptr, "\n\t\thelp\t\t\"");
	
	text = ev->GetDocumentation();
	for ( i=0; i<strlen(text); i++ )
	{
		if ( text[i] == '\n' )
			fprintf(fileptr, "\n\t\t\t\t");
		else
			fprintf(fileptr, "%c", text[i]);
	}
	
	fprintf(fileptr, "\"\n");
	fprintf(fileptr, "\t}\n");
	
	text = ev->GetDocumentation();
}

//================================================================
// Name:				OutputArgument
// Class:			ToolDocFileOutput
//
// Description:	Write the argument output for this type of file
//
// Parameters:		EventArgDef *evarg -- EventArgs to write out
//
// Returns:			None
//
//================================================================
void ToolDocFileOutput::OutputArgument(EventArgDef *evarg)
{
	// Check for Randoms.  If there is one, print out the "random" keyword
	// and set the random flag, so next next time this function is called,
	// it is assumed to be the random parameter's name.
	if ( !strcmp(evarg->getName(), "[randomtype]") )
	{
		fprintf(fileptr, "\t\trandom");
		randFlag = true; // Set the random flag
		return;
	}
	
	// If the random flag is set, we assume the previous call to this function
	// was a [randomtype] parameter.  We now output the name of the random parameter,
	// which is assumed to be a float. (All randoms ARE floats, correct?)
	if ( randFlag ) 
	{
		fprintf(fileptr, "\t\t%s\t\"%s\"\n",evarg->getName(), evarg->getName());
		randFlag = false;
		return;
	}
	
	// If it's a vector that's supposed to specify a color, we directly print it here
	if ( strstr(evarg->getName(), "color_") && evarg->getType() == IS_VECTOR )
	{
		fprintf(fileptr, "\t\trgb\t\t%s\t\"%s\"\n",evarg->getName(), evarg->getName());
		return;
	}
	
	// If the event takes an RGB color (three floats), we must do special casing to 
	// output a single color parameter.
	// If we find color_red, we ASSUME it's the start of an RGB set.
	if ( strstr(evarg->getName(), "color_red") && evarg->getType() != IS_VECTOR )
	{
		char tmpstr[25], *sptr;
		strcpy(tmpstr,evarg->getName());
		sptr = strtok(tmpstr,"_"); // color
		sptr = strtok(NULL,"_"); // red
		sptr = strtok(NULL,"_"); // potential name or NULL
		
		colorFlag = true;
		if ( sptr == NULL )
			fprintf(fileptr, "\t\trgb\t\tcolor\t\"color\"\n");
		else
			fprintf(fileptr, "\t\trgb\t\t%s\t\"%s\"\n", sptr, sptr);
		return;
	}
	
	// If we find color_blue, we ASSUME is the end of an RGB set
	if ( colorFlag && strstr(evarg->getName(), "color_blue") )
	{
		colorFlag = false;
		return;
	}
	
	// If the colorFlag is still set by the time we get here, we are in
	// the process of ignoring parameters until the color is done.
	if ( colorFlag )
		return;
	
	// If we reach this point it is assumed that it is not one of the special case parameters
	// listed above.
	if ( evarg->isOptional() )
		fprintf(fileptr, "\t\t|" );
	else
		fprintf(fileptr, "\t\t");
	
	switch( evarg->getType() )
	{
	case IS_ENTITY:
		fprintf(fileptr, "string\t\t" );
		break;
	case IS_VECTOR:
		fprintf(fileptr, "xyz\t\t" );
		break;
	case IS_INTEGER:
		fprintf(fileptr, "int\t\t" );
		break;
	case IS_FLOAT:
		fprintf(fileptr, "float\t\t" );
		break;
	case IS_STRING:
		fprintf(fileptr, "string\t\t" );
		break;
	case IS_BOOLEAN:
		fprintf(fileptr, "boolean\t\t" );
		break;
	}
	fprintf(fileptr, "%s\t", evarg->getName() );
	fprintf(fileptr, "\"%s\"\t", evarg->getName() );
	
	// print the range of the argument
	OutputRange(evarg);
	fprintf(fileptr, "\n");
	
	DocFileOutput::OutputArgument(evarg);
}

//================================================================
// Name:				OutputRange
// Class:			ToolDocFileOutput
//
// Description:	Writes the range of the event arg passed if any
//
// Parameters:		EventArgDef *evarg -- Argument to check for a range
//
// Returns:			None
//
//================================================================
void ToolDocFileOutput::OutputRange(EventArgDef *evarg)
{
	int i;
	
	// Event type is a vector
	if ( evarg->getType() == IS_VECTOR )
	{
		for ( i=0; i<3; i++ )
		{
			if ( evarg->GetMinRangeDefault(i) || evarg->GetMaxRangeDefault(i) )
				return;
		}
		
		fprintf(fileptr, "%.2f %.2f %.2f %.2f %.2f %.2f\n", 
			evarg->GetMinRange(0), evarg->GetMinRange(1), evarg->GetMinRange(2), 
			evarg->GetMaxRange(0), evarg->GetMaxRange(1), evarg->GetMaxRange(2) );
		
		return;
	}
	
	// Event type is a string
	if ( evarg->getType() == IS_STRING )
	{
		if ( !evarg->GetMinRangeDefault(0) )
		{
			fprintf(fileptr, "%d", ( int )evarg->GetMinRange(0) );
		}
		
		return;
	}
	
	// Default checks
	if ( !evarg->GetMinRangeDefault(0) && !evarg->GetMaxRangeDefault(0) )
	{
		if ( evarg->getType() == IS_FLOAT )
			fprintf(fileptr, "%.2f %.2f", evarg->GetMinRange(0), evarg->GetMaxRange(0) );
		else
			fprintf(fileptr, "%d %d", ( int )evarg->GetMinRange(0), ( int )evarg->GetMaxRange(0) );
	}
}

//================================================================
// Name:				PrintSubClasses (private)
// Class:			ToolDocFileOutput
//
// Description:	Recursive function that prints all the passed classes
//						subclasses.
//
// Parameters:		ClassDef *in_class -- the class to print the subclasses of
//
// Returns:			None
//
//================================================================
void ToolDocFileOutput::PrintSubClasses(ClassDef *in_class, ClassDef *classlist)
{
	ClassDef *c;
	OutputClass(in_class);
	for( c = classlist->next; c != classlist; c = c->next )
	{
		if ( c->super == in_class )
			PrintSubClasses(c, classlist);
	}
}

//================================================================
// Name:				OutputClasses
// Class:			ToolDocFileOutput
//
// Description:	Override of the base class version.  This is a special
//						fuctions that loops through the classes in a specific way.
//						For each root node, we recursively print all it's children.
//						This way, all classes' super classes are printed first
//
// Parameters:		ClassDef *classlist -- The list of classes to loop through
//
// Returns:			None
//
//================================================================
void ToolDocFileOutput::OutputClasses(ClassDef *classlist)
{
	ClassDef *c;
	
	// Make a container of all the root class names
	for( c = classlist->next; c != classlist; c = c->next )
	{
		if ( c->super == NULL )
			PrintSubClasses(c, classlist);
	}
}
