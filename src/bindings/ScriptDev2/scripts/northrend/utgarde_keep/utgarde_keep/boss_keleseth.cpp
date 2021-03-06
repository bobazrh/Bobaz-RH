/* ScriptData
SDName: Boss_Prince_Keleseth
SD%Complete: 90
SDComment:  Needs Prince Movements, Needs adjustments to blizzlike timers, Needs Shadowbolt castbar, Needs right Ressurect Visual, Needs Some Heroic Spells
SDCategory: Utgarde Keep
EndScriptData */

#include "precompiled.h"
#include "def_utgarde_keep.h"

#define SPELL_SHADOWBOLT                         43667
#define SPELL_SHADOWBOLT_HEROIC                  59389
#define SPELL_FROST_TOMB                         48400
#define SPELL_FROST_TOMB_SUMMON                  42714
#define SPELL_DECREPIFY                          42702
#define SPELL_BONE_ARMOR			 59386
#define SPELL_SCOURGE_RESSURRECTION              42704
#define CREATURE_FROSTTOMB                       23965
#define CREATURE_SKELETON                        23970

#define SAY_AGGRO                                -1574000
#define SAY_KILL                                 -1574001
#define SAY_DEATH                                -1574002
#define SAY_FROST_TOMB                           -1574003
#define SAY_SKELETONS                            -1574004

#define SKELETONSPAWN_Z                          42.8668

float SkeletonSpawnPoint[5][5]=
{
    {156.4559, 259.2093},
    {156.2559, 259.0093},
    {156.3559, 259.2093},
    {156.2559, 259.2093},
    {156.2559, 259.3093},
};

float AttackLoc[3]={197.636, 194.046, 40.8164};

struct MANGOS_DLL_DECL mob_frost_tombAI : public ScriptedAI
{
    mob_frost_tombAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        FrostTombGUID = 0;
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsHeroicMode = pCreature->GetMap()->IsHeroic();
        Reset();
    }

    uint64 FrostTombGUID;
    ScriptedInstance* m_pInstance;
    bool m_bIsHeroicMode;

    void SetPrisoner(Unit* uPrisoner)
    {
        FrostTombGUID = uPrisoner->GetGUID();
    }

    void Reset(){ FrostTombGUID = 0; }
    void Aggro(Unit* who) {}
    void AttackStart(Unit* who) {}
    void MoveInLineOfSight(Unit* who) {}

    void JustDied(Unit *killer)
    {
        if(FrostTombGUID)
        {
            Unit* FrostTomb = Unit::GetUnit((*m_creature),FrostTombGUID);
            if(FrostTomb)
                FrostTomb->RemoveAurasDueToSpell(SPELL_FROST_TOMB);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        Unit* temp = Unit::GetUnit((*m_creature),FrostTombGUID);
        if((temp && temp->isAlive() && !temp->HasAura(SPELL_FROST_TOMB,0)) || !temp )
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
    }
};

struct MANGOS_DLL_DECL  boss_kelesethAI : public ScriptedAI
{
    boss_kelesethAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
        Heroic = c->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;

    uint32 FrostTombTimer;
    uint32 SummonSkeletonsTimer;
    uint32 RespawnSkeletonsTimer;
    uint32 ShadowboltTimer;
    uint64 SkeletonGUID[5];
    bool Skeletons;
    bool Heroic;
    bool RespawnSkeletons;

    void Reset()
    {
        ShadowboltTimer = 1000;
        Skeletons = false;

        ResetTimer();

        if(pInstance)
            pInstance->SetData(DATA_PRINCEKELESETH, NOT_STARTED);
    }

    void KilledUnit(Unit *victim)
    {
        if(victim == m_creature)
            return;

        DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* killer)
    {
        if(pInstance)
            pInstance->SetData(DATA_PRINCEKELESETH, DONE);

        DoScriptText(SAY_DEATH, m_creature);
    }

    void Aggro(Unit* who)
    {
        if(pInstance)
            pInstance->SetData(DATA_PRINCEKELESETH, IN_PROGRESS);

        DoScriptText(SAY_AGGRO, m_creature);
	m_creature->SetInCombatWithZone();
    }

    void ResetTimer(uint32 inc = 0)
    {
        SummonSkeletonsTimer = 5000 + inc;
        FrostTombTimer = 28000 + inc;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        if(ShadowboltTimer < diff)
        {
           m_creature->CastSpell(m_creature->getVictim(), Heroic ? SPELL_SHADOWBOLT_HEROIC : SPELL_SHADOWBOLT,false);
            ShadowboltTimer = 10000;
        }else ShadowboltTimer -= diff;

        if(!Skeletons)
            if((SummonSkeletonsTimer < diff))
            {
                Creature* Skeleton;
                DoScriptText(SAY_SKELETONS, m_creature);
                for(uint8 i = 0; i < 5; ++i)
                {
                    Skeleton = m_creature->SummonCreature(CREATURE_SKELETON, SkeletonSpawnPoint[i][0], SkeletonSpawnPoint[i][1] , SKELETONSPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN,20000);
                    if(Skeleton)
                    {
                        Skeleton->RemoveMonsterMoveFlag(MONSTER_MOVE_WALK);
                        Skeleton->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX(), m_creature->GetPositionY() , m_creature->GetPositionZ());
                        Skeleton->AddThreat(m_creature->getVictim(), 0.0f);
			            m_creature->SetInCombatWithZone();
                    }
                }
                Skeletons = true;
            }else SummonSkeletonsTimer -= diff;

        if(FrostTombTimer < diff)
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if(target && target->isAlive() && target->GetTypeId() == TYPEID_PLAYER)
            {
                //DoCast(target, SPELL_FROST_TOMB_SUMMON, true);
                Creature* Chains = m_creature->SummonCreature(CREATURE_FROSTTOMB, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 20000);
                if(Chains)
                {
                    ((mob_frost_tombAI*)Chains->AI())->SetPrisoner(target);
                    Chains->CastSpell(target, SPELL_FROST_TOMB, true);

                    DoScriptText(SAY_FROST_TOMB, m_creature);
                }
            }
            FrostTombTimer = 15000;
        }else FrostTombTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct MANGOS_DLL_DECL  mob_vrykul_skeletonAI : public ScriptedAI
{
    mob_vrykul_skeletonAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    ScriptedInstance *pInstance;
    uint32 Respawn_Time;
    uint64 Target_Guid;
    uint32 Decrepify_Timer;
    uint32 BoneArmor_Timer;

    bool isDead;

    void Reset()
    {
        Respawn_Time = 11000+rand()%2000;
        Decrepify_Timer = 1000 + rand()%5000;
	BoneArmor_Timer = 2000 + rand()%2000;
        isDead = false;
    }

    void Aggro(Unit *who){}
    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if(done_by->GetGUID() == m_creature->GetGUID())
            return;

        if(damage >= m_creature->GetHealth())
        {
            PretendToDie();
            damage = 0;
        }
    }


    void PretendToDie()
    {
        isDead = true;
        m_creature->InterruptNonMeleeSpells(true);
        m_creature->RemoveAllAuras();
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->GetMotionMaster()->MovementExpired(false);
        m_creature->GetMotionMaster()->MoveIdle();
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
    };

    void Resurrect()
    {
        isDead = false;
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        m_creature->CastSpell(m_creature,SPELL_SCOURGE_RESSURRECTION,true);

        if(m_creature->getVictim())
        {
            m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
            m_creature->AI()->AttackStart(m_creature->getVictim());
        }
        else
            m_creature->GetMotionMaster()->Initialize();
    };

    void UpdateAI(const uint32 diff)
    {
	if(!pInstance)
	{
		return;
	}
        if(pInstance->GetData(DATA_PRINCEKELESETH) == IN_PROGRESS)
        {
            if(isDead)
            {
                if(Respawn_Time < diff)
                {
                    Resurrect();
                    Respawn_Time = 12000+rand()%4000;
                }else Respawn_Time -= diff;
            }
            else
            {
                if(!m_creature->SelectHostilTarget() || !m_creature->getVictim())
                    return;

		if(BoneArmor_Timer < diff)
		{
		   DoCast(m_creature,SPELL_BONE_ARMOR);
		   BoneArmor_Timer = 8000 + rand()%6000;
		}else BoneArmor_Timer -= diff;

                if(Decrepify_Timer < diff)
                {
                    DoCast(m_creature->getVictim(),SPELL_DECREPIFY);
                    Decrepify_Timer = 30000;
                }else Decrepify_Timer -= diff;

                DoMeleeAttackIfReady();
            }
        }else
        {
            if(m_creature->isAlive())
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }

    }
};

CreatureAI* GetAI_mob_frost_tomb(Creature *_Creature)
{
    return new mob_frost_tombAI(_Creature);
}

CreatureAI* GetAI_boss_keleseth(Creature *_Creature)
{
    return new boss_kelesethAI (_Creature);
}

CreatureAI* GetAI_mob_vrykul_skeleton(Creature *_Creature)
{
    return new mob_vrykul_skeletonAI (_Creature);
}

void AddSC_boss_keleseth()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_keleseth";
    newscript->GetAI = &GetAI_boss_keleseth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_frost_tomb";
    newscript->GetAI = &GetAI_mob_frost_tomb;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_vrykul_skeleton";
    newscript->GetAI = GetAI_mob_vrykul_skeleton;
    newscript->RegisterSelf();
}
