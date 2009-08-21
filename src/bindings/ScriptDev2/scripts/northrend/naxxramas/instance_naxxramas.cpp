/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Instance_Naxxramas
SD%Complete: 10
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "instance_naxxramas.h"



struct MANGOS_DLL_DECL instance_naxxramas : public ScriptedInstance
{
    instance_naxxramas(Map* pMap) : ScriptedInstance(pMap) {Initialize();}

    std::string strInstData;
    uint32 m_auiEncounter[MAX_ENCOUNTER];

	uint64 m_uiDoor_AnubGUID;
	uint64 m_uiGate_AnubGUID;
	uint64 m_uiDoor_FaerlinaGUID;

    void Initialize()
    {
        memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
    }

    void OnCreatureCreate(Creature* pCreature)
    {
    }

    void OnObjectCreate(GameObject* pGo)
    {
		switch(pGo->GetEntry())
        {
			case GO_ARAC_ANUB_DOOR:
                m_uiDoor_AnubGUID = pGo->GetGUID();
                if (m_auiEncounter[TYPE_ARAC_ANUBREKHAN] == DONE || m_auiEncounter[TYPE_ARAC_ANUBREKHAN] == NOT_STARTED)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
			case GO_ARAC_ANUB_GATE:
				m_uiGate_AnubGUID = pGo->GetGUID();
				if (m_auiEncounter[TYPE_ARAC_ANUBREKHAN] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
			case GO_ARAC_DOOD_DOOR_2:
				m_uiDoor_FaerlinaGUID = pGo->GetGUID();
				if (m_auiEncounter[TYPE_ARAC_FAERLINA] == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
		}
    }

    void SetData(uint32 uiType, uint32 uiData)
    {
		switch(uiType)
        {
			case TYPE_ARAC_ANUBREKHAN:
				m_auiEncounter[TYPE_ARAC_ANUBREKHAN]=uiData;
				if (uiData == DONE)
                    if (GameObject* pGo = instance->GetGameObject(m_uiGate_AnubGUID))
                        pGo->SetGoState(GO_STATE_ACTIVE);
					if (GameObject* pGo = instance->GetGameObject(m_uiDoor_AnubGUID))
                        pGo->SetGoState(GO_STATE_ACTIVE);
				if (uiData == IN_PROGRESS)
                {
					if (GameObject* pGo = instance->GetGameObject(m_uiDoor_AnubGUID))
                        pGo->SetGoState(GO_STATE_READY);
				}
				break;
			case TYPE_ARAC_FAERLINA:
				m_auiEncounter[TYPE_ARAC_FAERLINA]=uiData;
				if (uiData == DONE)
					if (GameObject* pGo = instance->GetGameObject(m_uiDoor_FaerlinaGUID))
                        pGo->SetGoState(GO_STATE_ACTIVE);
		}
    }

    uint32 GetData(uint32 uiType)
    {
        return 0;
    }

    uint64 GetData64(uint32 uiData)
    {
        return 0;
    }
};

InstanceData* GetInstanceData_instance_naxxramas(Map* pMap)
{
    return new instance_naxxramas(pMap);
}

void AddSC_instance_naxxramas()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "instance_naxxramas";
    pNewScript->GetInstanceData = &GetInstanceData_instance_naxxramas;
    pNewScript->RegisterSelf();
}
