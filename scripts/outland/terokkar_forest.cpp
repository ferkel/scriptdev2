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
SDName: Terokkar_Forest
SD%Complete: 80
SDComment: Quest support: 9889(test script only, sql inside script), 10009, 10873, 10896, 10446/10447, 10887, 11096. Skettis->Ogri'la Flight
SDCategory: Terokkar Forest
EndScriptData */

/* ContentData
mob_unkor_the_ruthless
mob_infested_root_walker
mob_rotting_forest_rager
mob_netherweb_victim
npc_akuno
npc_floon
npc_mana_bomb_exp_trigger
go_mana_bomb
npc_skyguard_handler_deesak
npc_slim
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## mob_unkor_the_ruthless
######*/

#define SAY_SUBMIT                      -1000194

#define FACTION_HOSTILE                 45
#define FACTION_FRIENDLY                35
#define QUEST_DONTKILLTHEFATONE         9889

#define SPELL_PULVERIZE                 2676
//#define SPELL_QUID9889                32174

struct MANGOS_DLL_DECL mob_unkor_the_ruthlessAI : public ScriptedAI
{
    mob_unkor_the_ruthlessAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool CanDoQuest;
    uint32 UnkorUnfriendly_Timer;
    uint32 Pulverize_Timer;

    void Reset()
    {
        CanDoQuest = false;
        UnkorUnfriendly_Timer = 0;
        Pulverize_Timer = 3000;
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        m_creature->setFaction(FACTION_HOSTILE);
    }

    void DoNice()
    {
        DoScriptText(SAY_SUBMIT, m_creature);
        m_creature->setFaction(FACTION_FRIENDLY);
        m_creature->SetStandState(UNIT_STAND_STATE_SIT);
        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop(true);
        UnkorUnfriendly_Timer = 60000;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by->GetTypeId() == TYPEID_PLAYER)
            if ((m_creature->GetHealth()-damage)*100 / m_creature->GetMaxHealth() < 30)
        {
            if (Group* pGroup = ((Player*)done_by)->GetGroup())
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player *pGroupie = itr->getSource();
                    if (pGroupie &&
                        pGroupie->GetQuestStatus(QUEST_DONTKILLTHEFATONE) == QUEST_STATUS_INCOMPLETE &&
                        pGroupie->GetReqKillOrCastCurrentCount(QUEST_DONTKILLTHEFATONE, 18260) == 10)
                    {
                        pGroupie->AreaExploredOrEventHappens(QUEST_DONTKILLTHEFATONE);
                        if (!CanDoQuest)
                            CanDoQuest = true;
                    }
                }
            } else
            if (((Player*)done_by)->GetQuestStatus(QUEST_DONTKILLTHEFATONE) == QUEST_STATUS_INCOMPLETE &&
                ((Player*)done_by)->GetReqKillOrCastCurrentCount(QUEST_DONTKILLTHEFATONE, 18260) == 10)
            {
                ((Player*)done_by)->AreaExploredOrEventHappens(QUEST_DONTKILLTHEFATONE);
                CanDoQuest = true;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (CanDoQuest)
        {
            if (!UnkorUnfriendly_Timer)
            {
                //DoCast(m_creature,SPELL_QUID9889);        //not using spell for now
                DoNice();
            }
            else
            {
                if (UnkorUnfriendly_Timer < diff)
                {
                    EnterEvadeMode();
                }else UnkorUnfriendly_Timer -= diff;
            }
        }

        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

        if (Pulverize_Timer < diff)
        {
            DoCast(m_creature,SPELL_PULVERIZE);
            Pulverize_Timer = 9000;
        }else Pulverize_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_unkor_the_ruthless(Creature* pCreature)
{
    return new mob_unkor_the_ruthlessAI(pCreature);
}

/*######
## mob_infested_root_walker
######*/

struct MANGOS_DLL_DECL mob_infested_root_walkerAI : public ScriptedAI
{
    mob_infested_root_walkerAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset() { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by && done_by->GetTypeId() == TYPEID_PLAYER)
            if (m_creature->GetHealth() <= damage)
                if (rand()%100 < 75)
                    //Summon Wood Mites
                    m_creature->CastSpell(m_creature,39130,true);
    }
};
CreatureAI* GetAI_mob_infested_root_walker(Creature* pCreature)
{
    return new mob_infested_root_walkerAI(pCreature);
}

/*######
## mob_rotting_forest_rager
######*/

struct MANGOS_DLL_DECL mob_rotting_forest_ragerAI : public ScriptedAI
{
    mob_rotting_forest_ragerAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset() { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by->GetTypeId() == TYPEID_PLAYER)
            if (m_creature->GetHealth() <= damage)
                if (rand()%100 < 75)
                    //Summon Lots of Wood Mights
                    m_creature->CastSpell(m_creature,39134,true);
    }
};
CreatureAI* GetAI_mob_rotting_forest_rager(Creature* pCreature)
{
    return new mob_rotting_forest_ragerAI(pCreature);
}

/*######
## mob_netherweb_victim
######*/

enum
{
    NPC_FREED_WARRIOR       = 22459,
    QUEST_TAKEN_IN_NIGHT    = 10873
    //SPELL_FREE_WEBBED       = 38950
};

const uint32 netherwebVictims[6] =
{
    18470, 16805, 21242, 18452, 22482, 21285
};
struct MANGOS_DLL_DECL mob_netherweb_victimAI : public ScriptedAI
{
    mob_netherweb_victimAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset() { }
    void MoveInLineOfSight(Unit* pWho) { }

    void JustDied(Unit* pKiller)
    {
        if (pKiller->GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)pKiller)->GetQuestStatus(QUEST_TAKEN_IN_NIGHT) == QUEST_STATUS_INCOMPLETE)
            {
                if (rand()%100 < 25)
                {
                    m_creature->SummonCreature(NPC_FREED_WARRIOR, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                    ((Player*)pKiller)->KilledMonsterCredit(NPC_FREED_WARRIOR, m_creature->GetGUID());
                }
                else
                    m_creature->SummonCreature(netherwebVictims[rand()%6], 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);

                if (rand()%100 < 75)
                    m_creature->SummonCreature(netherwebVictims[rand()%6], 0.0f, 0.0f, 0.0f,0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);

                m_creature->SummonCreature(netherwebVictims[rand()%6], 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
            }
        }
    }
};

CreatureAI* GetAI_mob_netherweb_victim(Creature* pCreature)
{
    return new mob_netherweb_victimAI(pCreature);
}

/*#####
## npc_akuno
#####*/

enum
{
    SAY_AKU_START           = -1000477,
    SAY_AKU_AMBUSH_A        = -1000478,
    SAY_AKU_AMBUSH_B        = -1000479,
    SAY_AKU_AMBUSH_B_REPLY  = -1000480,
    SAY_AKU_COMPLETE        = -1000481,

    SPELL_CHAIN_LIGHTNING   = 39945,

    QUEST_ESCAPING_TOMB     = 10887,
    NPC_CABAL_SKIRMISHER    = 21661
};

static float m_afAmbushB1[]= {-2895.525879, 5336.431641, -11.800};
static float m_afAmbushB2[]= {-2890.604980, 5331.938965, -11.282};

struct MANGOS_DLL_DECL npc_akunoAI : public npc_escortAI
{
    npc_akunoAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    uint32 m_uiChainLightningTimer;

    void Reset()
    {
        m_uiChainLightningTimer = 1000;
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 5:
                DoScriptText(SAY_AKU_AMBUSH_A, m_creature);
                m_creature->SummonCreature(NPC_CABAL_SKIRMISHER, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                break;
            case 14:
                DoScriptText(SAY_AKU_AMBUSH_B, m_creature);

                if (Creature* pTemp = m_creature->SummonCreature(NPC_CABAL_SKIRMISHER, m_afAmbushB1[0], m_afAmbushB1[1], m_afAmbushB1[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000))
                    DoScriptText(SAY_AKU_AMBUSH_B_REPLY, pTemp);

                m_creature->SummonCreature(NPC_CABAL_SKIRMISHER, m_afAmbushB2[0], m_afAmbushB2[1], m_afAmbushB2[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                break;
            case 15:
                SetRun();
                break;
            case 18:
                DoScriptText(SAY_AKU_COMPLETE, m_creature);

                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_ESCAPING_TOMB, m_creature);

                break;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

        if (m_uiChainLightningTimer < uiDiff)
        {
            DoCast(m_creature->getVictim(), SPELL_CHAIN_LIGHTNING);
            m_uiChainLightningTimer = urand(7000, 14000);
        }
        else
            m_uiChainLightningTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_akuno(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ESCAPING_TOMB)
    {
        if (npc_akunoAI* pEscortAI = dynamic_cast<npc_akunoAI*>(pCreature->AI()))
        {
            pCreature->SetStandState(UNIT_STAND_STATE_STAND);
            pCreature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);

            DoScriptText(SAY_AKU_START, pCreature);
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest);
        }
    }
    return true;
}

CreatureAI* GetAI_npc_akuno(Creature* pCreature)
{
    return new npc_akunoAI(pCreature);
}

/*######
## npc_floon
######*/

enum
{
    SAY_FLOON_ATTACK        = -1000195,

    SPELL_SILENCE           = 6726,
    SPELL_FROSTBOLT         = 9672,
    SPELL_FROST_NOVA        = 11831,

    FACTION_HOSTILE_FL      = 1738,
    QUEST_CRACK_SKULLS      = 10009
};

#define GOSSIP_FLOON1       "You owe Sim'salabim money. Hand them over or die!"
#define GOSSIP_FLOON2       "Hand over the money or die...again!"

struct MANGOS_DLL_DECL npc_floonAI : public ScriptedAI
{
    npc_floonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_uiNormFaction = pCreature->getFaction();
        Reset();
    }

    uint32 m_uiNormFaction;
    uint32 m_uiSilence_Timer;
    uint32 m_uiFrostbolt_Timer;
    uint32 m_uiFrostNova_Timer;

    void Reset()
    {
        m_uiSilence_Timer = 2000;
        m_uiFrostbolt_Timer = 4000;
        m_uiFrostNova_Timer = 9000;

        if (m_creature->getFaction() != m_uiNormFaction)
            m_creature->setFaction(m_uiNormFaction);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

        if (m_uiSilence_Timer < uiDiff)
        {
            DoCast(m_creature->getVictim(),SPELL_SILENCE);
            m_uiSilence_Timer = 30000;
        }else m_uiSilence_Timer -= uiDiff;

        if (m_uiFrostNova_Timer < uiDiff)
        {
            DoCast(m_creature,SPELL_FROST_NOVA);
            m_uiFrostNova_Timer = 20000;
        }else m_uiFrostNova_Timer -= uiDiff;

        if (m_uiFrostbolt_Timer < uiDiff)
        {
            DoCast(m_creature->getVictim(),SPELL_FROSTBOLT);
            m_uiFrostbolt_Timer = 5000;
        }else m_uiFrostbolt_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_floon(Creature* pCreature)
{
    return new npc_floonAI(pCreature);
}

bool GossipHello_npc_floon(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_CRACK_SKULLS) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_FLOON1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(9442, pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_floon(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_FLOON2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(9443, pCreature->GetGUID());
    }
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->setFaction(FACTION_HOSTILE_FL);
        DoScriptText(SAY_FLOON_ATTACK, pCreature, pPlayer);
        ((npc_floonAI*)pCreature->AI())->AttackStart(pPlayer);
    }
    return true;
}

/*######
## npc_skyguard_handler_deesak
######*/

#define GOSSIP_SKYGUARD "Fly me to Ogri'la please"

bool GossipHello_npc_skyguard_handler_deesak(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetReputationRank(1031) >= REP_HONORED)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SKYGUARD, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_skyguard_handler_deesak(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pPlayer->CastSpell(pPlayer,41279,true);               //TaxiPath 705 (Taxi - Skettis to Skyguard Outpost)
    }
    return true;
}

/*######
## npc_mana_bomb_exp_trigger
######*/

enum
{
    SAY_COUNT_1                 = -1000472,
    SAY_COUNT_2                 = -1000473,
    SAY_COUNT_3                 = -1000474,
    SAY_COUNT_4                 = -1000475,
    SAY_COUNT_5                 = -1000476,

    SPELL_MANA_BOMB_LIGHTNING   = 37843,
    SPELL_MANA_BOMB_EXPL        = 35513,

    NPC_MANA_BOMB_EXPL_TRIGGER  = 20767,
    NPC_MANA_BOMB_KILL_TRIGGER  = 21039
};

struct MANGOS_DLL_DECL npc_mana_bomb_exp_triggerAI : public ScriptedAI
{
    npc_mana_bomb_exp_triggerAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    GameObject* pManaBomb;

    bool m_bIsActivated;
    uint32 m_uiEventTimer;
    uint32 m_uiEventCounter;

    void Reset()
    {
        pManaBomb = NULL;
        m_bIsActivated = false;
        m_uiEventTimer = 1000;
        m_uiEventCounter = 0;
    }

    void DoTrigger(Player* pPlayer, GameObject* pGo)
    {
        if (m_bIsActivated)
            return;

        m_bIsActivated = true;

        pPlayer->KilledMonsterCredit(NPC_MANA_BOMB_KILL_TRIGGER, 0);

        pManaBomb = pGo;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_bIsActivated)
            return;

        if (m_uiEventTimer < uiDiff)
        {
            m_uiEventTimer = 1000;

            if (m_uiEventCounter < 10)
                m_creature->CastSpell(m_creature, SPELL_MANA_BOMB_LIGHTNING, false);

            switch(m_uiEventCounter)
            {
                case 5:
                    if (pManaBomb)
                        pManaBomb->SetGoState(GO_STATE_ACTIVE);

                    DoScriptText(SAY_COUNT_1, m_creature);
                    break;
                case 6:
                    DoScriptText(SAY_COUNT_2, m_creature);
                    break;
                case 7:
                    DoScriptText(SAY_COUNT_3, m_creature);
                    break;
                case 8:
                    DoScriptText(SAY_COUNT_4, m_creature);
                    break;
                case 9:
                    DoScriptText(SAY_COUNT_5, m_creature);
                    break;
                case 10:
                    m_creature->CastSpell(m_creature, SPELL_MANA_BOMB_EXPL, false);
                    break;
                case 30:
                    if (pManaBomb)
                        pManaBomb->SetGoState(GO_STATE_READY);

                    Reset();
                    break;
            }

            ++m_uiEventCounter;
        }
        else
            m_uiEventTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_mana_bomb_exp_trigger(Creature* pCreature)
{
    return new npc_mana_bomb_exp_triggerAI(pCreature);
}

/*######
## go_mana_bomb
######*/

bool GOHello_go_mana_bomb(Player* pPlayer, GameObject* pGo)
{
    if (Creature* pCreature = GetClosestCreatureWithEntry(pGo, NPC_MANA_BOMB_EXPL_TRIGGER, INTERACTION_DISTANCE))
    {
        if (npc_mana_bomb_exp_triggerAI* pBombAI = dynamic_cast<npc_mana_bomb_exp_triggerAI*>(pCreature->AI()))
            pBombAI->DoTrigger(pPlayer, pGo);
    }

    return true;
}

/*######
## npc_slim
######*/

enum
{
    FACTION_CONSORTIUM  = 933
};

bool GossipHello_npc_slim(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isVendor() && pPlayer->GetReputationRank(FACTION_CONSORTIUM) >= REP_FRIENDLY)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        pPlayer->SEND_GOSSIP_MENU(9896, pCreature->GetGUID());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(9895, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_slim(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetGUID());

    return true;
}

void AddSC_terokkar_forest()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mob_unkor_the_ruthless";
    newscript->GetAI = &GetAI_mob_unkor_the_ruthless;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_infested_root_walker";
    newscript->GetAI = &GetAI_mob_infested_root_walker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_rotting_forest_rager";
    newscript->GetAI = &GetAI_mob_rotting_forest_rager;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_netherweb_victim";
    newscript->GetAI = &GetAI_mob_netherweb_victim;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_akuno";
    newscript->GetAI = &GetAI_npc_akuno;
    newscript->pQuestAccept = &QuestAccept_npc_akuno;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_floon";
    newscript->GetAI = &GetAI_npc_floon;
    newscript->pGossipHello =  &GossipHello_npc_floon;
    newscript->pGossipSelect = &GossipSelect_npc_floon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mana_bomb_exp_trigger";
    newscript->GetAI = &GetAI_npc_mana_bomb_exp_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_mana_bomb";
    newscript->pGOHello = &GOHello_go_mana_bomb;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_skyguard_handler_deesak";
    newscript->pGossipHello =  &GossipHello_npc_skyguard_handler_deesak;
    newscript->pGossipSelect = &GossipSelect_npc_skyguard_handler_deesak;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_slim";
    newscript->pGossipHello =  &GossipHello_npc_slim;
    newscript->pGossipSelect = &GossipSelect_npc_slim;
    newscript->RegisterSelf();
}
