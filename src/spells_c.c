#include "angband.h"

void cause_wounds_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cause Light Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to damage a single foe.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3, spell_power(8), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_1, dir, spell_power(damroll(3, 8) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void cause_wounds_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cause Medium Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to damage a single foe.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(8, spell_power(8), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_2, dir, spell_power(damroll(8, 8) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void cause_wounds_III_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cause Critical Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to damage a single foe.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(15), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_3, dir, spell_power(damroll(10, 15) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void cause_wounds_IV_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cause Mortal Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to damage a single foe.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(15, spell_power(15), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_4, dir, spell_power(damroll(15, 15) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void clairvoyance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Clairvoyance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Maps and lights whole dungeon level and gives telepathy for a while.");
        break;
    case SPELL_CAST:
        virtue_add(VIRTUE_KNOWLEDGE, 1);
        virtue_add(VIRTUE_ENLIGHTENMENT, 1);

        wiz_lite(p_ptr->tim_superstealth > 0);

        if (!p_ptr->telepathy)
            set_tim_esp(randint1(30) + 25, FALSE);

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void clear_mind_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Clear Mind");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Player regains 2+(L/30) SP. This won't work if the player has any pets.");
        break;
    case SPELL_CAST:
    {
        int amt;

        var_set_bool(res, FALSE);
        if (total_friends)
        {
            msg_print("You need to concentrate on your pets now.");
            return;
        }
        if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
        {
            msg_print("Your mind remains cloudy.");
            return;
        }

        msg_print("You feel your head clear a little.");

        amt = 2 + p_ptr->lev/30;

        sp_player(amt);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_clear_mind(void) { return cast_spell(clear_mind_spell); }

void confuse_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Confuse");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to confuse one or more monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 2));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 40)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            confuse_monster(dir, p_ptr->lev*2);
        }
        else
            confuse_monsters(p_ptr->lev*2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cold_touch_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cold Touch");
        break;
    case SPELL_DESC:
        var_set_string(res, "Freeze things with your icy fingers!");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(2 * p_ptr->lev)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your hands get very cold.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your hands warm up.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can freeze things with a touch.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int x, y;
        cave_type *c_ptr;

        if (!get_rep_dir2(&dir))
        {
            var_set_bool(res, FALSE);
            break;
        }
        var_set_bool(res, TRUE);
        y = py + ddy[dir];
        x = px + ddx[dir];
        c_ptr = &cave[y][x];

        if (!c_ptr->m_idx)
        {
            msg_print("You wave your hands in the air.");
            break;
        }
        fire_bolt(GF_COLD, dir, spell_power(2 * p_ptr->lev));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_cold_touch(void) { return cast_spell(cold_touch_spell); }

void confusing_lights_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Confusing Lights");
        break;
    case SPELL_DESC:
        var_set_string(res, "Emits confusing lights, slowing, stunning, confusing, scaring and freezing nearby monsters.");
        break;
    case SPELL_CAST:
        msg_print("You glare nearby monsters with a dazzling array of confusing lights!");
        slow_monsters(p_ptr->lev * 3);
        stun_monsters(5 + p_ptr->lev/5);
        confuse_monsters(p_ptr->lev * 3);
        turn_monsters(p_ptr->lev * 3);
        stasis_monsters(p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void crafting_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Crafting");
        break;
    case SPELL_DESC:
        var_set_string(res, "Makes chosen weapon, armor or ammo an ego item.");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        bool         okay = FALSE;
        char         o_name[MAX_NLEN];

        var_set_bool(res, FALSE);

        prompt.prompt = "Enchant which item?";
        prompt.error = "You have nothing to enchant.";
        prompt.filter = object_is_weapon_armour_ammo;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_EQUIP;
        prompt.where[2] = INV_QUIVER;
        prompt.where[3] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj || !prompt.obj->number) return;

        object_desc(o_name, prompt.obj, (OD_OMIT_PREFIX | OD_NAME_ONLY));

        if (!object_is_nameless(prompt.obj))
        {
            msg_print("You cannot enchant that item any further.");
            return;
        }

        if ((!object_is_ammo(prompt.obj)) && (prompt.obj->number > 1))
        {
            msg_print("You cannot use Crafting on more than one item at a time.");
            return;
        }

        if ((object_is_(prompt.obj, TV_SWORD, SV_POISON_NEEDLE)) ||
            (object_is_(prompt.obj, TV_SWORD, SV_RUNESWORD)) ||
            (object_is_(prompt.obj, TV_POLEARM, SV_DEATH_SCYTHE)))
        {
            msg_print("You cannot enchant that item.");
            return;
        }

        if (prompt.obj->number > 59)
        {
            msg_print("You cannot use Crafting on more than 59 projectiles at a time.");
            return;
        }

        if (prompt.obj->number > 30)
        {
            int mahis = ((prompt.obj->number * 20) - 597) / 6;
            if (!get_check(format("The enchantment has %s %d%% chance to fail. Proceed anyway? ", ((mahis / 10) == 8) ? "an" : "a", mahis)))
            return;
        }
        
        if (object_is_nameless(prompt.obj))
        {
            if (object_is_ammo(prompt.obj) && randint1(30) > (prompt.obj->number - 30))
            {
                if (brand_weapon_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
            else if (object_is_weapon(prompt.obj) && prompt.obj->number == 1)
            {
                if (brand_weapon_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
            else if (object_is_armour(prompt.obj) && prompt.obj->number == 1)
            {
                if (brand_armour_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
        }

        msg_format("The %s glow%s brightly!", o_name,
                ((prompt.obj->number > 1) ? "" : "s"));

        if (!okay)
        {
            if (flush_failure) flush();
            msg_print("The enchantment failed.");
            if (one_in_(3)) virtue_add(VIRTUE_ENCHANTMENT, -1);
        }
        else
        {
            virtue_add(VIRTUE_ENCHANTMENT, 1);
            object_origins(prompt.obj, ORIGIN_CRAFTING);
            prompt.obj->mitze_type = 0;
            obj_identify_fully(prompt.obj);
            if (!prompt.obj->mitze_type) object_mitze(prompt.obj, MITZE_ID);
            obj_display(prompt.obj);
            obj_release(prompt.obj, OBJ_RELEASE_ENCHANT);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_crafting(void) { return cast_spell(crafting_spell); }

void create_darkness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Create Darkness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Darken nearby area and inside of a room.");
        break;
    case SPELL_CAST:
        unlite_area(0, 3);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void create_food_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Create Food");
        break;
    case SPELL_DESC:
        if (p_ptr->prace == RACE_HOBBIT)
            var_set_string(res, "It's time for second breakfast!  Cook up a tasty meal.");
        else
            var_set_string(res, "Create a ration of tasty food.");
        break;
    case SPELL_CAST:
    {
        object_type forge;

        object_prep(&forge, lookup_kind(TV_FOOD, SV_FOOD_RATION));
        drop_near(&forge, -1, py, px);
        object_origins(&forge, ORIGIN_ACQUIRE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_create_food(void) { return cast_spell(create_food_spell); }

void create_major_trap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Create Major Trap");
        break;
    case SPELL_DESC:
        var_set_string(res, "Sets a trap under you. This trap will have various effects on a passing monster.");
        break;
    case SPELL_CAST:
        set_trap(py, px, feat_rogue_trap2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void create_minor_trap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Create Minor Trap");
        break;
    case SPELL_DESC:
        var_set_string(res, "Sets a weak trap under you. This trap will have various weak effects on a passing monster.");
        break;
    case SPELL_CAST:
        set_trap(py, px, feat_rogue_trap1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void create_ultimate_trap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Create Ultimate Trap");
        break;
    case SPELL_DESC:
        var_set_string(res, "Sets an extremely powerful trap under you. This trap will have various strong effects on a passing monster.");
        break;
    case SPELL_CAST:
        set_trap(py, px, feat_rogue_trap3);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void crusade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Crusade");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to charm all good monsters in sight, and scare all non-charmed monsters, and summons great number of knights, and gives heroism, bless, speed and protection from evil.");
        break;
    case SPELL_CAST:
    {
        int base = 25;
        int sp_sides = 20 + p_ptr->lev;
        int sp_base = p_ptr->lev;
        int i;

        project_hack(GF_CRUSADE, p_ptr->lev*4);
        for (i = 0; i < 12; i++)
        {
            int attempt = 10;
            int my = 0, mx = 0;

            while (attempt--)
            {
                scatter(&my, &mx, py, px, 4, 0);
                if (cave_empty_bold2(my, mx)) break;
            }
            if (attempt < 0) continue;
            summon_specific(-1, my, mx, p_ptr->lev, SUMMON_KNIGHT, (PM_ALLOW_GROUP | PM_FORCE_PET | PM_HASTE));
        }
        set_hero(randint1(base) + base, FALSE);
        set_blessed(randint1(base) + base, FALSE);
        set_fast(randint1(sp_sides) + sp_base, FALSE);
        set_protevil(randint1(base) + base, FALSE);
        fear_clear_p();

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_crusade(void) { return cast_spell(crusade_spell); }

void cure_poison_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cure Poison");
        break;
    case SPELL_DESC:
        var_set_string(res, "Cure poison status.");
        break;
    case SPELL_CAST:
        set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cure_wounds_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cure Light Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Heals cut and HP a little.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(2, spell_power(10), 0));
        break;
    case SPELL_CAST:
        hp_player(spell_power(damroll(2, 10)));
        set_cut(p_ptr->cut - 10, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cure_wounds_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cure Medium Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Heals cut and HP more.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(4, spell_power(10), 0));
        break;
    case SPELL_CAST:
        hp_player(spell_power(damroll(4, 10)));
        set_cut((p_ptr->cut / 2) - 20, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cure_wounds_III_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cure Critical Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Heals cut, stun and HP greatly.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(8, spell_power(10), 0));
        break;
    case SPELL_CAST:
        hp_player(spell_power(damroll(8, 10)));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void curing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Curing");
        break;
    case SPELL_DESC:
        var_set_string(res, "It heals you a bit and cures blindness, poison, confusion, stunning, cuts and hallucination.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, spell_power(50)));
        break;
    case SPELL_CAST:
        hp_player(spell_power(50));
        set_blind(0, TRUE);
        set_poisoned(p_ptr->poisoned - MAX(150, p_ptr->poisoned / 3), TRUE);
        set_confused(0, TRUE);
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        set_image(0, TRUE);
        set_shero(0,TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _darkness_storm_I_dam(void)
{
    if (p_ptr->pclass == CLASS_WILD_TALENT) /* Wild-Talents gain both I and II versions ... */
        return 100 + py_prorata_level_aux(100, 1, 1, 0);
    return 100 + py_prorata_level_aux(200, 1, 1, 2);
}

void darkness_storm_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Darkness Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of darkness.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_darkness_storm_I_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You invoke a darkness storm.");
        fire_ball(
            GF_DARK,
            dir,
            spell_power(_darkness_storm_I_dam() + p_ptr->to_d_spell),
            spell_power(4)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _darkness_storm_II_dam(void)
{
    return py_prorata_level_aux(450, 1, 0, 2);
}

void darkness_storm_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Darkness Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of darkness of unmatched power.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_darkness_storm_II_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You invoke a darkness storm.");
        fire_ball(GF_DARK, dir,
            spell_power(_darkness_storm_II_dam() + p_ptr->to_d_spell),
            spell_power(4));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void day_of_the_dove_spell(int cmd, variant *res)
{
    int power = spell_power(p_ptr->lev * 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Day of the Dove");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to charm all monsters in sight.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(power));
        break;
    case SPELL_CAST:
        charm_monsters(power);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dazzle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dazzle");
        break;
    case SPELL_DESC:
        var_set_string(res, "Emits dazzling lights, stunning, confusing and scaring nearby monsters.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the ability to emit dazzling lights.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the ability to emit dazzling lights.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can emit confusing, blinding radiation.");
        break;
    case SPELL_CAST:
        stun_monsters(5 + p_ptr->lev/5);
        confuse_monsters(p_ptr->lev * 4);
        turn_monsters(p_ptr->lev * 4);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_dazzle(void) { return cast_spell(dazzle_spell); }

void detect_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Life");
        break;
    case SPELL_DESC:
        var_set_string(res, "Locate nearby living monsters.");
        break;
    case SPELL_CAST:
        detect_monsters_living(DETECT_RAD_DEFAULT, "You sense the presence of life around you.");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void detect_unlife_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Unlife");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects all nonliving monsters in your vicinity.");
        break;
    case SPELL_CAST:
        detect_monsters_nonliving(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void demon_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Fire/Nether");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathe a powerful blast of either fire or nether at your opponent.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 3)));
        break;
    case SPELL_CAST:
    {
        int type = (one_in_(2) ? GF_NETHER : GF_FIRE);
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        stop_mouth();

        msg_format("You breathe %s.", (type == GF_NETHER) ? "nether" : "fire");

        fire_ball(type, dir, spell_power(p_ptr->lev * 3), -(p_ptr->lev / 15) - 1);
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev/3);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void destruction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Word of Destruction");
        break;
    case SPELL_DESC:
        var_set_string(res, "Destroys everything in your nearby vicinity ... except you, of course.");
        break;
    case SPELL_CAST:
        destroy_area(py, px, 12 + randint1(4), spell_power(4 * p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_destruction(void) { return cast_spell(destruction_spell); }

static void _detect_curses(obj_ptr obj)
{
    if (object_is_cursed(obj))
        obj->feeling = FEEL_CURSED;
}

void detect_curses_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Curses");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detected cursed items in your inventory.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You can feel evil magics.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You can no longer feel evil magics.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can feel the danger of evil magic.");
        break;
    case SPELL_CAST:
    {
        pack_for_each(_detect_curses);
        equip_for_each(_detect_curses);
        quiver_for_each(_detect_curses);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_curses(void) { return cast_spell(detect_curses_spell); }

void detect_doors_stairs_traps_spell(int cmd, variant *res)
{
    int rad = DETECT_RAD_DEFAULT;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Doors & Traps");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects doors, stairs, and traps in your vicinity.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_radius(rad));
        break;
    case SPELL_CAST:
        detect_traps(rad, TRUE);
        detect_doors(rad);
        detect_stairs(rad);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_doors_stairs_traps(void) { return cast_spell(detect_doors_stairs_traps_spell); }

void detect_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Evil");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby evil monsters.");
        break;
    case SPELL_CAST:
        detect_monsters_evil(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void detect_menace_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Ferocity");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby menacing monsters. Only intelligent monsters are detected.");
        break;
    case SPELL_CAST:
        detect_monsters_mind(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void detect_monsters_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Monsters");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby monsters.");
        break;
    case SPELL_CAST:
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_monsters(void) { return cast_spell(detect_monsters_spell); }

void detect_objects_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Objects");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby objects.");
        break;
    case SPELL_CAST:
        detect_objects_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_objects(void) { return cast_spell(detect_objects_spell); }

void detect_traps_spell(int cmd, variant *res)
{
    int rad = DETECT_RAD_DEFAULT;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Traps");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects traps in your vicinity.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_radius(rad));
        break;
    case SPELL_CAST:
        detect_traps(rad, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_traps(void) { return cast_spell(detect_traps_spell); }

void detect_treasure_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Treasure");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby treasure.");
        break;
    case SPELL_CAST:
        detect_treasure(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_treasure(void) { return cast_spell(detect_treasure_spell); }

void detection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detection");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.");
        break;
    case SPELL_CAST:
        detect_all(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dimension_door_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dimension Door");
        break;
    case SPELL_DESC:
        var_set_string(res, "Open a portal to another dimension and step to a nearby location with great precision.");
        break;
    case SPELL_CAST:
        var_set_bool(res, dimension_door(p_ptr->lev / 2 + 10));
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
            var_set_int(res, 30);
        else
            default_spell(cmd, res);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_dimension_door(void) { return cast_spell(dimension_door_spell); }

void disintegrate_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Disintegrate");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of disintegration.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev + 70 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dam = spell_power(p_ptr->lev + 70 + p_ptr->to_d_spell);
        int rad = 3 + p_ptr->lev / 40;
        int dir;

        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_DISINTEGRATE, dir, dam, rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_evil_spell(int cmd, variant *res)
{
    int sides = spell_power(p_ptr->lev * 4);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dispel Evil");
        break;
    case SPELL_DESC:
        var_set_string(res, "Damages all evil monsters in sight.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        dispel_evil(randint1(sides) + spell_power(p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_life_spell(int cmd, variant *res)
{
    int ds = spell_power(p_ptr->lev * 4);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dispel Life");
        break;
    case SPELL_DESC:
        var_set_string(res, "Damages all living monsters in sight.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        dispel_living(randint1(ds) + spell_power(p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dispel Magic");
        break;
    case SPELL_DESC:
        var_set_string(res, "Dispels one monster, negating invulnerability spheres and temporary speed effects.");
        break;
    case SPELL_CAST:
    {
        int m_idx;

        var_set_bool(res, FALSE);
        if (!target_set(TARGET_KILL)) return;
        m_idx = cave[target_row][target_col].m_idx;
        if (!m_idx) return;

        var_set_bool(res, TRUE);
        if (!player_has_los_bold(target_row, target_col)) return;
        if (!projectable(py, px, target_row, target_col)) return;
        dispel_monster_status(m_idx);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_undead_spell(int cmd, variant *res)
{
    int dice = 1;
    int sides = spell_power(p_ptr->lev * 5);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dispel Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Damages all undead monsters in sight.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dice, sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        if(project_hack(GF_DISP_UNDEAD, damroll(dice, sides) + spell_power(p_ptr->to_d_spell)))
            virtue_add(VIRTUE_UNLIFE, -2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dominate_living_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dominate a Living Thing");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CONTROL_LIVING, dir, p_ptr->lev, 0);
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev+3)/4);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dominate_living_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dominate Living Things");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        project_hack(GF_CONTROL_LIVING, p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev+20)/2);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void drain_mana_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Drain Mana");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to drain mana from chosen monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("%d+d%d", spell_power(p_ptr->lev), spell_power(p_ptr->lev*3)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_DRAIN_MANA, dir, spell_power(randint1(p_ptr->lev*3)+p_ptr->lev), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void earthquake_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Earthquake");
        break;
    case SPELL_DESC:
        var_set_string(res, "The walls will tremble and the ground will shake.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the ability to wreck the dungeon.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the ability to wreck the dungeon.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can bring down the dungeon around your ears.");
        break;
    case SPELL_CAST:
        earthquake(py, px, 10);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_earthquake(void) { return cast_spell(earthquake_spell); }

void eat_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Eat Magic");
        break;
    case SPELL_DESC:
        var_set_string(res, "Consumes magical devices to regain spell points.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your magic items look delicious.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your magic items no longer look delicious.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can consume magic energy for your own use.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (eat_magic(20 + p_ptr->lev * 8 / 5)) /* skillmasters can do this on CL1 ... */
            var_set_bool(res, TRUE);
        break;
    case SPELL_FAIL_MIN:
        var_set_int(res, 11);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_eat_magic(void) { return cast_spell(eat_magic_spell); }

void eat_rock_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Eat Rock");
        break;
    case SPELL_DESC:
        var_set_string(res, "Consumes nearby rock.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("The walls look delicious.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("The walls look unappetizing.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can consume solid rock.");
        break;
    case SPELL_CAST:
    {
        int x, y;
        cave_type *c_ptr;
        feature_type *f_ptr, *mimic_f_ptr;
        int dir = 0;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) break;
        y = py + ddy[dir];
        x = px + ddx[dir];
        c_ptr = &cave[y][x];
        f_ptr = &f_info[c_ptr->feat];
        mimic_f_ptr = &f_info[get_feat_mimic(c_ptr)];

        stop_mouth();

        if (!have_flag(mimic_f_ptr->flags, FF_HURT_ROCK))
        {
            msg_print("You cannot eat this feature.");
            break;
        }
        else if (have_flag(f_ptr->flags, FF_PERMANENT))
        {
            msg_format("Ouch!  This %s is harder than your teeth!",
                f_name + mimic_f_ptr->name);

            break;
        }
        else if (c_ptr->m_idx)
        {
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            msg_print("There's something in the way!");
            if (!m_ptr->ml || !is_pet(m_ptr)) py_attack(y, x, 0);
            break;
        }
        else if (have_flag(f_ptr->flags, FF_TREE))
        {
            msg_print("You don't like the woody taste!");
            break;
        }
        else if (have_flag(f_ptr->flags, FF_GLASS))
        {
            msg_print("You don't like the glassy taste!");
            break;
        }
        else if (have_flag(f_ptr->flags, FF_DOOR) || have_flag(f_ptr->flags, FF_CAN_DIG))
        {
            if (elemental_is_(ELEMENTAL_EARTH))
                set_food(MIN(p_ptr->food + 500, PY_FOOD_MAX - 1));
            else
                set_food(p_ptr->food + 3000);
        }
        else if (have_flag(f_ptr->flags, FF_MAY_HAVE_GOLD) || have_flag(f_ptr->flags, FF_HAS_GOLD))
        {
            if (elemental_is_(ELEMENTAL_EARTH))
                set_food(MIN(p_ptr->food + 1000, PY_FOOD_MAX - 1));
            else
                set_food(p_ptr->food + 5000);
        }
        else
        {
            if (elemental_is_(ELEMENTAL_EARTH))
                set_food(MIN(p_ptr->food + 2000, PY_FOOD_MAX - 1));
            else
            {
                msg_format("This %s is very filling!",
                    f_name + mimic_f_ptr->name);

                set_food(p_ptr->food + 10000);
            }
        }

        /* Destroy the wall */
        cave_alter_feat(y, x, FF_HURT_ROCK);

        /* Move the player */
        move_player_effect(y, x, MPE_DONT_PICKUP);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_eat_rock(void) { return cast_spell(eat_rock_spell); }

void evil_bless_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Evil Bless");
        break;
    default:
        bless_spell(cmd, res);
        break;
    }
}

void evocation_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 4 + p_ptr->to_d_spell);
    int power = spell_power(p_ptr->lev * 4);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Evocation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Dispels, scares and banishes all monsters in view.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
        dispel_monsters(dam);
        turn_monsters(power);
        banish_monsters(power);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void minor_enchantment_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Minor Enchantment");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to enchant a weapon, ammo or armor.");
        break;
    case SPELL_CAST:
        var_set_bool(res, craft_enchant(2 + p_ptr->lev/5, 1));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void enchantment_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Enchantment");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to enchant a weapon, ammo or armor.");
        break;
    case SPELL_CAST:
        var_set_bool(res, craft_enchant(15, 3));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_enchantment(void) { return cast_spell(enchantment_spell); }

void enslave_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Enslave Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to enslave an undead monster.");
        break;
    case SPELL_CAST:
    {
        int power, dir;
        if (p_ptr->pclass == CLASS_NECROMANCER)
            power = spell_power(p_ptr->lev*3);
        else
            power = spell_power(p_ptr->lev);

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        control_one_undead(dir, power);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void explosive_rune_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Explosive Rune");
        break;
    case SPELL_DESC:
        var_set_string(res, "Sets a rune which will explode on a passing monster.");
        break;
    case SPELL_CAST:
        msg_print("You carefully set an explosive rune...");
        explosive_rune();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void eye_for_an_eye_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "An Eye for an Eye");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives special aura for a while. When you are attacked by a monster, the monster are injured with same amount of damage as you take.");
        break;
    case SPELL_CAST:
        set_tim_eyeeye(spell_power(randint1(10) + 10), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fire_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generate a Fire Ball on chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 30 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(
            GF_FIRE,
            dir,
            spell_power(3*p_ptr->lev/2 + 30 + p_ptr->to_d_spell),
            2
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void fire_bolt_spell(int cmd, variant *res)
{
    int dd = 5 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt or beam of fire.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_FIRE,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void flow_of_lava_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "The Flow of Lava");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a ball of fire centered on you which transforms floors to magma.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(55 + p_ptr->lev + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        fire_ball(GF_FIRE, 0, spell_power(55 + p_ptr->lev + p_ptr->to_d_spell), 3);
        fire_ball_hide(GF_LAVA_FLOW, 0, 2 + randint1(2), 3);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void force_branding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Force Branding");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporarily brands your weapon with force.");
        break;
    case SPELL_CAST:
    {
        int base = spell_power(p_ptr->lev / 4);
        set_tim_force(base + randint1(base), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void frost_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Frost Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generate a Frost Ball on chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 25 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_COLD, dir, spell_power(3*p_ptr->lev/2 + 25 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void frost_bolt_spell(int cmd, variant *res)
{
    int dd = 4 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Frost Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt or beam of frost.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_COLD,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void genocide_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Genocide");
        break;
    case SPELL_DESC:
        var_set_string(res, "Eliminates an entire class of monster, exhausting you. Powerful or unique monsters may resist.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(spell_power(p_ptr->lev*3)));
        break;
    case SPELL_CAST:
    {
        int power = spell_power(p_ptr->lev*3);
        var_set_bool(res, symbol_genocide(power, TRUE));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void glyph_of_warding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Glyph of Warding");
        break;
    case SPELL_DESC:
        var_set_string(res, "Sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break the glyph.");
        break;
    case SPELL_CAST:
        warding_glyph();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void grow_mold_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Grow Mold");
        break;
    case SPELL_DESC:
        var_set_string(res, "Surrounds yourself with moldy things.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel a sudden affinity for mold.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel a sudden dislike for mold.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can cause mold to grow near you.");
        break;
    case SPELL_CAST:
    {
        int i;
        for (i = 0; i < 8; i++)
        {
            summon_specific(-1, py, px, p_ptr->lev, SUMMON_BIZARRE1, PM_FORCE_PET);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_grow_mold(void) { return cast_spell(grow_mold_spell); }

