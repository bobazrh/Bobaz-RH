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
SDName: Boss_Anomalus
SD%Complete: 10%
SDComment:
SDCategory: Nexus
EndScriptData */

#include "precompiled.h"
#include "def_nexus.h"

enum
{
    SAY_AGGRO               = -1576006,
    SAY_RIFT                = -1576007,
    SAY_SHIELD              = -1576008,
    SAY_KILL                = -1576009,
    SAY_DEATH               = -1576010,
    EMOTE_OPEN_RIFT         = -1576021,
    EMOTE_SHIELD            = -1576022,

    SPELL_CREATE_RIFT       = 47743,
    SPELL_CHARGE_RIFT       = 47747,
    SPELL_RIFT_SHIELD       = 47748,

    SPELL_SPARK             = 47751,
    SPELL_SPARK_H           = 57062,

    CREATURE_RIFT	    = 26918
};

enum
{
	SPELL_BURST				= 47688,
	SPELL_CHARGED_BURST		= 47737,
	SPELL_SUMMON			= 47692,
	SPELL_VISUAL			= 47686,

	CHAOS_THEORY			= 2037,
};

/*######
## boss_anomalus
######*/

struct MANGOS_DLL_DECL boss_anomalusAI : public ScriptedAI
{
    boss_anomalusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;

    uint32 m_uiShieldTimer;
    uint32 m_uiSparkTimer;
    uint32 m_uiRiftTimer;
    uint32 m_uiShieldCounter;
    uint32 m_uiChargeTimer;
	bool m_bRiftKilled;
	std::list<Creature*> m_lRifts; 
	SpellEntry const * chargeInfo;
		
    bool m_bShield;

    void Reset() 
    {
		m_lRifts.clear();
		m_uiShieldTimer=45000;
		m_uiChargeTimer=2000;
		m_uiSparkTimer=6000+rand()%2000;
		m_uiRiftTimer=10000+rand()%2000;
		m_uiShieldCounter = 0;
		chargeInfo = sSpellStore.LookupEntry(SPELL_CHARGE_RIFT); 
		m_bShield = false;
		m_bRiftKilled = false;
		if(m_pInstance)
				m_pInstance->SetData(NPC_ANOMALUS, NOT_STARTED);
    }

    void Aggro(Unit* pWho)
    {
		if(m_pInstance)
				m_pInstance->SetData(NPC_ANOMALUS, IN_PROGRESS);
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);
		if(!m_bRiftKilled && m_bIsHeroicMode){
			//TODO: Reward Chaos Theory
		}
		if(m_pInstance)
				m_pInstance->SetData(NPC_ANOMALUS, DONE);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (urand(0, 1))
            DoScriptText(SAY_KILL, m_creature);
    }

    void RiftDied(Creature * pRift){
	m_bRiftKilled=true;
    	for(std::list<Creature*>::iterator itr = m_lRifts.begin(); itr != m_lRifts.end(); ++itr)
	{
		if (*itr == pRift);
			m_lRifts.erase(itr);
		if(m_lRifts.empty())
			m_uiShieldTimer=0;
		break;
	}
    }

    void SummonRift(){
		DoScriptText(SAY_RIFT, m_creature);
		Creature *pCreature = m_creature->SummonCreature(CREATURE_RIFT, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 10);
		if(pCreature){
			m_lRifts.push_front(pCreature);
			if(m_bShield){
				Aura *Aur = CreateAura(chargeInfo, 0, NULL,pCreature,m_creature);
				pCreature->AddAura(Aur);
			} 
			pCreature->Attack(m_creature->getVictim(),false);
            pCreature->setFaction(m_creature->getFaction());
		}
    }
    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;
	if(!m_bShield){
		if (m_uiSparkTimer < uiDiff){
			DoCast(m_creature->getVictim(),m_bIsHeroicMode ? SPELL_SPARK_H : SPELL_SPARK);
			m_uiSparkTimer = 8000 + rand()%4000;
		} else m_uiSparkTimer -= uiDiff;

		if (m_uiRiftTimer < uiDiff){
			// Summon Rift
			m_uiRiftTimer = 25000 + rand()%10000;
			SummonRift();
		} else m_uiRiftTimer -= uiDiff;

		if (m_uiShieldCounter < 3 && m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < (75 - 25*m_uiShieldCounter)){
			m_uiShieldCounter++;
			m_bShield = true;
			m_uiShieldTimer=45000;
			m_uiChargeTimer=2000;
			//Stop Movement
			m_creature->InterruptNonMeleeSpells(false);
			m_creature->GetMotionMaster()->MoveIdle();
		
			DoScriptText(SAY_SHIELD, m_creature);
			SummonRift();
			DoCast(m_creature,SPELL_RIFT_SHIELD,false);
			return;
		}
        	DoMeleeAttackIfReady();
	} else {
		if(m_uiChargeTimer < uiDiff){
			for(std::list<Creature*>::iterator itr = m_lRifts.begin(); itr != m_lRifts.end(); ++itr)
			{
				if ((*itr)->isAlive()){
					Aura *Aur = CreateAura(chargeInfo, 0, NULL,(*itr),m_creature);
					(*itr)->AddAura(Aur);
				}
			}
			m_uiChargeTimer=50000;
		} else m_uiChargeTimer -= uiDiff;
		if(m_uiShieldTimer < uiDiff){
			m_bShield = false;
			m_creature->RemoveAurasDueToSpell(SPELL_RIFT_SHIELD, NULL);
			//start movement
			Unit * victim;
            if(victim = m_creature->getVictim()) 
			{
                      		m_creature->SendMeleeAttackStart(victim);
				m_creature->GetMotionMaster()->MoveChase(victim);
				m_creature->Attack(victim, false);
			} 
         }
	}

    }
};

/*######
## mob_chaotic_rift
######*/

struct MANGOS_DLL_DECL mob_chaotic_riftAI : public Scripted_NoMovementAI
{
	mob_chaotic_riftAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
	{
		m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
		Reset();
	}

	ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;
	bool m_bIsCharged;
	uint32 m_uiBurstTimer;
	uint32 m_uiSpawnTimer;

	friend class boss_anomalusAI;

	void Reset() 
    	{
		m_uiBurstTimer = 300+rand()%600;
		m_uiSpawnTimer = 5500 + rand()%10000;
		m_bIsCharged = false;
	}

	void Aggro(Unit* pWho)
    {
    }

    void JustDied(Unit* pKiller)
    {
	if(m_pInstance)
	{
		Creature* pAnomalus = ((Creature *)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_ANOMALUS)));
		((boss_anomalusAI*)(pAnomalus->AI()))->RiftDied(m_creature);
	}
    }

    void KilledUnit(Unit* pVictim)
    {
    }

    void UpdateAI(const uint32 uiDiff)
    {
		if (m_uiBurstTimer < uiDiff){
			if(m_creature->HasAura(SPELL_CHARGE_RIFT)){
				m_bIsCharged = true;
			} else m_bIsCharged = false;
			DoCast(m_creature->getVictim(),m_bIsCharged ? SPELL_CHARGED_BURST : SPELL_BURST);
			DoCast(m_creature,SPELL_VISUAL);
			m_uiBurstTimer = 700 + rand()%600;
		} else m_uiBurstTimer -= uiDiff;

		if (m_uiSpawnTimer < uiDiff){
			DoCast(m_creature->getVictim(),SPELL_SUMMON);
			m_uiSpawnTimer = m_bIsCharged ?  5000 + rand()%2000 : 14000 + rand()%4000;
		} else m_uiSpawnTimer -= uiDiff;
	}

};




CreatureAI* GetAI_boss_anomalus(Creature* pCreature)
{
    return new boss_anomalusAI(pCreature);
}

CreatureAI* GetAI_mob_chaotic_rift(Creature* pCreature)
{
    return new mob_chaotic_riftAI(pCreature);
}

void AddSC_boss_anomalus()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_anomalus";
    newscript->GetAI = &GetAI_boss_anomalus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_chaotic_rift";
    newscript->GetAI = &GetAI_mob_chaotic_rift;
    newscript->RegisterSelf();
}
