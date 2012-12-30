//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/program.cpp                                $
// $Revision:: 23                                                             $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// 
//
// DESCRIPTION:
// 

#include "_pch_cpp.h"
#include "program.h"
#include "compiler.h"

inline void type_t::Archive( Archiver &arc )
{
	int i;

	Class::Archive( arc );

	ArchiveEnum( type, etype_t );

	if ( arc.Loading() )
	{
		bool onList;

		arc.ArchiveBool( &onList );

		if ( !onList )
		{
			def = new def_t;

			arc.ArchiveObject( ( Class * )def );
		}
		else
		{
			arc.ArchiveObjectPointer( ( Class ** )&def );
		}

	}
	else
	{
		arc.ArchiveBool( &def->_onDefList );

		if ( !def->_onDefList )
		{
			arc.ArchiveObject( ( Class * )def );
		}
		else
		{
			arc.ArchiveObjectPointer( ( Class ** )&def );
		}
	}

	/* if ( arc.Loading() )
		def = new def_t;

	arc.ArchiveObject( ( Class * )def ); */

	arc.ArchiveObjectPointer( ( Class ** )&def );

	arc.ArchiveObjectPointer( ( Class ** )&aux_type );

	arc.ArchiveInteger( &num_parms );
	arc.ArchiveInteger( &min_parms );

	for ( i = 0; i < num_parms; i++ )
		arc.ArchiveObjectPointer( ( Class ** )&parm_types[i] );
}


inline void def_t::Archive( Archiver &arc )
{
	Class::Archive( arc );

	arc.ArchiveObjectPointer( ( Class ** )&type );
	arc.ArchiveString( &name );
//	arc.ArchiveObjectPointer( ( Class ** )&next );
	arc.ArchiveInteger( &ofs );
	arc.ArchiveInteger( &localofs );
	arc.ArchiveObjectPointer( ( Class ** )&scope );
	arc.ArchiveInteger( &initialized );
	arc.ArchiveBool( &caseSensitive );

	arc.ArchiveBool( &_onDefList );
	//arc.ArchiveObjectPointer( ( Class ** )&type );
}

inline void dfunction_t::Archive( Archiver &arc )
{
	Class::Archive( arc );

	arc.ArchiveInteger( &eventnum );
	arc.ArchiveInteger( &first_statement );
	arc.ArchiveInteger( &parm_start );
	arc.ArchiveInteger( &parm_total );
	arc.ArchiveInteger( &locals );

	arc.ArchiveInteger( &profile );

	arc.ArchiveString( &s_name );
	arc.ArchiveString( &s_file );

	arc.ArchiveInteger( &numparms );
	arc.ArchiveInteger( &minparms );

	if ( arc.Loading() )
	{
		memset( &parm_size, 0, sizeof( parm_size[0] ) * MAX_PARMS );
		memset( &parm_type, 0, sizeof( parm_type[0] ) * MAX_PARMS );
	}

	arc.ArchiveRaw( parm_size, sizeof( parm_size[0] ) * numparms );
	arc.ArchiveRaw( parm_type, sizeof( parm_type[0] ) * numparms );
}

// These two pointers are dummy pointers for load/save games.  They MUST stay global because of how
//   loadgames change the pointer at a later time, hence if they are on the stack it will corrupt the stack.

//type_t *forceTypeSave;
//def_t *forceDefSave;

inline void Program::Archive( Archiver &arc )
{
	int i, num;
	type_t *curtype, *newtype;
	def_t *curdef, *newdef;

	Class::Archive( arc );

	/* // Force all of the defs to have indexes

	forceDefSave = &def_void;
	arc.ArchiveObjectPointer( ( Class ** )&forceDefSave );
	forceDefSave = &def_string;
	arc.ArchiveObjectPointer( ( Class ** )&forceDefSave );
	forceDefSave = &def_float;
	arc.ArchiveObjectPointer( ( Class ** )&forceDefSave );
	forceDefSave = &def_vector;
	arc.ArchiveObjectPointer( ( Class ** )&forceDefSave );
	forceDefSave = &def_entity;
	arc.ArchiveObjectPointer( ( Class ** )&forceDefSave );
	forceDefSave = &def_function;
	arc.ArchiveObjectPointer( ( Class ** )&forceDefSave );

	// Force all of the types to have indexes

	forceTypeSave = &type_void;
	arc.ArchiveObjectPointer( ( Class ** )&forceTypeSave );
	forceTypeSave = &type_string;
	arc.ArchiveObjectPointer( ( Class ** )&forceTypeSave );
	forceTypeSave = &type_float;
	arc.ArchiveObjectPointer( ( Class ** )&forceTypeSave );
	forceTypeSave = &type_vector;
	arc.ArchiveObjectPointer( ( Class ** )&forceTypeSave );
	forceTypeSave = &type_entity;
	arc.ArchiveObjectPointer( ( Class ** )&forceTypeSave );
	forceTypeSave = &type_function;
	arc.ArchiveObjectPointer( ( Class ** )&forceTypeSave ); */

	// NOTE: must archive global data for pointer fixups
	arc.ArchiveObject( &def_void );
	arc.ArchiveObject( &def_string );
	arc.ArchiveObject( &def_float );
	arc.ArchiveObject( &def_vector );
	arc.ArchiveObject( &def_entity );
	arc.ArchiveObject( &def_function );

	arc.ArchiveObject( &def_ret );
	arc.ArchiveObject( &junkdef );

	arc.ArchiveObject( &type_void );
	arc.ArchiveObject( &type_string );
	arc.ArchiveObject( &type_float );
	arc.ArchiveObject( &type_vector );
	arc.ArchiveObject( &type_entity );
	arc.ArchiveObject( &type_function );

	arc.ArchiveInteger( &numpr_globals );

	if ( arc.Loading() )
	{
		memset( pr_globals, 0, sizeof( pr_globals[0] ) * MAX_REGS );
	}

	arc.ArchiveRaw( pr_globals, sizeof( pr_globals[0] ) * numpr_globals );

	arc.ArchiveInteger( &locals_start );
	arc.ArchiveInteger( &locals_end );

	for ( i = 0; i < MAX_STRINGS; i++ ) 
	{
		arc.ArchiveBool( &strings[i].inuse );

		if ( strings[i].inuse )
		{
			arc.ArchiveString( &strings[i].s );
		}
		else
		{
			strings[i].s = "";
		}
	}

	arc.ArchiveInteger( &numstatements );
	arc.ArchiveRaw( statements, sizeof( statements[0] ) * numstatements );

	arc.ArchiveInteger( &numfunctions );
	for ( i = 0; i < numfunctions; i++ )
		arc.ArchiveObject( ( Class * )&functions[i] );

	// archive types
	if ( arc.Saving() ) 
	{
		for ( curtype = types, num = 0; curtype; curtype = curtype->next )
		{
			num++;
		}

		// Don't count type_function

		num--;
	}

	arc.ArchiveInteger( &num );

	if ( arc.Saving() ) 
	{
		for ( curtype = types; curtype; curtype = curtype->next )
		{
			// Skip type_function (we archive it seperately above)

			if ( curtype == &type_function )
				continue;

			arc.ArchiveObject( ( Class * )curtype );
		}
	}
	else 
	{
		curtype = types;

		for ( i = 0; i < num; i++ ) 
		{
			newtype = new type_t;
			arc.ArchiveObject( ( Class * )newtype );
			newtype->next = NULL;

			curtype->next = newtype;
			curtype = newtype;
		}
	}

	// archive defs
	if ( arc.Saving() ) {
		for ( curdef = def_head.next, num = 0; curdef; curdef = curdef->next )
			num++;
	}

	arc.ArchiveInteger( &num );

	if ( arc.Saving() ) {
		for ( curdef = def_head.next; curdef; curdef = curdef->next )
			arc.ArchiveObject( ( Class * )curdef );
	}
	else {
		def_tail = &def_head;
		curdef = def_tail;

		for ( i = 0; i < num; i++ ) {
			newdef = new def_t;
			arc.ArchiveObject( ( Class * )newdef );
			newdef->next = NULL;

			curdef->next = newdef;
			curdef = newdef;
		}
	}

	arc.ArchiveInteger( &pr_error_count );

	filenames.Archive( arc );
	arc.ArchiveString( &s_file );

	if ( arc.Loading() )
	{
		memset( pr_global_defs, 0, sizeof( pr_global_defs ) );
	}

	for ( i = 0; i < numpr_globals; i++ )
	{
		arc.ArchiveObjectPointer( ( Class ** )&pr_global_defs[i] );
	}
}

CLASS_DECLARATION( Class, dfunction_t, NULL )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( Class, type_t, NULL )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( Class, def_t, NULL )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( Class, Program, NULL )
{
	{ NULL, NULL }
};

// defined as a str so that it is only allocated once
const str complextypestring( "COMPLEX TYPE" );
const str emptystring;
const str resultstring( "<RESULT>" );
const str immediatestring( "IMMEDIATE" );

def_t::def_t()
{
	type        = NULL;
	next        = NULL;
	ofs         = 0;
	localofs    = 0;
	scope       = NULL;
	initialized = false;
	caseSensitive = true;
}

/*
============
FindType

Returns a preexisting complex type that matches the parm, or allocates
a new one and copies it out.
============
*/
type_t *Program::FindType( const type_t *type )
{
	def_t	   *def;
	type_t	*check;
	int		i;
	
	for( check = types; check != NULL; check = check->next )
	   {
		if ( ( check->type != type->type ) || ( check->aux_type != type->aux_type ) )
		{
			continue;
		}
		
		if ( check->min_parms == -1 )
		{
			// non-event functions
			if ( check->num_parms != type->num_parms )
            {
				continue;
            }
		}
		else
		{
			// event functions
			if ( ( check->min_parms != type->min_parms ) || ( check->num_parms != type->num_parms ) )
            {
				continue;
            }
		}
		
		for( i = 0; i < type->num_parms; i++ )
		{
			if ( check->parm_types[ i ] != type->parm_types[ i ] )
            {
				break;
            }
		}
		
		if ( i == type->num_parms )
		{
			return check;
		}
	   }
	
	// allocate a new one
	check = new type_t;	
	*check = *type;
	check->next = types;
	types = check;
	
	// allocate a generic def for the type, so fields can reference it
	def = new def_t;
	def->name = complextypestring;
	def->type = check;
	def->_onDefList = false;
	check->def = def;
	
	return check;
}

/*
============
GetDef

If type is NULL, it will match any type
If allocate is true, a new def will be allocated if it can't be found
============
*/
def_t *Program::GetDef( type_t *type, const char *name, def_t *scope, bool allocate, Lexer *lex )
{
	def_t	*def;
	char  element[ MAX_NAME ];
	
	// see if the name is already in use
	for( def = def_head.next; def; def = def->next )
	{
		if ( ( !def->caseSensitive && ( stricmp( def->name, name ) == 0 ) ) || ( strcmp( def->name, name ) == 0 ) )
		{
			if ( def->scope && ( def->scope != scope ) )
            {
				// in a different function
				continue;
            }
			
			if ( type && ( def->type != type ) && lex )
            {
				lex->ParseError( "Type mismatch on redeclaration of %s", name );
            }
			
			return def;
		}
	}
	
	if ( !allocate )
	{
		return NULL;
	}
	
	// allocate a new def
	def = new def_t;
	
	def->next = NULL;
	def_tail->next = def;
	def_tail = def;

	def->_onDefList = true;
	
	def->name = name;
	def->type = type;
	
	def->scope = scope;
	
	def->ofs = numpr_globals;
	if ( scope )
	{
		// since we don't know how many local variables there are,
		// we have to have them go backwards on the stack
		def->localofs = -( numpr_globals - locals_start ) - 1;
	}
	else
	{
		def->localofs = def->ofs;
	}
	
	pr_global_defs[ numpr_globals ] = def;
	
	//
	// make automatic defs for the vectors elements
	// .origin can be accessed as .origin_x, .origin_y, and .origin_z
	//
	if ( type->type == ev_vector )
	   {
		sprintf( element, "%s_x", name );
		GetDef( &type_float, element, scope, true, lex );
		
		sprintf( element, "%s_y", name );
		GetDef( &type_float, element, scope, true, lex );
		
		sprintf( element, "%s_z", name );
		GetDef( &type_float, element, scope, true, lex  );
	   }
	else
	{
		numpr_globals += type_size[ type->type ];
	}
	
	return def;
}

void Program::CreateDefForEvent( Event *ev )
{
	type_t	   newtype;
	type_t	   *type;
	const char  *name;
	def_t		   *def;
	EventArgDef *arg;
	int         num;
	int         i;
	dfunction_t	*df;
	qboolean    hitoptional;
	
	num = ev->getNumArgDefs();
	if ( num > MAX_PARMS )
	{
		gi.WDPrintf( "Event '%s' has too many arguments for function call.\n", ev->getName() );
		return;
	}
	
	if ( numfunctions >= MAX_FUNCTIONS )
	{
		gi.Error( ERR_DROP, "Exceeded max functions while declaring events." );
	}
	
	df = &functions[ numfunctions ];
	df->parm_total = 0;
	df->parm_start = numpr_globals;	
	
	memset( &newtype, 0, sizeof( newtype ) );
	newtype.type = ev_function;
	
	// set the return type
	arg = ev->getReturnType();
	if ( arg )
	{
		switch( arg->getType() )
		{
		case IS_STRING :
            newtype.aux_type = &type_string;
            break;
			
		case IS_VECTOR :
            newtype.aux_type = &type_vector;
            break;
			
		case IS_ENTITY :
            newtype.aux_type = &type_entity;
            break;
			
		case IS_BOOLEAN :
		case IS_INTEGER :
		case IS_FLOAT :
		default:
            newtype.aux_type = &type_float;
            break;
		}
	}
	else
	{
		newtype.aux_type = &type_void;
	}
	
	newtype.num_parms = 0;
	newtype.min_parms = 0;
	hitoptional = false;
	for( i = 1; i <= num; i++ )
	{
		arg = ev->getArgDef( i );
		
		if ( !hitoptional )
		{
			hitoptional = arg->isOptional();
			if ( !hitoptional )
            {
				newtype.min_parms++;
            }
		}
		
		switch( arg->getType() )
		{
		case IS_STRING :
            type = &type_string;
            break;
			
		case IS_VECTOR :
            type = &type_vector;
            break;
			
		case IS_ENTITY :
            type = &type_entity;
            break;
			
		case IS_BOOLEAN :
		case IS_INTEGER :
		case IS_FLOAT :
		default:
            type = &type_float;
            break;
		}
		
		df->parm_total += type_size[ type->type ];
		df->parm_size[ newtype.num_parms ] = type_size[ type->type ];
		df->parm_type[ newtype.num_parms ] = type->type;      
		newtype.parm_types[ newtype.num_parms ] = type;
		newtype.num_parms++;
	}
	
	type = FindType( &newtype );
	name = ev->getName();
	def = GetDef( type, name, NULL, true, NULL );
	def->initialized = 1;
	def->caseSensitive = false;
	
	setFunction( def->ofs, numfunctions );
	
	// fill in the dfunction
	df->eventnum         = int( *ev );
	df->first_statement  = -1;
	df->s_name           = def->name;
	df->s_file           = s_file;
	df->numparms         = def->type->num_parms;
	df->minparms         = def->type->min_parms;
	df->locals           = 0;
	
	numfunctions++;
}

void Program::CreateEventDefs( void )
{
	int num;
	int i;
	
	num = Event::NumEventDefs();
	for( i = 1; i <= num; i++ )
	{
		CreateDefForEvent( Event::GetEventDef( i ) );
	}
}

/*
============
CopyString

returns an offset from the string heap
============
*/
int Program::CopyString( const char *str )
{
	int idx;
	
	idx = AllocString();
	
	if ( str )
	{
		strings[ idx ].s = str;
	}
	else
	{
		strings[ idx ].s = emptystring;
	}
	
	return idx;
}

/*
============
AllocString

returns an unused string index
============
*/
int Program::AllocString()
{
	int i;

	for ( i = 0; i < MAX_STRINGS; i++ )
	{
		if ( !strings[i].inuse ) {
			strings[i].inuse = true;
			strings[i].s.CapLength( 0 );
			return i;
		}
	}

	gi.Error( ERR_DROP, "Program::GetFreeString : Too many strings allocated!\n" );
	return 0;
}

/*
============
FreeString
============
*/
void Program::FreeString( int idx )
{
	assert( ( idx > 0 ) && ( idx < MAX_STRINGS ) );
	assert( strings[idx].inuse );

	strings[idx].inuse = false;
	strings[idx].s.CapLength( 0 );
}

/*
============
CountStrings
============
*/
int Program::CountUsedStrings()
{
	int i, count = 0;

	for ( i = 0; i < MAX_STRINGS; i++ ) {
		if ( strings[i].inuse )
			count++;
	}

	return count;
}

/*
==============
BeginCompilation

called before compiling a batch of files, clears the pr struct
==============
*/
void Program::BeginCompilation( void )
{
	int i;
	
	numpr_globals = RESERVED_OFS;
	locals_end = numpr_globals;
	locals_start = numpr_globals;
	
	numstatements = 0;
	numfunctions = 1;
	
	for ( i = 0; i < MAX_STRINGS; i++ ) {
		strings[i].inuse = false;
		strings[i].s.CapLength( 0 );
	}
	strings[0].inuse = true;	// used as NULL
	strings[1].inuse = true;	// always used as the return string
	
	def_tail = &def_head;
	
	for( i = 0; i < RESERVED_OFS; i++ )
	{
		pr_global_defs[ i ] = &def_void;
	}
	
	// link the function type in so state forward declarations match proper type
	types = &type_function;
	type_function.next = NULL;
	
	pr_error_count = 0;
	
	// define any predefined objects
	GetDef( &type_void, "cam", NULL, true, NULL );
}

/*
==============
FinishCompilation

called after all files are compiled to check for errors
Returns false if errors were detected.
==============
*/
bool Program::FinishCompilation( void )
{
	def_t	*d;
	bool  errors;
	
	errors = false;
	
	// check to make sure all functions prototyped have code
	for( d = def_head.next; d; d = d->next )
	   {
		if ( ( d->type->type == ev_function ) && !d->scope )
		{
			// function parms are ok
			if (!d->initialized)
			{
				gi.WPrintf( "function %s was not defined\n", d->name.c_str() );
				errors = true;
			}
		}
	   }
	
	// also bail if there were ANY errors; the above block may be unneccessary with this in place.
	if( program.pr_error_count > 0 )
	{
		errors = true;
	}
	
	return !errors;
}

void Program::Compile( const char *filename )
{
	char *src;
	Compiler compiler( *this );
	int filenum;
	
	str oldfile( s_file );
	
	s_file = filename;
	if ( gi.FS_ReadFile( filename, ( void ** )&src, true ) < 0 )
	{
		s_file = oldfile;
		gi.WPrintf( "***\n***\n***\n*** Couldn't load %s\n***\n***\n***\n", filename );
		throw "Error";
	}
	
	filenum = filenames.AddObject( s_file );
	//if ( !compiler.CompileFile( src, filename, filenum ) )
	if ( !compiler.CompileFile( src, filenum ) )
	{
		s_file = oldfile;
		gi.FS_FreeFile( src );
		gi.WPrintf( "Compile failed.\n" );
		throw "Error";
   	}
	
	gi.FS_FreeFile( src );
	s_file = oldfile;
}

void Program::Load( const char *filename )
{
	FreeData();
	
	BeginCompilation();
	CreateEventDefs();
	
	try
	{
		Compile( filename );
	}
	
	catch( ... )
	{
	};
	
	if ( !FinishCompilation() )
	{
		gi.Error( ERR_DROP, "Compile failed." );
	}
}

func_t Program::findFunction( const char *name )
{
	int i;
	
	assert( name );
	
	for( i = 0; i < numfunctions; i++ )
	{
		if ( !functions[ i ].s_name.cmp( name ) )
		{
			return i;
		}
	}
	
	return -1;
}

void Program::FreeData( void )
{
	type_t   *curtype;
	type_t   *prevtype;
	def_t    *curdef;
	def_t    *prevdef;
	int      i;
	
	for( curtype = types; curtype != &type_function; curtype = prevtype )
	{
		prevtype = curtype->next;
		delete curtype->def;
		delete curtype;
	}
	
	// link the function type in so state forward declarations match proper type
	types = &type_function;
	type_function.next = NULL;
	
	for( curdef = def_head.next; curdef != NULL; curdef = prevdef )
	{
		prevdef = curdef->next;
		delete curdef;
	}
	
	def_head.next = NULL;
	def_tail = NULL;
	
	// cause all our strings to share their data
	for( i = 0; i < MAX_STRINGS; i++ ) {
		strings[ i ].inuse = false;
		strings[ i ].s = emptystring;
	}
	strings[0].inuse = true;	// always used as return string
	
	memset( pr_global_defs, 0, sizeof( pr_global_defs ) );
	memset( pr_globals, 0, sizeof( pr_globals ) );
	numpr_globals = 0;
	
	locals_start = 0;
	locals_end = 0;
	
	memset( statements, 0, sizeof( statements ) );
	numstatements = 0;
	
	for( i = 0; i < MAX_FUNCTIONS; i++ )
	{
		functions[ i ].eventnum = 0;
		functions[ i ].first_statement = 0;
		functions[ i ].parm_start = 0;
		functions[ i ].parm_total = 0;
		functions[ i ].locals = 0;
		functions[ i ].profile = 0;
		
		functions[ i ].s_name = emptystring;
		functions[ i ].s_file = emptystring;
		
		functions[ i ].numparms = 0;
		functions[ i ].minparms = 0;
		
		memset( functions[ i ].parm_size, 0, sizeof( functions[ i ].parm_size ) );
		memset( functions[ i ].parm_type, 0, sizeof( functions[ i ].parm_type ) );
	}
	
	numfunctions = 1;
	
	pr_error_count = 0;
	s_file = emptystring;
}

Program::Program()
{
	int i;
	str tempstring( "temp" );
	
	def_void.type = &type_void;
	def_void.name = tempstring;
	
	def_string.type = &type_string;
	def_string.name = tempstring;
	
	def_float.type = &type_float;
	def_float.name = tempstring;
	
	def_vector.type = &type_vector;
	def_vector.name = tempstring;
	
	def_entity.type = &type_entity;
	def_entity.name = tempstring;
	
	def_function.type = &type_function;
	def_function.name = tempstring;
	
	def_tail = NULL;
	
	// link the function type in so state forward declarations match proper type
	types = &type_function;
	type_function.next = NULL;
	
	// cause all our strings to share their data
	for( i = 0; i < MAX_STRINGS; i++ ) {
		strings[ i ].inuse = false;
		strings[ i ].s = "";
	}
	strings[0].inuse = true;	// always used as return string
	
	filenames.FreeObjectList();
	
	numpr_globals = 0;
	numstatements = 0;
	numfunctions  = 1;
	
	locals_start = 0;
	locals_end   = 0;
	
	pr_error_count = 0;
}

Program::~Program()
{
	FreeData();
}

// Stuff From .h File
float Program::getFloat( int offset )
{
	return pr_globals[ offset ];
}

int Program::getInteger( int offset )
{
	return *( int * )&pr_globals[ offset ];
}

float *Program::getVector( int offset )
{
	return &pr_globals[ offset ];
}

const char *Program::getString( int offset )
{
	return strings[ *( int * )&pr_globals[ offset ] ].s.c_str();
	//   return ( ( str * )&pr_globals[ offset ] ] ] )->c_str();
	//   return ( ( str * )( &pr_globals[ offset ] ) )->getString();
}

func_t Program::getFunction( int offset )
{
	return *( func_t * )&pr_globals[ offset ];
}

Entity *Program::getEntity( int offset )
{
	Entity *ent;
	int entnum;
	
	entnum = *( int * )&pr_globals[ offset ];
	
	if ( entnum > 0 )
	{
		ent = G_GetEntity( entnum - 1 );
		return ent;
	}
	
	return NULL;
}

TargetList *Program::getTargetList( int offset )
{
	int entnum;
	
	entnum = *( int * )&pr_globals[ offset ];
	
	if ( entnum < 0 )
	{
		return world->GetTargetList( -entnum );
	}
	
	return NULL;
}

gentity_t *Program::getEdict( int offset )
{
	int entnum;
	Entity *ent;
	
	entnum = *( int * )&pr_globals[ offset ];
	
	if ( entnum > 0 )
	{
		ent = G_GetEntity( entnum - 1 );
		if ( ent )
		{
			return ent->edict;
		}
	}
	else if ( entnum < 0 )
	{
	}
	
	return NULL;
}

void Program::setFunction( int offset, func_t func )
{
	*( func_t * )&pr_globals[ offset ] = func;
}

void Program::setEntity( int offset, const Entity *ent )
{
	if ( ent )
	{
		*( int * )&pr_globals[ offset ] = ent->entnum + 1;
	}
	else
	{
		*( int * )&pr_globals[ offset ] = 0;
	}
}

void Program::setTargetList( int offset, const TargetList *list )
{
	if ( list )
	{
		*( int * )&pr_globals[ offset ] = -list->index;
	}
	else
	{
		*( int * )&pr_globals[ offset ] = 0;
	}
}

void Program::setString( int offset, const char *text )
{
	strings[ offset ].s = text;
	*( int * )&pr_globals[ offset ] = offset;
}

void Program::setFloat( int offset, float value )
{
	pr_globals[ offset ] = value;
}

void Program::setInteger( int offset, int value )
{
	*( int * )&pr_globals[ offset ] = value;
}

void Program::setVector( int offset, const Vector &vec )
{
	*( Vector * )&pr_globals[ offset ] = vec;
}

const char *Program::GetFilename( int num )
{
	return filenames.ObjectAt( num );
}
