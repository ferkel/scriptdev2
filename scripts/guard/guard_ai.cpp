/* Copyright (C) 2006,2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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


#include "guard_ai.h"

// **** This script is for use within every single guard to save coding time ****

#define GENERIC_CREATURE_COOLDOWN 5000

/*guardAI::guardAI(Creature *c) : ScriptedAI(c)
{
    EnterEvadeMode();
}*/

void guardAI::EnterEvadeMode()
{
    GlobalCooldown = 0;
    BuffTimer = 0;          //Rebuff as soon as we can
    InCombat = false;

    m_creature->RemoveAllAuras();
    m_creature->DeleteThreatList();
    m_creature->CombatStop();
    DoGoHome();
}

void guardAI::AttackStart(Unit *who)
{
    if (!who)
        return;

    //Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
    if (who->GetTypeId() == TYPEID_PLAYER)
        m_creature->SendZoneUnderAttackMessage((Player*)who);

    if (who->isTargetableForAttack() && who!= m_creature)
    {
        //Begin melee attack if we are within range
        if (m_creature->IsWithinDistInMap(who, ATTACK_DISTANCE))
            DoStartMeleeAttack(who);
        else DoStartRangedAttack(who);

        InCombat = true;
    }
}

void guardAI::MoveInLineOfSight(Unit *who)
{
    if (!who || m_creature->getVictim())
        return;

    if (who->isTargetableForAttack() && who->isInAccessablePlaceFor(m_creature) && (m_creature->IsHostileTo(who) || who->IsHostileToPlayer()))
    {
        float attackRadius = m_creature->GetAttackDistance(who);
        if (m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE && m_creature->IsWithinLOSInMap(who))
        {
            if(who->HasStealthAura())
                who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

            //Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
            if (who->GetTypeId() == TYPEID_PLAYER)
                m_creature->SendZoneUnderAttackMessage((Player*)who);

            //Begin melee attack if we are within range
            if (m_creature->IsWithinDistInMap(who, ATTACK_DISTANCE))
                DoStartMeleeAttack(who);
            else DoStartRangedAttack(who);

            InCombat = true;
        }
    }
}

void guardAI::UpdateAI(const uint32 diff)
{
    //Always decrease our global cooldown first
    if (GlobalCooldown > diff)
        GlobalCooldown -= diff;
    else GlobalCooldown = 0;

    //Buff timer (only buff when we are alive and not in combat
    if (m_creature->isAlive() && !InCombat)
        if (BuffTimer < diff )
        {
            //Find a spell that targets friendly and applies an aura (these are generally buffs)
            SpellEntry const *info = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_AURA);

            if (info && !GlobalCooldown)
            {
                //Cast the buff spell
                DoCastSpell(m_creature, info);

                //Set our global cooldown
                GlobalCooldown = GENERIC_CREATURE_COOLDOWN;

                //Set our timer to 10 minutes before rebuff
                BuffTimer = 600000;
            }//Try agian in 30 seconds
            else BuffTimer = 30000;
        }else BuffTimer -= diff;

    //Return since we have no target
    if (!m_creature->SelectHostilTarget())
        return;

    //Check if we have a current target
    if( m_creature->getVictim() && m_creature->isAlive())
    {
        //Check if we should stop attacking because our victim is no longer in range or we are to far from spawn point
        if (CheckTether())
        {
            EnterEvadeMode();
            return;
        }
        
        //If we are within range melee the target
        if( m_creature->IsWithinDistInMap(m_creature->getVictim(), ATTACK_DISTANCE))
        {
            //Make sure our attack is ready and we arn't currently casting
            if( m_creature->isAttackReady() && !m_creature->m_currentSpell)
            {
                bool Healing = false;
                SpellEntry const *info = NULL;

                //Select a healing spell if less than 30% hp
                if (m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 30)
                    info = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

                //No healing spell available, select a hostile spell
                if (info) Healing = true;
                    else info = SelectSpell(m_creature->getVictim(), -1, -1, SELECT_TARGET_ANY_ENEMY, 0, 0, 0, 0, SELECT_EFFECT_DONTCARE);

                //20% chance to replace our white hit with a spell
                if (info && rand() % 5 == 0 && !GlobalCooldown)
                {
                    //Cast the spell
                    if (Healing)DoCastSpell(m_creature, info);
                        else DoCastSpell(m_creature->getVictim(), info);

                    //Set our global cooldown
                    GlobalCooldown = GENERIC_CREATURE_COOLDOWN;
                }
                else m_creature->AttackerStateUpdate(m_creature->getVictim());

                m_creature->resetAttackTimer();
            }
        }
        else 
        {
            //Only run this code if we arn't already casting
            if (!m_creature->m_currentSpell)
            {
                bool Healing = false;
                SpellEntry const *info = NULL;

                //Select a healing spell if less than 30% hp ONLY 33% of the time
                if (m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 30 && rand() % 3 == 0)
                    info = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

                //No healing spell available, See if we can cast a ranged spell (Range must be greater than ATTACK_DISTANCE)
                if (info) Healing = true;
                    else info = SelectSpell(m_creature->getVictim(), -1, -1, SELECT_TARGET_ANY_ENEMY, 0, 0, ATTACK_DISTANCE, 0, SELECT_EFFECT_DONTCARE);
           
                //Found a spell, check if we arn't on cooldown
                if (info && !GlobalCooldown)
                {
                    //If we are currently moving stop us and set the movement generator
                    if ((*m_creature)->top()->GetMovementGeneratorType()!=IDLE_MOTION_TYPE)
                    {
                        (*m_creature)->Clear(false);
                        (*m_creature)->Idle();
                    }

                    //Face target
                    DoFaceTarget(m_creature->getVictim());

                    //Cast spell
                    if (Healing) DoCastSpell(m_creature,info);
                        else DoCastSpell(m_creature->getVictim(),info);

                    //Set our global cooldown
                    GlobalCooldown = GENERIC_CREATURE_COOLDOWN;
                    

                }//If no spells available and we arn't moving run to target
                else if ((*m_creature)->top()->GetMovementGeneratorType()!=TARGETED_MOTION_TYPE)
                {
                    //Cancel our current spell and then mutate new movement generator
                    m_creature->InterruptSpell();
                    (*m_creature)->Clear(false);
                    (*m_creature)->Mutate(new TargetedMovementGenerator(*m_creature->getVictim()));
                }
            }
        }
    }
}