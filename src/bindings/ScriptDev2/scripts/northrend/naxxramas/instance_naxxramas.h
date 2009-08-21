#ifndef DEF_NAXXRAMAS_H
#define DEF_NAXXRAMAS_H

enum
{
    MAX_ENCOUNTER               = 15,

	TYPE_ARAC_ANUBREKHAN		= 0,
	TYPE_ARAC_FAERLINA			= 1,
	TYPE_ARAC_MAEXXNA			= 2,
	TYPE_PLAG_NOTH				= 3,
	TYPE_PLAG_HEIGAN			= 4,
	TYPE_PLAG_LOATHEB			= 5,
	TYPE_MILI_RAZUVIOUS			= 6,
	TYPE_MILI_GOTHIK			= 7,
	TYPE_MILI_HORSEMEN			= 8,
	TYPE_CONS_PATCHWERK			= 9,
	TYPE_CONS_GROBBULUS			= 10,
	TYPE_CONS_GLUTH				= 11,
	TYPE_CONS_THADDIUS			= 12,
	TYPE_SAPPHIRON				= 13,
	TYPE_KELTHUZAD				= 14,

    GO_ARAC_ANUB_GATE           = 181195,                   //open when anub dead?
    GO_ARAC_ANUB_DOOR           = 181126,                   //encounter door
    GO_ARAC_FAER_WEB            = 181235,                   //encounter door
    GO_ARAC_DOOD_DOOR_2         = 194022,                   //after faerlina, to outer ring
    GO_ARAC_MAEX_OUTER_DOOR     = 181209,                   //right before maex
    GO_ARAC_MAEX_INNER_DOOR     = 181197,                   //encounter door

    GO_PLAG_SLIME01_DOOR        = 181198,
    GO_PLAG_SLIME02_DOOR        = 181199,
    GO_PLAG_NOTH_ENTRY_DOOR     = 181200,                   //encounter door
    GO_PLAG_NOTH_EXIT_DOOR      = 181201,                   //exit, open when boss dead
    GO_PLAG_HEIG_ENTRY_DOOR     = 181202,
    GO_PLAG_HEIG_EXIT_DOOR      = 181203,                   //exit, open when boss dead
    GO_PLAG_LOAT_DOOR           = 181241,                   //encounter door

    GO_MILI_GOTH_ENTRY_GATE     = 181124,
    GO_MILI_GOTH_EXIT_GATE      = 181125,
    GO_MILI_GOTH_COMBAT_GATE    = 181170,                   //encounter door (?)
    GO_MILI_HORSEMEN_DOOR       = 181119,                   //encounter door

    GO_CHEST_HORSEMEN_NORM      = 181366,                   //four horsemen event, DoRespawnGameObject() when event == DONE
    GO_CHEST_HORSEMEN_HERO      = 193426,

    GO_CONS_PATH_EXIT_DOOR      = 181123,
    GO_CONS_GLUT_EXIT_DOOR      = 181120,                   //encounter door (?)
    GO_CONS_THAD_DOOR           = 181121,

    NPC_ZELIEK                  = 16063,
    NPC_THANE                   = 16064,
    NPC_BLAUMEUX                = 16065,
    NPC_RIVENDARE               = 30549,

    NPC_THADDIUS                = 15928,
    NPC_STALAGG                 = 15929,
    NPC_FEUGEN                  = 15930,

    AREATRIGGER_FROSTWYRM       = 4120                      //not needed here, but AT to be scripted
};

#endif