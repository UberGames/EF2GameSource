//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/compiler.cpp                                   $
// $Revision:: 10                                                             $
//     $Date:: 10/10/02 3:05p                                                 $
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
#include "compiler.h"

opcode_t pr_opcodes[] =
{
	{ "<DONE>",			"DONE",			-1, false,	&def_entity,	&def_entity,	&def_void },
		
	{ "++",				"UINC_F",		1,	true,	&def_float,		&def_void,		&def_void },
	{ "--",				"UDEC_F",		1,	true,	&def_float,		&def_void,		&def_void },
	
	{ "*",				"MUL_F",		3,	false,	&def_float,		&def_float,		&def_float },
	{ "*",				"MUL_V",		3,	false,	&def_vector,	&def_vector,	&def_float },
	{ "*",				"MUL_FV",		3,	false,	&def_float,		&def_vector,	&def_vector },
	{ "*",				"MUL_VF",		3,	false,	&def_vector,	&def_float,		&def_vector },
	
	{ "/",				"DIV",			3,	false,	&def_float,		&def_float,		&def_float },
	
	{ "+",				"ADD_F",		4,	false,	&def_float,		&def_float,		&def_float },
	{ "+",				"ADD_V",		4,	false,	&def_vector,	&def_vector,	&def_vector },
	{ "+",				"ADD_S",		4,	false,	&def_string,	&def_string,	&def_string },
	{ "+",				"ADD_FS",		4,	false,	&def_float,		&def_string,	&def_string },
	{ "+",				"ADD_SF",		4,	false,	&def_string,	&def_float,		&def_string },
	{ "+",				"ADD_VS",		4,	false,	&def_vector,	&def_string,	&def_string },
	{ "+",				"ADD_SV",		4,	false,	&def_string,	&def_vector,	&def_string },
	
	{ "-",				"SUB_F",		4,	false,	&def_float,		&def_float,		&def_float },
	{ "-",				"SUB_V",		4,	false,	&def_vector,	&def_vector,	&def_vector },
	
	{ "==",				"EQ_F",			5,	false,	&def_float,		&def_float,		&def_float },
	{ "==",				"EQ_V",			5,	false,	&def_vector,	&def_vector,	&def_float },
	{ "==",				"EQ_S",			5,	false,	&def_string,	&def_string,	&def_float },
	{ "==",				"EQ_E",			5,	false,	&def_entity,	&def_entity,	&def_float },
	{ "==",				"EQ_FNC",		5,	false,	&def_function,	&def_function,	&def_float },
	
	{ "!=",				"NE_F",			5,	false,	&def_float,		&def_float,		&def_float },
	{ "!=",				"NE_V",			5,	false,	&def_vector,	&def_vector,	&def_float },
	{ "!=",				"NE_S",			5,	false,	&def_string,	&def_string,	&def_float },
	{ "!=",				"NE_E",			5,	false,	&def_entity,	&def_entity,	&def_float },
	{ "!=",				"NE_FNC",		5,	false,	&def_function,	&def_function,	&def_float },
	
	{ "<=",				"LE",			5,	false,	&def_float,		&def_float,		&def_float },
	{ ">=",				"GE",			5,	false,	&def_float,		&def_float,		&def_float },
	{ "<",				"LT",			5,	false,	&def_float,		&def_float,		&def_float },
	{ ">",				"GT",			5,	false,	&def_float,		&def_float,		&def_float },
	
	{ "=",				"STORE_F",		6,	true,	&def_float,		&def_float,		&def_float },
	{ "=",				"STORE_V",		6,	true,	&def_vector,	&def_vector,	&def_vector },
	{ "=",				"STORE_S",		6,	true,	&def_string,	&def_string,	&def_string },
	{ "=",				"STORE_ENT",	6,	true,	&def_entity,	&def_entity,	&def_entity },
	{ "=",				"STORE_FNC",	6,	true,	&def_function,	&def_function,	&def_function },
	
	{ "=",				"STORE_FTOS",	6,	true,	&def_string,	&def_float,		&def_string },
	
	{ "*=",				"UMUL_F",		6,	true,	&def_float,		&def_float,		&def_void },
	{ "/=",				"UDIV_F",		6,	true,	&def_float,		&def_float,		&def_void },
	{ "+=",				"UADD_F",		6,	true,	&def_float,		&def_float,		&def_void },
	{ "-=",				"USUB_F",		6,	true,	&def_float,		&def_float,		&def_void },
	{ "&=",				"UAND_F",		6,	true,	&def_float,		&def_float,		&def_void },
	{ "|=",				"UOR_F",		6,	true,	&def_float,		&def_float,		&def_void },
	
	{ "<RETURN>",		"RETURN",		-1, false,	&def_void,		&def_void,		&def_void },
	
	{ "!",				"NOT_F",		-1, false,	&def_float,		&def_void,		&def_float },
	{ "!",				"NOT_V",		-1, false,	&def_vector,	&def_void,		&def_float },
	{ "!",				"NOT_S",		-1, false,	&def_vector,	&def_void,		&def_float },
	{ "!",				"NOT_ENT",		-1, false,	&def_entity,	&def_void,		&def_float },
	{ "!",				"NOT_FNC",		-1, false,	&def_function,	&def_void,		&def_float },
	
	{ "<IF>",			"IF",			-1, false,	&def_float,		&def_float,		&def_void },
	{ "<IFNOT>",		"IFNOT",		-1, false,	&def_float,		&def_float,		&def_void },
	
	// calls returns REG_RETURN
	{ "<CALL>",			"CALL",			-1, false,	&def_function,	&def_void,		&def_void },
	{ "<OCALL>",		"OCALL",		-1, false,	&def_function,	&def_entity,	&def_void },
	{ "<THREAD>",		"THREAD",		-1, false,	&def_function,	&def_void,		&def_void },
	
	{ "<PUSH_F>",		"PUSH_F",		-1, false,	&def_float,		&def_void,		&def_void },
	{ "<PUSH_V>",		"PUSH_V",		-1, false,	&def_vector,	&def_void,		&def_void },
	{ "<PUSH_S>",		"PUSH_S",		-1, false,	&def_string,	&def_void,		&def_void },
	{ "<PUSH_ENT>",		"PUSH_ENT",		-1, false,	&def_entity,	&def_void,		&def_void },
	{ "<PUSH_FNC>",		"PUSH_FNC",		-1, false,	&def_function,	&def_void,		&def_void },
	{ "<PUSH_FTOS>",	"PUSH_FTOS",	-1, false,	&def_string,	&def_void,		&def_void },
	
	{ "<GOTO>",			"GOTO",			-1, false,	&def_float,		&def_void,		&def_void },
	
	{ "&&",				"AND",			7,	false,	&def_float,		&def_float,		&def_float },
	{ "||",				"OR",			7,	false,	&def_float,		&def_float,		&def_float },
	
	{ "&",				"BITAND",		3,	false,	&def_float,		&def_float,		&def_float },
	{ "|",				"BITOR",		3,	false,	&def_float,		&def_float,		&def_float },
	
	{ NULL }
};

def_t	def_ret;
def_t	junkdef;

#define  FUNCTION_PRIORITY 2
#define	TOP_PRIORITY	   7
#define	NOT_PRIORITY	   5

Compiler::Compiler( Program &prg ) : 
	program( prg ), lex( prg )

{
	pr_scope = NULL;
	callthread = false;
	filenumber = 0;
}

/*
============
Statement

Emits a primitive statement, returning the var it places it's value in
============
*/

def_t *Compiler::Statement( const opcode_t *op, def_t *var_a, const def_t *var_b )
{
	dstatement_t	*statement;
	def_t			   *var_c;
	
	if ( program.numstatements >= MAX_STATEMENTS )
	{
		lex.ParseError( "Exceeded max statements." );
	}
	
	statement = &program.statements[ program.numstatements ];
	statement->linenumber = lex.SourceLine();
	statement->file = filenumber;
	program.numstatements++;
	
	statement->op = op - pr_opcodes;
	statement->a = var_a ? var_a->localofs : 0;
	statement->b = var_b ? var_b->localofs : 0;
	if ( ( op->type_c == &def_void ) || op->right_associative )
	   {
		// ifs, gotos, and assignments don't need vars allocated
		var_c = NULL;
		statement->c = 0;
	   }
	else
	   {
		// allocate result space
		var_c = new def_t;
		
		var_c->ofs        = program.numpr_globals;
		var_c->localofs   = var_c->ofs;
		var_c->type       = op->type_c->type;
		var_c->name       = resultstring;
		
		program.def_tail->next = var_c;
		program.def_tail = var_c;
		
		statement->c = program.numpr_globals;
		program.pr_global_defs[ program.numpr_globals ] = var_c;
		
		if ( var_c->type == &type_string )
		{
			eval_t temp;
			
			VectorClear( temp.vector );
			temp.string = program.CopyString( NULL );
			memcpy( program.pr_globals + var_c->ofs, &temp, 4 * type_size[ var_c->type->type ] );
		}
		
		program.numpr_globals += type_size[ op->type_c->type->type ];
	   }
	
	if ( op->right_associative )
	{
		return var_a;
	}
	
	return var_c;
}

/*
============
ParseImmediate

Looks for a preexisting constant
============
*/
def_t	*Compiler::ParseImmediate( void )
{
	def_t	*cn;
	
	// check for a constant with the same value
	for( cn = program.def_head.next; cn; cn = cn->next )
	   {
		if ( cn->initialized != 2 )
		{
			continue;
		}
		
		if ( cn->type != lex.pr_immediate_type )
		{
			continue;
		}
		
		if ( lex.pr_immediate_type == &type_entity )
		{
			if ( program.getInteger( cn->ofs ) == lex.pr_immediate.entity )
			{
				lex.Lex();
				return cn;
			}
		}
		else if ( lex.pr_immediate_type == &type_string )
		{
			if ( !strcmp( program.getString( cn->ofs ), lex.pr_immediate_string ) )
			{
				lex.Lex();
				return cn;
			}
		}
		else if ( lex.pr_immediate_type == &type_float )
		{
			if ( program.getFloat( cn->ofs ) == lex.pr_immediate._float )
			{
				lex.Lex();
				
				return cn;
			}
		}
		else if ( lex.pr_immediate_type == &type_vector )
		{
			if ( ( program.getFloat( cn->ofs ) == lex.pr_immediate.vector[ 0 ] ) && 
				( program.getFloat( cn->ofs + 1 ) == lex.pr_immediate.vector[ 1 ] ) && 
				( program.getFloat( cn->ofs + 2 ) == lex.pr_immediate.vector[ 2 ] ) )
			{
				lex.Lex();
				
				return cn;
			}
		}
		else
		{
			lex.ParseError( "weird immediate type" );
		}
	   }
	
	// allocate a new one
	cn = new def_t;
	cn->next = NULL;
	program.def_tail->next = cn;
	program.def_tail = cn;
	cn->type = lex.pr_immediate_type;
	cn->name = immediatestring;
	cn->initialized = 2;
	cn->scope = NULL;		// always share immediates
	
	// copy the immediate to the global area
	cn->ofs = program.numpr_globals;
	cn->localofs = cn->ofs;
	program.pr_global_defs[ cn->ofs ] = cn;
	program.numpr_globals += type_size[ lex.pr_immediate_type->type ];
	if ( lex.pr_immediate_type == &type_string )
	{
		lex.pr_immediate.string = program.CopyString( lex.pr_immediate_string );
	}
	
	memcpy( program.pr_globals + cn->ofs, &lex.pr_immediate, 4 * type_size[ lex.pr_immediate_type->type ] );
	
	lex.Lex();
	
	return cn;
}

/*
============
ParseFunctionCall
============
*/
def_t *Compiler::ParseFunctionCall( def_t *func )
{
	def_t		*e;
	int		arg;
	type_t	*t;
	opcode_t	*op;
	dstatement_t *patch;
	int      size;
	bool callThreadForThisFunction;
	
	callThreadForThisFunction = callthread;
	
	callthread = false;
	
	t = func->type;
	
	if ( t->type != ev_function )
	{
		lex.ParseError( "not a function" );
	}
	
	// copy the arguments to the global parameter variables
	arg = 0;
	size = 0;
	if ( !lex.Check( ")" ) )
	   {
		do
		{
			if ( ( t->num_parms != -1 ) && ( arg >= t->num_parms ) )
            {
				lex.ParseError( "too many parameters" );
            }
			
			e = Expression( TOP_PRIORITY );
			
			if ( ( t->num_parms != -1 ) && ( e->type != t->parm_types[ arg ] ) )
            {
				// special case for when we're storing a float in a string
				if ( ( t->parm_types[ arg ]->type != ev_string ) ||
					( e->type->type != ev_float ) )
				{
					lex.ParseError( "type mismatch on parm %i", arg + 1 );
				}
            }
			
			switch( e->type->type )
            {
            case ev_string :
				op = &pr_opcodes[ OP_PUSH_S ];
				
				break;
				
            case ev_float :
				if ( t->parm_types[ arg ]->type == ev_string )
				{
					op = &pr_opcodes[ OP_PUSH_FTOS ];
				}
				else
				{
					op = &pr_opcodes[ OP_PUSH_F ];
				}
				break;
				
            case ev_vector :
				op = &pr_opcodes[ OP_PUSH_V ];
				break;
				
            case ev_entity :
				op = &pr_opcodes[ OP_PUSH_ENT ];
				break;
				
            case ev_function :
				op = &pr_opcodes[ OP_PUSH_FNC ];
				break;
				
            default:
				op = NULL; // shut up compiler
				lex.ParseError( "No appropriate operator for storing type '%s'", e->type->def->name.c_str() );
				break;
            }
			
			size += type_size[ e->type->type ];
			
			Statement( op, e, 0 );
			
			arg++;
		}
		while( lex.Check( "," ) );
		
		if ( ( t->num_parms != -1 ) && ( arg < t->min_parms ) )
		{
			lex.ParseError( "too few parameters" );
		}
		
		lex.Expect( ")" );
	   }
	
	if ( ( t->num_parms != -1 ) && ( arg < t->min_parms ) )
	{
		lex.ParseError( "too few parameters" );
	}
	
	if ( arg > MAX_PARMS )
	{
		lex.ParseError( "More than %d parameters", MAX_PARMS );
	}
	
	patch = &program.statements[ program.numstatements ];
	if ( callThreadForThisFunction )
	{
		if ( func->initialized && program.functions[ program.getFunction( func->ofs ) ].eventnum )
		{
			lex.ParseError( "Built-in functions cannot be called as threads" );
		}
		Statement( &pr_opcodes[ OP_THREAD ], func, 0 );
		callthread = false;
		
		// threads return the thread number
		def_ret.type = &type_float;
	}
	else
	{
		Statement( &pr_opcodes[ OP_CALL ], func, 0 );
		def_ret.type = t->aux_type;
	}
	patch->b = size;
	
	return &def_ret;
}

/*
============
ParseObjectCall
============
*/
def_t *Compiler::ParseObjectCall( def_t *func, def_t *object )
{
	def_t		*e;
	int		arg;
	type_t	*t;
	opcode_t	*op;
	dstatement_t *patch;
	int      size;
	
	t = func->type;
	
	if ( t->type != ev_function )
	{
		lex.ParseError( "not a function" );
	}
	
	lex.Check( "(" );
	
	// copy the arguments to the global parameter variables
	arg = 0;
	size = 0;
	if ( !lex.Check( ")" ) )
	   {
		do
		{
			if ( ( t->num_parms != -1 ) && ( arg >= t->num_parms ) )
            {
				lex.ParseError( "too many parameters" );
            }
			
			e = Expression( TOP_PRIORITY );
			
			if ( ( t->num_parms != -1 ) && ( e->type != t->parm_types[ arg ] ) )
            {
				// special case for when we're storing a float in a string
				if ( ( t->parm_types[ arg ]->type != ev_string ) ||
					( e->type->type != ev_float ) )
				{
					lex.ParseError( "type mismatch on parm %i", arg + 1 );
				}
            }
			
			switch( e->type->type )
            {
            case ev_string :
				op = &pr_opcodes[ OP_PUSH_S ];
				
				break;
				
            case ev_float :
				if ( t->parm_types[ arg ]->type == ev_string )
				{
					op = &pr_opcodes[ OP_PUSH_FTOS ];
				}
				else
				{
					op = &pr_opcodes[ OP_PUSH_F ];
				}
				break;
				
            case ev_vector :
				op = &pr_opcodes[ OP_PUSH_V ];
				break;
				
            case ev_entity :
				op = &pr_opcodes[ OP_PUSH_ENT ];
				break;
				
            case ev_function :
				op = &pr_opcodes[ OP_PUSH_FNC ];
				break;
				
            default:
				op = NULL; // shut up compiler
				lex.ParseError( "No appropriate operator for storing type '%s'", e->type->def->name.c_str() );
				break;
            }
			
			size += type_size[ e->type->type ];
			
			Statement( op, e, 0 );
			
			arg++;
		}
		while( lex.Check( "," ) );
		
		if ( ( t->num_parms != -1 ) && ( arg < t->min_parms ) )
		{
			lex.ParseError( "too few parameters" );
		}
		
		lex.Expect( ")" );
	   }
	
	if ( ( t->num_parms != -1 ) && ( arg < t->min_parms ) )
	{
		lex.ParseError( "too few parameters" );
	}
	
	if ( arg > MAX_PARMS )
	{
		lex.ParseError( "More than %d parameters", MAX_PARMS );
	}
	
	patch = &program.statements[ program.numstatements ];
	Statement( &pr_opcodes[ OP_OCALL ], func, object );
	patch->c = size;
	
	def_ret.type = t->aux_type;
	
	return &def_ret;
}

/*
============
ParseValue

Returns the global ofs for the current token
============
*/
def_t	*Compiler::ParseValue( void )
{
	def_t		*d;
	char		*name;
	
	// if the token is an immediate, allocate a constant for it
	if ( lex.pr_token_type == tt_immediate )
	{
		return ParseImmediate();
	}
	
	if ( lex.Check( "thread" ) )
	{
		callthread = true;
	}
	
	name = lex.ParseName();
	
	// look through the defs
	d = program.GetDef( NULL, name, pr_scope, false, &lex );
	if ( !d )
	{
		lex.ParseError( "Unknown value \"%s\"", name );
	}
	
	return d;
}

/*
============
Term
============
*/
def_t *Compiler::Term( void )
{
	def_t	   *e;
	int      op;
	etype_t	t;
	
	if ( lex.Check( "!" ) )
	{
		e = Expression( NOT_PRIORITY );
		t = e->type->type;
		switch( t )
		{
		case ev_float :
			op = OP_NOT_F;
            break;
			
		case ev_string :
            op = OP_NOT_S;
            break;
			
		case ev_entity :
			op = OP_NOT_ENT;
            break;
			
		case ev_vector :
            op = OP_NOT_V;
            break;
			
		case ev_function :
            op = OP_NOT_FNC;
            break;
			
		default :
			lex.ParseError( "type mismatch for !" );
			
            // shut up compiler
            op = OP_NOT_FNC;
            break;
		}
		
		return Statement( &pr_opcodes[ op ], e, 0 );
	}
	
	if ( lex.Check( "(" ) )
	{
		e = Expression( TOP_PRIORITY );
		lex.Expect( ")" );
		
		return e;
	}
	
	return ParseValue();
}

/*
==============
Expression
==============
*/
def_t *Compiler::Expression( int priority )
{
	opcode_t	*op;
	opcode_t	*oldop;
	def_t		*e;
	def_t		*e2;
	etype_t	type_a;
	etype_t	type_b;
	char		*name;
	
	if ( priority == 0 )
	{
		return Term();
	}
	
	e = Expression( priority - 1 );
	
	while( 1 )
	   {
		if ( priority == FUNCTION_PRIORITY )
		{
			if ( lex.Check( "(" ) )
            {
				return ParseFunctionCall( e );
            }
			else if ( ( ( e->type->type == ev_entity ) || 
				( ( e->type->type == ev_void ) && ( e->ofs < OFS_END ) ) ) && 
				( lex.Check( "." ) ) )
            {
				if ( lex.pr_token_type != tt_name )
				{
					lex.ParseError( "Expecting function call" );
				}
				
				name = lex.ParseName();
				
				// look through the defs
				e2 = program.GetDef( NULL, name, pr_scope, false, &lex );
				if ( !e2 )
				{
					lex.ParseError( "Unknown value \"%s\"", name );
				}
				else if ( e2->type->type != ev_function )
				{
					lex.ParseError( "\"%s\" is not a valid function name", name );
				}
				
				return ParseObjectCall( e2, e );
            }
			
			//else if ( lex.Check( ";" ) )
			//{
			//	lex.ParseError( "Invalid command - %s", e->name.c_str() );
			//}
		}
		
		for( op = pr_opcodes; op->name; op++ )
		{
			if ( op->priority != priority )
            {
				continue;
            }
			
			if ( !lex.Check( op->name ) )
            {
				continue;
            }
			
			if ( op->type_b == &def_void )
            {
				e = Statement( op, e, 0 );
				return e;
            }
			
			if ( op->right_associative )
			{
				e2 = Expression( priority );
			}
			else
            {
				e2 = Expression( priority - 1 );
            }
			
			// type check
			type_a = e->type->type;
			type_b = e2->type->type;
			
			oldop = op;
			while( ( type_a != op->type_a->type->type ) ||
				( type_b != op->type_b->type->type ) )
			{
				op++;
				if ( !op->name || strcmp( op->name, oldop->name ) )
				{
					lex.ParseError( "type mismatch for %s", oldop->name );
				}
			}
			
			if ( op->right_associative )
            {
				e = Statement( op, e2, e );
            }
			else
            {
				e = Statement( op, e, e2 );
            }
			break;
		}
		
		if ( !op->name )
		{
			// next token isn't at this priority level
			break;
		}
	}
	
	return e;
}

/*
============
ParseStatement

============
*/
void Compiler::ParseStatement( void )
{
	def_t				*e;
	dstatement_t	*patch1;
	dstatement_t	*patch2;
	dstatement_t	*patch3;
	dstatement_t	*patch4;
	etype_t	      type_a;
	etype_t	      type_b;
	opcode_t	      *op;
	
	if ( lex.Check( "{" ) )
	{
		do
		{
			ParseStatement();
		}
		while( !lex.Check( "}" ) );
		
		return;
	}
	
	if ( lex.Check( "return" ) )
	{
		if ( lex.Check( ";" ) )
		{
			if ( pr_scope->type->aux_type->type != ev_void )
            {
				lex.ParseError( "expecting return value" );
            }
			
			Statement( &pr_opcodes[ OP_RETURN ], 0, 0 );
			return;
		}
		
		e = Expression( TOP_PRIORITY );
		lex.Expect( ";" );
		
		type_a = e->type->type;
		type_b = pr_scope->type->aux_type->type;
		
		if ( type_a == type_b )
		{
			Statement( &pr_opcodes[ OP_RETURN ], e, 0 );
			
			return;
		}
		
		for( op = pr_opcodes; op->name; op++ )
		{
			if ( !strcmp( op->name, "=" ) )
            {
				break;
            }
		}
		
		assert( op->name );
		
		while( ( type_a != op->type_a->type->type ) ||
			( type_b != op->type_b->type->type ) )
		{
			op++;
			if ( !op->name || strcmp( op->name, "=" ) )
            {
				lex.ParseError( "type mismatch for return value" );
            }
		}
		
		def_ret.type = pr_scope->type->aux_type;
		Statement( op, e, &def_ret );
		Statement( &pr_opcodes[ OP_RETURN ], 0, 0 );
		
		return;
	}
	
	if ( lex.Check( "while" ) )
	{
		lex.Expect( "(" );
		
		patch2 = &program.statements[ program.numstatements ];
		e = Expression( TOP_PRIORITY );
		lex.Expect( ")" );
		patch1 = &program.statements[ program.numstatements ];
		
		Statement( &pr_opcodes[ OP_IFNOT ], e, 0 );
		ParseStatement();
		
		junkdef.ofs = patch2 - &program.statements[ program.numstatements ];
		junkdef.localofs = junkdef.ofs;
		Statement( &pr_opcodes[ OP_GOTO ], &junkdef, 0 );
		patch1->b = &program.statements[ program.numstatements ] - patch1;
		
		return;
	}
	
	if ( lex.Check( "for" ) )
	{
		lex.Expect( "(" );
		
		// init
		if ( !lex.Check( ";" ) )
		{
			do
            {
				Expression( TOP_PRIORITY );
            }
			while( lex.Check( "," ) );
			
			lex.Expect( ";" );
		}
		
		// condition
		patch2 = &program.statements[ program.numstatements ];
		
		e = Expression( TOP_PRIORITY );
		lex.Expect( ";" );
		
		patch1 = &program.statements[ program.numstatements ];
		Statement( &pr_opcodes[ OP_IFNOT ], e, 0 );
		
		// counter
		if ( !lex.Check( ")" ) )
		{
			patch3 = &program.statements[ program.numstatements ];
			Statement( &pr_opcodes[ OP_IF ], e, 0 );
			
			patch4 = patch2;
			patch2 = &program.statements[ program.numstatements ];
			do
            {
				Expression( TOP_PRIORITY );
            }
			while( lex.Check( "," ) );
			lex.Expect( ")" );
			
			// goto patch4
			junkdef.ofs = patch4 - &program.statements[ program.numstatements ];
			junkdef.localofs = junkdef.ofs;
			Statement( &pr_opcodes[ OP_GOTO ], &junkdef, 0 );
			
			// fixup patch3
			patch3->b = &program.statements[ program.numstatements ] - patch3;
		}
		
		ParseStatement();
		
		// goto patch2
		junkdef.ofs = patch2 - &program.statements[ program.numstatements ];
		junkdef.localofs = junkdef.ofs;
		Statement( &pr_opcodes[ OP_GOTO ], &junkdef, 0 );
		
		// fixup patch1
		patch1->b = &program.statements[ program.numstatements ] - patch1;
		
		return;
	}
	
	if ( lex.Check( "do" ) )
	{
		patch1 = &program.statements[ program.numstatements ];
		ParseStatement();
		lex.Expect( "while" );
		lex.Expect( "(" );
		e = Expression( TOP_PRIORITY );
		lex.Expect( ")" );
		lex.Expect( ";" );
		
		junkdef.ofs = patch1 - &program.statements[ program.numstatements ];
		junkdef.localofs = junkdef.ofs;
		Statement( &pr_opcodes[ OP_IF ], e, &junkdef );
		return;
	}
	
	if ( lex.CheckType() != NULL )
	{
		ParseDefs();
		program.locals_end = program.numpr_globals;
		
		return;
	}
	
	if ( lex.Check( "if" ) )
	{
		lex.Expect( "(" );
		e = Expression( TOP_PRIORITY );
		lex.Expect( ")" );
		
		patch1 = &program.statements[ program.numstatements ];
		Statement( &pr_opcodes[ OP_IFNOT ], e, 0 );
		
		ParseStatement();
		
		if ( lex.Check( "else" ) )
		{
			patch2 = &program.statements[ program.numstatements ];
			Statement( &pr_opcodes[ OP_GOTO ], 0, 0 );
			patch1->b = &program.statements[ program.numstatements ] - patch1;
			ParseStatement();
			patch2->a = &program.statements[ program.numstatements ] - patch2;
		}
		else
		{
			patch1->b = &program.statements[ program.numstatements ] - patch1;
		}
		
		return;
	}
	
	Expression( TOP_PRIORITY );
	lex.Expect(";");
}

/*
============
ParseImmediateStatements

Parse a function body
============
*/
function_t Compiler::ParseImmediateStatements( const type_t *type, const char parm_names[ MAX_PARMS ][ MAX_NAME ], 
		int numparms )
{
	int			i;
	function_t	f;
	def_t		   *defs[ MAX_PARMS ];
	
	//
	// define the parms
	//
	for( i = 0; i < numparms; i++ )
	{
		defs[ i ] = program.GetDef( type->parm_types[ i ], parm_names[ i ], pr_scope, true, &lex );
		
		f.parm_ofs[ i ] = defs[ i ]->ofs;
		if ( ( i > 0 ) && ( f.parm_ofs[ i ] < f.parm_ofs[ i - 1 ] ) )
		{
			gi.Error( ERR_DROP, "bad parm order" );
		}
	}
	
	f.code = program.numstatements;
	
	//
	// parse regular statements
	//
	// we've already passed the {
	//lex.Expect( "{" );
	
	while( !lex.Check( "}" ) )
	{
		ParseStatement();
	}
	
	// emit an end of statements opcode
	Statement( pr_opcodes, 0, 0 );
	
	return f;
}

/*
============
ParseFunction

Parses a function type
============
*/
type_t *Compiler::ParseFunction( type_t *returntype, char parm_names[ MAX_PARMS ][ MAX_NAME ], int *num_parms )
{
	type_t	newtype;
	type_t	*type;
	char	   *name;
   	
	// function type
	memset( &newtype, 0, sizeof( newtype ) );
	
	newtype.type       = ev_function;
	newtype.aux_type   = returntype;	      // return type
	newtype.num_parms  = 0;
	newtype.min_parms  = -1;
	
	if ( !lex.Check( ")" ) )
	{
		if ( lex.Check( "..." ) )
		{
			newtype.num_parms = -1;  	// variable args
		}
		else
		{
			do
			{
				if ( newtype.num_parms >= MAX_PARMS )
				{
					lex.ParseError( "Too many arguments for function call." );
				}
				
				type = lex.ParseType();
				name = lex.ParseName();
				strcpy( parm_names[ newtype.num_parms ], name );
				newtype.parm_types[ newtype.num_parms ] = type;
				newtype.num_parms++;
			}
			while( lex.Check( "," ) );
		}
		
		lex.Expect( ")" );
	}
	
	if ( num_parms )
	{
		*num_parms = newtype.num_parms;
	}
	
	return program.FindType( &newtype );
}

void Compiler::ParseFunctionDef( type_t *type, const char *name )
{
	def_t		   *def;
	function_t	f;
	dfunction_t	*df;
	int			i;
	char        parm_names[ MAX_PARMS ][ MAX_NAME ];
	int         numparms;
	
	if ( pr_scope )
	{
		lex.ParseError( "Functions must be global" );
	}
	
	type = ParseFunction( type, parm_names, &numparms );
	def = program.GetDef( type, name, NULL, true, &lex );
	
	// check if this is a prototype or declaration
	if ( !lex.Check( "{" ) )
	{
		// it's just a prototype, so get the ; and move on
		lex.Expect( ";" );
		return;
	}
	
	if ( def->initialized )
	{
		lex.ParseError( "%s redeclared", name );
	}
	
	program.locals_start = program.numpr_globals;
	program.locals_end = program.numpr_globals;
	
	pr_scope = def;
	f = ParseImmediateStatements( type, parm_names, numparms );
	pr_scope = NULL;
	
	program.locals_end = program.numpr_globals;
	
	def->initialized = 1;
	
	if ( program.numfunctions >= MAX_FUNCTIONS )
	{
		lex.ParseError( "Exceeded max functions." );
		gi.Error( ERR_DROP, "Compile failed." );
	}
	
	program.setFunction( def->ofs, program.numfunctions );
	
	// fill in the dfunction
	df = &program.functions[ program.numfunctions ];
	program.numfunctions++;
	
	df->eventnum = 0;
	df->first_statement = f.code;
	df->s_name = def->name;
	df->s_file = program.s_file;
	df->numparms = def->type->num_parms;
	df->minparms = def->type->min_parms;
	df->locals = program.locals_end - program.locals_start;
	df->parm_start = program.locals_start;
	df->parm_total = 0;
	
	for( i = 0; i < df->numparms; i++ )
	{
		df->parm_size[ i ] = type_size[ def->type->parm_types[ i ]->type ];
		df->parm_total += df->parm_size[ i ];
	}
	
	program.locals_start = program.locals_end;
}

void Compiler::ParseVariableDef( type_t *type, const char *name )
{
	def_t *def;
	
	def = program.GetDef( type, name, pr_scope, true, &lex );
	
	// check for an initialization
	if ( lex.Check( "=" ) )
	{
		if ( def->initialized )
		{
			lex.ParseError( "%s redeclared", name );
		}
		
		if ( lex.pr_immediate_type != type )
		{
			lex.ParseError( "wrong immediate type for %s", name );
		}
		
		def->initialized = 1;
		
		if ( type == &type_string )
		{
			lex.pr_immediate.string = program.CopyString( lex.pr_immediate_string );
			
			if ( pr_scope )
			{
				//Statement( &pr_opcodes[ OP_STORE_S ], immediate, def );
				gi.WDPrintf( "Declaring and initializing a string at the same on the stack doesn't work right now.\n  Please change line %d\n", lex.SourceLine() );
			}
		}
		
		memcpy( program.pr_globals + def->ofs, &lex.pr_immediate, 4 * type_size[ lex.pr_immediate_type->type ] );
		
		lex.Lex();
	}
	else if ( type == &type_string )
	{
		eval_t temp;
		
		VectorClear( temp.vector );
		temp.string = program.CopyString( NULL );
		memcpy( program.pr_globals + def->ofs, &temp, 4 * type_size[ type->type ] );
	}
}

/*
================
ParseDefs

Called at the outer layer and when a local statement is hit
================
*/
void Compiler::ParseDefs( void )
{
	str      name;
	type_t	*type;
	
	if ( lex.pr_token_type == tt_preprocessor )
	{
		if ( lex.Check( "#include" ) )
		{
			if ( pr_scope )
            {
				lex.ParseError( "#include can only be used globally." );
            }
			
			if ( ( lex.pr_token_type != tt_immediate ) || ( lex.pr_immediate_type != &type_string ) )
            {
				lex.ParseError( "Expected quoted filename" );
            }
			
			program.Compile( lex.pr_immediate_string );
			lex.Lex();
			
			return;
		}
		else
		{
			lex.ParseError( "Unknown preprocessor command" );
		}
		
		return;
	}
	
	type = lex.ParseType();
	name = lex.ParseName();
	
	// check for a function prototype or declaraction
	if ( lex.Check( "(" ) )
	{
		ParseFunctionDef( type, name.c_str() );
	}
	else
	{
		ParseVariableDef( type, name.c_str() );
		while( lex.Check( "," ) )
		{
			name = lex.ParseName();
			ParseVariableDef( type, name.c_str() );
		}
		lex.Expect( ";" );
	}
}
   
/*
============
CompileFile

compiles the 0 terminated text, adding defintions to the pr structure
============
*/
bool Compiler::CompileFile( const char *text, int filenum )
{
	filenumber = filenum;
	
	callthread = false;
	
	def_ret.ofs = OFS_RETURN;
	def_ret.localofs = def_ret.ofs;
	
	lex.SetSource( text );
	
	// read first token
	lex.Lex();
	
	while( lex.pr_token_type != tt_eof )
	{
		try
		{
			// outside all functions
			pr_scope = NULL;
			
			ParseDefs();
		}
		
		catch( ... )
		{
			if ( ++program.pr_error_count > scr_maxerrors->integer )
            {
				break;
            }
			
			if ( pr_scope )
            {
				lex.SkipOutOfFunction();
            }
			else
            {
				lex.SkipToSemicolon();
            }
		}
	}
	
	return( program.pr_error_count == 0 );
}
