#include "angband.h"

#define lawyer_aptitude ((p_ptr->pclass == CLASS_LAWYER) || (prace_is_(RACE_VAMPIRE)) || (prace_is_(RACE_ENT)))

/* Epic mother of all hacks
 * Returns the slevel, smana or sfail of a spell after certain adjustments
 * Although this is named lawyer_hack() and mostly only changes anything
 * in the law realm, it can easily support similar adjustments for other
 * realms (or classes or races) as well */
byte lawyer_hack(magic_type *s_ptr, int tyyppi)
{
    int kerroin = 100, sopivuus = 3;
    if ((tyyppi == LAWYER_HACK_MANA) && (s_ptr->realm == REALM_DEATH) && (s_ptr->idx == 21)) /* vampirism true */
    {
        int tulos = (int)s_ptr->smana;
        int lisays = tulos;
        if (lisays < 50) lisays = 50;
        if (lisays > 100) lisays = 100;
        tulos += lisays;
        if (tulos > 250) tulos = 250;
        return tulos;
    }
    if ((s_ptr->realm == REALM_LIFE) && (tyyppi == LAWYER_HACK_LEVEL)
        && (s_ptr->idx == 23) && (p_ptr->realm1 != REALM_LIFE))
    {
        /* Warding True - only available to primary-realm Life casters
         * Warding True very easily runs into conflicts with the 11-glyph
         * limit (which primary-realm Life casters are exempt from) */
        return 99;
    }
    if (s_ptr->realm == REALM_LAW)
    {
        if (prace_is_(RACE_VAMPIRE) || prace_is_(RACE_ENT)) sopivuus++;
        if (p_ptr->pclass != CLASS_LAWYER) sopivuus -= 2;
        //if (p_ptr->realm2 == REALM_LAW) sopivuus -= 1;

        /* No Advanced Bloodsucking for blood mages */
        if ((tyyppi == LAWYER_HACK_LEVEL) && (p_ptr->pclass == CLASS_BLOOD_MAGE) &&
            (s_ptr->idx == 22)) return 99;

        if (sopivuus != 3)
        {
            switch (tyyppi)
            {
                case LAWYER_HACK_LEVEL:
                {
                     kerroin = 130 - (sopivuus * 10);
                     break;
                }
                default:
                {
                     kerroin = 160 - (sopivuus * 20);
                     break;
                }
            }
        }
    }
    switch (tyyppi)
    {
        case LAWYER_HACK_LEVEL:
        {
            int taso;
            if (kerroin == 100) return s_ptr->slevel;
            taso = (int)s_ptr->slevel * kerroin / 100;
            if (taso >= 58) taso = 99;
            else if (taso > 50) taso = 50;
            return MAX(1, taso);
        }
        case LAWYER_HACK_MANA:
        {
            int tulos;
            if (kerroin == 100) return s_ptr->smana;
            tulos = (int)s_ptr->smana * kerroin / 100;
            if (tulos > 255) tulos = 255;
            if ((tulos < 1) && (s_ptr->smana)) tulos = 1;
            return tulos;
        }
        default:
        {
            if ((p_ptr->prace == RACE_WEREWOLF) || (p_ptr->prace == RACE_BEORNING)) /* increased fail rates */
            {
                int tulos;
                if (werewolf_in_human_form() || beorning_is_(BEORNING_FORM_HUMAN)) tulos = s_ptr->sfail + ((kerroin < 101) ? 15 : 20);
                else tulos = MAX(s_ptr->sfail + ((kerroin < 101) ? 25 : 30), (int)s_ptr->sfail * 2);
                if (tulos > 255) tulos = 255;
                return tulos;
            }
            return ((kerroin < 101) ? s_ptr->sfail : s_ptr->sfail + 10);
        }
    }
}

/* Handle law spells */
cptr do_law_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool lisa;

    int plev = p_ptr->lev;
    int rad = DETECT_RAD_DEFAULT;
    int dir;

    if (plev >= 50)
        rad = DETECT_RAD_ALL;
    else
        rad += plev;

    switch (spell)
    {
    /* Attractions of Law */
    case 0:
        if (name) return "Detect Money";
        if (desc) return "Detects all treasures in your vicinity.";
        if (info) return info_radius(rad);

        if (cast)
        {
            detect_treasure(rad);
            detect_objects_gold(rad);
        }
        break;


    case 1:
        if (name) return "Detect Traps";
        if (desc) return "Detects nearby traps.";
        if (info) return info_radius(rad);
        if (cast)
            detect_traps(rad, TRUE);
        break;

    case 2:
        if (name) return "Satisfy Hunger";
        if (desc) return "Satisfies hunger.";
        if (cast)
            set_food(PY_FOOD_MAX - 1);
        break;

    case 3:
        if (name) return "Detect Objects";
        if (desc) return "Detects all items in your vicinity.";
        if (info) return info_radius(rad);

        if (cast)
            detect_objects_normal(rad);
        break;

    case 4:
        if (name) return "Basic Trap";
        if (desc) return "Sets a trap under you. The trap will have a random weak effect on the monster that triggers it.";

        if (cast)
            set_trap(py, px, feat_rogue_trap1);
        break;

    case 5:
        if (name) return "Disarm Traps";
        if (desc) return "Fires a beam which disarms traps.";

        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            disarm_trap(dir);
        }
        break;

    case 6:
        lisa = ((plev >= 45) && (lawyer_aptitude));
        if (name) return "Identify";
        if (desc) return (lisa ? "Fully identifies an item." : "Identifies an item.");

        {
            if (cast)
            {
                if ((lisa) && (!identify_fully(NULL))) return NULL;
                else if (lisa) break;
                else if (!ident_spell(NULL)) return NULL;
            }
        }
        break;

    case 7:
        if (name) return "Dig";
        if (desc) return "Turns one rock square to mud.";

        {
            int dice = 1;
            int sides = 30;
            int base = 20;

            if (info) return info_damage(dice, sides, base);

            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                wall_to_mud(dir);
            }
        }
        break;

    /* Obstacle Coursebook */
    case 8:
        lisa = (plev >= 25);
        if (name) return "Detect Monsters";
        if (desc) return (lisa ? "Detects all monsters in your vicinity." : "Detects all monsters in your vicinity unless invisible.");

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_normal(rad);
                if (lisa) detect_monsters_invis(rad);
            }
        }
        break;

    case 9:
        if (name) return "Slow Monster";
        if (desc) return "Attempts to slow a monster.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                slow_monster(dir, power);
            }
        }
        break;

    case 10:
        if (name) return "Confuse Monster";
        if (desc) return "Attempts to confuse a monster.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                confuse_monster(dir, power);
            }
        }
        break;

    case 11:
        if (name) return "Scare Monster";
        if (desc) return "Attempts to scare a monster.";

        {
            int power = spell_power(plev * 3 / 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fear_monster(dir, power);
            }
        }
        break;

    case 12:
        if (name) return "Semicolon of Punishment";
        if (desc) return "Inscribes a semicolon of punishment on the floor beneath you.";
        if (info) return info_damage(0, 0, 32 + plev);

        if (cast)
            set_trap(py, px, feat_semicolon);
        break;

    case 13:
        if (name) return "Confuse Everybody";
        if (desc) return "Attempts to confuse all monsters in sight.";

        {
            int power = spell_power(plev * 2 - 5);

            if (info) return info_power(power);

            if (cast)
            {
                confuse_monsters(power);
            }
        }
        break;

    case 14:
        if (name) return "Create Doors";
        if (desc) return "Creates doors on all surrounding squares.";

        if (cast)
        {
            project(0, 1, py, px, 0, GF_MAKE_DOOR, PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE);
            p_ptr->update |= (PU_FLOW);
            p_ptr->redraw |= (PR_MAP);
        }
        break;

    case 15:
        if (name) return "Semicolon of Warding";
        if (desc) return "Inscribes a semicolon of warding on the floor beneath you.";

        {
            if (cast)
            {
                warding_glyph();
            }
        }
        break;

    /* Building Alternative Realities */
    case 16:
        if (name) return "Charm Monster";
        if (desc) return "Attempts to charm a monster.";

        if (cast)
        {
            int power = (p_ptr->lev / 2) + damroll((p_ptr->pclass == CLASS_LAWYER) ? 7 : 5, 7);
            if (!get_fire_dir(&dir)) return NULL;

            charm_monster(dir, power);
        }
        break;

    case 17:
        if (name) return "Expert Trap";
        if (desc) return "Sets a trap under you. The trap will have a random effect on the monster that triggers it.";

        if (cast)
            set_trap(py, px, feat_rogue_trap2);
        break;

    case 18:
        if (name) return "Getaway";
        if (desc) return "Provides a random means of escape.";
        if (cast)
        {
            switch (randint1(13))
            {
            case 1: case 2: case 3: case 4: case 5:
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(10, 0L);
                break;
            case 6: case 7: case 8: case 9: case 10:
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(222, 0L);
                break;
            case 11: case 12:
                stair_creation(FALSE);
                break;
            default:
                (void)py_teleport_level("Teleport Level? ");
            }
        }
        break;

    case 19:
        if (name) return "Blame Undead";
        if (desc) return "Hurts all undead monsters in sight.";

        {
            int dam = spell_power(50 + plev + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
                dispel_undead(dam);
        }
        break;

    case 20:
        if (name) return "Probe";
        if (desc) return "Reveals information about nearby monsters.";
        if (cast) probing();
        break;

    case 21:
        if (name) return "Spin";
        if (desc) return "Provides temporary resistance to nether and makes charming effects more powerful.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_spin(randint1(base) + base, FALSE);
            }
        }
        break;

    case 22:
        if (name) return "Advanced Bloodsucking";
        if (desc) return "Absorbs life from a nearby living creature.";

        {
            int dam = spell_power(50 + (plev * 2 / 3) + p_ptr->to_d_spell/3);
            if (prace_is_(RACE_VAMPIRE)) { dam *= ((p_ptr->pclass == CLASS_LAWYER) ? 4 : 3); dam /= 2; }

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                virtue_add(VIRTUE_SACRIFICE, -1);
                virtue_add(VIRTUE_VITALITY, -1);

                if (drain_life(dir, dam) && p_ptr->pclass != CLASS_BLOOD_MAGE)
                       vamp_player(dam);
            }
        }
        break;

    case 23:
        if (name) return "Alter Reality";
        if (desc) return "Recreates the current dungeon level.";

        if (cast) { p_ptr->alter_reality = 2; do_alter_reality(); }
        break;

    /* Acquiris Quodcumque Rapis */
    case 24:
        if (name) return "Blink";
        if (desc) return "Teleport a short distance.";

        {
            int range = 10;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use = 30;
                teleport_player(range, 0L);
            }
        }
        break;

    case 25:
        if (name) return "Tread Softly";
        if (desc) return "Grants enhanced stealth for a bit.";
        {
            int base = spell_power(50);

            if (info) return info_duration(base, base);
            if (cast)
                set_tim_dark_stalker(base + randint1(base), FALSE);
        }
        break;

    case 26:
        lisa = ((plev >= 48) && (lawyer_aptitude));
        if (name) return "Map Surroundings";
        if (desc) return ((lisa) ? "Maps and detects the nearby area." : "Maps the nearby area.");

        {
            int rad = DETECT_RAD_MAP;

            if (info) return info_radius(rad);

            if (cast)
            {
                map_area(rad);
                if (lisa) detect_all(rad);
            }
        }
        break;

    case 27:
        if (name) return "Dig Deep";
        if (desc) return "Fires a beam of disintegration.";

        {
            int dam = spell_power(30);
            int range = spell_power(plev / 4 + 1);
            if (lawyer_aptitude) range += 4;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                project_length = range;

                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_DISINTEGRATE, dir, dam);
            }
        }
        break;

    case 28:
        if (name) return "Unholy Rage";
        if (desc) return "Puts you in a temporary berserk rage and heals 75 HP.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_shero(randint1(base) + base, FALSE);
                hp_player(75);
            }
        }
        break;


    case 29:
        if (name) return "Subpoena";
        if (desc) return "Teleports a visible monster to you.";

        if (cast)
        {
            monster_type *m_ptr;
            monster_race *r_ptr;
            char m_name[80];

            if (!target_set(TARGET_KILL)) return NULL;
            if (!cave[target_row][target_col].m_idx) return NULL;
            if (!player_has_los_bold(target_row, target_col)) return NULL;
            if (!projectable(py, px, target_row, target_col)) return NULL;

            /* This would cause bizarre behavior */
            if (cave[target_row][target_col].m_idx == p_ptr->riding) return NULL;

            m_ptr = &m_list[cave[target_row][target_col].m_idx];
            r_ptr = &r_info[m_ptr->r_idx];
            monster_desc(m_name, m_ptr, 0);
            if (r_ptr->flagsr & RFR_RES_TELE)
            { /* Note lack of check for RF1_UNIQUE */
                if (r_ptr->flagsr & RFR_RES_ALL)
                {
                    mon_lore_r(m_ptr, RFR_RES_TELE);
                    msg_format("%^s is unaffected!", m_name);
                    break;
                }
                else if (r_ptr->level > (p_ptr->pclass == CLASS_LAWYER ? randint1(100) + plev : randint1(100)))
                {
                    mon_lore_r(m_ptr, RFR_RES_TELE);
                    msg_format("%^s resists!", m_name);
                    break;
                }
            }
            /* Wake it up */
            (void)set_monster_csleep(cave[target_row][target_col].m_idx, 0);
            msg_format("You command %s to return.", m_name);
            teleport_monster_to(cave[target_row][target_col].m_idx, py, px, 100, TELEPORT_PASSIVE);
        }
        break;

    case 30:
        if (name) return "Teleport";
        if (desc) return "Teleport a long distance.";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use = 30;
                teleport_player(range, 0L);
            }
        }
        break;

    case 31:
        if (name) return "Dazzle";
        if (desc) return "Attempts to slow, stun, confuse, scare and freeze nearby monsters.";

        {
            int power = spell_power(60 + plev);
            if (info) return info_power(power);
            if (cast)
            {
                slow_monsters(power);
                stun_monsters(5 + power/10);
                confuse_monsters(power);
                turn_monsters(power);
                stasis_monsters(power/3);
            }
        }
        break;

    }
    return "";
}

/****************************************************************************
 * Bonuses
 ****************************************************************************/
static void _calc_bonuses(void)
{
    p_ptr->skill_dig += p_ptr->lev / 2;
    p_ptr->skills.stl += p_ptr->lev / 15;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "legal trick";
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 33;
        me.encumbrance.enc_wgt = 1000;
        init = TRUE;
    }
    me.which_stat = A_WIS;
    me.min_level = 1;
    me.min_fail = 0;
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_SCROLL, SV_SCROLL_PHASE_DOOR, 3 + randint1(3));
    py_birth_spellbooks();

    p_ptr->au += 200;
}

/****************************************************************************
 * Public
 ****************************************************************************/
class_t *lawyer_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  33,  42,   3,  40,  32,  55,  40};
    skills_t xs = { 15,  11,  10,   0,   1,   0,  15,  12};

        me.name = "Lawyer";
        me.desc = "Known for his unsurpassed expertise in the mysterious realm of Law,"
                  " the Lawyer has a comprehensive skillset that allows him to set and"
                  " detect traps, to slow or mystify his enemies, to quietly escape or"
                  " avoid dangerous fights, to acquire knowledge about the world around"
                  " him, and to make friends and influence people. As he relies on his"
                  " familiarity with Law and with tricks and approaches that have"
                  " worked in the past, the lawyer uses Wisdom as his spellcasting stat.";

        me.stats[A_STR] = -3;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 102;
        me.base_hp = 12;
        me.exp = 115;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
