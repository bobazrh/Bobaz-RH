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
SDName: Boss_Novos
SD%Complete: 20%
SDComment:
SDCategory: Drak'Tharon Keep
EndScriptData */

#include "precompiled.h"
#include "def_draktharon.h"

enum
{
    SAY_AGGRO                       = -1600005,
    SAY_DEATH                       = -1600006,
    SAY_KILL                        = -1600007,
    SAY_ADDS                        = -1600008,
    SAY_BUBBLE_1                    = -1600009,
    SAY_BUBBLE_2                    = -1600010,

    EMOTE_ASSISTANCE                = -1600011,

	NPC_SHADOWCASTER				= 27600,
	NPC_FETIDTROLL					= 27598,
	NPC_HULKING						= 27597,
	NPC_HANDLER						= 26627,

	SPELL_ARCANE_FIELD				= 47346,

    SPELL_BLIZZARD                  = 49198,
    SPELL_BLIZZARD_H                = 59909,

    SPELL_ARCANE_BLAST              = 49034,
    SPELL_ARCANE_BLAST_H            = 59854,

    SPELL_WRATH                     = 50089,
    SPELL_WRATH_H                   = 59856,

    SPELL_FROSTBOLT                 = 49037,
    SPELL_FROSTBOLT_H               = 59855

};

struct Waypoint
{
    float m_fX, m_fY, m_fZ;
};

Waypoint m_aSummon1=
{
	-374.407, -812.079, 59.747
};

Waypoint m_aSummon2=
{
	-383.870, -812.184, 59.747
};

Waypoint m_aSummonElite = 
{
	-378.814, -791.131, 44.156
};

/*######
## boss_novos
######*/

struct MANGOS_DLL_DECL boss_novosAI : public Scripted_NoMovementAI
{
    boss_novosAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;

	bool m_bShielded;

	uint32 m_uiSummonTimer;
	uint32 m_uiSummonElite;
	uint32 m_uiSummonHandler;

	uint32 m_uiHandlerCount;
	uint32 m_uiCrystalCount;

    uint32 m_uiWrathTimer;
    uint32 m_uiFrostboltTimer;
    uint32 m_uiArcaneBlastTimer;
    uint32 m_uiBlizzardTimer;

    void Reset()
    {
		m_uiSummonTimer = urand(1000,2000);
		m_uiSummonElite = urand(9000,12000);
		m_uiSummonHandler = urand(15000,16000);

		m_uiHandlerCount = 0;
		m_uiCrystalCount = 0;

        m_uiWrathTimer = urand(8000,12000);
        m_uiFrostboltTimer = urand(2000,3000);
        m_uiArcaneBlastTimer = urand(6000,6500);
        m_uiBlizzardTimer = urand(10000,20000);

		m_bShielded=false;
		m_creature->InterruptNonMeleeSpells(true);

		if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_NOVOS, NOT_STARTED);
		}
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);
		m_bShielded = true;
		m_creature->CastSpell(m_creature,SPELL_ARCANE_FIELD,false);
		if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_NOVOS, IN_PROGRESS);
		}
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);
		if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_NOVOS,DONE);
		}
    }

	void HandlerDied(Creature * pHandler)
	{
		m_uiCrystalCount++;
		if(m_uiCrystalCount >=4)
		{
			m_bShielded = false;
			m_creature->InterruptNonMeleeSpells(true);
		}
	}

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

		if(m_bShielded)
		{
			if (m_uiSummonTimer < uiDiff)
			{
				Creature *pCreature = m_creature->SummonCreature(NPC_SHADOWCASTER, m_aSummon1.m_fX, m_aSummon1.m_fY, m_aSummon1.m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 10);
                //pCreature->RemoveMonsterMoveFlag(MONSTER_MOVE_WALK);
                pCreature->AddThreat(m_creature->getVictim(),0.0f);
                pCreature->SetInCombatWith(m_creature->getVictim());
				pCreature = m_creature->SummonCreature(NPC_FETIDTROLL, m_aSummon2.m_fX, m_aSummon2.m_fY, m_aSummon2.m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 10);
				//pCreature->RemoveMonsterMoveFlag(MONSTER_MOVE_WALK);
                pCreature->AddThreat(m_creature->getVictim(),0.0f);
                pCreature->SetInCombatWith(m_creature->getVictim());
				m_uiSummonTimer = urand(9000, 10000);
			}else m_uiSummonTimer -= uiDiff;

			if (m_uiSummonElite < uiDiff)
			{
				Creature *pCreature = m_creature->SummonCreature(NPC_HULKING, m_aSummonElite.m_fX, m_aSummonElite.m_fY, m_aSummonElite.m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 10);
				//pCreature->RemoveMonsterMoveFlag(MONSTER_MOVE_WALK);
                pCreature->AddThreat(m_creature->getVictim(),0.0f);
                pCreature->SetInCombatWith(m_creature->getVictim());
				m_uiSummonElite = urand(14000, 16000);
			}else m_uiSummonElite -= uiDiff;

			if (m_uiHandlerCount < 4 && m_uiSummonHandler < uiDiff)
			{
				Creature *pCreature = m_creature->SummonCreature(NPC_HANDLER, m_aSummonElite.m_fX, m_aSummonElite.m_fY, m_aSummonElite.m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 10);
				//pCreature->RemoveMonsterMoveFlag(MONSTER_MOVE_WALK);
                pCreature->AddThreat(m_creature->getVictim(),0.0f);
                pCreature->SetInCombatWith(m_creature->getVictim());
				m_uiSummonHandler = urand(25000, 36000);
				m_uiHandlerCount++;
			}else m_uiSummonHandler -= uiDiff;

		} else {
			if (m_uiWrathTimer < uiDiff)
			{
                DoCast(this->SelectUnit(SELECT_TARGET_RANDOM,0),(m_bIsHeroicMode) ? SPELL_WRATH_H : SPELL_WRATH);
                m_uiWrathTimer = urand(12000,13500);
            } else m_uiWrathTimer -= uiDiff;

            if (m_uiFrostboltTimer < uiDiff)
			{
                DoCast(this->SelectUnit(SELECT_TARGET_RANDOM,0),(m_bIsHeroicMode) ? SPELL_FROSTBOLT_H : SPELL_FROSTBOLT);
                m_uiFrostboltTimer = urand(4000,5500);
            } else m_uiFrostboltTimer -= uiDiff;

            if (m_uiArcaneBlastTimer < uiDiff)
			{
                DoCast(m_creature->getVictim(),(m_bIsHeroicMode) ? SPELL_ARCANE_BLAST_H : SPELL_ARCANE_BLAST);
                m_uiArcaneBlastTimer = urand(10000,11000);
            } else m_uiArcaneBlastTimer -= uiDiff;

            if (m_uiBlizzardTimer < uiDiff)
			{
                DoCast(this->SelectUnit(SELECT_TARGET_RANDOM,0),(m_bIsHeroicMode) ? SPELL_BLIZZARD_H : SPELL_BLIZZARD);
                m_uiBlizzardTimer = urand(16000,17500);
            } else m_uiBlizzardTimer -= uiDiff;
		}
        //DoMeleeAttackIfReady();
    }
	
};

struct MANGOS_DLL_DECL mob_crystal_handlerAI : public ScriptedAI
{
    mob_crystal_handlerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;

	void Reset()
	{

	}

	void JustDied(Unit* pKiller)
    {
		if(m_pInstance)
		{
			m_pInstance->SetData(GO_RITUAL_CRYSTAL_1,0);
			Creature* pNovos = ((Creature *)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_NOVOS)));
			((boss_novosAI*)(pNovos->AI()))->HandlerDied(m_creature);
		}
    }
};

CreatureAI* GetAI_boss_novos(Creature* pCreature)
{
    return new boss_novosAI(pCreature);
}

CreatureAI* GetAI_mob_crystal_handler(Creature* pCreature)
{
    return new mob_crystal_handlerAI(pCreature);
}

void AddSC_boss_novos()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_novos";
    newscript->GetAI = &GetAI_boss_novos;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_crystal_handler";
    newscript->GetAI = &GetAI_mob_crystal_handler;
    newscript->RegisterSelf();
}
