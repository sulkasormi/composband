#include "angband.h"

/****************************************************************************
 * Burglary, the preferred realm for the Rogue
 ****************************************************************************/
static cptr _rogue_pick_pocket(int power)
{
    int           y, x, m_idx, dir;
    monster_type *m_ptr;
    monster_race *r_ptr;
    char          m_name[MAX_NLEN];
    char          o_name[MAX_NLEN];

    power += p_ptr->lev;
    power += adj_stat_save[p_ptr->stat_ind[A_DEX]];

    if (!get_rep_dir2(&dir)) return NULL;
    if (dir == 5) return NULL;

    y = py + ddy[dir];
    x = px + ddx[dir];

    if (!cave[y][x].m_idx)
    {
        msg_print("There is no monster.");
        return NULL;
    }

    m_idx = cave[y][x].m_idx;
    m_ptr = &m_list[m_idx];
    r_ptr = &r_info[m_ptr->r_idx];

    if (!m_ptr->ml || p_ptr->image) /* Can't see it, so can't steal! */
    {
        msg_print("There is no monster.");
        return NULL;
    }

    monster_desc(m_name, m_ptr, 0);

    if ( !mon_save_aux(m_ptr->r_idx, power)
      || (MON_CSLEEP(m_ptr) && !mon_save_aux(m_ptr->r_idx, power)))
    {
        object_type loot = {0};

        if (m_ptr->hold_o_idx && one_in_(2))
        {
            object_copy(&loot, &o_list[m_ptr->hold_o_idx]);
            delete_object_idx(m_ptr->hold_o_idx);
            loot.held_m_idx = 0;
            object_origins(&loot, ORIGIN_STOLEN);
            loot.origin_xtra = m_ptr->r_idx;
        }
        else if (m_ptr->drop_ct > m_ptr->stolen_ct)
        {
            if (get_monster_drop(m_idx, &loot))
            {
                m_ptr->stolen_ct++;
                if (r_ptr->flags1 & RF1_UNIQUE)
                    r_ptr->stolen_ct++;
                object_origins(&loot, ORIGIN_STOLEN);
                loot.origin_xtra = m_ptr->r_idx;
            }
        }

        if (!loot.k_idx)
        {
            msg_print("There is nothing to steal!");
        }
        else
        {
            object_desc(o_name, &loot, 0);
            if (mon_save_aux(m_ptr->r_idx, power))
            {
                msg_format("Oops! You drop %s.", o_name);
                drop_near(&loot, -1, y, x);
            }
            else if (loot.tval == TV_GOLD)
            {
                if ((p_ptr->prace == RACE_WEREWOLF) && (strpos("silver", o_name)))
                {
                    msg_print("You steal some silver coins, but drop them immediately.");
                    take_hit(DAMAGE_NOESCAPE, randint1(10), "contact with silver coins");
                }
                else {
                    msg_format("You steal %d gold pieces' worth of %s.", (int)loot.pval, o_name);
                    sound(SOUND_SELL);
                    p_ptr->au += loot.pval;
                    stats_on_gold_find(loot.pval);
                    p_ptr->redraw |= (PR_GOLD);
                }
            }
            else
            {
                pack_carry(&loot);
                msg_format("You steal %s.", o_name);
            }
        }

        if ((r_ptr->flags1 & RF1_UNIQUE) || mon_save_aux(m_ptr->r_idx, power))
        {
            set_monster_csleep(m_idx, 0);
            if ( allow_ticked_off(r_ptr)
              && ((r_ptr->flags1 & RF1_UNIQUE) || mon_save_aux(m_ptr->r_idx, power)) )
            {
                msg_format("%^s wakes up and looks very mad!", m_name);
                mon_anger(m_ptr);
            }
            else
                msg_format("%^s wakes up.", m_name);
        }

        if (loot.k_idx)
        {
            if (mon_save_aux(m_ptr->r_idx, power))
                msg_print("You fail to run away!");
            else
            {
                if (p_ptr->lev < 35 || get_check("Run away?"))
                    teleport_player(25 + p_ptr->lev/2, 0L);
            }
        }
    }
    else if (MON_CSLEEP(m_ptr))
    {
        set_monster_csleep(m_idx, 0);
        if (allow_ticked_off(r_ptr))
        {
            msg_format("Failed! %^s wakes up and looks very mad!", m_name);
            mon_anger(m_ptr);
        }
        else
            msg_format("Failed! %^s wakes up.", m_name);
    }
    else if (allow_ticked_off(r_ptr))
    {
        msg_format("Failed! %^s looks very mad!", m_name);
        mon_anger(m_ptr);
    }
    else
    {
        msg_print("Failed!");
    }

    if (is_friendly(m_ptr) || is_pet(m_ptr))
    {
        msg_format("%^s suddenly becomes hostile!", m_name);
        set_hostile(m_ptr);
    }
    return "";
}

static cptr _rogue_negotiate(void)
{
    int           m_idx = 0;
    monster_type *m_ptr;
    monster_race *r_ptr;
    char          m_name[MAX_NLEN];

    if (target_set(TARGET_MARK))
    {
        if (target_who > 0)
            m_idx = target_who;
        else
            m_idx = cave[target_row][target_col].m_idx;
    }

    if (!m_idx)
    {
        msg_print("There is no monster.");
        return NULL;
    }

    m_ptr = &m_list[m_idx];
    r_ptr = &r_info[m_ptr->r_idx];

    if (!m_ptr->ml || p_ptr->image)
    {
        msg_print("There is no monster.");
        return NULL;
    }

    monster_desc(m_name, m_ptr, 0);

    if (is_pet(m_ptr) || is_friendly(m_ptr))
    {
        msg_format("%^s is already in your services.", m_name);
        return NULL;
    }

    set_monster_csleep(m_idx, 0);

    if (r_ptr->flags2 & RF2_THIEF)
        mon_lore_2(m_ptr, RF2_THIEF);

    if (!(r_ptr->flags2 & RF2_THIEF))
    {
        msg_format("%^s is not open to any sort of deal!", m_name);
    }
    else if (!mon_save_p(m_ptr->r_idx, A_CHR))
    {
        int cost = 10 + r_ptr->level * 100;

        if (r_ptr->flags1 & RF1_UNIQUE)
            cost *= 10;

        if (p_ptr->au >= cost)
        {
            msg_format("%^s says 'My services will cost you %d gold pieces.'", m_name, cost);

            if (get_check("Do you pay?"))
            {
                sound(SOUND_SELL);
                p_ptr->au -= cost;
                stats_on_gold_services(cost);
                p_ptr->redraw |= PR_GOLD;

                if (mon_save_p(m_ptr->r_idx, A_CHR))
                {
                    msg_format("%^s says 'Fool! Never trust a thief!'", m_name);
                    mon_anger(m_ptr);
                }
                else
                {
                    msg_format("%^s says 'Deal!'", m_name);
                    if (!(r_ptr->flags1 & RF1_UNIQUE) && !mon_save_p(m_ptr->r_idx, A_CHR))
                        set_pet(m_ptr);
                    else
                        set_friendly_ingame(m_ptr);
                }
            }
            else
            {
                msg_format("%^s says 'Scoundrel!'", m_name);
                mon_anger(m_ptr);
            }
        }
        else
        {
            msg_format("%^s says 'Hah! You can't afford my help!", m_name);
        }
    }
    else
    {
        msg_format("%^s is insulted you would ask such a question!", m_name);
        mon_anger(m_ptr);
    }
    return "";
}


cptr do_burglary_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;

    int plev = p_ptr->lev;
    int rad = DETECT_RAD_DEFAULT;
    int dir;

    if (plev >= 45)
        rad = DETECT_RAD_ALL;
    else
        rad += plev;

    switch (spell)
    {
    /* Burglar's Handbook */
    case 0:
        if (name) return "Detect Traps";
        if (desc) return "Detects nearby traps.";
        if (info) return info_radius(rad);
        if (cast)
            detect_traps(rad, TRUE);
        break;

    case 1:
        if (name) return "Disarm Traps";
        if (desc) return "Fires a beam which disarms traps.";

        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            disarm_trap(dir);
        }
        break;

    case 2:
        if (name) return "Detect Treasure";
        if (desc) return "Detects all treasures in your vicinity.";
        if (info) return info_radius(rad);

        if (cast)
        {
            detect_treasure(rad);
            detect_objects_gold(rad);
        }
        break;

    case 3:
        if (name) return "Detect Objects";
        if (desc) return "Detects all items in your vicinity.";
        if (info) return info_radius(rad);

        if (cast)
            detect_objects_normal(rad);
        break;

    case 4:
        if (name) return "See in the Dark";
        if (desc) return "Gives infravision for a while.";
        {
            int base = spell_power(100);

            if (info) return info_duration(base, base);

            if (cast)
                set_tim_infra(base + randint1(base), FALSE);
        }
        break;

    case 5:
        if (name) return "Tread Softly";
        if (desc) return "Grants enhanced stealth for a bit.";
        {
            int base = spell_power(50);

            if (info) return info_duration(base, base);
            if (cast)
                set_tim_dark_stalker(base + randint1(base), FALSE);
        }
        break;

    case 6:
        if (name) return "Minor Getaway";
        if (desc) return "Teleport medium distance.";

        {
            int range = 30;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use = 30;
                teleport_player(range, 0L);
            }
        }
        break;

    case 7:
        if (name) return "Set Minor Trap";
        if (desc) return "Sets a weak trap under you. This trap will have various weak effects on a passing monster.";

        if (cast)
            set_trap(py, px, feat_rogue_trap1);
        break;

    /* Thieving Ways */
    case 8:
        if (name) return "Map Escape Route";
        if (desc) return "Maps nearby area.";
        if (info) return info_radius(rad);

        if (cast)
            map_area(rad);
        break;

    case 9:
        if (name) return "Pick Pocket";
        if (desc) return "Attempt to steal an item or treasure from an adjacent monster.";

        if (cast)
            return _rogue_pick_pocket(0);
        break;

    case 10:
        if (name) return "Negotiate";
        if (desc) return "Attempt to bargain for the services of a nearby thief.";

        if (cast)
            return _rogue_negotiate();
        break;

    case 11:
        if (name) return "Fetch Object";
        if (desc) return "Pulls a distant item close to you.";

        {
            int weight = spell_power(plev * 15);
            if (info) return info_weight(weight);
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;
                fetch(dir, weight, FALSE);
            }
        }
        break;

    case 12:
        if (name) return "Eye for Danger";
        if (desc) return "Gives telepathy for a while.";
        {
            int base = 25;
            int sides = 30;

            if (info) return info_duration(base, sides);

            if (cast)
                set_tim_esp(randint1(sides) + base, FALSE);
        }
        break;

    case 13:
        if (name) return "Examine Loot";
        if (desc) return "Identifies an item.";

        if (cast)
        {
            if (!ident_spell(NULL))
                return NULL;
        }
        break;

    case 14:
        if (name) return "Set Major Trap";
        if (desc) return "Sets a trap under you. This trap will have various effects on a passing monster.";

        if (cast)
            set_trap(py, px, feat_rogue_trap2);
        break;

    case 15:
        if (name) return "Make Haste";
        if (desc) return "Hastes you for a while.";

        {
            int base = spell_power(plev);
            int sides = spell_power(20 + plev);

            if (info) return info_duration(base, sides);

            if (cast)
                set_fast(randint1(sides) + base, FALSE);
        }
        break;

    /* Great Escapes */
    case 16:
        if (name) return "Create Stairs";
        if (desc) return "Creates a flight of stairs underneath you.";

        if (cast)
            stair_creation(FALSE);
        break;

    case 17:
        if (name) return "Panic Hit";
        if (desc) return "Attack an adjacent monster and attempt a getaway.";

        if (cast)
        {
            int dir = 0;
            int x, y;

            if (!get_rep_dir2(&dir)) return NULL;
            y = py + ddy[dir];
            x = px + ddx[dir];
            if (cave[y][x].m_idx)
            {
                py_attack(y, x, 0);
                if (randint0(p_ptr->skills.dis) < 7)
                    msg_print("You failed to teleport.");
                else
                    teleport_player(30, 0);
            }
            else
            {
                msg_print("You don't see any monster in this direction");
                msg_print(NULL);
                return NULL;
            }
        }
        break;

    case 18:
        if (name) return "Panic Shot";
        if (desc) return "Shoot a nearby monster and attempt a getaway.";

        if (cast)
        {
            if (!do_cmd_fire()) return NULL;
            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("You failed to teleport.");
            else
                teleport_player(30, 0);
        }
        break;

    case 19:
        if (name) return "Panic Summons";
        if (desc) return "Summon assistance and attempt a getaway.";

        if (cast)
        {
            trump_summoning(damroll(2, 3), !fail, py, px, 0, SUMMON_THIEF, PM_ALLOW_GROUP);

            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("You failed to teleport.");
            else
                teleport_player(30, 0);
        }
        break;

    case 20:
        if (name) return "Panic Traps";
        if (desc) return "Set multiple weak traps and attempt a getaway.";

        if (cast)
        {
            int y = 0, x = 0;
            int dir;

            for (dir = 0; dir <= 8; dir++)
            {
                y = py + ddy_ddd[dir];
                x = px + ddx_ddd[dir];

                set_trap(y, x, feat_rogue_trap1);
            }

            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("You failed to teleport.");
            else
                teleport_player(30, 0);
        }
        break;

    case 21:
        if (name) return "Flee Level";
        if (desc) return "Flee your current level without delay.";

        if (cast)
        {
            if (!py_teleport_level("Are you sure? (Flee Level) ")) return NULL;
        }
        break;

    case 22:
        if (name) return "New Beginnings";
        if (desc) return "Recreates current dungeon level after a short delay.";
        if (info) return info_delay(15, 20);

        if (cast)
            alter_reality();
        break;

    case 23:
        if (name) return "Major Getaway";
        if (desc) return "Teleport long distance with very little energy use.";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                energy_use = 15;
                teleport_player(range, 0L);
            }
        }
        break;

    /* Book of Shadows */
    case 24:
        if (name) return "Protect Loot";
        if (desc) return "For a long time, items in your inventory will have a chance at resisting destruction.";

        {
            int base = spell_power(plev*2);
            int sides = spell_power(plev*2);

            if (info) return info_duration(base, sides);

            if (cast)
                set_tim_inven_prot(randint1(sides) + base, FALSE);
        }
        break;

    case 25:
        if (name) return "Teleport To";
        if (desc) return "Teleport a visible monster next to you without disturbing it.";

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
            if (mon_save_tele_to(m_ptr, m_name, TRUE)) break;
            msg_format("You command %s to return.", m_name);
            teleport_monster_to(cave[target_row][target_col].m_idx, py, px, 100, TELEPORT_PASSIVE);
        }
        break;

    case 26:
        if (name) return "Master Thievery";
        if (desc) return "The ultimate in thievery. With a light touch, you attempt to relieve monsters of their goods.";

        if (cast)
            return _rogue_pick_pocket(100);
        break;

    case 27:
        if (name) return "Shadow Storm";
        if (desc) return "Fires a huge ball of darkness.";

        {
            int dam = spell_power(10 * (plev - 20) + p_ptr->to_d_spell);
            int rad = spell_power(4);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_DARK, dir, dam, rad);
            }
        }
        break;

    case 28:
        if (name) return "Hide in Shadows";
        if (desc) return "You become shrouded in darkness, your torch light magically dimmed.";
        {
            int d = plev;
            if (info) return info_duration(spell_power(d), spell_power(d));
            if (cast)
            {
                if (p_ptr->tim_superstealth)
                {
                    msg_print("You are already hiding in the shadows.");
                    return NULL;
                }
                set_tim_superstealth(spell_power(randint1(d) + d), FALSE);
            }
        }
        break;

    case 29:
        if (name) return "Den of Thieves";
        if (desc) return "As a Thief Lord, you may summon assistance from your minions at will.";
        if (cast)
        {
            int i;
            for (i = 0; i < 12; i++)
            {
                int attempt = 10;
                int my = 0, mx = 0;

                while (attempt--)
                {
                    scatter(&my, &mx, py, px, 4, 0);

                    /* Require empty grids */
                    if (cave_empty_bold2(my, mx)) break;
                }
                if (attempt < 0) continue;
                summon_specific(-1, my, mx, plev*3/2, SUMMON_THIEF, PM_FORCE_PET | PM_HASTE);
            }
        }
        break;

    case 30:
        if (name) return "Set Ultimate Trap";
        if (desc) return "Sets an extremely powerful trap under you. This trap will have various strong effects on a passing monster.";

        if (cast)
            set_trap(py, px, feat_rogue_trap3);
        break;

    case 31:
        if (name) return "Assassinate";
        if (desc) return "Attempt to instantly kill a sleeping monster.";

        if (cast)
        {
            int y, x, dir;
            if (!get_rep_dir2(&dir)) return NULL;
            if (dir == 5) return NULL;

            y = py + ddy[dir];
            x = px + ddx[dir];

            if (cave[y][x].m_idx)
            {
                monster_type *m_ptr = &m_list[cave[y][x].m_idx];
                if (MON_CSLEEP(m_ptr))
                    py_attack(y, x, ROGUE_ASSASSINATE);
                else
                {
                    msg_print("This only works for sleeping monsters.");
                    return NULL;
                }
            }
            else
            {
                msg_print("There is no monster.");
                return NULL;
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
    /* rogues are decent shooters all around, but especially good with slings */
    slot_t slot = equip_find_obj(TV_BOW, SV_SLING); /* fyi, shooter_info not set yet ... */
    if (slot) p_ptr->skills.thb += 20 + p_ptr->lev;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 33;
        me.encumbrance.enc_wgt = 1000;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    if (p_ptr->realm1 == REALM_BURGLARY)
    {
        me.which_stat = A_DEX;
        me.min_level = 1;
        me.min_fail = 0;
    }
    else
    {
        me.which_stat = A_INT;
        me.min_level = 5;
        me.min_fail = 5;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_SCROLL, SV_SCROLL_TELEPORT, randint1(3));
    py_birth_spellbooks();

    p_ptr->au += 200;
}

/****************************************************************************
 * Public
 ****************************************************************************/
class_t *rogue_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  37,  36,   5,  32,  24,  60,  60};
    skills_t xs = { 15,  12,  10,   0,   0,   0,  21,  14};

        me.name = "Rogue";
        me.desc = "A Rogue is a character who prefers to live by their cunning, yet is "
                    "capable of fighting their way out of a tight spot when necessary. Rogues "
                    "are good at locating secret doors and hidden traps, and are masters of "
                    "trap-disarming and lockpicking. Their exceptional stealth allows "
                    "rogues to sneak around sleeping creatures without having to fight; "
                    "they can also backstab a fleeing monster or surprise an "
                    "enemy with a telling first blow. Rogues are fairly good at ranged combat, "
                    "and receive special bonuses when shooting with a sling.\n \n"
                    "Rogues can select one realm - Sorcery, Death, Trump, Arcane, Craft, Law, "
                    "or Burglary. Except for this last realm, rogues have certain limitations " 
                    "on which spells they can learn, and they do not learn new spells very quickly. "
                    "The Burglary realm is unique to the rogue, and the speciality of the class; it "
                    "offers spells for setting traps, picking pockets, negotiating with "
                    "other thieves, and escaping from a tight spot. Burglary rogues are "
                    "agents of the Black Market and receive favorable pricing from "
                    "that shop. Burglary rogues use Dexterity as their spellcasting stat, "
                    "and can use this special art from the beginning of their adventures; rogues of other "
                    "realms, however, rely on Intelligence and cannot learn magic spells before they "
                    "reach level 5.";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 12;
        me.exp = 125;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;
        
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
