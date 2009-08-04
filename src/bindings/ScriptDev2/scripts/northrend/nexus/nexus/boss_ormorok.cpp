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
SDName: Boss_Ormorok
SD%Complete: 10%
SDComment:
SDCategory: Nexus
EndScriptData */

#include "precompiled.h"
#include "def_nexus.h"

enum
{
    SAY_AGGRO                   = -1576011,
    SAY_KILL                    = -1576012,
    SAY_FRENZY                  = -1576013,
    SAY_ICESPIKE                = -1576014,
    SAY_DEATH                   = -1576015,

    SPELL_REFLECTION            = 47981,

    SPELL_CRYSTAL_SPIKES        = 47958,
    SPELL_CRYSTAL_SPIKES_H1     = 57082,
    SPELL_CRYSTAL_SPIKES_H2     = 57083,

    SPELL_FRENZY                = 48017,
    SPELL_FRENZY_H              = 57086,

    SPELL_TRAMPLE               = 48016,
    SPELL_TRAMPLE_H             = 57066,

    SPELL_SUMMON_TANGLER_H      = 61564
};

/*######
## boss_ormorok
######*/

struct MANGOS_DLL_DECL boss_ormorokAI : public ScriptedAI
{
    boss_ormorokAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;

    uint32 m_TrampleTimer;
    uint32 m_SpellReflectionTimer;
    uint32 m_SpikeTimer;
    uint32 m_TanglerTimer;
    bool m_Enraged;

    void Reset() 
    {
		m_TrampleTimer=1000 + rand()%2000;
		m_SpikeTimer=15000+rand()%5000;
		m_SpellReflectionTimer=10000+rand()%5000;
		m_Enraged = false;
		if(m_pInstance)
				m_pInstance->SetData(NPC_ORMOROK, NOT_STARTED);
    }

    void Aggro(Unit* pWho)
    {
		if(m_pInstance)
			m_pInstance->SetData(NPC_ORMOROK, IN_PROGRESS);
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
		if(m_pInstance)
			m_pInstance->SetData(NPC_ORMOROK, DONE);
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

	if(m_TrampleTimer < uiDiff)
	{
	    DoCast(m_creature->getVictim(), (!m_bIsHeroicMode) ? SPELL_TRAMPLE : SPELL_TRAMPLE_H);
	    m_TrampleTimer = 2000+rand()%2000;
	} else m_TrampleTimer -= uiDiff;

	if(m_SpikeTimer < uiDiff)
	{
	    DoCast(m_creature, (!m_bIsHeroicMode) ? SPELL_CRYSTAL_SPIKES : SPELL_CRYSTAL_SPIKES_H1);
	    DoScriptText(SAY_ICESPIKE, m_creature);
	    m_SpikeTimer = 20000;
	} else m_SpikeTimer -= uiDiff;

	if(m_SpellReflectionTimer < uiDiff)
	{
	   DoCast(m_creature, SPELL_REFLECTION);
	   m_SpellReflectionTimer = 8000+rand()%15000;
	} else m_SpellReflectionTimer -= uiDiff;

	if(!m_Enraged && m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 25)
	{
	   DoScriptText(SAY_FRENZY, m_creature);
	   m_creature->InterruptNonMeleeSpells(false);
	   DoCast(m_creature, (!m_bIsHeroicMode) ? SPELL_FRENZY : SPELL_FRENZY_H);
	   m_Enraged = true;
	}

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ormorok(Creature* pCreature)
{
    return new boss_ormorokAI(pCreature);
}

void AddSC_boss_ormorok()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_ormorok";
    newscript->GetAI = &GetAI_boss_ormorok;
    newscript->RegisterSelf();
}
