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
SDName: Instance_Draktharon
SD%Complete: 10
SDComment:
SDCategory: DrakTharonKeep
EndScriptData */

#include "precompiled.h"
#include "def_draktharon.h"



struct MANGOS_DLL_DECL instance_draktharon : public ScriptedInstance
{
    instance_naxxramas(Map* pMap) : ScriptedInstance(pMap) {Initialize();}

    std::string strInstData;
    uint32 m_auiEncounter[MAX_ENCOUNTER];
	uint32 m_goCrystals[4];
	uint32 m_uiCrystalCounter;

	uint64 m_iNovosGUID;

    void Initialize()
    {
        memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
		memset(&m_goCrystals, 0, sizeof(m_goCrystals));
    }

    void OnCreatureCreate(Creature* pCreature)
    {
		switch(creature->GetEntry())
        {
			case NPC_NOVOS: m_iNovosGUID = pCreature->GetGUID;
		}
    }

    void OnObjectCreate(GameObject* pGo)
    {
		switch(pGo->GetEntry())
        {
			case GO_RITUAL_CRYSTAL_1:
                m_goCrystals[0] = pGo->GetGUID();
                if (m_auiEncounter[TYPE_NOVOS] == NOT_STARTED)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
			case GO_RITUAL_CRYSTAL_2:
                m_goCrystals[1] = pGo->GetGUID();
                if (m_auiEncounter[TYPE_NOVOS] == NOT_STARTED)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
			case GO_RITUAL_CRYSTAL_3:
                m_goCrystals[2] = pGo->GetGUID();
                if (m_auiEncounter[TYPE_NOVOS] == NOT_STARTED)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
			case GO_RITUAL_CRYSTAL_4:
                m_goCrystals[3] = pGo->GetGUID();
                if (m_auiEncounter[TYPE_NOVOS] == NOT_STARTED)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
		}
    }

    void SetData(uint32 uiType, uint32 uiData)
    {
		switch(uiType)
        {
			case TYPE_NOVOS:
				m_auiEncounter[TYPE_NOVOS]=uiData;
				if(uiData == NOT_STARTED)m_uiCrystalCounter=0;
				break;
			case GO_RITUAL_CRYSTAL_1:
				if (m_uiCrystalCounter<4 && GameObject* pGo = instance->GetGameObject(m_goCrystals[m_uiCrystalCounter]))
				{
					m-uiCrystalCounter++;
                    pGo->SetGoState(GO_STATE_READY);
				}
		}
    }

    uint32 GetData(uint32 uiType)
    {
        return 0;
    }

    uint64 GetData64(uint32 uiData)
    {
		switch(uiData)
        {
			case NPC_NOVOS:	          return m_iNovos;
        }
        return 0;
    }
};

InstanceData* GetInstanceData_instance_draktharon(Map* pMap)
{
    return new instance_draktharon(pMap);
}

void AddSC_instance_naxxramas()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "instance_draktharon";
    pNewScript->GetInstanceData = &GetInstanceData_instance_draktharon;
    pNewScript->RegisterSelf();
}
