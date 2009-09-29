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
#include "def_nexus.h"

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

uint32 Splits[2] = {50,15};

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

	Creature * pArcaneAdd;
	Creature * pFrostAdd;
	Creature * pFireAdd;

    void Reset() 
    {
		m_uiFireBomb = 1000+rand()%1000;
		m_uiFrostNova = 10000+rand()%1000;
		m_uiGravityWell = 18000+rand()%4000;
		m_uiCastBar = 2000;
		m_bIsNotCasting = true;
		m_bIsSplit = false;
		m_uiAdds=0;
		m_uiSplitCount = 0;

		pArcaneAdd = NULL;
        pFrostAdd = NULL;
        pFireAdd = NULL;

		if(m_pInstance)
            m_pInstance->SetData(NPC_TELESTRA, NOT_STARTED);
    }

    void Aggro(Unit* pWho)
    {
		if(m_pInstance)
            m_pInstance->SetData(NPC_TELESTRA, IN_PROGRESS);
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
		if(m_pInstance)
            m_pInstance->SetData(NPC_TELESTRA, DONE);
        DoScriptText(SAY_DEATH, m_creature);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (urand(0, 1))
            DoScriptText(SAY_KILL, m_creature);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;
		
		if(m_bIsSplit)
		{
			CheckAddDied();
			if(m_uiAdds <= 0 && pFireAdd == NULL && pArcaneAdd == NULL && pFrostAdd == NULL)
			{
				DoUnSplit();
				m_bIsSplit = false;
			}
			return;
		}

		if(m_bIsNotCasting)
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

			if(m_uiSplitCount<((m_bIsHeroicMode) ? 2 : 1 )&& 
				m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < Splits[m_uiSplitCount])
			{
				m_bIsSplit = true;
				m_uiSplitCount++;
				m_uiAdds=3;
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

	void CheckAddDied()
	{
		if(pArcaneAdd && !pArcaneAdd->isAlive())
		{
			pArcaneAdd = NULL;
			m_uiAdds--;
		}
		if(pFrostAdd && !pFrostAdd->isAlive())
		{
			pFrostAdd = NULL;
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
		
		m_creature->GetClosePoint(x,y,z,m_creature->GetObjectSize(),3.5f);
		pArcaneAdd=m_creature->SummonCreature(NPC_TELEST_ARCANE, x,y,z,2.0f, TEMPSUMMON_CORPSE_DESPAWN, 30000);
		if(pArcaneAdd){
			pArcaneAdd->CastSpell(pArcaneAdd,SPELL_ARCANE_VISUAL,true);
		}
		
		m_creature->GetClosePoint(x,y,z,m_creature->GetObjectSize(),3.5f);
		pFireAdd=m_creature->SummonCreature(NPC_TELEST_FIRE, x,y,z,0.0f, TEMPSUMMON_CORPSE_DESPAWN, 30000);
		if(pFireAdd){
			pFireAdd->CastSpell(pFireAdd,SPELL_FIRE_VISUAL,true);	
		}
		
		m_creature->GetClosePoint(x,y,z,m_creature->GetObjectSize(),3.5f);
		pFrostAdd=m_creature->SummonCreature(NPC_TELEST_FROST, x,y,z,4.0f, TEMPSUMMON_CORPSE_DESPAWN, 30000);
		if(pFrostAdd){
                       pFrostAdd->CastSpell(pFrostAdd,SPELL_FROST_VISUAL,true); 
                }

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
