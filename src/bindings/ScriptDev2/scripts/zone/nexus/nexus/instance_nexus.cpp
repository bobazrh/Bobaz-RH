/* ScriptData
SDName: Instance_Nexus
SDComment: Instance Data Scripts and functions to acquire mobs and set encounter status for use in various Nexus Scripts
SDCategory: Nexus
EndScriptData */

#include "precompiled.h"
#include "def_nexus.h"

#define ENCOUNTERS     5

/* Nexus encounters:
0 - Ormorok
1 - Anomalus
2 - Telestra
3 - Heroic boss
4 - Kerialstrasza
*/

struct MANGOS_DLL_DECL  instance_nexus : public ScriptedInstance
{
    instance_nexus(Map *Map) : ScriptedInstance(Map) {Initialize();};

    uint64 m_iAnomalus;
    uint32 Encounters[ENCOUNTERS];
    std::string str_data;

   void Initialize()
   {
	m_iAnomalus=0;

        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            Encounters[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS) return true;

        return false;
    }

	Player* GetPlayerInMap()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* plr = itr->getSource())
                return plr;
            }
        }

        debug_log("TSCR: Instance Nexus: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    void HandleGameObject(uint64 guid, uint32 state)
    {
        Player *player = GetPlayerInMap();

        if (!player || !guid)
        {
            debug_log("TSCR: Nexus: HandleGameObject fail");
            return;
        }

       // if (GameObject *go = GameObject::GetGameObject(*player,guid))
         //   go->SetGoState(state);
    }

    void OnCreatureCreate(Creature *creature)
    {
        switch(creature->GetEntry())
        {
            case NPC_ANOMALUS:    m_iAnomalus = creature->GetGUID();             break;
        }
    }

    void OnObjectCreate(GameObject* go)
    {
        //switch(go->GetEntry())
        //{
        //door and object id
        //}
    }

    uint64 GetData64(uint32 identifier)
    {
        switch(identifier)
        {
        case NPC_ANOMALUS:	          return m_iAnomalus;
        }

        return 0;
    }

    void SetData(uint32 type, uint32 data)
    {
		switch(type)
        {
			case NPC_ORMOROK:					 Encounters[0]=data;break;
			case NPC_ANOMALUS:            	     Encounters[1]=data;break;
			case NPC_TELESTRA:					 Encounters[2]=data;break;
			case NPC_KERISTRASZA:				 Encounters[4]=data;break;
        }

        if (data == DONE)
        {
            SaveToDB();
        }
    }

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
			case NPC_ORMOROK:					 return Encounters[0];
			case NPC_ANOMALUS:            	     return Encounters[1];
			case NPC_TELESTRA:					 return Encounters[2];
			case NPC_KERISTRASZA:				 return Encounters[4];
        }

        return 0;
    }

   const char* Save()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << "N X " << Encounters[0] << " " << Encounters[1] << " "
            << Encounters[2] << " " << Encounters[3] << " " << Encounters[4];

        str_data = saveStream.str();

        OUT_SAVE_INST_DATA_COMPLETE;
        return str_data.c_str();
    }

    void Load(const char* in)
    {
        if (!in)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(in);

        char dataHead1, dataHead2;
        uint16 data0,data1,data2,data3,data4;

        std::istringstream loadStream(in);
        loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3 >> data4; 

        if( dataHead1 == 'N' && dataHead2 == 'X')
        {
            Encounters[0] = data0;
            Encounters[1] = data1;
            Encounters[2] = data2;
	    Encounters[3] = data3;
	    Encounters[4] = data4;

            for(uint8 i = 0; i < ENCOUNTERS; ++i)
                if (Encounters[i] == IN_PROGRESS)
                    Encounters[i] = NOT_STARTED;

        }else OUT_LOAD_INST_DATA_FAIL;

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_nexus(Map* map)
{
   return new instance_nexus(map);
}

void AddSC_instance_nexus()
{
   Script *newscript;
   newscript = new Script;
   newscript->Name = "instance_nexus";
   newscript->GetInstanceData = &GetInstanceData_instance_nexus;
   newscript->RegisterSelf();
}
