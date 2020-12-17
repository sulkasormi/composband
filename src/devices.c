#include "angband.h"

#include <assert.h>

static bool ang_sort_comp_pet(vptr u, vptr v, int a, int b)
{
    u16b *who = (u16b*)(u);

    int w1 = who[a];
    int w2 = who[b];

    monster_type *m_ptr1 = &m_list[w1];
    monster_type *m_ptr2 = &m_list[w2];
    monster_race *r_ptr1 = &r_info[m_ptr1->r_idx];
    monster_race *r_ptr2 = &r_info[m_ptr2->r_idx];

    /* Unused */
    (void)v;

    if (m_ptr1->nickname && !m_ptr2->nickname) return TRUE;
    if (m_ptr2->nickname && !m_ptr1->nickname) return FALSE;

    if ((r_ptr1->flags1 & RF1_UNIQUE) && !(r_ptr2->flags1 & RF1_UNIQUE)) return TRUE;
    if ((r_ptr2->flags1 & RF1_UNIQUE) && !(r_ptr1->flags1 & RF1_UNIQUE)) return FALSE;

    if (r_ptr1->level > r_ptr2->level) return TRUE;
    if (r_ptr2->level > r_ptr1->level) return FALSE;

    if (m_ptr1->hp > m_ptr2->hp) return TRUE;
    if (m_ptr2->hp > m_ptr1->hp) return FALSE;

    return w1 <= w2;
}

bool class_uses_spell_scrolls(int mika)
{
    if (mika == CLASS_WARRIOR ||
      mika == CLASS_MINDCRAFTER ||
      mika == CLASS_PSION ||
      mika == CLASS_SORCERER ||
      mika == CLASS_ARCHER ||
      mika == CLASS_MAGIC_EATER ||
      mika == CLASS_DEVICEMASTER ||
      mika == CLASS_RED_MAGE ||
      mika == CLASS_SAMURAI ||
      mika == CLASS_CAVALRY ||
      mika == CLASS_BERSERKER ||
      mika == CLASS_WEAPONSMITH ||
      mika == CLASS_MIRROR_MASTER ||
      mika == CLASS_TIME_LORD ||
      mika == CLASS_BLOOD_KNIGHT ||
      mika == CLASS_WARLOCK ||
      mika == CLASS_ARCHAEOLOGIST ||
      mika == CLASS_DUELIST ||
      mika == CLASS_RUNE_KNIGHT ||
      mika == CLASS_WILD_TALENT ||
      mika == CLASS_BLUE_MAGE ||
      mika == CLASS_NINJA ||
      mika == CLASS_NINJA_LAWYER ||
      mika == CLASS_SCOUT ||
      mika == CLASS_MYSTIC ||
      mika == CLASS_MAULER ||
      mika == CLASS_POLITICIAN ||
      mika == CLASS_ALCHEMIST ||
      mika == CLASS_DISCIPLE ||
      mika == CLASS_SKILLMASTER )
        return FALSE;
    return TRUE;
}

/* Devices: We are following the do_spell() pattern which is quick and dirty,
   but not my preferred approach ... */

/* Fail Rates ... Scaled by 10 (95.2% returned as 952)
 * cf design/devices.ods */
int _difficulty(int d)
{
    /* A non-linear difficulty protects the high level end game
     * devices from inappropriate usage. -Rockets are now much
     * harder to use (and _Healing only marginally more difficult).
     * Formerly, OF_MAGIC_MASTERY was useless to device classes
     * like the mage. No longer! 100 -> 130, but with cubic weighting.
     * Two versions: The first is more punishing than the second, depending
     * on the chosen cutoff. As usual, see design/devices.ods or ^A"2.
    return d + 30 * d * d / 100 * d / 10000; */
    int cutoff = 40;
    int xtra   = 30;
    if (d > cutoff)
    {
        int l = d - cutoff;
        int m = 100 - cutoff;
        assert(cutoff < 100);
        return d + xtra * l * l / m * l / (m * m);
    }
    return d;
}
/* in progress: I find this calculation hard to grok ... let's
 * rephrase in terms of skill vs. difficulty and expose a simple
 * api so I can view fail(s,d) (a function of 2 variables).
 * cf ^A"2 for online spoiler tables (wizard1.c) */
int device_calc_fail_rate_aux(int skill, int difficulty)
{
    int min = USE_DEVICE;
    int fail = 0;
    difficulty = _difficulty(difficulty);
    if (skill > difficulty) difficulty -= (skill - difficulty)*2;
    else skill -= (difficulty - skill)*2;
    if (difficulty < min) difficulty = min;
    if (skill < min) skill = min;
    if (skill > difficulty)
        fail = difficulty * 500 / skill;
    else
        fail = 1000 - skill * 500 / difficulty;
    return fail;
}

/* XXX design is sloppy atm ... I want devicemasters to have a skill
 * boost with their speciality device type (e.g. wands), and less
 * magic skills overall. But the "effect" api layer has no info about
 * the obj->tval ... so we'll need to pass that along. The "effect" layer
 * is public and is also used for equipment with activations. */
static int effect_calc_fail_rate_aux(effect_t *effect, int skill_boost)
{
    int skill = p_ptr->skills.dev + skill_boost;
    int fail;

    if (p_ptr->pclass == CLASS_BERSERKER) return 1000;
    if (beorning_is_(BEORNING_FORM_BEAR)) return 1000;

    if (p_ptr->confused) skill = 3 * skill / 4;

    fail = device_calc_fail_rate_aux(skill, effect->difficulty);
    if ((p_ptr->stun) && (fail < 950))
    {
        fail += 500 * p_ptr->stun / 100;
        if (fail > 950) fail = 950;
    }
    return fail;
}

int effect_calc_fail_rate(effect_t *effect)
{
    return effect_calc_fail_rate_aux(effect, 0);
}

int device_calc_fail_rate(object_type *o_ptr)
{
    int lev, chance, fail;

    if (o_ptr->activation.type)
    {
        effect_t effect = o_ptr->activation;
        u32b     flgs[OF_ARRAY_SIZE];
        int      skill_boost = 0;

        if (devicemaster_is_speciality(o_ptr))
            skill_boost = 5 + 3*p_ptr->lev/5; /* 40+15 base = 115 -> 150 */

        if ((mut_present(MUT_IMPOTENCE)) && (o_ptr->tval == TV_STAFF || o_ptr->tval == TV_ROD))
        {
            skill_boost -= 10;
            if (effect.type == EFFECT_SPEED || effect.type == EFFECT_SPEED_HERO || effect.type == EFFECT_BALL_FIRE || o_ptr->name2 == EGO_DEVICE_QUICKNESS)
                skill_boost -= 20;
        }

        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, OF_EASY_SPELL))
        {
            int d = effect.difficulty;

            d -= MAX(o_ptr->pval, effect.difficulty * 10 * o_ptr->pval / 300);
            if (d <= 1) d = 1;
            effect.difficulty = d;
        }

        if (o_ptr->curse_flags & OFC_CURSED)
            effect.difficulty += effect.difficulty / 5;

        return effect_calc_fail_rate_aux(&effect, skill_boost);
    }
    if (p_ptr->pclass == CLASS_BERSERKER) return 1000;
    if (beorning_is_(BEORNING_FORM_BEAR)) return 1000;

    lev = k_info[o_ptr->k_idx].level;
    if (lev > 50) lev = 50 + (lev - 50)/2;
    chance = p_ptr->skills.dev;
    if (p_ptr->confused) chance = chance / 2;
    chance = chance - lev;
    if (chance < USE_DEVICE)
        fail = 1000 - 1000/(3 * (USE_DEVICE - chance + 1));
    else
        fail = (USE_DEVICE-1)*1000/chance;

    if ((p_ptr->stun) && (fail < 950))
    {
        fail += 500 * p_ptr->stun / 100;
        if (fail > 950) fail = 950;
    }
    if (o_ptr->tval == TV_SCROLL && fail > 500) fail = 500;
    return fail;
}

/* Hack: When using an unknown rod we force the user to target. Also
   Trap Location should not spoil with the view_unsafe_grids option. */
bool device_known = FALSE;

/* Hack: Allow player to learn device power thru actual use. They can
 * also learn the fail rate (i.e. difficulty) by failing enough times,
 * but that is handled elsewhere. We deal solely with OFL_DEVICE_POWER. */
bool device_lore = FALSE;

/* Hack: When using an unknown device, was there an observable effect?
   If so, identify the device. */
bool device_noticed = FALSE;

/* Hack for Device Master's desperation. This power uses all the charges
   in a device at once (with diminishing returns) and potentially destroys
   the device as well. */
int  device_extra_power = 0;

/* Hack for identifying all relevant objects in a single action.
   It's ugly, but worthwhile! */
int  device_available_charges = 0; /* How many can we do? */
int  device_used_charges = 0;      /* How many did we do? */
static bool _use_charges = FALSE;
static bool _multi_charge_lock = FALSE;

static void _do_identify_aux(obj_ptr obj)
{
    char name[MAX_NLEN];
    bool old_known;

    if ((_use_charges) && (device_used_charges >= device_available_charges)) return;

    old_known = identify_item(obj);
    object_desc(name, obj, OD_COLOR_CODED);
    switch (obj->loc.where)
    {
    case INV_EQUIP:
        msg_format("%^s: %s (%c).", equip_describe_slot(obj->loc.slot),
                name, slot_label(obj->loc.slot));
        break;
/* case INV_PACK:
        msg_format("In your pack: %s.", name);
        break;
    case INV_QUIVER:
        msg_format("In your quiver: %s.", name);
        break;*/
    case INV_PACK:
    case INV_QUIVER:
        obj->marked |= OM_DELAYED_MSG;
        p_ptr->notice |= PN_CARRY;
        break;
    case INV_FLOOR:
        msg_format("On the ground: %s.", name);
        break;
    }
    autopick_alter_obj(obj, destroy_identify && !old_known);
    obj_release(obj, OBJ_RELEASE_ID | OBJ_RELEASE_QUIET);
    if (_use_charges) device_used_charges++;
}

void mass_identify(bool use_charges) /* shared with Sorcery spell */
{
    inv_ptr floor = inv_filter_floor(point(px, py), obj_exists);

    _use_charges = use_charges;
    _multi_charge_lock = TRUE;
    pack_for_each_that(_do_identify_aux, obj_is_unknown);
    equip_for_each_that(_do_identify_aux, obj_is_unknown);
    quiver_for_each_that(_do_identify_aux, obj_is_unknown);
    inv_for_each_that(floor, _do_identify_aux, obj_is_unknown);
    _multi_charge_lock = FALSE;

    inv_free(floor);
}

static int _cmd_handler(obj_prompt_context_ptr context, int cmd)
{
    if (cmd == '*')
        return OP_CMD_DISMISS;
    return OP_CMD_SKIPPED;
}

static bool _do_identify(void)
{
    obj_prompt_t prompt = {0};

    assert(device_used_charges == 0);

    prompt.prompt = "Identify which item <color:w>(<color:keypress>*</color> for all)</color>?";
    prompt.error = "All items are identified.";
    prompt.filter = obj_is_unknown;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;
    prompt.cmd_handler = _cmd_handler;
    obj_prompt_add_special_packs(&prompt);

    switch (obj_prompt(&prompt))
    {
    case OP_CUSTOM:
        mass_identify(TRUE);
        return TRUE;
    case OP_SUCCESS:
        _use_charges = TRUE;
        _do_identify_aux(prompt.obj);
        return TRUE;
    }
    return FALSE;
}

/* Using Devices
      if (!device_try(o_ptr)) ... "You failed to use the device" ...
      if (device_use(o_ptr)) ... Decrement Charges/Unstack/Etc. ...
*/
bool device_try(object_type *o_ptr)
{
    int fail = device_calc_fail_rate(o_ptr);
    if (randint0(1000) < fail)
        return FALSE;
    return TRUE;
}

bool device_use(object_type *o_ptr, int boost)
{
    device_known = object_is_known(o_ptr);
    if (do_device(o_ptr, SPELL_CAST, boost))
        return TRUE;
    return FALSE;
}

static int _scroll_power(int val)
{
    if (devicemaster_is_(DEVICEMASTER_SCROLLS))
    {
        val += val * device_extra_power / 100;
        return device_power_aux(val, /*p_ptr->device_power + */p_ptr->lev/10);
    }
    return val;
}

static int _potion_power(int val)
{
    if (devicemaster_is_(DEVICEMASTER_POTIONS))
    {
        val += val * device_extra_power / 100;
        return device_power_aux(val, /*p_ptr->device_power + */p_ptr->lev/10);
    }
    return val;
}

static cptr _do_potion(int sval, int mode)
{
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    switch (sval)
    {
    case SV_POTION_WATER:
        if (desc) return "It is just water.";
        if (cast)
        {
            msg_print("You feel less thirsty.");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_APPLE_JUICE:
        if (desc) return "It tastes sweet.";
        if (cast)
        {
            msg_print("You feel less thirsty.");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SLIME_MOLD:
        if (desc) return "It tastes weird.";
        if (cast)
        {
            msg_print("You feel less thirsty.");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SLOWNESS:
        if (desc) return "It slows you down temporarily when you quaff it.";
        if (cast)
        {
            if (set_slow(randint1(25) + 15, FALSE))
                device_noticed = TRUE;
        }
        break;
    case SV_POTION_SALT_WATER:
        if (desc) return "It makes you nearly faint from hunger and paralyzes you, but it cures poison when you quaff it.";
        if (cast)
        {
            if (( !(get_race()->flags & RACE_IS_NONLIVING)
              && !prace_is_(RACE_MON_JELLY) ) || prace_is_(RACE_EINHERI))
            {
                msg_print("The potion makes you vomit!");
                set_food(PY_FOOD_STARVE - 1);
                set_paralyzed(randint1(4), FALSE);
                set_poisoned(0, TRUE);
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_POISON:
        if (desc) return "It poisons you when you quaff it.";
        if (cast)
        {
            if (!res_save_default(RES_POIS))
            {
                if (set_poisoned(p_ptr->poisoned + randint0(15) + 10, FALSE))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_BLINDNESS:
        if (desc) return "It blinds you when you quaff it.";
        if (cast)
        {
            if (!res_save_default(RES_BLIND))
            {
                if (set_blind(p_ptr->blind + randint0(100) + 100, FALSE))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_CONFUSION: /* Booze */
        if (desc) return "It confuses and hallucinates you when you quaff it. If you are a monk, you may be a drunken master.";
        if (cast)
        {
            if (p_ptr->pclass != CLASS_MONK)
                virtue_add(VIRTUE_HARMONY, -1);
            if (!res_save_default(RES_CONF))
            {
                if (p_ptr->pclass == CLASS_MONK)
                    p_ptr->special_attack |= ATTACK_SUIKEN;
                if (set_confused(randint0(20) + 15, FALSE))
                    device_noticed = TRUE;
            }

            if (!res_save_default(RES_CHAOS))
            {
                if (one_in_(2))
                {
                    if (set_image(p_ptr->image + randint0(25) + 25, FALSE))
                        device_noticed = TRUE;
                }
                if (one_in_(13) && (p_ptr->pclass != CLASS_MONK))
                {
                    device_noticed = TRUE;
                    if (one_in_(3)) lose_all_info();
                    else wiz_dark();
                    teleport_player_aux(100, TELEPORT_NONMAGICAL | TELEPORT_PASSIVE);
                    wiz_dark();
                    msg_print("You wake up somewhere with a sore head...");
                    msg_print("You can't remember a thing, or how you got here!");
                }
            }
        }
        break;
    case SV_POTION_SLEEP:
        if (desc) return "It paralyzes you when you quaff it.";
        if (cast)
        {
            if (!free_act_save_p(0))
            {
                msg_print("You fall asleep.");

                if (ironman_nightmare)
                {
                    msg_print("A horrible vision enters your mind.");
                    get_mon_num_prep(get_nightmare, NULL);
                    have_nightmare(get_mon_num(MAX_DEPTH));
                    get_mon_num_prep(NULL, NULL);
                }
                if (set_paralyzed(randint1(4), FALSE))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case SV_POTION_LIQUID_LOGRUS:
        if (desc) return "It has unpredictable effects when you quaff it, either "
                         "1) curing 5000 HP and all temporary ailments, "
                         "2) increasing all stats, 3) decreasing all stats, "
                         "4) providing temporary light speed, "
                         "5) temporarily turning your skin to stone, "
                         "6) giving or curing mutations randomly, "
                         "7) repeatedly giving or curing mutations randomly, "
                         "8) temporarily making you gigantic, "
                         "9) triggering the Baby Foul Curse, "
                         "10) temporarily polymorphing you into an Ent, "
                         "11) making you hallucinate temporarily, "
                         "12) curing 420 HP and all ailments but making you "
                         "hallucinate temporarily, "
                         "or 13) exploding in your mouth for 11d66 damage, "
                         "massive stunning and a dangerous wound.";
        if (cast)
        {
            int noppa = randint1(13);
            if (p_ptr->good_luck) noppa--;
            else if (mut_present(MUT_BAD_LUCK)) noppa++;
            switch (noppa)
            {
                case 0:
                case 1:
                    virtue_add(VIRTUE_CHANCE, 5);
                    (void)_do_potion(SV_POTION_LIFE, SPELL_CAST);
                     break;
                case 2:
                    virtue_add(VIRTUE_CHANCE, 5);
                    (void)_do_potion(SV_POTION_AUGMENTATION, SPELL_CAST);
                    break;
                case 3:
                    virtue_add(VIRTUE_CHANCE, -5);
                    msg_print("You feel less powerful!");
                    dec_stat(A_DEX, 10, TRUE);
                    dec_stat(A_WIS, 10, TRUE);
                    dec_stat(A_CON, 10, TRUE);
                    dec_stat(A_STR, 10, TRUE);
                    dec_stat(A_CHR, 10, TRUE);
                    dec_stat(A_INT, 10, TRUE);
                    break;
                case 4:
                    virtue_add(VIRTUE_CHANCE, 2);
                    set_lightspeed(_potion_power(25 + randint1(50)), FALSE);
                    break;
                case 5:
                    (void)_do_potion(SV_POTION_STONE_SKIN, SPELL_CAST);
                    break;
                case 6:
                case 7:
                {
                    int ii, ct = 1;
                    if (noppa == 7) ct += randint1(3);
                    for (ii = 0; ii < ct; ii++)
                    {
                        (void)_do_potion(SV_POTION_POLYMORPH, SPELL_CAST);
                    }
                    break;
                }
                case 8:
                    (void)_do_potion(SV_POTION_GIANT_STRENGTH, SPELL_CAST);
                    break;
                case 9:
                    virtue_add(VIRTUE_CHANCE, -2);
                    nonlethal_ty_substitute(TRUE);
                    break;
                case 10:
                    if (player_obviously_poly_immune(TRUE) || mut_present(MUT_DRACONIAN_METAMORPHOSIS))
                        nonlethal_ty_substitute(TRUE);
                    else
                        set_mimic(20 + randint1(20), RACE_ENT, TRUE);
                    break;
                case 11:
                    set_image(50 + randint1(50), TRUE);
                    break;
                case 12:
                    virtue_add(VIRTUE_CHANCE, 1);
                    virtue_add(VIRTUE_VITALITY, 1);
                    virtue_add(VIRTUE_UNLIFE, -5);
                    msg_print("You feel life flow through your body!");
                    restore_level();
                    lp_player(1000);
                    set_poisoned(0, TRUE);
                    set_blind(0, TRUE);
                    set_confused(0, TRUE);
                    set_image(0, TRUE);
                    set_stun(0, TRUE);
                    set_cut(0, TRUE);
                    set_unwell(0, TRUE);
                    do_res_stat(A_STR);
                    do_res_stat(A_CON);
                    do_res_stat(A_DEX);
                    do_res_stat(A_WIS);
                    do_res_stat(A_INT);
                    do_res_stat(A_CHR);
                    set_shero(0,TRUE);
                    (void)p_inc_minislow(-10);
                    p_ptr->slow = 0;
                    update_stuff();
                    hp_player(_potion_power(420));
                    set_image(50 + randint1(50), TRUE);
                    break;
                default:
                    virtue_add(VIRTUE_CHANCE, -10);
                    msg_print("Massive explosions rupture your body!");
                    take_hit(DAMAGE_NOESCAPE, damroll(11, 66), "a potion of Liquid Logrus");

                    set_stun(MAX(p_ptr->stun, STUN_MASSIVE), FALSE);
                    set_cut(p_ptr->cut + 3630, FALSE);
                    device_noticed = TRUE;
                    break;
            }
        }
        break;
    case SV_POTION_LOSE_MEMORIES:
        if (desc) return "You lose experience when you quaff it.";
        if (cast)
        {
            if (!p_ptr->hold_life && (p_ptr->exp > 0))
            {
                msg_print("You feel your memories fade.");
                virtue_add(VIRTUE_KNOWLEDGE, -5);
                lose_exp(p_ptr->exp / 4);
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_MEAD_OF_POETRY:
        if (desc) return "Brewed from the blood of the murdered sage Kvasir, it temporarily gives its drinker great wisdom and eloquence.";
        if (info) return info_duration(_potion_power(75), _potion_power(75));
        if (cast)
        {
            int dur = _potion_power(100 + randint1(100));
            if (set_tim_poet(p_ptr->tim_poet + dur, FALSE))
            {
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_RUINATION:
        if (desc) return "You take damage and it decreases all your stats permanently when you quaff it.";
        if (cast)
        {
            msg_print("Your nerves and muscles feel weak and lifeless!");
            take_hit(DAMAGE_LOSELIFE, damroll(10, 10), "a potion of Ruination");

            dec_stat(A_DEX, 25, TRUE);
            dec_stat(A_WIS, 25, TRUE);
            dec_stat(A_CON, 25, TRUE);
            dec_stat(A_STR, 25, TRUE);
            dec_stat(A_CHR, 25, TRUE);
            dec_stat(A_INT, 25, TRUE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_STR:
        if (desc) return "It decreases your strength when you quaff it.";
        if (cast)
        {
            if (do_dec_stat(A_STR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_INT:
        if (desc) return "It decreases your intelligence when you quaff it.";
        if (cast)
        {
            if (do_dec_stat(A_INT)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_WIS:
        if (desc) return "It decreases your wisdom when you quaff it.";
        if (cast)
        {
            if (do_dec_stat(A_WIS)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_DEX:
        if (desc) return "It decreases your dexterity when you quaff it.";
        if (cast)
        {
            if (do_dec_stat(A_DEX)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_CON:
        if (desc) return "It decreases your constitution when you quaff it.";
        if (cast)
        {
            if (do_dec_stat(A_CON)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_CHR:
        if (desc) return "It decreases your charisma when you quaff it.";
        if (cast)
        {
            if (do_dec_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DETONATIONS:
        if (desc) return "It explodes in your mouth when you quaff it.";
        if (cast)
        {
            msg_print("Massive explosions rupture your body!");
            take_hit(DAMAGE_NOESCAPE, damroll(50, 20), "a potion of Detonation");

            set_stun(MAX(p_ptr->stun, STUN_MASSIVE), FALSE);
            set_cut(p_ptr->cut + 5000, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEATH:
        if (desc) return "You die when you quaff it.";
        if (cast)
        {
            virtue_add(VIRTUE_VITALITY, -1);
            virtue_add(VIRTUE_UNLIFE, 5);
            msg_print("A feeling of Death flows through your body.");
            take_hit(DAMAGE_LOSELIFE, 5000, "a potion of Death");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SIGHT:
        if (desc) return "It gives temporary see invisible and infravision and cures blindness when you quaff it.";
        if (info) return info_duration(_potion_power(100), _potion_power(100));
        if (cast)
        {
            int dur = _potion_power(100 + randint1(100));
            if (set_tim_infra(p_ptr->tim_infra + dur, FALSE))
            {
                device_noticed = TRUE;
            }
			if (set_tim_invis(p_ptr->tim_invis + dur, FALSE))
			{
				device_noticed = TRUE;
			}
			if (set_blind(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURE_POISON: //anti-toxin
        if (desc) return "It relieves poisoning and grants temporary poison resistance when you quaff it.";
        if (cast)
        {
			int dur = _potion_power(10 + randint1(10));
			if (set_poisoned(p_ptr->poisoned - MAX(400, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
			if (set_oppose_pois(p_ptr->oppose_pois + dur, FALSE))
			{
				device_noticed = TRUE;
			}
        }
        break;
    case SV_POTION_BOLDNESS:
        if (desc) return "It removes fear when you quaff it.";
        if (cast)
        {
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_SPEED:
        if (desc) return "It hastes you temporarily when you quaff it.";
        if (info) return format("dur d%d+%d", _potion_power(25), _potion_power(15));
        if (cast)
        {
            if (!p_ptr->fast)
            {
                int dur = _potion_power(randint1(25) + 15);
                if (set_fast(dur, FALSE)) device_noticed = TRUE;
            }
            else if (p_ptr->pclass == CLASS_MAULER)
                set_fast(p_ptr->fast + 10, FALSE);
            else
                set_fast(p_ptr->fast + 5, FALSE);
        }
        break;
    case SV_POTION_THERMAL:
        if (desc) return "You get temporary resistance to fire and cold when you quaff it. This resistance is cumulative with equipment.";
        if (info) return format("dur d%d+%d", _potion_power(10), _potion_power(10));
        if (cast)
        {
            int dur = _potion_power(10 + randint1(10));
            if (set_oppose_fire(p_ptr->oppose_fire + dur, FALSE))
            {
                device_noticed = TRUE;
            }
			if (set_oppose_cold(p_ptr->oppose_cold + dur, FALSE))
			{
				device_noticed = TRUE;
			}
        }
        break;
    case SV_POTION_VIGOR:
        if (desc) return "It cures all stunning and temporary slowness when you quaff it.";
        if (cast)
        {
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_slow(0, TRUE)) device_noticed = TRUE;
            if (p_inc_minislow(-10)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_HEROISM:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "It causes temporary heroism and provides special Alchemist bonuses (+2 speed and level-dependent extra to-hit, extra to-dam and extra shots) when you quaff it." : "It causes temporary heroism when you quaff it.";
        if (info) return format("dur d%d+%d", _potion_power(25), _potion_power(25));
        if (cast)
        {
            int dur = _potion_power(25 + randint1(25));
            if (set_hero(p_ptr->hero + dur, FALSE)) device_noticed = TRUE;
            if (p_ptr->pclass == CLASS_ALCHEMIST)
            {
                alchemist_set_hero(&device_noticed, p_ptr->hero + dur, TRUE);
                if (device_noticed)
                {
                    p_ptr->update |= (PU_BONUS);
                    handle_stuff();
                }
            }
        }
        break;
    case SV_POTION_BERSERK_STRENGTH:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "It causes you to go berserk and provides special Alchemist bonuses (+4 speed and level-dependent extra to-hit, extra to-dam, extra blows and extra shots) when you quaff it." : "It causes you to go berserk when you quaff it.";
        if (info) return format("dur d%d+%d", _potion_power(25), _potion_power(25));
        if (cast)
        {
            int dur = _potion_power(25 + randint1(25));
            if (set_shero(p_ptr->shero + dur, FALSE)) device_noticed = TRUE;
            if (hp_player(30)) device_noticed = TRUE;
            if (p_ptr->pclass == CLASS_ALCHEMIST)
            {
                alchemist_set_hero(&device_noticed, p_ptr->shero + dur, FALSE);
                if (device_noticed)
                {
                    p_ptr->update |= (PU_BONUS);
                    handle_stuff();
                }
            }
        }
        break;
    case SV_POTION_CURE_LIGHT:
        if (desc) return "It heals you slightly, cures berserk and reduces cuts when you quaff it.";
        if (info) return info_heal(4, _potion_power(8), 0);
        if (cast)
        {
            if (hp_player(_potion_power(damroll(4, 8)))) device_noticed = TRUE;
            if (set_cut(p_ptr->cut - 15, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURE_SERIOUS:
        if (desc) return "It heals you a bit, cures berserk and reduces cuts when you quaff it.";
        if (info) return info_heal(8, _potion_power(8), 0);
        if (cast)
        {
            if (hp_player(_potion_power(damroll(8, 8)))) device_noticed = TRUE;
            if (set_cut((p_ptr->cut / 2) - 50, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURE_CRITICAL:
        if (desc) return "It heals you and cures stunning, cuts and berserk when you quaff it.";
        if (info) return info_heal(12, _potion_power(8), 0);
        if (cast)
        {
            if (hp_player(_potion_power(damroll(12, 8)))) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
//	if (set_poisoned(p_ptr->poisoned - MAX(150, p_ptr->poisoned / 3), TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_BLOOD:
        if (desc) return "A much needed infusion! It heals you a bit and cures blindness, confusion, and stunning when you quaff it.";
        if (info) return info_heal(0, 0, _potion_power(200));
        if (cast)
        {
            if (hp_player(_potion_power(200))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_HEALING: {
        int amt = 300;
        if (desc) return "It heals you and cures blindness, confusion, stunning, cuts and berserk when you quaff it.";
        if (info) return info_heal(0, 0, _potion_power(amt));
        if (cast)
        {
            if (hp_player(_potion_power(amt))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
            if (p_inc_minislow(-1)) device_noticed = TRUE;
        }
        break; }
    case SV_POTION_STAR_HEALING:
        if (desc) return "It heals you and cures blindness, confusion, poison, stunning, cuts, illnesses and berserk when you quaff it.";
        if (info) return info_heal(0, 0, _potion_power(1000));
        if (cast)
        {
            if (hp_player(_potion_power(1000))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0, TRUE)) device_noticed = TRUE;
            if (set_unwell(0, TRUE)) device_noticed = TRUE;
            if (p_inc_minislow(-1)) device_noticed = TRUE;
            update_stuff(); /* hp may change if the player was unwell ... */
        }
        break;
    case SV_POTION_LIFE:
        if (desc) return "It heals you completely, restores life, experience and all your stats and cures blindness, confusion, poison, hallucination, stunning, cuts, slowness, illnesses and berserk when you quaff it.";
        if (info) return info_heal(0, 0, _potion_power(5000));
        if (cast)
        {
            virtue_add(VIRTUE_VITALITY, 1);
            virtue_add(VIRTUE_UNLIFE, -5);
            msg_print("You feel life flow through your body!");
            restore_level();
            lp_player(1000);
            set_poisoned(0, TRUE);
            set_blind(0, TRUE);
            set_confused(0, TRUE);
            set_image(0, TRUE);
            set_stun(0, TRUE);
            set_cut(0, TRUE);
            set_unwell(0, TRUE);
            do_res_stat(A_STR);
            do_res_stat(A_CON);
            do_res_stat(A_DEX);
            do_res_stat(A_WIS);
            do_res_stat(A_INT);
            do_res_stat(A_CHR);
            set_shero(0,TRUE);
            (void)p_inc_minislow(-10);
            p_ptr->slow = 0;
            update_stuff();
            hp_player(_potion_power(5000));
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_CLARITY:
        if (desc) return "It clears your mind when you quaff it, curing confusion and restoring some mana.";
        if (info) return format("3d%d + %d", _potion_power(6), _potion_power(3));
        if (cast)
        {
            int amt = _potion_power(damroll(3, 6) + 3);

            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("You are unaffected.");
            else if (sp_player(amt))
            {
                msg_print("You feel your mind clear.");
                device_noticed = TRUE;
            }
		if (set_confused(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_GREAT_CLARITY:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "It greatly clears your mind when you quaff it and cures confusion, stunning and hallucinations." : "It greatly clears your mind when you quaff it and cures confusion and hallucinations.";
        if (info) return format("10d%d + %d", _potion_power(10), _potion_power(15));
        if (cast)
        {
            int amt = _potion_power(damroll(10, 10) + 15);

            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("You are unaffected.");
            else if (sp_player(amt))
            {
                msg_print("You feel your mind clear.");
                device_noticed = TRUE;
            }
			if (set_confused(0, TRUE)) device_noticed = TRUE;
			if (set_image(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RESTORE_MANA:
        if (desc) return "It restores mana to full and cures berserk when you quaff it. It also partially recharges any devices in your pack.";
        if (cast)
        {
            if (restore_mana()) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RESTORE_EXP:
        if (desc) return "It restores your life and experience when you quaff it.";
        if (cast)
        {
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(150)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_STR:
        if (desc) return "It restores your strength when you quaff it.";
        if (cast)
        {
            if (do_res_stat(A_STR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_INT:
        if (desc) return "It restores your intelligence when you quaff it.";
        if (cast)
        {
            if (do_res_stat(A_INT)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_WIS:
        if (desc) return "It restores your wisdom when you quaff it.";
        if (cast)
        {
            if (do_res_stat(A_WIS)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_DEX:
        if (desc) return "It restores your dexterity when you quaff it.";
        if (cast)
        {
            if (do_res_stat(A_DEX)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_CON:
        if (desc) return "It restores your constitution when you quaff it.";
        if (cast)
        {
            if (do_res_stat(A_CON)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_CHR:
        if (desc) return "It restores your charisma when you quaff it.";
        if (cast)
        {
            if (do_res_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
	case SV_POTION_RES_ALL:
		if (desc) return "It restores your stats, life and experience when you quaff it.";
		if (cast)
		{
			if (do_res_stat(A_STR)) device_noticed = TRUE;
			if (do_res_stat(A_INT)) device_noticed = TRUE;
			if (do_res_stat(A_WIS)) device_noticed = TRUE;
			if (do_res_stat(A_DEX)) device_noticed = TRUE;
			if (do_res_stat(A_CON)) device_noticed = TRUE;
			if (do_res_stat(A_CHR)) device_noticed = TRUE;
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(150)) device_noticed = TRUE;
		}
		break;
    case SV_POTION_INC_STR:
        if (desc) return "It increases your strength when you quaff it.";
        if (cast)
        {
            if (do_inc_stat(A_STR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_INT:
        if (desc) return "It increases your intelligence when you quaff it.";
        if (cast)
        {
            if (do_inc_stat(A_INT)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_WIS:
        if (desc) return "It increases your wisdom when you quaff it.";
        if (cast)
        {
            if (do_inc_stat(A_WIS)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_DEX:
        if (desc) return "It increases your dexterity when you quaff it.";
        if (cast)
        {
            if (do_inc_stat(A_DEX)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_CON:
        if (desc) return "It increases your constitution when you quaff it.";
        if (cast)
        {
            if (do_inc_stat(A_CON)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_CHR:
        if (desc) return "It increases your charisma when you quaff it.";
        if (cast)
        {
            if (do_inc_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_AUGMENTATION:
        if (desc) return "It increases all your stats when you quaff it.";
        if (cast)
        {
            if (do_inc_stat(A_STR)) device_noticed = TRUE;
            if (do_inc_stat(A_INT)) device_noticed = TRUE;
            if (do_inc_stat(A_WIS)) device_noticed = TRUE;
            if (do_inc_stat(A_DEX)) device_noticed = TRUE;
            if (do_inc_stat(A_CON)) device_noticed = TRUE;
            if (do_inc_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_ENLIGHTENMENT:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "It maps, lights permanently and detects all items on the entire level and provides temporary telepathy when you quaff it." : "It maps, lights permanently and detects all items on the entire level when you quaff it.";
        if (cast)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            msg_print("An image of your surroundings forms in your mind...");
            wiz_lite(p_ptr->tim_superstealth > 0);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_STAR_ENLIGHTENMENT: /* warning - long line ahead */
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "It maps, lights permanently and detects all items on the entire level, increases your intelligence and wisdom, detects all traps, doors, stairs, treasures in your vicinity, identifies all items in pack, provides temporary telepathy and gives information about yourself when you quaff it." : "It maps, lights permanently and detects all items on the entire level, increases your intelligence and wisdom, detects all traps, doors, stairs, treasures in your vicinity, identifies all items in pack and gives information about yourself when you quaff it.";
        if (cast)
        {
            msg_print("You begin to feel more enlightened...");
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 2);
            msg_print(NULL);
            wiz_lite(p_ptr->tim_superstealth > 0);
            do_inc_stat(A_INT);
            do_inc_stat(A_WIS);
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
            detect_treasure(DETECT_RAD_DEFAULT);
            detect_objects_gold(DETECT_RAD_DEFAULT);
            detect_objects_normal(DETECT_RAD_DEFAULT);
            identify_pack();
            self_knowledge();
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SELF_KNOWLEDGE:
        if (desc) return "It gives information about yourself when you quaff it.";
        if (cast)
        {
            msg_print("You begin to know yourself a little better...");
            msg_print(NULL);
            self_knowledge();
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_EXPERIENCE:
        if (desc) return "You become more experienced when you quaff it.";
        if (cast)
        {
            if (p_ptr->prace == RACE_ANDROID) break;
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            if (p_ptr->exp < PY_MAX_EXP)
            {
                s32b ee = _potion_power((p_ptr->exp / 2) + 10);
                s32b max = _potion_power(100000);
                if (mut_present(MUT_FAST_LEARNER))
                {
                    ee = ee * 5/3;
                    max = max * 5/3;
                }
                if (ee > max) ee = max;
                msg_print("You feel more experienced.");
                gain_exp(ee);
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_RESISTANCE:
        if (desc) return "You get temporary resistance to the elements and poison when you quaff it. ";
        if (info) return format("dur d%d+%d", _potion_power(20), _potion_power(20));
        if (cast)
        {
            int dur = _potion_power(20 + randint1(20));
            set_oppose_base(dur, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURING: {
        if (desc) return "It cures blindness, confusion, stunning, cuts and hallucination and reduces poisoning when you quaff it.";
        if (cast)
        {
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(p_ptr->poisoned - MAX(200, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_image(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break; }
    case SV_POTION_INVULNERABILITY:
        if (desc) return "You become invulnerable temporarily when you quaff it.";
        if (info) return format("dur d%d+%d", _potion_power(4), _potion_power(4));
        if (cast)
        {
            int dur = _potion_power(4 + randint1(4));
            set_invuln(p_ptr->invuln + dur, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_NEW_LIFE:
        if (desc) return "It changes your life rating and max of all your stats and cures all mutations when you quaff it.";
        if (cast)
        {
            do_cmd_rerate(FALSE);
            lp_player(1000);
            get_max_stats();
            p_ptr->update |= PU_BONUS;
            mut_lose_all();
            device_noticed = TRUE;
            if (p_ptr->personality == PERS_SPLIT)
                split_shuffle(2);
            if (p_ptr->pclass == CLASS_WILD_TALENT)
                wild_talent_new_life();
            /* XXX Originally, this was here as an act of mercy for players new to this class.
             * However, it is hugely scummable since you can pick the powers useful in early play
             * and then new life to switch over to those useful in late game. Several psion powers
             * are huge in the early game, but not so much later on. Players will abuse this. Also,
             * the Skillmaster has exactly the same permanent-irreversible-don't-screw-up-your-choices
             * game mechanic and receive no such love. Keeping things for the Wild Talent makes
             * sense though, since they are random (and don't get a say in the matter anyway).
            if (p_ptr->pclass == CLASS_PSION && get_check("Relearn Powers? "))
                psion_relearn_powers();*/
        }
        break;
    case SV_POTION_NEO_TSUYOSHI:
        if (desc) return "It cures hallucination and increases your strength and constitution temporarily when you quaff it but your strength and constitution permanently decrease lower than before when the effect expires.";
        if (cast)
        {
            set_image(0, TRUE);
            set_tsuyoshi(p_ptr->tsuyoshi + randint1(100) + 100, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_TSUYOSHI:
        if (desc) return "It decreases your strength and constitution permanently and makes you hallucinate when you quaff it.";
        if (cast)
        {
            msg_print("Brother OKURE!");
            msg_print(NULL);
            p_ptr->tsuyoshi = 1;
            set_tsuyoshi(0, TRUE);
            if (!res_save_default(RES_CHAOS))
                set_image(50 + randint1(50), FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_GIANT_STRENGTH:
        if (desc) return "It greatly increases your stature temporarily when you quaff it.";
        if (info) return format("dur d%d+%d", _potion_power(20), _potion_power(20));
        if (cast)
        {
            if (set_tim_building_up(_potion_power(20 + randint1(20)), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_POLYMORPH:
        if (desc) return "It mutates you when you quaff it. Rarely it cures all mutations.";
        if (cast)
        {
            int count = mut_count(mut_unlocked_pred);
            if (count > 1 && one_in_(23))
            {
                mut_lose_all();
                if (p_ptr->pclass == CLASS_WILD_TALENT)
                    wild_talent_new_life();
            }
            else
            {
                do
                {
                    if (one_in_(2))
                    {
                        if(mut_gain_random(NULL))
                        {
                            count++;
                            device_noticed = TRUE;
                        }
                    }
                    else if (count > 5 || one_in_(6 - count))
                    {
                        if (mut_lose_random(NULL))
                        {
                            count--;
                            device_noticed = TRUE;
                        }
                    }
                } while (!device_noticed || one_in_(2));

                if (p_ptr->pclass == CLASS_WILD_TALENT && one_in_(2))
                    wild_talent_scramble();
            }
        }
        break;
    case SV_POTION_STONE_SKIN:
        if (desc) return "It temporarily turns your skin to stone, granting enhanced armor class, when you quaff it.";
        if (info) return format("dur d%d+%d", _potion_power(20), _potion_power(20));
        if (cast)
        {
            if (set_shield(_potion_power(20 + randint1(20)), FALSE)) device_noticed = TRUE;
        }
        break;
    }
    return "";
}

static bool _scroll_check_no_effect(int sval)
{
    int k_idx = lookup_kind(TV_SCROLL, sval);
    if (!k_info[k_idx].aware) return TRUE;
    if (msg_prompt("This scroll will have no effect here. Read it anyway? <color:y>[y/n]</color>", "ny", PROMPT_DEFAULT) == 'y') return TRUE;
    return FALSE;
}

static cptr _do_scroll(int sval, int mode)
{
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    switch (sval)
    {
    case SV_SCROLL_DARKNESS:
        if (desc) return "It darkens nearby area or current room and blinds you when you read it.";
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_DARK))
            {
                if (set_blind(p_ptr->blind + 3 + randint1(5), FALSE)) device_noticed = TRUE;
            }
            if (unlite_area(10, 3)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_AGGRAVATE_MONSTER:
        if (desc) return "It aggravates monsters in your vicinity when you read it.";
        if (cast)
        {
            msg_print("There is a high pitched humming noise.");
            aggravate_monsters(0);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_CURSE_ARMOR:
        if (desc) return "It makes your current armour (Blasted) when you read it.";
        if (cast)
        {
            int slot = equip_random_slot(object_is_armour);
            if (slot && curse_armor(slot)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_CURSE_WEAPON:
        if (desc) return "It blasts your current melee weapon when you read it.";
        if (cast)
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot && curse_weapon(FALSE, slot)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SUMMON_MONSTER:
        if (desc) return "It summons several monsters as enemies when you read it.";
        if (cast)
        {
            int i;
            for (i = 0; i < randint1(3); i++)
            {
                if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_SCROLL_SUMMON_UNDEAD:
        if (desc) return "It summons several undead monsters as enemies when you read it.";
        if (cast)
        {
            int i;
            for (i = 0; i < randint1(3); i++)
            {
                if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_UNDEAD, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_SCROLL_SUMMON_PET:
        if (desc)
        {
            if (p_ptr->prace == RACE_MON_RING)
                return "It summons a ring bearer as your pet when you read it.";
            else
                return "It summons a monster as your pet when you read it.";
        }
        if (cast)
        {
            int type = 0;
            if (p_ptr->prace == RACE_MON_RING)
                type = SUMMON_RING_BEARER;
            if (p_ptr->inside_arena && !type && !prace_is_(RACE_MON_QUYLTHULG) && !_scroll_check_no_effect(sval)) return NULL;
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, _scroll_power(dun_level), type, (PM_ALLOW_GROUP | PM_FORCE_PET)))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SUMMON_KIN:
        if (desc) return "It summons a monster corresponding to your race as your pet when you read it.";
        if (cast)
        {
            if (p_ptr->inside_arena && !prace_is_(RACE_MON_QUYLTHULG) && !_scroll_check_no_effect(sval)) return NULL;
            if (summon_kin_player(_scroll_power(p_ptr->lev), py, px, (PM_FORCE_PET | PM_ALLOW_GROUP)))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_TRAP_CREATION:
        if (desc) return "It creates traps on the squares adjacent to you when you read it.";
        if (cast)
        {
            if (trap_creation(py, px))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_PHASE_DOOR:
        if (desc) return "It teleports you a short distance when you read it.";
        if (cast)
        {
            teleport_player(10, 0L);
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_TELEPORT:
        if (desc) return "It teleports you a long distance when you read it.";
        if (cast)
        {
            teleport_player(100, 0L);
            energy_use = energy_use * 3 / 2;
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_TELEPORT);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_TELEPORT_LEVEL:
        if (desc) return "It teleports you one dungeon level up or down immediately when you read it.";
        if (cast)
        {
            if ((TELE_LEVEL_IS_INEFF(-1)) && (!_scroll_check_no_effect(sval))) return NULL;
            device_noticed = TRUE;
            if (k_info[lookup_kind(TV_SCROLL, sval)].aware)
            {
                if (!py_teleport_level(NULL)) return NULL;
            }
            else teleport_level(0);
        }
        break;
    case SV_SCROLL_WORD_OF_RECALL:
        if (desc) return "It recalls you to the town, or back into the dungeon you have entered when you read it.";
        if (cast)
        {
            device_noticed = TRUE;
            if (!word_of_recall(k_info[lookup_kind(TV_SCROLL, sval)].aware)) return NULL;
        }
        break;
    case SV_SCROLL_IDENTIFY:
        if (desc) return "It identifies an item when you read it.";
        if (cast)
        {
            device_noticed = TRUE;
            if (!_do_identify()) return NULL;
        }
        break;
    case SV_SCROLL_STAR_IDENTIFY:
        if (desc) return "It reveals all information about an item when you read it.";
        if (cast)
        {
            device_noticed = TRUE;
            if (!identify_fully(NULL)) return NULL;
        }
        break;
    case SV_SCROLL_REMOVE_CURSE:
        if (desc) return "It removes normal curses from equipped items when you read it.";
        if (cast)
        {
            if (remove_curse())
            {
                msg_print("You feel as if someone is watching over you.");
                device_noticed = TRUE;
            }
        }
        break;
    case SV_SCROLL_STAR_REMOVE_CURSE:
        if (desc) return "It removes normal and heavy curses from equipped items when you read it.";
        if (cast)
        {
            if (remove_all_curse())
                msg_print("You feel as if someone is watching over you.");
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ENCHANT_ARMOR:
        if (desc) return "It increases an armour's to-AC when you read it.";
        if (cast)
        {
            if (!enchant_spell(0, 0, 1)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
        if (desc) return "It increases a weapon's to-hit when you read it.";
        if (cast)
        {
            if (!enchant_spell(1, 0, 0)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
        if (desc) return "It increases a weapon's to-dam when you read it.";
        if (cast)
        {
            if (!enchant_spell(0, 1, 0)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_ENCHANT_ARMOR:
        if (desc) return "It increases an armour's to-ac powerfully when you read it.";
        if (cast)
        {
            if (!enchant_spell(0, 0, randint1(3) + 3)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_ENCHANT_WEAPON:
        if (desc) return "It increases a weapon's to-hit and to-dam powerfully when you read it.";
        if (cast)
        {
            if (!enchant_spell(randint1(3) + 3, randint1(3) + 3, 0)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_RECHARGING:
        if (desc) return "It attempts to recharge a magical device using the mana of a source device. This usually destroys the source device.";
        if (cast)
        {
            if (!recharge_from_device(_scroll_power(100))) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MUNDANITY:
        if (desc) return "This removes the ego or artifact status and all enchantment of an item. As a bonus, if you have a stack of them, the extras are destroyed.";
        if (cast)
        {
            if (!mundane_spell(FALSE)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_LIGHT:
        if (desc) return "It lights up nearby area or the current room permanently when you read it.";
        if (cast)
        {
            if (lite_area(damroll(2, 8), 2)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MAPPING:
        if (desc) return "It maps your vicinity when you read it.";
        if (cast)
        {
            map_area(_scroll_power(DETECT_RAD_MAP));
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_GOLD:
        if (desc) return "It detects all treasures in your vicinity when you read it.";
        if (cast)
        {
            if (detect_treasure(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
            if (detect_objects_gold(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_ITEM:
        if (desc) return "It detects all items in your vicinity when you read it.";
        if (cast)
        {
            if (detect_objects_normal(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_TRAP:
        if (desc) return "It detects all traps in your vicinity when you read it.";
        if (cast)
        {
            if (detect_traps(_scroll_power(DETECT_RAD_DEFAULT), device_known)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_DOOR:
        if (desc) return "It detects all doors and stairs in your vicinity when you read it.";
        if (cast)
        {
            if (detect_doors(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
            if (detect_stairs(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_INVIS:
        if (desc) return "It detects all invisible monsters in your vicinity when you read it.";
        if (cast)
        {
            if (detect_monsters_invis(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_MONSTERS:
        if (desc) return "It detects all monsters in your vicinity when you read it.";
        if (cast)
        {
            if (detect_monsters_normal(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SATISFY_HUNGER:
        if (desc) return "It satisfies hunger when you read it.";
        if (cast)
        {
            if (set_food(PY_FOOD_MAX - 1)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_BLESSING:
        if (desc) return "It blesses you temporarily when you read it.";
        if (cast)
        {
            if (set_blessed(p_ptr->blessed + _scroll_power(randint1(12) + 6), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_HOLY_CHANT:
        if (desc) return "It blesses you temporarily when you read it.";
        if (cast)
        {
            if (set_blessed(p_ptr->blessed + _scroll_power(randint1(24) + 12), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_HOLY_PRAYER:
        if (desc) return "It blesses you temporarily when you read it.";
        if (cast)
        {
            if (set_blessed(p_ptr->blessed + _scroll_power(randint1(48) + 24), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MONSTER_CONFUSION:
        if (desc) return "You can confuse monster you hit just for once when you read it.";
        if (cast)
        {
            if (p_ptr->prace == RACE_MON_RING) /* no melee attacks */
            {
                msg_print("There is no effect.");
            }
            else if (!(p_ptr->special_attack & ATTACK_CONFUSE))
            {
                msg_print("Your hands begin to glow.");
                p_ptr->special_attack |= ATTACK_CONFUSE;
                p_ptr->redraw |= (PR_STATUS);
            }
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_PROTECTION_FROM_EVIL:
        if (desc) return "It gives temporary protection from lesser evil creatures when you read it.";
        if (cast)
        {
            if (set_protevil(p_ptr->protevil + _scroll_power(randint1(25) + 3 * p_ptr->lev), FALSE))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_RUNE_OF_PROTECTION:
        if (desc) return "It creates a glyph on the floor you stand when you read it.";
        if (cast)
        {
            device_noticed = TRUE;
            if ((!cave_clean_bold(py, px)) && (msg_prompt("Glyphs of Warding can only be created on empty squares. Read the scroll anyway? <color:y>[y/n]</color>", "ny", PROMPT_DEFAULT) != 'y')) return NULL;
            warding_glyph();
        }
        break;
    case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
        if (desc) return "It destroys traps on the floors adjacent to you when you read it.";
        if (cast)
        {
            if (destroy_doors_touch()) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_DESTRUCTION:
        if (desc) return "It destroys everything nearby you when you read it.";
        if (cast)
        {
            if (((!py_in_dungeon()) || (!quests_allow_all_spells()))
               && (!_scroll_check_no_effect(sval))) return NULL;
            if (destroy_area(py, px, 13 + randint0(5), _scroll_power(2000)))
                device_noticed = TRUE;
            else
                msg_print("The ground trembles...");
        }
        break;
    case SV_SCROLL_DISPEL_UNDEAD:
        if (desc) return "It damages all undead monsters in sight when you read it.";
        if (info) return info_damage(0, 0, _scroll_power(80));
        if (cast)
        {
            if (dispel_undead(_scroll_power(80))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SPELL:
        if (desc) return "Increases the number of spells you can study. Only has an effect for classes who study spells.";
        if (cast)
        {
            if (!class_uses_spell_scrolls(p_ptr->pclass))
            {
                msg_print("There is no effect.");
            }
            else
            {
                p_ptr->add_spells++;
                p_ptr->update |= (PU_SPELLS);
            }
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_GENOCIDE:
        if (desc) return "It eliminates an entire class of monster, exhausting you. Powerful or unique monsters may resist.";
        if (cast)
        {
            if (((!quests_allow_all_spells()) || (p_ptr->inside_arena) || (p_ptr->inside_battle))
               && (!_scroll_check_no_effect(sval))) return NULL;
            if (!symbol_genocide(_scroll_power(300), TRUE)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MASS_GENOCIDE:
        if (desc) return "It eliminates all nearby monsters, exhausting you. Powerful or unique monsters may be able to resist.";
        if (cast)
        {
            if (((!quests_allow_all_spells()) || (p_ptr->inside_arena) || (p_ptr->inside_battle))
               && (!_scroll_check_no_effect(sval))) return NULL;
            mass_genocide(_scroll_power(300), TRUE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ACQUIREMENT:
        if (desc) return "It creates one great item when you read it. Gives better results on deep levels.";
        if (cast)
        {
            acquirement(py, px, 1, TRUE, FALSE, ORIGIN_ACQUIRE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_ACQUIREMENT:
        if (desc) return "It creates some great items when you read it. Gives better results on deep levels.";
        if (cast)
        {
            acquirement(py, px, _scroll_power(randint1(2) + 1), TRUE, FALSE, ORIGIN_ACQUIRE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_FOREST_CREATION:
        if (desc) return "It surrounds you with verdure.";
        if (cast)
        {
            if (tree_creation()) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_WALL_CREATION:
        if (desc) return "It surrounds you with rock.";
        if (cast)
        {
            if (wall_stone()) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_VENGEANCE:
        if (desc) return "For a short time, monsters that attack you receive an equal amount of damage in retaliation.";
        if (cast)
        {
            set_tim_eyeeye(_scroll_power(randint1(25) + 25), FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_RUMOR:
        if (desc) return "A rumor is in it.";
        if (cast)
        {
            char Rumor[1024];
            errr err = 0;

            switch (randint1(20))
            {
            case 1:
                err = get_rnd_line("chainswd.txt", 0, Rumor);
                break;
            case 2:
                err = get_rnd_line("error.txt", 0, Rumor);
                break;
            case 3:
            case 4:
            case 5:
                err = get_rnd_line("death.txt", 0, Rumor);
                break;
            default:
                err = get_rnd_line("rumors.txt", 0, Rumor);
            }

            if (err) strcpy(Rumor, "Some rumors are wrong.");
            msg_format("<color:B>There is a message on the scroll. It says:</color> %s", Rumor);
            msg_print("The scroll disappears in a puff of smoke!");
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ARTIFACT:
        if (desc) return "It creates an artifact from a nameless weapon or armour when you read it. Gives better results on deeper levels. Don't be greedy - you will get only one artifact.";
        if (cast)
        {
            device_noticed = TRUE;
            if (no_artifacts)
            {
                if (!brand_weapon(-1)) return NULL;
            }
            else
            {
                if (!artifact_scroll()) return NULL;
            }
        }
        break;
    case SV_SCROLL_MADNESS:
        if (desc) return "It seems to be the hurried scribblings of a mad wizard on the verge of some great arcane discovery. You can't make heads or tails of it. Do you read it to see what happens?";
        if (cast)
        {
            int n = randint0(_scroll_power(100));
            device_noticed = TRUE;
            if (n < 2)
            {
                int curses = 1 + randint1(3);
                bool stop_ty = FALSE;
                int count = 0;

                cmsg_print(TERM_VIOLET, "The scroll has an ancient, foul curse!");
                curse_equipment(100, 50);
                do
                {
                    stop_ty = activate_ty_curse(stop_ty, &count);
                }
                while (--curses);
            }
            else if (n < 12)
            {
                msg_print("Ooops! That didn't work at all!");
                destroy_area(py, px, 13 + randint0(5), 300);
            }
            else if (n < 17)
            {
                msg_print("You faintly hear crazy laughter for a moment.");
                summon_cyber(-1, py, px);
            }
            else if (n < 27)
            {
                msg_print("The scroll explodes violently!");
                project(0, 10, py, px, 300, GF_MANA, PROJECT_KILL | PROJECT_ITEM);
            }
            else if (n < 50)
            {
                _do_scroll(SV_SCROLL_CURSE_ARMOR, mode);
            }
            else if (n < 75)
            {
                _do_scroll(SV_SCROLL_CURSE_WEAPON, mode);
            }
            else if (n < 95)
            {
                if (!_do_scroll(SV_SCROLL_CRAFTING, mode)) return NULL;
            }
            else
            {
                _do_scroll(SV_SCROLL_ARTIFACT, mode);
            }
        }
        break;
    case SV_SCROLL_CRAFTING:
        if (desc) return "It makes a chosen weapon, armor or ammo an ego item when you read it.";
        if (cast)
        {
            device_noticed = TRUE;
            if (!cast_crafting())
                return NULL;
        }
        break;
    case SV_SCROLL_RESET_RECALL:
        if (desc) return "It resets the dungeon level for recall spell when you read it.";
        if (cast)
        {
            device_noticed = TRUE;
            if (!reset_recall()) return NULL;
        }
        break;
    case SV_SCROLL_FIRE:
        if (desc) return "It creates a huge fire ball centered on you.";
        if (info) return info_damage(0, 0, _scroll_power(333));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_FIRE, 0, _scroll_power(666), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS) && !res_save_default(RES_FIRE))
            {
                int dam = res_calc_dam(RES_FIRE, 25 + randint1(25));
                take_hit(DAMAGE_NOESCAPE, dam, "a Scroll of Fire");
            }
        }
        break;
    case SV_SCROLL_ICE:
        if (desc) return "It creates a huge ice ball centered on you.";
        if (info) return info_damage(0, 0, _scroll_power(400));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_ICE, 0, _scroll_power(800), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS) && !res_save_default(RES_COLD))
            {
                int dam = res_calc_dam(RES_COLD, 30 + randint1(30));
                take_hit(DAMAGE_NOESCAPE, dam, "a Scroll of Ice");
            }
        }
        break;
/*    case SV_SCROLL_CHAOS:
        if (desc) return "It creates a huge ball of logrus centered on you.";
        if (info) return info_damage(0, 0, _scroll_power(500));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_CHAOS, 0, _scroll_power(1000), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS) && !res_save_default(RES_CHAOS))
            {
                int dam = res_calc_dam(RES_CHAOS, 50 + randint1(50));
                take_hit(DAMAGE_NOESCAPE, dam, "a Scroll of Logrus");
            }
        }
        break;*/
    case SV_SCROLL_UNDERSTANDING:
        if (desc) return "It identifies all items in your pack and gives temporary auto-identify.";
        if (cast)
        {
            if (set_tim_understanding(_scroll_power(40), FALSE))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MANA:
        if (desc) return "It creates a huge ball of pure mana centered on you.";
        if (info) return info_damage(0, 0, _scroll_power(550));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_MANA, 0, _scroll_power(1100), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS))
                take_hit(DAMAGE_NOESCAPE, 50 + randint1(50), "a Scroll of Mana");
        }
        break;
    case SV_SCROLL_BANISHMENT:
        if (desc) return "It teleports all monsters in sight away unless resisted.";
        if (info) return info_power(_scroll_power(150));
        if (cast)
        {
            if (banish_monsters(_scroll_power(150)))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_INVEN_PROT:
        if (desc) return "It creates a temporary protective shield around your inventory.";
        if (cast)
        {
            if (set_tim_inven_prot2(p_ptr->tim_inven_prot2 + _scroll_power(25), FALSE)) device_noticed = TRUE;
        }
        break;
    }
    return "";
}

cptr do_device(object_type *o_ptr, int mode, int boost)
{
    cptr result = NULL;

    device_noticed = FALSE;
    if (!_multi_charge_lock) device_used_charges = 0;
    device_lore = FALSE;

    if (o_ptr->activation.type)
    {
        u32b flgs[OF_ARRAY_SIZE];

        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, OF_DEVICE_POWER))
            boost += device_power_aux(100, o_ptr->pval) - 100;

        result = do_effect(&o_ptr->activation, mode, boost);
    }
    else
    {
        switch (o_ptr->tval)
        {
            case TV_SCROLL: result = _do_scroll(o_ptr->sval, mode); break;
            case TV_POTION:
            {
                result = _do_potion(o_ptr->sval, mode);
                if ((p_ptr->pclass == CLASS_ALCHEMIST) && (mode == SPELL_CAST)) alchemist_super_potion_effect(o_ptr->sval);
                break;
            }
        }
    }
    device_known = FALSE;
    device_extra_power = 0;
    if (!_multi_charge_lock) device_available_charges = 0;
    return result;
}

/* Effects: We are following the do_spell() pattern which is quick and dirty,
   but not my preferred approach ... Also, we could conceivably merge all
   devices into effects, handling rods, staves, wands, potions, scrolls and
   activations uniformly. For the moment, effects are *just* activations,
   and I should mention that each type of effect has its own little quirky
   fail rate calculation ... sigh.

   Update: The Device Rewrite is merging Wands/Rods/Staves into the effect system!*/
effect_t obj_get_effect(object_type *o_ptr)
{
    if (o_ptr->activation.type)
        return o_ptr->activation;
    if (o_ptr->name1 && a_info[o_ptr->name1].activation.type)
        return a_info[o_ptr->name1].activation;
    if (o_ptr->name2 && e_info[o_ptr->name2].activation.type)
        return e_info[o_ptr->name2].activation;
    return k_info[o_ptr->k_idx].activation;
}

cptr obj_get_effect_msg(object_type *o_ptr)
{
    u32b offset;

    if (o_ptr->activation.type)
        return 0;

    if (o_ptr->name1 && a_info[o_ptr->name1].activation.type)
    {
        offset = a_info[o_ptr->name1].activation_msg;
        if (offset)
            return a_text + offset;
        else
            return 0;
    }
    if (o_ptr->name2 && e_info[o_ptr->name2].activation.type)
    {
        return 0;
    }

    offset = k_info[o_ptr->k_idx].activation_msg;
    if (offset)
        return k_text + offset;

    return 0;
}

bool obj_has_effect(object_type *o_ptr)
{
    effect_t e = obj_get_effect(o_ptr);
    if (e.type)
        return TRUE;
    return FALSE;
}

bool effect_try(effect_t *effect)
{
    int fail = effect_calc_fail_rate(effect);
    if (randint0(1000) < fail)
        return FALSE;
    return TRUE;
}

bool effect_use(effect_t *effect, int boost)
{
    device_noticed = FALSE;
    device_used_charges = 0;
    device_available_charges = 1;
    if (do_effect(effect, SPELL_CAST, boost))
        return TRUE;
    return FALSE;
}

int effect_value(effect_t *effect)
{
    int  result = 0;
    cptr hack = do_effect(effect, SPELL_VALUE, 0);
    if (hack)
        result = atoi(hack);
    return result;
}

int effect_cost_extra(effect_t *effect)
{
    int  result = 0;
    cptr hack = do_effect(effect, SPELL_COST_EXTRA, 0);
    if (hack)
        result = atoi(hack);
    return result;
}

byte effect_color(effect_t *effect)
{
    byte result = TERM_WHITE;
    cptr hack = do_effect(effect, SPELL_COLOR, 0);
    if (hack && strlen(hack))
        result = atoi(hack);
    return result;
}

typedef struct
{
    cptr text;
    int  type;
    int  level;
    int  cost;
    int  rarity;
    int  bias;
    bool known;
} _effect_info_t, *_effect_info_ptr;

/*  Allocation Table for Random Artifact Activations
    This also assists parsing a_info.txt, k_info.txt and e_info.txt.
    Order is irrelevant. Use Rarity 0 to exclude allocations.
*/
static _effect_info_t _effect_info[] =
{
    /* Detection:                                   Lv    T   R  Bias */
    {"LITE_AREA",       EFFECT_LITE_AREA,            1,  10,  1, BIAS_MAGE},
    {"LITE_MAP_AREA",   EFFECT_LITE_MAP_AREA,       20,  50,  3, 0},
    {"ENLIGHTENMENT",   EFFECT_ENLIGHTENMENT,       20,  50,  2, BIAS_PRIESTLY | BIAS_ARCHER},
    {"CLAIRVOYANCE",    EFFECT_CLAIRVOYANCE,        35, 100,  8, BIAS_MAGE},

    {"DETECT_TRAPS",    EFFECT_DETECT_TRAPS,         3,  10,  1, BIAS_ROGUE},
    {"DETECT_MONSTERS", EFFECT_DETECT_MONSTERS,      5,  20,  1, BIAS_MAGE | BIAS_ARCHER},
    {"DETECT_OBJECTS",  EFFECT_DETECT_OBJECTS,       8,  25,  1, BIAS_ROGUE},
    {"DETECT_ALL",      EFFECT_DETECT_ALL,          25,  50,  3, BIAS_ROGUE},
    {"DETECT_GOLD",     EFFECT_DETECT_GOLD,          5,  20,  1, BIAS_ROGUE},
    {"DETECT_INVISIBLE",EFFECT_DETECT_INVISIBLE,     5,  20,  1, 0},
    {"DETECT_DOOR_STAIRS",EFFECT_DETECT_DOOR_STAIRS,10,  25,  1, 0},
    {"DETECT_EVIL",     EFFECT_DETECT_EVIL,         20,  30,  1, BIAS_PRIESTLY},

    /* Utility:                                     Lv    T   R  Bias */
    {"PHASE_DOOR",      EFFECT_PHASE_DOOR,           8,  20,  1, BIAS_MAGE | BIAS_ARCHER},
    {"TELEPORT",        EFFECT_TELEPORT,            12,  20,  1, BIAS_ROGUE},
    {"TELEPORT_AWAY",   EFFECT_TELEPORT_AWAY,       20,  50,  2, BIAS_MAGE},
    {"STRAFING",        EFFECT_STRAFING,            20,  10,  3, BIAS_ARCHER},
    {"DIMENSION_DOOR",  EFFECT_DIMENSION_DOOR,      50, 100,  8, BIAS_MAGE | BIAS_ARCHER},
    {"ESCAPE",          EFFECT_ESCAPE,              30,  70,  3, BIAS_ROGUE},
    {"RECALL",          EFFECT_RECALL,              25, 100,  2, BIAS_MAGE},

    {"STONE_TO_MUD",    EFFECT_STONE_TO_MUD,        15,  20,  1, BIAS_STR | BIAS_RANGER},
    {"EARTHQUAKE",      EFFECT_EARTHQUAKE,          25, 100,  1, BIAS_STR},
    {"DESTRUCTION",     EFFECT_DESTRUCTION,         50, 250,  6, BIAS_DEMON},
    {"GENOCIDE",        EFFECT_GENOCIDE,            70, 500, 16, BIAS_NECROMANTIC},
    {"MASS_GENOCIDE",   EFFECT_MASS_GENOCIDE,       80, 750, 16, BIAS_NECROMANTIC},

    {"RECHARGE_FROM_DEVICE", EFFECT_RECHARGE_FROM_DEVICE, 35, 500,  3, BIAS_MAGE | BIAS_DEMON},
    {"RECHARGE_FROM_PLAYER", EFFECT_RECHARGE_FROM_PLAYER, 90, 500, 16, BIAS_MAGE | BIAS_DEMON},

    {"ENCHANTMENT",     EFFECT_ENCHANTMENT,         30, 900, 16, 0},
    {"IDENTIFY",        EFFECT_IDENTIFY,            15,  50,  1, BIAS_ROGUE | BIAS_MAGE},
    {"IDENTIFY_FULL",   EFFECT_IDENTIFY_FULL,       50, 200,  3, BIAS_ROGUE | BIAS_MAGE},
    {"PROBING",         EFFECT_PROBING,             30,  50,  1, BIAS_MAGE},
    {"RUNE_EXPLOSIVE",  EFFECT_RUNE_EXPLOSIVE,      30, 100,  2, BIAS_MAGE},
    {"RUNE_PROTECTION", EFFECT_RUNE_PROTECTION,     70, 500,  4, BIAS_PRIESTLY},

    {"SATISFY_HUNGER",  EFFECT_SATISFY_HUNGER,       5, 100,  1, BIAS_RANGER},
    {"DESTROY_TRAP",    EFFECT_DESTROY_TRAP,        20,  50,  1, 0},
    {"DESTROY_TRAPS",   EFFECT_DESTROY_TRAPS,       25,  50,  1, BIAS_ROGUE},
    {"WHIRLWIND_ATTACK",EFFECT_WHIRLWIND_ATTACK,    50, 100,  4, BIAS_WARRIOR},
    {"LIST_UNIQUES",    EFFECT_LIST_UNIQUES,        80, 250,  8, 0},
    {"LIST_ARTIFACTS",  EFFECT_LIST_ARTIFACTS,      80, 250,  8, 0},
    {"BANISH_EVIL",     EFFECT_BANISH_EVIL,         50, 100,  4, BIAS_PRIESTLY | BIAS_LAW},
    {"BANISH_ALL",      EFFECT_BANISH_ALL,          50, 100,  8, BIAS_MAGE},
    {"TELEKINESIS",     EFFECT_TELEKINESIS,         25, 100,  2, BIAS_MAGE},
    {"ALCHEMY",         EFFECT_ALCHEMY,             70, 100,  4, BIAS_MAGE},
    {"SELF_KNOWLEDGE",  EFFECT_SELF_KNOWLEDGE,      70, 500,  3, BIAS_MAGE},
    {"GENOCIDE_ONE",    EFFECT_GENOCIDE_ONE,        60, 250,  8, BIAS_NECROMANTIC},

    /* Timed Buffs:                                 Lv    T   R  Bias */
    {"STONE_SKIN",      EFFECT_STONE_SKIN,          25, 150,  2, BIAS_WARRIOR | BIAS_PROTECTION},
    {"RESIST_ACID",     EFFECT_RESIST_ACID,         15, 100,  1, BIAS_ACID | BIAS_PROTECTION},
    {"RESIST_ELEC",     EFFECT_RESIST_ELEC,         15, 100,  1, BIAS_ELEC | BIAS_PROTECTION},
    {"RESIST_FIRE",     EFFECT_RESIST_FIRE,         15, 100,  1, BIAS_FIRE | BIAS_DEMON | BIAS_PROTECTION},
    {"RESIST_COLD",     EFFECT_RESIST_COLD,         15, 100,  1, BIAS_COLD | BIAS_PROTECTION},
    {"RESIST_POIS",     EFFECT_RESIST_POIS,         30, 150,  2, BIAS_POIS | BIAS_PROTECTION},
    {"RESISTANCE",      EFFECT_RESISTANCE,          35, 200,  4, BIAS_RANGER | BIAS_PROTECTION},
    {"PROT_EVIL",       EFFECT_PROT_EVIL,           35, 200,  2, BIAS_PRIESTLY | BIAS_LAW | BIAS_PROTECTION},
    {"HOLY_GRAIL",      EFFECT_HOLY_GRAIL,          50, 500,  4, BIAS_PRIESTLY},
    {"BLESS",           EFFECT_BLESS,               10, 100,  1, BIAS_PRIESTLY | BIAS_DEMON},
    {"HEROISM",         EFFECT_HEROISM,             15, 100,  1, BIAS_WARRIOR | BIAS_PRIESTLY | BIAS_DEMON},
    {"BERSERK",         EFFECT_BERSERK,             20, 100,  2, BIAS_WARRIOR},
    {"SPEED",           EFFECT_SPEED,               25, 150,  4, BIAS_ROGUE | BIAS_MAGE | BIAS_ARCHER},
    {"SPEED_HERO",      EFFECT_SPEED_HERO,          35, 200,  6, BIAS_WARRIOR},
    {"SPEED_HERO_BLESS",EFFECT_SPEED_HERO_BLESS,    40, 250,  8, 0},
    {"LIGHT_SPEED",     EFFECT_LIGHT_SPEED,         99, 999, 99, 0},
    {"ENLARGE_WEAPON",  EFFECT_ENLARGE_WEAPON,      80, 900,  0, 0},
    {"TELEPATHY",       EFFECT_TELEPATHY,           30, 150,  8, BIAS_MAGE | BIAS_ARCHER},
    {"WRAITHFORM",      EFFECT_WRAITHFORM,          90, 666, 16, BIAS_NECROMANTIC},
    {"INVULNERABILITY", EFFECT_INVULNERABILITY,     90, 777, 16, BIAS_MAGE | BIAS_PROTECTION},

    /* Pets:                                        Lv    T   R  Bias */
    {"SUMMON_MONSTERS", EFFECT_SUMMON_MONSTERS,     30, 500,  1, BIAS_MAGE},
    {"SUMMON_HOUNDS",   EFFECT_SUMMON_HOUNDS,       35, 500,  1, BIAS_RANGER},
    {"SUMMON_ANTS",     EFFECT_SUMMON_ANTS,         20, 200,  1, BIAS_RANGER},
    {"SUMMON_HYDRAS",   EFFECT_SUMMON_HYDRAS,       40, 500,  1, BIAS_RANGER},
    {"SUMMON_OCTOPUS",  EFFECT_SUMMON_OCTOPUS,      30, 600, 16, 0},
    {"SUMMON_DAWN",     EFFECT_SUMMON_DAWN,         60, 888, 16, 0},
    {"SUMMON_PHANTASMAL",EFFECT_SUMMON_PHANTASMAL,  25, 150,  1, BIAS_MAGE},
    {"SUMMON_ELEMENTAL",EFFECT_SUMMON_ELEMENTAL,    30, 150,  1, BIAS_MAGE},
    {"SUMMON_DRAGON",   EFFECT_SUMMON_DRAGON,       60, 600,  2, BIAS_MAGE},
    {"SUMMON_UNDEAD",   EFFECT_SUMMON_UNDEAD,       60, 600,  2, BIAS_NECROMANTIC},
    {"SUMMON_DEMON",    EFFECT_SUMMON_DEMON,        66, 666,  2, BIAS_CHAOS | BIAS_DEMON},
    {"SUMMON_CYBERDEMON",EFFECT_SUMMON_CYBERDEMON,  90, 900, 16, BIAS_CHAOS},
    {"SUMMON_ANGEL",    EFFECT_SUMMON_ANGEL,        70, 777,  8, BIAS_PRIESTLY},
    {"SUMMON_KRAKEN",   EFFECT_SUMMON_KRAKEN,       90, 900,  8, 0},

    {"CHARM_ANIMAL",    EFFECT_CHARM_ANIMAL,        30, 200,  1, BIAS_RANGER},
    {"CHARM_DEMON",     EFFECT_CHARM_DEMON,         50, 500,  1, BIAS_CHAOS | BIAS_DEMON},
    {"CHARM_UNDEAD",    EFFECT_CHARM_UNDEAD,        50, 500,  1, BIAS_NECROMANTIC},
    {"CHARM_MONSTER",   EFFECT_CHARM_MONSTER,       30, 100,  1, BIAS_MAGE},

    {"RETURN_PETS",     EFFECT_RETURN_PETS,         10,   0,  0, 0},
    {"CAPTURE_PET",     EFFECT_CAPTURE_PET,         20,   0,  0, 0},

    /* Healing and Recovery:                        Lv    T   R  Bias */
    {"RESTORE_STATS",   EFFECT_RESTORE_STATS,       50, 600,  2, BIAS_PRIESTLY},
    {"RESTORE_EXP",     EFFECT_RESTORE_EXP,         40, 500,  1, BIAS_PRIESTLY},
    {"RESTORING",       EFFECT_RESTORING,           70, 800,  3, BIAS_PRIESTLY},
    {"HEAL",            EFFECT_HEAL,                40, 500,  1, BIAS_PRIESTLY},
    {"CURING",          EFFECT_CURING,              45, 200,  1, BIAS_PRIESTLY},
    {"HEAL_CURING",     EFFECT_HEAL_CURING,         60, 900,  4, BIAS_PRIESTLY},
    {"HEAL_CURING_HERO",EFFECT_HEAL_CURING_HERO,    70, 900,  4, BIAS_PRIESTLY},
    {"RESTORE_MANA",    EFFECT_RESTORE_MANA,        80, 900,  8, BIAS_MAGE},
    {"CURE_POIS",       EFFECT_CURE_POIS,           10,  50,  1, BIAS_POIS | BIAS_PRIESTLY | BIAS_RANGER},
    {"CURE_FEAR",       EFFECT_CURE_FEAR,           25, 100,  1, BIAS_PRIESTLY},
    {"CURE_FEAR_POIS",  EFFECT_CURE_FEAR_POIS,      30, 100,  1, BIAS_PRIESTLY},
    {"REMOVE_CURSE",    EFFECT_REMOVE_CURSE,        30, 200,  1, BIAS_PRIESTLY},
    {"REMOVE_ALL_CURSE",EFFECT_REMOVE_ALL_CURSE,    70, 500,  4, BIAS_PRIESTLY},
    {"CLARITY",         EFFECT_CLARITY,             20,  15, 12, BIAS_PRIESTLY | BIAS_MAGE},
    {"GREAT_CLARITY",   EFFECT_GREAT_CLARITY,       80,  75, 64, BIAS_PRIESTLY | BIAS_MAGE},

    /* Offense: Bolts                               Lv    T   R  Bias */
    {"BOLT_MISSILE",    EFFECT_BOLT_MISSILE,         1,  10,  1, BIAS_MAGE | BIAS_ARCHER},
    {"BOLT_ACID",       EFFECT_BOLT_ACID,           15,  20,  1, BIAS_ACID},
    {"BOLT_ELEC",       EFFECT_BOLT_ELEC,           15,  20,  1, BIAS_ELEC},
    {"BOLT_FIRE",       EFFECT_BOLT_FIRE,           15,  20,  1, BIAS_FIRE | BIAS_DEMON},
    {"BOLT_COLD",       EFFECT_BOLT_COLD,           15,  20,  1, BIAS_COLD},
    {"BOLT_POIS",       EFFECT_BOLT_POIS,           10,  10,  1, BIAS_POIS},
    {"BOLT_LITE",       EFFECT_BOLT_LITE,           20,  25,  1, 0},
    {"BOLT_DARK",       EFFECT_BOLT_DARK,           20,  25,  1, BIAS_NECROMANTIC},
    {"BOLT_CONF",       EFFECT_BOLT_CONF,           30,  50,  2, 0},
    {"BOLT_NETHER",     EFFECT_BOLT_NETHER,         20,  25,  1, BIAS_NECROMANTIC | BIAS_DEMON},
    {"BOLT_NEXUS",      EFFECT_BOLT_NEXUS,          20,  25,  2, 0},
    {"BOLT_SOUND",      EFFECT_BOLT_SOUND,          40,  50,  2, BIAS_LAW},
    {"BOLT_SHARDS",     EFFECT_BOLT_SHARDS,         40,  50,  2, BIAS_LAW},
    {"BOLT_CHAOS",      EFFECT_BOLT_CHAOS,          50, 100,  2, BIAS_CHAOS},
    {"BOLT_DISEN",      EFFECT_BOLT_DISEN,          40,  50,  2, 0},
    {"BOLT_TIME",       EFFECT_BOLT_TIME,           60, 200, 90, 0},
    {"BOLT_WATER",      EFFECT_BOLT_WATER,          55, 150,  4, BIAS_MAGE},
    {"BOLT_MANA",       EFFECT_BOLT_MANA,           50, 100,  4, BIAS_MAGE},
    {"BOLT_ICE",        EFFECT_BOLT_ICE,            50, 100,  4, BIAS_MAGE | BIAS_COLD},
    {"BOLT_PLASMA",     EFFECT_BOLT_PLASMA,         50, 100,  4, BIAS_FIRE},

    /* Offense: Beams                               Lv    T   R  Bias */
    {"BEAM_LITE_WEAK",  EFFECT_BEAM_LITE_WEAK,      10,  20,  1, 0},
    {"BEAM_LITE",       EFFECT_BEAM_LITE,           40, 100,  2, 0},
    {"BEAM_GRAVITY",    EFFECT_BEAM_GRAVITY,        50, 150,  8, 0},
    {"BEAM_DISINTEGRATE",EFFECT_BEAM_DISINTEGRATE,  60, 200, 16, 0},
    {"BEAM_ACID",       EFFECT_BEAM_ACID,           20,  20,  2, BIAS_ACID},
    {"BEAM_ELEC",       EFFECT_BEAM_ELEC,           20,  20,  2, BIAS_ELEC},
    {"BEAM_FIRE",       EFFECT_BEAM_FIRE,           20,  20,  2, BIAS_FIRE | BIAS_DEMON},
    {"BEAM_COLD",       EFFECT_BEAM_COLD,           20,  20,  2, BIAS_COLD},
    {"BEAM_SOUND",      EFFECT_BEAM_SOUND,          45,  50,  3, BIAS_LAW},
    {"BEAM_CHAOS",      EFFECT_BEAM_CHAOS,          55, 100,  3, BIAS_CHAOS},

    /* Offense: Balls                               Lv    T   R  Bias */
    {"BALL_ACID",       EFFECT_BALL_ACID,           25,  50,  1, BIAS_ACID},
    {"BALL_ELEC",       EFFECT_BALL_ELEC,           25,  50,  1, BIAS_ELEC},
    {"BALL_FIRE",       EFFECT_BALL_FIRE,           25,  50,  1, BIAS_FIRE | BIAS_DEMON},
    {"BALL_COLD",       EFFECT_BALL_COLD,           25,  50,  1, BIAS_COLD},
    {"BALL_POIS",       EFFECT_BALL_POIS,           10,   5,  1, BIAS_POIS},
    {"BALL_LITE",       EFFECT_BALL_LITE,           65, 100,  2, 0},
    {"BALL_DARK",       EFFECT_BALL_DARK,           66, 100,  2, BIAS_NECROMANTIC},
    {"BALL_CONF",       EFFECT_BALL_CONF,           50, 150,  2, 0},
    {"BALL_NETHER",     EFFECT_BALL_NETHER,         40,  50,  2, BIAS_NECROMANTIC | BIAS_DEMON},
    {"BALL_NEXUS",      EFFECT_BALL_NEXUS,          50, 100,  2, 0},
    {"BALL_SOUND",      EFFECT_BALL_SOUND,          60, 100,  2, BIAS_LAW},
    {"BALL_SHARDS",     EFFECT_BALL_SHARDS,         60, 100,  2, BIAS_LAW},
    {"BALL_CHAOS",      EFFECT_BALL_CHAOS,          65, 150,  4, BIAS_CHAOS},
    {"BALL_DISEN",      EFFECT_BALL_DISEN,          55, 100,  2, 0},
    {"BALL_TIME",       EFFECT_BALL_TIME,           80, 250, 32, 0},
    {"BALL_WATER",      EFFECT_BALL_WATER,          70, 200,  4, BIAS_MAGE},
    {"BALL_MANA",       EFFECT_BALL_MANA,           80, 200,  6, BIAS_MAGE},
    {"BALL_DISINTEGRATE", EFFECT_BALL_DISINTEGRATE, 60, 200, 16, 0},

    /* Offense: Breaths                             Lv    T   R  Bias */
    {"BREATHE_ACID",    EFFECT_BREATHE_ACID,        40, 100,  2, BIAS_ACID},
    {"BREATHE_ELEC",    EFFECT_BREATHE_ELEC,        40, 100,  2, BIAS_ELEC},
    {"BREATHE_FIRE",    EFFECT_BREATHE_FIRE,        40, 100,  2, BIAS_FIRE},
    {"BREATHE_COLD",    EFFECT_BREATHE_COLD,        40, 100,  2, BIAS_COLD},
    {"BREATHE_POIS",    EFFECT_BREATHE_POIS,        40, 100,  2, BIAS_POIS},
    {"BREATHE_LITE",    EFFECT_BREATHE_LITE,        50, 125,  3, 0},
    {"BREATHE_DARK",    EFFECT_BREATHE_DARK,        50, 125,  3, BIAS_NECROMANTIC},
    {"BREATHE_CONF",    EFFECT_BREATHE_CONF,        60, 200,  4, 0},
    {"BREATHE_NETHER",  EFFECT_BREATHE_NETHER,      50,  75,  2, BIAS_NECROMANTIC | BIAS_DEMON},
    {"BREATHE_NEXUS",   EFFECT_BREATHE_NEXUS,       60, 150,  4, 0},
    {"BREATHE_SOUND",   EFFECT_BREATHE_SOUND,       70, 200,  4, BIAS_LAW},
    {"BREATHE_SHARDS",  EFFECT_BREATHE_SHARDS,      70, 200,  4, BIAS_LAW},
    {"BREATHE_CHAOS",   EFFECT_BREATHE_CHAOS,       75, 250,  4, BIAS_CHAOS},
    {"BREATHE_DISEN",   EFFECT_BREATHE_DISEN,       60, 150,  8, 0},
	{"BREATHE_INERTIA", EFFECT_BREATHE_INERTIA,     60, 200,  8, 0},
	{"BREATHE_WATER",   EFFECT_BREATHE_WATER,       65, 150,  8, BIAS_MAGE },
    {"BREATHE_TIME",    EFFECT_BREATHE_TIME,        90, 500, 32, 0},
    {"BREATHE_ELEMENTS", EFFECT_BREATHE_ELEMENTS,   60, 100, 64, 0},

    {"BREATHE_ONE_MULTIHUED", EFFECT_BREATHE_ONE_MULTIHUED, 0, 0, 0, 0},
    {"BREATHE_ONE_CHAOS",EFFECT_BREATHE_ONE_CHAOS,   0, 0, 0, 0},
    {"BREATHE_ONE_LAW", EFFECT_BREATHE_ONE_LAW,      0, 0, 0, 0},
    {"BREATHE_ONE_BALANCE",EFFECT_BREATHE_ONE_BALANCE, 0, 0, 0, 0},
    {"BREATHE_ONE_SHINING",EFFECT_BREATHE_ONE_SHINING, 0, 0, 0, 0},

    /* Offense: Other                               Lv    T   R  Bias */
    {"DISPEL_EVIL",     EFFECT_DISPEL_EVIL,         50, 200,  2, BIAS_PRIESTLY | BIAS_LAW},
    {"DISPEL_EVIL_HERO",EFFECT_DISPEL_EVIL_HERO,    60, 250,  3, BIAS_PRIESTLY},
    {"DISPEL_GOOD",     EFFECT_DISPEL_GOOD,         50, 150,  1, BIAS_NECROMANTIC},
    {"DISPEL_LIFE",     EFFECT_DISPEL_LIFE,         55, 200,  1, BIAS_NECROMANTIC},
    {"DISPEL_DEMON",    EFFECT_DISPEL_DEMON,        60, 200,  2, BIAS_LAW},
    {"DISPEL_UNDEAD",   EFFECT_DISPEL_UNDEAD,       60, 200,  2, BIAS_PRIESTLY},
    {"DISPEL_MONSTERS", EFFECT_DISPEL_MONSTERS,     70, 250,  8, 0},
    {"DRAIN_LIFE",      EFFECT_DRAIN_LIFE,          40, 100,  2, BIAS_NECROMANTIC},
    {"STAR_BALL",       EFFECT_STAR_BALL,           80, 900, 64, BIAS_LAW},
    {"ROCKET",          EFFECT_ROCKET,              70, 200,  8, BIAS_DEMON},
    {"MANA_STORM",      EFFECT_MANA_STORM,          80, 250,  8, BIAS_MAGE},
    {"CONFUSING_LITE",  EFFECT_CONFUSING_LITE,      60, 100,  6, BIAS_CHAOS},
    {"ARROW",           EFFECT_ARROW,               30, 100,  2, BIAS_RANGER | BIAS_ARCHER},
    {"WRATH_OF_GOD",    EFFECT_WRATH_OF_GOD,        80, 250, 32, BIAS_LAW},
    {"METEOR",          EFFECT_METEOR,              55, 150,  8, 0},
    {"HOLINESS",        EFFECT_HOLINESS,            50, 250,  8, BIAS_PRIESTLY | BIAS_LAW},
    {"STARBURST",       EFFECT_STARBURST,           70, 500, 32, BIAS_LAW},
    {"DARKNESS_STORM",  EFFECT_DARKNESS_STORM,      70, 500, 32, BIAS_NECROMANTIC},
    {"PESTICIDE",       EFFECT_PESTICIDE,           10,   5,  8, 0},

    /* Misc                                         Lv    T   R  Bias */
    {"POLY_SELF",       EFFECT_POLY_SELF,           20, 500,  1, BIAS_CHAOS},
    {"ANIMATE_DEAD",    EFFECT_ANIMATE_DEAD,        25, 100,  1, BIAS_NECROMANTIC},
    {"SCARE_MONSTERS",  EFFECT_SCARE_MONSTERS,      20, 100,  1, BIAS_NECROMANTIC},
    {"SLEEP_MONSTERS",  EFFECT_SLEEP_MONSTERS,      25, 100,  1, BIAS_ROGUE},
    {"SLOW_MONSTERS",   EFFECT_SLOW_MONSTERS,       25, 100,  1, BIAS_RANGER},
    {"STASIS_MONSTERS", EFFECT_STASIS_MONSTERS,     50, 250,  8, BIAS_MAGE},
    {"CONFUSE_MONSTERS",EFFECT_CONFUSE_MONSTERS,    25, 100,  1, BIAS_CHAOS},
    {"FISHING",         EFFECT_FISHING,             10,   0,  0, 0},
    {"PIERCING_SHOT",   EFFECT_PIERCING_SHOT,       30, 100,  0, BIAS_ARCHER},
    {"CHARGE",          EFFECT_CHARGE,              15, 100,  0, 0},
    {"WALL_BUILDING",   EFFECT_WALL_BUILDING,       90, 750,  0, 0},
    {"SLEEP_MONSTER",   EFFECT_SLEEP_MONSTER,        5, 100,  1, 0},
    {"SLOW_MONSTER",    EFFECT_SLOW_MONSTER,         5, 100,  1, 0},
    {"CONFUSE_MONSTER", EFFECT_CONFUSE_MONSTER,      5, 100,  1, 0},
    {"SCARE_MONSTER",   EFFECT_SCARE_MONSTER,       10, 100,  1, 0},
    {"POLYMORPH",       EFFECT_POLYMORPH,           15, 100,  2, BIAS_CHAOS},
    {"STARLITE",        EFFECT_STARLITE,            20, 100,  2, 0},
    {"NOTHING",         EFFECT_NOTHING,              1,   1,  0, 0},
    {"ENDLESS_QUIVER",  EFFECT_ENDLESS_QUIVER,      50, 150,  0, BIAS_ARCHER},

    /* Bad Effects                                  Lv    T   R  Bias */
    {"AGGRAVATE",       EFFECT_AGGRAVATE,           10, 100,  1, BIAS_DEMON},
    {"HEAL_MONSTER",    EFFECT_HEAL_MONSTER,         2,  50,  0, 0},
    {"HASTE_MONSTER",   EFFECT_HASTE_MONSTER,       20, 100,  0, 0},
    {"HASTE_MONSTERS",  EFFECT_HASTE_MONSTERS,      10,  50,  0, 0},
    {"CLONE_MONSTER",   EFFECT_CLONE_MONSTER,       15, 100,  0, 0},
    {"DARKNESS",        EFFECT_DARKNESS,             5,  10,  0, 0},
    {"SUMMON_ANGRY_MONSTERS",
                        EFFECT_SUMMON_ANGRY_MONSTERS,10,200,  0, 0},
    {"SLOWNESS",        EFFECT_SLOWNESS,             5,  10,  0, 0},

    /* Specific Artifacts                           Lv    T   R  Bias */
    {"JEWEL",           EFFECT_JEWEL,                0,   0,  0, 0},
    {"HERMES",          EFFECT_HERMES,               0,   0,  0, 0},
    {"ARTEMIS",         EFFECT_ARTEMIS,              0,   0,  0, 0},
    {"DEMETER",         EFFECT_DEMETER,              0,   0,  0, 0},
    {"EYE_VECNA",       EFFECT_EYE_VECNA,            0,   0,  0, 0},
    {"ONE_RING",        EFFECT_ONE_RING,             0,   0,  0, 0},
    {"BLADETURNER",     EFFECT_BLADETURNER,          0,   0,  0, 0},
    {"MITO_KOUMON",     EFFECT_MITO_KOUMON,          0,   0,  0, 0},
    {"BLOODY_MOON",     EFFECT_BLOODY_MOON,          0,   0,  0, 0},
    {"SACRED_KNIGHTS",  EFFECT_SACRED_KNIGHTS,       0,   0,  0, 0},
    {"GONG",            EFFECT_GONG,                 0,   0,  0, 0},
    {"MURAMASA",        EFFECT_MURAMASA,             0,   0,  0, 0},
    {"EXPERTSEXCHANGE", EFFECT_EXPERTSEXCHANGE,      0,   0,  0, 0},
    {"EYE_HYPNO",       EFFECT_EYE_HYPNO,            0,   0,  0, 0},
    {"STUNNING_KICK",   EFFECT_STUNNING_KICK,        0,   0,  0, 0},
    {"RAMA_ARROW",      EFFECT_RAMA_ARROW,           0,   0,  0, 0},
    {"UNFOCUS_RAGE",    EFFECT_UNFOCUS_RAGE,         0,   0,  0, 0},

    {0}
};

_effect_info_ptr _get_effect_info(int type)
{
    int i;
    for (i = 0; ; i++)
    {
        _effect_info_ptr e = &_effect_info[i];
        if (!e->type) break;
        if (e->type == type) return e;
    }
    return NULL;
}

bool effect_is_known(int type)
{
    _effect_info_ptr e = _get_effect_info(type);
    if (e)
        return e->known;
    return FALSE;
}

bool effect_learn(int type)
{
    _effect_info_ptr e = _get_effect_info(type);
    if (e && !e->known)
    {
        e->known = TRUE;
        return TRUE;
    }
    return FALSE;
}

int effect_parse_type(cptr type)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!_effect_info[i].text) break;
        if (streq(type, _effect_info[i].text))
            return _effect_info[i].type;
    }
    return EFFECT_NONE;
}

errr effect_parse(char *line, effect_t *effect) /* LITE_AREA:<Lvl>:<Timeout>:<Extra> */
{
    char *tokens[5];
    int   num = tokenize(line, 5, tokens, 0);
    int   i;

    if (num < 1) return PARSE_ERROR_TOO_FEW_ARGUMENTS;

    WIPE(effect, effect_t);

    switch (num)
    {
    case 4: effect->extra = atoi(tokens[3]);
    case 3: effect->cost = atoi(tokens[2]);
    case 2: effect->power = atoi(tokens[1]);
            effect->difficulty = effect->power;
    case 1:
        for (i = 0; ; i++)
        {
            if (!_effect_info[i].text) break;
            if (streq(tokens[0], _effect_info[i].text))
            {
                effect->type = _effect_info[i].type;
                break;
            }
        }
    }
    if (!effect->type) return 1;
    return 0;
}

static int _choose_random_p(effect_p p)
{
    int i, n;
    int tot = 0;

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (!_effect_info[i].rarity) continue;
        if (p && !p(_effect_info[i].type)) continue;

        tot += MAX(255 / _effect_info[i].rarity, 1);
    }

    if (!tot) return -1;
    n = randint1(tot);

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (!_effect_info[i].rarity) continue;
        if (p && !p(_effect_info[i].type)) continue;

        n -= MAX(255 / _effect_info[i].rarity, 1);
        if (n <= 0) return i;
    }
    return -1;
}

static int _choose_random(int bias)
{
    int i, n;
    int tot = 0;

/*  if (one_in_(3)) bias = 0; */

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].level < object_level / 3) continue;
        if (bias && !(_effect_info[i].bias & bias)) continue;
        if (!_effect_info[i].rarity) continue;

        tot += MAX(255 / _effect_info[i].rarity, 1);
    }

    if (!tot) return -1;
    n = randint1(tot);

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].level < object_level / 3) continue;
        if (bias && !(_effect_info[i].bias & bias)) continue;
        if (!_effect_info[i].rarity) continue;

        n -= MAX(255 / _effect_info[i].rarity, 1);
        if (n <= 0) return i;
    }
    return -1;
}

static void _add_index(object_type *o_ptr, int index)
{
    if (index >= 0)
    {
        o_ptr->activation.type = _effect_info[index].type;
        o_ptr->activation.power = _effect_info[index].level;
        o_ptr->activation.difficulty = _effect_info[index].level;
        o_ptr->activation.cost = _effect_info[index].cost;
        o_ptr->activation.extra = 0;
        o_ptr->timeout = 0;
        add_flag(o_ptr->flags, OF_ACTIVATE); /* for object lore */
    }
}

bool effect_add_random_p(object_type *o_ptr, effect_p p)
{
    int i = _choose_random_p(p);
    if (i >= 0)
    {
        _add_index(o_ptr, i);
        return TRUE;
    }
    return FALSE;
}

bool effect_add_random(object_type *o_ptr, int bias)
{
    int i = _choose_random(bias);
    if (i >= 0)
    {
        _add_index(o_ptr, i);
        return TRUE;
    }
    return FALSE;
}

bool effect_add(object_type *o_ptr, int type)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].type == type)
        {
            _add_index(o_ptr, i);
            return TRUE;
        }
    }
    return FALSE;
}

/***********************************************************************
 * Redoing Devices (Wands, Staves and Rods)
 ***********************************************************************/

#define _DROP_GOOD       0x0001
#define _DROP_GREAT      0x0002
#define _NO_DESTROY      0x0004
#define _STOCK_TOWN      0x0008
#define _COMMON          0x0010
#define _RARE            0x0020

device_effect_info_t wand_effect_table[] =
{
    /*                            Lvl Cost Rarity  Max  Difficulty Flags */
    {EFFECT_BOLT_MISSILE,           1,   3,     1,  20,    10,  0, _STOCK_TOWN},
    {EFFECT_HEAL_MONSTER,           2,   3,     1,  50,     0,  0, 0},
    {EFFECT_BEAM_LITE_WEAK,         2,   3,     1,  20,    10,  0, _STOCK_TOWN},
    {EFFECT_BALL_POIS,              5,   4,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_SLEEP_MONSTER,          5,   5,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_SLOW_MONSTER,           5,   5,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_CONFUSE_MONSTER,        5,   5,     1,  25,    33,  0, _STOCK_TOWN},
    {EFFECT_SCARE_MONSTER,          7,   5,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_STONE_TO_MUD,          10,   5,     1,   0,    10,  0, _COMMON},
    {EFFECT_POLYMORPH,             12,   6,     1,  30,     0,  0, 0},
    {EFFECT_BOLT_COLD,             12,   7,     1,  30,    33,  0, _STOCK_TOWN},
    {EFFECT_BOLT_ELEC,             15,   7,     1,  30,    33,  0, _STOCK_TOWN},
    {EFFECT_BOLT_ACID,             17,   8,     1,  35,    33,  0, _STOCK_TOWN},
    {EFFECT_BOLT_FIRE,             19,   9,     1,  35,    33,  0, _STOCK_TOWN},
    {EFFECT_HASTE_MONSTER,         20,   3,     1,  50,     0,  0, 0},
    {EFFECT_TELEPORT_AWAY,         20,  10,     1,   0,    10,  0, _COMMON},
    {EFFECT_DESTROY_TRAPS,         20,  10,     1,   0,    10,  0, 0},
    {EFFECT_CHARM_MONSTER,         25,  11,     1,  50,    33,  0, 0},
    {EFFECT_BALL_COLD,             26,   5,     1,   0,    50, 10, 0},
    {EFFECT_BALL_ELEC,             28,   5,     1,   0,    50, 10, 0},
    {EFFECT_BALL_ACID,             29,   7,     1,   0,    50, 10, 0},
    {EFFECT_BALL_FIRE,             30,   8,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_WATER,            30,   9,     1,   0,    50, 10, 0},
    {EFFECT_DRAIN_LIFE,            32,  20,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_PLASMA,           38,  10,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_ICE,              40,  11,     1,   0,    50, 10, 0},
    {EFFECT_ARROW,                 45,  13,     1,   0,    50, 10, 0},
    {EFFECT_BALL_NEXUS,            47,  14,     1,   0,    50, 10, _DROP_GOOD},
    {EFFECT_BREATHE_COLD,          50,  15,     1,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BREATHE_FIRE,          50,  16,     1,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
	{EFFECT_BREATHE_WATER,         50,  16,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BEAM_GRAVITY,          55,  32,     2,   0,    33,  0, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_METEOR,                55,  32,     2,   0,    50, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BREATHE_ONE_MULTIHUED, 60,  17,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_GENOCIDE_ONE,          65,  35,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BALL_WATER,            70,  20,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BALL_DISINTEGRATE,     75,  20,     2,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_ROCKET,                85,  20,     3,   0,    70, 20, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_WALL_BUILDING,        100,  50,    16,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {0}
};

device_effect_info_t rod_effect_table[] =
{
    /*                            Lvl Cost Rarity  Max  Difficulty Flags */
    {EFFECT_PESTICIDE,              1,   7,     1,  30,    10,  0, 0},
    {EFFECT_DETECT_TRAPS,           5,   9,     1,  30,    10,  0, 0},
    {EFFECT_LITE_AREA,             10,  10,     1,  40,    10,  0, 0},
    {EFFECT_DETECT_DOOR_STAIRS,    12,  10,     1,  40,    10,  0, 0},
    {EFFECT_DETECT_MONSTERS,       15,  10,     1,  40,    10,  0, 0},
    {EFFECT_BEAM_ELEC,             17,   8,     1,  50,    33,  0, 0},
    {EFFECT_BEAM_COLD,             19,   8,     1,  50,    33,  0, 0},
    {EFFECT_BEAM_FIRE,             21,   9,     1,  60,    33,  0, 0},
    {EFFECT_BEAM_ACID,             23,   9,     1,  60,    33,  0, 0},
    {EFFECT_BEAM_LITE,             25,  12,     2,   0,    50, 10, 0},
    {EFFECT_RECALL,                27,  15,     1,   0,    10,  0, 0},
    {EFFECT_DETECT_ALL,            30,  17,     2,   0,    10,  0, _COMMON},
    {EFFECT_ESCAPE,                30,  20,     1,   0,    10,  0, 0},
    {EFFECT_BEAM_CHAOS,            32,  12,     2,  60,    33,  0, 0},
    {EFFECT_BEAM_SOUND,            32,  12,     2,  70,    33,  0, 0},
    {EFFECT_CLARITY,               35,  15,     3,  80,    33,  0, _DROP_GOOD},
    {EFFECT_BALL_ELEC,             40,  13,     1,   0,    50, 10, 0},
    {EFFECT_BALL_COLD,             40,  14,     1,   0,    50, 10, 0},
    {EFFECT_BALL_FIRE,             42,  15,     1,   0,    50, 10, 0},
    {EFFECT_BALL_ACID,             44,  16,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_MANA,             45,  17,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_BALL_NETHER,           45,  18,     1,   0,    50, 10, 0},
    {EFFECT_BALL_DISEN,            47,  19,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_ENLIGHTENMENT,         50,  33,     2,   0,    10,  0, _COMMON},
    {EFFECT_BALL_SOUND,            52,  22,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_BEAM_DISINTEGRATE,     60,  37,     2,   0,    33,  0, _DROP_GOOD},
    {EFFECT_SPEED_HERO,            70,  40,     2,   0,    10,  0, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_GREAT_CLARITY,         75,  60,     4,   0,    50, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_HEAL_CURING_HERO,      80,  60,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_RESTORING,             80,  60,     3,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _RARE},
    {EFFECT_BALL_MANA,             80,  24,     2,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_BALL_SHARDS,           80,  25,     2,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_BALL_CHAOS,            85,  27,     3,   0,    70, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_CLAIRVOYANCE,          90, 100,     3,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _RARE},
    {EFFECT_BALL_LITE,             95,  27,     3,   0,    70, 10, _DROP_GOOD | _DROP_GREAT},
    {0}
};

device_effect_info_t staff_effect_table[] =
{
    /*                            Lvl Cost Rarity  Max  Difficulty Flags */
    {EFFECT_NOTHING,                1,   1,     0,   0,     0,  0, 0},
    {EFFECT_DARKNESS,               1,   3,     1,  15,     0,  0, 0},
    {EFFECT_LITE_AREA,              1,   3,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_GOLD,            5,   4,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_OBJECTS,         5,   4,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_INVISIBLE,       5,   4,     1,  30,    10,  0, 0},
    {EFFECT_DETECT_TRAPS,           5,   5,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_DOOR_STAIRS,     5,   5,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_EVIL,            7,   5,     1,  30,    10,  0, 0},
    {EFFECT_HASTE_MONSTERS,        10,   5,     1,  30,    50, 10, 0},
    {EFFECT_SUMMON_ANGRY_MONSTERS, 10,   5,     1,  30,    50, 10, 0},
    {EFFECT_IDENTIFY,              10,   4,     1,   0,    10,  0, _STOCK_TOWN | _COMMON},
    {EFFECT_SLEEP_MONSTERS,        10,   6,     1,  40,    33,  0, 0},
    {EFFECT_SLOW_MONSTERS,         10,   6,     1,  40,    33,  0, 0},
    {EFFECT_CONFUSE_MONSTERS,      15,   8,     1,  40,    33,  0, 0},
    {EFFECT_TELEPORT,              20,  10,     1,   0,    10,  0, 0},
    {EFFECT_ENLIGHTENMENT,         20,  10,     1,  70,    10,  0, _STOCK_TOWN},
    {EFFECT_STARLITE,              20,  10,     1,  50,    33,  0, 0},
    {EFFECT_EARTHQUAKE,            20,  10,     2,   0,    10,  0, 0},
    {EFFECT_HEAL,                  20,  10,     2,  70,    33,  0,  _COMMON}, /* Cure Wounds for ~50hp */
    {EFFECT_CURING,                25,  12,     1,  70,    10,  0, 0}, /* Curing no longer heals */
    {EFFECT_SUMMON_HOUNDS,         27,  25,     2,   0,    10,  0, 0},
    {EFFECT_SUMMON_HYDRAS,         27,  25,     3,   0,    10,  0, 0},
    {EFFECT_SUMMON_ANTS,           27,  20,     2,   0,    10,  0, 0},
    {EFFECT_PROBING,               30,  15,     3,  70,    10,  0, 0},
    {EFFECT_TELEPATHY,             30,  16,     2,   0,    10,  0, 0},
    {EFFECT_SUMMON_MONSTERS,       32,  30,     2,   0,    33,  0, 0},
    {EFFECT_ANIMATE_DEAD,          35,  17,     2,  70,    33,  0, 0},
    {EFFECT_SLOWNESS,              40,  19,     3,  70,    50, 10, 0},
    {EFFECT_SPEED,                 40,  19,     2,   0,    10,  0, _COMMON},
    {EFFECT_IDENTIFY_FULL,         40,  20,     3,   0,    10,  0, _COMMON},
    {EFFECT_REMOVE_CURSE,          40,  20,     4,   0,    10,  0, 0},
    {EFFECT_DISPEL_DEMON,          45,  10,     2,   0,    50, 10, 0},
    {EFFECT_DISPEL_UNDEAD,         45,  10,     2,   0,    50, 10, 0},
    {EFFECT_DISPEL_LIFE,           50,  12,     3,   0,    50, 10, 0},
    {EFFECT_DISPEL_EVIL,           55,  13,     3,   0,    50, 10, 0},
    {EFFECT_DISPEL_MONSTERS,       55,  15,     5,   0,    50, 10, 0},
    {EFFECT_DESTRUCTION,           50,  15,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_CONFUSING_LITE,        55,  26,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_HEAL_CURING,           55,  10,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_BANISH_EVIL,           60,  31,     2,   0,    33,  0, _DROP_GOOD},
    {EFFECT_BANISH_ALL,            70,  32,     3,   0,    33,  0, _DROP_GOOD},
    {EFFECT_MANA_STORM,            85,  10,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_STARBURST,             85,  12,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_DARKNESS_STORM,        85,  12,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_GENOCIDE,              90,  50,     8,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _RARE},
    {EFFECT_RESTORE_MANA,         100, 100,    16,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY | _RARE},
    {0}
};

/* MAX(1, _rand_normal(1, 10)) is probablematic. Think about why! */
static int _bounds_check(int value, int min, int max)
{
    int result = value;
    if (result < min)
        result = min;
    if (result > max)
        result = max;
    return result;
}

/* I like to set my deviation as a percentage of the mean.
   Also, the scaling and rounding makes the distribution no
   longer normal, but I like small casting costs to vary as
   well (e.g. _rand_normal(1, 10) can give 2 (about 3% of the
   time) whereas randnor(1, 0.1), even if legal, would not. */
static int _rand_normal(int mean, int pct)
{
    int result = 0;
    int m = mean * 10;
    int d = m * pct / 100;
    int r = randnor(m, d);

    assert(mean >= 0);
    assert(pct >= 0);

    result = r/10;
    if (randint0(10) < (r%10))
        result++;

    return result;
}
/*static int _rand_normal_hi(int mean, int pct)
{
    int result = _rand_normal(mean, pct);
    if (result < mean)
        result = mean + (mean - result);
    return result;
}*/

static int _effect_rarity(device_effect_info_ptr entry, int level)
{
    int r = entry->rarity;
    if (!r) return 0;
    if (entry->max_depth && entry->max_depth < level) return 0;
    if (entry->flags & _RARE)
    {
        int n = entry->counts.found + entry->counts.bought;
        while (n--)
            r *= 2;
    }
    else if (level > entry->level)
    {
        int d = level - entry->level;
        int spread = entry->max_depth ? entry->max_depth - entry->level : 100 - entry->level;
        int n = (entry->flags & _COMMON) ? spread/2 : spread*2/7;
        while (d >= n)
        {
            r *= 2;
            d -= n;
        }
        r += d*r/n;
    }
    return r;
}

static void _device_adjust_activation_difficulty(object_type *o_ptr, device_effect_info_ptr entry)
{
    int d = 10*(o_ptr->activation.power - o_ptr->activation.difficulty);
    int b = entry->difficulty_base * d / 100;

    b += randint0(entry->difficulty_xtra * d / 100);
    o_ptr->activation.difficulty += b/10;
    if (randint0(10) < b%10)
        o_ptr->activation.difficulty++;
    if (o_ptr->activation.difficulty > o_ptr->activation.power) /* paranoia */
        o_ptr->activation.difficulty = o_ptr->activation.power;
}

static void _device_pick_effect(object_type *o_ptr, device_effect_info_ptr table, int level, int mode)
{
    int i, n;
    int tot = 0;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        int                    rarity;

        if (!entry->type) break;

        entry->prob = 0;
        rarity = _effect_rarity(entry, level);

        if (!rarity) continue;
        if (entry->level > device_level(o_ptr)) continue;
        if ((mode & AM_GOOD) && !(entry->flags & _DROP_GOOD)) continue;
        if ((mode & AM_GREAT) && !(entry->flags & _DROP_GREAT)) continue;
        if ((mode & AM_STOCK_TOWN) && !(entry->flags & _STOCK_TOWN)) continue;
		if (easy_id && entry->type == EFFECT_IDENTIFY_FULL) continue;
		if (easy_lore && entry->type == EFFECT_PROBING) continue;

        entry->prob = 64 / rarity;
        tot += entry->prob;
    }

    if (!tot) return;
    n = randint1(tot);

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];

        if (!entry->type) break;
        if (!entry->prob) continue;

        n -= entry->prob;
        if (n <= 0)
        {
            int cost;

            o_ptr->activation.type = entry->type;

            /* Power is the casting level of the device and determines damage or power of the effect.
               Difficulty is the level of the effect, and determines the fail rate of the effect.
               Difficulty is set to the base level of the effect, and then scaled based on the
               power of the effect using the difficulty_base and xtra percentages. */
            o_ptr->activation.power = device_level(o_ptr);
            o_ptr->activation.difficulty = entry->level;
            if (o_ptr->activation.power > o_ptr->activation.difficulty)
            {
                _device_adjust_activation_difficulty(o_ptr, entry);
            }

            cost = entry->cost;
            cost += effect_cost_extra(&o_ptr->activation);
            o_ptr->activation.cost = _bounds_check(_rand_normal(cost, 5), 1, 1000);

            if (entry->flags & _NO_DESTROY)
            {
                add_flag(o_ptr->flags, OF_IGNORE_ACID);
                add_flag(o_ptr->flags, OF_IGNORE_ELEC);
                add_flag(o_ptr->flags, OF_IGNORE_FIRE);
                add_flag(o_ptr->flags, OF_IGNORE_COLD);
            }

            return;
        }
    }
}

static bool _is_valid_device(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
    case TV_WAND:
    case TV_ROD:
    case TV_STAFF:
        return TRUE;
    }
    return FALSE;
}

/* Initialize a device with a random effect for monster drops, dungeon objects, etc */
bool device_init(object_type *o_ptr, int level, int mode)
{
    if (!_is_valid_device(o_ptr))
        return FALSE;

    if (!(mode & (AM_STOCK_TOWN | AM_STOCK_BM)) && one_in_(GREAT_OBJ))
    {
        int boost = level;
        if (boost < 20)
            boost = 20;
        level += rand_range(boost/4, boost/2);
    }

    if (level > 100)
        level = 100;

    /* device_level
     * 90%+-10% means 84.13% <= level (modulo rounding, of course)
     * 95%+-10% means ~70% <= level. So ~30% at or *above* level.
     * See how generous I've become ;) */
    o_ptr->xtra3 = _bounds_check(_rand_normal(level*95/100, 10), 1, 100);

    switch (o_ptr->tval)
    {
    case TV_WAND:
        _device_pick_effect(o_ptr, wand_effect_table, o_ptr->xtra3, mode);
        if (!o_ptr->activation.type)
            return FALSE;
        /* device_max_sp */
        o_ptr->xtra4 = _bounds_check(_rand_normal(3*o_ptr->xtra3, 15), o_ptr->activation.cost*4, 1000);
        break;
    case TV_ROD:
        _device_pick_effect(o_ptr, rod_effect_table, o_ptr->xtra3, mode);
        if (!o_ptr->activation.type)
            return FALSE;
        /* device_max_sp: rods have fewer sp but regen more quickly. */
        o_ptr->xtra4 = _bounds_check(_rand_normal(3*o_ptr->xtra3/2, 15), o_ptr->activation.cost*2, 1000);
        break;
    case TV_STAFF:
        _device_pick_effect(o_ptr, staff_effect_table, o_ptr->xtra3, mode);
        if (!o_ptr->activation.type)
            return FALSE;
        /* device_max_sp */
        o_ptr->xtra4 = _bounds_check(_rand_normal(3*o_ptr->xtra3, 15), o_ptr->activation.cost*4, 1000);
        break;
    }
    /* device_sp */
    o_ptr->xtra5 = _bounds_check(_rand_normal(o_ptr->xtra4/2, 25), o_ptr->activation.cost, o_ptr->xtra4);
    o_ptr->xtra5 *= 100; /* scale current sp by 100 for smoother regeneration */

    add_flag(o_ptr->flags, OF_ACTIVATE);

    /* cf obj_create_device in ego.c for egos */
    return TRUE;
}

static device_effect_info_ptr _device_find_effect(device_effect_info_ptr table, int effect)
{
    int i;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];

        if (!entry->type) break;
        if (entry->type == effect) return entry;
    }

    return NULL;
}

bool device_is_valid_effect(int tval, int effect)
{
    switch (tval)
    {
    case TV_WAND:
        return _device_find_effect(wand_effect_table, effect) != NULL;
    case TV_ROD:
        return _device_find_effect(rod_effect_table, effect) != NULL;
    case TV_STAFF:
        return _device_find_effect(staff_effect_table, effect) != NULL;
    }
    return FALSE;
}

/* Initialize a device with a fixed effect. This is useful for birth objects, quest rewards, etc */
bool device_init_fixed(object_type *o_ptr, int effect)
{
    device_effect_info_ptr e_ptr = NULL;

    if (!_is_valid_device(o_ptr))
        return FALSE;

    switch (o_ptr->tval)
    {
    case TV_WAND:
        e_ptr = _device_find_effect(wand_effect_table, effect);
        if (!e_ptr)
            return FALSE;
        break;
    case TV_ROD:
        e_ptr = _device_find_effect(rod_effect_table, effect);
        if (!e_ptr)
            return FALSE;
        break;
    case TV_STAFF:
        e_ptr = _device_find_effect(staff_effect_table, effect);
        if (!e_ptr)
            return FALSE;
        break;
    }

    o_ptr->xtra3 = e_ptr->level;
    if (o_ptr->level < 0) /* Level hack */
    {
        o_ptr->xtra3 = MAX(e_ptr->level, MIN(e_ptr->level - o_ptr->level, (e_ptr->max_depth ? e_ptr->max_depth : 100)));
        o_ptr->level = 0;
    }

    if (o_ptr->xtra3 < 7)
        o_ptr->xtra3 = 7;

    o_ptr->activation.type = e_ptr->type;
    o_ptr->activation.power = o_ptr->xtra3;
    o_ptr->activation.difficulty = e_ptr->level;
    if (o_ptr->activation.power > o_ptr->activation.difficulty)
    {
        _device_adjust_activation_difficulty(o_ptr, e_ptr);
    }
    
    o_ptr->activation.cost = e_ptr->cost + effect_cost_extra(&o_ptr->activation);

    if (o_ptr->tval == TV_ROD)
    {
        o_ptr->xtra4 = _bounds_check(3 * o_ptr->xtra3 / 2, o_ptr->activation.cost*2, 1000);
    }
    else
    {
        o_ptr->xtra4 = _bounds_check(3 * o_ptr->xtra3, o_ptr->activation.cost*4, 1000);
    }
    o_ptr->xtra5 = o_ptr->xtra4; /* Fully Charged */
    o_ptr->xtra5 *= 100; /* scale current sp by 100 for smoother regeneration */

    if (e_ptr->flags & _NO_DESTROY)
    {
        add_flag(o_ptr->flags, OF_IGNORE_ACID);
        add_flag(o_ptr->flags, OF_IGNORE_ELEC);
        add_flag(o_ptr->flags, OF_IGNORE_FIRE);
        add_flag(o_ptr->flags, OF_IGNORE_COLD);
    }

    add_flag(o_ptr->flags, OF_ACTIVATE);

    return TRUE;
}

/* TODO: See wiz_obj.c for reliance on xtra fields */
int device_level(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
        return o_ptr->xtra3;
    return 0;
}

int device_sp(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
        return o_ptr->xtra5 / 100;
    return 0;
}

int device_charges(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr) && o_ptr->activation.cost)
        return  device_sp(o_ptr) / o_ptr->activation.cost;
    return 0;
}

int device_max_charges(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr) && o_ptr->activation.cost)
        return  device_max_sp(o_ptr) / o_ptr->activation.cost;
    return 0;
}

void device_decrease_sp(object_type *o_ptr, int amt)
{
    if (_is_valid_device(o_ptr))
    {
        int charges = device_charges(o_ptr);
        o_ptr->xtra5 -= amt * 100;
        if (o_ptr->xtra5 < 0)
            o_ptr->xtra5 = 0;
        if (device_charges(o_ptr) != charges)
            p_ptr->window |= PW_INVEN;
    }
}

void device_increase_sp(object_type *o_ptr, int amt)
{
    if (_is_valid_device(o_ptr))
    {
        int charges = device_charges(o_ptr);
        o_ptr->xtra5 += amt * 100;
        if (o_ptr->xtra5 > o_ptr->xtra4 * 100)
            o_ptr->xtra5 = o_ptr->xtra4 * 100;
        if (device_charges(o_ptr) != charges)
            p_ptr->window |= PW_INVEN;
    }
}

bool device_is_fully_charged(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        if (o_ptr->xtra5 == o_ptr->xtra4 * 100)
            return TRUE;
        else
            return FALSE;
    }
    return FALSE; /* ?? */
}

/* Note: Rods fire every 10 game turns; wands and staves fire every 100 game turns.*/
void device_regen_sp_aux(object_type *o_ptr, int per_mill)
{
    if (!device_is_fully_charged(o_ptr))
    {
        int div = 1000;
        int amt = o_ptr->xtra4 * 100 * per_mill;
        int charges = device_charges(o_ptr);

        o_ptr->xtra5 += amt / div;
        if (randint0(div) < (amt % div))
            o_ptr->xtra5++;

        if (o_ptr->xtra5 > o_ptr->xtra4 * 100)
            o_ptr->xtra5 = o_ptr->xtra4 * 100;

        if (device_is_fully_charged(o_ptr))
            recharged_notice(o_ptr, '!');

        if (device_charges(o_ptr) != charges)
        {
            if (!charges) recharged_notice(o_ptr, '1');
            p_ptr->window |= PW_INVEN;
        }
    }
}

void device_regen_sp(object_type *o_ptr, int base_per_mill)
{
    int  per_mill = base_per_mill;
    u32b flgs[OF_ARRAY_SIZE];

    if (!_is_valid_device(o_ptr))
        return;

    if (device_is_fully_charged(o_ptr))
        return;

    if (devicemaster_is_speciality(o_ptr))
        per_mill += base_per_mill;

    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_REGEN))
        per_mill += o_ptr->pval * base_per_mill;

    device_regen_sp_aux(o_ptr, per_mill);
}

int device_max_sp(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
        return o_ptr->xtra4;
    return 0;
}

int device_value(object_type *o_ptr, int options)
{
    int  result = 0;
    u32b flgs[OF_ARRAY_SIZE];
    int  pval = 0;

    if (!_is_valid_device(o_ptr))
        return 0;

    switch (o_ptr->tval)
    {
    case TV_WAND: result = 100; break;
    case TV_STAFF: result = 100; break;
    case TV_ROD: result = 250; break;
    }

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        pval = o_ptr->pval;
        if (o_ptr->activation.type)
        {
            int value = effect_value(&o_ptr->activation);

            if (o_ptr->activation.cost)
            {
                int base_charges = 40; /* scaled by 10 */
                int charges = device_max_sp(o_ptr) * 10 / o_ptr->activation.cost;

                if (o_ptr->tval == TV_ROD)
                {
                    base_charges /= 2;
                    value = value * 150 / 100; /* rods should value more than wands (durable+fast recharge) */
                }

                value = value * charges / base_charges;
            }
            result += value;
        }
    }

    if (options & COST_REAL)
        obj_flags(o_ptr, flgs);
    else
        obj_flags_known(o_ptr, flgs);

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        if (o_ptr->name2 == EGO_DEVICE_RESISTANCE) /* I don't want artifacts to get an extra boost for TR_IGNORE_* */
            result += result * 25 / 100;
    }
    if (have_flag(flgs, OF_REGEN))
        result += result * 20 * pval / 100;

    if (have_flag(flgs, OF_EASY_SPELL))
        result += result * 10 * pval / 100;

    if (have_flag(flgs, OF_DEVICE_POWER))
        result += result * 20 * pval / 100;

    if (have_flag(flgs, OF_HOLD_LIFE))
        result += result * 40 / 100;

    if (have_flag(flgs, OF_SPEED))
        result += result * 25 * pval / 100;

    return result;
}

/* Statistics */
device_effect_info_ptr device_get_effect_info(int tval, int effect)
{
    switch (tval)
    {
    case TV_WAND:
        return _device_find_effect(wand_effect_table, effect);
        break;
    case TV_ROD:
        return _device_find_effect(rod_effect_table, effect);
        break;
    case TV_STAFF:
        return _device_find_effect(staff_effect_table, effect);
        break;
    }
    return NULL;
}

static void _device_stats_reset_imp(device_effect_info_ptr table)
{
    int i;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        if (!entry->type) break;
        WIPE(&entry->counts, counts_t);
    }
}

void device_stats_reset(void)
{
    _device_stats_reset_imp(wand_effect_table);
    _device_stats_reset_imp(rod_effect_table);
    _device_stats_reset_imp(staff_effect_table);
}

static void _device_stats_save_imp(savefile_ptr file, device_effect_info_ptr table)
{
    int i, ct = 0;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        if (!entry->type) break;
        ct++;
    }

    savefile_write_s32b(file, ct);
    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        if (!entry->type) break;
        savefile_write_s32b(file, entry->type);

        savefile_write_s32b(file, entry->counts.generated);
        savefile_write_s32b(file, entry->counts.found);
        savefile_write_s32b(file, entry->counts.bought);
        savefile_write_s32b(file, entry->counts.used);
        savefile_write_s32b(file, entry->counts.destroyed);
    }

}

static void _device_stats_load_imp(savefile_ptr file, device_effect_info_ptr table)
{
    int ct, i;

    /* We reset since not every current table entry need exist in the savefile.
       In other words, code changes over time and I might add a new entry :) */
    _device_stats_reset_imp(table);

    ct = savefile_read_s32b(file);
    for (i = 0; i < ct; i++)
    {
        int                     type = savefile_read_s32b(file);
        counts_t                counts;
        device_effect_info_ptr  entry = _device_find_effect(table, type);

        counts.generated = savefile_read_s32b(file);
        counts.found = savefile_read_s32b(file);
        counts.bought = savefile_read_s32b(file);
        counts.used = savefile_read_s32b(file);
        counts.destroyed = savefile_read_s32b(file);

        if (entry)
            entry->counts = counts;
    }
}

void device_stats_on_save(savefile_ptr file)
{
    int i, ct = 0;
    _device_stats_save_imp(file, wand_effect_table);
    _device_stats_save_imp(file, rod_effect_table);
    _device_stats_save_imp(file, staff_effect_table);

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].known) ct++;
    }
    savefile_write_s32b(file, ct);
    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].known)
            savefile_write_s32b(file, _effect_info[i].type);
    }
}

void device_stats_on_load(savefile_ptr file)
{
    int i, ct;

    _device_stats_load_imp(file, wand_effect_table);
    _device_stats_load_imp(file, rod_effect_table);
    _device_stats_load_imp(file, staff_effect_table);

    ct = savefile_read_s32b(file);
    for (i = 0; i < ct; i++)
    {
        int type = savefile_read_s32b(file);
        _effect_info_ptr e = _get_effect_info(type);
        if (e)
            e->known = TRUE;
    }
}

void device_stats_on_find(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.found++;
    }
}

void device_stats_on_use(object_type *o_ptr, int num)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.used += num;
    }
}

void device_stats_on_destroy(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.destroyed++;
    }
}

void device_stats_on_purchase(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.bought++;
    }
}

static int _extra(effect_t *effect, int def)
{
    int result = effect->extra;
    if (!result)
        result = def;
    return result;
}

static int _boost(int value, int boost)
{
    return MAX(0, value * (100 + boost) / 100);
}

static int _avg_damroll(int dd, int ds)
{
    return dd * (ds + 1) / 2;
}

/* Device casting is non-linear in difficulty (cf design/devices.ods)
 * Yet device power (e.g. damage) is (or was?) linear. This is hardly fair! */
typedef struct { int w1, w2, w3; } _weights_t;
static _weights_t _weights(int w1, int w2, int w3)
{
    _weights_t w;
    w.w1 = w1;
    w.w2 = w2;
    w.w3 = w3;
    return w;
}
typedef struct { int lvl, max; } _level_t;
static _level_t _level_aux(int lvl, int max)
{
    _level_t l;
    l.lvl = lvl;
    l.max = max;
    return l;
}
static _level_t _level(int lvl)
{
    return _level_aux(lvl, 100);
}
static _level_t _level_offset(int lvl, int start)
{
    int l = MAX(0, lvl - start);
    return _level_aux(l, 100 - start);
}
static int _power_curve_aux(int amt, _level_t l, _weights_t w)
{
    int result = 0;
    int wt = w.w1 + w.w2 + w.w3;

    if (l.lvl == l.max)
        return amt;

    result += amt * l.lvl * w.w1 / (l.max*wt);
    result += amt * l.lvl * l.lvl * w.w2 / (l.max*l.max*wt);
    result += (amt * l.lvl * l.lvl / l.max) * l.lvl * w.w3 / (l.max*l.max*wt);

    return result;
}
static int _power_curve(int amt, int lvl)
{
    return _power_curve_aux(amt, _level(lvl), _weights(1, 1, 1));
}
static int _power_curve_offset(int amt, int lvl, int start)
{
    return _power_curve_aux(amt, _level_offset(lvl, start), _weights(1, 1, 1));
}

/************************************************************************
 * The Effects
 ***********************************************************************/
#define _BOOST(n) (_boost((n), boost))
cptr do_effect(effect_t *effect, int mode, int boost)
{
    bool name = (mode == SPELL_NAME);
    bool desc = (mode == SPELL_DESC);
    bool info = (mode == SPELL_INFO);
    bool cast = (mode == SPELL_CAST);
    bool value = (mode == SPELL_VALUE);
    bool color = (mode == SPELL_COLOR);
    bool cost = (mode == SPELL_COST_EXTRA);
    int  dir = 0;

    switch (effect->type)
    {
    case EFFECT_NONE:
        if (name) return "Nothing";
        if (desc) return "It does absolutely nothing at all.";
        if (cast)
        {
            msg_print("Nothing happens.");
            device_noticed = TRUE;
        }
        break;
    /* Detection */
    case EFFECT_LITE_AREA:
        if (name) return "Illumination";
        if (desc) return "It lights up nearby area or current room permanently.";
        if (info) return info_damage(2 + effect->power/20, _BOOST(15), 0);
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (lite_area(_BOOST(damroll(2 + effect->power/20, 15)), 3))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_LITE_MAP_AREA:
        if (name) return "Magic Mapping and Illumination";
        if (desc) return "It maps your vicinity and lights up your current room.";
        if (info) return info_damage(2 + effect->power/20, _BOOST(15), 0);
        if (value) return format("%d", 1300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            map_area(DETECT_RAD_MAP);
            lite_area(_BOOST(damroll(2 + effect->power/20, 15)), 3);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_ENLIGHTENMENT:
        if (name) return "Enlightenment";
        if (desc) return "It maps your vicinity.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            map_area(DETECT_RAD_MAP);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_CLAIRVOYANCE:
        if (name) return "Clairvoyance";
        if (desc) return "It maps, lights permanently and detects all items on the entire level.";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            wiz_lite(p_ptr->tim_superstealth > 0);
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_TRAPS:
        if (name) return "Detect Traps";
        if (desc) return "It detects all traps in your vicinity.";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_traps(DETECT_RAD_DEFAULT, device_known))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_MONSTERS:
        if (name) return "Detect Monsters";
        if (desc) return "It detects all visible monsters in your vicinity.";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_monsters_normal(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_OBJECTS:
        if (name) return "Detect Objects";
        if (desc) return "It detects all objects in your vicinity.";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_objects_normal(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_ALL:
        if (name) return "Detection";
        if (desc) return "It detects all traps, doors, stairs, treasures, items and monsters in your vicinity.";
        if (value) return format("%d", 2000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            detect_all(DETECT_RAD_DEFAULT);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_GOLD:
        if (name) return "Detect Treasure";
        if (desc) return "It detects all treasures in your vicinity when you use it.";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (detect_treasure(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
            if (detect_objects_gold(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_INVISIBLE:
        if (name) return "Detect Invisible";
        if (desc) return "It detects all invisible monsters in your vicinity when you use it.";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_monsters_invis(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_DOOR_STAIRS:
        if (name) return "Detect Doors & Stairs";
        if (desc) return "It detects all doors and stairs in your vicinity when you use it.";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_doors(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
            if (detect_stairs(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_EVIL:
        if (name) return "Detect Evil";
        if (desc) return "It detects all evil monsters in your vicinity when you use it.";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (detect_monsters_evil(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;

    /* Utility */
    case EFFECT_PHASE_DOOR:
        if (name) return "Phase Door";
        if (desc) return "It teleports you a short distance.";
        if (value) return format("%d", 1000);
        if (cast)
        {
            teleport_player(10, 0L);
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            device_noticed = TRUE;
        }
        break;
    case EFFECT_TELEPORT:
        if (name) return "Teleport";
        if (desc) return "It teleports you a long distance.";
        if (value) return format("%d", 1500);
        if (cast)
        {
            teleport_player(100, 0L);
            energy_use = energy_use * 3 / 2;
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            device_noticed = TRUE;
            if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_TELEPORT);
        }
        break;
    case EFFECT_TELEPORT_AWAY:
        if (name) return "Teleport Other";
        if (desc) return "It fires a beam that teleports all affected monsters away.";
        if (info) return format("dist %d", MAX_SIGHT * 5);
        if (value) return format("%d", 1500);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (teleport_monster(dir)) device_noticed = TRUE;
        }
        break;
    case EFFECT_STRAFING:
        if (name) return "Strafing";
        if (desc) return "It teleports you to a nearby visible location.";
        if (value) return format("%d", 1500);
        if (cast)
        {
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            teleport_player(10, TELEPORT_LINE_OF_SIGHT);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DIMENSION_DOOR:
        if (name) return "Dimension Door";
        if (desc) return "It teleports you to a chosen location.";
        if (info) return info_range(_BOOST(effect->power / 2 + 10));
        if (value) return format("%d", 10000);
        if (cast)
        {
            if (dimension_door(_BOOST(effect->power / 2 + 10)))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_ESCAPE:
        if (name) return "Getaway";
        if (desc) return "It provides a random means of escape.";
        if (value) return format("%d", 2000);
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
            device_noticed = TRUE;
        }
        break;
    case EFFECT_RECALL:
        if (name) return "Recall";
        if (desc) return "It recalls you to the surface, or back into a dungeon you have entered.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            device_noticed = TRUE;
            if (!word_of_recall(TRUE)) return NULL;
        }
        break;

    case EFFECT_STONE_TO_MUD:
        if (name) return "Stone to Mud";
        if (desc) return "It turns a door, rock, or wall to mud.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            if (wall_to_mud(dir)) device_noticed = TRUE;
        }
        break;
    case EFFECT_EARTHQUAKE:
        if (name) return "Earthquake";
        if (desc) return "It causes a massive earthquake nearby.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (!earthquake(py, px, _extra(effect, 10)))
                msg_print("The ground trembles.");
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DESTRUCTION:
    {
        int power = _extra(effect, 150 + _power_curve_offset(400, effect->power, 50));
        if (name) return "Destruction";
        if (desc) return "It destroys everything nearby.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", power * 8 + ((100 - (MIN(100, 8000 / power))) * 125));
        if (color) return format("%d", TERM_RED);
        if (cost) return format("%d", power/15);
        if (cast)
        {
            if (destroy_area(py, px, 13 + randint0(5), _BOOST(power)))
                device_noticed = TRUE;
            else
                msg_print("The ground trembles...");
        }
        break;
    }
    case EFFECT_GENOCIDE:
    {
        int power = _extra(effect, effect->power * 3);
        if (name) return "Genocide";
        if (desc) return "It eliminates an entire class of monster, exhausting you. Powerful or unique monsters may resist.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", power*50);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!symbol_genocide(_BOOST(power), TRUE)) return NULL;
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_MASS_GENOCIDE:
    {
        int power = _extra(effect, 100 + effect->power * 3);
        if (name) return "Mass Genocide";
        if (desc) return "It eliminates all nearby monsters, exhausting you. Powerful or unique monsters may be able to resist.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", power*60);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (mass_genocide(_BOOST(power), TRUE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RECHARGE_FROM_DEVICE:
    {
        int power = _extra(effect, 100);
        if (name) return "Recharging";
        if (desc) return "It attempts to recharge a magical device using the mana of a source device.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", power*30);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            device_noticed = TRUE;
            if (!recharge_from_device(_BOOST(power))) return NULL;
        }
        break;
    }
    case EFFECT_RECHARGE_FROM_PLAYER:
    {
        int power = _extra(effect, 100 + effect->power);
        if (name) return "*Recharging*";
        if (desc) return "It attempts to recharge a magical device using your mana as the source.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", power*30);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            device_noticed = TRUE;
            if (!recharge_from_player(_BOOST(power))) return NULL;
        }
        break;
    }
    case EFFECT_ENCHANTMENT:
        if (name) return "Enchantment";
        if (desc) return "It attempts to enchant a weapon, ammo or armor.";
        if (value) return format("%d", 5000);
        if (cast)
        {
            enchantment_hack = TRUE; /* TODO: Hephaestus only ... */
            device_noticed = TRUE;
            cast_enchantment();
            enchantment_hack = FALSE;
        }
        break;
    case EFFECT_IDENTIFY:
        if (name) return "Identify";
        if (desc) return "It identifies an item.";
        if (value) return format("%d", 500);
        if (cast)
        {
            device_noticed = TRUE;
            if (!_do_identify()) return NULL;
        }
        break;
    case EFFECT_IDENTIFY_FULL:
        if (name) return "*Identify*";
        if (desc) return "It reveals all information about an item.";
        if (value) return format("%d", 5000);
        if (cast)
        {
            device_noticed = TRUE;
            if (!identify_fully(NULL)) return NULL;
        }
        break;
    case EFFECT_PROBING:
        if (name) return "Probing";
        if (desc) return "It probes all visible monsters' alignment, HP, AC, speed, current experience and true character.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (probing()) device_noticed = TRUE;
        }
        break;
    case EFFECT_RUNE_EXPLOSIVE:
        if (name) return "Explosive Rune";
        if (desc) return "It sets a rune which will explode on a passing monster.";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (explosive_rune()) device_noticed = TRUE;
        }
        break;
    case EFFECT_RUNE_PROTECTION:
        if (name) return "Rune of Protection";
        if (desc) return "It creates a glyph that inhibits monsters from attacking you.";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (warding_glyph()) device_noticed = TRUE;
        }
        break;
    case EFFECT_SATISFY_HUNGER:
        if (name) return "Satisfy Hunger";
        if (desc) return "It fills your belly with nourishing victuals.";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (set_food(PY_FOOD_MAX - 1)) device_noticed = TRUE;
        }
        break;
    case EFFECT_DESTROY_TRAP:
        if (name) return "Trap and Door Destruction";
        if (desc) return "It destroys all traps and doors in adjacent squares.";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (destroy_doors_touch()) device_noticed = TRUE;
        }
        break;
    case EFFECT_DESTROY_TRAPS:
        if (name) return "Unbarring Ways";
        if (desc) return "It fires a beam which destroys traps and doors.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            if (destroy_door(dir)) device_noticed = TRUE;
        }
        break;
    case EFFECT_WHIRLWIND_ATTACK:
        if (name) return "Whirlwind Attack";
        if (desc) return "It causes you to attack all adjacent monsters in a single turn.";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            int           y = 0, x = 0;
            cave_type    *c_ptr;
            monster_type *m_ptr;
            int           dir;

            for (dir = 0; dir < 8; dir++)
            {
                y = py + ddy_ddd[dir];
                x = px + ddx_ddd[dir];
                c_ptr = &cave[y][x];
                m_ptr = &m_list[c_ptr->m_idx];
                if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
                {
                    py_attack(y, x, 0);
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_LIST_UNIQUES:
        if (name) return "List Uniques";
        if (desc) return "It lists all uniques on the current level.";
        if (value) return format("%d", 12000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            int i;
            for (i = m_max - 1; i >= 1; i--)
            {
                if (!m_list[i].r_idx) continue;
                if (r_info[m_list[i].r_idx].flags1 & RF1_UNIQUE)
                {
                    msg_format("%s. ", r_name + r_info[m_list[i].r_idx].name);
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_LIST_ARTIFACTS:
        if (name) return "List Artifacts";
        if (desc) return "It lists all artifacts on the current level.";
        if (value) return format("%d", 15000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            int i;
            for (i = 1; i < o_max; i++)
            {
                object_type *o_ptr = &o_list[i];
                if (!o_ptr->k_idx) continue;
                if (o_ptr->held_m_idx) continue;
                if (o_ptr->name1)
                {
                    msg_format("%s. ", a_name + a_info[o_ptr->name1].name);
                    device_noticed = TRUE;
                }
                if (o_ptr->art_name)
                {
                    msg_format("%s. ", quark_str(o_ptr->art_name));
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_BANISH_EVIL:
    {
        int power = _extra(effect, 100);
        if (name) return "Banish Evil";
        if (desc) return "It attempts to teleport all visible evil monsters away.";
        if (info) return format("dist %d", _BOOST(power));
        if (value) return format("%d", 50*power);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (banish_evil(_BOOST(power)))
            {
                msg_print("The holy power banishes evil!");
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_BANISH_ALL:
    {
        int power = _extra(effect, 150);
        if (name) return "Banish";
        if (desc) return "It teleports all monsters in sight away unless resisted.";
        if (info) return format("dist %d", _BOOST(power));
        if (value) return format("%d", 70*power);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (banish_monsters(_BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_TELEKINESIS:
    {
        int weight = effect->power * 7;
        if (name) return "Telekinesis";
        if (desc) return "It pulls a distant item close to you.";
        if (info) return info_weight(_BOOST(weight));
        if (value) return format("%d", 8*weight);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            fetch(dir, _BOOST(weight), FALSE);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_ALCHEMY:
        if (name) return "Alchemy";
        if (desc) return "It turns an item into gold.";
        if (value) return format("%d", 2000);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (!alchemy()) return NULL;
            device_noticed = TRUE;
        }
        break;
    case EFFECT_SELF_KNOWLEDGE:
        if (name) return "Self Knowledge";
        if (desc) return "It reveals information about your stats, resistances and life rating.";
        if (value) return format("%d", 2500);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            self_knowledge();
            device_noticed = TRUE;
        }
        break;
    case EFFECT_GENOCIDE_ONE:
    {
        int power = _extra(effect, 50 + effect->power * 3);
        if (name) return "Annihilation";
        if (desc) return "It removes a monster from current dungeon level unless resisted when you use it.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", power*50);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball_hide(GF_GENOCIDE, dir, _BOOST(power), 0);
            device_noticed = TRUE;
        }
        break;
    }
    /* Timed Buffs */
    case EFFECT_STONE_SKIN:
    {
        int power = _extra(effect, 20);
        if (name) return "Stone Skin";
        if (desc) return "It temporarily turns your skin to stone, granting enhanced armor class.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 4000 + 50*power);
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (set_shield(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_ACID:
    {
        int power = _extra(effect, 20);
        if (name) return "Resist Acid";
        if (desc) return "It grants temporary acid resistance.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_ACID));
        if (cast)
        {
            if (set_oppose_acid(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_ELEC:
    {
        int power = _extra(effect, 20);
        if (name) return "Resist Lightning";
        if (desc) return "It grants temporary lightning resistance.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cast)
        {
            if (set_oppose_elec(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_FIRE:
    {
        int power = _extra(effect, 20);
        if (name) return "Resist Fire";
        if (desc) return "It grants temporary fire resistance.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cast)
        {
            if (set_oppose_fire(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_COLD:
    {
        int power = _extra(effect, 20);
        if (name) return "Resist Cold";
        if (desc) return "It grants temporary cold resistance.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_COLD));
        if (cast)
        {
            if (set_oppose_cold(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_POIS:
    {
        int power = _extra(effect, 20);
        if (name) return "Resist Poison";
        if (desc) return "It grants temporary poison resistance.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 2500 + 25*power);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (set_oppose_pois(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESISTANCE:
    {
        int power = _extra(effect, 20);
        if (name) return "Resistance";
        if (desc) return "It grants temporary resistance to the elements and poison.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 5000 + 25*power);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            int dur = _BOOST(power + randint1(power));
            if (set_oppose_base(dur, FALSE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_PROT_EVIL:
    {
        int power = _extra(effect, 100);
        if (name) return "Protection from Evil";
        if (desc) return "It gives temporary melee protection from evil creatures.";
        if (info) return format("dur d%d+%d", 25, _BOOST(power));
        if (value) return format("%d", 2000 + 10*power);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (set_protevil(_BOOST(randint1(25) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HOLY_GRAIL:
        if (name) return "Healing and Magic Resistance";
        if (desc) return "It heals you and gives temporary resistance to magic.";
        if (info) return format("dur d%d+%d", 50, _BOOST(50));
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (hp_player(250))
                device_noticed = TRUE;
            if (set_resist_magic(_BOOST(50 + randint1(50)), FALSE))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_BLESS:
    {
        int power = _extra(effect, 24);
        if (name) return "Holy Prayer";
        if (desc) return "It blesses you temporarily when you read it.";
        if (info) return format("dur d%d+%d", _BOOST(power), 6);
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", TERM_WHITE);
        if (cast)
        {
            if (set_blessed(_BOOST(randint1(power) + 6), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HEROISM:
    {
        int power = _extra(effect, 25);
        if (name) return "Heroism";
        if (desc) return "It grants temporary heroism.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1500 + 25*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (set_hero(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BERSERK:
    {
        int power = _extra(effect, 25);
        if (name) return "Berserk";
        if (desc) return "It causes you to enter a berserk rage, granting enhanced combat prowess but diminished stealth and skills.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1500 + 25*power);
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            if (set_shero(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
            if (hp_player(30))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SPEED:
    {
        int power = _extra(effect, 20);
        if (name) return "Speed";
        if (desc) return "It grants a temporary speed boost.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 2500 + 25*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (set_fast(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SPEED_HERO:
    {
        int power = _extra(effect, effect->power/2);
        if (name) return "Heroic Speed";
        if (desc) return "It grants temporary speed and heroism.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 5000 + 25*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            int dur = _BOOST(randint1(power) + power);
            if (set_fast(dur, FALSE)) device_noticed = TRUE;
            if (set_hero(dur, FALSE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SPEED_HERO_BLESS:
    {
        int power = _extra(effect, 15);
        if (name) return "Heroic Song";
        if (desc) return "It grants temporary speed, blessing and heroism.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 5000 + 30*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            int dur = _BOOST(randint1(power) + power);
            if (set_fast(dur, FALSE)) device_noticed = TRUE;
            if (set_hero(dur, FALSE)) device_noticed = TRUE;
            if (set_blessed(dur, FALSE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_LIGHT_SPEED:
    {
        int power = _extra(effect, 16);
        if (name) return "Light Speed";
        if (desc) return "It temporarily grants you impossible powers of motion.";
        if (info) return format("dur %d", _BOOST(power));
        if (value) return format("%d", 5000 + 500*power);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (set_lightspeed(_BOOST(power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_ENLARGE_WEAPON:
    {
        int power = _extra(effect, 7);
        if (name) return "Enlarge Weapon";
        if (desc) return "It temporarily increases the damage dice of your melee weapon.";
        if (info) return format("dur %d", _BOOST(power));
        if (value) return format("%d", 2000 + 250*power);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            if (set_tim_enlarge_weapon(_BOOST(power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_TELEPATHY:
    {
        int power = _extra(effect, 30);
        if (name) return "Telepathy";
        if (desc) return "It grants you the power of telepathy temporarily.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(25));
        if (value) return format("%d", 2000 + 50*power);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (set_tim_esp(_BOOST(randint1(power) + 25), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_WRAITHFORM:
    {
        int power = _extra(effect, 25);
        if (name) return "Wraithform";
        if (desc) return "It turns you int a wraith, giving the ability to pass through walls as well as reducing the amount of physical damage sustained from attacks.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 10000 + 100*power);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (set_wraith_form(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_INVULNERABILITY:
    {
        int power = _extra(effect, 8);
        if (name) return "Globe of Invulnerability";
        if (desc) return "It generates barrier which completely protect you from almost all damages. Takes a few your turns when the barrier breaks or duration time is exceeded.";
        if (info) return format("dur d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 10000 + 500*power);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (set_invuln(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }

    /* Pets */
    case EFFECT_SUMMON_MONSTERS:
        if (name) return "Summon Monsters";
        if (desc) return "It attempts to summon monsters for assistance.";
        if (value) return format("%d", 1000);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(10)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, 0, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("You get the feeling that something's wrong...");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, 0, PM_FORCE_PET | PM_ALLOW_GROUP | PM_NO_SUMMONERS))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_HOUNDS:
        if (name) return "Summon Hounds";
        if (desc) return "It attempts to summon hounds for assistance.";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(4)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, SUMMON_HOUND, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("You get the feeling that something's wrong...");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_HOUND, PM_FORCE_PET | PM_ALLOW_GROUP))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_ANTS:
        if (name) return "Summon Ants";
        if (desc) return "It attempts to summon ants for assistance.";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(3)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, SUMMON_ANT, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("You get the feeling that something's wrong...");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_ANT, PM_FORCE_PET | PM_ALLOW_GROUP))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_HYDRAS:
        if (name) return "Summon Hydras";
        if (desc) return "It attempts to summon hydras for assistance.";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(3)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, SUMMON_REPTILE, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("You get the feeling that something's wrong...");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_HYDRA, PM_FORCE_PET | PM_ALLOW_GROUP))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_OCTOPUS:
        if (name) return "Summon Octopus";
        if (desc) return "It attempts to summon octopi for assistance.";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint0(3);
            int i;
            for (i = 0; i < num; i++)
            {
                if (summon_named_creature(-1, py, px, MON_JIZOTAKO, PM_FORCE_PET | PM_ALLOW_GROUP))
                    device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SUMMON_DAWN:
        if (name) return "Summon Legion of the Dawn";
        if (desc) return "It attempts to summon Warriors of the Dawn for assistance.";
        if (value) return format("%d", 2500);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_DAWN, (PM_ALLOW_GROUP | PM_FORCE_PET)))
            {
                msg_print("You summon the Legion of the Dawn.");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SUMMON_PHANTASMAL:
        if (name) return "Summon Phantasmal Servant";
        if (desc) return "It attempts to summon a single Phantasmal Servant for assistance.";
        if (value) return format("%d", 1000);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_PHANTOM, (PM_ALLOW_GROUP | PM_FORCE_PET)))
            {
                msg_print("You summon a phantasmal servant.");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SUMMON_ELEMENTAL:
        if (name) return "Summon Elemental";
        if (desc) return "It attempts to conjure a single elemental to serve you.";
        if (value) return format("%d", 1000);
        if (cast)
        {
            bool pet = one_in_(3);
            int  lvl = dun_level;
            u32b mode = pet ? PM_FORCE_PET : PM_NO_PET;
            int  who = SUMMON_WHO_PLAYER;

            if (!pet || lvl >= 50)
                mode |= PM_ALLOW_GROUP;

            if (summon_specific(who, py, px, lvl, SUMMON_ELEMENTAL, mode))
            {
                device_noticed = TRUE;
                msg_print("An elemental materializes...");
                if (pet)
                    msg_print("It seems obedient to you.");
                else
                    msg_print("You fail to control it!");
            }
        }
        break;
    case EFFECT_SUMMON_DRAGON:
        if (name) return "Summon Dragon";
        if (desc) return "It attempts to summon a single dragon for assistance.";
        if (value) return format("%d", 1500);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_DRAGON, PM_FORCE_PET))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_UNDEAD:
        if (name) return "Summon Undead";
        if (desc) return "It attempts to summon a single undead monster to serve you.";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            bool pet = one_in_(3);
            int  lvl = dun_level;
            int  type = lvl > 75 ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD;
            u32b mode = pet ? PM_FORCE_PET : PM_NO_PET;
            int  who = SUMMON_WHO_PLAYER;

            if (!pet || lvl >= 50)
                mode |= PM_ALLOW_GROUP;

            if (summon_specific(who, py, px, lvl, type, mode))
            {
                device_noticed = TRUE;
                msg_print("Cold winds begin to blow around you, carrying with them the stench of decay...");
                if (pet)
                    msg_print("Ancient, long-dead forms arise from the ground to serve you!");
                else
                    msg_print("'The dead arise... to punish you for disturbing them!'");
            }
        }
        break;
    case EFFECT_SUMMON_DEMON:
        if (name) return "Summon Demon";
        if (desc) return "It attempts to summon a single demon to serve you.";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            bool pet = one_in_(3);
            int  lvl = dun_level;
            u32b mode = pet ? PM_FORCE_PET : PM_NO_PET;
            int  who = SUMMON_WHO_PLAYER;

            if (!pet || lvl >= 50)
                mode |= PM_ALLOW_GROUP;

            if (summon_specific(who, py, px, lvl, SUMMON_DEMON, mode))
            {
                device_noticed = TRUE;
                msg_print("The area fills with a stench of sulphur and brimstone.");
                if (pet)
                    msg_print("'What is thy bidding... Master?'");
                else
                    msg_print("'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'");
            }
        }
        break;
    case EFFECT_SUMMON_CYBERDEMON:
        if (name) return "Summon Cyberdemon";
        if (desc) return "It attempts to summon a single Cyberdemon for assistance.";
        if (value) return format("%d", 7500);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_CYBER, PM_FORCE_PET))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_ANGEL:
        if (name) return "Summon Angel";
        if (desc) return "It attempts to summon a single angel for assistance.";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_ANGEL, PM_FORCE_PET))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_KRAKEN:
        if (name) return "Summon Kraken";
        if (desc) return "It attempts to summon powerful kraken for assistance.";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            int num = randint0(3);
            int ct = 0;
            int i;
            fire_ball_hide(GF_WATER_FLOW, 0, 3, 3);
            device_noticed = TRUE;
            for (i = 0; i < num; i++)
                ct += summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_KRAKEN, PM_FORCE_PET);
            if (!ct)
                msg_print("No help arrives.");
        }
        break;

    case EFFECT_CHARM_ANIMAL:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "Charm Animal";
        if (desc) return "It attempts to charm a single animal.";
        if (info) return format("power %d", _BOOST(lvl));
        if (value) return format("%d", 10*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (charm_animal(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CHARM_DEMON:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "Dominate Demon";
        if (desc) return "It attempts to dominate a single demon.";
        if (info) return format("power %d", _BOOST(lvl));
        if (value) return format("%d", 15*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (control_one_demon(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CHARM_UNDEAD:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "Enslave Undead";
        if (desc) return "It attempts to enslave a single undead monster.";
        if (info) return format("power %d", _BOOST(lvl));
        if (value) return format("%d", 15*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (control_one_undead(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CHARM_MONSTER:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "Charm Monster";
        if (desc) return "It attempts to charm a single monster.";
        if (info) return format("power %d", _BOOST(lvl));
        if (value) return format("%d", 15*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (charm_monster(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RETURN_PETS:
        if (name) return "Return Pets";
        if (desc) return "It calls your pets back to you.";
        if (value) return format("%d", 500);
        if (cast)
        {
            int pet_ctr, i;
            u16b *who;
            int max_pet = 0;
            u16b dummy_why;

            stop_mouth();

            C_MAKE(who, max_m_idx, u16b);

            for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
            {
                if (is_pet(&m_list[pet_ctr]) && (p_ptr->riding != pet_ctr))
                    who[max_pet++] = pet_ctr;
            }

            ang_sort_comp = ang_sort_comp_pet;
            ang_sort_swap = ang_sort_swap_hook;
            ang_sort(who, &dummy_why, max_pet);

            for (i = 0; i < max_pet; i++)
            {
                pet_ctr = who[i];
                teleport_monster_to(pet_ctr, py, px, 100, TELEPORT_PASSIVE);
                device_noticed = TRUE;
            }

            C_KILL(who, max_m_idx, u16b);
        }
        break;

    case EFFECT_CAPTURE_PET:
        if (name) return "Capture Pet";
        if (desc) return "It attempts to capture targetted monster.";
        if (value) return format("%d", 500);
        if (cast)
        {
            /* TODO: This is handled elsewhere in cm6.c, since we need the object_type
               for the capture ball in order to "reconstitute" the captured pet.
             */
        }
        break;

    /* Healing and Recovery */
    case EFFECT_RESTORE_STATS:
        if (name) return "Restore Stats";
        if (desc) return "It restores your stats.";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (do_res_stat(A_STR)) device_noticed = TRUE;
            if (do_res_stat(A_INT)) device_noticed = TRUE;
            if (do_res_stat(A_WIS)) device_noticed = TRUE;
            if (do_res_stat(A_DEX)) device_noticed = TRUE;
            if (do_res_stat(A_CON)) device_noticed = TRUE;
            if (do_res_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case EFFECT_RESTORE_EXP:
        if (name) return "Restore Life";
        if (desc) return "It restores your life and experience.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(150)) device_noticed = TRUE;
        }
        break;
    case EFFECT_RESTORING:
        if (name) return "Restoring";
        if (desc) return "It restores your stats, life and experience.";
        if (value) return format("%d", 6000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (do_res_stat(A_STR)) device_noticed = TRUE;
            if (do_res_stat(A_INT)) device_noticed = TRUE;
            if (do_res_stat(A_WIS)) device_noticed = TRUE;
            if (do_res_stat(A_DEX)) device_noticed = TRUE;
            if (do_res_stat(A_CON)) device_noticed = TRUE;
            if (do_res_stat(A_CHR)) device_noticed = TRUE;
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(1000)) device_noticed = TRUE;
        }
        break;
    case EFFECT_HEAL:
    {
        int amt = _extra(effect, 25 + effect->power);
        if (name) return (amt < 100) ? "Cure Wounds" : "Healing";
        if (desc) return "It heals your hitpoints and cures cuts.";
        if (info) return info_heal(0, 0, _BOOST(amt));
        if (value) return format("%d", 15*amt);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            amt = _BOOST(amt);

            if (hp_player(amt)) device_noticed = TRUE;
            if (amt >= 100)
            {
                if (set_cut(0, TRUE)) device_noticed = TRUE;
            }
            else
            {
                if (set_cut(p_ptr->cut - amt, TRUE)) device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_CURING:
    {
        if (name) return "Curing";
        if (desc) return "It cures blindness, confusion, stunning, cuts and hallucination and reduces poisoning.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE))
                device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_image(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HEAL_CURING:
    {
        int amt = _extra(effect, 30 + 4*effect->power);
        if (amt < 100)
        {
            if (name) return "Cure Wounds";
            if (desc) return "It heals your hitpoints and cures blindness and cuts.";
        }
        else
        {
            if (name) return "Healing";
            if (desc) return "It heals your hitpoints and cures your ailments.";
        }
        if (info) return info_heal(0, 0, _BOOST(amt));
        if (value) return format("%d", ((amt > 100) ? 5000 : 1000) + 23*amt);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", amt/10);
        if (cast)
        {
            amt = _BOOST(amt);

            if (hp_player(amt)) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (amt >= 100)
            {
                if (set_cut(0, TRUE)) device_noticed = TRUE;
                if (set_confused(0, TRUE)) device_noticed = TRUE;
                if (set_stun(0, TRUE)) device_noticed = TRUE;
            }
            else
            {
                if (set_cut(p_ptr->cut - amt, TRUE)) device_noticed = TRUE;
            }
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HEAL_CURING_HERO:
    {
        int amt = _extra(effect, 300 + _power_curve_offset(477, effect->power, 70));
        if (name) return "Angelic Healing";
        if (desc) return "It heals your hitpoints, cures what ails you, and makes you heroic.";
        if (info) return info_heal(0, 0, _BOOST(amt));
        /* XXX The following is too low for -AngelicHealing, but avoids over-valuing Lohengrin */
        if (value) return format("%d", 750 + 15*amt);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (hp_player(_BOOST(amt))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(p_ptr->poisoned - MAX(300, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
            if (set_hero(_BOOST(randint1(25) + 25), FALSE)) device_noticed = TRUE;
            if (p_inc_minislow(-1)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESTORE_MANA:
        if (name) return "Restore Mana";
        if (desc) return "It completely restores your mana. It also partially recharges any devices in your pack.";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (restore_mana()) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case EFFECT_CURE_POIS:
        if (name) return "Cure Poison";
        if (desc) return "It reduces poisoning.";
        if (value) return format("%d", 500);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (set_poisoned(p_ptr->poisoned - MAX(300, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_CURE_FEAR:
        if (name) return "Boldness";
        if (desc) return "It restores your courage.";
        if (value) return format("%d", 750);
        if (color) return format("%d", res_color(RES_FEAR));
        if (cast)
        {
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_CURE_FEAR_POIS:
        if (name) return "Cure Fear and Poison";
        if (desc) return "It reduces poisoning and restores your courage in battle.";
        if (value) return format("%d", 1250);
        if (color) return format("%d", res_color(RES_FEAR));
        if (cast)
        {
            if (set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE))
                device_noticed = TRUE;
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_REMOVE_CURSE:
        if (name) return "Remove Curse";
        if (desc) return "It removes normal curses from equipped items.";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (remove_curse())
            {
                msg_print("You feel as if someone is watching over you.");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_REMOVE_ALL_CURSE:
        if (name) return "*Remove Curse*";
        if (desc) return "It removes normal and heavy curses from equipped items.";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (remove_all_curse())
            {
                msg_print("You feel as if someone is watching over you.");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_CLARITY:
    {
        int amt = _extra(effect, effect->cost);
        if (name) return "Clarity";
        if (desc) return "It clears your mind, restoring some mana.";
        if (info) return format("%dsp", _BOOST(amt));
        if (value) return format("%d", 1000 + 50*amt);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("You are unaffected.");
            else if (sp_player(_BOOST(amt)))
            {
                msg_print("You feel your mind clear.");
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_GREAT_CLARITY: /* FYI: This is a separate effect from EFFECT_CLARITY for purposes */
    {                          /*      of statistics tracking, which is by effect id. */
        int amt = _extra(effect, effect->cost);
        if (name) return "Great Clarity";
        if (desc) return "It clears your mind, restoring some mana.";
        if (info) return format("%dsp", _BOOST(amt));
        if (value) return format("%d", 1000 + 50*amt);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("You are unaffected.");
            else if (sp_player(_BOOST(amt)))
            {
                msg_print("You feel your mind clear.");
                device_noticed = TRUE;
            }
        }
        break;
    }

    /* Offense: Bolts */
    case EFFECT_BOLT_MISSILE:
    {
        int dd = _extra(effect, 2 + effect->power/10);
        int ds = 6;
        if (name) return "Magic Missile";
        if (desc) return "It fires a weak bolt of magic.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_MISSILE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_ACID:
    {
        int dd = _extra(effect, 6 + effect->power/7);
        int ds = 8;
        if (name) return "Acid Bolt";
        if (desc) return "It fires a bolt of acid.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 30*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_ACID));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ACID, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_ELEC:
    {
        int dd = _extra(effect, 4 + effect->power/9);
        int ds = 8;
        if (name) return "Lightning Bolt";
        if (desc) return "It fires a bolt of lightning.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_ELEC));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ELEC, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_FIRE:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Fire Bolt";
        if (desc) return "It fires a bolt of fire.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_FIRE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_FIRE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_COLD:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "Frost Bolt";
        if (desc) return "It fires a bolt of frost.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_COLD));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_COLD, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_POIS:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "Poison Dart";
        if (desc) return "It fires a poison dart.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_POIS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_LITE:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "Light Bolt";
        if (desc) return "It fires a bolt of light.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 30*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_LITE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_DARK:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "Dark Bolt";
        if (desc) return "It fires a bolt of darkness.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 30*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_DARK));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_DARK, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_CONF:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "Confusion Bolt";
        if (desc) return "It fires a bolt of confusion.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_CONFUSION, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_NETHER:
    {
        int dd = _extra(effect, 10 + effect->power/6);
        int ds = 8;
        if (name) return "Nether Bolt";
        if (desc) return "It fires a bolt of nether.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_NETHER));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_NETHER, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_NEXUS:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Nexus Bolt";
        if (desc) return "It fires a bolt of nexus.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 35*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_NEXUS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_NEXUS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_SOUND:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Sound Bolt";
        if (desc) return "It fires a bolt of sound.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 45*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_SOUND));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_SOUND, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_SHARDS:
    {
        int dd = _extra(effect, 7 + effect->power/5);
        int ds = 8;
        if (name) return "Shard Bolt";
        if (desc) return "It fires a bolt of shards.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 45*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_SHARDS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_SHARDS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_CHAOS:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Chaos Bolt";
        if (desc) return "It fires a bolt of chaos.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 35*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_CHAOS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_DISEN:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Disenchantment Bolt";
        if (desc) return "It fires a bolt of disenchantment.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 35*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_DISEN));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_DISENCHANT, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_TIME:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Time Bolt";
        if (desc) return "It fires a bolt of time.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 45*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_TIME));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_TIME, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_WATER:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(400, effect->power));
        int base = 20;
        if (name) return "Water Bolt";
        if (desc) return "It fires a bolt of water.";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", TERM_BLUE);
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/7);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_WATER, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_MANA:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(500, effect->power));
        int base = 50;
        if (name) return "Mana Bolt";
        if (desc) return "It fires a powerful bolt of mana.";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", TERM_L_BLUE);
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/8);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_MANA, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_ICE:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(400, effect->power));
        int base = 30;
        if (name) return "Ice Bolt";
        if (desc) return "It fires a bolt of ice.";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/7);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ICE, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_PLASMA:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(400, effect->power));
        int base = 40;
        if (name) return "Plasma Bolt";
        if (desc) return "It fires a bolt of plasma.";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/7);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_PLASMA, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }


    /* Offense: Beams */
    case EFFECT_BEAM_LITE_WEAK:
    {
        int dd = _extra(effect, 6);
        int ds = 8;
        if (name) return "Moonbeam";
        if (desc) return "It fires a beam of weak light, damaging light-sensitive creatures.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            msg_print("A line of pale shimmering light appears.");
            project_hook(GF_LITE_WEAK, dir, _BOOST(damroll(dd, ds)), PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_LITE:
    {
        int dam = _extra(effect, 10 + _power_curve(275, effect->power));
        if (name) return "Beam of Light";
        if (desc) return "It fires a powerful beam of light.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cost) return format("%d", dam/7);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            msg_print("A line of pure white light appears.");
            fire_beam(GF_LITE, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_GRAVITY:
    {
        int dd = _extra(effect, 9 + effect->power/8);
        int ds = 8;
        if (name) return "Beam of Gravity";
        if (desc) return "It fires a beam of gravity.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 55*_avg_damroll(dd, ds));
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_GRAVITY, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_DISINTEGRATE:
    {
        int dd = _extra(effect, 9 + effect->power/8);
        int ds = 8;
        if (name) return "Beam of Disintegration";
        if (desc) return "It fires a beam of disintegration.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 40*_avg_damroll(dd, ds));
        if (color) return format("%d", TERM_SLATE);
        if (cast)
        {
            if (!get_fire_dir_aux(&dir, TARGET_DISI)) return NULL;
            fire_beam(GF_DISINTEGRATE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_ACID:
    {
        int dam = _extra(effect, 5 + _power_curve(270, effect->power));
        if (name) return "Shoot Acid";
        if (desc) return "It fires a beam of acid.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_ACID));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_ACID, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_ELEC:
    {
        int dam = _extra(effect, 5 + _power_curve(250, effect->power));
        if (name) return "Lightning Strike";
        if (desc) return "It fires a beam of lightning.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_ELEC, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_FIRE:
    {
        int dam = _extra(effect, 5 + _power_curve(280, effect->power));
        if (name) return "Line of Fire";
        if (desc) return "It fires a beam of fire.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_FIRE, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_COLD:
    {
        int dam = _extra(effect, 5 + _power_curve(260, effect->power));
        if (name) return "Ray of Cold";
        if (desc) return "It fires a beam of frost.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_COLD, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_SOUND:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Sound Strike";
        if (desc) return "It fires a beam of sound.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 50*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_SOUND));
        if (cost) return format("%d", _avg_damroll(dd, ds)/5);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_SOUND, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_CHAOS:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "Chaos Strike";
        if (desc) return "It fires a beam of chaos.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 40*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cost) return format("%d", _avg_damroll(dd, ds)/5);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_CHAOS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }

    /* Offense: Balls */
    case EFFECT_BALL_ACID:
    {
        int dam = _extra(effect, 20 + _power_curve(300, effect->power));
        if (name) return "Acid Ball";
        if (desc) return "It fires a ball of acid.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_ACID));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ACID, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_ELEC:
    {
        int dam = _extra(effect, 20 + _power_curve(250, effect->power));
        if (name) return "Lightning Ball";
        if (desc) return "It fires a ball of lightning.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ELEC, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_FIRE:
    {
        int dam = _extra(effect, 20 + _power_curve(350, effect->power));
        if (name) return "Fire Ball";
        if (desc) return "It fires a ball of fire.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_FIRE, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_COLD:
    {
        int dam = _extra(effect, 20 + _power_curve(275, effect->power));
        if (name) return "Frost Ball";
        if (desc) return "It fires a ball of frost.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_COLD, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_POIS:
    {
        int dam = _extra(effect, 12 + effect->power/4);
        if (name) return "Stinking Cloud";
        if (desc) return "It fires a ball of poison.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_POIS, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_LITE:
    {
        int dam = _extra(effect, 200 + _power_curve_offset(350, effect->power, 80));
        if (name) return "Star Burst";
        if (desc) return "It fires a huge ball of powerful light.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_LITE, dir, _BOOST(dam), 4);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_DARK:
    {
        int dam = _extra(effect, 100 + 7*effect->power/2);
        if (name) return "Darkness Storm";
        if (desc) return "It fires a huge ball of darkness.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_DARK));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DARK, dir, _BOOST(dam), 4);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_CONF:
    {
        int dam = _extra(effect, 30 + effect->power);
        if (name) return "Confusion Ball";
        if (desc) return "It fires a ball of confusion.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NETHER, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_NETHER:
    {
        int dam = _extra(effect, 125 + _power_curve_offset(250, effect->power, 30));
        if (name) return "Nether Ball";
        if (desc) return "It fires a ball of nether.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 25*dam);
        if (color) return format("%d", res_color(RES_NETHER));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NETHER, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_NEXUS:
    {
        int dam = _extra(effect, 100 + _power_curve_offset(200, effect->power, 40));
        if (name) return "Nexus Ball";
        if (desc) return "It fires a ball of nexus.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_NEXUS));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NEXUS, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_SOUND:
    {
        int dam = _extra(effect, 70 + _power_curve_offset(280, effect->power, 40));
        if (name) return "Sound Ball";
        if (desc) return "It fires a ball of sound.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_SOUND));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SOUND, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_SHARDS:
    {
        int dam = _extra(effect, 175 + _power_curve_offset(325, effect->power, 75));
        if (name) return "Shard Ball";
        if (desc) return "It fires a ball of shards.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_SHARDS));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SHARDS, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_CHAOS:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(350, effect->power, 70));
        if (name) return "Invoke Logrus";
        if (desc) return "It fires a huge ball of chaos.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_CHAOS, dir, _BOOST(dam), 5);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_DISEN:
    {
        int dam = _extra(effect, 90 + _power_curve_offset(250, effect->power, 40));
        if (name) return "Disenchantment Ball";
        if (desc) return "It fires a ball of disenchantment.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_DISEN));
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DISENCHANT, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_TIME:
    {
        int dam = _extra(effect, 50 + effect->power);
        if (name) return "Time Ball";
        if (desc) return "It fires a ball of time.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_TIME));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_TIME, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_WATER:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(200, effect->power, 50));
        if (name) return "Whirlpool";
        if (desc) return "It fires a huge ball of water.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_BLUE);
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_WATER, dir, _BOOST(dam), 4);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_MANA:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(300, effect->power, 60));
        if (name) return "Mana Ball";
        if (desc) return "It fires a powerful ball of mana.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", TERM_L_BLUE);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_ball(GF_MANA, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_DISINTEGRATE:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(200, effect->power, 50));
        if (name) return "Disintegrate";
        if (desc) return "It fires a powerful ball of disintegration.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_SLATE);
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DISINTEGRATE, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }

    /* Offense: Breaths */
    case EFFECT_BREATHE_ACID:
    {
        int dam = _extra(effect, 100 + effect->power*3);
        if (name) return "Breathe Acid";
        if (desc) return "It breathes acid.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_ACID));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ACID, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ELEC:
    {
        int dam = _extra(effect, 70 + effect->power*3);
        if (name) return "Breathe Lightning";
        if (desc) return "It breathes lightning.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ELEC, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_FIRE:
    {
        int dam = _extra(effect, 160 + _power_curve_offset(300, effect->power, 40));
        if (name) return "Dragon's Flame";
        if (desc) return "It breathes fire.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_FIRE, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_COLD:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(300, effect->power, 40));
        if (name) return "Dragon's Frost";
        if (desc) return "It breathes frost.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_COLD, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_POIS:
    {
        int dam = _extra(effect, 60 + effect->power*2);
        if (name) return "Breathe Poison";
        if (desc) return "It breathes poison.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_POIS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_LITE:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe Light";
        if (desc) return "It breathes light.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_LITE, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_DARK:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe Darkness";
        if (desc) return "It breathes darkness.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_DARK));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DARK, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_CONF:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe Confusion";
        if (desc) return "It breathes confusion.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_CONFUSION, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_NETHER:
    {
        int dam = _extra(effect, 100 + effect->power*3);
        if (name) return "Breathe Nether";
        if (desc) return "It breathes nether.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_NETHER));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NETHER, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_NEXUS:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe Nexus";
        if (desc) return "It breathes nexus.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_NEXUS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NEXUS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_SOUND:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe Sound";
        if (desc) return "It breathes sound.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_SOUND));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SOUND, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_SHARDS:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "Breathe Shards";
        if (desc) return "It breathes shards.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_SHARDS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SHARDS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_CHAOS:
    {
        int dam = _extra(effect, 75 + effect->power*2);
        if (name) return "Breathe Chaos";
        if (desc) return "It breathes chaos.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_CHAOS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_DISEN:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe Disenchantment";
        if (desc) return "It breathes disenchantment.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_DISEN));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DISENCHANT, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
	case EFFECT_BREATHE_INERTIA:
	{
		int dam = _extra(effect, 50 + effect->power * 2);
		if (name) return "Breathe Inertia";
		if (desc) return "It breathes inertia.";
		if (info) return info_damage(0, 0, _BOOST(dam));
		if (value) return format("%d", 35 * dam);
		if (cast)
		{
			if (!get_fire_dir(&dir)) return NULL;
			fire_ball(GF_INERT, dir, _BOOST(dam), -2);
			device_noticed = TRUE;
		}
		break;
	}
	case EFFECT_BREATHE_WATER:
	{
		int dam = _extra(effect, 41 + effect->power * 7 / 4);
		if (name) return "Tsunami";
		if (desc) return "It fires a torrent of water.";
		if (info) return info_damage(0, 0, _BOOST(dam));
		if (value) return format("%d", 30 * dam);
		if (cost) return format("%d", dam/32);
		if (cast)
		{
			if (!get_fire_dir(&dir)) return NULL;
			fire_ball(GF_WATER, dir, _BOOST(dam), -2);
			device_noticed = TRUE;
		}
		break;
	}
    case EFFECT_BREATHE_TIME:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe Time";
        if (desc) return "It breathes time.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_TIME));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_TIME, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_MULTIHUED:
    {
        int dam = _extra(effect, 170 + _power_curve_offset(300, effect->power, 40));
        if (name) return "Dragon's Breath";
        if (desc) return "It breathes acid, lightning, fire, frost or poison.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", TERM_ORANGE);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[5] = {
                { GF_ACID, "acid"},
                { GF_ELEC, "lightning"},
                { GF_FIRE, "fire"},
                { GF_COLD, "frost"},
                { GF_POIS, "poison"},
            };
            int which = randint0(5);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("It breathes %s.", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_CHAOS:
    {
        int dam = _extra(effect, 75 + effect->power*2);
        if (name) return "Breathe";
        if (desc) return "It breathes chaos or disenchantment.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[2] = {
                { GF_CHAOS, "chaos"},
                { GF_DISENCHANT, "disenchantment"},
            };
            int which = randint0(2);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("It breathes %s.", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_LAW:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "Breathe";
        if (desc) return "It breathes sound or shards.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_SOUND));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[2] = {
                { GF_SOUND, "sound"},
                { GF_SHARDS, "shards"},
            };
            int which = randint0(2);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("It breathes %s.", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_BALANCE:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "Breathe";
        if (desc) return "It breathes sound, shards, chaos or disenchantment.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_DISEN));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[4] = {
                { GF_SOUND, "sound"},
                { GF_SHARDS, "shards"},
                { GF_CHAOS, "chaos"},
                { GF_DISENCHANT, "disenchantment"},
            };
            int which = randint0(4);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("It breathes %s.", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_SHINING:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "Breathe";
        if (desc) return "It breathes light or darkness.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[2] = {
                { GF_LITE, "light"},
                { GF_DARK, "darkness"},
            };
            int which = randint0(2);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("It breathes %s.", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ELEMENTS:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "Breathe Elements";
        if (desc) return "It breathes the elements.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_MISSILE, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }

    /* Offense: Other */
    case EFFECT_DISPEL_EVIL:
    {
        int dam = _extra(effect, 50 + _power_curve_offset(250, effect->power, 50));
        if (name) return "Dispel Evil";
        if (desc) return "It damages all evil monsters in sight.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (dispel_evil(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_EVIL_HERO:
    {
        int dam = _extra(effect, 2*effect->power);
        if (name) return "Dispel Evil";
        if (desc) return "It damages all evil monsters in sight and grants temporary heroism.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 500 + 30*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (dispel_evil(_BOOST(dam)))
                device_noticed = TRUE;
            if (set_hero(_BOOST(25 + randint1(25)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_GOOD:
    {
        int dam = _extra(effect, 2*effect->power);
        if (name) return "Dispel Good";
        if (desc) return "It damages all good monsters in sight.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cost) return format("%d", dam/12);
        if (cast)
        {
            if (dispel_good(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_LIFE:
    {
        int dam = _extra(effect, 50 + _power_curve_offset(250, effect->power, 50));
        if (name) return "Dispel Life";
        if (desc) return "It damages all living monsters in sight.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (dispel_living(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_DEMON:
    {
        int dam = _extra(effect, 100 + _power_curve_offset(400, effect->power, 50));
        if (name) return "Dispel Demons";
        if (desc) return "It damages all demonic monsters in sight.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/15);
        if (cast)
        {
            if (dispel_demons(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_UNDEAD:
    {
        int dam = _extra(effect, 100 + _power_curve_offset(400, effect->power, 50));
        if (name) return "Dispel Undead";
        if (desc) return "It damages all undead monsters in sight.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/15);
        if (cast)
        {
            if (dispel_undead(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_MONSTERS:
    {
        int dam = _extra(effect, 50 + _power_curve_offset(200, effect->power, 50));
        if (name) return "Dispel Monsters";
        if (desc) return "It damages all monsters in sight.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (dispel_monsters(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DRAIN_LIFE:
    {
        int dam = _extra(effect, 50 + effect->power/2);
        if (name) return "Vampirism";
        if (desc) return "It fires a bolt that steals life from a foe when you use it.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            dam = _BOOST(dam);
            if (drain_life(dir, dam))
            {
                vamp_player(dam);
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_STAR_BALL:
    {
        int dam = _extra(effect, 150);
        if (name) return "Star Ball";
        if (desc) return "It fires a multitude of lightning balls in random open directions.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            int num = _BOOST(damroll(5, 3));
            int y, x, i;
            int attempts;

            for (i = 0; i < num; i++)
            {
                attempts = 1000;
                while (attempts--)
                {
                    scatter(&y, &x, py, px, 4, 0);
                    if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                    if (!player_bold(y, x)) break;
                }
                project(0, 3, y, x, _BOOST(dam), GF_ELEC,
                    (PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
            }
        }
        break;
    }
    case EFFECT_WRATH_OF_GOD:
    {
        int dam = _extra(effect, 25 + effect->power*3/2);
        if (name) return "Wrath of the God";
        if (desc) return "It drops many balls of disintegration near the target.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (cast)
        {
            if (!cast_wrath_of_the_god(_BOOST(dam), 2)) return NULL;
        }
        break;
    }
    case EFFECT_ROCKET:
    {
        int dam = _extra(effect, 200 + _power_curve_offset(300, effect->power, 60));
        if (name) return "Rocket";
        if (desc) return "It fires a rocket.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", TERM_UMBER);
        if (cost) return format("%d", dam/11);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_rocket(GF_ROCKET, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_METEOR:
    {
        int dd = _extra(effect, 15 + effect->power/5);
        int ds = 13;
        if (name) return "Meteor";
        if (desc) return "It fires a meteor when you use it.";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 40*_avg_damroll(dd, ds));
        if (color) return format("%d", TERM_UMBER);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_METEOR, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_MANA_STORM:
    {
        int dam = _extra(effect, 375 + _power_curve_offset(200, effect->power, 80));
        if (name) return "Mana Storm";
        if (desc) return "It produces a huge mana ball centered on you.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_RED);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            msg_print("Mighty magics rend your enemies!");
            project(0, 5, py, px,
                _BOOST(dam*2),
                GF_MANA, PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CONFUSING_LITE:
    {
        int pow = _extra(effect, effect->power*2);
        if (name) return "Confusing Lights";
        if (desc) return "It emits dazzling lights which slow, stun, confuse, scare and even freeze nearby monsters.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 60*pow);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            msg_print("You glare nearby monsters with a dazzling array of confusing lights!");
            pow = _BOOST(pow);
            slow_monsters(pow);
            stun_monsters(5 + pow/10);
            confuse_monsters(pow);
            turn_monsters(pow);
            stasis_monsters(pow/3);
            device_noticed = TRUE; /* You see the dazzling lights, no? */
        }
        break;
    }
    case EFFECT_ARROW:
    {
        int dam = _extra(effect, 70 + effect->power);
        if (name) return "Magic Arrow";
        if (desc) return "It fires a powerful magical arrow.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", TERM_SLATE);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ARROW, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HOLINESS:
    {
        int dam = _extra(effect, effect->power*2);
        if (name) return "Holiness";
        if (desc) return "It does damage to all evil monsters in sight, gives temporary protection from lesser evil creature, cures poison, stunning, cuts, removes fear and heals you when you use it.";
        if (info) return info_power(_BOOST(dam));
        if (value) return format("%d", 5000 + 30*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (dispel_evil(_BOOST(dam))) device_noticed = TRUE;
            if (set_protevil(p_ptr->protevil + _BOOST(dam/2), FALSE)) device_noticed = TRUE;
            if (hp_player(_BOOST(dam))) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_STARBURST:
    {
        int dam = _extra(effect, 375 + _power_curve_offset(200, effect->power, 80));
        if (name) return "Star Burst";
        if (desc) return "It produces a huge ball of light centered on you.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_LITE))
            {
                set_blind(p_ptr->blind + 3 + randint1(5), FALSE);
            }
            project(0, 5, py, px, _BOOST(dam*2),
                    GF_LITE, PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DARKNESS_STORM:
    {
        int dam = _extra(effect, 375 + _power_curve_offset(200, effect->power, 80));
        if (name) return "Darkness Storm";
        if (desc) return "It produces a huge ball of darkness centered on you.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_DARK))
            {
                set_blind(p_ptr->blind + 3 + randint1(5), FALSE);
            }
            project(0, 5, py, px, _BOOST(dam*2),
                    GF_DARK, PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_PESTICIDE:
    {
        int dam = _extra(effect, 4);
        if (name) return "Pesticide";
        if (desc) return "It does slight damage to all monsters in sight when you zap it.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 250);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (dispel_monsters(_BOOST(4)))
                device_noticed = TRUE;
        }
        break;
    }
    /* Misc */
    case EFFECT_POLY_SELF:
        if (name) return "Polymorph";
        if (desc) return "It mutates you. Warning: You might not like the results!";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            if (get_check("This might be risky. Are you sure? "))
            {
                do_poly_self();
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_ANIMATE_DEAD:
        if (name) return "Animate Dead";
        if (desc) return "It raises corpses and skeletons nearby you from dead and makes them your pet when you use it.";
        if (value) return format("%d", 750);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (animate_dead(0, py, px))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SCARE_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "Terrify Monsters";
        if (desc) return "It attempts to frighten all nearby visible monsters.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 10*pow);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (turn_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SLEEP_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "Sleep Monsters";
        if (desc) return "It attempts to sleep all nearby visible monsters.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 15*pow);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            if (sleep_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SLOW_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "Slow Monsters";
        if (desc) return "It attempts to slow all nearby visible monsters.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 15*pow);
        if (color) return format("%d", TERM_SLATE);
        if (cast)
        {
            if (slow_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_STASIS_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "Freeze Monsters";
        if (desc) return "It attempts to freeze all nearby visible monsters.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 30*pow);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            if (stasis_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_EYE_HYPNO:
    {
        int pow = _extra(effect, effect->power*2);
        if (name) return "Hypnotize";
        if (desc) return "It attempts to freeze and charm all nearby visible monsters.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 125*pow);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            (void)stasis_monsters(_BOOST(pow));
            (void)charm_monsters(MIN(56, _BOOST(pow) / 2));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CONFUSE_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "Confuse Monsters";
        if (desc) return "It attempts to confuse all nearby visible monsters.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 15*pow);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (confuse_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_FISHING:
        if (name) return "Fishing";
        if (desc) return "It allows you to relax and unwind from the pressures of adventuring.";
        if (value) return format("%d", 100);
        if (cast)
        {
            int x, y;
            if (!get_rep_dir2(&dir)) return NULL;
            y = py+ddy[dir];
            x = px+ddx[dir];
            tsuri_dir = dir;
            if (!cave_have_flag_bold(y, x, FF_WATER))
            {
                msg_print("There is no fishing place.");
                break;
            }
            else if (cave[y][x].m_idx)
            {
                char m_name[MAX_NLEN];
                monster_desc(m_name, &m_list[cave[y][x].m_idx], 0);
                msg_format("%^s is standing in your way.", m_name);
                energy_use = 0;
                break;
            }
            set_action(ACTION_FISH);
            p_ptr->redraw |= (PR_STATE);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_CHARGE:
        if (name) return "Charge";
        if (desc) return "If riding, you charge a chosen foe doing extra damage.";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            bool charged = FALSE;
            /* For the lance activation, the player really should be riding.
               At the moment, only the Heavy Lance 'Impaler' has this effect. */
            if (!p_ptr->riding)
            {
                msg_print("You need to be mounted in order to charge.");
                return NULL;
            }
            charged = rush_attack(7, NULL);
            if (!charged) return NULL;
        }
        break;
    case EFFECT_PIERCING_SHOT:
        if (name) return "Piercing Shot";
        if (desc) return "It shoots a bolt through multiple foes.";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            bool fired = FALSE;
            msg_print("");
            shoot_hack = SHOOT_PIERCE;
            fired = do_cmd_fire();
            shoot_hack = SHOOT_NONE;
            if (!fired) return NULL;
            device_known = TRUE;
        }
        break;
    case EFFECT_ENDLESS_QUIVER: /* should only be on a quiver ... */
        if (name) return "Endless Quiver";
        if (desc) return "Your quiver will refill with average ammo.";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            obj_t forge = {0};
            int   tval = p_ptr->shooter_info.tval_ammo;

            if ((!tval) || (tval == TV_NO_AMMO)) tval = TV_ARROW;
            if (tval == TV_ANY_AMMO) tval = TV_BOLT;

            object_prep(&forge, lookup_kind(tval, SV_ARROW)); /* Hack: SV_ARROW == SV_BOLT == SV_PEBBLE */
            forge.number = MAX(0, MIN(50, quiver_capacity() - quiver_count(NULL)));
            obj_identify_fully(&forge);
            object_origins(&forge, ORIGIN_ENDLESS);

            if (!forge.number)
                msg_print("Your quiver is full.");
            else
            {
                msg_print("Your quiver refills.");
                quiver_carry(&forge);
            }
        }
        break;
    case EFFECT_WALL_BUILDING:
        if (name) return "Wall Building";
        if (desc) return "It creates a wall of stone.";
        if (value) return format("%d", 50000);
        if (color) return format("%d", TERM_UMBER);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            fire_beam(GF_MAKE_WALL, dir, 0);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_SLEEP_MONSTER:
    {
        int power = _extra(effect, 10 + effect->power);
        if (name) return "Sleep Monster";
        if (desc) return "It puts a monster to sleep when you use it.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", 10*power);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (sleep_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SLOW_MONSTER:
    {
        int power = _extra(effect, MAX(10, effect->power * 2 - 6));
        if (name) return "Slow Monster";
        if (desc) return "It slows a monster down when you use it.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", 15 * power);
        if (color) return format("%d", TERM_SLATE);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (slow_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CONFUSE_MONSTER:
    {
//        int power = _extra(effect, MIN(98, MAX(21, effect->power * 7 - 52)));
        int power = _extra(effect, MIN(105, 25 + _power_curve_offset(75, effect->power + 75, 80)));
        if (name) return "Confuse Monster";
        if (desc) return "It confuses a monster when you use it.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", 15*power);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (confuse_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SCARE_MONSTER:
    {
        int power = _extra(effect, 10 + effect->power);
        if (name) return "Scare Monster";
        if (desc) return "It scares a monster when you use it.";
        if (info) return format("power %d", _BOOST(power));
        if (value) return format("%d", 10*power);
        if (color) return format("%d", res_color(RES_FEAR));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (fear_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_POLYMORPH:
        if (name) return "Polymorph";
        if (desc) return "It changes a monster into another when you use it.";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (poly_monster(dir))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_STARLITE:
    {
        int dd = _extra(effect, 6 + effect->power / 10);
        if (name) return "Starlight";
        if (desc) return "It fires multiple beams of weak light in random open directions when you use it.";
        if (value) return format("%d", 750);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            int num = damroll(5, 3);
            int y = 0, x = 0, k;
            int attempts;

            for (k = 0; k < num; k++)
            {
                attempts = 1000;
                while (attempts--)
                {
                    scatter(&y, &x, py, px, 4, 0);
                    if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                    if (!player_bold(y, x)) break;
                }
                project(0, 0, y, x, _BOOST(damroll(dd, 10)), GF_LITE_WEAK,
                          PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_KILL);
            }
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_NOTHING:
        if (name) return "Nothing";
        if (desc) return "It is your food.";
        if (value) return format("%d", 1);
        if (cast)
            msg_print("What a pity ... You are wasting your food!");
        break;

    /* Bad Effects */
    case EFFECT_AGGRAVATE:
        if (name) return "Aggravate Monsters";
        if (desc) return "It aggravates nearby monsters.";
        if (value) return format("%d", 100); /* This actually *can* be useful ... */
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            aggravate_monsters(0);
            device_known = TRUE;
        }
        break;
    case EFFECT_HEAL_MONSTER:
    {
        int dd = _extra(effect, 10 + effect->power / 10);
        if (name) return "Heal Monster";
        if (desc) return "It heals a monster when you use it.";
        if (info) return format("heal %dd10", dd);
        if (value) return format("%d", dd / 2);
        if (cast)
        {
            bool old_target_pet = target_pet;
            target_pet = TRUE;
            if (!get_fire_dir(&dir))
            {
                target_pet = old_target_pet;
                return NULL;
            }
            target_pet = old_target_pet;
            if (heal_monster(dir, _BOOST(damroll(dd, 10))))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HASTE_MONSTER:
        if (name) return "Haste Monster";
        if (desc) return "It hastes a monster when you use it.";
        if (color) return format("%d", TERM_VIOLET);
        if (value) return format("%d", 15);
        if (cast)
        {
            bool old_target_pet = target_pet;
            target_pet = TRUE;
            if (!get_fire_dir(&dir))
            {
                target_pet = old_target_pet;
                return NULL;
            }
            target_pet = old_target_pet;
            if (speed_monster(dir))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_HASTE_MONSTERS:
        if (name) return "Haste Monsters";
        if (desc) return "It hastes all monsters in sight when you use it.";
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (speed_monsters())
                device_noticed = TRUE;
        }
        break;
    case EFFECT_CLONE_MONSTER:
        if (name) return "Clone Monster";
        if (desc) return "It clones a non-unique monster when you use it.";
        if (value) return format("%d", 10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (clone_monster(dir))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DARKNESS:
        if (name) return "Darkness";
        if (desc) return "It darkens nearby area or current room and blinds you when you use it.";
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_DARK))
            {
                if (set_blind(p_ptr->blind + 3 + randint1(5), FALSE))
                    device_noticed = TRUE;
            }
            if (unlite_area(10, 3))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_ANGRY_MONSTERS:
        if (name) return "Summoning";
        if (desc) return "It summons several monsters as enemies when you use it.";
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            int i;
            int num = randint1(4);
            for (i = 0; i < num; i++)
            {
                if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
                    device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SLOWNESS:
        if (name) return "Slowness";
        if (desc) return "It slows you down temporarily when you use it.";
        if (color) return format("%d", TERM_UMBER);
        if (cast)
        {
            if (set_slow(p_ptr->slow + randint1(30) + 15, FALSE))
                device_noticed = TRUE;
        }
        break;

    /* Specific Artifacts ... Try to minimize! */
    case EFFECT_JEWEL:
        if (name) return "Clairvoyance and Recall";
        if (desc) return "It maps, lights permanently and detects all items on the entire level.";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            wiz_lite(p_ptr->tim_superstealth > 0);
            msg_print("The Jewel drains your vitality...");
            take_hit(DAMAGE_LOSELIFE, damroll(3, 8), "the Jewel of Judgement");
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
            if (get_check("Activate recall? "))
                word_of_recall(TRUE);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_HERMES:
        if (name) return "Haste and Dimension Door";
        if (desc) return "It hastes you and teleports you to a chosen nearby location.";
        if (value) return format("%d", 15000);
        if (cast)
        {
            if (set_fast(_BOOST(randint1(75) + 75), FALSE)) device_noticed = TRUE;
            if (dimension_door(_BOOST(p_ptr->lev / 2 + 10))) device_noticed = TRUE;
        }
        break;
    case EFFECT_ARTEMIS:
        if (name) return "Create Arrows";
        if (desc) return "It magically creates arrows out of thin air!";
        if (value) return format("%d", 7500);
        if (cast)
        {
            object_type forge;
            char o_name[MAX_NLEN];

            object_prep(&forge, lookup_kind(TV_ARROW, m_bonus(1, p_ptr->lev)+ 1));
            forge.number = (byte)rand_range(5, 10);
            apply_magic(&forge, p_ptr->lev, AM_NO_FIXED_ART);
            obj_identify(&forge);
            object_origins(&forge, ORIGIN_ACQUIRE);
            
            forge.discount = 99;

            object_desc(o_name, &forge, 0);
            msg_format("It creates %s.", o_name);

            pack_carry(&forge);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DEMETER:
        if (name) return "Flame of Demeter";
        if (desc) return "It heals you and satiates your hunger.";
        if (info) return info_heal(0, 0, _BOOST(500));
        if (value) return format("%d", 10000);
        if (cast)
        {
            if (hp_player(_BOOST(500))) device_noticed = TRUE;
            if (set_food(PY_FOOD_MAX - 1)) device_noticed = TRUE;
        }
        break;
    case EFFECT_EYE_VECNA:
        if (name) return "Eye of Vecna";
        if (desc) return "It opens your eyes to visions of all that surrounds you.";
        if (value) return format("%d", 10000);
        if (cast)
        {
            take_hit(DAMAGE_LOSELIFE, damroll(8, 8), "the Eye of Vecna");
            wiz_lite(TRUE);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_ONE_RING:
        if (name) return "Something Weird";
        if (desc) return "It does something completely unpredictable and probably rather bad.";
        if (value) return format("%d", 1000);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            ring_of_power(dir);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_BLADETURNER:
        if (name) return "Heroism, Resistance and Breathe Elements";
        if (desc) return "It grants temporary heroism, blessing and elemental resistance and also allows you to breathe the elements.";
        if (value) return format("%d", 10000);
        if (cast)
        {
            int dur;
            if (!get_fire_dir(&dir)) return NULL;
            msg_print("You breathe the elements.");

            fire_ball(GF_MISSILE, dir, _BOOST(300), 4);
            device_noticed = TRUE;

            msg_print("Your armor glows many colours...");

            dur = _BOOST(randint1(50) + 50);

            set_hero(dur, FALSE);
            set_blessed(dur, FALSE);
            set_oppose_base(dur, FALSE);
        }
        break;
    case EFFECT_MITO_KOUMON:
        if (name) return "Reveal Identity";
        if (desc) return "It reveals your true identity.";
        if (value) return format("%d", 1000);
        if (cast)
        {
            int count = 0, i;
            monster_type *m_ptr;
            cptr kakusan = "";

            if (summon_named_creature(0, py, px, MON_SUKE, PM_FORCE_PET))
            {
                msg_print("Suke-san appears.");
                kakusan = "Suke-san";
                count++;
            }
            if (summon_named_creature(0, py, px, MON_KAKU, PM_FORCE_PET))
            {
                msg_print("Kaku-san appears.");
                kakusan = "Kaku-san";
                count++;
            }
            if (!count)
            {
                for (i = m_max - 1; i > 0; i--)
                {
                    m_ptr = &m_list[i];
                    if (!m_ptr->r_idx) continue;
                    if (!((m_ptr->r_idx == MON_SUKE) || (m_ptr->r_idx == MON_KAKU))) continue;
                    if (!los(m_ptr->fy, m_ptr->fx, py, px)) continue;
                    if (!projectable(m_ptr->fy, m_ptr->fx, py, px)) continue;
                    count++;
                    break;
                }
            }
            if (count)
            {
                msg_format("%^s says 'WHO do you think this person is! Bow your head, down your knees!'", kakusan);

                sukekaku = TRUE;
                stun_monsters(15);
                confuse_monsters(120);
                turn_monsters(120);
                stasis_monsters(120);
                sukekaku = FALSE;
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_BLOODY_MOON:
        if (name) return "Change Zokusei";
        if (desc) return "It changes its slays and resistances.";
        if (value) return format("%d", 5000);
        if (cast)
        {
            /* TODO: Again, we need the underlying object ...
               For now, we safely assume the artifact is Bloody Moon. */
            int slot = equip_find_art(ART_BLOOD);
            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                get_bloody_moon_flags(o_ptr);
                obj_identify_fully(o_ptr);
                obj_display(o_ptr);
                if (p_ptr->prace == RACE_ANDROID) android_calc_exp();
                p_ptr->update |= (PU_BONUS | PU_HP);
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SACRED_KNIGHTS:
        if (name) return "Dispel Curse and Probing";
        if (desc) return "It removes all normal curses from your equipment and probes nearby monsters.";
        if (value) return format("%d", 5000);
        if (cast)
        {
            if (remove_all_curse())
            {
                msg_print("You feel as if someone is watching over you.");
                device_noticed = TRUE;
            }
            if (probing())
                device_noticed = TRUE;
        }
        break;
    case EFFECT_GONG:
    {
        int dam = _extra(effect, 3*effect->power);
        if (name) return "Bang a Gong";
        if (desc) return "It makes some very loud noise.";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (cast)
        {
            if (!res_save_default(RES_SOUND))
                project(-1, 0, py, px, _BOOST(dam), GF_SOUND, PROJECT_KILL | PROJECT_HIDE);
            project(0, 18, py, px, _BOOST(dam*2), GF_SOUND, PROJECT_KILL | PROJECT_ITEM);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_MURAMASA:
        if (name) return "Gain Strength";
        if (desc) return "It attempts to increase your strength, but is destroyed upon failure.";
        if (value) return format("%d", 5000);
        if (cast)
        {
            if (get_check("Are you sure?!"))
            {
                msg_print("");
                do_inc_stat(A_STR);
                if (one_in_(2))
                {
                    /* TODO: We need to know the exact equipment slot being used so
                        we can destroy it. For now, we assume the artifact is Muramasa.
                        Note: Effects might someday be triggered by spells, so passing
                        an object to this routine won't always make sense!
                     */
                    int slot = equip_find_art(ART_MURAMASA);
                    if (slot)
                    {
                        msg_print("The Muramasa is destroyed!");
                        curse_weapon(TRUE, slot);
                    }
                }
            }
        }
        break;
    case EFFECT_EXPERTSEXCHANGE: /* adapted from Frogspawn */
        if (name) return "Change Sex";
        if (desc) return "It changes your biological gender.";
        if (value) return format("%d", 100);
        if (cast)
        {
            if ((prace_is_(RACE_MON_POSSESSOR)) || (prace_is_(RACE_MON_MIMIC)))
            {
                msg_print("Nothing happens. Maybe you should just try another body?");
                break;
            }
            p_ptr->psex = (SEX_MALE + SEX_FEMALE) - p_ptr->psex;
            if (p_ptr->psex == SEX_FEMALE)
            {
                mut_lose(MUT_IMPOTENCE);
                take_hit(DAMAGE_NOESCAPE, 10, "sex reassignment");
                msg_print("Congratulations! You are now a female!");
                switch (randint0(7))
                {
                    case 0: msg_print("(Well, that was quick, and didn't hurt. Much.)"); break;
                    case 1: msg_print("(Time to start kicking ass.)"); break;
                    case 2: msg_print("(Already you hate those chauvinist male pigs.)"); break;
                    case 3: msg_print("(Your remaining inner dirty male is already getting ideas...)"); break;
                    case 4: msg_print("(At last you can think clearly, without all those weird hormones flowing through your body.)"); break;
                    case 5: msg_print("(The unfair sex! You've looked forward to this!)"); break;
                    default: msg_print("(You miss your old body a bit, but you've never been afraid to experiment.)"); break;
                }
            }
            else
            {
                if ((one_in_(12)) && (p_ptr->prace != RACE_ENT))
                {
                    msg_print("Congratulations! You are now an Ent!");
                    msg_print(NULL);
                    msg_print("Nah, just kidding. You are now a male.");
                    break;
                }
                else msg_print("Congratulations! You are now a male!");
                switch (randint0(7))
                {
                    case 0: msg_print("(Well, that was quick. You'll make them suffer now.)"); break;
                    case 1: msg_print("(You feel testosterone flowing through your body. Time to pillage!)"); break;
                    case 2: msg_print("(Who are you going to boss around now?)"); break;
                    case 3: msg_print("RAAAAAAAARRH!"); break;
                    case 4: msg_print("You feel very strong! (Maybe that's just you, though? Either that, or your stats haven't updated properly.)"); break;
                    case 5: msg_print("(You hope other males will stop pestering you now.)"); break;
                    default: msg_print("(They'd better pay you a hundred gold pieces for every eighty you earn now.)"); break;
                }
            }
            device_noticed = TRUE;
        }
        break;
    case EFFECT_STUNNING_KICK:
    {
        int pow = effect->power * 2;
        if (name) return "Stunning Stomp";
        if (desc) return "Forcefully stuns an adjacent monster with a powerful stomp.";
        if (info) return format("power %d", pow);
        if (value) return format("%d", 125*MAX(0, pow - 25));
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if ((d_info[dungeon_type].flags1 & DF1_NO_MELEE) || (no_melee_challenge))
            {
                msg_print("Something prevents you from attacking.");
                break;
            }
            else
            {
                int x = 0, y = 0, m_idx = 0;

                if (!get_adjacent_target(&x, &y, &m_idx))
                {
                    break;
                }

                (void)project(0, 0, y, x, pow, GF_STUN, PROJECT_STOP | PROJECT_KILL | PROJECT_THRU);
            }
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RAMA_ARROW:
    {
        if (name) return "Mighty Arrow";
        if (desc) return "Fires a single arrow with extra might, tripling the normal damage.";
        if (info) return "dam x3";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)                         {
            if (melee_challenge)
            {
                msg_print("You are reminded of your solemn vow to rely on melee only.");
                break;
            }
            else
            {
                shoot_hack = SHOOT_RAMA;
                command_cmd = 'f'; /* hack for inscriptions */
                do_cmd_fire();
                shoot_hack = 0;
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_UNFOCUS_RAGE:
    {
        int pow = _extra(effect, effect->power * 2);
        if (name) return "Unfocus Rage";
        if (desc) return ((p_ptr->pclass == CLASS_RAGE_MAGE) ? "Removes fear, removes berserk strength and converts up to 200 SP to HP." : "Removes fear and berserk strength.");
        if ((info) && (p_ptr->pclass == CLASS_RAGE_MAGE)) return format("power %d", pow);
        if (value) return format("%d", 2500);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
            if (set_shero(0, TRUE)) device_noticed = TRUE;
            if ((p_ptr->csp > 0) && (p_ptr->pclass == CLASS_RAGE_MAGE))
            {
                int healing = MIN(pow, p_ptr->csp);
                hp_player(healing);
                sp_player(-healing);
                device_noticed = TRUE;
            }
        }
        break;
    }
    default:
        if (name) return format("Invalid Effect: %d", effect->type);
    }
    return "";
}
#undef _BOOST

