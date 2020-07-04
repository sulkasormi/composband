/* File: do-spell.c */

/* Purpose: Do everything for each spell */

#include "angband.h"

/* Hack: Increase spell power! */
static int _current_realm_hack = 0;

int spell_power_aux(int pow, int bonus)
{
    return MAX(0, pow + pow*bonus/13);
}

int spell_power(int pow)
{
    int tmp = p_ptr->spell_power;
    if (p_ptr->tim_blood_rite)
        tmp += 7;
/*  if (_current_realm_hack && _current_realm_hack == p_ptr->easy_realm1)
        tmp += 2; */
    return spell_power_aux(pow, tmp);
}

int device_power_aux(int pow, int bonus)
{
    return MAX(0, pow + pow*bonus/20);
}

int device_power(int pow)
{
    return device_power_aux(pow, p_ptr->device_power);
}

int spell_cap_aux(int cap, int bonus)
{
    return MAX(0, cap + cap*bonus/20);
}

int spell_cap(int cap)
{
    return spell_cap_aux(cap, p_ptr->spell_cap);
}

/*
 * Generate dice info string such as "foo 2d10"
 */
static cptr info_string_dice(cptr str, int dice, int sides, int base)
{
    /* Fix value */
    if (!dice)
        return format("%s%d", str, base);

    /* Dice only */
    else if (!base)
        return format("%s%dd%d", str, dice, sides);

    /* Dice plus base value */
    else
        return format("%s%dd%d%+d", str, dice, sides, base);
}


/*
 * Generate damage-dice info string such as "dam 2d10"
 */
cptr info_damage(int dice, int sides, int base)
{
    return info_string_dice("dam ", dice, sides, base);
}


/*
 * Generate duration info string such as "dur 20+1d20"
 */
cptr info_duration(int base, int sides)
{
    return format("dur %d+1d%d", base, sides);
}


/*
 * Generate range info string such as "range 5"
 */
cptr info_range(int range)
{
    return format("range %d", range);
}


/*
 * Generate heal info string such as "heal 2d8"
 */
cptr info_heal(int dice, int sides, int base)
{
    if ( p_ptr->pclass == CLASS_BLOOD_MAGE
      || p_ptr->pclass == CLASS_BLOOD_KNIGHT )
    {
        sides /= 2;
        base /= 2;
    }

    if ( p_ptr->prace == RACE_EINHERI
      || p_ptr->mimic_form == RACE_EINHERI )
    {
        if (dice % 2) sides /= 2;
        else dice /= 2;
        base /= 2;
    }

    return info_string_dice("heal ", dice, sides, base);
}


/*
 * Generate delay info string such as "delay 15+1d15"
 */
cptr info_delay(int base, int sides)
{
    return format("delay %d+1d%d", base, sides);
}


/*
 * Generate multiple-damage info string such as "dam 25 each"
 */
static cptr info_multi_damage(int dam)
{
    return format("dam %d each", dam);
}


/*
 * Generate multiple-damage-dice info string such as "dam 5d2 each"
 */
static cptr info_multi_damage_dice(int dice, int sides)
{
    return format("dam %dd%d each", dice, sides);
}


/*
 * Generate power info string such as "power 100"
 */
cptr info_power(int power)
{
    return format("power %d", power);
}


/*
 * Generate power info string such as "power 1d100"
 */
static cptr info_power_dice(int dice, int sides)
{
    return format("power %dd%d", dice, sides);
}


/*
 * Generate radius info string such as "rad 100"
 */
cptr info_radius(int rad)
{
    return format("rad %d", rad);
}


/*
 * Generate weight info string such as "max wgt 15"
 */
cptr info_weight(int weight)
{
    return format("max wgt %d", weight/10);
}

/*
 * Generate distance info string such as "dist 100"
 */

cptr info_dist(int dist)
{
    return format("dist %d", dist);
}


/*
 * Prepare standard probability to become beam for fire_bolt_or_beam()
 */
int beam_chance(void)
{
    if (p_ptr->pclass == CLASS_MAGE || p_ptr->pclass == CLASS_BLOOD_MAGE || p_ptr->pclass == CLASS_NECROMANCER || p_ptr->pclass == CLASS_YELLOW_MAGE || p_ptr->pclass == CLASS_GRAY_MAGE)
        return p_ptr->lev;
    if (p_ptr->pclass == CLASS_HIGH_MAGE || p_ptr->pclass == CLASS_SORCERER)
        return p_ptr->lev + 10;

    return p_ptr->lev / 2;
}


/*
 * Handle summoning and failure of trump spells
 */
bool trump_summoning(int num, bool pet, int y, int x, int lev, int type, u32b mode)
{
    int plev = p_ptr->lev;

    int who = SUMMON_WHO_PLAYER;
    int i;
    bool success = FALSE;

    /* Default level */
    if (!lev) lev = spell_power(plev) + randint1(spell_power(plev * 2 / 3));

    if (pet)
    {
        /* Become pet */
        mode |= PM_FORCE_PET;

        /* Only sometimes allow unique monster */
        if (mode & PM_ALLOW_UNIQUE)
        {
            /* Forbid often */
            if (randint1(50 + plev) >= plev / 10)
                mode &= ~PM_ALLOW_UNIQUE;
        }
    }
    else
    {
        /* Prevent taming, allow unique monster */
        mode |= PM_NO_PET;
    }

    for (i = 0; i < num; i++)
    {
        if (summon_specific(who, y, x, lev, type, mode))
            success = TRUE;
    }

    if (!success)
    {
        if (p_ptr->pclass == CLASS_NECROMANCER)
            msg_print("Nobody answers to your foul summons.");
        else
            msg_print("Nobody answers to your Trump call.");
    }

    return success;
}


/*
 * This spell should become more useful (more controlled) as the
 * player gains experience levels. Thus, add 1/5 of the player's
 * level to the die roll. This eliminates the worst effects later on,
 * while keeping the results quite random. It also allows some potent
 * effects only at high level.
 */
void cast_wonder(int dir)
{
    int plev = p_ptr->lev;
    int die = randint1(100) + plev / 5;
    int vir = virtue_current(VIRTUE_CHANCE);

    if (vir > 0)
    {
        while (randint1(400) < vir) die++;
    }
    else if (vir < 0)
    {
        while (randint1(400) < -vir) die--;
    }

    if (p_ptr->pclass == CLASS_WILD_TALENT)
        die += randint1(25 + p_ptr->lev/2);

    if (die < 26)
        virtue_add(VIRTUE_CHANCE, 1);

    if (die > 100)
    {
        msg_print("You feel a surge of power!");
    }

    if (die < 8) clone_monster(dir);
    else if (die < 14) speed_monster(dir);
    else if (die < 26) heal_monster(dir, damroll(4, 6));
    else if (die < 31) poly_monster(dir);
    else if (die < 36)
        fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir,
                  damroll(3 + ((plev - 1) / 5) + p_ptr->to_d_spell, 4));
    else if (die < 41) confuse_monster(dir, plev);
    else if (die < 46) fire_ball(GF_POIS, dir, 20 + (plev / 2) + p_ptr->to_d_spell, 3);
    else if (die < 51) (void)lite_line(dir);
    else if (die < 56)
        fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir,
                  damroll(3 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 61)
        fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir,
                  damroll(5 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 66)
        fire_bolt_or_beam(beam_chance(), GF_ACID, dir,
                  damroll(6 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 71)
        fire_bolt_or_beam(beam_chance(), GF_FIRE, dir,
                  damroll(8 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 76) drain_life(dir, 75 + p_ptr->to_d_spell);
    else if (die < 81) fire_ball(GF_ELEC, dir, 30 + plev / 2 + p_ptr->to_d_spell, 2);
    else if (die < 86) fire_ball(GF_ACID, dir, 40 + plev + p_ptr->to_d_spell, 2);
    else if (die < 91) fire_ball(GF_ICE, dir, 70 + plev + p_ptr->to_d_spell, 3);
    else if (die < 96) fire_ball(GF_FIRE, dir, 80 + plev + p_ptr->to_d_spell, 3);
    else if (die < 101) drain_life(dir, 100 + plev + p_ptr->to_d_spell);
    else if (die < 104)
    {
        earthquake(py, px, 12);
    }
    else if (die < 106)
    {
        (void)destroy_area(py, px, 13 + randint0(5), 2 * p_ptr->lev);
    }
    else if (die < 108)
    {
        symbol_genocide(plev+50, TRUE);
    }
    else if (die < 110) dispel_monsters(120);
    else /* RARE */
    {
        dispel_monsters(150 + p_ptr->to_d_spell);
        slow_monsters(p_ptr->lev);
        sleep_monsters(p_ptr->lev);
        hp_player(300);
    }
    if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_CHANCE);
}


static void cast_invoke_spirits(int dir)
{
    int plev = p_ptr->lev;
    int die = spell_power(randint1(100) + plev / 5);
    int vir = virtue_current(VIRTUE_CHANCE);

    if (vir > 0)
    {
        while (randint1(400) < vir) die++;
    }
    else if (vir < 0)
    {
        while (randint1(400) < -vir) die--;
    }

    msg_print("You call on the power of the dead...");

    if (die < 26)
        virtue_add(VIRTUE_CHANCE, 1);

    if (die > 100)
    {
        msg_print("You feel a surge of eldritch force!");
    }


    if (die < 8)
    {
        msg_print("Oh no! Mouldering forms rise from the earth around you!");

        (void)summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_UNDEAD, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
        virtue_add(VIRTUE_UNLIFE, 1);
    }
    else if (die < 14)
    {
        msg_print("An unnamable evil brushes against your mind...");

        fear_add_p(FEAR_TERRIFIED);
    }
    else if (die < 26)
    {
        msg_print("Your head is invaded by a horde of gibbering spectral voices...");

        set_confused(p_ptr->confused + randint1(4) + 4, FALSE);
    }
    else if (die < 31)
    {
        poly_monster(dir);
    }
    else if (die < 36)
    {
        fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir,
                  damroll(3 + ((plev - 1) / 5) + p_ptr->to_d_spell, 4));
    }
    else if (die < 41)
    {
        confuse_monster (dir, plev);
    }
    else if (die < 46)
    {
        fire_ball(GF_POIS, dir, 20 + (plev / 2) + p_ptr->to_d_spell, 3);
    }
    else if (die < 51)
    {
        (void)lite_line(dir);
    }
    else if (die < 56)
    {
        fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir,
                  damroll(3+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 61)
    {
        fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir,
                  damroll(5+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 66)
    {
        fire_bolt_or_beam(beam_chance(), GF_ACID, dir,
                  damroll(6+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 71)
    {
        fire_bolt_or_beam(beam_chance(), GF_FIRE, dir,
                  damroll(8+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 76)
    {
        drain_life(dir, 75 + p_ptr->to_d_spell);
    }
    else if (die < 81)
    {
        fire_ball(GF_ELEC, dir, 30 + plev / 2 + p_ptr->to_d_spell, 2);
    }
    else if (die < 86)
    {
        fire_ball(GF_ACID, dir, 40 + plev + p_ptr->to_d_spell, 2);
    }
    else if (die < 91)
    {
        fire_ball(GF_ICE, dir, 70 + plev + p_ptr->to_d_spell, 3);
    }
    else if (die < 96)
    {
        fire_ball(GF_FIRE, dir, 80 + plev + p_ptr->to_d_spell, 3);
    }
    else if (die < 101)
    {
        drain_life(dir, 100 + plev + p_ptr->to_d_spell);
    }
    else if (die < 104)
    {
        earthquake(py, px, 12);
    }
    else if (die < 106)
    {
        (void)destroy_area(py, px, 13 + randint0(5), 2 * p_ptr->lev);
    }
    else if (die < 108)
    {
        symbol_genocide(plev+50, TRUE);
    }
    else if (die < 110)
    {
        dispel_monsters(120 + p_ptr->to_d_spell);
    }
    else
    { /* RARE */
        dispel_monsters(150 + p_ptr->to_d_spell);
        slow_monsters(p_ptr->lev);
        sleep_monsters(p_ptr->lev);
        hp_player(300);
    }

    if (die < 31)
    {
        msg_print("Sepulchral voices chuckle. 'Soon you will join us, mortal.'");
    }
}

void do_sneeze(void)
{
    int kiep_ct = randint1(3), y = 0, x = 0, yrk = 1000, i;
    while (yrk--)
    {
        scatter(&y, &x, py, px, 4, 0);
        if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
        if (!player_bold(y, x)) break;
    }

    for (i = 0; i < kiep_ct; i++)
    {
        disturb(0, 0);
        msg_print("AAAAHHHHH-CHEEOO!");
        msg_print(NULL);

        project(0, 0, y, x, p_ptr->lev / 2, GF_COLD,
            PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_KILL);
    }
}

static void wild_magic(int spell)
{
    int counter = 0;
    int type = SUMMON_BIZARRE1 + randint0(6);

    if (type < SUMMON_BIZARRE1) type = SUMMON_BIZARRE1;
    else if (type > SUMMON_BIZARRE6) type = SUMMON_BIZARRE6;

    switch (randint1(spell) + randint1(8) + 1)
    {
    case 1:
    case 2:
    case 3:
        teleport_player(10, TELEPORT_PASSIVE);
        break;
    case 4:
    case 5:
    case 6:
        teleport_player(100, TELEPORT_PASSIVE);
        break;
    case 7:
    case 8:
        teleport_player(200, TELEPORT_PASSIVE);
        break;
    case 9:
    case 10:
    case 11:
        unlite_area(10, 3);
        break;
    case 12:
    case 13:
    case 14:
        lite_area(damroll(2, 3), 2);
        break;
    case 15:
        destroy_doors_touch();
        break;
    case 16: case 17:
        wall_breaker();
    case 18:
        sleep_monsters_touch();
        break;
    case 19:
    case 20:
        trap_creation(py, px);
        break;
    case 21:
    case 22:
        door_creation();
        break;
    case 23:
    case 24:
    case 25:
        aggravate_monsters(0);
        break;
    case 26:
        earthquake(py, px, 5);
        break;
    case 27:
    case 28:
        mut_gain_random(NULL);
        break;
    case 29:
    case 30:
        apply_disenchant(1);
        break;
    case 31:
        lose_all_info();
        break;
    case 32:
        fire_ball(GF_CHAOS, 0, spell + 5, 1 + (spell / 10));
        break;
    case 33:
        wall_stone();
        break;
    case 34:
    case 35:
        while (counter++ < 8)
        {
            (void)summon_specific(0, py, px, (dun_level * 3) / 2, type, (PM_ALLOW_GROUP | PM_NO_PET));
        }
        break;
    case 36:
    case 37:
        activate_hi_summon(py, px, FALSE);
        break;
    case 38:
        (void)summon_cyber(-1, py, px);
        break;
    default:
        {
            int count = 0;
            (void)activate_ty_curse(FALSE, &count);
            break;
        }
    }
}


void cast_shuffle(void)
{
    int plev = p_ptr->lev;
    int dir;
    int die;
    int vir = virtue_current(VIRTUE_CHANCE);
    int i;

    /* Card sharks and high mages get a level bonus */
    if ((p_ptr->pclass == CLASS_ROGUE) ||
        (p_ptr->pclass == CLASS_HIGH_MAGE) ||
        (p_ptr->pclass == CLASS_SORCERER) ||
        (p_ptr->pclass == CLASS_DISCIPLE))
        die = (randint1(110)) + plev / 5;
    else
        die = randint1(120);


    if (vir > 0)
    {
        while (randint1(400) < vir) die++;
    }
    else if (vir < 0)
    {
        while (randint1(400) < -vir) die--;
    }

    msg_print("You shuffle the deck and draw a card...");

    if (die < 30)
        virtue_add(VIRTUE_CHANCE, 1);

    if (die < 7)
    {
        msg_print("Oh no! It's Death!");

        for (i = 0; i < randint1(3); i++)
            activate_hi_summon(py, px, FALSE);
    }
    else if (die < 14)
    {
        msg_print("Oh no! It's the Devil!");

        summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_DEMON, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
    }
    else if (die < 18)
    {
        int count = 0;
        msg_print("Oh no! It's the Hanged Man.");

        activate_ty_curse(FALSE, &count);
    }
    else if (die < 22)
    {
        msg_print("It's the swords of discord.");

        aggravate_monsters(0);
    }
    else if (die < 26)
    {
        msg_print("It's the Fool.");

        do_dec_stat(A_INT);
        do_dec_stat(A_WIS);
    }
    else if (die < 30)
    {
        msg_print("It's the picture of a strange monster.");

        trump_summoning(1, FALSE, py, px, (dun_level * 3 / 2), (SUMMON_BIZARRE1 + randint0(6)), PM_ALLOW_GROUP | PM_ALLOW_UNIQUE);
    }
    else if (die < 33)
    {
        msg_print("It's the Moon.");

        unlite_area(10, 3);
    }
    else if (die < 38)
    {
        msg_print("It's the Wheel of Fortune.");

        wild_magic(randint0(32));
    }
    else if (die < 40)
    {
        msg_print("It's a teleport trump card.");

        teleport_player(10, TELEPORT_PASSIVE);
    }
    else if (die < 42)
    {
        msg_print("It's Justice.");

        set_blessed(p_ptr->lev, FALSE);
    }
    else if (die < 47)
    {
        msg_print("It's a teleport trump card.");

        teleport_player(100, TELEPORT_PASSIVE);
    }
    else if (die < 52)
    {
        msg_print("It's a teleport trump card.");

        teleport_player(200, TELEPORT_PASSIVE);
    }
    else if (die < 60)
    {
        msg_print("It's the Tower.");

        wall_breaker();
    }
    else if (die < 72)
    {
        msg_print("It's Temperance.");

        sleep_monsters_touch();
    }
    else if (die < 80)
    {
        msg_print("It's the Tower.");

        earthquake(py, px, 5);
    }
    else if (die < 82)
    {
        msg_print("It's the picture of a friendly monster.");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE1, 0L);
    }
    else if (die < 84)
    {
        msg_print("It's the picture of a friendly monster.");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE2, 0L);
    }
    else if (die < 86)
    {
        msg_print("It's the picture of a friendly monster.");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE4, 0L);
    }
    else if (die < 88)
    {
        msg_print("It's the picture of a friendly monster.");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE5, 0L);
    }
    else if (die < 96)
    {
        msg_print("It's the Lovers.");

        if (get_fire_dir(&dir))
            charm_monster(dir, MIN(p_ptr->lev, 20));
    }
    else if (die < 101)
    {
        msg_print("It's the Hermit.");

        wall_stone();
    }
    else if (die < 111)
    {
        msg_print("It's the Judgement.");

        do_cmd_rerate(FALSE);
        mut_lose_all();
    }
    else if (die < 120)
    {
        msg_print("It's the Sun.");

        virtue_add(VIRTUE_KNOWLEDGE, 1);
        virtue_add(VIRTUE_ENLIGHTENMENT, 1);
        wiz_lite(p_ptr->tim_superstealth > 0);
    }
    else
    {
        msg_print("It's the World.");

        if (p_ptr->exp < PY_MAX_EXP)
        {
            s32b ee = (p_ptr->exp / 25) + 1;
            if (ee > 5000) ee = 5000;
            msg_print("You feel more experienced.");

            gain_exp(ee);
        }
    }
}


/*
 * Drop 10+1d10 meteor ball at random places near the player
 */
static void cast_meteor(int dam, int rad)
{
    int i;
    int b = 10 + randint1(10);

    for (i = 0; i < b; i++)
    {
        int y = 0, x = 0;
        int count;

        for (count = 0; count <= 20; count++)
        {
            int dy, dx, d;

            x = px - 8 + randint0(17);
            y = py - 8 + randint0(17);

            dx = (px > x) ? (px - x) : (x - px);
            dy = (py > y) ? (py - y) : (y - py);

            /* Approximate distance */
            d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));

            if (d >= 9) continue;

            if (!in_bounds(y, x) || !projectable(py, px, y, x)
                || !cave_have_flag_bold(y, x, FF_PROJECT)) continue;

            /* Valid position */
            break;
        }

        if (count > 20) continue;

        project(0, rad, y, x, dam, GF_METEOR, PROJECT_KILL | PROJECT_JUMP | PROJECT_ITEM);
    }
}


/*
 * Drop 10+1d10 disintegration ball at random places near the target
 */
bool cast_wrath_of_the_god(int dam, int rad)
{
    int x, y, tx, ty;
    int nx, ny;
    int dir, i;
    int b = 10 + randint1(10);

    if (!get_fire_dir(&dir)) return FALSE;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Hack -- Use an actual "target" */
    if ((dir == 5) && target_okay())
    {
        tx = target_col;
        ty = target_row;
    }

    x = px;
    y = py;

    while (1)
    {
        /* Hack -- Stop at the target */
        if ((y == ty) && (x == tx)) break;

        ny = y;
        nx = x;
        mmove2(&ny, &nx, py, px, ty, tx);

        /* Stop at maximum range */
        if (MAX_RANGE <= distance(py, px, ny, nx)) break;

        /* Stopped by walls/doors */
        if (!cave_have_flag_bold(ny, nx, FF_PROJECT)) break;

        /* Stopped by monsters */
        if ((dir != 5) && cave[ny][nx].m_idx != 0) break;

        /* Save the new location */
        x = nx;
        y = ny;
    }
    tx = x;
    ty = y;

    for (i = 0; i < b; i++)
    {
        int count = 20, d = 0;

        while (count--)
        {
            int dx, dy;

            x = tx - 5 + randint0(11);
            y = ty - 5 + randint0(11);

            dx = (tx > x) ? (tx - x) : (x - tx);
            dy = (ty > y) ? (ty - y) : (y - ty);

            /* Approximate distance */
            d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));
            /* Within the radius */
            if (d < 5) break;
        }

        if (count < 0) continue;

        /* Cannot penetrate perm walls */
        if (!in_bounds(y,x) ||
            cave_stop_disintegration(y,x) ||
            !in_disintegration_range(ty, tx, y, x))
            continue;

        project(0, rad, y, x, dam, GF_DISINTEGRATE, PROJECT_JUMP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
    }

    return TRUE;
}


/*
 * An "item_tester_hook" for offer
 */
static bool item_tester_offer(object_type *o_ptr)
{
    /* Flasks of oil are okay */
    if (o_ptr->tval != TV_CORPSE) return (FALSE);

    if (o_ptr->sval != SV_CORPSE) return (FALSE);

    if (my_strchr("pht", r_info[o_ptr->pval].d_char)) return (TRUE);

    /* Assume not okay */
    return (FALSE);
}


/*
 * Daemon spell Summon Greater Demon
 */
bool cast_summon_greater_demon(void)
{
    obj_prompt_t prompt = {0};
    int plev = p_ptr->lev;
    int summon_lev;

    prompt.prompt = "Sacrifice which corpse?";
    prompt.error = "You have nothing to sacrifice.";
    prompt.filter = item_tester_offer;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;

    summon_lev = plev * 2 / 3 + r_info[prompt.obj->pval].level;

    if (summon_specific(SUMMON_WHO_PLAYER, py, px, summon_lev, SUMMON_HI_DEMON, (PM_ALLOW_GROUP | PM_FORCE_PET)))
    {
        msg_print("The area fills with a stench of sulphur and brimstone.");
        msg_print("'What is thy bidding... Master?'");

        prompt.obj->number--;
        obj_release(prompt.obj, 0);
    }
    else
    {
        msg_print("No Greater Demons arrive.");
    }

    return TRUE;
}



static cptr do_life_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool spoil = (mode == SPELL_SPOIL_DESC) ? TRUE : FALSE;

    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Cure Light Wounds";
        if (desc) return "Heals cut and HP a little.";

        {
            int dice = 2;
            int sides = 10;

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(spell_power(damroll(dice, sides)));
                set_cut(p_ptr->cut - 10, TRUE);
            }
        }
        break;

    case 1:
        if (name) return "Bless";
        if (desc) return "Gives bonus to hit and AC for a few turns.";

        {
            int base = spell_power(12);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_blessed(randint1(base) + base, FALSE);
            }
        }
        break;

    case 2:
        if (name) return "Regeneration";
        if (desc) return "Gives regeneration ability for a while.";

        {
            int base = spell_power(80);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_regen(base + randint1(base), FALSE);
            }
        }
        break;

    case 3:
        if (name) return "Call Light";
        if (desc) return "Lights up nearby area and the inside of a room permanently.";

        {
            int dice = 2;
            int sides = plev / 2;
            int rad = spell_power(plev / 10 + 1);

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(spell_power(damroll(dice, sides)), rad);
            }
        }
        break;

    case 4:
        if (name) return "Detect Doors & Traps";
        if (desc) return "Detects traps, doors, and stairs in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 5:
        if (name) return "Cure Medium Wounds";
        if (desc) return "Heals cut and HP more.";

        {
            int dice = 6;
            int sides = 10;

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(spell_power(damroll(dice, sides)));
                set_cut((p_ptr->cut / 2) - 20, TRUE);
            }
        }
        break;
    case 6:
        if (name) return "Cure Poison";
        if (desc) return "Relieves poisoning a bit. Completely cures low-level poisoning.";
        if (cast)
            set_poisoned(p_ptr->poisoned - MAX(125, p_ptr->poisoned / 3), TRUE);
        break;
    case 7:
        if (name) return "Satisfy Hunger";
        if (desc) return "Satisfies hunger.";
        if (cast)
            set_food(PY_FOOD_MAX - 1);
        break;
    case 8:
        if (name) return "Remove Curse";
        if (desc) return "Removes normal curses from equipped items.";

        {
            if (cast)
            {
                if (remove_curse())
                {
                    msg_print("You feel as if someone is watching over you.");
                }
            }
        }
        break;

    case 9:
        if (name) return "Fasting";
        if (desc) return "Begin a religious fast. In time, your god may restore you!";
        if (spoil) return "Player begins a fast. Once hungry there is a small chance that the player will have a random stat restored, or will have their life restored.";

        if (cast)
        {
            if (p_ptr->fasting)
            {
                msg_print("You are already fasting. Perhaps you should pray as well?");
                return NULL;
            }
            msg_print("You begin to fast.");
            set_food(p_ptr->food/2);
            p_ptr->redraw |= PR_STATUS;
            p_ptr->fasting = TRUE;
        }
        break;

    case 10:
        if (name) return "Cure Critical Wounds";
        if (desc) return "Heals cut, stun and HP greatly.";

        {
            int dice = 12;
            int sides = 12;

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(spell_power(damroll(dice, sides)));
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }
        break;

    case 11:
        if (name) return "Resist Heat and Cold";
        if (desc) return "Gives temporary resistance to fire and cold.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_cold(dur, FALSE);
                set_oppose_fire(dur, FALSE);
            }
        }
        break;

    case 12:
        if (name) return "Sense Surroundings";
        if (desc) return "Maps nearby area.";

        {
            int rad = DETECT_RAD_MAP;

            if (info) return info_radius(rad);

            if (cast)
            {
                map_area(rad);
            }
        }
        break;

    case 13:
        if (name) return "Turn Undead";
        if (desc) return "Attempts to scare undead monsters in sight.";

        {
            if (cast)
            {
                turn_undead();
            }
        }
        break;

    case 14:
        if (name) return "Healing";
        if (desc) return "Much powerful healing magic, and heals cut and stun completely.";

        {
            int heal = spell_power(300);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }
        break;

    case 15:
        if (name) return "Glyph of Warding";
        if (desc) return "Sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break the glyph.";

        {
            if (cast)
            {
                warding_glyph();
            }
        }
        break;

    case 16:
        if (name) return "Dispel Curse";
        if (desc) return "Removes normal and heavy curse from equipped items.";

        {
            if (cast)
            {
                if (remove_all_curse())
                {
                    msg_print("You feel as if someone is watching over you.");
                }
            }
        }
        break;

    case 17:
        if (name) return "Perception";
        if (desc) return "Identifies an item.";

        {
            if (cast)
            {
                if (!ident_spell(NULL)) return NULL;
                /*identify_pack();*/
            }
        }
        break;

    case 18:
        if (name) return "Dispel Undead";
        if (desc) return "Damages all undead monsters in sight.";

        {
            int dam = spell_power(plev * 3 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
                dispel_undead(dam);
        }
        break;

    case 19:
        if (name) return "Sustaining";
        if (desc) return "Grants temporary stat sustains, depending on your level.";
        if (spoil) return "Player gains up to L/7 stat sustains for L turns.";

        {
            int dur = spell_power(plev);

            if (info) return info_duration(dur, 0);

            if (cast)
            {
                int num = plev / 7;

                if (randint0(7) < num)
                {
                    set_tim_hold_life(dur, FALSE);
                    num--;
                }
                if (randint0(6) < num)
                {
                    set_tim_sustain_con(dur, FALSE);
                    num--;
                }
                if (randint0(5) < num)
                {
                    set_tim_sustain_str(dur, FALSE);
                    num--;
                }
                if (randint0(4) < num)
                {
                    set_tim_sustain_int(dur, FALSE);
                    num--;
                }
                if (randint0(3) < num)
                {
                    set_tim_sustain_dex(dur, FALSE);
                    num--;
                }
                if (randint0(2) < num)
                {
                    set_tim_sustain_wis(dur, FALSE);
                    num--;
                }
                if (num)
                {
                    set_tim_sustain_chr(dur, FALSE);
                    num--;
                }

            }
        }
        break;

    case 20:
        if (name) return "Cure Mutation";
        if (desc) return "Remove a random mutation.";
        if (spoil) return "Remove a random mutation. There is a 1 in 100/L chance of removing a bad mutation only.";

        if (cast)
        {
            if (one_in_(100/plev))
                mut_lose_random(mut_bad_pred);
            else
                mut_lose_random(NULL);
        }
        break;

    case 21:
        if (name) return "Word of Recall";
        if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 22:
        if (name) return "Transcendence";
        if (desc) return "For a short while, any damage you receive will be absorbed by your spell points.";

        {
            int dur = spell_power(plev/10);

            if (info) return format("dur %d", dur);

            if (cast)
                set_tim_transcendence(dur, FALSE);
        }
        break;

    case 23:
        if (name) return "Warding True";
        if (desc) return "Creates glyphs in all adjacent squares and under you.";

        {
            int rad = 1;

            if (info) return info_radius(rad);

            if (cast)
            {
                warding_glyph();
                glyph_creation();
            }
        }
        break;

    case 24:
        if (name) return "Sterilization";
        if (desc) return "Prevents any breeders on current level from breeding.";

        {
            if (cast)
            {
                num_repro += MAX_REPRO;
            }
        }
        break;

    case 25:
        if (name) return "Detection";
        if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 26:
        if (name) return "Annihilate Undead";
        if (desc) return "Eliminates all nearby undead monsters, exhausting you. Powerful or unique monsters may be able to resist.";

        {
            int power = spell_power(plev + 50);

            if (info) return info_power(power);

            if (cast)
            {
                mass_genocide_undead(power, TRUE);
            }
        }
        break;

    case 27:
        if (name) return "Clairvoyance";
        if (desc) return "Maps and lights whole dungeon level. Knows all objects location. And gives telepathy for a while.";

        {
            if (cast)
            {
                wiz_lite(p_ptr->tim_superstealth > 0);
            }
        }
        break;

    case 28:
        if (name) return "Restoration";
        if (desc) return "Restores all stats, life and experience.";

        {
            if (cast)
            {
                do_res_stat(A_STR);
                do_res_stat(A_INT);
                do_res_stat(A_WIS);
                do_res_stat(A_DEX);
                do_res_stat(A_CON);
                do_res_stat(A_CHR);
                restore_level();
                lp_player(1000);
            }
        }
        break;

    case 29:
        if (name) return "Healing True";
        if (desc) return "The greatest healing magic. Heals all HP, cut and stun.";
        if (spoil) return "Removes cuts and stuns, and heals the player 2000hp.";

        {
            int heal = spell_power(2000);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }
        break;

    case 30:
        if (name) return "Holy Vision";
        if (desc) return "Fully identifies an item.";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 31:
        if (name) return "Ultimate Resistance";
        if (desc) return "Gives ultimate resistance, bonus to AC and speed.";
        if (spoil) return "Player gains all resistances, auras, sustains, FA, SI, slow digestion, regeneration, levitation and reflection as well as double base resistance, haste, and +100AC for X+dX rounds where X=L/2.";

        {
            int base = spell_power(plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int v = randint1(base) + base;
                set_fast(v, FALSE);
                set_oppose_base(v, FALSE);
                set_ultimate_res(v, FALSE);
            }
        }
        break;
    }

    return "";
}


static cptr do_sorcery_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Detect Monsters";
        if (desc) return "Detects all monsters in your vicinity unless invisible.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_normal(rad);
            }
        }
        break;

    case 1:
        if (name) return "Phase Door";
        if (desc) return "Teleport short distance.";

        {
            int range = 10;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 2:
        if (name) return "Detect Doors and Traps";
        if (desc) return "Detects traps, doors, and stairs in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 3:
        if (name) return "Light Area";
        if (desc) return "Lights up nearby area and the inside of a room permanently.";

        {
            int dice = 2;
            int sides = plev / 2;
            int rad = plev / 10 + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(spell_power(damroll(dice, sides)), rad);
            }
        }
        break;

    case 4:
        if (name) return "Confuse Monster";
        if (desc) return "Attempts to confuse a monster.";

        {
            int power = spell_power((plev * 3) / 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                confuse_monster(dir, power);
            }
        }
        break;

    case 5:
        if (name) return "Teleport";
        if (desc) return "Teleport long distance.";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 6:
        if (name) return "Sleep Monster";
        if (desc) return "Attempts to sleep a monster.";

        {
            int power = spell_power(plev * 3 /2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                sleep_monster(dir, power);
            }
        }
        break;

    case 7:
        if (name) return "Recharging";
        if (desc)
        {
            if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                return "It attempts to recharge a device using your blood for power.";
            else
                return "It attempts to recharge a device using your mana for power.";
        }

        {
            int power = spell_power(plev * 3);

            if (info) return info_power(power);

            if (cast)
            {
                if (!recharge_from_player(power)) return NULL;
            }
        }
        break;

    case 8:
        if (name) return "Magic Mapping";
        if (desc) return "Maps nearby area.";

        {
            int rad = DETECT_RAD_MAP;

            if (info) return info_radius(rad);

            if (cast)
            {
                map_area(rad);
            }
        }
        break;

    case 9:
        if (name) return plev < 30 ? "Identify" : "Mass Identify";
        if (desc) return plev < 30 ? "Identifies an item." : "Identifies all items in your pack";

        {
            if (cast)
            {
                if (plev < 30) 
                {
                    if (!ident_spell(NULL))
                        return NULL;
                }
                else
                    mass_identify(FALSE);
            }
        }
        break;

    case 10:
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

    case 11:
        if (plev < 35)
        {
            if (name) return "Mass Sleep";
            if (desc) return "Attempts to sleep all monsters in sight.";
        }
        else
        {
            if (name) return "Mass Stasis";
            if (desc) return "Attempts to suspend all monsters in sight.";
        }

        {
            int power = spell_power(plev * ((plev < 35) ? 10 : 7) / 3);

            if (info) return info_power(power);

            if (cast)
            {
                if (plev < 35)
                    sleep_monsters(power);
                else
                    stasis_monsters(power);
            }
        }
        break;

    case 12:
        if (name) return "Teleport Away";
        if (desc) return "Teleports all monsters on the line away unless resisted.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 13:
        if (name) return "Haste Self";
        if (desc) return "Hastes you for a while.";

        {
            int base = spell_power(plev);
            int sides = spell_power(20 + plev);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_fast(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 14:
        if (name) return "Detection True";
        if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 15:
        if (name) return "Identify True";
        if (desc) return "*Identifies* an item.";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 16:
        if (name) return "Inventory Protection";
        if (desc) return "For a short while, items in your pack have a chance to resist destruction.";

        {
            int base = spell_power(30);

            if (info) return info_duration(30, base);

            if (cast)
                set_tim_inven_prot(base + randint1(base), FALSE);
        }
        break;

    case 17:
        if (name) return "Stair Creation";
        if (desc) return "Creates a stair which goes down or up.";

        if (cast)
            stair_creation(FALSE);
        break;

    case 18:
        if (name) return "Sense Minds";
        if (desc) return "Gives telepathy for a while.";

        {
            int base = 25;
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "Teleport to Town";
        if (desc) return "Teleport to a town which you choose in a moment. Can only be used outdoors.";

        {
            if (cast)
            {
                if (!tele_town()) return NULL;
            }
        }
        break;

    case 20:
        if (name) return "Self Knowledge";
        if (desc) return "Gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.";

        {
            if (cast)
            {
                self_knowledge();
            }
        }
        break;

    case 21:
        if (name) return "Teleport Level";
        if (desc) return "Teleport to up or down stairs in a moment.";

        {
            if (cast)
            {
                if (!py_teleport_level("Are you sure? (Teleport Level) ")) return NULL;
            }
        }
        break;

    case 22:
        if (name) return "Word of Recall";
        if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 23:
        if (name) return "Dimension Door";
        if (desc) return "Teleport to given location.";

        {
            int range = spell_power(plev / 2 + 10);

            if (info) return info_range(range);

            if (cast)
            {
                msg_print("You open a dimensional gate. Choose a destination.");

                if (!dimension_door(range)) return NULL;
            }
        }
        break;

    case 24:
        if (name) return "Probing";
        if (desc) return "Proves all monsters' alignment, HP, speed and their true character.";

        {
            if (cast)
            {
                probing();
            }
        }
        break;

    case 25:
        if (name) return "Door Creation";
        if (desc) return "Creates doors on all surrounding squares.";

        if (cast)
        {
            project(0, 1, py, px, 0, GF_MAKE_DOOR, PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE);
            p_ptr->update |= (PU_FLOW);
            p_ptr->redraw |= (PR_MAP);
        }
        break;

    case 26:
        if (name) return "Telekinesis";
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

    case 27:
        if (name) return "Clairvoyance";
        if (desc) return "Maps and lights whole dungeon level. Knows all objects location. And gives telepathy for a while.";

        {
            int base = 25;
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                virtue_add(VIRTUE_KNOWLEDGE, 1);
                virtue_add(VIRTUE_ENLIGHTENMENT, 1);

                wiz_lite(p_ptr->tim_superstealth > 0);

                if (!p_ptr->telepathy)
                {
                    set_tim_esp(randint1(sides) + base, FALSE);
                }
            }
        }
        break;

    case 28:
        if (name) return "Device Mastery";
        if (desc) return "For a very short time, your magical devices are more powerful.";

        {
            int base = spell_power(plev/10);

            if (info) return info_duration(base, base);

            if (cast)
                set_tim_device_power(base + randint1(base), FALSE);
        }
        break;

    case 29:
        if (name) return "Alchemy";
        if (desc) return "Turns an item into 1/3 of its value in gold.";

        {
            if (cast)
            {
                if (!alchemy()) return NULL;
            }
        }
        break;

    case 30:
        if (name) return "Banishment";
        if (desc) return "Teleports all monsters in sight away unless resisted.";

        {
            int power = spell_power(plev * 4);

            if (info) return info_power(power);

            if (cast)
            {
                banish_monsters(power);
            }
        }
        break;

    case 31:
        if (name) return "Globe of Invulnerability";
        if (desc) return "Generates barrier which completely protect you from almost all damages. Takes a few your turns when the barrier breaks or duration time is exceeded.";

        {
            int base = 4;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_invuln(spell_power(randint1(base) + base), FALSE);
            }
        }
        break;
    }

    return "";
}


static cptr do_nature_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Detect Creatures";
        if (desc) return "Detects all monsters in your vicinity unless invisible.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_normal(rad);
            }
        }
        break;

    case 1:
        if (name) return "Lightning";
        if (desc) return "Fires a short beam of lightning.";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;
            int range = spell_power(plev / 6 + 2);

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                project_length = range;

                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_ELEC, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 2:
        if (name) return "Detect Doors and Traps";
        if (desc) return "Detects traps, doors, and stairs in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 3:
        if (name) return "Produce Food";
        if (desc) return "Produces a Ration of Food.";

        {
            if (cast)
            {
                object_type forge, *q_ptr = &forge;

                msg_print("A food ration is produced.");

                /* Create the food ration */
                object_prep(q_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
                object_origins(q_ptr, ORIGIN_ACQUIRE);

                /* Drop the object from heaven */
                drop_near(q_ptr, -1, py, px);
            }
        }
        break;

    case 4:
        if (name) return "Daylight";
        if (desc) return "Lights up nearby area and the inside of a room permanently.";

        {
            int dice = 2;
            int sides = spell_power(plev / 2);
            int rad = spell_power((plev / 10) + 1);

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(damroll(dice, sides), rad);

                if ( (prace_is_(RACE_VAMPIRE) || prace_is_(RACE_MON_VAMPIRE) || p_ptr->mimic_form == MIMIC_VAMPIRE)
                  && !res_save_default(RES_LITE) )
                {
                    msg_print("The daylight scorches your flesh!");
                    take_hit(DAMAGE_NOESCAPE, damroll(2, 2), "daylight");
                }
            }
        }
        break;

    case 5:
        if (name) return "Wind Walker";
        if (desc) return "Grants temporary levitation.";

        {
            int dur = spell_power(30);

            if (info) return info_duration(dur, dur);

            if (cast)
                set_tim_levitation(randint1(dur) + dur, FALSE);
        }
        break;

    case 6:
        if (name) return "Resist Environment";
        if (desc) return "Gives resistance to fire, cold and electricity for a while.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_cold(dur, FALSE);
                set_oppose_fire(dur, FALSE);
                set_oppose_elec(dur, FALSE);
            }
        }
        break;

    case 7:
        if (name) return "Cure Wounds & Poison";
        if (desc) return "Heals all cuts. Heals HP a little. Relieves poisoning a bit; completely cures low-level poisoning.";

        {
            int dice = 2;
            int sides = spell_power(8);

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                if (p_ptr->pclass != CLASS_BLOOD_MAGE)
                    hp_player(damroll(dice, sides));
                set_cut(0, TRUE);
                set_poisoned(p_ptr->poisoned - MAX(125, p_ptr->poisoned / 3), TRUE);
            }
        }
        break;

    case 8:
        if (name) return "Stone to Mud";
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

    case 9:
        if (name) return "Frost Bolt";
        if (desc) return "Fires a bolt or beam of cold.";

        {
            int dice = 3 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_COLD,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 10:
        if (name) return "Nature Awareness";
        if (desc) return "Maps nearby area. Detects all monsters, traps, doors and stairs.";

        {
            int rad1 = DETECT_RAD_MAP;
            int rad2 = DETECT_RAD_DEFAULT;

            if (info) return info_radius(MAX(rad1, rad2));

            if (cast)
            {
                map_area(rad1);
                detect_traps(rad2, TRUE);
                detect_doors(rad2);
                detect_stairs(rad2);
                detect_monsters_normal(rad2);
            }
        }
        break;

    case 11:
        if (name) return "Fire Bolt";
        if (desc) return "Fires a bolt or beam of fire.";

        {
            int dice = 5 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 12:
        if (name) return "Ray of Sunlight";
        if (desc) return "Fires a beam of light which damages to light-sensitive monsters.";

        {
            int dice = 6;
            int sides = 8;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                msg_print("A line of sunlight appears.");

                project_hook(
                    GF_LITE_WEAK,
                    dir,
                    spell_power(damroll(6, 8) + p_ptr->to_d_spell),
                    PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL
                );
            }
        }
        break;

    case 13:
        if (name) return "Entangle";
        if (desc) return "Attempts to slow all monsters in sight.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                slow_monsters(power);
            }
        }
        break;

    case 14:
        if (name) return "Nature's Gate";
        if (desc) return "Summons one or more animals. At higher levels, might summon hounds, reptiles or even an Ent!";

        if (cast)
        {
            bool success = FALSE;
            if (plev < 30)
                success = trump_summoning(1, TRUE, py, px, 0, SUMMON_ANIMAL_RANGER, PM_ALLOW_GROUP);
            else if (plev < 47)
            {
                switch (randint1(3))
                {
                case 1:
                    success = trump_summoning(1, TRUE, py, px, 0, SUMMON_HOUND, PM_ALLOW_GROUP);
                    break;
                case 2:
                    success = trump_summoning(1, TRUE, py, px, 0, SUMMON_HYDRA, PM_ALLOW_GROUP);
                    break;
                case 3:
                    success = trump_summoning((1 + (plev - 15)/ 10), TRUE, py, px, 0, SUMMON_ANIMAL_RANGER, PM_ALLOW_GROUP);
                    break;
                }
            }
            else
            {
                if (one_in_(5))
                    success = trump_summoning(1, TRUE, py, px, 0, SUMMON_ENT, PM_ALLOW_GROUP);
            }
            if (!success)
                msg_print("No help arrives.");
        }
        break;

    case 15:
        if (name) return "Herbal Healing";
        if (desc) return "Heals HP greatly. And heals cut, stun and perhaps poison.";

        {
            int heal = spell_power(500);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
                set_poisoned(p_ptr->poisoned - MAX(300, p_ptr->poisoned / 2), TRUE);
            }
        }
        break;

    case 16:
        if (name) return "Stair Building";
        if (desc) return "Creates a stair which goes down or up.";

        {
            if (cast)
            {
                stair_creation(FALSE);
            }
        }
        break;

    case 17:
        if (name) return "Stone Skin";
        if (desc) return "Gives bonus to AC for a while.";

        {
            int base = spell_power(20);
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_shield(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 18:
        if (name) return "Resistance True";
        if (desc) return "Gives resistance to fire, cold, electricity, acid and poison for a while.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_base(randint1(base) + base, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "Forest Creation";
        if (desc) return "Creates trees in all adjacent squares.";

        {
            if (cast)
            {
                tree_creation();
            }
        }
        break;

    case 20:
        if (name) return "Stone Tell";
        if (desc) return "*Identifies* an item.";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 21:
        if (name) return "Wall of Stone";
        if (desc) return "Creates granite walls in all adjacent squares.";

        {
            if (cast)
            {
                wall_stone();
            }
        }
        break;

    case 22:
        if (name) return "Protect from Corrosion";
        if (desc) return "Makes an equipment acid-proof.";

        {
            if (cast)
            {
                if (!rustproof()) return NULL;
            }
        }
        break;

    case 23:
        if (name) return "Call Sunlight";
        if (desc) return "Generates ball of light centered on you. Maps and lights whole dungeon level. Knows all objects location.";

        {
            int dam = spell_power(150 + p_ptr->to_d_spell);
            int rad = 8;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                fire_ball(GF_LITE, 0, dam, rad);
                virtue_add(VIRTUE_KNOWLEDGE, 1);
                virtue_add(VIRTUE_ENLIGHTENMENT, 1);
                wiz_lite(FALSE);

                if ( (prace_is_(RACE_VAMPIRE) || prace_is_(RACE_MON_VAMPIRE) || p_ptr->mimic_form == MIMIC_VAMPIRE)
                  && !res_save_default(RES_LITE) )
                {
                    msg_print("The sunlight scorches your flesh!");
                    take_hit(DAMAGE_NOESCAPE, 50, "sunlight");
                }
            }
        }
        break;

    case 24:
        if (name) return "Earthquake";
        if (desc) return "Shakes dungeon structure, and results in random swapping of floors and walls.";

        {
            int rad = spell_power(10);

            if (info) return info_radius(rad);

            if (cast)
            {
                earthquake(py, px, rad);
            }
        }
        break;

    case 25:
        if (name) return "Fire Storm";
        if (desc) return "Fires a huge ball of fire.";

        {
            int dam = spell_power(60 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;

    case 26:
        if (name) return "Blizzard";
        if (desc) return "Fires a huge ball of cold.";

        {
            int dam = spell_power(70 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_COLD, dir, dam, rad);
            }
        }
        break;

    case 27:
        if (name) return "Lightning Storm";
        if (desc) return "Fires a huge electric ball.";

        {
            int dam = spell_power(90 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ELEC, dir, dam, rad);
                break;
            }
        }
        break;

    case 28:
        if (name) return "Whirlpool";
        if (desc) return "Fires a huge ball of water.";

        {
            int dam = spell_power(100 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_WATER, dir, dam, rad);
            }
        }
        break;

    case 29:
        if (name) return "Ice Bolt";
        if (desc) return "Fires a bolt of ice.";

        {
            int dice = 5 + 15*plev/50;
            int sides = 15;

            if (info) return info_damage(spell_power(dice), sides, p_ptr->to_d_spell);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_ICE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 30:
        if (name) return "Gravity Storm";
        if (desc) return "Fires a huge ball of gravity.";

        {
            int dam = spell_power(70 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_GRAVITY, dir, dam, rad);
            }
        }
        break;

    case 31:
        if (name) return "Nature's Wrath";
        if (desc) return "You unleash Nature's full fury, the exact consequences of which can't be predicted.";

        if (cast)
        {
            int i;
            switch (randint1(6))
            {
            case 1: /* The original effect: Line of Sight damage, earthquake, disintegration ball */
                msg_print("Nature's Fury is unleashed!");
                dispel_monsters(spell_power(4 * plev + p_ptr->to_d_spell));
                earthquake(py, px, spell_power(20 + plev / 2));
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((100 + plev + p_ptr->to_d_spell) * 2),
                    GF_DISINTEGRATE,
                    PROJECT_KILL | PROJECT_ITEM
                );
                break;

            case 2: /* Deadly bolt of lightning */
                msg_print("Your hands crackle with electricity!");
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_ELEC,
                    dir,
                    spell_power(plev * 8 + p_ptr->to_d_spell)
                );
                break;

            case 3: /* Immense thunderclap */
                msg_print("There is a large thunderclap!");
                project_hack(GF_SOUND, spell_power(plev * 5 + p_ptr->to_d_spell));
                break;

            case 4: /* Gravitational Wave */
                msg_print("Space warps around you!");
                project_hack(GF_GRAVITY, spell_power(plev * 4 + p_ptr->to_d_spell));
                break;

            case 5: /* Elemental Storm */
                msg_print("You unleash the elements!");
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((120 + plev + p_ptr->to_d_spell) * 2),
                    GF_FIRE,
                    PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL
                );
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((120 + plev + p_ptr->to_d_spell) * 2),
                    GF_COLD,
                    PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL
                );
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((120 + plev + p_ptr->to_d_spell) * 2),
                    GF_ELEC,
                    PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL
                );
                break;

            case 6: /* Rock Storm */
                msg_print("You fire a storm of boulders!");
                if (!get_fire_dir(&dir)) return NULL;
                for (i = 0; i < 3; i++)
                    fire_ball(GF_SHARDS, dir, spell_power(70 + plev + p_ptr->to_d_spell), 1);
                break;
            }
        }
        break;
    }

    return "";
}


static cptr do_chaos_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    static const char s_dam[] = "dam ";
    static const char s_random[] = "random";

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Magic Missile";
        if (desc) return "Fires a weak bolt of magic.";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_MISSILE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "Trap / Door Destruction";
        if (desc) return "Destroys all traps in adjacent squares.";

        {
            int rad = 1;

            if (info) return info_radius(rad);

            if (cast)
            {
                destroy_doors_touch();
            }
        }
        break;

    case 2:
        if (name) return "Flash of Light";
        if (desc) return "Lights up nearby area and the inside of a room permanently.";

        {
            int dice = 2;
            int sides = spell_power(plev / 2);
            int rad = (plev / 10) + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(damroll(dice, sides), rad);
            }
        }
        break;

    case 3:
        if (name) return "Touch of Confusion";
        if (desc) return "Attempts to confuse the next monster that you hit.";

        {
            if (cast)
            {
                if (!(p_ptr->special_attack & ATTACK_CONFUSE))
                {
                    msg_print("Your hands start glowing.");

                    p_ptr->special_attack |= ATTACK_CONFUSE;
                    p_ptr->redraw |= (PR_STATUS);
                }
            }
        }
        break;

    case 4:
        if (name) return "Mana Burst";
        if (desc) return "Fires a ball of magic.";

        {
            int dice = 3;
            int sides = 5;
            int rad = spell_power((plev < 30) ? 2 : 3);
            int base;

            if (p_ptr->pclass == CLASS_MAGE ||
                p_ptr->pclass == CLASS_BLOOD_MAGE ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER ||
                p_ptr->pclass == CLASS_YELLOW_MAGE ||
                p_ptr->pclass == CLASS_GRAY_MAGE)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;


            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_MISSILE, /* GF_MANA? */
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 5:
        if (name) return "Fire Bolt";
        if (desc) return "Fires a bolt or beam of fire.";

        {
            int dice = 8 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 6:
        if (name) return "Fist of Force";
        if (desc) return "Fires a tiny ball of disintegration.";

        {
            int dice = 8 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_DISINTEGRATE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
                    0
                );
            }
        }
        break;

    case 7:
        if (name) return "Teleport Self";
        if (desc) return "Teleport long distance.";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                teleport_player(range, 0L);
            }
        }
        break;

    case 8:
        if (name) return "Wonder";
        if (desc) return "Fires something with random effects.";

        {
            if (info) return s_random;

            if (cast)
            {

                if (!get_fire_dir(&dir)) return NULL;

                cast_wonder(dir);
            }
        }
        break;

    case 9:
        if (name) return "Chaos Bolt";
        if (desc) return "Fires a bolt or beam of chaos.";

        {
            int dice = 10 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_CHAOS,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 10:
        if (name) return "Sonic Boom";
        if (desc) return "Generates a ball of sound centered on you.";

        {   /* Note: Damage is high relative to say fireball, but this is
               an adjacent spell while fireball is a distance spell. So it should
               do something like 33% *more* damage than a fireball. However, stunning
               is a bonus to be reckoned with ... (Design: Ranged = 75% Melee) */
            int dam = spell_power(50 + plev + p_ptr->to_d_spell);
            int rad = spell_power(plev / 10 + 2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                msg_print("BOOM! Shake the room!");
                project(0, rad, py, px, dam*2, GF_SOUND, PROJECT_KILL | PROJECT_ITEM);
            }
        }
        break;

    case 11:
        if (name) return "Doom Bolt";
        if (desc) return "Fires a beam of pure mana.";

        {
            int dice = 11 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(
                    GF_MANA,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 12:
        if (name) return "Fire Ball";
        if (desc) return "Fires a ball of fire.";

        {
            int dam = spell_power(plev + 55 + p_ptr->to_d_spell);
            int rad = spell_power(2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;

    case 13:
        if (name) return "Teleport Other";
        if (desc) return "Teleports all monsters on the line away unless resisted.";

        {
            int power = spell_power(plev*2);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 14:
        if (name) return "Word of Destruction";
        if (desc) return "Destroy everything in nearby area.";

        {
            int base = 12;
            int sides = 4;
            int power = spell_power(4 * plev);

            if (info) return info_power(power);

            if (cast)
            {
                destroy_area(py, px, base + randint1(sides), power);
            }
        }
        break;

    case 15:
        if (name) return "Invoke Logrus";
        if (desc) return "Fires a huge ball of chaos.";

        {
            int dam = spell_power(plev * 2 + 99 + p_ptr->to_d_spell);
            int rad = spell_power(plev / 5);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_CHAOS, dir, dam, rad);
            }
        }
        break;

    case 16:
        if (name) return "Polymorph Other";
        if (desc) return "Attempts to polymorph a monster.";

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                poly_monster(dir);
            }
        }
        break;

    case 17:
        if (name) return "Chain Lightning";
        if (desc) return "Fires lightning beams in all directions.";

        {
            int dice = 5 + plev / 10;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                for (dir = 0; dir <= 9; dir++)
                {
                    fire_beam(
                        GF_ELEC,
                        dir,
                        spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                    );
                }
            }
        }
        break;

    case 18:
        if (name) return "Arcane Binding";
        if (desc) return "It attempts to recharge a device using your mana for power.";

        {
            int power = spell_power(90);

            if (info) return info_power(power);

            if (cast)
            {
                if (!recharge_from_player(power)) return NULL;
            }
        }
        break;

    case 19:
        if (name) return "Disintegrate";
        if (desc) return "Fires a huge ball of disintegration.";

        {
            int dam = spell_power(plev + 70 + p_ptr->to_d_spell);
            int rad = 3 + plev / 40;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_DISINTEGRATE, dir, dam, rad);
            }
        }
        break;

    case 20:
        if (name) return "Alter Reality";
        if (desc) return "Recreates current dungeon level.";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                alter_reality();
            }
        }
        break;

    case 21:
        if (name) return "Magic Rocket";
        if (desc) return "Fires a magic rocket.";

        {
            int dam = spell_power(50 + plev * 4 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                msg_print("You launch a rocket!");
                fire_rocket(GF_ROCKET, dir, dam, rad);
            }
        }
        break;

    case 22:
        if (name) return "Chaos Branding";
        if (desc) return "Makes current weapon a Chaotic weapon.";

        {
            if (cast)
            {
                brand_weapon(EGO_WEAPON_CHAOS);
            }
        }
        break;

    case 23:
        if (name) return "Summon Demon";
        if (desc) return "Summons a demon.";

        {
            if (cast)
            {
                u32b mode = 0L;
                bool pet = !one_in_(3);

                if (pet) mode |= PM_FORCE_PET;
                else mode |= PM_NO_PET;
                if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, (plev * 3) / 2, SUMMON_DEMON, mode))
                {
                    msg_print("The area fills with a stench of sulphur and brimstone.");

                    if (pet)
                    {
                        msg_print("'What is thy bidding... Master?'");
                    }
                    else
                    {
                        msg_print("'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'");
                    }
                }
            }
        }
        break;

    case 24:
        if (name) return "Beam of Gravity";
        if (desc) return "Fires a beam of gravity.";

        {
            int dice = 9 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(
                    GF_GRAVITY,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 25:
        if (name) return "Meteor Swarm";
        if (desc) return "Makes meteor balls fall down to nearby random locations.";

        {
            int dam = spell_power(plev * 2 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_multi_damage(dam);

            if (cast)
            {
                cast_meteor(dam, rad);
            }
        }
        break;

    case 26:
        if (name) return "Flame Strike";
        if (desc) return "Generate a huge ball of fire centered on you.";

        {
            int dam = spell_power(300 + 3 * plev + p_ptr->to_d_spell*2);
            int rad = 8;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                fire_ball(GF_FIRE, 0, dam, rad);
            }
        }
        break;

    case 27:
        if (name) return "Call Chaos";
        if (desc) return "Generate random kind of balls or beams.";

        {
            if (cast)
            {
                call_chaos(100);
            }
        }
        break;

    case 28:
        if (name) return "Polymorph Self";
        if (desc) return "Polymorphs yourself into a new form.";

        if (cast)
        {
            int which;
            switch (randint1(50))
            {
            case 1: which = one_in_(10) ? MIMIC_DEMON_LORD : MIMIC_DEMON; break;
            case 2: which = MIMIC_VAMPIRE; break;
            default:
                for (;;)
                {
                    which = randint0(MAX_RACES);
                    if ( which != RACE_HUMAN
                      && which != RACE_DEMIGOD
                      && which != RACE_DRACONIAN
                      && which != RACE_ANDROID
                      && which != RACE_WEREWOLF
                      && which != RACE_BEORNING
                      && which != RACE_DOPPELGANGER
                      && p_ptr->prace != which
                      && !(get_race_aux(which, 0)->flags & RACE_IS_MONSTER)
                      && !(get_race_aux(which, 0)->flags & RACE_NO_POLY) )
                    {
                        break;
                    }
                }
            }
            set_mimic(50 + randint1(50), which, FALSE);
            if (p_ptr->mimic_form == MIMIC_NONE) msg_print("There is no effect.");
        }
        break;

    case 29:
        if (name) return "Mana Storm";
        if (desc) return "Fires an extremely powerful huge ball of pure mana.";

        {
            int dam = spell_power(300 + plev * 4 + p_ptr->to_d_spell);
            int rad = spell_power(4);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_MANA, dir, dam, rad);
            }
        }
        break;

    case 30:
        if (name) return "Breathe Logrus";
        if (desc) return "Fires an extremely powerful ball of chaos.";

        {
            int dam = spell_power(3*p_ptr->chp/4 + p_ptr->to_d_spell);
            int rad = spell_power(2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_CHAOS, dir, dam, rad);
            }
        }
        break;

    case 31:
        if (name) return "Call the Void";
        if (desc) return "Fires rockets, mana balls and nuclear waste balls in all directions each unless you are not adjacent to any walls. Otherwise *destroys* huge area.";

        {
            if (info) return format("%s3 * 175", s_dam);

            if (cast)
            {
                call_the_();
            }
        }
        break;
    }

    return "";
}


static cptr do_death_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    static const char s_dam[] = "dam ";
    static const char s_random[] = "random";

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Detect Unlife";
        if (desc) return "Detects all nonliving monsters in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_nonliving(rad);
            }
        }
        break;

    case 1:
        if (name) return "Malediction";
        if (desc) return "Fires a tiny ball of evil power which hurts good monsters greatly.";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;
            int rad = 0;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                int dam;
                if (!get_fire_dir(&dir)) return NULL;
                dam = spell_power(damroll(dice, sides) + p_ptr->to_d_spell);
                fire_ball(GF_HELL_FIRE, dir, dam, rad);

                if (one_in_(5))
                {
                    /* Special effect first */
                    int effect = randint1(1000);

                    if (effect == 666)
                        fire_ball_hide(GF_DEATH_RAY, dir, spell_power(plev * 200), 0);
                    else if (effect < 500)
                        fire_ball_hide(GF_TURN_ALL, dir, spell_power(plev), 0);
                    else if (effect < 800)
                        fire_ball_hide(GF_OLD_CONF, dir, dam, 0);
                    else
                        fire_ball_hide(GF_STUN, dir, dam, 0);
                }
            }
        }
        break;

    case 2:
        if (name) return "Detect Evil";
        if (desc) return "Detects all evil monsters in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_evil(rad);
            }
        }
        break;

    case 3:
        if (name) return "Stinking Cloud";
        if (desc) return "Fires a ball of poison.";

        {
            int dam = spell_power(10 + plev / 2 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_POIS, dir, dam, rad);
            }
        }
        break;

    case 4:
        if (name) return "Black Sleep";
        if (desc) return "Attempts to sleep a monster.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                sleep_monster(dir, power);
            }
        }
        break;

    case 5:
        if (name) return "Undead Resistance";
        if (desc) return "Gives resistance to poison and cold.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_cold(dur, FALSE);
                set_oppose_pois(dur, FALSE);
            }
        }
        break;

    case 6:
        if (name) return "Horrify";
        if (desc) return "Attempts to scare and stun a monster.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fear_monster(dir, power);
                stun_monster(dir, 5 + plev/5);
            }
        }
        break;

    case 7:
        if (name) return "Enslave Undead";
        if (desc) return "Attempts to charm an undead monster.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                control_one_undead(dir, power);
            }
        }
        break;

    case 8:
        if (name) return "Orb of Entropy";
        if (desc) return "Fires a ball which damages living monsters.";

        {
            int dice = 3;
            int sides = 6;
            int rad = (plev < 30) ? 2 : 3;
            int base;

            if (p_ptr->pclass == CLASS_MAGE ||
                p_ptr->pclass == CLASS_BLOOD_MAGE ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER ||
                p_ptr->pclass == CLASS_YELLOW_MAGE ||
                p_ptr->pclass == CLASS_GRAY_MAGE)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;


            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_OLD_DRAIN,
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 9:
        if (name) return "Nether Bolt";
        if (desc) return "Fires a bolt or beam of nether.";

        {
            int dice = 8 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_NETHER,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 10:
        if (name) return "Cloud Kill";
        if (desc) return "Generate a ball of poison centered on you.";

        {
            int dam = spell_power((30 + plev) * 2 + p_ptr->to_d_spell);
            int rad = spell_power(plev / 10 + 2);

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                project(0, rad, py, px, dam, GF_POIS, PROJECT_GRID | PROJECT_KILL | PROJECT_ITEM);
            }
        }
        break;

    case 11:
        if (name) return "Genocide One";
        if (desc) return "Attempts to vanish a monster.";

        {
            int power = spell_power(plev*3);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball_hide(GF_GENOCIDE, dir, power, 0);
            }
        }
        break;

    case 12:
        if (name) return "Poison Branding";
        if (desc) return "Makes current weapon poison branded.";

        {
            if (cast)
            {
                brand_weapon_slaying(OF_BRAND_POIS, OF_RES_POIS);
            }
        }
        break;

    case 13:
        if (name) return "Vampiric Drain";
        if (desc) return "Absorbs some HP from a monster and gives them to you. You will also gain nutritional sustenance from this.";

        {
            int dice = 1;
            int sides = spell_power(plev * 2);
            int base = spell_power(plev * 2 + p_ptr->to_d_spell);

            if (info) return info_damage(dice, sides, base);

            if (cast)
            {
                int dam = base + damroll(dice, sides);

                if (!get_fire_dir(&dir)) return NULL;

                if (drain_life(dir, dam))
                {
                    if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                    {
                        msg_print("You are unaffected.");
                        break;
                    }

                    virtue_add(VIRTUE_SACRIFICE, -1);
                    virtue_add(VIRTUE_VITALITY, -1);

                    hp_player(dam);

                    /*
                     * Gain nutritional sustenance:
                     * 150/hp drained
                     *
                     * A Food ration gives 5000
                     * food points (by contrast)
                     * Don't ever get more than
                     * "Full" this way But if we
                     * ARE Gorged, it won't cure
                     * us
                     */
                    dam = p_ptr->food + MIN(5000, 100 * dam);

                    /* Not gorged already */
                    if (p_ptr->food < PY_FOOD_MAX)
                        set_food(dam >= PY_FOOD_MAX ? PY_FOOD_MAX - 1 : dam);
                }
            }
        }
        break;

    case 14:
        if (name) return "Animate Dead";
        if (desc) return "Resurrects nearby corpse and skeletons. And makes these your pets.";

        {
            if (cast)
            {
                animate_dead(0, py, px);
            }
        }
        break;

    case 15:
        if (name) return "Genocide";
        if (desc) return "Eliminates an entire class of monster, exhausting you. Powerful or unique monsters may resist.";

        {
            int power = spell_power(plev*3);

            if (info) return info_power(power);

            if (cast)
            {
                symbol_genocide(power, TRUE);
            }
        }
        break;

    case 16:
        if (name) return "Berserk";
        if (desc) return "Gives bonus to hit and HP, immunity to fear for a while. But decreases AC.";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_shero(randint1(base) + base, FALSE);
                hp_player(30);
            }
        }
        break;

    case 17:
        if (name) return "Invoke Spirits";
        if (desc) return "Causes random effects.";

        {
            if (info) return s_random;

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                cast_invoke_spirits(dir);
            }
        }
        break;

    case 18:
        if (name) return "Dark Bolt";
        if (desc) return "Fires a bolt or beam of darkness.";

        {
            int dice = 4 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_DARK,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 19:
        if (name) return "Battle Frenzy";
        if (desc) return "Gives a bonus to accuracy and HP and fear resistance for a while, hastes you, and increases AC.";

        {
            int b_base = spell_power(25);
            int sp_base = spell_power(plev / 2);
            int sp_sides = 20 + plev / 2;

            if (info) return info_duration(b_base, b_base);

            if (cast)
            {
                set_hero(randint1(b_base) + b_base, FALSE);
                set_blessed(randint1(b_base) + b_base, FALSE);
                set_fast(randint1(sp_sides) + sp_base, FALSE);
            }
        }
        break;

    case 20:
        if (name) return "Vampiric Branding";
        if (desc) return "Makes current weapon Vampiric.";

        {
            if (cast)
            {
                brand_weapon(EGO_WEAPON_DEATH);
            }
        }
        break;

    case 21:
        if (name) return "Vampirism True";
        if (desc) return "Fires 3 bolts. Each of the bolts absorbs some HP from a monster and gives them to you.";

        {
            int dam = spell_power(100 + p_ptr->to_d_spell/3);

            if (info) return format("%s3*%d", s_dam, dam);

            if (cast)
            {
                int i;

                if (!get_fire_dir(&dir)) return NULL;

                virtue_add(VIRTUE_SACRIFICE, -1);
                virtue_add(VIRTUE_VITALITY, -1);

                for (i = 0; i < 3; i++)
                {
                    if (drain_life(dir, dam) && p_ptr->pclass != CLASS_BLOOD_MAGE)
                        vamp_player(dam);
                }
            }
        }
        break;

    case 22:
        if (name) return "Nether Wave";
        if (desc) return "Damages all living monsters in sight.";

        {
            int sides = plev * 3;

            if (info) return info_damage(1, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                dispel_living(spell_power(randint1(sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 23:
        if (name) return "Darkness Storm";
        if (desc) return "Fires a huge ball of darkness.";

        {
            int dam = 100 + py_prorata_level_aux(200, 1, 1, 2);
            int rad = spell_power(4);

            dam = spell_power(dam + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_DARK, dir, dam, rad);
            }
        }
        break;

    case 24:
        if (name) return "Death Ray";
        if (desc) return "Fires a beam of death.";

        {
            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                project_hook(GF_DEATH_RAY, dir, plev * 200, /*  v--- This is mean as it auto kills the player! */
                                PROJECT_STOP | PROJECT_KILL /*| PROJECT_REFLECTABLE*/);
            }
        }
        break;

    case 25:
        if (name) return "Raise the Dead";
        if (desc) return "Summons an undead monster.";

        {
            if (cast)
            {
                int type;
                bool pet = one_in_(3);
                u32b mode = 0L;

                type = (plev > 47 ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD);

                if (!pet || (pet && (plev > 24) && one_in_(3)))
                    mode |= PM_ALLOW_GROUP;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, (plev * 3) / 2, type, mode))
                {
                    msg_print("Cold winds begin to blow around you, carrying with them the stench of decay...");


                    if (pet)
                    {
                        msg_print("Ancient, long-dead forms arise from the ground to serve you!");
                    }
                    else
                    {
                        msg_print("'The dead arise... to punish you for disturbing them!'");
                    }

                    virtue_add(VIRTUE_UNLIFE, 1);
                }
            }
        }
        break;

    case 26:
        if (name) return "Esoteria";
        if (desc) return "Identifies an item. Or *identifies* an item at higher level.";

        {
            if (cast)
            {
                if (randint1(50) > spell_power(plev))
                {
                    if (!ident_spell(NULL)) return NULL;
                }
                else
                {
                    if (!identify_fully(NULL)) return NULL;
                }
            }
        }
        break;

    case 27:
        if (name) return "Polymorph Vampire";
        if (desc) return "Mimic a vampire for a while. Loses abilities of original race and gets abilities as a vampire.";

        {
            int base = spell_power(10 + plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_mimic(base + randint1(base), MIMIC_VAMPIRE, FALSE);
                if (p_ptr->mimic_form == MIMIC_NONE) msg_print("There is no effect.");
            }
        }
        break;

    case 28:
        if (name) return "Restore Life";
        if (desc) return "Restore lost life force and experience.";

        {
            if (cast)
            {
                restore_level();
                lp_player(1000);
            }
        }
        break;

    case 29:
        if (name) return "Mass Genocide";
        if (desc) return "Eliminates all nearby monsters, exhausting you. Powerful or unique monsters may be able to resist.";

        {
            int power = spell_power(plev*3);

            if (info) return info_power(power);

            if (cast)
            {
                mass_genocide(power, TRUE);
            }
        }
        break;

    case 30:
        if (name) return "Nether Storm";
        if (desc) return "Generates a huge ball of nether.";

        {
            int dam = spell_power(plev * 12 + p_ptr->to_d_spell + 5);
            int rad = spell_power(plev / 5);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_NETHER, dir, dam, rad);
            }
        }
        break;

    case 31:
        if (name) return "Wraithform";
        if (desc) return "Gives the ability to pass walls and reduces most damages by half.";

        {
            int base = spell_power(plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_wraith_form(randint1(base) + base, FALSE);
            }
        }
        break;
    }

    return "";
}

static cptr do_trump_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;

    static const char s_random[] = "random";

    int dir;
    int plev = p_ptr->lev;
    int x = px;
    int y = py;

    if (!fail && old_target_okay() && los(py, px, target_row, target_col) && !one_in_(3))
    {
        y = target_row;
        x = target_col;
    }

    switch (spell)
    {
    case 0:
        if (name) return "Phase Door";
        if (desc) return "Teleport short distance.";

        {
            int range = 10;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 1:
        if (name) return "Trump Spiders";
        if (desc) return "Summons spiders.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of a spider...");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_SPIDER, PM_ALLOW_GROUP))
                {
                    if (fail)
                    {
                        msg_print("The summoned spiders get angry!");
                    }
                }
            }
        }
        break;

    case 2:
        if (name) return "Shuffle";
        if (desc) return "Causes random effects.";

        {
            if (info) return s_random;

            if (cast)
            {
                if (TRUE || get_check("Are you sure you wish to shuffle?"))
                    cast_shuffle();
                else
                    return NULL;
            }
        }
        break;

    case 3:
        if (name) return "Reset Recall";
        if (desc) return "Resets the 'deepest' level for recall spell.";

        {
            if (cast)
            {
                if (!reset_recall()) return NULL;
            }
        }
        break;

    case 4:
        if (name) return "Teleport";
        if (desc) return "Teleport long distance.";

        {
            int range = plev * 4;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 5:
        if (name) return "Trump Spying";
        if (desc) return "Gives telepathy for a while.";

        {
            int base = spell_power(25);
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 6:
        if (name) return "Teleport Away";
        if (desc) return "Teleports all monsters on the line away unless resisted.";

        {
            int power = spell_power(plev*2);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 7:
        if (name) return "Trump Animals";
        if (desc) return "Summons an animal.";

        {
            if (cast || fail)
            {
                int type = (!fail ? SUMMON_ANIMAL_RANGER : SUMMON_ANIMAL);

                msg_print("You concentrate on the trump of an animal...");

                if (trump_summoning(1, !fail, y, x, 0, type, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned animal gets angry!");
                    }
                }
            }
        }
        break;

    case 8:
        if (name) return "Trump Reach";
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

    case 9:
        if (name) return "Trump Kamikaze";
        if (desc) return "Summons monsters which explode by itself.";

        {
            if (cast || fail)
            {
                int x, y;
                int type;

                if (cast)
                {
                    if (!target_set(TARGET_KILL)) return NULL;
                    x = target_col;
                    y = target_row;
                }
                else
                {
                    /* Summons near player when failed */
                    x = px;
                    y = py;
                }

                if (p_ptr->pclass == CLASS_BEASTMASTER)
                    type = SUMMON_KAMIKAZE_LIVING;
                else
                    type = SUMMON_KAMIKAZE;

                msg_print("You concentrate on several trumps at once...");

                if (trump_summoning(2 + randint0(plev / 7), !fail, y, x, 0, type, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned creatures get angry!");
                    }
                }
            }
        }
        break;

    case 10:
        if (name) return "Phantasmal Servant";
        if (desc) return "Summons a ghost.";

        {
            /* Phantasmal Servant is not summoned as enemy when failed */
            if (cast)
            {
                int summon_lev = plev * 2 / 3 + randint1(plev / 2);

                if (trump_summoning(1, !fail, y, x, (summon_lev * 3 / 2), SUMMON_PHANTOM, 0L))
                {
                    msg_print("'Your wish, master?'");
                }
            }
        }
        break;

    case 11:
        if (name) return "Haste Monster";
        if (desc) return "Hastes a monster.";

        {
            if (cast)
            {
                bool result;

                /* Temporary enable target_pet option */
                bool old_target_pet = target_pet;
                target_pet = TRUE;

                result = get_fire_dir(&dir);

                /* Restore target_pet option */
                target_pet = old_target_pet;

                if (!result) return NULL;

                speed_monster(dir);
            }
        }
        break;

    case 12:
        if (name) return "Teleport Level";
        if (desc) return "Teleport to up or down stairs in a moment.";

        {
            if (cast)
            {
                if (!py_teleport_level("Are you sure? (Teleport Level) ")) return NULL;
            }
        }
        break;

    case 13:
        if (name) return "Dimension Door";
        if (desc) return "Teleport to given location.";

        {
            int range = plev / 2 + 10;

            if (info) return info_range(range);

            if (cast)
            {
                msg_print("You open a dimensional gate. Choose a destination.");

                if (!dimension_door(range)) return NULL;
            }
        }
        break;

    case 14:
        if (name) return "Word of Recall";
        if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 15:
        if (name) return "Banish";
        if (desc) return "Teleports all monsters in sight away unless resisted.";

        {
            int power = spell_power(plev * 4);

            if (info) return info_power(power);

            if (cast)
            {
                banish_monsters(power);
            }
        }
        break;

    case 16:
        if (name) return "Swap Position";
        if (desc) return "Swaps positions with a monster.";

        {
            if (cast)
            {
                bool result;

                /* HACK -- No range limit */
                project_length = -1;

                result = get_fire_dir(&dir);

                /* Restore range to default */
                project_length = 0;

                if (!result) return NULL;

                teleport_swap(dir);
            }
        }
        break;

    case 17:
        if (name) return "Trump Undead";
        if (desc) return "Summons an undead monster.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of an undead creature...");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_UNDEAD, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned undead creature gets angry!");
                    }
                }
            }
        }
        break;

    case 18:
        if (name) return "Trump Reptiles";
        if (desc) return "Summons a hydra.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of a reptile...");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_HYDRA, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned reptile gets angry!");
                    }
                }
            }
        }
        break;

    case 19:
        if (name) return "Trump Monsters";
        if (desc) return "Summons some monsters.";

        {
            if (cast || fail)
            {
                int type;

                msg_print("You concentrate on several trumps at once...");

                if (p_ptr->pclass == CLASS_BEASTMASTER)
                    type = SUMMON_LIVING;
                else
                    type = 0;

                if (trump_summoning((1 + (plev - 15)/ 10), !fail, y, x, 0, type, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned creatures get angry!");
                    }
                }

            }
        }
        break;

    case 20:
        if (name) return "Trump Hounds";
        if (desc) return "Summons a group of hounds.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of a hound...");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_HOUND, PM_ALLOW_GROUP))
                {
                    if (fail)
                    {
                        msg_print("The summoned hounds get angry!");
                    }
                }
            }
        }
        break;

    case 21:
        if (name) return "Trump Branding";
        if (desc) return "Makes current weapon a Trump weapon.";

        {
            if (cast)
            {
                brand_weapon(EGO_WEAPON_TRUMP);
            }
        }
        break;

    case 22:
        if (name) return "Living Trump";
        if (desc) return "Gives mutation which makes you teleport randomly or makes you able to teleport at will.";

        {
            if (cast)
            {
                int mutation;

                if (one_in_(7))
                    /* Teleport control */
                    mutation = MUT_TELEPORT;
                else
                    /* Random teleportation (uncontrolled) */
                    mutation = MUT_TELEPORT_RND;

                /* Gain the mutation */
                if (mut_gain(mutation))
                {
                    msg_print("You have turned into a Living Trump.");
                }
            }
        }
        break;

    case 23:
        if (name) return "Trump Cyberdemon";
        if (desc) return "Summons a cyber demon.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of a Cyberdemon...");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_CYBER, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned Cyberdemon gets angry!");
                    }
                }
            }
        }
        break;

    case 24:
        if (name) return "Trump Divination";
        if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 25:
        if (name) return "Trump Lore";
        if (desc) return "*Identifies* an item.";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 26:
        if (name) return "Heal Monster";
        if (desc) return "Heal a monster.";

        {
            int heal = spell_power(plev * 10 + 200);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                bool result;

                /* Temporary enable target_pet option */
                bool old_target_pet = target_pet;
                target_pet = TRUE;

                result = get_fire_dir(&dir);

                /* Restore target_pet option */
                target_pet = old_target_pet;

                if (!result) return NULL;

                heal_monster(dir, heal);
            }
        }
        break;

    case 27:
        if (name) return "Trump Dragon";
        if (desc) return "Summons a dragon.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of a dragon...");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_DRAGON, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned dragon gets angry!");
                    }
                }
            }
        }
        break;

    case 28:
        if (name) return "Trump Meteor";
        if (desc) return "Makes meteor balls fall down to nearby random locations.";

        {
            int dam = spell_power(plev * 2 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_multi_damage(dam);

            if (cast)
            {
                cast_meteor(dam, rad);
            }
        }
        break;

    case 29:
        if (name) return "Trump Demon";
        if (desc) return "Summons a demon.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of a demon...");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_DEMON, 0L))
                {
                    if (fail)
                    {
                        msg_print("The summoned demon gets angry!");
                    }
                }
            }
        }
        break;

    case 30:
        if (name) return "Trump Greater Undead";
        if (desc) return "Summons a greater undead.";

        {
            if (cast || fail)
            {
                msg_print("You concentrate on the trump of a greater undead being...");
                /* May allow unique depend on level and dice roll */
                if (trump_summoning(1, !fail, y, x, 0, SUMMON_HI_UNDEAD, PM_ALLOW_UNIQUE))
                {
                    if (fail)
                    {
                        msg_print("The summoned greater undead creature gets angry!");
                    }
                }
            }
        }
        break;

    case 31:
        if (name) return "Trump Ancient Dragon";
        if (desc) return "Summons an ancient dragon.";

        {
            if (cast)
            {
                int type;

                if (p_ptr->pclass == CLASS_BEASTMASTER)
                    type = SUMMON_HI_DRAGON_LIVING;
                else
                    type = SUMMON_HI_DRAGON;

                msg_print("You concentrate on the trump of an ancient dragon...");

                /* May allow unique depend on level and dice roll */
                if (trump_summoning(1, !fail, y, x, 0, type, PM_ALLOW_UNIQUE))
                {
                    if (fail)
                    {
                        msg_print("The summoned ancient dragon gets angry!");
                    }
                }
            }
        }
        break;
    }

    return "";
}


static cptr do_arcane_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Zap";
        if (desc) return "Fires a bolt or beam of lightning.";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 3;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_ELEC,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "Wizard Lock";
        if (desc) return "Locks a door.";

        {
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                wizard_lock(dir);
            }
        }
        break;

    case 2:
        if (name) return "Detect Invisibility";
        if (desc) return "Detects all invisible monsters in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_invis(rad);
            }
        }
        break;

    case 3:
        if (name) return "Detect Monsters";
        if (desc) return "Detects all monsters in your vicinity unless invisible.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_normal(rad);
            }
        }
        break;

    case 4:
        if (name) return "Blink";
        if (desc) return "Teleport short distance.";

        {
            int range = 10;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 5:
        if (name) return "Light Area";
        if (desc) return "Lights up nearby area and the inside of a room permanently.";

        {
            int dice = 2;
            int sides = spell_power(plev / 2);
            int rad = plev / 10 + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(damroll(dice, sides), rad);
            }
        }
        break;

    case 6:
        if (name) return "Trap & Door Destruction";
        if (desc) return "Fires a beam which destroys traps and doors.";

        {
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                destroy_door(dir);
            }
        }
        break;

    case 7:
        if (name) return "Cure Light Wounds";
        if (desc) return "Heals cut and HP a little.";

        {
            int dice = 2;
            int sides = spell_power(8);

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(damroll(dice, sides));
                set_cut(p_ptr->cut - 10, TRUE);
            }
        }
        break;

    case 8:
        if (name) return "Detect Doors & Traps";
        if (desc) return "Detects traps, doors, and stairs in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 9:
        if (name) return "Phlogiston";
        if (desc) return "Adds more turns of light to a lantern or torch.";

        {
            if (cast)
            {
                phlogiston();
            }
        }
        break;

    case 10:
        if (name) return "Detect Treasure";
        if (desc) return "Detects all treasures in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_treasure(rad);
                detect_objects_gold(rad);
            }
        }
        break;

    case 11:
        if (name) return "Detect Enchantment";
        if (desc) return "Detects all magical items in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_objects_magic(rad);
            }
        }
        break;

    case 12:
        if (name) return "Detect Objects";
        if (desc) return "Detects all items in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_objects_normal(rad);
            }
        }
        break;

    case 13:
        if (name) return "Cure Poison";
        if (desc) return "Relieves poisoning a bit. Completely cures low-level poisoning.";

        {
            if (cast)
            {
                set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE);
            }
        }
        break;

    case 14:
        if (name) return "Resist Cold";
        if (desc) return "Gives resistance to cold.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_cold(randint1(base) + base, FALSE);
            }
        }
        break;

    case 15:
        if (name) return "Resist Fire";
        if (desc) return "Gives resistance to fire.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_fire(randint1(base) + base, FALSE);
            }
        }
        break;

    case 16:
        if (name) return "Resist Lightning";
        if (desc) return "Gives resistance to electricity.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_elec(randint1(base) + base, FALSE);
            }
        }
        break;

    case 17:
        if (name) return "Resist Acid";
        if (desc) return "Gives resistance to acid.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_acid(randint1(base) + base, FALSE);
            }
        }
        break;

    case 18:
        if (name) return "Cure Medium Wounds";
        if (desc) return "Heals cut and HP more.";

        {
            int dice = 4;
            int sides = spell_power(8);

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(damroll(dice, sides));
                set_cut((p_ptr->cut / 2) - 50, TRUE);
            }
        }
        break;

    case 19:
        if (name) return "Teleport";
        if (desc) return "Teleport long distance.";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 20:
        if (name) return "Identify";
        if (desc) return "Identifies an item.";

        {
            if (cast)
            {
                if (!ident_spell(NULL)) return NULL;
            }
        }
        break;

    case 21:
        if (name) return "Stone to Mud";
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

    case 22:
        if (name) return "Ray of Light";
        if (desc) return "Fires a beam of light which damages to light-sensitive monsters.";

        {
            int dice = 6;
            int sides = 8;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                msg_print("A line of light appears.");

                lite_line(dir);
            }
        }
        break;

    case 23:
        if (name) return "Satisfy Hunger";
        if (desc) return "Satisfies hunger.";

        {
            if (cast)
            {
                set_food(PY_FOOD_MAX - 1);
            }
        }
        break;

    case 24:
        if (name) return "See Invisible";
        if (desc) return "Gives see invisible for a while.";

        {
            int base = spell_power(24);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_invis(randint1(base) + base, FALSE);
            }
        }
        break;

    case 25:
        if (name) return "Resist Poison";
        if (desc) return "Gives resistance to poison.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_pois(randint1(base) + base, FALSE);
            }
        }
        break;

    case 26:
        if (name) return "Teleport Level";
        if (desc) return "Teleport to up or down stairs in a moment.";

        {
            if (cast)
            {
                if (!py_teleport_level("Are you sure? (Teleport Level) ")) return NULL;
            }
        }
        break;

    case 27:
        if (name) return "Teleport Away";
        if (desc) return "Teleports all monsters on the line away unless resisted.";

        {
            int power = spell_power(plev);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 28:
        if (name) return "Recharging";
        if (desc)
        {
            if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                return "It attempts to recharge a device using your blood for power.";
            else
                return "It attempts to recharge a device using your mana for power.";
        }

        {
            int power = spell_power(plev * 3 / 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!recharge_from_player(power)) return NULL;
            }
        }
        break;

    case 29:
        if (name) return "Detection";
        if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 30:
        if (name) return "Word of Recall";
        if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 31:
        if (name) return "Clairvoyance";
        if (desc) return "Maps and lights whole dungeon level. Knows all objects location. And gives telepathy for a while.";

        {
            int base = 25;
            int sides = 30;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                virtue_add(VIRTUE_KNOWLEDGE, 1);
                virtue_add(VIRTUE_ENLIGHTENMENT, 1);

                wiz_lite(p_ptr->tim_superstealth > 0);

                if (!p_ptr->telepathy)
                {
                    set_tim_esp(randint1(sides) + base, FALSE);
                }
            }
        }
        break;
    }

    return "";
}

static bool _can_enchant(obj_ptr obj) {
    if (object_is_(obj, TV_SWORD, SV_POISON_NEEDLE)) return FALSE;
    return object_is_weapon_armour_ammo(obj);
}
bool craft_enchant(int max, int inc)
{
    obj_prompt_t prompt = {0};
    char         o_name[MAX_NLEN];
    bool         improved = FALSE;
    u32b         flgs[OF_ARRAY_SIZE];

    prompt.prompt = "Enchant which item?";
    prompt.error = "You have nothing to enchant.";
    prompt.filter = _can_enchant;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;

    object_desc(o_name, prompt.obj, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    /* Some objects cannot be enchanted */
    obj_flags(prompt.obj, flgs);
    if (have_flag(flgs, OF_NO_ENCHANT))
        return FALSE;

    /* Enchanting is now automatic ... It was always possible to max
     * out enchanting quickly with skilled macro usage, but other players
     * are inviting carpal tunnel issues to no purpose. */
    if (object_is_weapon_ammo(prompt.obj))
    {
        if (prompt.obj->to_h < max)
        {
            prompt.obj->to_h = MIN(max, prompt.obj->to_h + inc);
            if (prompt.obj->to_h >= 0)
                break_curse(prompt.obj);
            improved = TRUE;
        }
        if (prompt.obj->to_d < max)
        {
            prompt.obj->to_d = MIN(max, prompt.obj->to_d + inc);
            if (prompt.obj->to_d >= 0)
                break_curse(prompt.obj);
            improved = TRUE;
        }
    }
    else
    {
        if (prompt.obj->to_a < max)
        {
            prompt.obj->to_a = MIN(max, prompt.obj->to_a + inc);
            if (prompt.obj->to_a >= 0)
                break_curse(prompt.obj);
            improved = TRUE;
        }
    }


    msg_format("%s %s glow%s brightly!",
            (prompt.obj->loc.where != INV_FLOOR) ? "Your" : "The", o_name,
            (prompt.obj->number > 1) ? "" : "s");

    if (!improved)
    {
        msg_print("The enchantment failed.");
        if (one_in_(3) && virtue_current(VIRTUE_ENCHANTMENT) < 100)
            virtue_add(VIRTUE_ENCHANTMENT, -1);
    }
    else
    {
        virtue_add(VIRTUE_ENCHANTMENT, 1);
        /* Enchantment should not allow gold farming ... */
        if (object_is_nameless(prompt.obj))
            prompt.obj->discount = 99;
        obj_release(prompt.obj, OBJ_RELEASE_ENCHANT);
    }
    return TRUE;
}

static cptr do_craft_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Minor Enchantment";
        if (desc) return "Attempts to increase +to-hit, +to-dam of a weapon, or to increase +AC of armor.";

        if (cast)
        {
            if (!craft_enchant(2 + plev/5, 1))
                return NULL;
        }
        break;

    case 1:
        if (name) return "Regeneration";
        if (desc) return "Gives regeneration ability for a while.";

        {
            int base = spell_power(80);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_regen(base + randint1(base), FALSE);
            }
        }
        break;

    case 2:
        if (name) return "Satisfy Hunger";
        if (desc) return "Satisfies hunger.";

        {
            if (cast)
            {
                set_food(PY_FOOD_MAX - 1);
            }
        }
        break;

    case 3:
        if (name) return "Resist Cold";
        if (desc) return "Gives resistance to cold.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_cold(randint1(base) + base, FALSE);
            }
        }
        break;

    case 4:
        if (name) return "Resist Fire";
        if (desc) return "Gives resistance to fire.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_fire(randint1(base) + base, FALSE);
            }
        }
        break;

    case 5:
        if (name) return "Heroism";
        if (desc) return "Removes fear, and gives bonus to hit and 10 more HP for a while.";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_hero(randint1(base) + base, FALSE);
            }
        }
        break;

    case 6:
        if (name) return "Resist Lightning";
        if (desc) return "Gives resistance to electricity.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_elec(randint1(base) + base, FALSE);
            }
        }
        break;

    case 7:
        if (name) return "Resist Acid";
        if (desc) return "Gives resistance to acid.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_acid(randint1(base) + base, FALSE);
            }
        }
        break;

    case 8:
        if (name) return "See Invisibility";
        if (desc) return "Gives see invisible for a while.";

        {
            int base = spell_power(24);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_invis(randint1(base) + base, FALSE);
            }
        }
        break;

    case 9:
        if (name) return "Remove Curse";
        if (desc) return "Removes normal curses from equipped items.";

        if (cast)
        {
            if (remove_curse())
                msg_print("You feel as if someone is watching over you.");
        }
        break;

    case 10:
        if (name) return "Resist Poison";
        if (desc) return "Gives resistance to poison.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_pois(randint1(base) + base, FALSE);
            }
        }
        break;

    case 11:
        if (name) return "Berserk";
        if (desc) return "Gives bonus to hit and HP, immunity to fear for a while. But decreases AC.";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_shero(randint1(base) + base, FALSE);
                hp_player(30);
            }
        }
        break;

    case 12:
        if (name) return "Self Knowledge";
        if (desc) return "Gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.";

        {
            if (cast)
            {
                self_knowledge();
            }
        }
        break;

    case 13:
        if (name) return "Identify";
        if (desc) return "Identifies an item.";

        {
            if (cast)
            {
                if (!ident_spell(NULL)) return NULL;
            }
        }
        break;

    case 14:
        if (name) return "Curing";
        if (desc) return "It cures what ails you including fear, poison, stunning, cuts and hallucination. Serious poisoning may not be cured completely.";
        {
            if (cast)
            {
                fear_clear_p();
                set_poisoned(p_ptr->poisoned - MAX(150, p_ptr->poisoned / 3), TRUE);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
                set_image(0, TRUE);
            }
        }
        break;

    case 15:
        if (name) return "Elemental Branding";
        if (desc) return "Grants your attacks a temporary elemental brand of your choice.";

        {
            int base = plev / 2;

            if (info) return info_duration(base, base);

            if (cast)
            {
                if (!choose_ele_attack()) return NULL;
            }
        }
        break;

    case 16:
        if (name) return "Telepathy";
        if (desc) return "Gives telepathy for a while.";

        {
            int base = 25;
            int sides = 30;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 17:
        if (name) return "Stone Skin";
        if (desc) return "Gives bonus to AC for a while.";

        {
            int base = 30;
            int sides = 20;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_shield(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 18:
        if (name) return "Resistance";
        if (desc) return "Gives resistance to fire, cold, electricity, acid and poison for a while.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_base(randint1(base) + base, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "Haste Self";
        if (desc) return "Hastes you for a while.";

        {
            int base = spell_power(plev);
            int sides = spell_power(20 + plev);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_fast(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 20:
        if (name) return "Whirlwind Attack";
        if (desc) return "Attacks all adjacent monsters.";

        {
            if (cast)
            {
                int              y = 0, x = 0;
                cave_type       *c_ptr;
                monster_type    *m_ptr;
                int              dir;

                for (dir = 0; dir < 8; dir++)
                {
                    y = py + ddy_ddd[dir];
                    x = px + ddx_ddd[dir];
                    c_ptr = &cave[y][x];
                    m_ptr = &m_list[c_ptr->m_idx];
                    if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
                        py_attack(y, x, 0);
                }
            }
        }
        break;

    case 21:
        if (name) return "Polish Shield";
        if (desc) return "Makes your shield reflect missiles and bolt spells.";
        if (cast)
        {
            if (!polish_shield()) return NULL;
        }
        break;

    case 22:
        if (name) return "Weaponmastery";
        if (desc) return "For a short time, your melee weapon becomes more deadly.";
        {
            int base = spell_power(3 + plev / 10);

            if (info) return info_duration(base, base);

            if (cast)
                set_tim_weaponmastery(randint1(base) + base, FALSE);
        }
        break;

    case 23:
        if (name) return "Magical Armor";
        if (desc) return "Gives resistance to magic, bonus to AC, resistance to confusion, blindness, reflection, free action and levitation for a while.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_magicdef(randint1(base) + base, FALSE);
            }
        }
        break;

    case 24:
        if (name) return "Remove All Curse";
        if (desc) return "Removes normal and heavy curse from equipped items.";

        {
            if (cast)
            {
                if (remove_all_curse())
                {
                    msg_print("You feel as if someone is watching over you.");
                }
            }
        }
        break;

    case 25:
        if (name) return "Simplification";
        if (desc) return "Removes all of an item's magical attributes, including its status as an ego item or artifact. Magically improved ego items cannot be simplified.";

        {
            if (cast)
            {
                if (!mundane_spell(FALSE)) return NULL;
            }
        }
        break;

    case 26:
        if (name) return "Knowledge True";
        if (desc) return "*Identifies* an item.";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 27:
        if (name) return "Enchantment";
        if (desc) return "Attempts to increase +to-hit, +to-dam of a weapon, or to increase +AC of armor.";

        if (cast)
        {
            if (!craft_enchant(15, 3))
                return NULL;
        }
        break;

    case 28:
        if (name) return "Crafting";
        if (desc) return "Makes chosen weapon, armor or ammo an ego item.";

        if (cast)
        {
            if (!cast_crafting())
                return NULL;
        }
        break;

    case 29:
        if (name) return "Living Trump";
        if (desc) return "Gives mutation which makes you teleport randomly or makes you able to teleport at will.";

        if (cast)
        {
            int mutation;

            if (one_in_(7) || dun_level == 0)
                mutation = MUT_TELEPORT;
            else
                mutation = MUT_TELEPORT_RND;

            if (mut_gain(mutation))
                msg_print("You have turned into a Living Trump.");
        }
        break;

    case 30:
        if (name) return "Immunity";
        if (desc) return "Gives an immunity to fire, cold, electricity or acid for a while.";

        {
            int base = spell_power(13);

            if (info) return info_duration(base, base);

            if (cast)
            {
                if (!choose_ele_immune(base + randint1(base))) return NULL;
            }
        }
        break;

    case 31:
        if (name) return "Mana Branding";
        if (desc) return "Temporarily brands your weapon with force.";

        {
        int base = spell_power(plev / 4);

            if (info) return info_duration(base, base);
            if (cast)
            {
                set_tim_force(base + randint1(base), FALSE);
            }
        }
        break;
    }

    return "";
}


static cptr do_daemon_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    static const char s_dam[] = "dam ";

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Magic Missile";
        if (desc) return "Fires a weak bolt of magic.";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_MISSILE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "Detect Unlife";
        if (desc) return "Detects all nonliving monsters in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_nonliving(rad);
            }
        }
        break;

    case 2:
        if (name) return "Evil Bless";
        if (desc) return "Gives bonus to hit and AC for a few turns.";

        {
            int base = spell_power(12);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_blessed(randint1(base) + base, FALSE);
            }
        }
        break;

    case 3:
        if (name) return "Resist Fire";
        if (desc) return "Gives resistance to fire for a while.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_fire(randint1(base) + base, FALSE);
            }
        }
        break;

    case 4:
        if (name) return "Horrify";
        if (desc) return "Attempts to scare and stun a monster.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fear_monster(dir, power);
                stun_monster(dir, 5 + plev/5);
            }
        }
        break;

    case 5:
        if (name) return "Nether Bolt";
        if (desc) return "Fires a bolt or beam of nether.";

        {
            int dice = 6 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_NETHER,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 6:
        if (name) return "Summon Manes";
        if (desc) return "Summons a manes.";

        {
            if (cast)
            {
                if (!summon_specific(SUMMON_WHO_PLAYER, py, px, spell_power(plev * 3 / 2), SUMMON_MANES, (PM_ALLOW_GROUP | PM_FORCE_PET)))
                {
                    msg_print("No Manes arrive.");
                }
            }
        }
        break;

    case 7:
        if (name) return "Hellish Flame";
        if (desc) return "Fires a ball of evil power. Hurts good monsters greatly.";

        {
            int dice = 3;
            int sides = 6;
            int rad = (plev < 30) ? 2 : 3;
            int base;

            if (p_ptr->pclass == CLASS_MAGE ||
                p_ptr->pclass == CLASS_BLOOD_MAGE ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER ||
                p_ptr->pclass == CLASS_YELLOW_MAGE ||
                p_ptr->pclass == CLASS_GRAY_MAGE)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;


            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_HELL_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 8:
        if (name) return "Dominate Demon";
        if (desc) return "Attempts to charm a demon.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                control_one_demon(dir, power);
            }
        }
        break;

    case 9:
        if (name) return "Vision";
        if (desc) return "Maps nearby area.";

        {
            int rad = DETECT_RAD_MAP;

            if (info) return info_radius(rad);

            if (cast)
            {
                map_area(rad);
            }
        }
        break;

    case 10:
        if (name) return "Resist Nether";
        if (desc) return "Gives resistance to nether for a while.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_res_nether(randint1(base) + base, FALSE);
            }
        }
        break;

    case 11:
        if (name) return "Plasma Bolt";
        if (desc) return "Fires a bolt or beam of plasma.";

        {
            int dice = 11 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_PLASMA,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 12:
        if (name) return "Fire Ball";
        if (desc) return "Fires a ball of fire.";

        {
            int dam = spell_power(plev + 55 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;

    case 13:
        if (name) return "Fire Branding";
        if (desc) return "Makes current weapon fire branded.";

        {
            if (cast)
            {
                brand_weapon_slaying(OF_BRAND_FIRE, OF_RES_FIRE);
            }
        }
        break;

    case 14:
        if (name) return "Nether Ball";
        if (desc) return "Fires a huge ball of nether.";

        {
            int dam = spell_power(plev * 3 / 2 + 100 + p_ptr->to_d_spell);
            int rad = spell_power(plev / 20 + 2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_NETHER, dir, dam, rad);
            }
        }
        break;

    case 15:
        if (name) return "Summon Demon";
        if (desc) return "Summons a demon.";

        {
            if (cast)
            {
                bool pet = !one_in_(3);
                u32b mode = 0L;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= PM_NO_PET;
                if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, spell_power(plev*2/3+randint1(plev/2)), SUMMON_DEMON, mode))
                {
                    msg_print("The area fills with a stench of sulphur and brimstone.");


                    if (pet)
                    {
                        msg_print("'What is thy bidding... Master?'");
                    }
                    else
                    {
                        msg_print("'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'");
                    }
                }
                else
                {
                    msg_print("No demons arrive.");
                }
                break;
            }
        }
        break;

    case 16:
        if (name) return "Devilish Eye";
        if (desc) return "Gives telepathy for a while.";

        {
            int base = spell_power(30);
            int sides = 25;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(base) + sides, FALSE);
            }
        }
        break;

    case 17:
        if (name) return "Devilish Cloak";
        if (desc) return "Gives resistance to fire, acid and poison as well as an aura of fire.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;

                set_oppose_fire(dur, FALSE);
                set_oppose_acid(dur, FALSE);
                set_oppose_pois(dur, FALSE);
                set_tim_sh_fire(dur, FALSE);
                break;
            }
        }
        break;

    case 18:
        if (name) return "The Flow of Lava";
        if (desc) return "Generates a ball of fire centered on you which transforms floors to magma.";

        {
            int dam = spell_power((55 + plev + p_ptr->to_d_spell) * 2);
            int rad = 3;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                fire_ball(GF_FIRE, 0, dam, rad);
                fire_ball_hide(GF_LAVA_FLOW, 0, 2 + randint1(2), rad);
            }
        }
        break;

    case 19:
        if (name) return "Plasma Ball";
        if (desc) return "Fires a ball of plasma.";

        {
            int dam = spell_power(plev * 3 / 2 + 80 + p_ptr->to_d_spell);
            int rad = spell_power(2 + plev / 40);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_PLASMA, dir, dam, rad);
            }
        }
        break;

    case 20:
        if (name) return "Polymorph Demon";
        if (desc) return "Mimic a demon for a while. Loses abilities of original race and gets abilities as a demon.";

        {
            int base = spell_power(10 + plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_mimic(base + randint1(base), MIMIC_DEMON, FALSE);
                if (p_ptr->mimic_form == MIMIC_NONE) msg_print("There is no effect.");
            }
        }
        break;

    case 21:
        if (name) return "Nether Wave";
        if (desc) return "Damages all monsters in sight. Hurts good monsters greatly.";

        {
            if (info) return info_damage(1, spell_power(plev*2), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                dispel_monsters(spell_power(randint1(plev * 2) + p_ptr->to_d_spell));
                dispel_good(spell_power(randint1(plev * 2) + p_ptr->to_d_spell));
            }
        }
        break;

    case 22:
        if (name) return "Kiss of Succubus";
        if (desc) return "Fires a ball of nexus.";

        {
            int dam = spell_power(75 + plev * 3 + p_ptr->to_d_spell);
            int rad = 4;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_NEXUS, dir, dam, rad);
            }
        }
        break;

    case 23:
        if (name) return "Doom Hand";
        if (desc) return "Attempt to mortally wound a target monster, draining a large proportion of their remaining health.";

        {
            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                else msg_print("You invoke the Hand of Doom!");

                fire_ball_hide(GF_HAND_DOOM, dir, spell_power(plev * 3), 0);
            }
        }
        break;

    case 24:
        if (name) return "Raise the Morale";
        if (desc) return "Gives bonus to hit and 10 more HP for a while.";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_hero(randint1(base) + base, FALSE);
            }
        }
        break;

    case 25:
        if (name) return "Immortal Body";
        if (desc) return "Gives resistance to time for a while.";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_res_time(randint1(base)+base, FALSE);
            }
        }
        break;

    case 26:
        if (name) return "Insanity Circle";
        if (desc) return "Generate balls of chaos, confusion and charm centered on you.";

        {
            int dam = spell_power(50 + plev + p_ptr->to_d_spell);
            int power = spell_power(20 + plev);
            int rad = spell_power(3 + plev / 20);

            if (info) return format("%s%d+%d", s_dam, dam/2, dam/2);

            if (cast)
            {
                fire_ball(GF_CHAOS, 0, dam, rad);
                fire_ball(GF_CONFUSION, 0, dam, rad);
                fire_ball(GF_CHARM, 0, power, rad);
            }
        }
        break;

    case 27:
        if (name) return "Explode Pets";
        if (desc) return "Makes all pets explode.";

        {
            if (cast)
            {
                discharge_minion();
            }
        }
        break;

    case 28:
        if (name) return "Summon Greater Demon";
        if (desc) return "Summons greater demon. It need to sacrifice a corpse of human ('p','h' or 't').";

        {
            if (cast)
            {
                if (!cast_summon_greater_demon()) return NULL;
            }
        }
        break;

    case 29:
        if (name) return "Hellfire";
        if (desc) return "Fires a powerful ball of evil power. Hurts good monsters greatly.";

        {
            int dam = spell_power(666 + p_ptr->to_d_spell);
            int rad = 3;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_HELL_FIRE, dir, dam, rad);
                take_hit(DAMAGE_USELIFE, 20 + randint1(30), "the strain of casting Hellfire");
            }
        }
        break;

    case 30:
        if (name) return "Send to Hell";
        if (desc) return "Attempts to send a single monster directly to hell.";

        {
            int power = 666;

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball_hide(GF_GENOCIDE, dir, power, 0);
            }
        }
        break;

    case 31:
        if (name) return "Polymorph Demonlord";
        if (desc) return "Mimic a demon lord for a while. Loses abilities of original race and gets great abilities as a demon lord. Even hard walls can't stop your walking.";

        {
            int base = spell_power(15);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_mimic(base + randint1(base), MIMIC_DEMON_LORD, FALSE);
                if (p_ptr->mimic_form == MIMIC_NONE) msg_print("There is no effect.");
            }
        }
        break;
    }

    return "";
}


static cptr do_crusade_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Punishment";
        if (desc) return "Fires a bolt or beam of lightning.";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_ELEC,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "Detect Evil";
        if (desc) return "Detects all evil monsters in your vicinity.";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_evil(rad);
            }
        }
        break;

    case 2:
        if (name) return "Remove Fear";
        if (desc) return "Removes fear.";

        if (cast)
            fear_clear_p();
        break;

    case 3:
        if (name) return "Scare Monster";
        if (desc) return "Attempts to scare a monster.";

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fear_monster(dir, power);
            }
        }
        break;

    case 4:
        if (name) return "Sanctuary";
        if (desc) return "Attempts to sleep monsters in the adjacent squares.";

        {
            int power = plev;

            if (info) return info_power(power);

            if (cast)
            {
                sleep_monsters_touch();
            }
        }
        break;

    case 5:
        if (name) return "Portal";
        if (desc) return "Teleport medium distance.";

        {
            int range = 25 + plev / 2;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 6:
        if (name) return "Star Dust";
        if (desc) return "Fires many bolts of light near the target.";

        {
            int dice = spell_power(3 + (plev - 1) / 9);
            int sides = 2;

            if (info) return info_multi_damage_dice(dice, sides);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_blast(GF_LITE, dir, dice, sides, 10, 3);
            }
        }
        break;

    case 7:
        if (name) return "Purify";
        if (desc) return "Heals cuts and stuns as well as reducing poison.";

        {
            if (cast)
            {
                set_cut(0, TRUE);
                set_poisoned(p_ptr->poisoned - MAX(50, p_ptr->poisoned / 3), TRUE);
                set_stun(0, TRUE);
            }
        }
        break;

    case 8:
        if (name) return "Scatter Evil";
        if (desc) return "Attempts to teleport an evil monster away.";

        {
            int power = MAX_SIGHT * 5;

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_AWAY_EVIL, dir, power, 0);
            }
        }
        break;

    case 9:
        if (name) return "Holy Orb";
        if (desc) return "Fires a ball of holy power. Hurts evil monsters greatly, but doesn't affect good monsters.";

        {
            int dice = 3;
            int sides = 6;
            int rad = (plev < 30) ? 2 : 3;
            int base;

            if (p_ptr->pclass == CLASS_PRIEST ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;

            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(
                    GF_HOLY_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 10:
        if (name) return "Exorcism";
        if (desc) return "Damages all undead and demons in sight, and scares all evil monsters in sight.";

        {
            int sides = plev;
            int power = plev;

            if (info) return info_damage(1, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                dispel_undead(spell_power(randint1(sides) + p_ptr->to_d_spell));
                dispel_demons(spell_power(randint1(sides) + p_ptr->to_d_spell));
                turn_evil(power);
            }
        }
        break;

    case 11:
        if (name) return "Remove Curse";
        if (desc) return "Removes normal curses from equipped items.";

        {
            if (cast)
            {
                if (remove_curse())
                {
                    msg_print("You feel as if someone is watching over you.");
                }
            }
        }
        break;

    case 12:
        if (name) return "Sense Unseen";
        if (desc) return "Gives see invisible for a while.";

        {
            int base = 24;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_invis(randint1(base) + base, FALSE);
            }
        }
        break;

    case 13:
        if (name) return "Protection from Evil";
        if (desc) return "Gives aura which protect you from evil monster's physical attack.";

        {
            int base = 25;
            int sides = 3 * plev;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_protevil(randint1(sides) + sides, FALSE);
            }
        }
        break;

    case 14:
        if (name) return "Judgment Thunder";
        if (desc) return "Fires a powerful bolt of lightning.";

        {
            int dam = spell_power(plev * 5 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(GF_ELEC, dir, dam);
            }
        }
        break;

    case 15:
        if (name) return "Holy Word";
        if (desc) return "Damages all evil monsters in sight, heals HP somewhat, and completely heals poison, stun and cut status.";

        {
            int dam_sides = plev * 6;
            int heal = spell_power(100);

            if (info) return format("dam:d%d/h%d", spell_power(dam_sides), heal);

            if (cast)
            {
                dispel_evil(spell_power(randint1(dam_sides) + p_ptr->to_d_spell));
                if (p_ptr->pclass != CLASS_BLOOD_MAGE)
                    hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
				set_poisoned(0, TRUE);
            }
        }
        break;

    case 16:
        if (name) return "Unbarring Ways";
        if (desc) return "Fires a beam which destroys traps and doors.";

        {
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                destroy_door(dir);
            }
        }
        break;

    case 17:
        if (name) return "Arrest";
        if (desc) return "Attempts to paralyze an evil monster.";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                stasis_evil(dir);
            }
        }
        break;

    case 18:
        if (name) return "Angelic Cloak";
        if (desc) return "Gives resistance to acid, cold and lightning. Gives aura of holy power which injures evil monsters which attacked you for a while.";

        {
            int base = 20;

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_acid(dur, FALSE);
                set_oppose_cold(dur, FALSE);
                set_oppose_elec(dur, FALSE);
                set_tim_sh_holy(dur, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "Dispel Undead & Demons";
        if (desc) return "Damages all undead and demons in sight.";

        {
            int dam = spell_power(plev * 3 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                dispel_undead(dam);
                dispel_demons(dam);
            }
        }
        break;

    case 20:
        if (name) return "Dispel Evil";
        if (desc) return "Damages all evil monsters in sight.";

        {
            int dam = spell_power(plev * 3 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                dispel_evil(dam);
            }
        }
        break;

    case 21:
        if (name) return "Holy Blade";
        if (desc) return "Makes current weapon especially deadly against evil monsters.";

        {
            if (cast)
            {
                brand_weapon_slaying(OF_SLAY_EVIL, OF_INVALID);
            }
        }
        break;

    case 22:
        if (name) return "Star Burst";
        if (desc) return "Fires a huge ball of powerful light.";

        {
            int dam = 100 + py_prorata_level_aux(200, 1, 1, 2);
            int rad = spell_power(4);

            dam = spell_power(dam + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_LITE, dir, dam, rad);
            }
        }
        break;

    case 23:
        if (name) return "Summon Angel";
        if (desc) return "Summons an angel.";

        {
            if (cast)
            {
                bool pet = !one_in_(3);
                u32b mode = 0L;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= PM_NO_PET;
                if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, (plev * 3) / 2, SUMMON_ANGEL, mode))
                {
                    if (pet)
                    {
                        msg_print("'What is thy bidding... Master?'");
                    }
                    else
                    {
                        msg_print("Mortal! Repent of thy impiousness.");
                    }
                }
            }
        }
        break;

    case 24:
        if (name) return "Heroism";
        if (desc) return "Removes fear, and gives bonus to hit and 10 more HP for a while.";

        {
            int base = 25;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_hero(randint1(base) + base, FALSE);
                hp_player(10);
            }
        }
        break;

    case 25:
        if (name) return "Dispel Curse";
        if (desc) return "Removes normal and heavy curse from equipped items.";

        {
            if (cast)
            {
                if (remove_all_curse())
                {
                    msg_print("You feel as if someone is watching over you.");
                }
            }
        }
        break;

    case 26:
        if (name) return "Banish Evil";
        if (desc) return "Teleports all evil monsters in sight away unless resisted.";

        {
            int power = spell_power(100);

            if (info) return info_power(power);

            if (cast)
            {
                if (banish_evil(power))
                {
                    msg_print("The holy power banishes evil!");

                }
            }
        }
        break;

    case 27:
        if (name) return "Armageddon";
        if (desc) return "Destroy everything in nearby area.";

        {
            int base = 12;
            int sides = 4;

            if (cast)
            {
                destroy_area(py, px, base + randint1(sides), spell_power(4 * plev));
            }
        }
        break;

    case 28:
        if (name) return "An Eye for an Eye";
        if (desc) return "Gives special aura for a while. When you are attacked by a monster, the monster are injured with same amount of damage as you take.";

        {
            int base = 10;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_eyeeye(randint1(base) + base, FALSE);
            }
        }
        break;

    case 29:
        if (name) return "Wrath of the God";
        if (desc) return "Drops many balls of disintegration near the target.";

        {
            int dam = spell_power(plev * 3 + 25 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_multi_damage(dam);

            if (cast)
            {
                if (!cast_wrath_of_the_god(dam, rad)) return NULL;
            }
        }
        break;

    case 30:
        if (name) return "Divine Intervention";
        if (desc) return "Damages all adjacent monsters with holy power. Damages and attempt to slow, stun, confuse, scare and freeze all monsters in sight. And heals HP.";

        {
            int b_dam = spell_power(plev * 11);
            int d_dam = spell_power(plev * 4 + p_ptr->to_d_spell);
            int heal = spell_power(100);
            int power = spell_power(plev * 4);

            if (info) return format("h%d/dm%d+%d", heal, d_dam, b_dam/2);

            if (cast)
            {
                project(0, 1, py, px, b_dam, GF_HOLY_FIRE, PROJECT_KILL);
                dispel_monsters(d_dam);
                slow_monsters(power);
                stun_monsters(5 + plev/5);
                confuse_monsters(power);
                turn_monsters(power);
                stasis_monsters(power/3);
                if (p_ptr->pclass != CLASS_BLOOD_MAGE)
                    hp_player(heal);
            }
        }
        break;

    case 31:
        if (name) return "Crusade";
        if (desc) return "Attempts to charm all good monsters in sight, and scare all non-charmed monsters, and summons great number of knights, and gives heroism, bless, speed and protection from evil.";
        if (cast) cast_crusade();
        break;
    }

    return "";
}


static cptr do_music_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;
    bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
    bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "Song of Holding";
        if (desc) return "Attempts to slow all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You start humming a slow, steady melody...");
            bard_start_singing(spell, MUSIC_SLOW);
        }

        {
            int power = plev;

            if (info) return info_power(power);

            if (cont)
            {
                slow_monsters(power);
            }
        }
        break;

    case 1:
        if (name) return "Song of Blessing";
        if (desc) return "Gives bonus to hit and AC for a few turns.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("The holy power of the Music of the Ainur enters you...");
            bard_start_singing(spell, MUSIC_BLESS);
        }

        if (stop)
        {
            if (!p_ptr->blessed)
            {
                msg_print("The prayer has expired.");
            }
        }

        break;

    case 2:
        if (name) return "Wrecking Note";
        if (desc) return "Fires a bolt of sound.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        {
            int dice = 4 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt(
                    GF_SOUND,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 3:
        if (name) return "Stun Pattern";
        if (desc) return "Attempts to stun all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You weave a pattern of sounds to bewilder and daze...");
            bard_start_singing(spell, MUSIC_STUN);
        }

        {
            int dice = spell_power(plev / 10);
            int sides = 2;

            if (info) return info_power_dice(dice, sides);

            if (cont)
            {
                stun_monsters(damroll(dice, sides));
            }
        }

        break;

    case 4:
        if (name) return "Flow of Life";
        if (desc) return "Heals HP a little.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("Life flows through you as you sing a song of healing...");
            bard_start_singing(spell, MUSIC_L_LIFE);
        }

        {
            int dice = 2;
            int sides = spell_power(6);

            if (info) return info_heal(dice, sides, 0);

            if (cont)
            {
                hp_player(damroll(dice, sides));
            }
        }

        break;

    case 5:
        if (name) return "Song of the Sun";
        if (desc) return "Lights up nearby area and the inside of a room permanently.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        {
            int dice = 2;
            int sides = plev / 2;
            int rad = plev / 10 + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                msg_print("Your uplifting song brings brightness to dark places...");

                lite_area(damroll(dice, sides), rad);
            }
        }
        break;

    case 6:
        if (name) return "Song of Fear";
        if (desc) return "Attempts to scare all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You start weaving a fearful pattern...");
            bard_start_singing(spell, MUSIC_FEAR);
        }

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cont)
            {
                project_hack(GF_TURN_ALL, power);
            }
        }

        break;

    case 7:
        if (name) return "Heroic Ballad";
        if (desc) return "Removes fear, and gives bonus to hit and 10 more HP for a while.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You start singing a song of intense fighting...");

            (void)hp_player(10);
            fear_clear_p();

            /* Recalculate hitpoints */
            p_ptr->update |= (PU_HP);

            bard_start_singing(spell, MUSIC_HERO);
        }

        if (stop)
        {
            if (!p_ptr->hero)
            {
                msg_print("The heroism wears off.");
                /* Recalculate hitpoints */
                p_ptr->update |= (PU_HP);
            }
        }

        break;

    case 8:
        if (name) return "Clairaudience";
        if (desc) return "Detects traps, doors and stairs in your vicinity. And detects all monsters at level 15, treasures and items at level 20. Maps nearby area at level 25. Lights and know the whole level at level 40. These effects occurs by turns while this song continues.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("Your quiet music sharpens your sense of hearing...");

            /* Hack -- Initialize the turn count */
            p_ptr->magic_num1[2] = 0;

            bard_start_singing(spell, MUSIC_DETECT);
        }

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cont)
            {
                int count = p_ptr->magic_num1[2];

                if (count >= 19) wiz_lite(FALSE);
                if (count >= 11)
                {
                    map_area(rad);
                    if (plev > 39 && count < 19)
                        p_ptr->magic_num1[2] = count + 1;
                }
                if (count >= 6)
                {
                    /* There are too many hidden treasure. So... */
                    /* detect_treasure(rad); */
                    detect_objects_gold(rad);
                    detect_objects_normal(rad);

                    if (plev > 24 && count < 11)
                        p_ptr->magic_num1[2] = count + 1;
                }
                if (count >= 3)
                {
                    detect_monsters_invis(rad);
                    detect_monsters_normal(rad);

                    if (plev > 19 && count < 6)
                        p_ptr->magic_num1[2] = count + 1;
                }
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);

                if (plev > 14 && count < 3)
                    p_ptr->magic_num1[2] = count + 1;
            }
        }

        break;

    case 9:
        if (name) return "Soul Shriek";
        if (desc) return "Damages all monsters in sight with PSI damages.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You start singing a song of soul in pain...");
            bard_start_singing(spell, MUSIC_PSI);
        }

        {
            int dice = 1;
            int sides = plev * 3 / 2;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                project_hack(
                    GF_PSI,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }

        break;

    case 10:
        if (name) return "Song of Lore";
        if (desc) return "Identifies all items which are in the adjacent squares.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You recall the rich lore of the world...");
            bard_start_singing(spell, MUSIC_ID);
        }

        {
            int rad = 1;

            if (info) return info_radius(rad);

            if (cont || cast)
            {
                project(0, rad, py, px, 0, GF_IDENTIFY, PROJECT_ITEM);
            }
        }

        break;

    case 11:
        if (name) return "Hiding Tune";
        if (desc) return "Gives improved stealth.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("Your song carries you beyond the sight of mortal eyes...");
            bard_start_singing(spell, MUSIC_STEALTH);
        }

        if (stop)
        {
            if (!p_ptr->tim_stealth)
            {
                msg_print("You are no longer hided.");
            }
        }

        break;

    case 12:
        if (name) return "Illusion Pattern";
        if (desc) return "Attempts to confuse all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You weave a pattern of sounds to beguile and confuse...");
            bard_start_singing(spell, MUSIC_CONF);
        }

        {
            int power = plev * 2;

            if (info) return info_power(power);

            if (cont)
            {
                confuse_monsters(power);
            }
        }

        break;

    case 13:
        if (name) return "Doomcall";
        if (desc) return "Damages all monsters in sight with booming sound.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("The fury of the Downfall of Numenor lashes out...");
            bard_start_singing(spell, MUSIC_SOUND);
        }

        {
            int dice = 10 + plev / 5;
            int sides = 7;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                project_hack(
                    GF_SOUND,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }

        break;

    case 14:
        if (name) return "Firiel's Song";
        if (desc) return "Resurrects nearby corpse and skeletons. And makes these your pets.";

        {
            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("The themes of life and revival are woven into your song...");

                animate_dead(0, py, px);
            }
        }
        break;

    case 15:
        if (name) return "Fellowship Chant";
        if (desc) return "Attempts to charm all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You weave a slow, soothing melody of imploration...");
            bard_start_singing(spell, MUSIC_CHARM);
        }

        {
            int dice = spell_power(10 + plev / 15);
            int sides = 6;

            if (info) return info_power_dice(dice, sides);

            if (cont)
            {
                charm_monsters(damroll(dice, sides));
            }
        }

        break;

    case 16:
        if (name) return "Sound of disintegration";
        if (desc) return "Makes you be able to burrow into walls. Objects under your feet evaporate.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You weave a violent pattern of sounds to break wall.");
            bard_start_singing(spell, MUSIC_WALL);
        }

        {
            if (cont || cast)
            {
                project(0, 0, py, px,
                    0, GF_DISINTEGRATE, PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE);
            }
        }
        break;

    case 17:
        if (name) return "Finrod's Resistance";
        if (desc) return "Gives resistance to fire, cold, electricity, acid and poison.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You sing a song of perseverance against powers...");
            bard_start_singing(spell, MUSIC_RESIST);
        }

        if (stop)
        {
            if (!p_ptr->oppose_acid)
            {
                msg_print("You feel less resistant to acid.");
            }

            if (!p_ptr->oppose_elec)
            {
                msg_print("You feel less resistant to electricity.");
            }

            if (!p_ptr->oppose_fire)
            {
                msg_print("You feel less resistant to fire.");
            }

            if (!p_ptr->oppose_cold)
            {
                msg_print("You feel less resistant to cold.");
            }

            if (!p_ptr->oppose_pois)
            {
                msg_print("You feel less resistant to poison.");
            }
        }

        break;

    case 18:
        if (name) return "Hobbit Melodies";
        if (desc) return "Hastes you.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You start singing joyful pop song...");
            bard_start_singing(spell, MUSIC_SPEED);
        }

        if (stop)
        {
            if (!p_ptr->fast)
            {
                msg_print("You feel yourself slow down.");
            }
        }

        break;

    case 19:
        if (name) return "World Contortion";
        if (desc) return "Teleports all nearby monsters away unless resisted.";

        {
            int rad = spell_power(plev / 15 + 1);
            int power = spell_power(plev * 3 + 1);

            if (info) return info_radius(rad);

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("Reality whirls wildly as you sing a dizzying melody...");

                project(0, rad, py, px, power, GF_AWAY_ALL, PROJECT_KILL);
            }
        }
        break;

    case 20:
        if (name) return "Dispelling chant";
        if (desc) return "Damages all monsters in sight. Hurts evil monsters greatly.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You cry out in an ear-wracking voice...");
            bard_start_singing(spell, MUSIC_DISPEL);
        }

        {
            int m_sides = plev * 3;
            int e_sides = plev * 3;

            if (info) return info_damage(1, spell_power(m_sides), spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                dispel_monsters(spell_power(randint1(m_sides) + p_ptr->to_d_spell));
                dispel_evil(spell_power(randint1(e_sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 21:
        if (name) return "The Voice of Saruman";
        if (desc) return "Attempts to slow and sleep all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You start humming a gentle and attractive song...");
            bard_start_singing(spell, MUSIC_SARUMAN);
        }

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cont)
            {
                slow_monsters(power);
                sleep_monsters(power);
            }
        }

        break;

    case 22:
        if (name) return "Song of the Tempest";
        if (desc) return "Fires a beam of sound.";

        {
            int dice = 15 + (plev - 1) / 2;
            int sides = 10;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(
                    GF_SOUND,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 23:
        if (name) return "Ambarkanta";
        if (desc) return "Recreates current dungeon level.";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("You sing of the primeval shaping of Middle-earth...");

                alter_reality();
            }
        }
        break;

    case 24:
        if (name) return "Wrecking Pattern";
        if (desc) return "Shakes dungeon structure, and results in random swapping of floors and walls.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You weave a pattern of sounds to contort and shatter...");
            bard_start_singing(spell, MUSIC_QUAKE);
        }

        {
            int rad = 10;

            if (info) return info_radius(rad);

            if (cont)
            {
                earthquake(py, px, 10);
            }
        }

        break;


    case 25:
        if (name) return "Stationary Shriek";
        if (desc) return "Attempts to freeze all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You weave a very slow pattern which is almost likely to stop...");
            bard_start_singing(spell, MUSIC_STASIS);
        }

        {
            int power = spell_power(plev * 3 + 10);

            if (info) return info_power(power);

            if (cont)
            {
                stasis_monsters(power);
            }
        }

        break;

    case 26:
        if (name) return "Endurance";
        if (desc) return "Sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break glyph.";

        {
            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("The holy power of the Music is creating sacred field...");

                warding_glyph();
            }
        }
        break;

    case 27:
        if (name) return "The Hero's Poem";
        if (desc) return "Hastes you. Gives heroism. Damages all monsters in sight.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("You chant a powerful, heroic call to arms...");
            (void)hp_player(10);

            /* Recalculate hitpoints */
            p_ptr->update |= (PU_HP);

            bard_start_singing(spell, MUSIC_SHERO);
        }

        if (stop)
        {
            if (!p_ptr->hero)
            {
                msg_print("The heroism wears off.");
                /* Recalculate hitpoints */
                p_ptr->update |= (PU_HP);
            }

            if (!p_ptr->fast)
            {
                msg_print("You feel yourself slow down.");
            }
        }

        {
            int dice = 1;
            int sides = plev * 3;

            if (info) return info_damage(dice, sides, spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                dispel_monsters(spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 28:
        if (name) return "Relief of Yavanna";
        if (desc) return "Powerful healing song. Also heals cut and stun completely.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("Life flows through you as you sing the song...");
            bard_start_singing(spell, MUSIC_H_LIFE);
        }

        {
            int dice = spell_power(15);
            int sides = 10;

            if (info) return info_heal(dice, sides, 0);

            if (cont)
            {
                hp_player(damroll(dice, sides));
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }

        break;

    case 29:
        if (name) return "Goddess' rebirth";
        if (desc) return "Restores all stats and experience.";

        {
            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("You strewed light and beauty in the dark as you sing. You feel refreshed.");
                (void)do_res_stat(A_STR);
                (void)do_res_stat(A_INT);
                (void)do_res_stat(A_WIS);
                (void)do_res_stat(A_DEX);
                (void)do_res_stat(A_CON);
                (void)do_res_stat(A_CHR);
                (void)restore_level();
                lp_player(1000);
            }
        }
        break;

    case 30:
        if (name) return "Wizardry of Sauron";
        if (desc) return "Fires an extremely powerful tiny ball of sound.";

        {
            int dice = 50 + plev;
            int sides = 10;
            int rad = 0;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_SOUND,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 31:
        if (name) return "Fingolfin's Challenge";
        if (desc) return "Temporarily makes you invulnerable to most attacks. Consumes an extra turn when the invulnerability ends.";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
                msg_print("You recall the valor of Fingolfin's challenge to the Dark Lord...");

                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

                bard_start_singing(spell, MUSIC_INVULN);
        }

        if (stop)
        {
            if (!p_ptr->invuln)
            {
                msg_print("The invulnerability wears off.");
                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

                /* Take an extra turn */
                p_ptr->energy_need += PY_ENERGY_NEED();
            }
        }

        break;
    }

    return "";
}



/* Hex */
static bool item_tester_hook_weapon_except_bow(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

static cptr do_hex_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
    bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;

    bool add = TRUE;

    int plev = p_ptr->lev;
    int power;

    switch (spell)
    {
    /*** 1st book (0-7) ***/
    case 0:
        if (name) return "Evily blessing";
        if (desc) return "Attempts to increase +to_hit of a weapon and AC";
        if (cast)
        {
            if (!p_ptr->blessed)
            {
                msg_print("You feel righteous!");
            }
        }
        if (stop)
        {
            if (!p_ptr->blessed)
            {
                msg_print("The prayer has expired.");
            }
        }
        break;

    case 1:
        if (name) return "Cure light wounds";
        if (desc) return "Heals cut and HP a little.";
        if (info) return info_heal(1, 10, 0);
        if (cast)
        {
            msg_print("You feel better and better.");
        }
        if (cast || cont)
        {
            hp_player(damroll(1, 10));
            set_cut(p_ptr->cut - 10, TRUE);
        }
        break;

    case 2:
        if (name) return "Demonic aura";
        if (desc) return "Gives fire aura and regeneration.";
        if (cast)
        {
            msg_print("You have enveloped by fiery aura!");
        }
        if (stop)
        {
            msg_print("Fiery aura disappeared.");
        }
        break;

    case 3:
        if (name) return "Stinking mist";
        if (desc) return "Deals few damages of poison to all monsters in your sight.";
        power = plev / 2 + 5 + p_ptr->to_d_spell;
        if (info) return info_damage(1, power, 0);
        if (cast || cont)
        {
            project_hack(GF_POIS, randint1(power));
        }
        break;

    case 4:
        if (name) return "Extra might";
        if (desc) return "Attempts to increase your strength.";
        if (cast)
        {
            msg_print("You feel you get stronger.");
        }
        break;

    case 5:
        if (name) return "Curse weapon";
        if (desc) return "Curses your weapon.";
        if (cast)
        {
            obj_prompt_t prompt = {0};
            char o_name[MAX_NLEN];
            u32b f[OF_ARRAY_SIZE];

            prompt.prompt = "Which weapon do you curse?";
            prompt.error = "You wield no weapons.";
            prompt.filter = item_tester_hook_weapon_except_bow;
            prompt.where[0] = INV_EQUIP;

            obj_prompt(&prompt);
            if (!prompt.obj) return FALSE;

            object_desc(o_name, prompt.obj, OD_NAME_ONLY);
            obj_flags(prompt.obj, f);

            if (!get_check(format("Do you curse %s, really?", o_name))) return FALSE;

            if (!one_in_(3) &&
                (object_is_artifact(prompt.obj) || have_flag(f, OF_BLESSED)))
            {
                msg_format("%s resists the effect.", o_name);
                if (one_in_(3))
                {
                    if (prompt.obj->to_d > 0)
                    {
                        prompt.obj->to_d -= randint1(3) % 2;
                        if (prompt.obj->to_d < 0) prompt.obj->to_d = 0;
                    }
                    if (prompt.obj->to_h > 0)
                    {
                        prompt.obj->to_h -= randint1(3) % 2;
                        if (prompt.obj->to_h < 0) prompt.obj->to_h = 0;
                    }
                    if (prompt.obj->to_a > 0)
                    {
                        prompt.obj->to_a -= randint1(3) % 2;
                        if (prompt.obj->to_a < 0) prompt.obj->to_a = 0;
                    }
                    msg_format("Your %s was disenchanted!", o_name);
                }
            }
            else
            {
                int power = 0;
                msg_format("A terrible black aura blasts your %s!", o_name);
                prompt.obj->curse_flags |= (OFC_CURSED);

                if (object_is_artifact(prompt.obj) || object_is_ego(prompt.obj))
                {

                    if (one_in_(3)) prompt.obj->curse_flags |= (OFC_HEAVY_CURSE);
                    if (one_in_(666))
                    {
                        prompt.obj->curse_flags |= (OFC_TY_CURSE);
                        if (one_in_(666)) prompt.obj->curse_flags |= (OFC_PERMA_CURSE);

                        add_flag(prompt.obj->flags, OF_AGGRAVATE);
                        add_flag(prompt.obj->flags, OF_VORPAL);
                        add_flag(prompt.obj->flags, OF_BRAND_VAMP);
                        msg_print("Blood, Blood, Blood!");
                        power = 2;
                    }
                }

                /* Clouded says getting actual bad curses on Hex objects is annoying,
                 * so we only rarely do it */
                if ((power > 0) || (one_in_(26))) prompt.obj->curse_flags |= get_curse(power, prompt.obj);
                else if (one_in_(2)) prompt.obj->curse_flags |= (OFC_ALLERGY);
            }

            p_ptr->update |= (PU_BONUS);
            add = FALSE;
        }
        break;

    case 6:
        if (name) return "Evil detection";
        if (desc) return "Detects evil monsters.";
        if (info) return info_range(MAX_SIGHT);
        if (cast)
        {
            msg_print("You attend to the presence of evil creatures.");
        }
        break;

    case 7:
        if (name) return "Patience";
        if (desc) return "Bursts hell fire strongly after patients any damage while few turns.";
        power = MIN(200, p_ptr->magic_num1[2] * 2 + p_ptr->to_d_spell);
        if (info) return info_damage(0, 0, power);
        if (cast)
        {
            int a = 3 - (p_ptr->pspeed - 100) / 10;
            int r = 3 + randint1(3) + MAX(0, MIN(3, a));

            if (p_ptr->magic_num2[2] > 0)
            {
                msg_print("You are already patienting.");
                return NULL;
            }

            p_ptr->magic_num2[1] = 1;
            p_ptr->magic_num2[2] = r;
            p_ptr->magic_num1[2] = 0;
            msg_print("You decide to patient all damages.");
            add = FALSE;
        }
        if (cont)
        {
            int rad = 2 + (power / 50);

            p_ptr->magic_num2[2]--;

            if ((p_ptr->magic_num2[2] <= 0) || (power >= 200))
            {
                msg_print("Time for end of patience!");
                if (power)
                {
                    project(0, rad, py, px, power, GF_HELL_FIRE,
                        (PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
                }
                if (p_ptr->wizard || easy_damage)
                {
                    msg_format("You return %d damages.", power);
                }

                /* Reset */
                p_ptr->magic_num2[1] = 0;
                p_ptr->magic_num2[2] = 0;
                p_ptr->magic_num1[2] = 0;
            }
        }
        break;

    /*** 2nd book (8-15) ***/
    case 8:
        if (name) return "Ice armor";
        if (desc) return "Gives cold aura and bonus to AC.";
        if (cast)
        {
            msg_print("You have enveloped by ice armor!");
        }
        if (stop)
        {
            msg_print("Ice armor disappeared.");
        }
        break;

    case 9:
        if (name) return "Cure serious wounds";
        if (desc) return "Heals cut and HP more.";
        if (info) return info_heal(2, 10, 0);
        if (cast)
        {
            msg_print("You feel better and better.");
        }
        if (cast || cont)
        {
            hp_player(damroll(2, 10));
            set_cut((p_ptr->cut / 2) - 10, TRUE);
        }
        break;

    case 10:
        if (name) return "Inhail potion";
        if (desc) return "Quaffs a potion without canceling of casting a spell.";
        if (cast)
        {
            p_ptr->magic_num1[0] |= (1L << HEX_INHAIL);
            do_cmd_quaff_potion();
            p_ptr->magic_num1[0] &= ~(1L << HEX_INHAIL);
            add = FALSE;
        }
        break;

    case 11:
        if (name) return "Vampiric mist";
        if (desc) return "Deals few damages of drain life to all monsters in your sight.";
        power = (plev / 2) + 5 + p_ptr->to_d_spell;
        if (info) return info_damage(1, power, 0);
        if (cast || cont)
        {
            project_hack(GF_OLD_DRAIN, randint1(power));
        }
        break;

    case 12:
        if (name) return "Swords to runeswords";
        if (desc) return "Gives vorpal ability to your weapon. Increases damages by your weapon according to curse of your weapon.";
        if (cast)
        {
            if (p_ptr->weapon_ct > 1)
                msg_print("Your weapons glow bright black.");
            else
                msg_print("Your weapon glows bright black.");
        }
        if (stop)
            msg_format("Brightness of weapon%s disappeared.", (p_ptr->weapon_ct <= 1) ? "" : "s");
        break;

    case 13:
        if (name) return "Touch of confusion";
        if (desc) return "Confuses a monster when you attack.";
        if (cast)
        {
            msg_print("Your hands glow bright red.");
        }
        if (stop)
        {
            msg_print("Brightness on your hands disappeard.");
        }
        break;

    case 14:
        if (name) return "Building up";
        if (desc) return "Attempts to increases your strength, dexterity and constitution.";
        if (cast)
        {
            msg_print("You feel your body is developed more now.");
        }
        break;

    case 15:
        if (name) return "Anti teleport barrier";
        if (desc) return "Obstructs all teleportations by monsters in your sight.";
        power = plev * 3 / 2;
        if (info) return info_power(power);
        if (cast)
        {
            msg_print("You feel anyone can not teleport except you.");
        }
        break;

    /*** 3rd book (16-23) ***/
    case 16:
        if (name) return "Cloak of shock";
        if (desc) return "Gives lightning aura and a bonus to speed.";
        if (cast)
        {
            msg_print("You have enveloped by electrical aura!");
        }
        if (stop)
        {
            msg_print("Electrical aura disappeared.");
        }
        break;

    case 17:
        if (name) return "Cure critical wounds";
        if (desc) return "Heals cut and HP greatly.";
        if (info) return info_heal(4, 10, 0);
        if (cast)
        {
            msg_print("You feel better and better.");
        }
        if (cast || cont)
        {
            hp_player(damroll(4, 10));
            set_stun(0, TRUE);
            set_cut(0, TRUE);
        }
        break;

    case 18:
        if (name) return "Recharging";
        if (desc)
        {
            if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                return "It attempts to recharge a device using your blood for power.";
            else
                return "It attempts to recharge a device using your mana for power.";
        }

        power = plev * 2;
        if (info) return info_power(power);
        if (cast)
        {
            if (!recharge_from_player(power)) return NULL;
            add = FALSE;
        }
        break;

    case 19:
        if (name) return "Animate Dead";
        if (desc) return "Raises corpses and skeletons from dead.";
        if (cast)
        {
            msg_print("You start to call deads.!");
        }
        if (cast || cont)
        {
            animate_dead(0, py, px);
        }
        break;

    case 20:
        if (name) return "Curse armor";
        if (desc) return "Curse a piece of armour that you wielding.";
        if (cast)
        {
            obj_prompt_t prompt = {0};
            char o_name[MAX_NLEN];
            u32b f[OF_ARRAY_SIZE];

            prompt.prompt = "Which piece of armour do you curse?";
            prompt.error = "You wield no piece of armours.";
            prompt.filter = object_is_armour;
            prompt.where[0] = INV_EQUIP;

            obj_prompt(&prompt);
            if (!prompt.obj) return FALSE;

            object_desc(o_name, prompt.obj, OD_NAME_ONLY);
            obj_flags(prompt.obj, f);

            if (!get_check(format("Do you curse %s, really?", o_name))) return FALSE;

            if (!one_in_(3) &&
                (object_is_artifact(prompt.obj) || have_flag(f, OF_BLESSED)))
            {
                msg_format("%s resists the effect.", o_name);
                if (one_in_(3))
                {
                    if (prompt.obj->to_d > 0)
                    {
                        prompt.obj->to_d -= randint1(3) % 2;
                        if (prompt.obj->to_d < 0) prompt.obj->to_d = 0;
                    }
                    if (prompt.obj->to_h > 0)
                    {
                        prompt.obj->to_h -= randint1(3) % 2;
                        if (prompt.obj->to_h < 0) prompt.obj->to_h = 0;
                    }
                    if (prompt.obj->to_a > 0)
                    {
                        prompt.obj->to_a -= randint1(3) % 2;
                        if (prompt.obj->to_a < 0) prompt.obj->to_a = 0;
                    }
                    msg_format("Your %s was disenchanted!", o_name);
                }
            }
            else
            {
                int power = 0;
                msg_format("A terrible black aura blasts your %s!", o_name);
                prompt.obj->curse_flags |= (OFC_CURSED);

                if (object_is_artifact(prompt.obj) || object_is_ego(prompt.obj))
                {

                    if (one_in_(3)) prompt.obj->curse_flags |= (OFC_HEAVY_CURSE);
                    if (one_in_(666))
                    {
                        prompt.obj->curse_flags |= (OFC_TY_CURSE);
                        if (one_in_(666)) prompt.obj->curse_flags |= (OFC_PERMA_CURSE);

                        add_flag(prompt.obj->flags, OF_AGGRAVATE);
                        add_flag(prompt.obj->flags, OF_RES_POIS);
                        add_flag(prompt.obj->flags, OF_RES_DARK);
                        add_flag(prompt.obj->flags, OF_RES_NETHER);
                        msg_print("Blood, Blood, Blood!");
                        power = 2;
                    }
                }

                prompt.obj->curse_flags |= get_curse(power, prompt.obj);
            }

            p_ptr->update |= (PU_BONUS);
            add = FALSE;
        }
        break;

    case 21:
        if (name) return "Cloak of shadow";
        if (desc) return "Gives aura of shadow.";
        if (cast)
        {
            int slot = equip_find_first(object_is_cloak);
            object_type *o_ptr = NULL;

            if (!slot)
            {
                msg_print("You are not wearing a cloak.");
                return NULL;
            }
            o_ptr = equip_obj(slot);
            if (!object_is_cursed(o_ptr))
            {
                msg_print("Your cloak is not cursed.");
                return NULL;
            }
            else
            {
                msg_print("You have enveloped by shadow aura!");
            }
        }
        if (cont)
        {
            int slot = equip_find_first(object_is_cloak);
            if (!slot || !object_is_cursed(equip_obj(slot)))
            {
                do_spell(REALM_HEX, spell, SPELL_STOP);
                p_ptr->magic_num1[0] &= ~(1L << spell);
                p_ptr->magic_num2[0]--;
                if (!p_ptr->magic_num2[0]) set_action(ACTION_NONE);
            }
        }
        if (stop)
        {
            msg_print("Shadow aura disappeared.");
        }
        break;

    case 22:
        if (name) return "Pains to mana";
        if (desc) return "Deals psychic damages to all monsters in sight, and drains some mana.";
        power = plev * 3 / 2 + p_ptr->to_d_spell;
        if (info) return info_damage(1, power, 0);
        if (cast || cont)
        {
            project_hack(GF_PSI_DRAIN, randint1(power));
        }
        break;

    case 23:
        if (name) return "Eye for an eye";
        if (desc) return "Returns same damage which you got to the monster which damaged you.";
        if (cast)
        {
            msg_print("You wish strongly you want to revenge anything.");
        }
        break;

    /*** 4th book (24-31) ***/
    case 24:
        if (name) return "Anti multiply barrier";
        if (desc) return "Obstructs all multiplying by monsters in entire floor.";
        if (cast)
        {
            msg_print("You feel anyone can not already multiply.");
        }
        break;

    case 25:
        if (name) return "Restore life";
        if (desc) return "Restores life energy and status.";
        if (cast)
        {
            msg_print("You feel your life energy starting to return.");
        }
        if (cast || cont)
        {
            bool flag = FALSE;
            int d = (p_ptr->max_exp - p_ptr->exp);
            int r = (p_ptr->exp / 20);
            int l = (1000 - p_ptr->clp);
            int i;

            if (d > 0)
            {
                if (d < r)
                    p_ptr->exp = p_ptr->max_exp;
                else
                    p_ptr->exp += r;

                /* Check the experience */
                check_experience();

                flag = TRUE;
            }
            if (l > 0)
            {
                lp_player(MIN(l, 15));
                flag = TRUE;
            }
            for (i = A_STR; i < 6; i ++)
            {
                if (p_ptr->stat_cur[i] < p_ptr->stat_max[i])
                {
                    if (p_ptr->stat_cur[i] < 18)
                        p_ptr->stat_cur[i]++;
                    else
                        p_ptr->stat_cur[i] += 10;

                    if (p_ptr->stat_cur[i] > p_ptr->stat_max[i])
                        p_ptr->stat_cur[i] = p_ptr->stat_max[i];

                    /* Recalculate bonuses */
                    p_ptr->update |= (PU_BONUS);

                    flag = TRUE;
                }
            }

            if (!flag)
            {
                msg_format("Finish casting '%^s'.", do_spell(REALM_HEX, HEX_RESTORE, SPELL_NAME));
                p_ptr->magic_num1[0] &= ~(1L << HEX_RESTORE);
                if (cont) p_ptr->magic_num2[0]--;
                if (!p_ptr->magic_num2[0]) set_action(ACTION_NONE);

                /* Redraw status */
                p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
                p_ptr->redraw |= (PR_EXTRA);

                return "";
            }
        }
        break;

    case 26:
        if (name) return "Drain curse power";
        if (desc) return "Drains curse on your weapon and heals SP a little.";
        if (cast)
        {
            obj_prompt_t prompt = {0};
            u32b f[OF_ARRAY_SIZE];

            prompt.prompt = "Which cursed equipment do you drain mana from?";
            prompt.error = "You have no cursed equipment.";
            prompt.filter = object_is_cursed;
            prompt.where[0] = INV_EQUIP;

            obj_prompt(&prompt);
            if (!prompt.obj) return FALSE;

            obj_flags(prompt.obj, f);

            p_ptr->csp += (plev / 5) + randint1(plev / 5);
            if (have_flag(f, OF_TY_CURSE) || (prompt.obj->curse_flags & OFC_TY_CURSE)) p_ptr->csp += randint1(5);
            if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;

            if (prompt.obj->curse_flags & OFC_PERMA_CURSE)
            {
                /* Nothing */
            }
            else if (prompt.obj->curse_flags & OFC_HEAVY_CURSE)
            {
                if (one_in_(7))
                {
                    msg_print("Heavy curse vanished away.");
                    prompt.obj->curse_flags = 0L;
                    prompt.obj->known_curse_flags = 0L;
                }
            }
            else if ((prompt.obj->curse_flags & (OFC_CURSED)) && one_in_(3))
            {
                msg_print("Curse vanished away.");
                prompt.obj->curse_flags = 0L;
                prompt.obj->known_curse_flags = 0L;
            }

            add = FALSE;
        }
        break;

    case 27:
        if (name) return "Swords to vampires";
        if (desc) return "Gives vampiric ability to your weapon.";
        if (cast)
        {
            if (p_ptr->weapon_ct > 1)
                msg_print("Your weapons want more blood now.");
            else
                msg_print("Your weapon wants more blood now.");
        }
        if (stop)
            msg_format("Thirsty of weapon%s disappeared.", (p_ptr->weapon_ct <= 1) ? "" : "s");
        break;

    case 28:
        if (name) return "Word of stun";
        if (desc) return "Stuns all monsters in your sight.";
        if (cast || cont)
        {
            stun_monsters(5 + plev/5);
        }
        break;

    case 29:
        if (name) return "Moving into shadow";
        if (desc) return "Teleports you close to a monster.";
        if (cast)
        {
            int i, y, x, dir;
            bool flag;

            for (i = 0; i < 3; i++)
            {
                if (!tgt_pt(&x, &y, plev+2)) return FALSE;

                flag = FALSE;

                for (dir = 0; dir < 8; dir++)
                {
                    int dy = y + ddy_ddd[dir];
                    int dx = x + ddx_ddd[dir];
                    if (dir == 5) continue;
                    if(cave[dy][dx].m_idx) flag = TRUE;
                }

                if (!cave_empty_bold(y, x) || (cave[y][x].info & CAVE_ICKY) ||
                    (distance(y, x, py, px) > plev + 2))
                {
                    msg_print("Can not teleport to there.");
                    continue;
                }
                break;
            }

            if (flag && randint0(plev * plev / 2))
            {
                teleport_player_to(y, x, 0L);
            }
            else
            {
                msg_print("Oops!");
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(30, 0L);
            }

            add = FALSE;
        }
        break;

    case 30:
        if (name) return "Anti magic barrier";
        if (desc) return "Obstructs all magic spell of monsters in your sight.";
        power = plev * 3 / 2;
        if (info) return info_power(power);
        if (cast)
        {
            msg_print("You feel anyone can not cast spells except you.");
        }
        break;

    case 31:
        if (name) return "Revenge sentence";
        if (desc) return "Fires  a ball of hell fire to try revenging after few turns.";
        power = p_ptr->magic_num1[2] + p_ptr->to_d_spell;
        if (info) return info_damage(0, 0, power);
        if (cast)
        {
            int r;
            int a = 3 - (p_ptr->pspeed - 100) / 10;
            r = 3 + randint1(2) + MAX(0, MIN(3, a));

            if (p_ptr->magic_num2[2] > 0)
            {
                msg_print("You already pronounced your revenge.");
                return NULL;
            }

            p_ptr->magic_num2[1] = 2;
            p_ptr->magic_num2[2] = r;
            msg_format("You pronounce your revenge. %d turns left.", r - 1);
            add = FALSE;
        }
        if (cont)
        {
            p_ptr->magic_num2[2]--;

            if (p_ptr->magic_num2[2] <= 0)
            {
                int dir;

                if (power)
                {
                    command_dir = 0;

                    do
                    {
                        msg_print("Time to revenge!");
                    }
                    while (!get_fire_dir(&dir));

                    fire_ball(GF_HELL_FIRE, dir, power, 1);

                    if (p_ptr->wizard || easy_damage)
                    {
                        msg_format("You return %d damages.", power);
                    }
                }
                else
                {
                    msg_print("You are not in a mood for revenge.");
                }
                p_ptr->magic_num1[2] = 0;
            }
        }
        break;
    }

    /* start casting */
    if ((cast) && (add))
    {
        /* add spell */
        p_ptr->magic_num1[0] |= 1L << (spell);
        p_ptr->magic_num2[0]++;

        if (p_ptr->action != ACTION_SPELL) set_action(ACTION_SPELL);
    }

    /* Redraw status */
    if (!info)
    {
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
        p_ptr->redraw |= (PR_EXTRA | PR_HP | PR_MANA);
    }

    return "";
}

static cptr do_armageddon_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int plev = p_ptr->lev;
    int dir;

    switch (spell)
    {
    /* Book of Elements */
    case 0:
        if (name) return "Lightning Bolt";
        if (desc) return "Fires a bolt or beam of electricity.";

        {
            int dice = 3 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_ELEC,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 1:
        if (name) return "Frost Bolt";
        if (desc) return "Fires a bolt or beam of cold.";

        {
            int dice = 4 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_COLD,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 2:
        if (name) return "Fire Bolt";
        if (desc) return "Fires a bolt or beam of fire.";

        {
            int dice = 5 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 3:
        if (name) return "Acid Bolt";
        if (desc) return "Fires a bolt or beam of acid.";

        {
            int dice = 5 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_ACID,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 4:
        if (name) return "Lightning Ball";
        if (desc) return "Fires a ball of electricity.";

        {
            int dam = spell_power(3*plev/2 + 20 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ELEC, dir, dam, rad);
            }
        }
        break;
    case 5:
        if (name) return "Frost Ball";
        if (desc) return "Fires a ball of cold.";

        {
            int dam = spell_power(3*plev/2 + 25 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_COLD, dir, dam, rad);
            }
        }
        break;
    case 6:
        if (name) return "Fire Ball";
        if (desc) return "Fires a ball of fire.";

        {
            int dam = spell_power(3*plev/2 + 30 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;
    case 7:
        if (name) return "Acid Ball";
        if (desc) return "Fires a ball of acid.";

        {
            int dam = spell_power(3*plev/2 + 35 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ACID, dir, dam, rad);
            }
        }
        break;

    /* Earth, Wind and Fire */
    case 8:
        if (name) return "Shard Bolt";
        if (desc) return "Fires a bolt or beam of shards.";

        {
            int dice = 7 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_SHARDS,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 9:
        if (name) return "Gravity Bolt";
        if (desc) return "Fires a bolt or beam of gravity.";

        {
            int dice = 5 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_GRAVITY,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 10:
        if (name) return "Plasma Bolt";
        if (desc) return "Fires a bolt or beam of plasma.";

        {
            int dice = 11 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_PLASMA,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 11:
        if (name) return "Meteor";
        if (desc) return "Fires a meteor.";

        {
            int dam = spell_power(plev + 60 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_METEOR, dir, dam, rad);
            }
        }
        break;
    case 12:
        if (name) return "Thunderclap";
        if (desc) return "Generates a ball of sound centered on you.";

        {
            int dam = spell_power((40 + plev + p_ptr->to_d_spell)*2);
            int rad = plev / 10 + 2;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                msg_print("BOOM!");
                project(0, rad, py, px, dam, GF_SOUND, PROJECT_KILL | PROJECT_ITEM);
            }
        }
        break;

    case 13:
        if (name) return "Windblast";
        if (desc) return "Fires a microburst of strong winds.";

        {
            int dam = spell_power(plev + 40 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_TELEKINESIS, dir, dam, rad);
            }
        }
        break;
    case 14:
        if (name) return "Hellstorm";
        if (desc) return "Generates a huge ball of fire centered on you.";

        {
            int dam = spell_power((6 * plev + p_ptr->to_d_spell)*2);
            int rad = 8;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
                fire_ball(GF_FIRE, 0, dam, rad);
        }
        break;
    case 15:
        if (name) return "Rocket";
        if (desc) return "Fires a rocket.";

        {
            int dam = spell_power(60 + plev * 4 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                msg_print("You launch a rocket!");
                fire_rocket(GF_ROCKET, dir, dam, rad);
            }
        }
        break;

    /* Path of Destruction */
    case 16:
        if (name) return "Ice Bolt";
        if (desc) return "Fires a bolt of ice.";

        {
            int dice = 5 + plev/4;
            int sides = 15;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_ICE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 17:
        if (name) return "Water Ball";
        if (desc) return "Fires a ball of water.";

        {
            int dam = spell_power(2*plev + 30 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_WATER, dir, dam, rad);
            }
        }
        break;
    case 18:
        if (name) return "Breathe Lightning";
        if (desc) return "Breathes a cone of electricity at chosen target.";

        {
            int dam = spell_power(9*plev/2 + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ELEC, dir, dam, rad);
            }
        }
        break;
    case 19:
        if (name) return "Breathe Frost";
        if (desc) return "Breathes a cone of cold at chosen target.";

        {
            int dam = spell_power(9*plev/2 + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_COLD, dir, dam, rad);
            }
        }
        break;
    case 20:
        if (name) return "Breathe Fire";
        if (desc) return "Breathes a cone of fire at chosen target.";

        {
            int dam = spell_power(5*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;
    case 21:
        if (name) return "Breathe Acid";
        if (desc) return "Breathes a cone of acid at chosen target.";

        {
            int dam = spell_power(5*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ACID, dir, dam, rad);
            }
        }
        break;
    case 22:
        if (name) return "Breathe Plasma";
        if (desc) return "Breathes a cone of plasma at chosen target.";

        {
            int dam = spell_power(11*plev/2 + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_PLASMA, dir, dam, rad);
            }
        }
        break;
    case 23:
        if (name) return "Breathe Gravity";
        if (desc) return "Breathes a cone of gravity at chosen target.";

        {
            int dam = spell_power(4*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_GRAVITY, dir, dam, rad);
            }
        }
        break;

    /* Day of Ragnarok */
    case 24:
        if (name) return "Mana Bolt";
        if (desc) return "Fires a bolt of mana.";

        {
            int dice = 1;
            int sides = 5*plev;

            if (info) return info_damage(dice, spell_power(sides), spell_power(50 + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_MANA,
                    dir,
                    spell_power(damroll(dice, sides) + 50 + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 25:
        if (name) return "Plasma Ball";
        if (desc) return "Fires a ball of plasma.";

        {
            int dam = spell_power(2*plev + 90 + p_ptr->to_d_spell);
            int rad = 3;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_PLASMA, dir, dam, rad);
            }
        }
        break;
    case 26:
        if (name) return "Mana Ball";
        if (desc) return "Fires a ball of pure mana.";

        {
            int dam = spell_power(4*plev + 100 + p_ptr->to_d_spell);
            int rad = 3;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_MANA, dir, dam, rad);
            }
        }
        break;
    case 27:
        if (name) return "Breathe Sound";
        if (desc) return "Breathes a cone of sound at chosen target.";

        {
            int dam = spell_power(6*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_SOUND, dir, dam, rad);
            }
        }
        break;
    case 28:
        if (name) return "Breathe Inertia";
        if (desc) return "Breathes a cone of inertia at chosen target.";

        {
            int dam = spell_power(5*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_INERT, dir, dam, rad);
            }
        }
        break;
    case 29:
        if (name) return "Breathe Disintegration";
        if (desc) return "Breathes a cone of disintegration at chosen target.";

        {
            int dam = spell_power(7*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir_aux(&dir, TARGET_DISI)) return NULL;
                fire_ball(GF_DISINTEGRATE, dir, dam, rad);
            }
        }
        break;
    case 30:
        if (name) return "Breathe Mana";
        if (desc) return "Breathes a cone of mana at chosen target.";

        {
            int dam = spell_power(9*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_MANA, dir, dam, rad);
            }
        }
        break;
    case 31:
        if (name) return "Breathe Shards";
        if (desc) return "Breathes a cone of shards at chosen target.";

        {
            int dam = spell_power(10*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_SHARDS, dir, dam, rad);
            }
        }
        break;
    }
    return "";
}
/*
 * Do everything for each spell
 */
cptr do_spell(int realm, int spell, int mode)
{
    cptr result = NULL;

    _current_realm_hack = realm;

    switch (realm)
    {
    case REALM_LIFE:     result = do_life_spell(spell, mode); break;
    case REALM_SORCERY:  result = do_sorcery_spell(spell, mode); break;
    case REALM_NATURE:   result = do_nature_spell(spell, mode); break;
    case REALM_CHAOS:    result = do_chaos_spell(spell, mode); break;
    case REALM_DEATH:    result = do_death_spell(spell, mode); break;
    case REALM_TRUMP:    result = do_trump_spell(spell, mode); break;
    case REALM_ARCANE:   result = do_arcane_spell(spell, mode); break;
    case REALM_CRAFT:    result = do_craft_spell(spell, mode); break;
    case REALM_DAEMON:   result = do_daemon_spell(spell, mode); break;
    case REALM_CRUSADE:  result = do_crusade_spell(spell, mode); break;
    case REALM_LAW:      result = do_law_spell(spell, mode); break;
    case REALM_MUSIC:    result = do_music_spell(spell, mode); break;
    case REALM_HISSATSU: result = do_hissatsu_spell(spell, mode); break;
    case REALM_HEX:      result = do_hex_spell(spell, mode); break;
    case REALM_NECROMANCY: result = do_necromancy_spell(spell, mode); break;
    case REALM_ARMAGEDDON: result = do_armageddon_spell(spell, mode); break;
    case REALM_BURGLARY: result = do_burglary_spell(spell, mode); break;
    }

    _current_realm_hack = 0;
    return result;
}

int get_realm_idx(cptr name)
{
    int i;
    for (i = 0; i < MAX_REALM; i++)
    {
        if (strcmp(name, realm_names[i]) == 0)
            return i;
    }
    return -1;
}
