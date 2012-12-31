//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/characterstate.cpp                        $
// $Revision:: 24                                                             $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#include "_pch_cpp.h"
#include "characterstate.h"
#include "animate.h"
#include "scriptmaster.h"

static const char *MoveControl_Names[] =
{
	"user",           // MOVECONTROL_USER
	"legs",           // MOVECONTROL_LEGS
	"anim",           // MOVECONTROL_ANIM
	"absolute",       // MOVECONTROL_ABSOLUTE
	"hanging",        // MOVECONTROL_HANGING
	"wallhug",        // MOVECONTROL_WALLHUG
	"monkeybars",     // MOVECONTROL_MONKEYBARS
	"pipecrawl",      // MOVECONTROL_PIPECRAWL
	"pipehang",       // MOVECONTROL_PIPEHANG
	"stepup",         // MOVECONTROL_STEPUP
	"rope_grab",      // MOVECONTROL_ROPE_GRAB
	"rope_release",   // MOVECONTROL_ROPE_RELEASE
	"rope_move",      // MOVECONTROL_ROPE_MOVE
	"pickupenemy",    // MOVECONTROL_PICKUPENEMY
	"push",           // MOVECONTROL_PUSH
	"climbwall",      // MOVECONTROL_CLIMBWALL
	"useanim",        // MOVECONTROL_USEANIM
	"crouch",         // MOVECONTROL_CROUCH
	"loopuseanim",    // MOVECONTROL_LOOPUSEANIM
	"useobject",      // MOVECONTROL_USEOBJECT
	"coolobject",     // MOVECONTROL_COOLOBJECT
	"fakeplayer",     // MOVECONTROL_FAKEPLAYER
	NULL
};

static const char *Camera_Names[] =
{
	"topdown",        // CAMERA_TOPDOWN
	"behind",         // CAMERA_BEHIND
	"front",          // CAMERA_FRONT
	"side",           // CAMERA_SIDE
	"behind_fixed",   // CAMERA_BEHIND_FIXED
	"side_left",      // CAMERA_SIDE_LEFT
	"side_right",     // CAMERA_SIDE_RIGHT
	"behind_nopitch", // CAMERA_BEHIND_NOPITCH
	NULL
};

Conditional::Conditional( const Condition<Class> &cond ) : 
	condition( cond )
{
	result = false;
	previous_result = false;
	checked = false;
}

Conditional::Conditional( void )
{
	gi.Error( ERR_DROP, "Conditional created with wrong constructor\n" );
}

Expression::Expression()
{
}

Expression::Expression( const Expression &exp )
{
	int i;
	
	value = exp.value;
	
	for( i = 1; i <= exp.conditions.NumObjects(); i++ )
	{
		conditions.AddObject( exp.conditions.ObjectAt( i ) );
	}
}

Expression::Expression( Script &script, State &state )
{
	str token;
	condition_t condition;
	int start;
	
	value = script.GetToken( true );
	
	if ( !script.TokenAvailable( false ) || Q_stricmp( script.GetToken( false ), ":" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting ':' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( false ) )
	{
		token = script.GetToken( true );
		
		switch( token[ 0 ] )
		{
		case '!' :
            condition.test = TC_ISFALSE;
            start = 1;
            break;
			
		case '+' :
            condition.test = TC_EDGETRUE;
            start = 1;
            break;
			
		case '-' :
            condition.test = TC_EDGEFALSE;
            start = 1;
            break;
			
		default :
            condition.test = TC_ISTRUE;
            start = 0;
		}
		
		if ( token.length() <= start )
		{
			gi.Error( ERR_DROP, "%s: Illegal syntax '%s' on line %d.\n", script.Filename(), &token, script.GetLineNumber() );
			condition.condition_index = 0;
			continue;
		}
		
		condition.condition_index = state.addCondition( &token[ start ], script );
		if ( !condition.condition_index )
		{
			gi.Error( ERR_DROP, "%s: Unknown condition '%s' on line %d.\n", script.Filename(), &token[ start ], script.GetLineNumber() );
		}
		
		conditions.AddObject( condition );
	}
}

Expression::Expression( Script &script, FuzzyVar &fuzzyVar )
{
	str token;
	condition_t condition;
	int start;
	
	value = script.GetToken( true );
	points = (float)atof(value.c_str());
	
	if ( !script.TokenAvailable( false ) || Q_stricmp( script.GetToken( false ), ":" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting ':' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( false ) )
	{
		token = script.GetToken( true );
		
		switch( token[ 0 ] )
		{
		case '!' :
            condition.test = TC_ISFALSE;
            start = 1;
            break;
			
		case '+' :
            condition.test = TC_EDGETRUE;
            start = 1;
            break;
			
		case '-' :
            condition.test = TC_EDGEFALSE;
            start = 1;
            break;
			
		default :
            condition.test = TC_ISTRUE;
            start = 0;
		}
		
		if ( token.length() <= start )
		{
			gi.Error( ERR_DROP, "%s: Illegal syntax '%s' on line %d.\n", script.Filename(), &token, script.GetLineNumber() );
			condition.condition_index = 0;
			continue;
		}
		
		condition.condition_index = fuzzyVar.addCondition( &token[ start ], script );
		if ( !condition.condition_index )
		{
			gi.Error( ERR_DROP, "%s: Unknown condition '%s' on line %d.\n", script.Filename(), &token[ start ], script.GetLineNumber() );
		}
		
		conditions.AddObject( condition );
	}
}


bool Expression::getResult( const State &state, Entity &ent, const Container<Conditional *> *sent_conditionals )
{
	int i;
	condition_t *cond;
	Conditional *conditional;

	Q_UNUSED(state);
	
	for( i = 1; i <= conditions.NumObjects(); i++ )
	{
		cond = &conditions.ObjectAt( i );
		conditional = sent_conditionals->ObjectAt( cond->condition_index );
		
		if ( !conditional || !conditional->getResult( cond->test, ent ) )
		{
			return false;
		}
	}
	
	return true;
}

bool Expression::getResult( const FuzzyVar &fVar, Entity &ent, const Container<Conditional *> *sent_conditionals )
{
	int i;
	condition_t *cond;
	Conditional *conditional;

	Q_UNUSED(fVar);
	
	for( i = 1; i <= conditions.NumObjects(); i++ )
	{
		cond = &conditions.ObjectAt( i );
		conditional = sent_conditionals->ObjectAt( cond->condition_index );
		
		if ( !conditional || !conditional->getResult( cond->test, ent ) )
		{
			return false;
		}
	}
	
	return true;
}

void State::readNextState( Script &script )
{
	nextState = script.GetToken( false );
}

void State::readMoveType( Script &script )
{
	str token;
	const char **name;
	int i;
	
	token = script.GetToken( false );
	
	for( i = 0, name = MoveControl_Names; *name != NULL; name++, i++ )
	{
		if ( !token.icmp( *name ) )
		{
			break;
		}
	}
	
	if ( *name == NULL )
	{
		gi.Error( ERR_DROP, "%s: Unknown movetype '%s' on line %d.\n", script.Filename(), token.c_str(), script.GetLineNumber() );
	}
	else
	{
		movetype = ( movecontrol_t )i;
	}
}

qboolean State::setCameraType( const str &ctype )
{
	const char **name;
	int i;
	
	for( i = 0, name = Camera_Names; *name != NULL; name++, i++ )
	{
		if ( !ctype.icmp( *name ) )
		{
			cameratype = ( cameratype_t )i;
			return true;
		}
	}
	return false;
}

void State::readCamera( Script &script )
{
	str token;
	
	token = script.GetToken( false );
	
	if ( !setCameraType( token ) )
	{
		gi.Error( ERR_DROP, "%s: Unknown camera type '%s' on line %d.\n", script.Filename(), token.c_str(), script.GetLineNumber() );
	}
}

void State::readLegs( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( true ) )
	{
		token = script.GetToken( true );
		if ( !Q_stricmp( token.c_str(), "}" ) )
		{
			break;
		}
		
		script.UnGetToken();
		legAnims.AddObject( Expression( script, *this ) );
	}
}

void State::readTorso( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( true ) )
	{
		token = script.GetToken( true );
		if ( !Q_stricmp( token.c_str(), "}" ) )
		{
			break;
		}
		
		script.UnGetToken();
		torsoAnims.AddObject( Expression( script, *this ) );
	}
}

void State::readBehavior( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting behavior name on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	behaviorName = script.GetToken( true );
	if ( !getClass( behaviorName ) )
	{
		gi.Error( ERR_DROP, "%s: Unknown behavior '%s' on line %d.\n", script.Filename(), behaviorName.c_str(), script.GetLineNumber() );
	}
	
	// Read in the behavior arguments if there are any
	
	// New Format - Parameters use ( x , y , z ) format
	while ( script.TokenAvailable( false ) && script.AtOpenParen( false ) )
	{
		// Need to burn off first "(" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), "(" ) )
		{
			script.UnGetToken();
		}
		
		while ( !script.AtCloseParen( false ) )
		{
			token = script.GetToken( false );
			if ( Q_stricmp( token.c_str(), "," ) )
			{
				addBehaviorParm( token );
			}
		} 
		
		// Need to burn off last ")" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), ")" ) )
		{
			script.UnGetToken();
		}
	}
	
	// Old Format - For Legacy Data
	while ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token = script.GetToken( false );		
		addBehaviorParm( token );
	}	
}

void State::readHeadBehavior( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting behavior name on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	headBehaviorName = script.GetToken( true );
	if ( !getClass( headBehaviorName ) )
	{
		gi.Error( ERR_DROP, "%s: Unknown behavior '%s' on line %d.\n", script.Filename(), headBehaviorName.c_str(), script.GetLineNumber() );
	}
	
	// Read in the behavior arguments if there are any
	
	// New Format - Parameters use ( x , y , z ) format
	while ( script.TokenAvailable( false ) && script.AtOpenParen( false ) )
	{
		// Need to burn off first "(" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), "(" ) )
		{
			script.UnGetToken();
		}
		
		while ( !script.AtCloseParen( false ) )
		{
			token = script.GetToken( false );
			if ( Q_stricmp( token.c_str(), "," ) )
			{
				addHeadBehaviorParm( token );
			}
		} 
		
		// Need to burn off last ")" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), ")" ) )
		{
			script.UnGetToken();
		}
	}
	
	// Old Format - For Legacy Data
	while ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token = script.GetToken( false );		
		addHeadBehaviorParm( token );
	}	
}

void State::readEyeBehavior( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting behavior name on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	eyeBehaviorName = script.GetToken( true );
	if ( !getClass( eyeBehaviorName ) )
	{
		gi.Error( ERR_DROP, "%s: Unknown behavior '%s' on line %d.\n", script.Filename(), headBehaviorName.c_str(), script.GetLineNumber() );
	}
	
	// Read in the behavior arguments if there are any
	
	// New Format - Parameters use ( x , y , z ) format
	while ( script.TokenAvailable( false ) && script.AtOpenParen( false ) )
	{
		// Need to burn off first "(" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), "(" ) )
		{
			script.UnGetToken();
		}
		
		while ( !script.AtCloseParen( false ) )
		{
			token = script.GetToken( false );
			if ( Q_stricmp( token.c_str(), "," ) )
			{
				addEyeBehaviorParm( token );
			}
		} 
		
		// Need to burn off last ")" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), ")" ) )
		{
			script.UnGetToken();
		}
	}
	
	// Old Format - For Legacy Data
	while ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token = script.GetToken( false );		
		addEyeBehaviorParm( token );
	}	
}

void State::readTorsoBehavior( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting behavior name on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	torsoBehaviorName = script.GetToken( true );
	if ( !getClass( torsoBehaviorName ) )
	{
		gi.Error( ERR_DROP, "%s: Unknown behavior '%s' on line %d.\n", script.Filename(), headBehaviorName.c_str(), script.GetLineNumber() );
	}
	
	// Read in the behavior arguments if there are any
	
	// New Format - Parameters use ( x , y , z ) format
	while ( script.TokenAvailable( false ) && script.AtOpenParen( false ) )
	{
		// Need to burn off first "(" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), "(" ) )
		{
			script.UnGetToken();
		}
		
		while ( !script.AtCloseParen( false ) )
		{
			token = script.GetToken( false );
			if ( Q_stricmp( token.c_str(), "," ) )
			{
				addTorsoBehaviorParm( token );
			}
		} 
		
		// Need to burn off last ")" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), ")" ) )
		{
			script.UnGetToken();
		}
	}
	
	// Old Format - For Legacy Data
	while ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token = script.GetToken( false );		
		addTorsoBehaviorParm( token );
	}	
}

void State::readTime( Script &script )
{
	str token;
	
	// Old Format - For Legacy Data
	if ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token   = script.GetToken( false );
		minTime = (float)atof( token.c_str() );
	}
	
	if ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token   = script.GetToken( false );
		maxTime = (float)atof( token.c_str() );
	}
	else
	{
		maxTime = minTime;
	}
	
	// New Format - Parameters use ( x , y , z ) format
	if ( script.TokenAvailable( false ) && script.AtOpenParen( false ) )
	{
		// Need to burn off first "(" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), "(" ) )
		{
			script.UnGetToken();
		}
		
		// Grab Min Time
		token = script.GetToken( false );
		minTime = (float)atof( token.c_str() );
		
		// Set a default value for maxTime
		maxTime = minTime;
		
		token = script.GetToken( false );
		
		if ( !Q_stricmp( token.c_str(), "," ))
		{
			token = script.GetToken( false );
			maxTime = (float)atof( token.c_str() );		
			
			// Grab Closing Paren
			token = script.GetToken( false );
		}
		
		// Check that we have the closing paren, if not - complain
		if ( Q_stricmp( token.c_str(), ")") )
		{
			gi.Error( ERR_DROP, "%s: Missing Closing Paren ) on line %d.\n", script.Filename(), script.GetLineNumber() );
		}
	}
}

void State::readStates( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( true ) )
	{
		token = script.GetToken( true );
		if ( !Q_stricmp( token.c_str(), "}" ) )
		{
			break;
		}
		
		script.UnGetToken();
		states.AddObject( Expression( script, *this ) );
	}
}

void State::ParseAndProcessCommand( const str &command, Entity *target )
{
	int      argc;
	const    char *argv[ MAX_COMMANDS ];
	char     args[ MAX_COMMANDS ][ MAXTOKEN ];
	Script   script;
	Event    *event;
	
	script.Parse( command, command.length(), "CommandString" );
	
	argc = 0;
	while( script.TokenAvailable( false ) )
	{
		if ( argc >= MAX_COMMANDS )
		{
			gi.WDPrintf( "State:ParseAndProcessCommand : Line exceeds %d command limit", MAX_COMMANDS );
			script.SkipToEOL();
			break;
		}
		strcpy( args[ argc ], script.GetToken( false ) );
		argv[ argc ] = args[ argc ];
		argc++;
	}
	
	assert( argc > 0 );
	
	if ( argc <= 0 )
		return;
	
	event = new Event( args[0] );
	event->AddTokens( argc - 1, &argv[ 1 ] );
	target->ProcessEvent( event );
}

void State::ProcessEntryCommands( Entity *target )
{
	int i,count;
	str command;
	
	assert( target );
	if ( !target )
	{
		return;
	}
	
	count = entryCommands.NumObjects();
	for( i = 1; i <= count; i++ )
	{
		ParseAndProcessCommand( entryCommands.ObjectAt( i ), target );
	}
}

void State::ProcessExitCommands( Entity *target )
{
	int i,count;
	str command;
	
	assert( target );
	if ( !target )
	{
		return;
	}
	
	count = exitCommands.NumObjects();
	for( i = 1; i <= count; i++ )
	{
		ParseAndProcessCommand( exitCommands.ObjectAt( i ), target );
	}
}

void State::readCommands( Script &script, Container<str> &container )
{
	str token;
	str command;
	qboolean finish_line;
	
	if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( true ) )
	{
		finish_line = false;
		
		while( script.TokenAvailable( false ) )
		{
			token = script.GetToken( false );
			
			//Skip Parens "()" or Commas ","
			while( !Q_stricmp( token.c_str(), "(" ) || !Q_stricmp( token.c_str(), ")" ) || !Q_stricmp( token.c_str(), "," ))
			{
				if ( script.TokenAvailable( false ) )
					token = script.GetToken( false );
				else
				{
					finish_line = true;
					break;
				}
			}
			
			if ( finish_line )
				break;
			
			if ( !Q_stricmp( token.c_str(), "}" ) )
				return;
			
			if ( token.length() )            
			{
				if ( strstr( token.c_str(), " " ) == NULL )
				{
					command.append( token );
				}
				else
				{
					command.append( "\"" );
					command.append( token );
					command.append( "\"" );
				}
			}
			else
			{
				command.append( "\"\"" );
			}
			
			command.append( " " );
		}
		
		container.AddObject( command );
		command = "";
	}
	
	return;
}

State *State::Evaluate( Entity &ent, Container<Conditional *> *sent_conditionals )
{
	int i;
	Expression *exp;
	State *state;
	int index;
	
	for( i = 1; i <= condition_indexes.NumObjects(); i++ )
	{
		index = condition_indexes.ObjectAt( i );
		sent_conditionals->ObjectAt( index )->clearCheck();
		//conditions.ObjectAt( i )->clearCheck();
	}
	
	for( i = 1; i <= states.NumObjects(); i++ )
	{
		exp = &states.ObjectAt( i );
		if ( exp->getResult( *this, ent, sent_conditionals ) )
		{
			state = statemap.FindState( exp->getValue() );
			return state;
		}
	}
	
	return this;
}

const char *State::getLegAnim( Entity &ent, Container<Conditional *> *sent_conditionals )
{
	int i;
	Expression *exp;
	int index;
	
	for( i = 1; i <= condition_indexes.NumObjects(); i++ )
	{
		index = condition_indexes.ObjectAt( i );
		sent_conditionals->ObjectAt( index )->clearCheck();
		//conditions.ObjectAt( i )->clearCheck();
	}
	
	for( i = 1; i <= legAnims.NumObjects(); i++ )
	{
		exp = &legAnims.ObjectAt( i );
		if ( exp->getResult( *this, ent, sent_conditionals ) )
		{
			return exp->getValue();
		}
	}
	
	return "";
}

const char *State::getTorsoAnim( Entity &ent, Container<Conditional *> *sent_conditionals )
{
	int i;
	Expression *exp;
	int index;
	
	for( i = 1; i <= condition_indexes.NumObjects(); i++ )
	{
		index = condition_indexes.ObjectAt( i );
		sent_conditionals->ObjectAt( index )->clearCheck();
	}
	
	for( i = 1; i <= torsoAnims.NumObjects(); i++ )
	{
		exp = &torsoAnims.ObjectAt( i );
		if ( exp->getResult( *this, ent, sent_conditionals ) )
		{
			return exp->getValue();
		}
	}
	
	return "";
}

const char *State::getBehaviorName( void )
{
	return behaviorName.c_str();
}

const char *State::getHeadBehaviorName( void )
{
	return headBehaviorName.c_str();
}

const char *State::getEyeBehaviorName( void )
{
	return eyeBehaviorName.c_str();
}

const char *State::getTorsoBehaviorName( void )
{
	return torsoBehaviorName.c_str();
}

float State::getMinTime( void )
{
	return minTime;
}

float State::getMaxTime( void )
{
	return maxTime;
}

int State::addCondition( const char *name, Script &script )
{
	Conditional       *condition;
	Condition<Class>  *cond;
	int index;
	
	str token;
	
	condition = NULL;
	cond = statemap.getCondition( name );
	if ( !cond )
	{
		return 0;
	}
	
	condition = new Conditional( *cond );
	
	// Get the paramaters
	// Legacy Format - Parameters use ""
	while ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token = script.GetToken( false );
		condition->addParm( token );      
	}
	
	// New Format - Parameters use ( x , y , z ) format
	while ( script.TokenAvailable( false ) && script.AtOpenParen( false ) )
	{
		// Need to burn off first "(" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), "(" ) )
		{
			script.UnGetToken();
		}
		
		while ( !script.AtCloseParen( false ) )
		{
			token = script.GetToken( false );
			if ( Q_stricmp( token.c_str(), "," ) )
			{
				condition->addParm( token );
			}
		} 
		
		// Need to burn off last ")" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), ")" ) )
		{
			script.UnGetToken();
		}
	}
	
	// only add a new conditional if a similar one doesn't exist
	index = statemap.findConditional( condition );
	
	if ( index )
	{
		// delete the one we just made
		delete condition;
	}
	else
	{
		index = statemap.addConditional( condition );
	}
	
	condition_indexes.AddUniqueObject( index );
	
	return index;
}

void State::CheckStates( void )
{
	const char *value;
	int i;
	
	if ( !statemap.FindState( nextState.c_str() ) )
	{
		gi.Error( ERR_DROP, "Unknown next state '%s' referenced in state '%s'.\n", nextState.c_str(), getName() );
	}
	
	for( i = 1; i <= states.NumObjects(); i++ )
	{
		value = states.ObjectAt( i ).getValue();
		if ( !statemap.FindState( value ) )
		{
			gi.WDPrintf( 
				"============\n"
				"ERROR in %s:\n"
				"  Possibly Unknown state '%s' referenced in state '%s'. -- Check your includes \n"
				"============", 
				statemap.Filename(), value, getName() );
		}
	}
}

void State::GetLegAnims( Container<const char *> *c )
{
	int i,j;
	qboolean addobj = true;
	
	for ( i=1; i<=legAnims.NumObjects(); i++ )
	{
		const char *value = legAnims.ObjectAt( i ).getValue();
		addobj = true;
		
		// Check to see if it's already in there
		for ( j=1; j<=c->NumObjects(); j++ )
		{
			if ( !stricmp( c->ObjectAt( j ), value ) )
            {
				addobj = false;
				break;
            }
		}
		if ( addobj )
			c->AddObject( value );
	}
}

void State::GetTorsoAnims( Container<const char *> *c )
{
	int i,j;
	qboolean addobj = true;
	
	for ( i=1; i<=torsoAnims.NumObjects(); i++ )
	{
		const char *value = torsoAnims.ObjectAt( i ).getValue();
		addobj = true;
		
		// Check to see if it's already in there
		for ( j=1; j<=c->NumObjects(); j++ )
		{
			if ( !stricmp( c->ObjectAt( j ), value ) )
            {
				addobj = false;
				break;
            }
		}
		if ( addobj )
			c->AddObject( value );
	}
}

qboolean State::IgnoreGlobalStates()
{
	return ignoreGlobalStates;
}

State::State( const char *statename, Script &script, StateMap &map ) : 
	statemap( map )

{
	str cmd;
	
	name = statename;
	nextState = statename;
	movetype = DEFAULT_MOVETYPE;
	cameratype = DEFAULT_CAMERA;
	//behaviorName = "idle";
	behaviorName = "";
	headBehaviorName = "";
	eyeBehaviorName = "";
	torsoBehaviorName = "";
	
	minTime = -1.0f;
	maxTime = -1.0f;
	
	ignoreGlobalStates = false;
	
	if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( true ) )
	{
		cmd = script.GetToken( true );
		if ( !cmd.icmp( "nextstate" ) )
		{
			readNextState( script );
		}
		else if ( !cmd.icmp( "movetype" ) )
		{
			readMoveType( script );
		}
		else if ( !cmd.icmp( "camera" ) )
		{
			readCamera( script );
		}
		else if ( !cmd.icmp( "legs" ) )
		{
			readLegs( script );
		}
		else if ( !cmd.icmp( "torso" ) )
		{
			readTorso( script );
		}
		else if ( !cmd.icmp( "behavior" ) )
		{
			readBehavior( script );
		}
		else if ( !cmd.icmp( "headbehavior" ) )
		{
			readHeadBehavior( script );
		}
		else if ( !cmd.icmp( "eyebehavior" ) )
		{
			readEyeBehavior( script );
		}
		else if ( !cmd.icmp( "torsobehavior" ) )
		{
			readTorsoBehavior( script );
		}
		else if ( !cmd.icmp( "time" ) )
		{
			readTime( script );
		}
		else if ( !cmd.icmp( "states" ) )
		{
			readStates( script );
		}
		else if ( !cmd.icmp( "ignoreglobalstate" ) )
		{
			ignoreGlobalStates = true;
		}
		else if ( !cmd.icmp( "entrycommands" ) )
		{
			readCommands( script, entryCommands );
		}
		else if ( !cmd.icmp( "exitcommands" ) )
		{
			readCommands( script, exitCommands );
		}
		else if ( !cmd.icmp( "}" ) )
		{
			break;
		}
		else
		{
			gi.Error( ERR_DROP, "%s: Unknown command '%s' on line %d.\n", script.Filename(), cmd.c_str(), script.GetLineNumber() );
		}
	}
}

StateMap::StateMap( void )
{
	gi.Error( ERR_DROP, "StateMap created with wrong constructor\n" );
}

StateMap::StateMap( const char *file_name, Condition<Class> *conditions, Container<Conditional *> *conditionals )
{
	
	assert( file_name );
	
	filename = file_name;
	
	this->current_conditions = conditions;
	
	this->current_conditionals = conditionals;
	
	Script *script;	
    script = new Script(file_name);
	if ( script )
	{
		ReadStates(script);
		delete script;
		script = 0;
	}
	
	// Have all the states check themselves to see if they reference any non-existant states.
	int i;
	for( i = 1; i <= stateList.NumObjects(); i++ )
	{
		stateList.ObjectAt( i )->CheckStates();
	}	
}

StateMap::~StateMap()
{
	int i,num;
	
	num = stateList.NumObjects();
	for( i=num; i>0; i-- )
	{
		delete stateList.ObjectAt( i );
	}
	stateList.FreeObjectList();
	
	num = globalStateList.NumObjects();
	for( i=num; i>0; i-- )
	{
		delete globalStateList.ObjectAt( i );
	}
	
	globalStateList.FreeObjectList();
}

Condition<Class> *StateMap::getCondition( const char *name )
{
	Condition<Class> *c;
	
	if ( current_conditions )
	{
		for( c = current_conditions; c->name; c++ )
		{
			if ( !strcmp( c->name, name ) )
            {
				return c;
            }
		}
	}
	
	return NULL;
}

void StateMap::ReadStates( Script *script )
{
	str cmd;
	str statename;
	State *state;
	const char *includedFile;
	
	while( script->TokenAvailable( true ) )
	{
		cmd = script->GetToken( true );
		//if ( cmd == "" )
		//	  break;
		if ( !cmd.icmp( "state" ) )
		{
			statename = script->GetToken( false );
			state = FindState( statename.c_str() );
			if ( state )
			{
				stateList.RemoveObject( state );
				delete state;
				state = NULL;
			}
			
			// parse the state even if we already have it defined
			state = new State( statename.c_str(), *script, *this );
			stateList.AddObject( state );
		}
		else if ( !cmd.icmp( "globalstate" ) )
		{
			statename = script->GetToken( false );
			state = FindGlobalState( statename.c_str() );
			if ( state )
			{
				globalStateList.RemoveObject( state );
				delete state;
				state = NULL;
			}
			
			state = new State( statename.c_str(), *script, *this );
			globalStateList.AddObject( state );
		}
		else if ( !cmd.icmp( "$include" ) )
		{
			includedFile = script->GetToken( false );
			Script *newScript = new Script(includedFile);
			if ( !newScript )
				break;
			
			ReadStates(newScript);
			
			// Copy over defines to the parent script's list
			Container<macro*> *macrolist = newScript->GetMacroList();
			Container<macro*> *mainlist = script->GetMacroList();
			macro *theMacro = 0;
			for( int i = 1; i <= macrolist->NumObjects(); i++)
			{
				theMacro = macrolist->ObjectAt( i );
				macro *addMacro = new macro;
				addMacro->macroName = theMacro->macroName;
				addMacro->macroText = theMacro->macroText;
				mainlist->AddObject( addMacro );
			}
			
			// delete our temporary script object
			delete newScript;
			newScript = 0;
		}
		else
		{
			gi.Error( ERR_DROP, "%s: Unknown command '%s' on line %d.\n", script->Filename(), cmd.c_str(), script->GetLineNumber() );
		}
	}
}

int StateMap::findConditional( Conditional *condition )
{
	int i;
	int j;
	Conditional *c;
	bool found;
	
	
	// Check for the one special case where we don't want to merge the conditionals
	
	if ( strcmp( condition->getName(), "CHANCE" ) == 0 )
		return 0;
	
	for( i = 1; i <= current_conditionals->NumObjects(); i++ )
	{
		c = current_conditionals->ObjectAt( i );
		if ( ( c->getName() == condition->getName() ) && ( c->numParms() == condition->numParms() ) )
		{
			found = true;
			for( j = 1; j <= c->numParms(); j++ )
            {
				if ( strcmp( c->getParm( j ), condition->getParm( j ) ) )
				{
					found = false;
					break;
				}
            }
			
			if ( found )
            {
				return i;
            }
		}
	}
	
	return 0;
}

int StateMap::addConditional( Conditional *condition )
{
	int index;
	index = current_conditionals->AddObject( condition );
	
	return index;
}

Conditional *StateMap::getConditional( const char *name )
{
	int i;
	Conditional       *c;
	Condition<Class>  *condition;
	
	for( i = 1; i <= current_conditionals->NumObjects(); i++ )
	{
		c = current_conditionals->ObjectAt( i );
		if ( !strcmp( c->getName(), name ) )
		{
			return c;
		}
	}
	
	condition = getCondition( name );
	
	c = new Conditional( *condition );
	current_conditionals->AddObject( c );
	
	return c;
}

State *StateMap::FindState( const char *name )
{
	int i;
	
	for( i = 1; i <= stateList.NumObjects(); i++ )
	{
		if ( !strcmp( stateList.ObjectAt( i )->getName(), name ) )
		{
			return stateList.ObjectAt( i );
		}
	}
	
	return NULL;
}

State *StateMap::FindGlobalState( const char *name )
{
	for ( int i = 1; i <= globalStateList.NumObjects(); i++ )
	{
		if ( !strcmp( globalStateList.ObjectAt( i )->getName(), name ) )
		{
			return globalStateList.ObjectAt( i );
		}
	}
	
	return NULL;
}

// Caching statemaps

struct cached_statemap_t
{
	StateMap *statemap;
	Container<Conditional *> *conditionals;
};

struct cached_fuzzyengine_t
{
	FuzzyEngine *fengine;
	Container<Conditional *> *conditionals;
};

Container<cached_statemap_t> cached_statemaps;
Container<cached_fuzzyengine_t> cached_fuzzyengines;

StateMap *GetStatemap( const str &filename, Condition<Class> *conditions, Container<Conditional *> *conditionals,
		qboolean reload, qboolean cache_only )
{
	int i;
	int j;
	cached_statemap_t *cache = NULL;
	cached_statemap_t new_cache;
	qboolean found = false;
	Conditional *new_conditional;
	Conditional *old_conditional;
	Condition<Class>  *cond;
	
	for( i = 1 ; i <= cached_statemaps.NumObjects() ; i++ )
	{
		cache = &cached_statemaps.ObjectAt( i );
		
		if ( strcmp( cache->statemap->Filename(), filename.c_str() ) == 0 )
		{
			found = true;
			break;
		}
	}
	
	if ( found && reload )
	{
		delete cache->statemap;
		delete cache->conditionals;
		
		cache->conditionals = new Container<Conditional *>;
		cache->statemap = new StateMap( filename, conditions, cache->conditionals );
	}
	
	if ( !found )
	{
		new_cache.conditionals = new Container<Conditional *>;
		new_cache.statemap = new StateMap( filename, conditions, new_cache.conditionals );
		
		cached_statemaps.AddObject( new_cache );
		
		cache = &new_cache;
	}
	
	// Copy conditionals over
	
	if ( !cache_only )
	{
		for( i = 1 ; i <= cache->conditionals->NumObjects() ; i++ )
		{
			old_conditional = cache->conditionals->ObjectAt( i );
			
			cond = cache->statemap->getCondition( old_conditional->condition.name );
			
			new_conditional = new Conditional( *cond );
			
			for( j = 1 ; j <= old_conditional->parmList.NumObjects() ; j++ )
			{
				new_conditional->parmList.AddObject( old_conditional->parmList.ObjectAt( j ) );
			}
			
			conditionals->AddObject( new_conditional );
		}
	}
	
	return cache->statemap;
}


FuzzyEngine *GetFuzzyEngine( const str &filename, Condition<Class> *conditions, Container<Conditional *> *conditionals,
		qboolean reload, qboolean cache_only )
{
	int i;
	int j;
	cached_fuzzyengine_t *cache = NULL;
	cached_fuzzyengine_t new_cache;
	qboolean found = false;
	Conditional *new_conditional;
	Conditional *old_conditional;
	Condition<Class>  *cond;
	
	for( i = 1 ; i <= cached_fuzzyengines.NumObjects() ; i++ )
	{
		cache = &cached_fuzzyengines.ObjectAt( i );
		
		if ( strcmp( cache->fengine->Filename(), filename.c_str() ) == 0 )
		{
			found = true;
			break;
		}
	}
	
	if ( found && reload )
	{
		delete cache->fengine; 
		delete cache->conditionals;
		
		cache->conditionals = new Container<Conditional *>;
		cache->fengine = new FuzzyEngine( filename, conditions, cache->conditionals );
	}
	
	if ( !found )
	{
		new_cache.conditionals = new Container<Conditional *>;
		new_cache.fengine = new FuzzyEngine( filename, conditions, new_cache.conditionals );
		
		cached_fuzzyengines.AddObject( new_cache );
		
		cache = &new_cache;
	}
	
	// Copy conditionals over
	
	if ( !cache_only )
	{
		for( i = 1 ; i <= cache->conditionals->NumObjects() ; i++ )
		{
			old_conditional = cache->conditionals->ObjectAt( i );
			
			cond = cache->fengine->getCondition( old_conditional->condition.name );
			
			new_conditional = new Conditional( *cond );
			
			for( j = 1 ; j <= old_conditional->parmList.NumObjects() ; j++ )
			{
				new_conditional->parmList.AddObject( old_conditional->parmList.ObjectAt( j ) );
			}
			
			conditionals->AddObject( new_conditional );
		}
	}
	
	return cache->fengine;
}

void CacheStatemap( const str &filename, Condition<Class> *conditions )
{
	GetStatemap( filename, conditions, NULL, false, true );
}

void CacheFuzzyEngine( const str &filename, Condition<Class> *conditions )
{
	GetFuzzyEngine( filename , conditions, NULL, false, true );
}
   
void StateMap::GetAllAnims( Container<const char *> *c )
{
	int i;
	
	for( i = 1; i <= stateList.NumObjects(); i++ )
	{
		stateList.ObjectAt( i )->GetLegAnims( c );
		stateList.ObjectAt( i )->GetTorsoAnims( c );
	}
	
	for( i = 1; i <= globalStateList.NumObjects(); i++ )
	{
		globalStateList.ObjectAt( i )->GetLegAnims( c );
		globalStateList.ObjectAt( i )->GetTorsoAnims( c );
	}	
}

void ClearCachedStatemaps( void )
{
	int i,j,num2;
	cached_statemap_t *cache;
	
	num2 = cached_statemaps.NumObjects();
	
	for( i=num2 ; i>0; i-- )
	{
		cache = &cached_statemaps.ObjectAt( i );
		
		delete cache->statemap;
		
		int num = cache->conditionals->NumObjects();
		for ( j=num; j>0; j-- )
		{
			Conditional *cond = cache->conditionals->ObjectAt( j );
			delete cond;
		}
		delete cache->conditionals;
	}
	
	cached_statemaps.FreeObjectList();
}

void ClearCachedFuzzyEngines( void )
{
	int i,j,num2;
	cached_fuzzyengine_t *cache;
	
	num2 = cached_fuzzyengines.NumObjects();
	
	for( i=num2 ; i>0; i-- )
	{
		cache = &cached_fuzzyengines.ObjectAt( i );
		
		delete cache->fengine;
		
		int num = cache->conditionals->NumObjects();
		for ( j=num; j>0; j-- )
		{
			Conditional *cond = cache->conditionals->ObjectAt( j );
			delete cond;
		}
		delete cache->conditionals;
	}
	
	cached_fuzzyengines.FreeObjectList();
}

//======================================
// Fuzzy Engine Implementation
//======================================
FuzzyEngine::FuzzyEngine( const char *file_name, Condition<Class> *conditions, Container<Conditional *> *conditionals )
{
	assert( file_name );
	_filename = file_name;
	
	this->_current_conditions = conditions;
	this->_current_conditionals = conditionals;
	
	ReadFuzzyVars(_filename);   
}

void FuzzyEngine::ReadFuzzyVars( const char *filename )
{
	str cmd;
	str fuzzyVarName;
	Script script;
	FuzzyVar *fuzzyVar;
	
	script.LoadFile( filename );
	
	while( script.TokenAvailable( true ) )
	{
		cmd = script.GetToken( true );
		if ( !cmd.icmp( "fuzzyvar" ) )
		{
			fuzzyVarName = script.GetToken( false );
			fuzzyVar = FindFuzzyVar( fuzzyVarName.c_str() );
			if ( fuzzyVar )
            {
				_varList.RemoveObject( fuzzyVar );
				gi.Error( ERR_DROP, "%s: Duplicate definition of FuzzyVar '%s' on line %d.\n", filename, fuzzyVarName.c_str(), script.GetLineNumber() );
            }
			// parse the state even if we already have it defined
			fuzzyVar = new FuzzyVar( fuzzyVarName.c_str(), script, *this );
			_varList.AddObject( fuzzyVar );
		}
		else
		{
			gi.Error( ERR_DROP, "%s: Unknown command '%s' on line %d.\n", script.Filename(), cmd.c_str(), script.GetLineNumber() );
		}
	}
}

FuzzyVar *FuzzyEngine::FindFuzzyVar( const char *name )
{
	int i;
	
	for( i = 1; i <= _varList.NumObjects(); i++ )
	{
		if ( !strcmp( _varList.ObjectAt( i )->getName(), name ) )
		{
			return _varList.ObjectAt( i );
		}
	}
	
	return NULL;
}

int FuzzyEngine::addConditional( Conditional *condition )
{
	int index;
	index = _current_conditionals->AddObject( condition );
	
	return index;
}

int FuzzyEngine::findConditional( Conditional *condition )
{
	int i;
	int j;
	Conditional *c;
	bool found;
	
	
	// Check for the one special case where we don't want to merge the conditionals
	
	if ( strcmp( condition->getName(), "CHANCE" ) == 0 )
		return 0;
	
	for( i = 1; i <= _current_conditionals->NumObjects(); i++ )
	{
		c = _current_conditionals->ObjectAt( i );
		if ( ( c->getName() == condition->getName() ) && ( c->numParms() == condition->numParms() ) )
		{
			found = true;
			for( j = 1; j <= c->numParms(); j++ )
            {
				if ( strcmp( c->getParm( j ), condition->getParm( j ) ) )
				{
					found = false;
					break;
				}
            }
			
			if ( found )
            {
				return i;
            }
		}
	}
	
	return 0;
}

const char *FuzzyEngine::Filename()
{
	return _filename.c_str();
}

Condition<Class> *FuzzyEngine::getCondition( const char *name )
{
	Condition<Class> *c;
	
	if ( _current_conditions )
	{
		for( c = _current_conditions; c->name; c++ )
		{
			if ( !strcmp( c->name, name ) )
            {
				return c;
            }
		}
	}
	
	return NULL;
}


//======================================
// Fuzzy Var Implementation
//======================================
FuzzyVar::FuzzyVar( const char *varname, Script &script, FuzzyEngine &engine ) : 
	_fuzzyEngine( engine )

{
	str cmd;
	
	_name = varname;
	
	if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( true ) )
	{
		cmd = script.GetToken( true );
		if ( !cmd.icmp( "evaluations" ) )
		{
			readEvaluations( script );
		}
		else if ( !cmd.icmp( "}" ) )
		{
			break;
		}
		else
		{
			gi.Error( ERR_DROP, "%s: Unknown command '%s' on line %d.\n", script.Filename(), cmd.c_str(), script.GetLineNumber() );
		}
	}
}

void FuzzyVar::readEvaluations( Script &script )
{
	str token;
	
	if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}
	
	while( script.TokenAvailable( true ) )
	{
		token = script.GetToken( true );
		if ( !Q_stricmp( token.c_str(), "}" ) )
		{
			break;
		}
		
		script.UnGetToken();
		_evaluations.AddObject( Expression( script, *this ) );
	}
}

float FuzzyVar::Evaluate( Entity &ent, Container<Conditional *> *sent_conditionals )
{
	int i;
	Expression *exp;
	float points;
	int index;
	
	points = 0.0f;
	
	for( i = 1; i <= _condition_indexes.NumObjects(); i++ )
	{
		index = _condition_indexes.ObjectAt( i );
		sent_conditionals->ObjectAt( index )->clearCheck();
	}
	
	for( i = 1; i <= _evaluations.NumObjects(); i++ )
	{
		exp = &_evaluations.ObjectAt( i );
		if ( exp->getResult( *this, ent, sent_conditionals ) )
		{
			points += exp->getPoints();         
		}
	}
	
	return points;
}

int FuzzyVar::addCondition( const char *name, Script &script )
{
	Conditional       *condition;
	Condition<Class>  *cond;
	int index;
	
	str token;
	
	condition = NULL;
	cond = _fuzzyEngine.getCondition( name );
	
	if ( !cond )
		return 0;
	
	
	condition = new Conditional( *cond );
	
	// Get the paramaters
	// Legacy Format - Parameters use ""
	while ( script.TokenAvailable( false ) && script.AtString( false ) )
	{
		token = script.GetToken( false );
		condition->addParm( token );      
	}
	
	// New Format - Parameters use ( x , y , z ) format
	while ( script.TokenAvailable( false ) && script.AtOpenParen( false ) )
	{
		// Need to burn off first "(" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), "(" ) )
		{
			script.UnGetToken();
		}
		
		while ( !script.AtCloseParen( false ) )
		{
			token = script.GetToken( false );
			if ( Q_stricmp( token.c_str(), "," ) )
			{
				condition->addParm( token );
			}
		} 
		
		// Need to burn off last ")" - but check it to be sure
		token = script.GetToken( false );
		if ( Q_stricmp( token.c_str(), ")" ) )
		{
			script.UnGetToken();
		}
	}
	
	// only add a new conditional if a similar one doesn't exist
	index = _fuzzyEngine.findConditional( condition );
	
	if ( index )
	{
		// delete the one we just made
		delete condition;
	}
	else
	{
		index = _fuzzyEngine.addConditional( condition );
	}
	
	_condition_indexes.AddUniqueObject( index );
	
	return index;
}

const char *FuzzyVar::getName()
{
	return _name.c_str();
}
