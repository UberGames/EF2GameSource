#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include "weapon.h"

class Equipment : public Weapon
{
	public:

		CLASS_PROTOTYPE(Equipment);

		Equipment();
		Equipment(const char* file);
		~Equipment();

	void airStrike(Event *ev);
	void scanStart(Event* ev);
	void scanEnd(Event* ev);
	void scan(Event* ev);

	void setScanner(Event* ev);
	void setRadar(Event* ev);
	void hasModes( Event *ev );
	void changeMode( Event *ev );
	void changeMode( int newMode );
	void updateMode( void );
	void resetMode( void );

	bool hasScanner()			{ return scanner;			}
	bool hasRadar()			{ return radar;			}
	bool isScanning()			{ return scanning;		}

	void							setTypeName( Event *ev );
	str								getTypeName( void ) const { return _typeName; }

	/*virtual*/
	void PutAway(void);
	void ProcessTargetedEntity(EntityPtr entity);
	void AttachToOwner( weaponhand_t hand );
	void Archive(	Archiver &arc );
	void Uninitialize( void );
	void init( void );

	/* virtual */ int				getStat( int statNum );

	/* virtual */ void Think();

	private:
		bool				scanner;		//the equipment has scanning capabilities. 
		bool				scanning;	//the equipment is scanning
											//THIS MUST BE TRUE FOR SCANNING TO BE SET

		int					_scanEndFrame;
		int					scanTime;	//how long the player has been scanning.
		
		bool				radar;		

		Container<str>		_modes;
		int					_currentMode;
		int					_lastMode;

		str					_typeName;

		EntityPtr			_scannedEntity;

		bool				_active;		//true when armed, false when put away

		float				_nextUseTime;
};


#endif
