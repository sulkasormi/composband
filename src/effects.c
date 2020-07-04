/* File: effects.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: effects of various "objects" */

#include "angband.h"

#include <assert.h>

bool free_act_save_p(int ml)
{
    int i, skill = p_ptr->skills.sav;
    if (p_ptr->pclass == CLASS_BERSERKER) return TRUE; /* negative skills */

    /* Put in a hard limit because Chris's implementation was so unpopular */
    if (p_ptr->free_act >= 3) return TRUE;
    if ((ml < 42) && (p_ptr->free_act == 2)) return TRUE;

    for (i = 0; i < p_ptr->free_act; i++)
    {
        if (randint0(100 + ml/2) < skill)
        {
            equip_learn_flag(OF_FREE_ACT);
            return TRUE;
        }
    }
    return FALSE;
}

void set_action(int typ)
{
    int prev_typ = p_ptr->action;

    if (typ == prev_typ)
        return;
    else
    {
        switch (prev_typ)
        {
        case ACTION_SEARCH:
            msg_print("You no longer walk carefully.");
            p_ptr->redraw |= PR_EFFECTS;
            break;
        case ACTION_REST:
            resting = 0;
            break;
        case ACTION_LEARN:
            msg_print("You stop learning spells.");
            new_mane = FALSE;
            break;
        case ACTION_KAMAE:
            msg_print("You stop assuming the posture.");
            p_ptr->special_defense &= ~(KAMAE_MASK);
            break;
        case ACTION_KATA:
            msg_print("You stop assuming the posture.");
            p_ptr->special_defense &= ~(KATA_MASK);
            p_ptr->update |= (PU_MONSTERS);
            p_ptr->redraw |= (PR_STATUS);
            break;
        case ACTION_SING:
            msg_print("You stop singing.");
            break;
        case ACTION_QUICK_WALK:
            msg_print("You are no longer moving extremely fast.");
            break;
        case ACTION_SPELL:
            msg_print("You stopped spelling all spells.");
            break;
        case ACTION_STALK:
            msg_print("You no longer stalk your prey.");
            break;
        }
    }

    p_ptr->action = typ;

    if (prev_typ == ACTION_SING) bard_stop_singing();

    switch (p_ptr->action)
    {
    case ACTION_SEARCH:
        msg_print("You begin to walk carefully.");
        p_ptr->redraw |= PR_EFFECTS;
        break;
    case ACTION_LEARN:
        msg_print("You begin to learn spells.");
        break;
    case ACTION_FISH:
        msg_print("You begin fishing...");
        break;
    case ACTION_QUICK_WALK:
        msg_print("You begin to move extremely fast.");
        break;
    case ACTION_STALK:
        msg_print("You begin to stalk your prey.");
        break;
    }

    p_ptr->update |= (PU_BONUS);
    if (p_ptr->action == ACTION_GLITTER || prev_typ == ACTION_GLITTER)
        p_ptr->update |= PU_FLOW;
    p_ptr->redraw |= (PR_STATE);
}

/* Stop singing or spelling
 * Compare stop_mouth() in racial.c */
void interrupt_singing(bool take_energy)
{
    warlock_stop_singing();
    if (music_singing_any() || hex_spelling_any())
    {
        cptr str = (music_singing_any()) ? "singing" : "spelling";
        p_ptr->magic_num1[1] = p_ptr->magic_num1[0];
        p_ptr->magic_num1[0] = 0;
        msg_format("Your %s is interrupted.", str);
        p_ptr->action = ACTION_NONE;

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS | PU_HP);

        /* Redraw map */
        p_ptr->redraw |= (PR_MAP | PR_STATUS | PR_STATE);

        /* Update monsters */
        p_ptr->update |= (PU_MONSTERS);

        /* Window stuff */
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

        if (take_energy) p_ptr->energy_need += PY_ENERGY_NEED();
    }
}

/* reset timed flags */
void reset_tim_flags(void)
{
    p_ptr->fast = 0;            /* Timed -- Fast */
    p_ptr->lightspeed = 0;
    p_ptr->slow = 0;            /* Timed -- Slow */
    p_ptr->minislow = 0;        /* Pseudo-timed */
    p_ptr->mini_energy = 0;
    p_ptr->blind = 0;           /* Timed -- Blindness */
    p_ptr->paralyzed = 0;       /* Timed -- Paralysis */
    p_ptr->confused = 0;        /* Timed -- Confusion */
    p_ptr->afraid = 0;          /* Timed -- Fear */
    p_ptr->image = 0;           /* Timed -- Hallucination */
    p_ptr->poisoned = 0;        /* Timed -- Poisoned */
    p_ptr->cut = 0;             /* Timed -- Cut */
    p_ptr->stun = 0;            /* Timed -- Stun */

    p_ptr->protevil = 0;        /* Timed -- Protection */
    p_ptr->invuln = 0;          /* Timed -- Invulnerable */
    p_ptr->ult_res = 0;
    p_ptr->hero = 0;            /* Timed -- Heroism */
    p_ptr->shero = 0;           /* Timed -- Super Heroism */
    p_ptr->shield = 0;          /* Timed -- Shield Spell */
    p_ptr->blessed = 0;         /* Timed -- Blessed */
    p_ptr->tim_invis = 0;       /* Timed -- Invisibility */
    p_ptr->tim_infra = 0;       /* Timed -- Infra Vision */
    p_ptr->tim_poet = 0;        /* Timed -- Poet */
    p_ptr->tim_understanding = 0; /* Timed -- Auto-ID */
    p_ptr->tim_regen = 0;       /* Timed -- Regeneration */
    p_ptr->tim_stealth = 0;     /* Timed -- Stealth */
    p_ptr->tim_esp = 0;
    p_ptr->wraith_form = 0;     /* Timed -- Wraith Form */
    p_ptr->tim_levitation = 0;
    p_ptr->tim_sh_touki = 0;
    p_ptr->tim_sh_fire = 0;
    p_ptr->tim_sh_holy = 0;
    p_ptr->tim_eyeeye = 0;
    p_ptr->magicdef = 0;
    p_ptr->resist_magic = 0;
    p_ptr->tsuyoshi = 0;
    p_ptr->kabenuke = 0;
    p_ptr->tim_res_nether = 0;
    p_ptr->tim_res_time = 0;
    p_ptr->tim_mimic = 0;
    if (p_ptr->prace == RACE_DOPPELGANGER && p_ptr->mimic_form != MIMIC_NONE)
    {
        mimic_race(MIMIC_NONE, NULL);
    }
    else p_ptr->mimic_form = MIMIC_NONE;
    p_ptr->tim_reflect = 0;
    p_ptr->multishadow = 0;
    p_ptr->dustrobe = 0;
    p_ptr->action = ACTION_NONE;
    
    p_ptr->tim_spurt = 0;
    p_ptr->tim_blood_rite = 0;
    p_ptr->tim_blood_shield = 0;
    p_ptr->tim_blood_seek = 0;
    p_ptr->tim_blood_sight = 0;
    p_ptr->tim_blood_feast = 0;
    p_ptr->tim_blood_revenge = 0;
    p_ptr->tim_superstealth = 0;
    p_ptr->tim_force = 0;
    p_ptr->tim_field = 0;
    p_ptr->fasting = FALSE;
    p_ptr->tim_sustain_str = 0;
    p_ptr->tim_sustain_int = 0;
    p_ptr->tim_sustain_wis = 0;
    p_ptr->tim_sustain_dex = 0;
    p_ptr->tim_sustain_con = 0;
    p_ptr->tim_sustain_chr = 0;
    p_ptr->tim_hold_life = 0;
    p_ptr->tim_transcendence = 0;
    p_ptr->tim_quick_walk = 0;
    p_ptr->tim_inven_prot = 0;
    p_ptr->tim_inven_prot2 = 0;
    p_ptr->tim_device_power = 0;
    p_ptr->tim_sh_time = 0;
    p_ptr->tim_foresight = 0;

    p_ptr->oppose_acid = 0;     /* Timed -- oppose acid */
    p_ptr->oppose_elec = 0;     /* Timed -- oppose lightning */
    p_ptr->oppose_fire = 0;     /* Timed -- oppose heat */
    p_ptr->oppose_cold = 0;     /* Timed -- oppose cold */
    p_ptr->oppose_pois = 0;     /* Timed -- oppose poison */
    p_ptr->spin = 0;            /* Timed -- spin (inc. oppose nether) */

    p_ptr->word_recall = 0;
    p_ptr->alter_reality = 0;
    p_ptr->sutemi = FALSE;
    p_ptr->counter = FALSE;
    p_ptr->ele_attack = 0;
    p_ptr->ele_immune = 0;
    p_ptr->special_attack = 0L;
    p_ptr->special_defense = 0L;

    wild_reset_counters();

    while(p_ptr->energy_need < 0) p_ptr->energy_need += PY_ENERGY_NEED();
    world_player = FALSE;

    if ((p_ptr->pclass == CLASS_BERSERKER) || (beorning_is_(BEORNING_FORM_BEAR))) p_ptr->shero = 1;
    else if (p_ptr->pclass == CLASS_ALCHEMIST)
    {
        alchemist_set_hero(NULL, 0, TRUE);
        alchemist_set_hero(NULL, 0, FALSE);
    }

    if (p_ptr->riding)
    {
        (void)set_monster_fast(p_ptr->riding, 0);
        (void)set_monster_slow(p_ptr->riding, 0);
        (void)set_monster_invulner(p_ptr->riding, 0, FALSE);
    }

    if ((p_ptr->pclass == CLASS_BARD) || (p_ptr->realm1 == REALM_HEX))
    {
        p_ptr->magic_num1[0] = 0;
        p_ptr->magic_num2[0] = 0;
    }
    if (disciple_is_(DISCIPLE_TROIKA)) troika_wipe_timeouts();
}

byte _slow_calc(bool is_slow, byte minislow)
{
    if (is_slow) return (10 + (minislow / 4));
    return minislow;
}

byte player_slow(void)
{
    return _slow_calc(((p_ptr->slow > 0) ? TRUE : FALSE), p_ptr->minislow);
}

byte monster_slow(monster_type *m_ptr)
{
    return _slow_calc(((MON_SLOW(m_ptr) > 0) ? TRUE : FALSE), m_ptr->minislow);
}

byte _inc_minislow(int minislow, int lisays)
{
    minislow += lisays;
    
    /* Force good values */
    if (minislow < 0) minislow = 0;
    if (minislow > 10) minislow = 10;

    return minislow;
}

/* Return TRUE if something happened, FALSE otherwise */
bool p_inc_minislow(int lisays)
{
    byte vanha = p_ptr->minislow;
    bool tulos;

    if (p_ptr->is_dead) return FALSE; /* paranoia */
    if (p_ptr->no_slow) return FALSE;

    p_ptr->minislow = _inc_minislow(p_ptr->minislow, lisays);
    tulos = (p_ptr->minislow != vanha);

    if (!tulos) return FALSE;

    if (!p_ptr->minislow)
    {
        msg_print("You no longer feel sluggish.");
    }
    else if (!vanha)
    {
        msg_print("You feel very sluggish!");
    }

    /* Disturb */
    if ((disturb_state) && ((!p_ptr->minislow) || (lisays > 0))) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= PU_BONUS;

    /* Handle stuff */
    handle_stuff();

    return TRUE;
}

void p_inc_fatigue(int check_mut, int lisays)
{
    if ((((!check_mut) || (!mut_present(check_mut))) && (!p_ptr->no_air)) || (lisays < 1)) return;

    /* Check for easy tiring */
    if ((one_in_(16 - p_ptr->minislow)) || ((check_mut == MUT_EASY_TIRING2) && (one_in_(6))))
    {
        if (p_ptr->mini_energy >= lisays)
        {
            p_ptr->mini_energy -= lisays;
        }
        else if (p_inc_minislow(1)) p_ptr->mini_energy += MAX(0, (100 - lisays));
        else p_ptr->mini_energy = 0;
    }
}

bool m_inc_minislow(monster_type *m_ptr, int lisays)
{
    byte vanha = m_ptr->minislow;
    bool tulos;
    m_ptr->minislow = _inc_minislow(m_ptr->minislow, lisays);
    tulos = (m_ptr->minislow != vanha);

    if (!tulos) return FALSE;

    check_mon_health_redraw(m_ptr->id);

    /* Check if the player's mount was affected */
    if ((p_ptr->riding == m_ptr->id) && (!p_ptr->leaving))
        p_ptr->update |= PU_BONUS;

    return TRUE;
}

void check_muscle_sprains(int chance, char *viesti)
{
    if (!mut_present(MUT_HUMAN_DEX)) return;
    if (!one_in_(chance)) return;
    if (p_ptr->slow) return;
    msg_format("%s", viesti);
    set_slow(50 + randint1(50), FALSE);
}

/* TODO: Timed player effects needs a complete rework ... */
bool disenchant_player(void)
{
    int attempts = 200;
    bool result = FALSE;
    if (disciple_is_(DISCIPLE_TROIKA)) result = troika_dispel_timeouts();
    for (; attempts; attempts--)
    {
        switch (randint1(33))
        {
        case 1:
            if (p_ptr->fast)
            {
                (void)set_fast(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 2:
            if (p_ptr->lightspeed)
            {
                (void)set_lightspeed(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 3:
            if (p_ptr->shield || p_ptr->tim_blood_shield)
            {
                (void)set_shield(0, TRUE);
                set_tim_blood_shield(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 4:
            if (p_ptr->blessed)
            {
                (void)set_blessed(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 5:
            if (p_ptr->hero)
            {
                (void)set_hero(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 6:
            if (p_ptr->shero)
            {
                (void)set_shero(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 7:
            if (p_ptr->protevil)
            {
                (void)set_protevil(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 8:
            if (p_ptr->invuln)
            {
                (void)set_invuln(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 9:
            if (p_ptr->wraith_form)
            {
                (void)set_wraith_form(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 10:
            if (p_ptr->kabenuke)
            {
                (void)set_kabenuke(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 11:
            if (p_ptr->tim_res_nether)
            {
                (void)set_tim_res_nether(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 12:
            if (p_ptr->tim_res_time)
            {
                (void)set_tim_res_time(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 13:
            if (p_ptr->tim_res_disenchantment)
            {
                (void)set_tim_res_disenchantment(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 14:
            if (p_ptr->tim_reflect)
            {
                (void)set_tim_reflect(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 15:
            if (p_ptr->tim_esp || p_ptr->tim_blood_seek || p_ptr->tim_blood_sight)
            {
                (void)set_tim_esp(0, TRUE);
                set_tim_blood_seek(0, TRUE);
                set_tim_blood_sight(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 16:
            if (p_ptr->tim_regen)
            {
                (void)set_tim_regen(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 17:
            if (p_ptr->tim_eyeeye || p_ptr->tim_blood_revenge)
            {
                (void)set_tim_eyeeye(0, TRUE);
                set_tim_blood_revenge(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 18:
            if (p_ptr->magicdef)
            {
                (void)set_magicdef(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 19:
            if (p_ptr->oppose_acid || p_ptr->oppose_cold || p_ptr->oppose_elec || p_ptr->oppose_fire || p_ptr->oppose_pois || p_ptr->spin)
            {
                (void)set_oppose_base(0, TRUE);
                (void)set_spin(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 20:
            if (p_ptr->ult_res)
            {
                (void)set_ultimate_res(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 21:
            if (p_ptr->ele_attack)
            {
                (void)set_ele_attack(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 22:
            if (p_ptr->ele_immune)
            {
                (void)set_ele_immune(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 23:
            if (p_ptr->tim_field)
            {
                (void)set_tim_field(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 24:
            if (p_ptr->tim_force)
            {
                (void)set_tim_force(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 25:
            if (p_ptr->tim_building_up)
            {
                (void)set_tim_building_up(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 26:
            if (p_ptr->tim_enlarge_weapon)
            {
                (void)set_tim_enlarge_weapon(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 27:
            if (p_ptr->tim_quick_walk)
            {
                (void)set_tim_quick_walk(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 28:
            if ((p_ptr->tim_inven_prot) || (p_ptr->tim_inven_prot2))
            {
                (void)set_tim_inven_prot(0, TRUE);
                (void)set_tim_inven_prot2(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 29:
            if (p_ptr->tim_dark_stalker)
            {
                (void)set_tim_dark_stalker(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 30:
            if (p_ptr->tim_nimble_dodge)
            {
                (void)set_tim_nimble_dodge(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 31:
            if (p_ptr->tim_stealthy_snipe)
            {
                (void)set_tim_stealthy_snipe(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 32:
            warlock_stop_singing();
            if (music_singing_any() || hex_spelling_any())
            {
                interrupt_singing(TRUE);
                result = TRUE;
                return result;
            }
            break;
        case 33:
            if (p_ptr->tim_poet)
            {
                (void)set_tim_poet(0, TRUE);
                result = TRUE;
                return result;
            }
            break;
        }
    }

    return result;
}

void dispel_player(void)
{
    (void)set_fast(0, TRUE);
    (void)set_lightspeed(0, TRUE);
    (void)set_slow(0, TRUE);
    (void)set_shield(0, TRUE);
    (void)set_blessed(0, TRUE);
    (void)set_tsuyoshi(0, TRUE);
    (void)set_hero(0, TRUE);
    (void)set_shero(0, TRUE);
    (void)set_protevil(0, TRUE);
    (void)set_invuln(0, TRUE);
    (void)set_wraith_form(0, TRUE);
    (void)set_kabenuke(0, TRUE);
    (void)set_tim_res_nether(0, TRUE);
    (void)set_tim_res_time(0, TRUE);
    (void)set_tim_res_disenchantment(0, TRUE);
    /* by henkma */
    (void)set_tim_reflect(0,TRUE);
    (void)set_multishadow(0,TRUE);
    (void)set_dustrobe(0,TRUE);

    (void)set_tim_invis(0, TRUE);
    (void)set_tim_infra(0, TRUE);
    (void)set_tim_poet(0, TRUE);
    (void)set_tim_understanding(0, TRUE);
    (void)set_tim_esp(0, TRUE);
    (void)set_tim_esp_magical(0, TRUE);
    (void)set_tim_regen(0, TRUE);
    (void)set_tim_stealth(0, TRUE);
    (void)set_tim_levitation(0, TRUE);
    (void)set_tim_sh_touki(0, TRUE);
    (void)set_tim_sh_fire(0, TRUE);
    (void)set_tim_sh_elements(0, TRUE);
    set_tim_sh_shards(0, TRUE);
    set_tim_sh_domination(0, TRUE);
    (void)set_tim_weaponmastery(0, TRUE);
    (void)set_tim_sh_holy(0, TRUE);
    (void)set_tim_eyeeye(0, TRUE);
    (void)set_magicdef(0, TRUE);
    (void)set_resist_magic(0, TRUE);
    (void)set_oppose_base(0, TRUE);
    (void)set_ultimate_res(0, TRUE);
    (void)set_spin(0, TRUE);
    
    /* Its important that doppelganger gets called correctly and not set_mimic()
       since we monkey with things like the experience factor! */
    if (p_ptr->prace == RACE_DOPPELGANGER && p_ptr->mimic_form != MIMIC_NONE && !p_ptr->tim_mimic)
        mimic_race(MIMIC_NONE, NULL);
    else
        (void)set_mimic(0, 0, TRUE);
        
    (void)set_ele_attack(0, 0);
    (void)set_ele_immune(0, 0);
    set_sanctuary(FALSE);

    set_tim_blood_shield(0, TRUE);
    set_tim_blood_seek(0, TRUE);
    set_tim_blood_sight(0, TRUE);
    set_tim_blood_feast(0, TRUE);
    set_tim_blood_revenge(0, TRUE);

    set_tim_force(0, TRUE);
    set_tim_building_up(0, TRUE);
    set_tim_enlarge_weapon(0, TRUE);
    set_tim_field(0, TRUE);

    set_tim_spell_reaction(0, TRUE);
    set_tim_resist_curses(0, TRUE);
    set_tim_armor_of_fury(0, TRUE);
    set_tim_spell_turning(0, TRUE);

    set_tim_sustain_str(0, TRUE);
    set_tim_sustain_int(0, TRUE);
    set_tim_sustain_wis(0, TRUE);
    set_tim_sustain_dex(0, TRUE);
    set_tim_sustain_con(0, TRUE);
    set_tim_sustain_chr(0, TRUE);
    set_tim_hold_life(0, TRUE);
    set_tim_transcendence(0, TRUE);
    set_tim_quick_walk(0, TRUE);
    set_tim_inven_prot(0, TRUE);
    set_tim_inven_prot2(0, TRUE);
    set_tim_device_power(0, TRUE);
    set_tim_sh_time(0, TRUE);
    set_tim_foresight(0, TRUE);

    set_tim_dark_stalker(0, TRUE);
    set_tim_nimble_dodge(0, TRUE);
    set_tim_stealthy_snipe(0, TRUE);

    set_tim_killing_spree(0, TRUE);
    set_tim_slay_sentient(0, TRUE);

    set_tim_spurt(0, TRUE);

    wild_dispel_player();
    psion_dispel_player();
    mimic_dispel_player();

    set_filibuster(FALSE);

    /* Cancel glowing hands */
    if (p_ptr->special_attack & ATTACK_CONFUSE)
    {
        p_ptr->special_attack &= ~(ATTACK_CONFUSE);
        msg_print("Your hands stop glowing.");
    }
    interrupt_singing(TRUE);
}


/*
 * Set "p_ptr->tim_mimic", and "p_ptr->mimic_form",
 * notice observable changes
 */
bool set_mimic(int v, int p, bool do_dec)
{
    bool notice = FALSE;
    bool dragon_poly = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Werewolves get their own two forms - human and wolf - and that's it.
     * In any case, horrible things might happen if werewolves could mimic,
     * or if anyone else could mimic a werewolf, due to the mega-hacks
     * involved in werewolf equipment handling */
    if (prace_is_(RACE_WEREWOLF)) return FALSE;
    if (get_race()->flags & RACE_NO_POLY) return FALSE;
    if ((p == RACE_WEREWOLF) || (p == RACE_BEORNING)) /* please don't let this even happen */
    {
        v = 0;
        p = MIMIC_NONE; /* paranoia */
    }


    if (p == MIMIC_DRAGON)
    {
        if (!disciple_is_(DISCIPLE_KARROT)) /* further paranoia */
        {
            v = 0;
            p = MIMIC_NONE;
        }
    }
    if ((p != MIMIC_NONE) && ((get_race_aux(p, 0)->flags & RACE_NO_POLY)))
    {
        v = 0;
        p = MIMIC_NONE;
    }

    dragon_poly = ((p == MIMIC_DRAGON) || (p_ptr->mimic_form == MIMIC_DRAGON));

    /* Open */
    if (v)
    {
        if (p_ptr->tim_mimic && (p_ptr->mimic_form == p) && !do_dec)
        {
            if (p_ptr->tim_mimic > v) return FALSE;
        }
        else if ((!p_ptr->tim_mimic) || (p_ptr->mimic_form != p))
        {
            msg_print("You feel that your body changes.");
            if (p_ptr->prace == RACE_DOPPELGANGER) mimic_race(MIMIC_NONE, "suppress");
            p_ptr->mimic_form=p;
            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_mimic)
        {
            msg_print("You are no longer transformed.");
            if (p_ptr->prace == RACE_DOPPELGANGER) 
            {
                p_ptr->tim_mimic = v; 
                mimic_race(MIMIC_NONE, NULL);
            }
            else p_ptr->mimic_form= MIMIC_NONE;
            notice = TRUE;
            p = MIMIC_NONE;
        }
    }

    /* Use the value */
    p_ptr->tim_mimic = v;
    if (dragon_poly) karrot_equip_on_poly();
    equip_on_change_race();

    /* Nothing to notice */
    if (!notice)
        return (FALSE);

    /* Disturb */
    if (disturb_state)
        disturb(0, 0);

    /* Redraw title */
    p_ptr->redraw |= (PR_BASIC | PR_STATUS | PR_MAP | PR_EQUIPPY | PR_EFFECTS);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_TORCH);
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->blind", notice observable changes
 *
 * Note the use of "PU_UN_LITE" and "PU_UN_VIEW", which is needed to
 * memorize any terrain features which suddenly become "visible".
 * Note that blindness is currently the only thing which can affect
 * "player_can_see_bold()".
 */
bool set_blind(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (!p_ptr->blind)
        {
            if (p_ptr->prace == RACE_ANDROID)
            {
                msg_print("You are blind!");
            }
            else
            {
                msg_print("You are blind!");
            }

            notice = TRUE;
            virtue_add(VIRTUE_ENLIGHTENMENT, -1);
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->blind)
        {
            if (p_ptr->prace == RACE_ANDROID)
            {
                msg_print("You can see again.");
            }
            else
            {
                msg_print("You can see again.");
            }

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->blind = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_EFFECTS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Fully update the visuals */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE | PU_VIEW | PU_LITE | PU_MONSTERS | PU_MON_LITE);
    if (prace_is_(RACE_MON_BEHOLDER))
        p_ptr->update |= PU_BONUS;

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

void lose_kata(void)
{
    msg_print("Your posture gets loose.");
    p_ptr->special_defense &= ~(KATA_MASK);
    p_ptr->update |= (PU_BONUS);
    p_ptr->update |= (PU_MONSTERS);
    p_ptr->redraw |= (PR_STATE);
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->action = ACTION_NONE;
}

void lose_kamae(void)
{
    msg_print("Your posture gets loose.");
    p_ptr->special_defense &= ~(KAMAE_MASK);
    p_ptr->update |= (PU_BONUS);
    p_ptr->redraw |= (PR_STATE);
    p_ptr->action = ACTION_NONE;
}

/*
 * Set "p_ptr->confused", notice observable changes
 */
bool set_confused(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if ((p_ptr->confused > v) && (!do_dec)) return FALSE;
        if (!p_ptr->confused)
        {
            msg_print("You are confused!");

            if (p_ptr->action == ACTION_LEARN)
            {
                msg_print("You cannot continue Learning!");
                new_mane = FALSE;

                p_ptr->redraw |= (PR_STATE);
                p_ptr->action = ACTION_NONE;
            }
            if (p_ptr->action == ACTION_KAMAE)
            {
                lose_kamae();
            }
            else if (p_ptr->action == ACTION_KATA)
            {
                lose_kata();
            }

            /* Sniper */
            if (p_ptr->concent) reset_concentration(TRUE);

            /* Hex */
            if (hex_spelling_any()) stop_hex_spell_all();

            notice = TRUE;
            p_ptr->counter = FALSE;
            virtue_add(VIRTUE_HARMONY, -1);
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->confused)
        {
            msg_print("You feel less confused now.");

            p_ptr->special_attack &= ~(ATTACK_SUIKEN);
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->confused = v;

    p_ptr->redraw |= PR_EFFECTS;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->poisoned", notice observable changes
 */
bool set_poisoned(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 20000) ? 20000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if ((p_ptr->poisoned > v) && (!do_dec)) return FALSE;
        if (!p_ptr->poisoned)
        {
            msg_print("You are poisoned!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->poisoned)
        {
            msg_print("You are no longer poisoned.");

            notice = TRUE;
        }
    }

    if ((v < p_ptr->poisoned) && (alert_poison)) poison_warning_hack = MIN(255, (v + 9) / 10);

    /* Use the value */
    p_ptr->poisoned = v;

    p_ptr->redraw |= PR_EFFECTS;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->paralyzed", notice observable changes
 */
bool set_paralyzed(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;
    if (!do_dec && p_ptr->paralyzed) return FALSE;

    /* Open */
    if (v)
    {
        if (!p_ptr->paralyzed)
        {
            if (repose_of_the_dead)
                msg_print("You enter a deep sleep!");
            else
                msg_print("<color:v>You are paralyzed!</color>");

            /* Sniper */
            if (p_ptr->concent) reset_concentration(TRUE);

            /* Hex */
            if (hex_spelling_any()) stop_hex_spell_all();

            p_ptr->counter = FALSE;
            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->paralyzed)
        {
            if (repose_of_the_dead)
            {
                msg_print("You awake refreshed!");
                restore_level();
                lp_player(1000);
                set_poisoned(0, TRUE);
                set_blind(0, TRUE);
                set_confused(0, TRUE);
                set_image(0, TRUE);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
                do_res_stat(A_STR);
                do_res_stat(A_CON);
                do_res_stat(A_DEX);
                do_res_stat(A_WIS);
                do_res_stat(A_INT);
                do_res_stat(A_CHR);
                set_shero(0,TRUE);
                /* Is this too much?
                hp_player(5000);
                if (p_ptr->csp < p_ptr->msp)
                {
                    p_ptr->csp = p_ptr->msp;
                    p_ptr->csp_frac = 0;

                    p_ptr->redraw |= (PR_MANA);
                    p_ptr->window |= (PW_PLAYER);
                    p_ptr->window |= (PW_SPELL);
                }
                */
                repose_of_the_dead = FALSE;
            }
            else
                msg_print("<color:B>You can move again.</color>");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->paralyzed = v;

    p_ptr->redraw |= (PR_EFFECTS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->image", notice observable changes
 *
 * Note that we must redraw the map when hallucination changes.
 */
bool set_image(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;
    
    if (mut_present(MUT_WEIRD_MIND))
    {
         v = 0;
         /*do_dec = TRUE;*/
    }

    /* Open */
    if (v)
    {
        set_tsuyoshi(0, TRUE);
        if ((p_ptr->image > v) && (!do_dec)) return FALSE;
        if (!p_ptr->image)
        {
            msg_print("Oh, wow! Everything looks so cosmic now!");

            /* Sniper */
            if (p_ptr->concent) reset_concentration(TRUE);

            p_ptr->counter = FALSE;
            notice = TRUE;

            /* Hack - image turn */
            image_turn = game_turn;
        }
        else if (game_turn > image_turn + 25)
        {
            image_turn = game_turn;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->image)
        {
            msg_print("You can see clearly again.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->image = v;

    /* Redraw status bar */
    p_ptr->redraw |= PR_EFFECTS;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Update the health bar */
    p_ptr->redraw |= PR_HEALTH_BARS;

    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_spurt(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_spurt)
        {
            if (p_ptr->tim_spurt > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel time slow down.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_spurt)
        {
            msg_print("You feel time speed up.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_spurt = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_blood_shield(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_blood_shield)
        {
            if (p_ptr->tim_blood_shield > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You are shielded in blood!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_blood_shield)
        {
            msg_print("Your blood shield vanishes.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_blood_shield = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_blood_revenge(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_blood_revenge)
        {
            if (p_ptr->tim_blood_revenge > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("Its time for bloody revenge!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_blood_revenge)
        {
            msg_print("The time for bloody revenge has passed.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_blood_revenge = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_blood_seek(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_blood_seek)
        {
            if (p_ptr->tim_blood_seek > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("Your weapon thirsts for life!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_blood_seek)
        {
            msg_print("Your weapon no longer thirsts for life.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_blood_seek = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_blood_sight(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_blood_sight)
        {
            if (p_ptr->tim_blood_sight > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You sense life!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_blood_sight)
        {
            msg_print("You no longer sense life.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_blood_sight = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_blood_feast(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_blood_feast)
        {
            if (p_ptr->tim_blood_feast > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You begin to feast on blood!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_blood_feast)
        {
            msg_print("You no longer feast on blood.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_blood_feast = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_superstealth(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_superstealth)
        {
            if (p_ptr->tim_superstealth > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You can hide in shadows!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_superstealth)
        {
            msg_print("You can no longer hide in shadows.");
            if (!player_is_ninja)
                set_superstealth(FALSE);
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_superstealth = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS | PU_TORCH); /* Note: Forcing PU_TORCH is the key!!! */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);
    p_ptr->update |= (PU_VIEW | PU_LITE);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_no_spells(int v, bool do_dec)
{
    bool notice = FALSE;
    /* Don't recalc v! 
       This is not your typical timer, and counts down after every
       player action.
    */

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (!p_ptr->tim_no_spells)
        {
            msg_print("You feel your magic blocked.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_no_spells)
        {
            msg_print("You feel your magic return.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_no_spells = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    p_ptr->redraw |= (PR_STATUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_no_device(int v, bool do_dec)
{
    bool notice = FALSE;
    /* Don't recalc v! 
       This is not your typical timer, and counts down after every
       player action.
    */

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (!p_ptr->tim_no_device)
        {
        /*  Hack: No message. This always comes with tim_no_spells as an added evil effect */ 
        /*    msg_print("You feel surrounded by powerful antimagic."); */
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_no_device)
        {
        /*  Hack: No message. This always comes with tim_no_spells as an added evil effect */ 
        /*    msg_print("You feel the antimagic forces leave you."); */
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_no_device = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    p_ptr->redraw |= (PR_STATUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_force(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_force)
        {
            if (p_ptr->tim_force > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("Your weapon seems very powerful.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_force)
        {
            msg_print("Your weapon seems normal once again.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_force = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_building_up(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_building_up)
        {
            if (p_ptr->tim_building_up > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You become gigantic!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_building_up)
        {
            msg_print("Your body reverts to normal size.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_building_up = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    p_ptr->update |= (PU_HP);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_vicious_strike(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_vicious_strike)
        {
            if (p_ptr->tim_vicious_strike > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel greatly exposed by your last attack.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_vicious_strike)
        {
            msg_print("You no longer feel greatly exposed.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_vicious_strike = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_enlarge_weapon(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_enlarge_weapon)
        {
            if (p_ptr->tim_enlarge_weapon > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel your weapon is much bigger.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_enlarge_weapon)
        {
            msg_print("Your weapon returns to normal.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_enlarge_weapon = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_field(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_field)
        {
            if (p_ptr->tim_field > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("An invisible force field surrounds your weapon!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_field)
        {
            msg_print("Your weapon is no longer surrounded by a force field.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_field = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_spell_reaction(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_spell_reaction)
        {
            if (p_ptr->tim_spell_reaction > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel ready for magical attacks.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_spell_reaction)
        {
            msg_print("You are no longer ready for magical attacks.");
            notice = TRUE;
        }
    }

    p_ptr->tim_spell_reaction = v;
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_resist_curses(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_resist_curses)
        {
            if (p_ptr->tim_resist_curses > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel resistant to curses.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_resist_curses)
        {
            msg_print("You are no longer resistant to curses.");
            notice = TRUE;
        }
    }

    p_ptr->tim_resist_curses = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_armor_of_fury(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_armor_of_fury)
        {
            if (p_ptr->tim_armor_of_fury > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel cloaked in rage.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_armor_of_fury)
        {
            msg_print("You are no longer cloaked in rage.");
            notice = TRUE;
        }
    }

    p_ptr->tim_armor_of_fury = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_spell_turning(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_spell_turning)
        {
            if (p_ptr->tim_spell_turning > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You begin to turn magical attacks.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_spell_turning)
        {
            msg_print("You are no longer turn magical attacks.");
            notice = TRUE;
        }
    }

    p_ptr->tim_spell_turning = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}


bool set_tim_blood_rite(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_blood_rite)
        {
            if (p_ptr->tim_blood_rite > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You invoke the ancient blood rite.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_blood_rite)
        {
            msg_print("The blood rite has ended.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_blood_rite = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->redraw |= (PR_STATUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->fast", notice observable changes
 */
bool set_fast(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->fast && !do_dec)
        {
            if (p_ptr->fast > v) return FALSE;
        }
        else if (!IS_FAST() && !IS_LIGHT_SPEED())
        {
            msg_print("You feel yourself moving much faster!");

            notice = TRUE;
            virtue_add(VIRTUE_PATIENCE, -1);
            virtue_add(VIRTUE_DILIGENCE, 1);
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->fast && !IS_LIGHT_SPEED() && !music_singing(MUSIC_SPEED) && !music_singing(MUSIC_SHERO))
        {
            msg_print("You feel yourself slow down.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->fast = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->lightspeed", notice observable changes
 */
bool set_lightspeed(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if (p_ptr->wild_mode) v = 0;

    /* Open */
    if (v)
    {
        if (p_ptr->lightspeed && !do_dec)
        {
            if (p_ptr->lightspeed > v) return FALSE;
        }
        else if (!p_ptr->lightspeed)
        {
            msg_print("You feel yourself moving extremely fast!");

            notice = TRUE;
            virtue_add(VIRTUE_PATIENCE, -1);
            virtue_add(VIRTUE_DILIGENCE, 1);
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->lightspeed)
        {
            msg_print("You feel yourself slow down.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->lightspeed = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->slow", notice observable changes
 */
bool set_slow(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if (p_ptr->no_slow) v = 0;

    /* Open */
    if (v)
    {
        if (p_ptr->slow && !do_dec)
        {
            if (p_ptr->slow > v) return FALSE;
        }
        else if (!p_ptr->slow)
        {
            msg_print("You feel yourself moving slower!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->slow)
        {
            msg_print("You feel yourself speed up.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->slow = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Effect of unwellness
 */
byte unwell_effect(int uw)
{
   if ((!uw) || (uw > UNWELL_EFFECTIVE_MAX)) return 0;
   if (uw > 30) return 4;
   return ((uw + 9) / 10);
}

/*
 * Set "p_ptr->unwell", notice observable changes
 */
bool set_unwell(int v, bool do_dec)
{
    bool notice = FALSE;
    byte old_eff = 0, new_eff = 0;

    /* Nonliving races don't get unwell */
    if (get_race()->flags & RACE_IS_NONLIVING)
    {
        p_ptr->unwell = 0; /* paranoia */
        return FALSE;
    }

    /* Hack -- Force good values */
    v = (v > 100) ? 100 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if ((p_ptr->unwell > 40) && (v >= p_ptr->unwell)) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->unwell && !do_dec)
        {
            if (p_ptr->unwell > v) return FALSE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->unwell)
        {
            msg_print("You no longer feel unwell.");

            notice = TRUE;
        }
    }
    old_eff = unwell_effect(p_ptr->unwell);
    new_eff = unwell_effect(v);

    /* Notice changes in unwellness level */
    if ((!notice) && (old_eff != new_eff)) notice = TRUE;
    if ((new_eff) && (!old_eff))
    {
        msg_print("You suddenly feel unwell!");
    }

    /* Use the value */
    p_ptr->unwell = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if ((disturb_state) && ((!new_eff) || (!old_eff)) && (!mut_present(MUT_HUMAN_CON))) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    p_ptr->redraw |= (PR_STATUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->no_air", notice observable changes
 */
bool set_no_air(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > NO_AIR_MAX) ? NO_AIR_MAX : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if ((p_ptr->no_air) && (v >= p_ptr->no_air)) return FALSE;

    if (py_on_surface()) v = 0; /* too much air */

    /* Open */
    if (v)
    {
        if (p_ptr->no_air && !do_dec)
        {
            if (p_ptr->no_air > v) return FALSE;
        }
        else if (!p_ptr->no_air)
        {
            msg_print("Suddenly, the cave feels devoid of air!");
            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->no_air)
        {
            msg_print("Air rushes back into the dungeon!");
            no_air_monster = 0;

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->no_air = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    p_ptr->redraw |= (PR_STATUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->shield", notice observable changes
 */
bool set_shield(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->shield && !do_dec)
        {
            if (p_ptr->shield > v) return FALSE;
        }
        else if (!p_ptr->shield)
        {
            msg_print("Your skin turns to stone.");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->shield)
        {
            msg_print("Your skin returns to normal.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->shield = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}



/*
 * Set "p_ptr->tsubureru", notice observable changes
 */
bool set_tsubureru(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tsubureru && !do_dec)
        {
            if (p_ptr->tsubureru > v) return FALSE;
        }
        else if (!p_ptr->tsubureru)
        {
            msg_print("Your body expands horizontally.");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tsubureru)
        {
            msg_print("Your body returns to normal.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tsubureru = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}



/*
 * Set "p_ptr->magicdef", notice observable changes
 */
bool set_magicdef(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->magicdef && !do_dec)
        {
            if (p_ptr->magicdef > v) return FALSE;
        }
        else if (!p_ptr->magicdef)
        {
            msg_print("You feel more resistant to magic.");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->magicdef)
        {
            msg_print("You feel less resistant to magic.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->magicdef = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}



/*
 * Set "p_ptr->blessed", notice observable changes
 */
bool set_blessed(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->blessed && !do_dec)
        {
            if (p_ptr->blessed > v) return FALSE;
        }
        else if (!IS_BLESSED())
        {
            msg_print("You feel righteous!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->blessed && !music_singing(MUSIC_BLESS))
        {
            msg_print("The prayer has expired.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->blessed = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->hero", notice observable changes
 */
bool set_hero(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->hero && !do_dec)
        {
            if (p_ptr->hero > v) return FALSE;
        }
        else if (!IS_HERO())
        {
            msg_print("You feel like a hero!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->hero && !music_singing(MUSIC_HERO) && !music_singing(MUSIC_SHERO))
        {
            msg_print("The heroism wears off.");

            notice = TRUE;
        }
    }

    /* Alchemist bookkeeping */
    if ((p_ptr->pclass == CLASS_ALCHEMIST) && (v < p_ptr->hero)) alchemist_set_hero(&notice, v, TRUE);

    /* Use the value */
    p_ptr->hero = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Recalculate hitpoints */
    p_ptr->update |= (PU_HP);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->shero", notice observable changes
 */
bool set_shero(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if ((p_ptr->pclass == CLASS_BERSERKER) || (beorning_is_(BEORNING_FORM_BEAR))) v = 1;
    /* Open */
    if (v)
    {
        if (p_ptr->shero && !do_dec)
        {
            if (p_ptr->shero > v) return FALSE;
        }
        else if (!p_ptr->shero)
        {
            msg_print("You feel like a killing machine!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->shero)
        {
            msg_print("You feel less Berserk.");

            notice = TRUE;
        }
    }

    /* Alchemist bookkeeping */
    if ((p_ptr->pclass == CLASS_ALCHEMIST) && (v < p_ptr->shero)) alchemist_set_hero(&notice, v, FALSE);

    /* Use the value */
    p_ptr->shero = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Recalculate hitpoints */
    p_ptr->update |= (PU_HP);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->protevil", notice observable changes
 */
bool set_protevil(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->protevil && !do_dec)
        {
            if (p_ptr->protevil > v) return FALSE;
        }
        else if (!p_ptr->protevil)
        {
            msg_print("You feel safe from evil!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->protevil)
        {
            msg_print("You no longer feel safe from evil.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->protevil = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->wraith_form", notice observable changes
 */
bool set_wraith_form(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->wraith_form && !do_dec)
        {
            if (p_ptr->wraith_form > v) return FALSE;
        }
        else if (!p_ptr->wraith_form)
        {
            msg_print("You leave the physical world and turn into a wraith-being!");

            notice = TRUE;

            virtue_add(VIRTUE_UNLIFE, 3);
            virtue_add(VIRTUE_HONOUR, -2);
            virtue_add(VIRTUE_SACRIFICE, -2);
            virtue_add(VIRTUE_VALOUR, -5);

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);

            /* Update monsters and HP*/
            p_ptr->update |= (PU_MONSTERS | PU_HP);

            /* Window stuff */
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->wraith_form)
        {
            msg_print("You feel opaque.");

            notice = TRUE;

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);

            /* Update monsters */
            p_ptr->update |= (PU_MONSTERS | PU_HP);

            /* Window stuff */
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        }
    }

    /* Use the value */
    p_ptr->wraith_form = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);

}


/*
 * Set "p_ptr->invuln", notice observable changes
 */
bool set_invuln(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->invuln && !do_dec)
        {
            if (p_ptr->invuln > v) return FALSE;
        }
        else if ((!IS_INVULN()) && (!p_ptr->ignore_invuln))
        {
            msg_print("Invulnerability!");

            notice = TRUE;

            virtue_add(VIRTUE_UNLIFE, -2);
            virtue_add(VIRTUE_HONOUR, -2);
            virtue_add(VIRTUE_SACRIFICE, -3);
            virtue_add(VIRTUE_VALOUR, -5);

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);

            /* Update monsters */
            p_ptr->update |= (PU_MONSTERS);

            /* Window stuff */
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->invuln && !music_singing(MUSIC_INVULN))
        {
            msg_print("The invulnerability wears off.");

            notice = TRUE;

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);

            /* Update monsters */
            p_ptr->update |= (PU_MONSTERS);

            /* Window stuff */
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

            p_ptr->energy_need += PY_ENERGY_NEED();
        }
    }

    /* Use the value */
    p_ptr->invuln = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_esp", notice observable changes
 */
bool set_tim_esp(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_esp && !do_dec)
        {
            if (p_ptr->tim_esp > v) return FALSE;
        }
        else if (!IS_TIM_ESP())
        {
            msg_print("You feel your consciousness expand!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_esp && !music_singing(MUSIC_MIND))
        {
            msg_print("Your consciousness contracts again.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_esp = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_esp_magical(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_esp_magical && !do_dec)
        {
            if (p_ptr->tim_esp_magical > v) return FALSE;
        }
        else if (!p_ptr->tim_esp_magical)
        {
            msg_print("You feel conscious of magical foes.");
            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_esp_magical)
        {
            msg_print("You are no longer conscious of magical foes.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_esp_magical = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_invis", notice observable changes
 */
bool set_tim_invis(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_invis && !do_dec)
        {
            if (p_ptr->tim_invis > v) return FALSE;
        }
        else if (!p_ptr->tim_invis)
        {
            msg_print("Your eyes feel very sensitive!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_invis)
        {
            msg_print("Your eyes feel less sensitive.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_invis = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_infra", notice observable changes
 */
bool set_tim_infra(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_infra && !do_dec)
        {
            if (p_ptr->tim_infra > v) return FALSE;
        }
        else if (!IS_TIM_INFRA())
        {
            msg_print("Your eyes begin to tingle!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_infra && !wild_has_power(WILD_INFRAVISION))
        {
            msg_print("Your eyes stop tingling.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_infra = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->tim_poet", notice observable changes
 */
bool set_tim_poet(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_poet && !do_dec)
        {
            if (p_ptr->tim_poet > v) return FALSE;
        }
        else if (!p_ptr->tim_poet)
        {
            msg_print("The wisdom of Kvasir flows through you!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_poet)
        {
            msg_print("You stop feeling wise and eloquent.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_poet = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->tim_understanding", notice observable changes
 */
bool set_tim_understanding(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_understanding && !do_dec)
        {
            if (p_ptr->tim_understanding > v) return FALSE;
        }
        else if (!p_ptr->tim_understanding)
        {
            msg_print("You feel like a loremaster!");
            identify_pack();

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_understanding)
        {
            msg_print("You stop feeling like a loremaster.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_understanding = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}



/*
 * Set "p_ptr->tim_regen", notice observable changes
 */
bool set_tim_regen(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_regen && !do_dec)
        {
            if (p_ptr->tim_regen > v) return FALSE;
        }
        else if (!p_ptr->tim_regen)
        {
            msg_print("You feel yourself regenerating quickly!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_regen)
        {
            msg_print("You feel yourself regenerating slowly.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_regen = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_stealth", notice observable changes
 */
bool set_tim_stealth(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_stealth && !do_dec)
        {
            if (p_ptr->tim_stealth > v) return FALSE;
        }
        else if (!IS_TIM_STEALTH())
        {
            msg_print("You begin to walk silently!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_stealth && !music_singing(MUSIC_STEALTH))
        {
            msg_print("You no longer walk silently.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_stealth = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_sanctuary(bool set)
{
    bool notice = FALSE;

    if (p_ptr->is_dead) return FALSE;

    if (set)
    {
        if (!(p_ptr->special_defense & DEFENSE_SANCTUARY))
        {
            msg_print("You claim Sanctuary!  Nothing can hurt you now!!");
            notice = TRUE;
            p_ptr->special_defense |= DEFENSE_SANCTUARY;
        }
    }
    else
    {
        if (p_ptr->special_defense & DEFENSE_SANCTUARY)
        {
            msg_print("You no longer feel safe.");
            notice = TRUE;
            p_ptr->special_defense &= ~(DEFENSE_SANCTUARY);
        }
    }

    if (!notice) return FALSE;
    p_ptr->redraw |= (PR_STATUS);
    if (disturb_state) disturb(0, 0);
    return TRUE;
}

bool set_superstealth(bool set)
{
    bool notice = FALSE;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (set)
    {
        if (!(p_ptr->special_defense & NINJA_S_STEALTH))
        {
            if (cave[py][px].info & CAVE_MNLT)
            {
                if (disturb_minor)
                    msg_print("<color:D>You are mantled in weak shadow from ordinary eyes.</color>");
                p_ptr->monlite = p_ptr->old_monlite = TRUE;
            }
            else
            {
                if (disturb_minor)
                    msg_print("<color:D>You are mantled in shadow from ordinary eyes!</color>");
                p_ptr->monlite = p_ptr->old_monlite = FALSE;
            }

            notice = TRUE;

            /* Use the value */
            p_ptr->special_defense |= NINJA_S_STEALTH;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->special_defense & NINJA_S_STEALTH)
        {
            if ((disturb_minor) && (!p_ptr->exit_bldg))
                msg_print("<color:y>You are exposed to common sight once more.</color>");

            notice = TRUE;

            /* Use the value */
            p_ptr->special_defense &= ~(NINJA_S_STEALTH);
        }
    }

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_levitation", notice observable changes
 */
bool set_tim_levitation(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_levitation && !do_dec)
        {
            if (p_ptr->tim_levitation > v) return FALSE;
        }
        else if (!p_ptr->tim_levitation)
        {
            msg_print("You begin to fly!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_levitation)
        {
            msg_print("You stop flying.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_levitation = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_sh_touki", notice observable changes
 */
bool set_tim_sh_touki(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sh_touki && !do_dec)
        {
            if (p_ptr->tim_sh_touki > v) return FALSE;
        }
        else if (!p_ptr->tim_sh_touki)
        {
            msg_print("You have enveloped by the aura of the Force!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_sh_touki)
        {
            msg_print("Aura of the Force disappeared.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_sh_touki = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_sh_fire", notice observable changes
 */
bool set_tim_sh_fire(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sh_fire && !do_dec)
        {
            if (p_ptr->tim_sh_fire > v) return FALSE;
        }
        else if (!p_ptr->tim_sh_fire)
        {
            msg_print("You have enveloped by fiery aura!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_sh_fire)
        {
            msg_print("Fiery aura disappeared.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_sh_fire = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_sh_holy", notice observable changes
 */
bool set_tim_sh_holy(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sh_holy && !do_dec)
        {
            if (p_ptr->tim_sh_holy > v) return FALSE;
        }
        else if (!p_ptr->tim_sh_holy)
        {
            msg_print("You have enveloped by holy aura!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_sh_holy)
        {
            msg_print("Holy aura disappeared.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_sh_holy = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}



/*
 * Set "p_ptr->tim_eyeeye", notice observable changes
 */
bool set_tim_eyeeye(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_eyeeye && !do_dec)
        {
            if (p_ptr->tim_eyeeye > v) return FALSE;
        }
        else if (!p_ptr->tim_eyeeye)
        {
            if (p_ptr->pclass == CLASS_BLOOD_KNIGHT)
                msg_print("You feel like bloody revenge!");
            else 
                msg_print("You feel like a keeper of commandments!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_eyeeye)
        {
            if (p_ptr->pclass == CLASS_BLOOD_KNIGHT)
                msg_print("You no longer feel like bloody revenge.");
            else 
                msg_print("You no longer feel like a keeper.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_eyeeye = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}



/*
 * Set "p_ptr->resist_magic", notice observable changes
 */
bool set_resist_magic(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->resist_magic && !do_dec)
        {
            if (p_ptr->resist_magic > v) return FALSE;
        }
        else if (!p_ptr->resist_magic)
        {
            msg_print("You have been protected from magic!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->resist_magic)
        {
msg_print("You are no longer protected from magic.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->resist_magic = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_reflect", notice observable changes
 */
bool set_tim_reflect(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_reflect && !do_dec)
        {
            if (p_ptr->tim_reflect > v) return FALSE;
        }
        else if (!p_ptr->tim_reflect)
        {
            msg_print("Your body becames smooth.");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_reflect)
        {
            msg_print("Your body is no longer smooth.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_reflect = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->multishadow", notice observable changes
 */
bool set_multishadow(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->multishadow && !do_dec)
        {
            if (p_ptr->multishadow > v) return FALSE;
        }
        else if (!p_ptr->multishadow)
        {
            msg_print("Your Shadow enveloped you.");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->multishadow)
        {
            msg_print("Your Shadow disappears.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->multishadow = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->dustrobe", notice observable changes
 */
bool set_dustrobe(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->dustrobe && !do_dec)
        {
            if (p_ptr->dustrobe > v) return FALSE;
        }
        else if (!p_ptr->dustrobe)
        {
            msg_print("You were enveloped by mirror shards.");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->dustrobe)
        {
            msg_print("The mirror shards disappear.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->dustrobe = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_regen", notice observable changes
 */
bool set_kabenuke(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->kabenuke && !do_dec)
        {
            if (p_ptr->kabenuke > v) return FALSE;
        }
        else if (!p_ptr->kabenuke)
        {
            msg_print("You became ethereal form.");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->kabenuke)
        {
            msg_print("You are no longer in an ethereal form.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->kabenuke = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


bool set_tsuyoshi(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tsuyoshi && !do_dec)
        {
            if (p_ptr->tsuyoshi > v) return FALSE;
        }
        else if (!p_ptr->tsuyoshi)
        {
            msg_print("Brother OKURE!");

            notice = TRUE;
            virtue_add(VIRTUE_VITALITY, 2);
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tsuyoshi)
        {
            msg_print("Your body had quickly shriveled.");

            (void)dec_stat(A_CON, 20, TRUE);
            (void)dec_stat(A_STR, 20, TRUE);

            notice = TRUE;
            virtue_add(VIRTUE_VITALITY, -3);
        }
    }

    /* Use the value */
    p_ptr->tsuyoshi = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Recalculate hitpoints */
    p_ptr->update |= (PU_HP);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set a temporary elemental brand. Clear all other brands. Print status 
 * messages. -LM-
 */
bool set_ele_attack(u32b attack_type, int v)
{
    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Clear all elemental attacks (only one is allowed at a time). */
    if ((p_ptr->special_attack & (ATTACK_ACID)) && (attack_type != ATTACK_ACID))
    {
        p_ptr->special_attack &= ~(ATTACK_ACID);
        msg_print("Your temporary acidic brand fades away.");
    }
    if ((p_ptr->special_attack & (ATTACK_ELEC)) && (attack_type != ATTACK_ELEC))
    {
        p_ptr->special_attack &= ~(ATTACK_ELEC);
        msg_print("Your temporary electrical brand fades away.");
    }
    if ((p_ptr->special_attack & (ATTACK_FIRE)) && (attack_type != ATTACK_FIRE))
    {
        p_ptr->special_attack &= ~(ATTACK_FIRE);
        msg_print("Your temporary fiery brand fades away.");
    }
    if ((p_ptr->special_attack & (ATTACK_COLD)) && (attack_type != ATTACK_COLD))
    {
        p_ptr->special_attack &= ~(ATTACK_COLD);
        msg_print("Your temporary frost brand fades away.");
    }
    if ((p_ptr->special_attack & (ATTACK_POIS)) && (attack_type != ATTACK_POIS))
    {
        p_ptr->special_attack &= ~(ATTACK_POIS);
        msg_print("Your temporary poison brand fades away.");
    }

    if ((v) && (attack_type))
    {
        /* Set attack type. */
        p_ptr->special_attack |= (attack_type);

        /* Set duration. */
        p_ptr->ele_attack = v;

        /* Message. */
        msg_format("For a while, the blows you deal will %s",
                 ((attack_type == ATTACK_ACID) ? "melt with acid!" :
                  ((attack_type == ATTACK_ELEC) ? "shock your foes!" :
                   ((attack_type == ATTACK_FIRE) ? "burn with fire!" : 
                ((attack_type == ATTACK_COLD) ? "chill to the bone!" : 
                 ((attack_type == ATTACK_POIS) ? "poison your enemies!" : 
                    "do nothing special."))))));
    }

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    return (TRUE);
}


/*
 * Set a temporary elemental brand. Clear all other brands. Print status 
 * messages. -LM-
 */
bool set_ele_immune(u32b immune_type, int v)
{
    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Clear all elemental attacks (only one is allowed at a time). */
    if ((p_ptr->special_defense & (DEFENSE_ACID)) && (immune_type != DEFENSE_ACID))
    {
        p_ptr->special_defense &= ~(DEFENSE_ACID);
        msg_print("You are no longer immune to acid.");
    }
    if ((p_ptr->special_defense & (DEFENSE_ELEC)) && (immune_type != DEFENSE_ELEC))
    {
        p_ptr->special_defense &= ~(DEFENSE_ELEC);
        msg_print("You are no longer immune to electricity.");
    }
    if ((p_ptr->special_defense & (DEFENSE_FIRE)) && (immune_type != DEFENSE_FIRE))
    {
        p_ptr->special_defense &= ~(DEFENSE_FIRE);
        msg_print("You are no longer immune to fire.");
    }
    if ((p_ptr->special_defense & (DEFENSE_COLD)) && (immune_type != DEFENSE_COLD))
    {
        p_ptr->special_defense &= ~(DEFENSE_COLD);
        msg_print("You are no longer immune to cold.");
    }
    if ((p_ptr->special_defense & (DEFENSE_POIS)) && (immune_type != DEFENSE_POIS))
    {
        p_ptr->special_defense &= ~(DEFENSE_POIS);
        msg_print("You are no longer immune to poison.");
    }

    if ((v) && (immune_type))
    {
        /* Set attack type. */
        p_ptr->special_defense |= (immune_type);

        /* Set duration. */
        p_ptr->ele_immune = v;

        /* Message. */
        msg_format("For a while, You are immune to %s",
                 ((immune_type == DEFENSE_ACID) ? "acid!" :
                  ((immune_type == DEFENSE_ELEC) ? "electricity!" :
                   ((immune_type == DEFENSE_FIRE) ? "fire!" : 
                ((immune_type == DEFENSE_COLD) ? "cold!" : 
                 ((immune_type == DEFENSE_POIS) ? "poison!" : 
                    "do nothing special."))))));
    }

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    return (TRUE);
}


/*
 * Set "p_ptr->oppose_acid", notice observable changes
 */
bool set_oppose_acid(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->oppose_acid && !do_dec)
        {
            if (p_ptr->oppose_acid > v) return FALSE;
        }
        else if (!IS_OPPOSE_ACID())
        {
            msg_print("You feel resistant to acid!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->oppose_acid && !music_singing(MUSIC_RESIST) && !(p_ptr->special_defense & KATA_MUSOU))
        {
            msg_print("You feel less resistant to acid.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_acid = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_elec", notice observable changes
 */
bool set_oppose_elec(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->oppose_elec && !do_dec)
        {
            if (p_ptr->oppose_elec > v) return FALSE;
        }
        else if (!IS_OPPOSE_ELEC())
        {
            msg_print("You feel resistant to electricity!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->oppose_elec && !music_singing(MUSIC_RESIST) && !(p_ptr->special_defense & KATA_MUSOU))
        {
            msg_print("You feel less resistant to electricity.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_elec = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_fire", notice observable changes
 */
bool set_oppose_fire(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if ((prace_is_(RACE_BALROG) && (p_ptr->lev > 44)) || (p_ptr->mimic_form == MIMIC_DEMON)) v = 1;
    /* Open */
    if (v)
    {
        if (p_ptr->oppose_fire && !do_dec)
        {
            if (p_ptr->oppose_fire > v) return FALSE;
        }
        else if (!IS_OPPOSE_FIRE())
        {
            msg_print("You feel resistant to fire!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->oppose_fire && !music_singing(MUSIC_RESIST) && !(p_ptr->special_defense & KATA_MUSOU))
        {
            msg_print("You feel less resistant to fire.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_fire = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_cold", notice observable changes
 */
bool set_oppose_cold(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->oppose_cold && !do_dec)
        {
            if (p_ptr->oppose_cold > v) return FALSE;
        }
        else if (!IS_OPPOSE_COLD())
        {
            msg_print("You feel resistant to cold!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->oppose_cold && !music_singing(MUSIC_RESIST) && !(p_ptr->special_defense & KATA_MUSOU))
        {
            msg_print("You feel less resistant to cold.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_cold = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_pois", notice observable changes
 */
bool set_oppose_pois(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if ((p_ptr->pclass == CLASS_NINJA) && (p_ptr->lev > 44)) v = 1;
    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->oppose_pois && !do_dec)
        {
            if (p_ptr->oppose_pois > v) return FALSE;
        }
        else if (!IS_OPPOSE_POIS())
        {
            msg_print("You feel resistant to poison!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->oppose_pois && !music_singing(MUSIC_RESIST) && !(p_ptr->special_defense & KATA_MUSOU))
        {
            msg_print("You feel less resistant to poison.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_pois = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/* Set temporary resistance to the base elements */
bool set_oppose_base(int v, bool do_dec)
{
    bool notice = FALSE;
    if (set_oppose_acid(v, do_dec)) notice = TRUE;
    if (set_oppose_elec(v, do_dec)) notice = TRUE;
    if (set_oppose_fire(v, do_dec)) notice = TRUE;
    if (set_oppose_cold(v, do_dec)) notice = TRUE;
    if (set_oppose_pois(v, do_dec)) notice = TRUE;
    return notice;
}

/*
 * Set "p_ptr->spin", notice observable changes
 */
bool set_spin(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->spin && !do_dec)
        {
            if (p_ptr->spin > v) return FALSE;
        }
        else if (!IS_SPINNING())
        {
            msg_print("You start spinning stories!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->spin)
        {
            msg_print("You stop putting your own spin on stories.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->spin = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Set "p_ptr->stun", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 * Note that stun effects no longer require PU_BONUS. Instead
 * the effects are computed on the fly in various areas of the system.
 */
stun_info_t stun_info(int stun)
{
    stun_info_t result = {0};
    if (stun >= STUN_KNOCKED_OUT)
    {
        result.level = STUN_KNOCKED_OUT;
        result.name = "Knocked Out";  /* <== PR_EFFECTS */
        result.msg = "knocked out";   /* <== You have been %s */
        result.attr = TERM_VIOLET;
    }
    else if (stun >= STUN_MASSIVE)
    {
        result.level = STUN_MASSIVE;
        result.name = "Massive Stun";
        result.msg = "massively stunned";
        result.attr = TERM_RED;
    }
    else if (stun >= STUN_HEAVY)
    {
        result.level = STUN_HEAVY;
        result.name = "Heavy Stun";
        result.msg = "heavily stunned";
        result.attr = TERM_L_RED;
    }
    else if (stun >= STUN_MODERATE)
    {
        result.level = STUN_MODERATE;
        result.name = "Stun";
        result.msg = "stunned";
        result.attr = TERM_ORANGE;
    }
    else if (stun >= STUN_LIGHT)
    {
        result.level = STUN_LIGHT;
        result.name = "Light Stun";
        result.msg = "lightly stunned";
        result.attr = TERM_YELLOW;
    }
    else if (stun >= STUN_DAZE)
    {
        result.level = STUN_DAZE;
        result.name = "Dazed";
        result.msg = "dazed";
        result.attr = TERM_L_UMBER;
    }
    else
    {
        assert(result.level == STUN_NONE);
    }
    return result;
}

bool set_stun(int v, bool do_dec)
{
    stun_info_t old_stun = stun_info(p_ptr->stun);
    stun_info_t new_stun;
    slot_t      slot;
    bool        notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if (p_ptr->no_stun) v = 0;
    slot = equip_find_first(object_is_body_armour);
    if (slot && equip_obj(slot)->rune == RUNE_WATER) v = 0;
    if (psion_mental_fortress()) v = 0;

    new_stun = stun_info(v);

    /* Increase stun level */
    if (new_stun.level > old_stun.level)
    {
        msg_format("You have been <color:%c>%s</color>.", attr_to_attr_char(new_stun.attr), new_stun.msg);
        if (randint1(1000) < v || one_in_(16))
        {
            msg_print("A vicious blow hits your head.");

            if (one_in_(3))
            {
                if (!p_ptr->sustain_int) (void)do_dec_stat(A_INT);
                if (!p_ptr->sustain_wis) (void)do_dec_stat(A_WIS);
            }
            else if (one_in_(2))
            {
                if (!p_ptr->sustain_int) (void)do_dec_stat(A_INT);
            }
            else
            {
                if (!p_ptr->sustain_wis) (void)do_dec_stat(A_WIS);
            }
        }
        if (p_ptr->special_defense & KATA_MASK)
        {
            lose_kata();
        }
        if (p_ptr->concent) reset_concentration(TRUE);
        if (hex_spelling_any()) stop_hex_spell_all();
        notice = TRUE;
    }

    /* Decrease stun level */
    else if (new_stun.level < old_stun.level)
    {
        if (old_stun.level == STUN_KNOCKED_OUT && new_stun.level > STUN_NONE)
            msg_format("You are no longer <color:%c>%s</color>.", attr_to_attr_char(old_stun.attr), old_stun.msg); 

        if  (new_stun.level == STUN_NONE)
        {
            msg_format("You are no longer <color:%c>%s</color>.", attr_to_attr_char(old_stun.attr), old_stun.msg);
            if (disturb_state) disturb(0, 0);
        }
        notice = TRUE;
    }

    p_ptr->stun = v;
    if (!notice) return (FALSE);

    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_EFFECTS;
    handle_stuff();
    return TRUE;
}


/*
 * Set "p_ptr->cut", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
cut_info_t cut_info(int cut)
{
    point_t tbl[7] = { {CUT_GRAZE, 10}, {CUT_LIGHT, 30}, {CUT_BAD, 70}, {CUT_NASTY, 160},
                       {CUT_SEVERE, 320}, {CUT_DEEP_GASH, 800}, {CUT_MORTAL_WOUND, 2000} };
    cut_info_t result = {0};
    int divisor = 100 * SPEED_TO_ENERGY(p_ptr->pspeed);
    if (cut)
    {
        if (p_ptr->wild_mode) result.dam = interpolate(cut, tbl, 7) / 10;
        else result.dam = (int)((s32b)(interpolate(cut, tbl, 7) * energy_use + divisor - 1) / divisor);
    }
    if (cut >= CUT_MORTAL_WOUND)
    {
        result.level = CUT_MORTAL_WOUND;
        result.desc = "Mortal Wound";
        result.attr = TERM_L_RED;
    }
    else if (cut >= CUT_DEEP_GASH)
    {
        result.level = CUT_DEEP_GASH;
        result.desc = "Deep Gash";
        result.attr = TERM_RED;
    }
    else if (cut >= CUT_SEVERE)
    {
        result.level = CUT_SEVERE;
        result.desc = "Severe Cut";
        result.attr = TERM_RED;
    }
    else if (cut >= CUT_NASTY)
    {
        result.level = CUT_NASTY;
        result.desc = "Nasty Cut";
        result.attr = TERM_ORANGE;
    }
    else if (cut >= CUT_BAD)
    {
        result.level = CUT_BAD;
        result.desc = "Bad Cut";
        result.attr = TERM_ORANGE;
    }
    else if (cut >= CUT_LIGHT)
    {
        result.level = CUT_LIGHT;
        result.desc = "Light Cut";
        result.attr = TERM_YELLOW;
    }
    else if (cut >= CUT_GRAZE)
    {
        result.level = CUT_GRAZE;
        result.desc = "Graze";
        result.attr = TERM_YELLOW;
    }
    else
    {
        assert(result.level == CUT_NONE);
    }
    return result;
}

bool set_cut(int v, bool do_dec)
{
    cut_info_t old_cut = {0}, new_cut = {0};
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    if (get_race()->flags & RACE_IS_NONLIVING)
        v = 0;

    if (p_ptr->no_cut)
        v = 0;

    old_cut = cut_info(p_ptr->cut);
    new_cut = cut_info(v);

    /* Increase cut */
    if (new_cut.level > old_cut.level)
    {
        msg_format("You have been given a <color:%c>%s</color>.", attr_to_attr_char(new_cut.attr), new_cut.desc);
        notice = TRUE;
    }
    /* Decrease cut */
    if (new_cut.level < old_cut.level)
    {
        if (new_cut.level == CUT_NONE)
            msg_print("You are no longer bleeding.");
        notice = TRUE;
    }

    /* Use the value */
    p_ptr->cut = v;

    /* No change */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Redraw the "cut" */
    p_ptr->redraw |= PR_EFFECTS;

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->food", notice observable changes
 *
 * The "p_ptr->food" variable can get as large as 20000, allowing the
 * addition of the most "filling" item, Elvish Waybread, which adds
 * 7500 food units, without overflowing the 32767 maximum limit.
 *
 * Perhaps we should disturb the player with various messages,
 * especially messages about hunger status changes. XXX XXX XXX
 *
 * Digestion of food is handled in "dungeon.c", in which, normally,
 * the player digests about 20 food units per 100 game turns, more
 * when "fast", more when "regenerating", less with "slow digestion",
 * but when the player is "gorged", he digests 100 food units per 10
 * game turns, or a full 1000 food units per 100 game turns.
 *
 * Note that the player's speed is reduced by 10 units while gorged,
 * so if the player eats a single food ration (5000 food units) when
 * full (15000 food units), he will be gorged for (5000/100)*10 = 500
 * game turns, or 500/(100/5) = 25 player turns (if nothing else is
 * affecting the player speed).
 */
bool set_food(int v)
{
    int old_aux, new_aux;
    int old_pct;
    int new_pct;

    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 20000) ? 20000 : (v < 0) ? 0 : v;

    /* CTK: I added a "food bar" to track hunger ... */
    if (easy_damage || p_ptr->wizard)
    {
        old_pct = p_ptr->food * 100 / PY_FOOD_FULL;
        new_pct = v * 100 / PY_FOOD_FULL;
    }
    else
    {
        old_pct = MIN(10, p_ptr->food * 10 / PY_FOOD_FULL);
        new_pct = MIN(10, v * 10 / PY_FOOD_FULL);
    }

    /* Fainting / Starving */
    if (p_ptr->food < PY_FOOD_FAINT)
    {
        old_aux = 0;
    }

    /* Weak */
    else if (p_ptr->food < PY_FOOD_WEAK)
    {
        old_aux = 1;
    }

    /* Hungry */
    else if (p_ptr->food < PY_FOOD_ALERT)
    {
        old_aux = 2;
    }

    /* Normal */
    else if (p_ptr->food < PY_FOOD_FULL)
    {
        old_aux = 3;
    }

    /* Full */
    else if (p_ptr->food < PY_FOOD_MAX)
    {
        old_aux = 4;
    }

    /* Gorged */
    else
    {
        old_aux = 5;
    }

    /* Fainting / Starving */
    if (v < PY_FOOD_FAINT)
    {
        new_aux = 0;
    }

    /* Weak */
    else if (v < PY_FOOD_WEAK)
    {
        new_aux = 1;
    }

    /* Hungry */
    else if (v < PY_FOOD_ALERT)
    {
        new_aux = 2;
    }

    /* Normal */
    else if (v < PY_FOOD_FULL)
    {
        new_aux = 3;
    }

    /* Full */
    else if (v < PY_FOOD_MAX)
    {
        new_aux = 4;
    }

    /* Gorged */
    else
    {
        new_aux = 5;
    }

    if (old_aux < 1 && new_aux > 0)
        virtue_add(VIRTUE_PATIENCE, 2);
    else if (old_aux < 3 && (old_aux != new_aux))
        virtue_add(VIRTUE_PATIENCE, 1);
    if (old_aux == 2)
        virtue_add(VIRTUE_TEMPERANCE, 1);
    if (old_aux == 0)
        virtue_add(VIRTUE_TEMPERANCE, -1);

    if (display_food_bar && new_pct != old_pct)
        notice = TRUE;

    /* Food increase */
    if (new_aux > old_aux)
    {
        /* Describe the state */
        switch (new_aux)
        {
            /* Weak */
            case 1:
            msg_print("You are still weak.");

            break;

            /* Hungry */
            case 2:
            msg_print("You are still hungry.");

            break;

            /* Normal */
            case 3:
            msg_print("You are no longer hungry.");

            break;

            /* Full */
            case 4:
            msg_print("You are full!");

            break;

            /* Bloated */
            case 5:
            msg_print("You have gorged yourself!");
            virtue_add(VIRTUE_HARMONY, -1);
            virtue_add(VIRTUE_PATIENCE, -1);
            virtue_add(VIRTUE_TEMPERANCE, -2);

            break;
        }

        /* Change */
        notice = TRUE;
    }

    if ((v > p_ptr->food) && p_ptr->fasting)
    {
        msg_print("You break your fast.");
        p_ptr->redraw |= PR_STATUS;
        p_ptr->fasting = FALSE;
    }

    /* Food decrease */
    else if (new_aux < old_aux)
    {
        /* Describe the state */
        switch (new_aux)
        {
            /* Fainting / Starving */
            case 0:
            msg_print("You are getting faint from hunger!");

            break;

            /* Weak */
            case 1:
            msg_print("You are getting weak from hunger!");

            break;

            /* Hungry */
            case 2:
            msg_print("You are getting hungry.");

            break;

            /* Normal */
            case 3:
            msg_print("You are no longer full.");

            break;

            /* Full */
            case 4:
            msg_print("You are no longer gorged.");

            break;
        }

        if (p_ptr->wild_mode && (new_aux < 2))
        {
            change_wild_mode();
        }

        /* Change */
        notice = TRUE;
    }

    /* Use the value */
    p_ptr->food = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    if (new_aux != old_aux)
    {
        /* Disturb */
        if (disturb_state) disturb(0, 0);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
    }

    /* Redraw hunger */
    p_ptr->redraw |= PR_EFFECTS;
    if (display_food_bar)
        p_ptr->redraw |= PR_HEALTH_BARS;

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

/*
 * Increases a stat by one randomized level             -RAK-
 *
 * Note that this function (used by stat potions) now restores
 * the stat BEFORE increasing it.
 */
bool inc_stat(int stat)
{
    int value, gain;

    /* Then augment the current/max stat */
    value = p_ptr->stat_cur[stat];

    /* Cannot go above 18/100 */
    if (value < p_ptr->stat_max_max[stat])
    {
        /* Gain one (sometimes two) points */
        if (value < 18)
        {
            gain = ((randint0(100) < 75) ? 1 : 2);
            value += gain;
        }
        else if (value < (p_ptr->stat_max_max[stat]-2))
        {                                                  /* v--- Scale all calcs by 10 */
            int delta = (p_ptr->stat_max_max[stat] - value) * 10;
            int pct = rand_range(200, 350);                /* Note: Old spread was about 14% to 40% */
            int max_value = p_ptr->stat_max_max[stat] - 1; /* e.g. 18/99 if max is 18/100 */
            int gain;

            gain = delta * pct / 1000;
            gain = (gain + 5) / 10; /* round back to an integer */
            if (gain < 2)
                gain = 2;

            value += gain;
            if (value > max_value)
                value = max_value;
        }
        /* Gain one point at a time */
        else
        {
            value++;
        }

        /* Save the new value */
        p_ptr->stat_cur[stat] = value;

        /* Bring up the maximum too */
        if (value > p_ptr->stat_max[stat])
        {
            p_ptr->stat_max[stat] = value;
        }

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Success */
        return (TRUE);
    }

    /* Nothing to gain */
    return (FALSE);
}



/*
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Amount could be a little higher in extreme cases to mangle very high
 * stats from massive assaults. -CWS
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent. This may not work exactly
 * as expected, due to "weirdness" in the algorithm, but in general,
 * if your stat is already drained, the "max" value will not drop all
 * the way down to the "cur" value.
 */
bool dec_stat(int stat, int amount, int permanent)
{
    int cur, max, loss, same, res = FALSE;


    /* Acquire current value */
    cur = p_ptr->stat_cur[stat];
    max = p_ptr->stat_max[stat];

    /* Note when the values are identical */
    same = (cur == max);

    /* Damage "current" value */
    if (cur > 3)
    {
        /* Handle "low" values */
        if (cur <= 18)
        {
            if (amount > 90) cur--;
            if (amount > 50) cur--;
            if (amount > 20) cur--;
            cur--;
        }

        /* Handle "high" values */
        else
        {
            /* Hack -- Decrement by a random amount between one-quarter */
            /* and one-half of the stat bonus times the percentage, with a */
            /* minimum damage of half the percentage. -CWS */
            loss = (((cur-18) / 2 + 1) / 2 + 1);

            /* Paranoia */
            if (loss < 1) loss = 1;

            /* Randomize the loss */
            loss = ((randint1(loss) + loss) * amount) / 100;

            /* Maximal loss */
            if (loss < amount/2) loss = amount/2;

            /* Lose some points */
            cur = cur - loss;

            /* Hack -- Only reduce stat to 17 sometimes */
            if (cur < 18) cur = (amount <= 20) ? 18 : 17;
        }

        /* Prevent illegal values */
        if (cur < 3) cur = 3;

        /* Something happened */
        if (cur != p_ptr->stat_cur[stat]) res = TRUE;
    }

    /* Damage "max" value */
    if (permanent && (max > 3))
    {
        virtue_add(VIRTUE_SACRIFICE, 1);
        if (stat == A_WIS || stat == A_INT)
            virtue_add(VIRTUE_ENLIGHTENMENT, -2);

        /* Handle "low" values */
        if (max <= 18)
        {
            if (amount > 90) max--;
            if (amount > 50) max--;
            if (amount > 20) max--;
            max--;
        }

        /* Handle "high" values */
        else
        {
            /* Hack -- Decrement by a random amount between one-quarter */
            /* and one-half of the stat bonus times the percentage, with a */
            /* minimum damage of half the percentage. -CWS */
            loss = (((max-18) / 2 + 1) / 2 + 1);
            loss = ((randint1(loss) + loss) * amount) / 100;
            if (loss < amount/2) loss = amount/2;

            /* Lose some points */
            max = max - loss;

            /* Hack -- Only reduce stat to 17 sometimes */
            if (max < 18) max = (amount <= 20) ? 18 : 17;
        }

        /* Hack -- keep it clean */
        if (same || (max < cur)) max = cur;

        /* Something happened */
        if (max != p_ptr->stat_max[stat]) res = TRUE;
    }

    /* Apply changes */
    if (res)
    {
        /* Actually set the stat to its new value. */
        p_ptr->stat_cur[stat] = cur;
        p_ptr->stat_max[stat] = max;

        /* Redisplay the stats later */
        p_ptr->redraw |= (PR_STATS);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
    }

    /* Done */
    return (res);
}


/*
 * Restore a stat. Return TRUE only if this actually makes a difference.
 */
bool res_stat(int stat)
{
    /* Restore if needed */
    if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat])
    {
        /* Restore */
        p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redisplay the stats later */
        p_ptr->redraw |= (PR_STATS);

        /* Success */
        return (TRUE);
    }

    /* Nothing to restore */
    return (FALSE);
}


/*
 * Increase players hit points, notice effects
 */
bool hp_player(int num)
{
    if (p_ptr->pclass == CLASS_BLOOD_KNIGHT || p_ptr->pclass == CLASS_BLOOD_MAGE)
    {
        num /= 2;        
        if (num == 0)
            return FALSE;
    }
    return hp_player_aux(num);
}

bool hp_player_aux(int num)
{
    int old_hp = p_ptr->chp;

    num = num * (virtue_current(VIRTUE_VITALITY) + 1250) / 1250;

    if (mut_present(MUT_SACRED_VITALITY))
    {
        num += num/5;
    }

    if ((p_ptr->prace == RACE_EINHERI) || (p_ptr->mimic_form == RACE_EINHERI)) num /= 2;
    if (disciple_is_(DISCIPLE_YEQREZH)) num = hp_player_yeqrezh(num);

    /* Healing needed */
    if (p_ptr->chp < p_ptr->mhp)
    {
        if ((num > 0) && (p_ptr->chp < (p_ptr->mhp/3)))
            virtue_add(VIRTUE_TEMPERANCE, 1);

        /* XXX Handle device lore ... of course, we don't know if a device
         * is actually being used atm, but it won't hurt to set the variable anyway. */
        if (p_ptr->chp + num <= p_ptr->mhp)
            device_lore = TRUE;

        /* Gain hitpoints */
        p_ptr->chp += num;

        /* Enforce maximum */
        if (p_ptr->chp >= p_ptr->mhp)
        {
            p_ptr->chp = p_ptr->mhp;
            p_ptr->chp_frac = 0;

            if (weaponmaster_is_(WEAPONMASTER_STAVES))
                p_ptr->update |= (PU_BONUS);
        }

        if (p_ptr->pclass == CLASS_BLOOD_KNIGHT)
            p_ptr->update |= PU_BONUS;

        /* Redraw */
        p_ptr->redraw |= (PR_HP);

        fear_heal_p(old_hp, p_ptr->chp);

        /* Notice */
        return (TRUE);
    }

    /* Ignore */
    return (FALSE);
}

bool lp_player(int num)
{
    bool notice = FALSE;
    int old_clp = p_ptr->clp;

    p_ptr->clp += num;
    if (p_ptr->clp > 1000) p_ptr->clp = 1000;
    if (p_ptr->clp <= 0)
    {
        if (p_ptr->pclass != CLASS_MONSTER)
        {
            int which;
            switch (randint1(5))
            {
            case 1: which = RACE_VAMPIRE; break;
            case 2: which = RACE_SKELETON; break;
            case 3: which = RACE_ZOMBIE; break;
            case 4: which = RACE_SPECTRE; break;
            case 5: which = RACE_EINHERI; break;
            }

            msg_print("<color:v>Your life force is exhausted!</color>");
            change_race(which, "");
            if (!(get_race()->flags & RACE_IS_NONLIVING)) /* race change failed for whatever reason, so instead of being undead we are now dead */
            {
                take_hit(DAMAGE_NOESCAPE, p_ptr->chp + 10, "running out of life force");
            }
            else
            {
                p_ptr->clp = 1000; /* full unlife */
            }
        }
        else
            p_ptr->clp = 0; /* monsters can't change their race ... */
    }
    if (p_ptr->clp != old_clp)
    {
        if (num < 0) msg_print("<color:D>You feel your life draining away!</color>");
        else if (num > 0) msg_print("<color:B>You feel your life returning.</color>");
        p_ptr->update |= PU_HP;
        p_ptr->redraw |= PR_EFFECTS;
        notice = TRUE;
    }
    return notice;
}

/* vampiric drain goes first to recovering the player's life,
 * and then, if any is left over, to recovering hit points */
bool vamp_player(int num)
{
    if (num > vampirism_hack) num = vampirism_hack;
    vampirism_hack = 1000;
    if (p_ptr->clp + num <= 1000)
        return lp_player(num);
    else if (p_ptr->clp < 1000)
    {
        int lp = 1000 - p_ptr->clp;
        lp_player(lp);
        num -= lp;
        assert(num > 0);
    }
    return hp_player(num);
}

bool sp_player(int num)
{
    bool notice = FALSE;
    int old_csp = p_ptr->csp;

    if (elemental_is_(ELEMENTAL_WATER)) return FALSE;

    p_ptr->csp += num;
    if (num > 0 && p_ptr->csp > p_ptr->msp) /* Mystics and Samurai super charge */
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }
    if (p_ptr->csp < 0)
    {
        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;
    }
    if (p_ptr->csp != old_csp)
    {
        p_ptr->redraw |= PR_MANA;
        notice = TRUE;
    }
    return notice;
}

/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_pos[] =
{
    "strong",

    "smart",

    "wise",

    "dextrous",

    "healthy",

    "self-confident"

};


/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_neg[] =
{
    "weak",
    "stupid",
    "naive",
    "clumsy",
    "sickly",
    "insecure"

};


/*
 * Lose a "point"
 */
bool do_dec_stat(int stat)
{
    bool sust = FALSE;

    /* Access the "sustain" */
    switch (stat)
    {
        case A_STR: if (p_ptr->sustain_str) sust = TRUE; break;
        case A_INT: if (p_ptr->sustain_int) sust = TRUE; break;
        case A_WIS: if (p_ptr->sustain_wis) sust = TRUE; break;
        case A_DEX: if (p_ptr->sustain_dex) sust = TRUE; break;
        case A_CON: if (p_ptr->sustain_con) sust = TRUE; break;
        case A_CHR: if (p_ptr->sustain_chr) sust = TRUE; break;
    }

    /* Sustain */
    if (sust && (!ironman_nightmare || randint0(13)))
    {
        if (disturb_minor)
            msg_format("You feel %s for a moment, but the feeling passes.", desc_stat_neg[stat]);

        equip_learn_flag(OF_SUST_STR + stat);
        return TRUE;
    }

    /* Attempt to reduce the stat */
    if (dec_stat(stat, 10, (ironman_nightmare && !randint0(13))))
    {
        /* Message */
        msg_format("You feel very <color:r>%s</color>.", desc_stat_neg[stat]);


        /* Notice effect */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}


/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int stat)
{
    /* Attempt to increase */
    if (res_stat(stat))
    {
        /* Message */
        msg_format("You feel less %s.", desc_stat_neg[stat]);


        /* Notice */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}


/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int stat)
{
    bool res;

    /* Restore strength */
    res = res_stat(stat);

    /* Attempt to increase */
    if (inc_stat(stat))
    {
        if (stat == A_WIS)
        {
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            virtue_add(VIRTUE_FAITH, 1);
        }
        else if (stat == A_INT)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
        }
        else if (stat == A_CON)
            virtue_add(VIRTUE_VITALITY, 1);

        /* Message */
        msg_format("Wow! You feel very %s!", desc_stat_pos[stat]);


        /* Notice */
        return (TRUE);
    }

    /* Restoration worked */
    if (res)
    {
        /* Message */
        msg_format("You feel less %s.", desc_stat_neg[stat]);


        /* Notice */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}


/*
 * Restores any drained experience
 */
bool restore_level(void)
{
    s32b max_exp = p_ptr->max_exp;

    /* Possessor Max Lvl is limited by their current form */
    if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
    {
        s32b racial_max = possessor_max_exp();
        if (max_exp > racial_max)
            max_exp = racial_max;
    }

    if (p_ptr->exp < max_exp)
    {
        msg_print("You feel your life energies returning.");
        p_ptr->exp = max_exp;
        check_experience();
        return TRUE;
    }
    return FALSE;
}

/*
 * Forget everything
 */
static void _forget(obj_ptr obj)
{
    if (!obj_is_identified_fully(obj))
    {
        obj->feeling = FEEL_NONE;
        obj->ident &= ~(IDENT_EMPTY);
        obj->ident &= ~(IDENT_TRIED);
        obj->ident &= ~(IDENT_KNOWN);
        obj->ident &= ~(IDENT_SENSE);
        if ((p_ptr->auto_pseudo_id) && ((obj_can_sense1(obj)) || (obj_can_sense2(obj))))
        {
            obj->feeling = value_check_aux1(obj, TRUE);
            if (!(obj->ident & (IDENT_KNOWN))) obj->ident |= IDENT_SENSE;
        }
    }
}

bool lose_all_info(void)
{
    if (never_forget) 
    {
        msg_print("You feel very forgetful for a moment, but the feeling passes.");
        return FALSE;
    }

    virtue_add(VIRTUE_KNOWLEDGE, -5);
    virtue_add(VIRTUE_ENLIGHTENMENT, -5);

    if (!p_ptr->auto_id)
    {
        pack_for_each(_forget);
        equip_for_each(_forget);
        quiver_for_each(_forget);

        p_ptr->update |= PU_BONUS;
        p_ptr->notice |= PN_OPTIMIZE_PACK;
        p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_OBJECT_LIST);
    }
    wiz_dark();

    return TRUE;
}

void do_poly_wounds(void)
{
    /* Changed to always provide at least _some_ healing */
    s16b wounds = p_ptr->cut;
    s16b hit_p = (p_ptr->mhp - p_ptr->chp);
    s16b change = damroll(p_ptr->lev, 5);
    bool Nasty_effect = one_in_(5);

    if (!(wounds || hit_p || Nasty_effect)) return;

    msg_print("Your wounds are polymorphed into less serious ones.");

    hp_player(change);
    if (Nasty_effect)
    {
        if (get_race()->flags & RACE_IS_NONLIVING)
        { /* Nonliving characters can't bleed but can suffer other bodily harm */
            msg_print("You suffer a new injury!");
            take_hit(DAMAGE_LOSELIFE, change / 2, "a polymorphed wound");
        }
        else
        {
            msg_print("A new wound was created!");
            take_hit(DAMAGE_LOSELIFE, change / 2, "a polymorphed wound");

            set_cut(change, FALSE);
        }
    }
    else
    {
        set_cut(p_ptr->cut - (change / 2), FALSE);
    }
}


/*
 * Change player race
 */
void change_race(int new_race, cptr effect_msg)
{
    cptr title = get_race_aux(new_race, 0)->name;
    int  old_race = p_ptr->prace;
    static bool _lock = FALSE; /* make sure change_race() can't cause calls to itself */

    if (_lock) return;
    if (get_race()->flags & RACE_IS_MONSTER) return;
    if (get_race_aux(new_race, 0)->flags & RACE_IS_MONSTER) return;
    if (old_race == RACE_ANDROID) return;
    if (player_obviously_poly_immune(FALSE)) return;
    if (new_race == old_race) return;
    if (comp_mode)
    {
        nonlethal_ty_substitute(TRUE);
        return;
    }

    _lock = TRUE;

    if (old_race == RACE_HUMAN || old_race == RACE_DEMIGOD || mut_present(p_ptr->demigod_power[0]))
    {
        int i, idx;
        for (i = 0; i < MAX_DEMIGOD_POWERS; i++)
        {
            idx = p_ptr->demigod_power[i];
            if (idx >= 0)
            {
                mut_unlock(idx);
                mut_lose(idx);
                p_ptr->demigod_power[i] = -1;
            }
        }
        p_ptr->psubrace = 0;
    }
    if (old_race == RACE_DRACONIAN)
    {
        int idx = p_ptr->draconian_power;
        if (idx >= 0)
        {
            mut_unlock(idx);
            mut_lose(idx);
            p_ptr->draconian_power = -1;
            if (idx == MUT_DRACONIAN_METAMORPHOSIS)
                equip_on_change_race();
        }
        p_ptr->psubrace = 0;
    }

    msg_format("You turn into %s %s%s!", (!effect_msg[0] && is_a_vowel(title[0]) ? "an" : "a"), effect_msg, title);

    virtue_add(VIRTUE_CHANCE, 2);

    if (p_ptr->prace < 32)
    {
        p_ptr->old_race1 |= 1L << p_ptr->prace;
    }
    else if (p_ptr->prace < 64)
    {
        p_ptr->old_race2 |= 1L << (p_ptr->prace-32);
    }
    else if (p_ptr->prace < 96)
    {
        p_ptr->old_race3 |= 1L << (p_ptr->prace-64);
    }
    p_ptr->prace = new_race;
    p_ptr->psubrace = 0;

    /* Experience factor */
    p_ptr->expfact = calc_exp_factor();

    do_cmd_rerate(FALSE);

    /* The experience level may be modified */
    check_experience();

    if (get_true_race()->flags & RACE_DEMI_TALENT)
    {
        race_t *race_ptr = get_true_race();
        if (race_ptr != NULL && race_ptr->gain_level != NULL)
            race_ptr->gain_level(p_ptr->lev);    /* This is OK ... Just make sure we get to choose racial powers on poly */
    }

    /* Check changes to body template (e.g. Centaurs) */
    equip_on_change_race();

    p_ptr->redraw |= (PR_BASIC);

    p_ptr->update |= (PU_BONUS);

    handle_stuff();

    /* Load an autopick preference file */
    if (old_race != p_ptr->prace) autopick_load_pref(0);

    /* Player's graphic tile may change */
    lite_spot(py, px);

    _lock = FALSE;
}


void do_poly_self(void)
{
    int power = p_ptr->lev;

    msg_print("You feel a change coming over you...");

    virtue_add(VIRTUE_CHANCE, 1);

    if ((power > randint0(20)) && one_in_(3) && (p_ptr->prace != RACE_ANDROID) && (p_ptr->prace != RACE_WEREWOLF) && (!(get_race()->flags & RACE_NO_POLY)))
    {
        char effect_msg[80] = "";
        int new_race, expfact, goalexpfact;

        /* Some form of racial polymorph... */
        power -= 10;

        if ((power > randint0(5)) && one_in_(4))
        {
            /* sex change */
            power -= 2;

            if (p_ptr->psex == SEX_MALE)
            {
                p_ptr->psex = SEX_FEMALE;
                sprintf(effect_msg, "female ");
                mut_lose(MUT_IMPOTENCE);
            }
            else
            {
                p_ptr->psex = SEX_MALE;
                sprintf(effect_msg, "male ");

            }
        }

        if ((power > randint0(30)) && one_in_(5))
        {
            int tmp = 0;

            /* Harmful deformity */
            power -= 15;

            while (tmp < 6)
            {
                if (one_in_(2))
                {
                    (void)dec_stat(tmp, randint1(6) + 6, one_in_(3));
                    power -= 1;
                }
                tmp++;
            }

            /* Deformities are discriminated against! */
            (void)dec_stat(A_CHR, randint1(6), TRUE);

            if (effect_msg[0])
            {
                char tmp_msg[10];
                sprintf(tmp_msg,"%s",effect_msg);
                sprintf(effect_msg,"deformed %s",tmp_msg);

            }
            else
            {
                sprintf(effect_msg,"deformed ");

            }
        }

        while ((power > randint0(20)) && one_in_(10))
        {
            /* Polymorph into a less mutated form */
            power -= 10;

            if (!mut_lose_random(NULL))
                msg_print("You feel oddly normal.");

        }

        /*
         * Restrict the race choices by exp penalty so
         * weak polymorph always means weak race
         */
        if (power < 0)
            goalexpfact = 100;
        else
            goalexpfact = 100 + 3 * randint0(power);

        do
        {
            new_race = randint0(36); /* Hack: Skip monster races and androids ... */
            expfact = get_race_aux(new_race, 0)->exp;
        }
        while (((new_race == p_ptr->prace) && (expfact > goalexpfact)) || (new_race == RACE_ANDROID));

        change_race(new_race, effect_msg);
    }

    if ((power > randint0(30)) && one_in_(6))
    {
        int tmp = 0;

        /* Abomination! */
        power -= 20;

        msg_print("Your internal organs are rearranged!");

        while (tmp < 6)
        {
            (void)dec_stat(tmp, randint1(6) + 6, one_in_(3));
            tmp++;
        }
        if (one_in_(6))
        {
            msg_print("You find living difficult in your present form!");
            take_hit(DAMAGE_LOSELIFE, damroll(randint1(10), p_ptr->lev), "a lethal mutation");

            power -= 10;
        }
    }

    if ((power > randint0(20)) && one_in_(4))
    {
        power -= 10;

        get_max_stats();
        do_cmd_rerate(FALSE);
    }

    while ((power > randint0(15)) && one_in_(3))
    {
        power -= 7;
        mut_gain_random(NULL);
    }

    if (power > randint0(5))
    {
        power -= 5;
        do_poly_wounds();
    }

    if (no_scrambling && one_in_(2)) return;

    /* Note: earlier deductions may have left power < 0 already. */
    while (power > 0)
    {
        mutate_player();
        if (no_scrambling) break;
        power--;
    }
}

/*
 * Decreases players hit points and sets death flag if necessary
 *
 * XXX XXX XXX Invulnerability needs to be changed into a "shield"
 *
 * XXX XXX XXX Hack -- this function allows the user to save (or quit)
 * the game when he dies, since the "You die." message is shown before
 * setting the player to "dead".
 */

int take_hit(int damage_type, int damage, cptr hit_from)
{
    int old_chp = p_ptr->chp;

    char death_message[1024];
    int warning = (p_ptr->mhp * hitpoint_warn / 10);

    /* Paranoia */
    if (p_ptr->is_dead) return 0;
    if (!damage) return 0;

    if (p_ptr->sutemi) damage *= 2;
    if (p_ptr->special_defense & KATA_IAI) damage += (damage + 4) / 5;
    if (check_foresight()) return 0;
    if (statistics_hack) return 0;

    if (damage_type != DAMAGE_USELIFE)
    {
        /* Disturb */
        disturb(1, 0);
    }

    /* Mega-Hack -- Apply "invulnerability" */
    if ( damage_type != DAMAGE_USELIFE
      && damage_type != DAMAGE_LOSELIFE )
    {
        if (IS_INVULN() && damage < 9000)
        {
            if (damage_type == DAMAGE_FORCE)
            {
                msg_print("The attack cuts your shield of invulnerability open!");
            }
            else if (one_in_(PENETRATE_INVULNERABILITY))
            {
                msg_print("The attack penetrates your shield of invulnerability!");
            }
            else
            {
                return 0;
            }
        }

        if (damage_type != DAMAGE_NOESCAPE && CHECK_MULTISHADOW())
        {
            if (damage_type == DAMAGE_FORCE)
            {
                msg_print("The attack hits Shadow together with you!");
            }
            else if (damage_type == DAMAGE_ATTACK)
            {
                msg_print("The attack hits Shadow, you are unharmed!");
                return 0;
            }
        }

        if (IS_WRAITH())
        {
            if (damage_type == DAMAGE_FORCE)
            {
                msg_print("The attack cuts through your ethereal body!");
            }
            else
            {
                damage /= 2;
                if ((damage == 0) && one_in_(2)) damage = 1;
            }
        }

        if (p_ptr->special_defense & KATA_MUSOU)
        {
            damage /= 2;
            if ((damage == 0) && one_in_(2)) damage = 1;
        }
    } /* not if LOSELIFE USELIFE */


    /* Tera-Hack:  Duelist Nemesis */
/*    if ( p_ptr->pclass == CLASS_DUELIST
      && p_ptr->duelist_target_idx
      && p_ptr->duelist_target_idx == hack_m_idx
      && p_ptr->lev >= 45
      && damage > p_ptr->chp )
    {
        nemesis_hack = TRUE;
        damage = 0;
        msg_print("Nemesis!!!!  You cannot be slain by your current target!");
        set_stun(99, FALSE);
        msg_format("%^s is no longer your current target.", duelist_current_challenge());
        p_ptr->duelist_target_idx = 0;
        p_ptr->redraw |= PR_STATUS;
    } */
    
    /* Rage Mage: "Rage Fueled" */
    if ( p_ptr->pclass == CLASS_RAGE_MAGE
      && (!hit_from || strcmp(hit_from, "Rage") != 0))
    {
        rage_mage_rage_fueled(damage);
    }

    /* Hurt the player */
    if (p_ptr->tim_transcendence && p_ptr->csp > 0)
    {
        int sp = MIN(p_ptr->csp, damage);
        sp_player(-sp);
        damage -= sp;
        damage = MAX(0, damage);
    }

    
    if ((p_ptr->wizard || easy_damage) && (damage > 0))
        msg_format("You take %d damage.", damage);

    p_ptr->chp -= damage;
    if(damage_type == DAMAGE_GENO && p_ptr->chp < 0)
    {
        damage += p_ptr->chp;
        p_ptr->chp = 0;
    }

    fear_hurt_p(old_chp, p_ptr->chp);

    if (p_ptr->prace == RACE_MON_POSSESSOR)
        possessor_on_take_hit();

    /* Display the hitpoints */
    p_ptr->redraw |= (PR_HP);

    /* This might slow things down a bit ... 
       But, Blood Knight power varies with hp. */
    if (p_ptr->pclass == CLASS_BLOOD_KNIGHT)
        p_ptr->update |= (PU_BONUS);

    if (weaponmaster_is_(WEAPONMASTER_STAVES))
        p_ptr->update |= (PU_BONUS);

    handle_stuff();

    if (damage_type != DAMAGE_GENO && p_ptr->chp == 0)
    {
        virtue_add(VIRTUE_SACRIFICE, 1);
        virtue_add(VIRTUE_CHANCE, 2);
        if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_CHANCE);
    }

    /* Dead player */
    if (p_ptr->chp < 0)
    {
        bool android = (p_ptr->prace == RACE_ANDROID ? TRUE : FALSE);

        /* Sound */
        sound(SOUND_DEATH);

        virtue_add(VIRTUE_SACRIFICE, 10);

        /* Leaving */
        p_ptr->leaving = TRUE;

        /* Note death */
        p_ptr->is_dead = TRUE;

        if (p_ptr->inside_arena)
        {
            cptr m_name = r_name+r_info[arena_info[p_ptr->arena_number].r_idx].name;
            msg_format("You are beaten by %s.", m_name);
            msg_print(NULL);
        }
        else if ((p_ptr->total_winner) && (unique_is_friend(MON_R_MACHINE)))
        {
            msg_print(android ? "You are broken." : "You die.");
            msg_print(NULL);
        }
        else
        {
            bool seppuku = streq(hit_from, "Seppuku");
            bool winning_seppuku = p_ptr->total_winner && seppuku;

            /* Note cause of death */
            if (seppuku)
            {
                strcpy(p_ptr->died_from, hit_from);
            }
            else
            {
                char dummy[1024];
                sprintf(dummy, "%s%s", hit_from, !p_ptr->paralyzed ? "" : " while helpless");
                clip_and_locate(" (Foe)", dummy);
                my_strcpy(p_ptr->died_from, dummy, sizeof p_ptr->died_from);
            }

            msg_add_tiny_screenshot(50, 24);

            flush();

            if (get_check_strict("Dump the screen? ", CHECK_NO_HISTORY))
            {
                do_cmd_save_screen();
            }

            flush();

            /* Initialize "last message" buffer */
            if (p_ptr->last_message) z_string_free(p_ptr->last_message);
            p_ptr->last_message = NULL;

            /* Hack -- Note death */
            if (!last_words)
            {
                msg_print(android ? "You are broken." : "You die.");
                msg_print(NULL);
            }
            else
            {
                if (winning_seppuku)
                {
                    get_rnd_line("seppuku.txt", 0, death_message);
                }
                else
                {
                    get_rnd_line("death.txt", 0, death_message);
                }

                do
                {
                    while (!get_string("Last word: ", death_message, 1024)) ;
                }
                while (winning_seppuku && !get_check_strict("Are you sure? ", CHECK_NO_HISTORY));

                if (death_message[0] == '\0')
                {
                    strcpy(death_message, android ? "You are broken." : "You die.");
                }
                else p_ptr->last_message = z_string_make(death_message);
                
                msg_print(death_message);
            }
        }

        /* Dead */
        return damage;
    }

    if ((p_ptr->personality == PERS_SPLIT) && (shuffling_hack_hp > 0) && (p_ptr->chp <= (p_ptr->mhp / 4)) &&
        (p_ptr->chp < (shuffling_hack_hp / 2)) && ((shuffling_hack_hp - p_ptr->chp) >= (p_ptr->mhp / 5)) &&
        (randint0(p_ptr->mhp / 3) >= p_ptr->chp))
    {
        split_shuffle(0);
        shuffling_hack_hp = 0;
    }

    /* Hitpoint warning */
    if (p_ptr->chp < warning && !world_monster)
    {
        if ((warning_hack_hp) && (warning_hack_hp < p_ptr->chp))
        {
        }
        else
        {
            sound(SOUND_WARN);

            /* Hack -- stop the player on first crossing the threshold */
            if (old_chp >= warning) 
            {
                msg_prompt("<color:v>*** LOW HITPOINT WARNING! ***</color> Press <color:y>Space</color> to continue.", " ", PROMPT_FORCE_CHOICE);
            }
            else
            {
                cmsg_print(TERM_VIOLET, "*Ouch!*");
                flush();
            }
        }
    }
    warning_hack_hp = 0;
    if (p_ptr->wild_mode && !p_ptr->leaving && (p_ptr->chp < MAX(warning, p_ptr->mhp/5)))
    {
        change_wild_mode();
    }
    else if ((disciple_is_(DISCIPLE_TROIKA)) && (p_ptr->mhp / (MAX(1, old_chp - p_ptr->chp))) < 6) /* counterintuitively, the MAX(1,) is needed - the difference actually can be zero in some strange circumstances */
    {
        troika_effect(TROIKA_TAKE_HIT);
    }
    return damage;
}


/*
 * Gain experience
 */
void gain_exp_64(s32b amount, u32b amount_frac)
{
    if (p_ptr->is_dead) return;

    if (p_ptr->prace == RACE_ANDROID) return;

    if ( (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC) 
      && !possessor_can_gain_exp() )
    {
        return;
    }

    /* Gain some experience */
    s64b_add(&(p_ptr->exp), &(p_ptr->exp_frac), amount, amount_frac);

    /* Slowly recover from experience drainage */
    if (p_ptr->exp < p_ptr->max_exp)
    {
        /* Gain max experience (20%) (was 10%) */
        p_ptr->max_exp += amount / 5;
    }

    /* Check Experience ... later. Definitely not during melee attacks.
     * However, stat runs can check now ... otherwise, they gain CL1 exp
     * from all kills until the stat run is finished! */
    if (statistics_hack)
        check_experience();
    else
        p_ptr->notice |= PN_EXP;
}


/*
 * Gain experience
 */
void gain_exp(s32b amount)
{
    gain_exp_64(amount, 0L);
}

/*
 * Lose experience
 */
void lose_exp(s32b amount)
{
    if (p_ptr->prace == RACE_ANDROID) return;

    /* Never drop below zero experience */
    if (amount > p_ptr->exp) amount = p_ptr->exp;

    /* Lose some experience */
    p_ptr->exp -= amount;

    /* Check Experience */
    check_experience();
}


/*
 * Drain experience
 * Return amount drained.
 */
int drain_exp(s32b drain, s32b slip, int hold_life_prob)
{
    int i;

    /* Androids use construction points, not experience points */
    if (p_ptr->prace == RACE_ANDROID) return 0;

    for (i = 0; i < p_ptr->hold_life; i++)
    {
        if (p_ptr->hold_life && (randint0(100) < hold_life_prob))
        {
            msg_print("You keep hold of your life force!");
            return 0;
        }
    }

    if (p_ptr->hold_life)
    {
        msg_print("You feel your life slipping away!");
        lose_exp(slip);
        return slip;
    }
    msg_print("You feel your life draining away!");
    lose_exp(drain);
    return drain;
}


bool set_ultimate_res(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->ult_res && !do_dec)
        {
            if (p_ptr->ult_res > v) return FALSE;
        }
        else if (!p_ptr->ult_res)
        {
            msg_print("You feel resistant!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->ult_res)
        {
            msg_print("You feel less resistant.");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->ult_res = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_res_nether(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_res_nether && !do_dec)
        {
            if (p_ptr->tim_res_nether > v) return FALSE;
        }
        else if (!p_ptr->tim_res_nether)
        {
            msg_print("You feel nether resistant!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_res_nether)
        {
            msg_print("You feel less nether resistant");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_res_nether = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_res_time(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_res_time && !do_dec)
        {
            if (p_ptr->tim_res_time > v) return FALSE;
        }
        else if (!p_ptr->tim_res_time)
        {
            msg_print("You feel time resistant!");

            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_res_time)
        {
            msg_print("You feel less time resistant");

            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_res_time = v;

    /* Redraw status bar */
    p_ptr->redraw |= (PR_STATUS);

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_state) disturb(0, 0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}

bool set_tim_res_disenchantment(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_res_disenchantment && !do_dec)
        {
            if (p_ptr->tim_res_disenchantment > v) return FALSE;
        }
        else if (!p_ptr->tim_res_disenchantment)
        {
            msg_print("You feel resistant to disenchantment.");
            notice = TRUE;
        }
    }

    /* Shut */
    else
    {
        if (p_ptr->tim_res_disenchantment)
        {
            msg_print("You no longer resist disenchantment.");
            notice = TRUE;
        }
    }

    p_ptr->tim_res_disenchantment = v;
    p_ptr->redraw |= PR_STATUS;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

/*
 * Choose a warrior-mage elemental attack. -LM-
 */
bool choose_ele_attack(void)
{
    int num;

    char choice;

    /* Save screen */
    screen_save();

    num = (p_ptr->lev - 20) / 5;

              c_prt(TERM_RED,    "        a) Fire Brand", 2, 14);

    if (num >= 2) c_prt(TERM_L_WHITE,"        b) Cold Brand", 3, 14);
    else prt("", 3, 14);

    if (num >= 3) c_prt(TERM_GREEN,  "        c) Poison Brand", 4, 14);
    else prt("", 4, 14);

    if (num >= 4) c_prt(TERM_L_DARK, "        d) Acid Brand", 5, 14);
    else prt("", 5, 14);

    if (num >= 5) c_prt(TERM_BLUE,   "        e) Elec Brand", 6, 14);
    else prt("", 6, 14);

    prt("", 7, 14);
    prt("", 8, 14);
    prt("", 9, 14);

    prt("", 1, 0);
    prt("        Choose a temporary elemental brand ", 1, 14);

    choice = inkey();

    if ((choice == 'a') || (choice == 'A')) 
        set_ele_attack(ATTACK_FIRE, p_ptr->lev/2 + randint1(p_ptr->lev/2));
    else if (((choice == 'b') || (choice == 'B')) && (num >= 2))
        set_ele_attack(ATTACK_COLD, p_ptr->lev/2 + randint1(p_ptr->lev/2));
    else if (((choice == 'c') || (choice == 'C')) && (num >= 3))
        set_ele_attack(ATTACK_POIS, p_ptr->lev/2 + randint1(p_ptr->lev/2));
    else if (((choice == 'd') || (choice == 'D')) && (num >= 4))
        set_ele_attack(ATTACK_ACID, p_ptr->lev/2 + randint1(p_ptr->lev/2));
    else if (((choice == 'e') || (choice == 'E')) && (num >= 5))
        set_ele_attack(ATTACK_ELEC, p_ptr->lev/2 + randint1(p_ptr->lev/2));
    else
    {
        msg_print("You cancel the temporary branding.");
        screen_load();
        return FALSE;
    }
    /* Load screen */
    screen_load();
    return TRUE;
}


/*
 * Choose a elemental immune. -LM-
 */
bool choose_ele_immune(int turn)
{
    char choice;

    /* Save screen */
    screen_save();

    c_prt(TERM_RED,    "        a) Immune Fire", 2, 14);

    c_prt(TERM_L_WHITE,"        b) Immune Cold", 3, 14);

    c_prt(TERM_L_DARK, "        c) Immune Acid", 4, 14);

    c_prt(TERM_BLUE,   "        d) Immune Elec", 5, 14);


    prt("", 6, 14);
    prt("", 7, 14);
    prt("", 8, 14);
    prt("", 9, 14);

    prt("", 1, 0);
    prt("        Choose a temporary elemental immune ", 1, 14);

    choice = inkey();

    if ((choice == 'a') || (choice == 'A')) 
        set_ele_immune(DEFENSE_FIRE, turn);
    else if ((choice == 'b') || (choice == 'B'))
        set_ele_immune(DEFENSE_COLD, turn);
    else if ((choice == 'c') || (choice == 'C'))
        set_ele_immune(DEFENSE_ACID, turn);
    else if ((choice == 'd') || (choice == 'D'))
        set_ele_immune(DEFENSE_ELEC, turn);
    else
    {
        msg_print("You cancel the temporary immune.");
        screen_load();
        return FALSE;
    }
    /* Load screen */
    screen_load();
    return TRUE;
}

bool set_tim_sustain_str(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sustain_str)
        {
            if (p_ptr->tim_sustain_str > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel your strength sustained.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_sustain_str)
        {
            msg_print("Your strength is no longer sustained.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sustain_str = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sustain_int(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sustain_int)
        {
            if (p_ptr->tim_sustain_int > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel your intelligence sustained.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_sustain_int)
        {
            msg_print("Your intelligence is no longer sustained.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sustain_int = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sustain_wis(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sustain_wis)
        {
            if (p_ptr->tim_sustain_wis > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel your wisdom sustained.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_sustain_wis)
        {
            msg_print("Your wisdom is no longer sustained.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sustain_wis = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sustain_dex(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sustain_dex)
        {
            if (p_ptr->tim_sustain_dex > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel your dexterity sustained.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_sustain_dex)
        {
            msg_print("Your dexterity is no longer sustained.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sustain_dex = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sustain_con(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sustain_con)
        {
            if (p_ptr->tim_sustain_con > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel your constitution sustained.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_sustain_con)
        {
            msg_print("Your constitution is no longer sustained.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sustain_con = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sustain_chr(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sustain_chr)
        {
            if (p_ptr->tim_sustain_chr > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel your charisma sustained.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_sustain_chr)
        {
            msg_print("Your charisma is no longer sustained.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sustain_chr = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_hold_life(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_hold_life)
        {
            if (p_ptr->tim_hold_life > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You feel a firm grip on your life force.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_hold_life)
        {
            msg_print("You lose your grip on your life force.");
            notice = TRUE;
        }
    }

    p_ptr->tim_hold_life = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_transcendence(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_transcendence)
        {
            if (p_ptr->tim_transcendence > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You transcend your lowly existence.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_transcendence)
        {
            msg_print("You are no longer transcendent.");
            notice = TRUE;
        }
    }

    p_ptr->tim_transcendence = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_dark_stalker(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_dark_stalker)
        {
            if (p_ptr->tim_dark_stalker > v && !do_dec) return FALSE;
        }
        else
        {
            if (p_ptr->pclass == CLASS_ROGUE || p_ptr->pclass == CLASS_SKILLMASTER)
                msg_print("You begin to tread softly.");
            else if (p_ptr->pclass == CLASS_NECROMANCER || p_ptr->pclass == CLASS_RUNE_KNIGHT || prace_is_(RACE_MON_MUMMY))
                msg_print("You are cloaked in darkness.");
            else
                msg_print("You begin to stalk your prey.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_dark_stalker)
        {
            if (p_ptr->pclass == CLASS_ROGUE || p_ptr->pclass == CLASS_SKILLMASTER)
                msg_print("You no longer tread softly.");
            else if (p_ptr->pclass == CLASS_NECROMANCER || p_ptr->pclass == CLASS_RUNE_KNIGHT || prace_is_(RACE_MON_MUMMY))
                msg_print("You are no longer cloaked in darkness.");
            else
                msg_print("You no longer stalk your prey.");
            notice = TRUE;
        }
    }

    p_ptr->tim_dark_stalker = v;
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_nimble_dodge(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_nimble_dodge)
        {
            if (p_ptr->tim_nimble_dodge > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You begin to dodge enemy breaths.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_nimble_dodge)
        {
            msg_print("You no longer dodge enemy breaths.");
            notice = TRUE;
        }
    }

    p_ptr->tim_nimble_dodge = v;
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_stealthy_snipe(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_stealthy_snipe)
        {
            if (p_ptr->tim_stealthy_snipe > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You are a stealthy sniper.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_stealthy_snipe)
        {
            msg_print("You are no longer a stealthy sniper.");
            notice = TRUE;
        }
    }

    p_ptr->tim_stealthy_snipe = v;
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_killing_spree(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_killing_spree)
        {
            if (p_ptr->tim_killing_spree > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You go on a killing spree!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_killing_spree)
        {
            msg_print("You have seen enough blood and suffering for now.");
            notice = TRUE;
        }
    }

    p_ptr->tim_killing_spree = v;
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_slay_sentient(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_slay_sentient)
        {
            if (p_ptr->tim_slay_sentient > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("Your weapon roars with thirsty glee!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_slay_sentient)
        {
            msg_print("Your weapon returns to normal.");
            notice = TRUE;
        }
    }

    p_ptr->tim_slay_sentient = v;
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= (PR_STATUS);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_quick_walk(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_quick_walk)
        {
            if (p_ptr->tim_quick_walk > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You move with great haste.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_quick_walk)
        {
            msg_print("You are no longer walking quickly.");
            notice = TRUE;
        }
    }

    p_ptr->tim_quick_walk = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_inven_prot(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_inven_prot)
        {
            if (p_ptr->tim_inven_prot > v && !do_dec) return FALSE;
        }
        else
        {
            if (p_ptr->pclass == CLASS_ROGUE)
                msg_print("You feel your loot is safe.");
            else
                msg_print("Your inventory seems safer now.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_inven_prot)
        {
            if (p_ptr->pclass == CLASS_ROGUE)
                msg_print("Your loot feels exposed once again.");
            else
                msg_print("Your inventory is no longer protected.");
            notice = TRUE;
        }
    }

    p_ptr->tim_inven_prot = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_inven_prot2(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 100) ? 100 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_inven_prot2)
        {
            if (p_ptr->tim_inven_prot2 > v && !do_dec) return FALSE;
        }
        else
        {
            if (p_ptr->pclass == CLASS_ROGUE)
                msg_print("You feel your loot is safe.");
            else
                msg_print("Your inventory seems safer now.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_inven_prot2)
        {
            if (p_ptr->pclass == CLASS_ROGUE)
                msg_print("Your loot is no longer completely safe.");
            else if (p_ptr->tim_inven_prot)
                msg_print("Your inventory is no longer completely safe.");
            else
                msg_print("Your inventory is no longer protected.");
            notice = TRUE;
        }
    }

    p_ptr->tim_inven_prot2 = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sh_shards(int v, bool do_dec)
{
    bool notice = FALSE;

    if (p_ptr->is_dead) return FALSE;

    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (v)
    {
        if (p_ptr->tim_sh_shards && !do_dec)
        {
            if (p_ptr->tim_sh_shards > v) return FALSE;
        }
        else if (!p_ptr->tim_sh_shards)
        {
            msg_print("You are enveloped in shards!");
            notice = TRUE;
        }
    }
    else
    {
        if (p_ptr->tim_sh_shards)
        {
            msg_print("You are no longer enveloped in shards.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sh_shards = v;
    p_ptr->redraw |= PR_STATUS;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sh_domination(int v, bool do_dec)
{
    bool notice = FALSE;

    if (p_ptr->is_dead) return FALSE;

    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (v)
    {
        if (p_ptr->tim_sh_domination && !do_dec)
        {
            if (p_ptr->tim_sh_domination > v) return FALSE;
        }
        else if (!p_ptr->tim_sh_domination)
        {
            msg_print("Your presence becomes truly awe-inspiring!");
            notice = TRUE;
        }
    }
    else
    {
        if (p_ptr->tim_sh_domination)
        {
            msg_print("Your presence returns to normal.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sh_domination = v;
    p_ptr->redraw |= PR_STATUS;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}


bool set_tim_sh_elements(int v, bool do_dec)
{
    bool notice = FALSE;

    if (p_ptr->is_dead) return FALSE;

    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (v)
    {
        if (p_ptr->tim_sh_elements && !do_dec)
        {
            if (p_ptr->tim_sh_elements > v) return FALSE;
        }
        else if (!p_ptr->tim_sh_elements)
        {
            msg_print("You are enveloped in the elements!");
            notice = TRUE;
        }
    }
    else
    {
        if (p_ptr->tim_sh_elements)
        {
            msg_print("Your elemental cloak has vanished.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sh_elements = v;
    p_ptr->redraw |= (PR_STATUS);
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_weaponmastery(int v, bool do_dec)
{
    bool notice = FALSE;

    if (p_ptr->is_dead) return FALSE;
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (v)
    {
        if (p_ptr->tim_weaponmastery && !do_dec)
        {
            if (p_ptr->tim_weaponmastery > v) return FALSE;
        }
        else if (!p_ptr->tim_weaponmastery)
        {
            if (p_ptr->weapon_info[0].bare_hands)
                msg_print("Your fists seem more powerful!");
            else
                msg_print("Your weapon seems more powerful!");
            notice = TRUE;
        }
    }
    else
    {
        if (p_ptr->tim_weaponmastery)
        {
            if (p_ptr->weapon_info[0].bare_hands)
                msg_print("Your fists return to normal.");
            else
                msg_print("Your weapon returns to normal.");
            notice = TRUE;
        }
    }

    p_ptr->tim_weaponmastery = v;
    p_ptr->redraw |= (PR_STATUS);
    if (!notice) return (FALSE);
    if (disturb_state) disturb(0, 0);
    p_ptr->update |= (PU_BONUS);
    handle_stuff();
    return (TRUE);
}

bool set_tim_device_power(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_device_power)
        {
            if (p_ptr->tim_device_power > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("Your magical devices feel more powerful.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_device_power)
        {
            msg_print("Your magical devices return to normal.");
            notice = TRUE;
        }
    }

    p_ptr->tim_device_power = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_sh_time(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_sh_time)
        {
            if (p_ptr->tim_sh_time > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You are cloaked in time.");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_sh_time)
        {
            msg_print("You are no longer cloaked in time.");
            notice = TRUE;
        }
    }

    p_ptr->tim_sh_time = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}

bool set_tim_foresight(int v, bool do_dec)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    /* Open */
    if (v)
    {
        if (p_ptr->tim_foresight)
        {
            if (p_ptr->tim_foresight > v && !do_dec) return FALSE;
        }
        else
        {
            msg_print("You can see the future!");
            notice = TRUE;
        }
    }
    /* Shut */
    else
    {
        if (p_ptr->tim_foresight)
        {
            msg_print("You can no longer see the future.");
            notice = TRUE;
        }
    }

    p_ptr->tim_foresight = v;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return TRUE;
}
