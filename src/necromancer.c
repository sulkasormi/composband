#include "angband.h"

/* HACK: Repose of the dead paralyzes the player for a few turns.
   When they wake up, they are restored. See set_paralyzed() for details.*/
bool repose_of_the_dead = FALSE;


/**********************************************************************
 * Utilities
 **********************************************************************/
static bool _necro_check_touch(void)
{
    int slot;
    if (p_ptr->afraid)
    {
        msg_print("You are too scared to do that!");
        return FALSE;
    }
    if (!equip_find_empty_hand() && !mut_present(MUT_DRACONIAN_METAMORPHOSIS))
    {
        msg_print("You need a free hand to touch.");
        return FALSE;
    }

    slot = equip_find_obj(TV_GLOVES, SV_ANY);
    if (slot && equip_obj(slot)->name1 != ART_HAND_OF_VECNA)
    {
        msg_print("You can't touch while wielding gloves.");
        return FALSE;
    }
    return TRUE;
}

static cptr _necro_info_damage(int dice, int sides, int base)
{
    if (equip_find_art(ART_HAND_OF_VECNA))
    {
        dice *= 2;
        base *= 2;
    }
    return info_damage(dice, spell_power(sides), spell_power(base));
}

static int _necro_damroll(int dice, int sides, int base)
{
    if (equip_find_art(ART_HAND_OF_VECNA))
    {
        dice *= 2;
        base *= 2;
    }
    return spell_power(damroll(dice, sides) + base);
}

void on_p_hit_m(int m_idx)
{
    if ((p_ptr->special_attack & ATTACK_CONFUSE) || (hex_spelling(HEX_CONFUSION)))
    {
        monster_type *m_ptr = &m_list[m_idx];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];
        char          m_name[MAX_NLEN];

        monster_desc(m_name, m_ptr, 0);

        if (p_ptr->special_attack & ATTACK_CONFUSE)
        {
            p_ptr->special_attack &= ~(ATTACK_CONFUSE);
            msg_print("Your hands stop glowing.");
            p_ptr->redraw |= (PR_STATUS);
        }

        if (r_ptr->flags3 & RF3_NO_CONF)
        {
            mon_lore_3(m_ptr, RF3_NO_CONF);
            msg_format("%^s is unaffected.", m_name);
        }
        else if (randint0(100) < r_ptr->level)
        {
            msg_format("%^s is unaffected.", m_name);
        }
        else
        {
            msg_format("%^s appears confused.", m_name);
            (void)set_monster_confused(m_idx, MON_CONFUSED(m_ptr) + 10 + randint0(p_ptr->lev) / 5);
        }
    }
}

static bool _necro_do_touch(int type, int dice, int sides, int base)
{
    int x, y;
    int dir = 0;
    int m_idx = 0;

    if (!_necro_check_touch()) return FALSE;

    /* For ergonomics sake, use currently targeted monster. This allows
       a macro of \e*tmaa or similar to pick an adjacent foe, while
       \emaa*t won't work, since get_rep_dir2() won't allow a target. */
    if (old_target_okay())
    {
        y = target_row;
        x = target_col;
        m_idx = cave[y][x].m_idx;
        if (m_idx)
        {
            if (m_list[m_idx].cdis > 1)
                m_idx = 0;
            else
                dir = 5; /* Hack so that fire_ball() works correctly */
        }
    }

    if (!m_idx)
    {
        if (!get_rep_dir2(&dir)) return FALSE;
        if (dir == 5) return FALSE;

        y = py + ddy[dir];
        x = px + ddx[dir];
        m_idx = cave[y][x].m_idx;

        if (!m_idx)
        {
            msg_print("There is no monster there.");
            return FALSE;
        }

    }

    if (m_idx)
    {
        int dam;
        monster_type *m_ptr = &m_list[m_idx];

        if (!is_hostile(m_ptr) &&
            !(p_ptr->stun || p_ptr->confused || p_ptr->image ||
            IS_SHERO() || !m_ptr->ml))
        {
            if (!get_check("Really hit it? "))
                return FALSE;
        }

        dam = _necro_damroll(dice, sides, base);
        on_p_hit_m(m_idx);
        touch_zap_player(m_idx);
        if (fire_ball(type, dir, dam, 0))
        {
            if (type == GF_OLD_DRAIN)
                hp_player(dam);
        }
    }
    return TRUE;
}

static void _necro_do_summon(int what, int num, bool fail)
{
    int x = px;
    int y = py;

    if (fail) /* Failing spells should not be insta-death ... */
        num = MAX(1, num/4);
    else
        num = spell_power(num);

    if (!fail && old_target_okay() && los(py, px, target_row, target_col) && !one_in_(3))
    {
        y = target_row;
        x = target_col;
    }
    if (trump_summoning(num, !fail, y, x, 0, what, PM_ALLOW_UNIQUE))
    {
        if (fail)
        {
            if (num == 1)
                msg_print("The summoned monster gets angry!");
            else
                msg_print("The summoned monsters get angry!");
        }
    }
}

/**********************************************************************
 * Spells: Note, we are still using the old "Book Spell System"
 **********************************************************************/
cptr do_necromancy_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;

    int plev = p_ptr->lev;

    switch (spell)
    {
    /* Stench of Death */
    case 0:
        if (name) return "Cold Touch";
        if (desc) return "Damage an adjacent monster with a chilling touch.";
        if (info) return _necro_info_damage(2, 6, plev + p_ptr->to_d_spell);
        if (cast && !_necro_do_touch(GF_COLD, 2, 6, plev + p_ptr->to_d_spell)) return NULL;
        break;

    case 1:
        if (name) return "Summon Rat";
        if (desc) return "Summons a rat to feast on the dead!";
        if (cast || fail) _necro_do_summon(SUMMON_RAT, 1, fail);
        break;

    case 2:
        if (name) return "Detect Life";
        if (desc) return "Detects all living monsters in your vicinity.";
        if (info) return info_radius(DETECT_RAD_DEFAULT);
        if (cast) detect_monsters_living(DETECT_RAD_DEFAULT, "You sense the presence of life around you.");
        break;

    case 3:
        if (name) return "Detect Unlife";
        if (desc) return "Detects all nonliving monsters in your vicinity.";
        if (info) return info_radius(DETECT_RAD_DEFAULT);
        if (cast) detect_monsters_nonliving(DETECT_RAD_DEFAULT);
        break;

    case 4:
        if (name) return "Poison Touch";
        if (desc) return "Damage an adjacent monster with a venomous touch.";
        if (info) return _necro_info_damage(4, 6, plev + p_ptr->to_d_spell);
        if (cast && !_necro_do_touch(GF_POIS, 4, 6, plev + p_ptr->to_d_spell)) return NULL;
        break;

    case 5:
        if (name) return "Summon Bats";
        if (desc) return "Summons bats to feast on the living!";
        if (cast || fail) _necro_do_summon(SUMMON_BAT, 1 + randint1(2), fail);
        break;

    case 6:
        if (name) return "Eldritch Howl";
        if (desc) return "Emit a terrifying howl.";
        if (cast) project_hack(GF_ELDRITCH_HOWL, spell_power(plev * 3));
        break;

    case 7:
        if (name) return "Black Touch";
        if (desc) return "Damage an adjacent monster with a dark touch.";
        if (info) return _necro_info_damage(6, 6, plev * 3 / 2 + p_ptr->to_d_spell);
        if (cast && !_necro_do_touch(GF_DARK, 6, 6, plev * 3 / 2 + p_ptr->to_d_spell)) return NULL;
        break;

    /* Sepulchral Ways */
    case 8:
        if (name) return "Summon Wolves";
        if (desc) return "Summons wolves to feast on the living!";
        if (cast || fail) _necro_do_summon(SUMMON_WOLF, 1 + randint1(2), fail);
        break;

    case 9:
        if (name) return "Black Cloak";
        if (desc) return "You become shrouded in darkness.";
        if (cast) 
        {
            set_tim_dark_stalker(spell_power(randint1(plev) + plev), FALSE);
        }
        break;

    case 10:
        if (name) return "Undead Sight";
        if (desc) return "Learn about your nearby surroundings by communing with the dead.";
        if (info) return info_radius(DETECT_RAD_MAP);
        if (cast)
        {
            map_area(DETECT_RAD_MAP);
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
        }
        break;

    case 11:
        if (name) return "Undead Lore";
        if (desc) return "Ask the dead to examine an object for you.";
        if (cast) ident_spell(NULL);
        break;

    case 12:
        if (name) return "Repelling Touch";
        if (desc) return "Conjure a foul wind to blow an adjacent monster away.";
    
        if (cast)
        {
            int y, x, dir;

            if (!_necro_check_touch()) return NULL;
            if (!get_rep_dir2(&dir)) return NULL;
            if (dir == 5) return NULL;

            y = py + ddy[dir];
            x = px + ddx[dir];

            if (!cave[y][x].m_idx)
            {
                msg_print("There is no monster.");
                return NULL;
            }
            else
            {
                int i;
                int ty = y, tx = x;
                int oy = y, ox = x;
                int m_idx = cave[y][x].m_idx;
                monster_type *m_ptr = &m_list[m_idx];
                char m_name[80];
    
                monster_desc(m_name, m_ptr, 0);
                touch_zap_player(cave[y][x].m_idx);    
    
                for (i = 0; i < 10; i++)
                {
                    y += ddy[dir];
                    x += ddx[dir];
                    if (cave_empty_bold(y, x))
                    {
                        ty = y;
                        tx = x;
                    }
                    else break;
                }
                if ((ty != oy) || (tx != ox))
                {
                    msg_format("A foul wind blows %s away!", m_name);
                    cave[oy][ox].m_idx = 0;
                    cave[ty][tx].m_idx = m_idx;
                    m_ptr->fy = ty;
                    m_ptr->fx = tx;
    
                    update_mon(m_idx, TRUE);
                    lite_spot(oy, ox);
                    lite_spot(ty, tx);
    
                    if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
                        p_ptr->update |= (PU_MON_LITE);
                }
            }
        }
        break;

    case 13:
        if (name) return "Vampiric Touch";
        if (desc) return "Steal life from an adjacent foe.";
        if (info) return _necro_info_damage(0, 0, plev * 4 + p_ptr->to_d_spell);
        if (cast && !_necro_do_touch(GF_OLD_DRAIN, 0, 0, plev * 4 + p_ptr->to_d_spell)) return NULL;
        break;

    case 14:
        if (name) return "Dread of Night";
        if (desc) return "Summons Dread to do your bidding. Beware of failure!";
        if (cast || fail) _necro_do_summon(SUMMON_DREAD, 1 + randint0(3), fail);
        break;

    case 15:
        if (name) return "Entomb";
        if (desc) return "Entombs chosen foe.";
        if (cast)
        {
            int dir; 
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball_hide(GF_ENTOMB, dir, plev, 0);
            p_ptr->update |= (PU_FLOW);
            p_ptr->redraw |= (PR_MAP);
        }
        break;

    /* Return of the Dead */
    case 16:
        if (name) return "Summon Zombies";
        if (desc) return "The dead are back and hungry for brains!";
        if (cast || fail) _necro_do_summon(SUMMON_ZOMBIE, 2 + randint1(3), fail);
        break;

    case 17:
        if (name) return "Summon Skeletons";
        if (desc) return "Summon skeletal assistance.";
        if (cast || fail) _necro_do_summon(SUMMON_SKELETON, 1 + randint0(3), fail);
        break;

    case 18:
        if (name) return "Summon Ghosts";
        if (desc) return "Recall the spirits of slain warriors for unholy servitude.";
        if (cast || fail) _necro_do_summon(SUMMON_GHOST, 1 + randint0(3), fail);
        break;

    case 19:
        if (name) return "Summon Vampires";
        if (desc) return "Its time to command the commanders!";
        if (cast || fail) _necro_do_summon(SUMMON_VAMPIRE, 1 + randint0(2), fail);
        break;

    case 20:
        if (name) return "Summon Wraiths";
        if (desc) return "Summon wights and wraiths to do your bidding.";
        if (cast || fail) _necro_do_summon(SUMMON_WIGHT, 1 + randint0(2), fail);
        break;

    case 21:
        if (name) return "Summon Liches";
        if (desc) return "Call forth former necromancers.";
        if (cast || fail) _necro_do_summon(SUMMON_LICH, 1 + randint0(2), fail);
        break;

    case 22:
        if (name) return "Unholy Word";
        if (desc) return "Utter an unspeakable word. The morale of your visible evil pets is temporarily boosted and they will serve you with renewed enthusiasm.";
        if (cast) project_hack(GF_UNHOLY_WORD, plev * 6);
        break;

    case 23:
        if (name) return "Lost Cause";
        if (desc) return "Make a last ditch Kamikaze effort for victory!";
        if (cast) discharge_minion();
        break;

    /* Necromatic Tome */
    case 24:
        if (name) return "Draining Touch";
        if (desc) return "Steal mana from an adjacent foe.";
        if (info) return _necro_info_damage(5, 5, plev/2 + p_ptr->to_d_spell);
        if (cast && !_necro_do_touch(GF_DRAINING_TOUCH, 5, 5, plev/2 + p_ptr->to_d_spell)) return NULL;
        break;

    case 25:
        if (name) return "Unhallow Ground";
        if (desc) return "Makes the current square unholy.";
        if (cast) warding_glyph(); /* TODO: Add new cave feature! */
        break;

    case 26:
    {
        int base = spell_power(20);
        if (name) return "Shield of the Dead";
        if (desc) return "Grants temporary protection";
        if (info) return info_duration(base, base);
        if (cast)
        {
            set_tim_res_nether(randint1(base) + base, FALSE);
            set_oppose_pois(randint1(base) + base, FALSE);
            set_oppose_cold(randint1(base) + base, FALSE);
            set_shield(randint1(base) + base, FALSE);
        }
        break;
    }
    case 27:
        if (name) return "Rending Touch";
        if (desc) return "Damage an adjacent monster with a disintegrating touch.";
        if (info) return _necro_info_damage(20, 20, plev + p_ptr->to_d_spell);
        if (cast && !_necro_do_touch(GF_DISINTEGRATE, 20, 20, plev + p_ptr->to_d_spell)) return NULL;
        break;

    case 28:
        if (name) return "Repose of the Dead";
        if (desc) return "Sleep the sleep of the dead for a few rounds, during which time nothing can awaken you, except perhaps death. When (if?) you wake up, you will be thoroughly refreshed!";
        if (cast)
        {
            if (!get_check("You will enter a deep slumber. Are you sure?")) return NULL;
            repose_of_the_dead = TRUE;
            set_paralyzed(4 + randint1(4), FALSE);
        }
        break;

    case 29:
        if (name) return "Sepulchral Wind";
        if (desc) return "You call forth the wind of the dead. All nearby monsters are blown away!";
        {
            int power = spell_power(plev * 4);
            if (info) return info_power(power);
            if (cast) banish_monsters(power);
        }
        break;

    case 30:
        if (name) return "Deadly Touch";
        if (desc) return "Attempt to kill an adjacent monster.";
        if (cast && !_necro_do_touch(GF_DEATH_TOUCH, 0, 0, plev * 200)) return NULL;
        break;

    case 31:
        if (name) return "Necromancy";
        if (desc) return "Bridge the world of the living with the world of the dead!  Vast hordes of undead will come forth to serve the one true necromancer!";
        if (cast)
        {
            int i;
            int sp_sides = 20 + plev;
            int sp_base = plev;
            int power = spell_power(plev);

            power += randint1(power);

            for (i = 0; i < 18; i++)
            {
                int attempt = 10;
                int my, mx, what;

                while (attempt--)
                {
                    scatter(&my, &mx, py, px, 4, 0);

                    /* Require empty grids */
                    if (cave_empty_bold2(my, mx)) break;
                }
                if (attempt < 0) continue;
                switch (randint1(4))
                {
                case 1: what = SUMMON_LICH; break;
                case 2: what = SUMMON_WIGHT; break;
                case 3: what = SUMMON_VAMPIRE; break;
                case 4:
                default: what = SUMMON_GHOST; break;
                }
                summon_specific(-1, my, mx, power, what, (PM_ALLOW_GROUP | PM_FORCE_PET | PM_HASTE));
            }
            set_fast(randint1(sp_sides) + sp_base, FALSE);
        }
        break;

    }

    return "";
}

static void _calc_bonuses(void)
{
    p_ptr->align -= 200;
    p_ptr->spell_cap += 2;

    if (p_ptr->lev >= 5) res_add(RES_COLD);
    if (p_ptr->lev >= 15) p_ptr->see_inv++;
    if (p_ptr->lev >= 25) p_ptr->hold_life++;
    if (p_ptr->lev >= 35) res_add(RES_POIS);
    if (p_ptr->lev >= 45) p_ptr->hold_life++;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 5) add_flag(flgs, OF_RES_COLD);
    if (p_ptr->lev >= 15) add_flag(flgs, OF_SEE_INVIS);
    if (p_ptr->lev >= 25) add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 35) add_flag(flgs, OF_RES_POIS);
}

static int _get_powers(spell_info* spells, int max)
{
    int ct = 0;

    spell_info* spell = &spells[ct++];
    spell->level = 1;
    spell->cost = 1;
    spell->fail = calculate_fail_rate(spell->level, 30, p_ptr->stat_ind[A_INT]);
    spell->fn = animate_dead_spell;

    spell = &spells[ct++];
    spell->level = 5;
    spell->cost = 5;
    spell->fail = calculate_fail_rate(spell->level, 30, p_ptr->stat_ind[A_INT]);
    spell->fn = enslave_undead_spell;

    return ct;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_spellbooks();
}

static bool _destroy_object(obj_ptr obj)
{
    if (obj->tval == TV_LIFE_BOOK || obj->tval == TV_CRUSADE_BOOK)
    {
        char name[MAX_NLEN];
        int  sp = 0;
        int  osp = p_ptr->csp;

        switch (obj->sval)
        {
        case 0: sp = 10; break;
        case 1: sp = 25; break;
        case 2: sp = 100; break;
        case 3: sp = 666; break;
        }

        sp_player(sp);
        object_desc(name, obj, OD_COLOR_CODED);
        msg_format("You gleefully destroy %s!", name);
        if (p_ptr->csp > osp)
            msg_print("You feel your head clear.");

        return TRUE;
    }
    return FALSE;
}

class_t *necromancer_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  40,  38,   4,  16,  20,  34,  20};
    skills_t xs = {  7,  15,  11,   0,   0,   0,   6,   7};

        me.name = "Necromancer";
        me.desc = "A Necromancer attempts to gain both power and knowledge through "
                  "communion with the dead. They use powerful necromancy magic to "
                  "summon aid from the dead, whether directly in terms of undead "
                  "servitude, or indirectly through other-worldly knowledge. Necromancy "
                  "also offers myriad foul offensive spells, but all of these require "
                  "the Necromancer to physically touch his foe. To do so, the Necromancer "
                  "may wield neither weapon, nor gloves. But a powerful necromancer is truly "
                  "awe inspiring, and may even kill foes with a single deadly touch! "
                  "In addition, they forever hunt for the legendary Eye and Hand of Vecna in "
                  "order to complete their power.",
        
        me.stats[A_STR] = -2;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] = -4;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 95;
        me.base_hp = 2;
        me.exp = 125;
        me.pets = 10;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        me.destroy_object = _destroy_object;
        init = TRUE;
    }

    return &me;
}
