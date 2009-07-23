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
SDName: Boss_Telestra
SD%Complete: 10%
SDComment:
SDCategory: Nexus
EndScriptData */

#include "precompiled.h"

enum
{
    SAY_AGGRO               = -1576000,
    SAY_SPLIT_1             = -1576001,
    SAY_SPLIT_2             = -1576002,
    SAY_MERGE               = -1576003,
    SAY_KILL                = -1576004,
    SAY_DEATH               = -1576005,

    SPELL_FIREBOMB          = 47773,
    SPELL_FIREBOMB_H        = 56934,

    SPELL_ICE_NOVA          = 47772,
    SPELL_ICE_NOVA_H        = 56935,

    SPELL_GRAVITY_WELL      = 47756,

    SPELL_SUMMON_CLONES     = 47710,

    SPELL_ARCANE_VISUAL     = 47704,
    SPELL_FIRE_VISUAL       = 47705,
    SPELL_FROST_VISUAL      = 47706,

    SPELL_SUMMON_FIRE       = 47707,
    SPELL_SUMMON_ARCANE     = 47708,
    SPELL_SUMMON_FROST      = 47709,

    SPELL_FIRE_DIES         = 47711,
    SPELL_ARCANE_DIES       = 47713,
    SPELL_FROST_DIES        = 47712,

    SPELL_SPAWN_BACK_IN     = 47714,

    NPC_TELEST_FIRE         = 26928,
    NPC_TELEST_ARCANE       = 26929,
    NPC_TELEST_FROST        = 26930
};

/*######
## boss_telestra
######*/

uint32 Splits = {50,15};

struct MANGOS_DLL_DECL boss_telestraAI : public ScriptedAI
{
    boss_telestraAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;
	bool m_bIsNotCasting;
	bool m_bIsSplit;
	uint32 m_uiCastBar;
	uint32 m_uiFireBomb;
	uint32 m_uiFrostNova;
	uint32 m_uiGravityWell;

	uint32 m_uiAdds;
	uint32 m_uiSplitCount;

	Creature * pArcaneAdd = NULL;
	Creature * pFrostAdd = NULL;
	Creature * pFireAdd = NULL;

    void Reset() 
    {
		m_uiFireBomb = 1000+rand()%1000;
		m_uiFrostNova = 10000+rand()%1000;
		m_uiGravityWell = 18000+rand()%4000;
		m_uiCastBar = 2000;
		m_bIsNotCasting = true;
		m_bIsSplit = false;
		m_uiAdds=0;
		m_uiSplitCount = (m_bIsHeroicMode) ? 1 : 0;
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (rand()%2)
            DoScriptText(SAY_KILL, m_creature);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;
		
		if(m_bIsSplit)
		{
			CheckAddDied();
			if(m_uiAdds <= 0)
			{
				DoUnSplit();
				m_bIsSplit = false;
			}
			return;
		}

		if(bIsNotCasting)
		{
			if (m_uiFireBomb < uiDiff)
			{
				DoCast(m_creature->getVictim(),m_bIsHeroicMode ? SPELL_FIREBOMB_H : SPELL_FIREBOMB);
				m_uiFireBomb = 4000 + rand()%2000;
				m_bIsNotCasting = false;
				m_creature->GetMotionMaster()->MoveIdle();
			} else m_uiFireBomb -= uiDiff;

			if (m_uiFrostNova < uiDiff)
			{
				DoCast(m_creature->getVictim(),m_bIsHeroicMode ? SPELL_ICE_NOVA_H : SPELL_ICE_NOVA);
				m_uiFrostNova = 6000 + rand()%4000;
				m_bIsNotCasting = false;
				m_creature->GetMotionMaster()->MoveIdle();
			} else m_uiFrostNova -= uiDiff;

			if (m_uiGravityWell < uiDiff)
			{
				DoCast(m_creature,SPELL_GRAVITY_WELL);
				m_uiGravityWell = 17000 + rand()%4000;
				m_bIsNotCasting = false;
				m_uiCastBar = 6000;
				m_creature->GetMotionMaster()->MoveIdle();
			} else m_uiGravityWell -= uiDiff;

			if(m_uiSplitCount>=0 && 
				m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < Splits[m_uiSplitCount])
			{
				m_bIsSplit = true;
				m_uiSplitCount--;
				DoSplit();
			}
			
			DoMeleeAttackIfReady();
		} else {
			if (m_uiCastBar < uiDiff)
			{
				m_uiCastBar = 2000;
				m_bIsNotCasting = true;
				DoMove();
			} else m_uiCastBar -= uiDiff;
		}
    }

	void CheckAddDied(Creature * pAdd)
	{
		if(pArcaneAdd && !pArcaneAdd->isAlive())
		{
			pArcaneAdd = NULL;
			m_uiAdds--;
		}
		if(pFireAdd && !pFireAdd->isAlive())
		{
			pFireAdd = NULL;
			m_uiAdds--;
		}
		if(pFireAdd && !pFireAdd->isAlive())
		{
			pFireAdd = NULL;
			m_uiAdds--;
		}
	}

	void DoSplit()
	{
		if(rand()%2)
			DoScriptText(SAY_SPLIT_1, m_creature);
		else
			DoScriptText(SAY_SPLIT_2, m_creature);

		DoUnTarget();
		m_creature->RemoveAllAuras();
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetVisibility(VISIBILITY_OFF);
		m_creature->GetMotionMaster()->MoveIdle();
		
		DoResetThreat();
        DoStopAttack();
		
		float x,y,z=0.0f;
		
		m_creature->GetClosePoint(x,y,z,m_creature->GetObjectSize(),2.0f);
		pArcaneAdd=m_creature->SummonCreature(NPC_TELEST_ARCANE, x,y,z TEMPSUMMON_CORPSE_DESPAWN, 30000);
		
		m_creature->GetClosePoint(x,y,z,m_creature->GetObjectSize(),2.0f);
		pFireAdd=m_creature->SummonCreature(NPC_TELEST_FIRE, x,y,z TEMPSUMMON_CORPSE_DESPAWN, 30000);
		
		m_creature->GetClosePoint(x,y,z,m_creature->GetObjectSize(),2.0f);
		pFrostAdd=m_creature->SummonCreature(NPC_TELEST_FROST, x,y,z TEMPSUMMON_CORPSE_DESPAWN, 30000);
	}

	void DoUnSplit()
	{
		m_creature->SetVisibility(VISIBILITY_ON);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		DoMove();
		DoCast(m_creature,SPELL_SPAWN_BACK_IN,true);
		
		DoScriptText(SAY_MERGE, m_creature);
	}

	// helpers

	void DoUnTarget()
	{
		for (int tryi = 0; tryi < 41; ++tryi)
        {
            Unit *targetpl = SelectUnit(SELECT_TARGET_RANDOM, 0);
			if(!targetpl)break;
            if (targetpl->GetTypeId() == TYPEID_PLAYER)
            {
                Group *grp = ((Player *)targetpl)->GetGroup();
                if (grp)
                {
                    for (int ici = 0; ici < TARGETICONCOUNT; ++ici)
                    {
                        grp->SetTargetIcon(ici, 0);
                    }
                }
                break;
            }
        }
	}

	void DoMove()
	{
		Unit * victim;
        if(victim = m_creature->getVictim()) 
		{
			m_creature->SendMeleeAttackStart(victim);
			m_creature->GetMotionMaster()->MoveChase(victim);
			m_creature->Attack(victim, false);
		} 
	}
};

CreatureAI* GetAI_boss_telestra(Creature* pCreature)
{
    return new boss_telestraAI(pCreature);
}

void AddSC_boss_telestra()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_telestra";
    newscript->GetAI = &GetAI_boss_telestra;
    newscript->RegisterSelf();
}
