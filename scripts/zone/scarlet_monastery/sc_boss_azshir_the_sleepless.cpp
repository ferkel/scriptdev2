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

#include "../../sc_defines.h"

// **** This script is still under Developement ****

#define SPELL_CALLOFTHEGRAVE        17831
#define SPELL_TERRIFY			    7399
#define SPELL_DAZED					1604
#define SPELL_SOULSIPHON		    7290

struct MANGOS_DLL_DECL boss_azshir_the_sleeplessAI : public ScriptedAI
{
    boss_azshir_the_sleeplessAI(Creature *c) : ScriptedAI(c) {Reset();}

    Unit *pTarget;
	uint32 SoulSiphon_Timer;
    uint32 CallOftheGrave_Timer;
    uint32 Terrify_Timer;

    void Reset()
    {
        pTarget = NULL;
		SoulSiphon_Timer = 80000;
        CallOftheGrave_Timer = 70000;
        Terrify_Timer = 45000;

        if (m_creature)
        {
            m_creature->RemoveAllAuras();
            DoStopAttack();
            DoGoHome();
        }
    }

    void AttackStart(Unit *who)
    {
        if (!who)
            return;

        if (m_creature->getVictim() == NULL && who->isTargetableForAttack() && who!= m_creature)
        {
            //Begin melee attack if we are within range
            if (m_creature->IsWithinDist(who, ATTACK_DIST))
				DoStartMeleeAttack(who);
            else DoCast(m_creature->getVictim(),SPELL_DAZED);

			pTarget = who;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who)
            return;

        if (who->isTargetableForAttack() && IsVisible(who) && who->isInAccessablePlaceFor(m_creature) && m_creature->IsHostileTo(who))
        {
            if ( m_creature->getVictim() == NULL)
            {
                //Begin melee attack if we are within range
                DoStartMeleeAttack(who);

                pTarget = who;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //If we had a target and it wasn't cleared then it means the player died from some unknown source
        //But we still need to reset
        if (m_creature->isAlive() && pTarget && !m_creature->getVictim())
        {
            Reset();
            return;
        }

        //Check if we have a current target
        if( m_creature->getVictim() && m_creature->isAlive())
        {
            //Check if we should stop attacking because our victim is no longer attackable
            if (needToStop())
            {
                Reset();
                return;
            }

            //If we are <50% hp cast Soul Siphon rank 1
            if ( m_creature->GetHealth()*100 / m_creature->GetMaxHealth() <= 50 && !m_creature->m_currentSpell)
            {
				//SoulSiphon_Timer
				if (SoulSiphon_Timer < diff)
				{

					DoCast(m_creature->getVictim(),SPELL_SOULSIPHON);
					return;
	
	                //180 seconds until we should cast this agian
	                SoulSiphon_Timer = 80000;
	            }else SoulSiphon_Timer -= diff;
            }

            //CallOfTheGrave_Timer
            if (CallOftheGrave_Timer < diff)
            {
                //Cast
                DoCast(m_creature->getVictim(),SPELL_CALLOFTHEGRAVE);

                //70 seconds until we should cast this agian
                CallOftheGrave_Timer = 70000;
            }else CallOftheGrave_Timer -= diff;

            //Terrify_Timer
            if (Terrify_Timer < diff)
            {
                //Cast
                DoCast(m_creature->getVictim(),SPELL_TERRIFY);

                //45 seconds until we should cast this agian
                Terrify_Timer = 45000;
            }else Terrify_Timer -= diff;

            //If we are within range melee the target
            if( m_creature->IsWithinDist(m_creature->getVictim(), ATTACK_DIST))
            {
                //Make sure our attack is ready and we arn't currently casting
                if( m_creature->isAttackReady() && !m_creature->m_currentSpell)
                {
                    Unit* newtarget = m_creature->SelectHostilTarget();
                    if(newtarget)
                        AttackStart(newtarget);

                    m_creature->AttackerStateUpdate(m_creature->getVictim());
                    m_creature->resetAttackTimer();
                }
            }
            
            //If we are still alive and we lose our victim it means we killed them
            if(m_creature->isAlive() && !m_creature->getVictim())
            {
                Reset();
            }
        }
    }
};

CreatureAI* GetAI_boss_azshir_the_sleepless(Creature *_Creature)
{
    return new boss_azshir_the_sleeplessAI (_Creature);
}

void AddSC_boss_azshir_the_sleepless()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_azshir_the_sleepless";
    newscript->GetAI = GetAI_boss_azshir_the_sleepless;
    m_scripts[nrscripts++] = newscript;
}