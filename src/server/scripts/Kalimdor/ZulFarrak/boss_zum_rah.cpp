/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
Name: Boss_Zum_Rah
Category: Tanaris, ZulFarrak
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "zulfarrak.h"

enum Says
{
    SAY_SANCT_INVADE    = 0,
    SAY_WARD            = 1,
    SAY_KILL            = 2
};

enum Spells
{
    SPELL_SHADOW_BOLT               = 12739,
    SPELL_SHADOWBOLT_VOLLEY         = 15245,
    SPELL_WARD_OF_ZUM_RAH           = 11086,
    SPELL_HEALING_WAVE              = 12491
};

enum Events
{
    EVENT_SHADOW_BOLT           = 1,
    EVENT_SHADOWBOLT_VOLLEY     = 2,
    EVENT_WARD_OF_ZUM_RAH       = 3,
    EVENT_HEALING_WAVE          = 4
};

class boss_zum_rah : public CreatureScript
{
public:
    boss_zum_rah() : CreatureScript("boss_zum_rah") { }

    struct boss_zum_rahAI : public BossAI
    {
        boss_zum_rahAI(Creature* creature) : BossAI(creature, BOSS_WITCH_DOCTOR_ZUM_RAH)
        {
            Initialize();
        }

        void Initialize()
        {
            _ward80 = false;
            _ward40 = false;
            _heal30 = false;
        }

        void Reset() override
        {
            _Reset();
            me->SetFaction(FACTION_FRIENDLY); // areatrigger sets faction to enemy
            Initialize();
        }

        void JustEngagedWith(Unit* who) override
        {
            _JustEngagedWith(who);
            Talk(SAY_SANCT_INVADE);
            events.ScheduleEvent(EVENT_SHADOW_BOLT, 1s);
            events.ScheduleEvent(EVENT_SHADOWBOLT_VOLLEY, 10s);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_KILL);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHADOW_BOLT:
                        DoCastVictim(SPELL_SHADOW_BOLT);
                        events.ScheduleEvent(EVENT_SHADOW_BOLT, 4s);
                        break;
                    case EVENT_WARD_OF_ZUM_RAH:
                        DoCast(me,SPELL_WARD_OF_ZUM_RAH);
                        break;
                    case EVENT_HEALING_WAVE:
                        DoCast(me,SPELL_HEALING_WAVE);
                        break;
                    case EVENT_SHADOWBOLT_VOLLEY:
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                            DoCast(target, SPELL_SHADOWBOLT_VOLLEY);
                        events.ScheduleEvent(EVENT_SHADOWBOLT_VOLLEY, 9s);
                        break;
                    default:
                        break;
                }
            }

            if (!_ward80 && HealthBelowPct(80))
            {
                _ward80 = true;
                Talk(SAY_WARD);
                events.ScheduleEvent(EVENT_WARD_OF_ZUM_RAH, 1s);
            }

            if (!_ward40 && HealthBelowPct(40))
            {
                _ward40 = true;
                Talk(SAY_WARD);
                events.ScheduleEvent(EVENT_WARD_OF_ZUM_RAH, 1s);
            }

            if (!_heal30 && HealthBelowPct(30))
            {
                _heal30 = true;
                events.ScheduleEvent(EVENT_HEALING_WAVE, 3s);
            }

            DoMeleeAttackIfReady();
        }

        private:
            bool _ward80;
            bool _ward40;
            bool _heal30;

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetZulFarrakAI<boss_zum_rahAI>(creature);
    }
};

void AddSC_boss_zum_rah()
{
    new boss_zum_rah();
}
