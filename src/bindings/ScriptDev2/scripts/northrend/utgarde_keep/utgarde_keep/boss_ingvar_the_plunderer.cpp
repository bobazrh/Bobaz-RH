/* ScriptData
SDName: Boss_Ingvar_The_Plunderer
SD%Complete: 95
SDComment: Some Problems with Annhylde Movement, Blizzlike Timers
SDCategory: Udgarde Keep
EndScriptData */

#include "precompiled.h"
#include "sc_creature.h"
#include "def_utgarde_keep.h"

//Yells Ingvar
#define SAY_AGGRO_1                         -1574015
#define SAY_AGGRO_2                         -1574022

#define SAY_DEAD_1                          -1574017
#define SAY_DEAD_2                          -1574024

#define SAY_KILL_1                          -1574016
#define SAY_KILL_2                          -1574023

//Ingvar Spells human form
#define MOB_INGVAR_HUMAN                            23954
#define SPELL_CLEAVE                                42724
#define SPELL_SMASH                                 42669
#define H_SPELL_SMASH                               59706
#define SPELL_STAGGERING_ROAR                       42708
#define H_SPELL_STAGGERING_ROAR                     59708
#define SPELL_ENRAGE                                42705
#define H_SPELL_ENRAGE                              59707

#define MOB_ANNHYLDE_THE_CALLER                     24068
#define SPELL_INGVAR_FEIGN_DEATH                    42795
#define SPELL_SUMMON_BANSHEE                        42912
#define SPELL_SCOURG_RESURRECTION_EFFEKTSPAWN       42863 //Spawn resurrecteffekt around Ingvar

#define MODEL_INGVAR_UNDEAD                         26351
#define MODEL_INGVAR_HUMAN                          21953

//Ingvar Spells undead form
#define MOB_INGVAR_UNDEAD                           23980
#define SPELL_DARK_SMASH                            42723
#define H_SPELL_DARK_SMASH			    59709
#define SPELL_DREADFUL_ROAR                         42729
#define H_SPELL_DREADFUL_ROAR                       59734
#define SPELL_WOE_STRIKE                            42730
#define H_SPELL_WOE_STRIKE                          59735

#define ENTRY_THROW_TARGET                          23996
#define SPELL_SHADOW_AXE_SUMMON                     42749

struct MANGOS_DLL_DECL boss_ingvar_the_plundererAI : public ScriptedAI
{
    boss_ingvar_the_plundererAI(Creature *c) : ScriptedAI(c) 
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        HeroicMode = c->GetMap()->IsHeroic();
        Reset();
    }

    ScriptedInstance* pInstance;

    bool HeroicMode;
    bool undead;
    bool event_inProgress;

    uint32 Cleave_Timer;
    uint32 Smash_Timer;
    uint32 Enrage_Timer;
    uint32 Roar_Timer;
    uint32 SpawnResTimer;
    uint32 wait_Timer;


    void Reset()
    {
        if(pInstance)
            pInstance->SetData(DATA_INGVAR, NOT_STARTED);

        if(undead) // Visual Hack
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_INGVAR_HUMAN);

        undead = false;
        event_inProgress = false;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        Cleave_Timer = 2000;
        Smash_Timer = 5000;
        Enrage_Timer = 10000;
        Roar_Timer = 15000;

        SpawnResTimer = 3000;

        wait_Timer = 0;
    }

    void DamageTaken(Unit *done_by, uint32 &damage) 
    {
        if( damage >= m_creature->GetHealth() && !undead)
        {
            //DoCast(m_creature,SPELL_INGVAR_FEIGN_DEATH,true);  // Dont work ???
            // visuel hack
            m_creature->SetHealth(0);
            m_creature->InterruptNonMeleeSpells(true);
            m_creature->RemoveAllAuras();
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->GetMotionMaster()->MovementExpired(false);
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
            // visuel hack end
            
            event_inProgress = true;
            undead = true;

            DoScriptText(SAY_AGGRO_1,m_creature);
        }

        if(event_inProgress)
        {
            damage = 0;
        }
    }

    void StartZombiePhase()
    {
        undead = true;
        event_inProgress = false;
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
        m_creature->AI()->AttackStart(m_creature->getVictim());

        DoScriptText(SAY_AGGRO_2,m_creature);
    }

    void Aggro(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(DATA_INGVAR, IN_PROGRESS);

        DoScriptText(SAY_AGGRO_1,m_creature);
    }

    void JustDied(Unit* killer)  
    {
        if(pInstance)
            pInstance->SetData(DATA_INGVAR, DONE);

        DoScriptText(SAY_DEAD_2,m_creature);
    }

    void KilledUnit(Unit *victim)
    {
        if(undead)
        {
            DoScriptText(SAY_KILL_1,m_creature);
        }
        else
        {
            DoScriptText(SAY_KILL_2,m_creature);
        }
    }


    void UpdateAI(const uint32 diff)
    {
        if(!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        if(event_inProgress)
        {
            if(SpawnResTimer)
                if(SpawnResTimer< diff)
                {
                    DoCast(m_creature,SPELL_SUMMON_BANSHEE); // Summons direktly on caster position 
                    //DoCast(m_creature,SPELL_SCOURG_RESURRECTION_EFFEKTSPAWN); // Dont needet ?
                    SpawnResTimer = 0;
                }else SpawnResTimer -= diff;

            return;
        }

        // This is used for a spell queue ... the spells should not castet if one spell is already casting
        if(wait_Timer)
            if(wait_Timer < diff)
            {
                wait_Timer = 0;
            }else wait_Timer -= diff;

        if(Cleave_Timer < diff)
        {
            if(!wait_Timer)
            {
                if(undead)
                    DoCast(m_creature->getVictim(),HeroicMode ? H_SPELL_WOE_STRIKE : SPELL_WOE_STRIKE);
                else
                    DoCast(m_creature->getVictim(),SPELL_CLEAVE);
                Cleave_Timer = rand()%5000 + 2000;

                wait_Timer = 1000;
            }
        }else Cleave_Timer -= diff;

        if(Smash_Timer < diff)
        {
            if(!wait_Timer)
            {
                if(undead)
                    DoCast(m_creature->getVictim(),HeroicMode ? H_SPELL_DARK_SMASH : SPELL_DARK_SMASH);
                else
                    DoCast(m_creature->getVictim(),HeroicMode ? H_SPELL_SMASH : SPELL_SMASH);
                Smash_Timer = 10000;

                wait_Timer = 5000;
            }
        }else Smash_Timer -= diff;

        if(!undead)
        {
            if(Enrage_Timer < diff)
            {
                DoCast(m_creature,HeroicMode ? H_SPELL_ENRAGE : SPELL_ENRAGE);
                Enrage_Timer = 10000;
            }else Enrage_Timer -= diff;
        }else // In Undead form used to summon weapon
        {
            if(Enrage_Timer < diff)
            {
                if(!wait_Timer)
                {
                    // Spawn target for Axe
                    Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);
                    if(target)
                    {
                        Creature* temp = m_creature->SummonCreature(ENTRY_THROW_TARGET,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,2000);

                        DoCast(m_creature,SPELL_SHADOW_AXE_SUMMON);
                    }
                    Enrage_Timer = 30000;
                }
            }else Enrage_Timer -= diff;
        }


        if(Roar_Timer < diff)
        {
            if(!wait_Timer)
            {
                if(undead)
                    DoCast(m_creature,HeroicMode ? H_SPELL_DREADFUL_ROAR : SPELL_DREADFUL_ROAR);
                else
                    DoCast(m_creature,HeroicMode ? H_SPELL_STAGGERING_ROAR : SPELL_STAGGERING_ROAR);
                Roar_Timer = 10000;

                wait_Timer = 5000;
            }
        }else Roar_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ingvar_the_plunderer(Creature *_Creature)
{
    return new boss_ingvar_the_plundererAI (_Creature);
}

#define SAY_RESSURECT                       -1574025

//Spells for Annhylde
#define SPELL_SCOURG_RESURRECTION_HEAL              42704 //Heal Max + DummyAura
#define SPELL_SCOURG_RESURRECTION_BEAM              42857 //Channeling Beam of Annhylde
#define SPELL_SCOURG_RESURRECTION_DUMMY             42862 //Some Emote Dummy?
#define SPELL_INGVAR_TRANSFORM                      42796

struct MANGOS_DLL_DECL mob_annhylde_the_callerAI : public ScriptedAI
{
    mob_annhylde_the_callerAI(Creature *c) : ScriptedAI(c) 
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    float x,y,z;
    ScriptedInstance* pInstance;
    uint32 Resurect_Timer;
    uint32 Resurect_Phase;

    void Reset()
    {
        m_creature->AddMonsterMoveFlag(MONSTER_MOVE_SPLINE_FLY);
        m_creature->SetSpeed(MOVE_SWIM , 0.1f);
        m_creature->SetSpeed(MOVE_RUN , 0.1f);
        m_creature->SetSpeed(MOVE_WALK , 0.1f);
        m_creature->SetSpeed(MOVE_FLIGHT , 0.1f);

        m_creature->GetPosition(x,y,z);
        //DoTeleportPlayer(m_creature, x+1, y, z+10, m_creature->GetOrientation());
        Unit* ingvar = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_INGVAR));
        if(ingvar)
        {
            m_creature->GetMotionMaster()->MovePoint(1,x,y,z+15);
            DoScriptText(SAY_RESSURECT,m_creature);
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE){
            error_log("SD2: Wrong movement of Ingvar's resser: %d, %d",type,id);	
	    return;
	}
        Unit* ingvar = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_INGVAR));
        if(ingvar)
        {
            switch (id)
            {
            case 1:
                ingvar->RemoveAura(SPELL_SUMMON_BANSHEE,1);
                ingvar->CastSpell(ingvar,SPELL_SCOURG_RESURRECTION_DUMMY,true);
                DoCast(ingvar,SPELL_SCOURG_RESURRECTION_BEAM);
                Resurect_Timer = 8000;
                Resurect_Phase = 1;
                break;
            case 2:
                m_creature->DealDamage(m_creature,m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                m_creature->RemoveCorpse();
                break;
            }
        }
	else {
	    error_log("SD2: WTF, no Ingvar in ingvar event?");
	}
    }

    void AttackStart(Unit* who) {}
    void MoveInLineOfSight(Unit* who) {}
    void Aggro(Unit *who) {}
    void UpdateAI(const uint32 diff)
    {
        if(Resurect_Timer)
            if(Resurect_Timer < diff)
            {
                if(Resurect_Phase == 1)
                {
                    Unit* ingvar = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_INGVAR));
                    if(ingvar)
                    {
                        ingvar->SetStandState(UNIT_STAND_STATE_STAND);
                        ingvar->CastSpell(ingvar,SPELL_SCOURG_RESURRECTION_HEAL,false);
                    }
                    Resurect_Timer = 3000;
                    Resurect_Phase = 2;
                }else if (Resurect_Phase == 2)
                {
                    Unit* ingvar = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_INGVAR));
                    if(ingvar)
                    {
                        ingvar->RemoveAurasDueToSpell(SPELL_SCOURG_RESURRECTION_DUMMY);
                        //ingvar->CastSpell(ingvar,SPELL_INGVAR_TRANSFORM,false);
                        ingvar->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_INGVAR_UNDEAD); // Visual Hack - when he dies he becomes human model -> wrong
                        Creature* c_ingvar = (Creature*)ingvar;

                        ((boss_ingvar_the_plundererAI*)(c_ingvar->AI()))->StartZombiePhase();

                        m_creature->GetMotionMaster()->MovePoint(2,x+1,y,z+30);
                        Resurect_Phase++;
                    }
                }
                
            }else Resurect_Timer -= diff;
    }
};

CreatureAI* GetAI_mob_annhylde_the_caller(Creature *_Creature)
{
    return new mob_annhylde_the_callerAI (_Creature);
}

#define SPELL_SHADOW_AXE_DAMAGE                     42750
#define H_SPELL_SHADOW_AXE_DAMAGE                   59719

struct MANGOS_DLL_DECL mob_ingvar_throw_dummyAI : public ScriptedAI
{
    mob_ingvar_throw_dummyAI(Creature *c) : ScriptedAI(c) 
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        HeroicMode = c->GetMap()->IsHeroic();
        Reset();
    }

    bool HeroicMode;
    uint32 Despawn_Timer;

    ScriptedInstance* pInstance;

    void Reset()
    {
//        Unit* target = FindCreature(ENTRY_THROW_TARGET,50,m_creature);
//        if(target)
//        {
//            DoCast(m_creature, HeroicMode ? H_SPELL_SHADOW_AXE_DAMAGE : SPELL_SHADOW_AXE_DAMAGE);
//            float x,y,z;
//            target->GetPosition(x,y,z);
//            m_creature->GetMotionMaster()->MovePoint(0,x,y,z);
//        }
        Despawn_Timer = 7000;
    }
    void AttackStart(Unit* who) {}
    void MoveInLineOfSight(Unit* who) {}
    void Aggro(Unit *who) {}
    void UpdateAI(const uint32 diff)
    {
        if(Despawn_Timer < diff)
        {
            m_creature->DealDamage(m_creature,m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->RemoveCorpse();
            Despawn_Timer = 0;
        }else Despawn_Timer -= diff;
    }
};

CreatureAI* GetAI_mob_ingvar_throw_dummy(Creature *_Creature)
{
    return new mob_ingvar_throw_dummyAI (_Creature);
}

void AddSC_boss_ingvar_the_plunderer()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_ingvar_the_plunderer";
    newscript->GetAI = GetAI_boss_ingvar_the_plunderer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_annhylde_the_caller";
    newscript->GetAI = GetAI_mob_annhylde_the_caller;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_ingvar_throw_dummy";
    newscript->GetAI = GetAI_mob_ingvar_throw_dummy;
    newscript->RegisterSelf();
}
