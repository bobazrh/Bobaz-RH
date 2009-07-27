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
SDName: Boss_Keristrasza
SD%Complete: 10%
SDComment:
SDCategory: Nexus
EndScriptData */

#include "precompiled.h"
#include "def_nexus.h"

enum
{
    SAY_AGGRO                   = -1576016,
    SAY_CRYSTAL_NOVA            = -1576017,
    SAY_ENRAGE                  = -1576018,
    SAY_KILL                    = -1576019,
    SAY_DEATH                   = -1576020,

    SPELL_CRYSTALFIRE_BREATH    = 48096,
    SPELL_CRYSTALFIRE_BREATH_H  = 57091,

    SPELL_CRYSTALLIZE           = 48179,
    
    SPELL_CRYSTAL_CHAINS        = 50997,
    SPELL_CRYSTAL_CHAINS_H      = 57050,

    SPELL_TAIL_SWEEP            = 50155,
    SPELL_INTENSE_COLD          = 48095,

    SPELL_ENRAGE                = 8599,

    SPELL_PRISON		= 47854
};

/*######
## boss_keristrasza
######*/

struct MANGOS_DLL_DECL boss_keristraszaAI : public ScriptedAI
{
    boss_keristraszaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;
	bool m_bEnraged;

	uint32 m_uiIntenseColdTimer;
	uint32 m_uiTailSweepTimer;
	uint32 m_uiBreathTimer;
	uint32 m_uiChainsTimer;
	uint32 m_uiNovaTimer;

	SpellEntry const * m_IntenseColdInfo;

    void Reset() 
    {
		m_uiIntenseColdTimer = 1000;
		m_uiTailSweepTimer = 5000 + rand()%2000;
		m_uiBreathTimer = 8000 + rand()%4000;
		m_uiChainsTimer = 10000+rand()%5000;
		m_uiNovaTimer = 20000+rand()%10000;
		m_bEnraged = false;

		m_IntenseColdInfo= sSpellStore.LookupEntry(SPELL_INTENSE_COLD);

		if(m_pInstance)
			m_pInstance->SetData(NPC_KERISTRASZA, NOT_STARTED);
    }

    void Aggro(Unit* pWho)
    {
		if(m_pInstance)
		{
			if(	(m_pInstance->GetData(NPC_ORMOROK) != DONE) ||
				(m_pInstance->GetData(NPC_ANOMALUS) != DONE)||
				(m_pInstance->GetData(NPC_TELESTRA) != DONE)
			  )
			{
				if (m_creature->getVictim())
				{
					DoCast(m_creature->getVictim(),64487);
				} else {
					if(m_pInstance)
						m_pInstance->SetData(NPC_KERISTRASZA, IN_PROGRESS);
				}
			}
		}
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
		if(m_pInstance)
			m_pInstance->SetData(NPC_KERISTRASZA, DONE);
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

		if (m_uiTailSweepTimer < uiDiff)
		{
			DoCast(NULL,SPELL_TAIL_SWEEP);
			m_uiTailSweepTimer = 5000 + rand()%2000;
		} else m_uiTailSweepTimer -= uiDiff;

		if (m_uiIntenseColdTimer < uiDiff)
		{
			DoCastIntense();
			m_uiIntenseColdTimer = 1000;
		} else m_uiIntenseColdTimer -= uiDiff;

		if (m_uiBreathTimer < uiDiff)
		{
			DoCast(m_creature->getVictim(),(m_bIsHeroicMode) ? SPELL_CRYSTALFIRE_BREATH_H : SPELL_CRYSTALFIRE_BREATH);
			m_uiBreathTimer = 8000+rand()%4000;
		} else m_uiBreathTimer -= uiDiff;

		if (m_bIsHeroicMode && m_uiNovaTimer < uiDiff)
		{
			DoScriptText(SAY_CRYSTAL_NOVA,m_creature);
			DoCast(m_creature->getVictim(),SPELL_CRYSTALLIZE);
			m_uiNovaTimer = 20000+rand()%10000;
		} else m_uiNovaTimer -= uiDiff;

		if (m_uiChainsTimer < uiDiff)
		{
			if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
			{
				DoCast(m_creature->getVictim(),(m_bIsHeroicMode) ? SPELL_CRYSTAL_CHAINS_H : SPELL_CRYSTAL_CHAINS);
			}
			m_uiChainsTimer = 10000+rand()%5000;
		} else m_uiChainsTimer -= uiDiff;

		if ( !m_bEnraged && (m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < 25)
		{
			DoScriptText(SAY_ENRAGE, m_creature);
			DoCast(m_creature,SPELL_ENRAGE);
			m_bEnraged = true;
		}

        DoMeleeAttackIfReady();
    }

	void DoCastIntense()
	{
		Unit *pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);
		for(int i=1; pTarget && i<41;i++)
		{
			Aura *Aur = CreateAura(m_IntenseColdInfo, 0, NULL,pTarget,m_creature);
			pTarget->AddAura(Aur);
			Aur = CreateAura(m_IntenseColdInfo, 1, NULL,pTarget,m_creature);
			pTarget->AddAura(Aur);
			pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 1);
		}
	}
};

CreatureAI* GetAI_boss_keristrasza(Creature* pCreature)
{
    return new boss_keristraszaAI(pCreature);
}

void AddSC_boss_keristrasza()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_keristrasza";
    newscript->GetAI = &GetAI_boss_keristrasza;
    newscript->RegisterSelf();
}
