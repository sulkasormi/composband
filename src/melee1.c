/* File: melee1.c */

/* Purpose: Monster attacks */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

#include "angband.h"

/*
 * Determine if a monster attack against the player succeeds.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match monster power against player armor.
 */
static int _check_hit(int skill, int ac)
{
    int k;

    /* Percentile dice */
    k = randint0(100);

    /* Drunken Boxing does not give AC damage reduction ... */
    if (p_ptr->special_attack & ATTACK_SUIKEN) ac += (p_ptr->lev * 2);

    /* Hack -- Always miss or hit */
    if (k < 10) return (k < 5);

    /* Power and Level compete against Armor */
    if (skill > 0 && randint1(skill) > ac*3/4) return TRUE;

    /* Assume miss */
    return FALSE;
}



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_insult[] =
{
    "insults you",
    "insults your mother",
    "gives you the finger",
    "humiliates you",
    "defiles you",
    "dances around you",
    "makes obscene gestures",
    "moons you",
    "calls you a parasite",
    "calls you a cyborg"
};



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_moan[] =
{
    "seems sad about something",
    "asks if you have seen his dogs",
    "tells you to get off his land",
    "mumbles something about mushrooms"

};

/* Player AC reduces the melee damage of HURT, SUPERHURT and SHATTER
 * attacks. Damage from other attack types is *not* reduced. */ 
int ac_melee_pct_aux(int ac, int max_reduce, int max_ac)
{
    int pct;
    if (ac > max_ac) ac = max_ac;
    if (ac < 0) ac = 0;
    pct = max_reduce*ac/max_ac;
    return 100 - pct;
}

int ac_melee_pct(int ac)
{
    return ac_melee_pct_aux(ac, 60, 180);
}

int reduce_melee_dam_p(int dam)
{
    int result = dam;
    switch (weaponmaster_get_toggle())
    {
    case TOGGLE_BULWARK:
        result -= (result + 2)/3;
        break;
    }

    if (result < 0) result = 0;
    return result;
}

/* Apologies for the following hacks ... I'm too lazy to refactor monster attack code! */
bool nemesis_hack = FALSE;
bool retaliation_hack = FALSE;
int retaliation_count = 0;

static int _aura_dam_p(int taso)
{
    return 2 + damroll(taso * 2 - 1 + (p_ptr->lev / 10), 2 + (p_ptr->lev / 10));
}

/*
 * Attack the player via physical attacks.
 */
bool make_attack_normal(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    int           stun = MON_STUNNED(m_ptr);

    int ap_cnt, ht_cnt;

    int j, k, ac, rlev, skill;

    s32b gold;

    object_type *o_ptr;

    char o_name[MAX_NLEN];

    char m_name[80];

    char ddesc[80];

    bool blinked;
    bool touched = FALSE, fear = FALSE, alive = TRUE;
    bool explode = FALSE;
    bool do_silly_attack = (one_in_(2) && p_ptr->image);
    int total_dam = 0;
    int opy = py;
    int opx = px;

    /* Silver monsters have special effects on werewolves */
    bool track_werewolf_dam = (((p_ptr->prace == RACE_WEREWOLF) || (p_ptr->current_r_idx == MON_WEREWOLF)) && (r_ptr->flags7 & RF7_SILVER)) ? TRUE : FALSE;

    /* Not allowed to attack */
    if (r_ptr->flags1 & (RF1_NEVER_BLOW)) return (FALSE);

    if (d_info[dungeon_type].flags1 & DF1_NO_MELEE) return (FALSE);

    /* ...nor if friendly */
    if (!is_hostile(m_ptr)) return FALSE;

    /* Extract the effective monster level */
    rlev = MAX(4, r_ptr->level);
    rlev = rlev * m_ptr->mpower / 1000;

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);

    /* Get the "died from" information (i.e. "a kobold") */
    monster_desc(ddesc, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);

    if (p_ptr->prace == RACE_MON_RING && ring_dominate_m(m_idx))
        return FALSE;

    if (p_ptr->special_defense & KATA_IAI)
    {
        msg_format("You took sen, draw and cut in one motion before %s move.", m_name);
        if (py_attack(m_ptr->fy, m_ptr->fx, HISSATSU_IAI)) return TRUE;
    }

    if ((p_ptr->special_defense & NINJA_KAWARIMI) && (randint0(55) < (p_ptr->lev*3/5+20)))
    {
        if (kawarimi(TRUE)) return TRUE;
    }
                           /* v---- Assume a beholder GAZE = {MST_BOLT, GF_ATTACK} */
    if (!retaliation_hack && !mon_spell_current())
        cmsg_format(TERM_GREEN, "%^s attacks you:", m_name);
    monster_desc(m_name, m_ptr, MD_PRON_VISIBLE);

    /* Assume no blink */
    blinked = FALSE;

    /* Scan through all four blows */
    nemesis_hack = FALSE;
    ht_cnt = 0;
    for (ap_cnt = 0; ap_cnt < MAX_MON_BLOWS; ap_cnt++)
    {
        bool         obvious = FALSE;
        mon_blow_ptr blow;
        int          blow_dam = 0; /* total physical damage for this blow */
        cptr         act = NULL;

        /* Revenge aura only gives a single retaliatory attempt per player strike
           We'll cycle thru monster attacks on each revenge strike, and the revenge
           will stop after the monster runs out of attacks. So 20 attack players need
           not fear insta-death (as much). All the hackery is communicated between
           here, py_attack_* and touch_zap_player(). Enjoy!
           (Note: If I had a mon_attack_p(int m_idx, int blow_idx), we could
           avoid all this nonsense ... )
         */
        if (retaliation_hack)
        {
            ap_cnt = retaliation_count;
            if (ap_cnt >= MAX_MON_BLOWS) return FALSE;
        }
        blow = &r_ptr->blows[ap_cnt];

        if (!m_ptr->r_idx) break;

        /* Call off the attacks on the Duelist's Nemesis power */
        if (nemesis_hack) break;

        /* Hack -- no more attacks */
        if (!blow->method) break;

        /* Stop if player is dead or gone (e.g. SHATTER knocks player back) */
        if (!p_ptr->playing || p_ptr->is_dead) break;
        if ((!mon_spell_current()) && ((py != opy) || (px != opx)) && (distance(py, px, m_ptr->fy, m_ptr->fx) > 1)) break;
        /*   ^--- Assume it is GAZE = {MST_BOLT, GF_ATTACK} from a beholder */

        /* Handle "leaving" */
        if (p_ptr->leaving) break;

        if (retaliation_hack)
        {
            if (m_ptr->ml)
                cmsg_format(TERM_GREEN, "(%^s retaliates:", m_name);
            mon_lore_2(m_ptr, RF2_AURA_REVENGE);
        }

        /* Calculate correct AC here (rather than in check_hit as formerly)
         * since AC is used to reduce melee damage. */
        ac = p_ptr->ac + p_ptr->to_a;
        if (p_ptr->pclass == CLASS_DUELIST && p_ptr->duelist_target_idx == m_idx)
            ac += 100;

        skill = blow->power + rlev*3;
        if (stun)
            skill -= skill*MIN(100, stun) / 150;

        /* Monster hits player */
        if ( !blow->effects[0].effect  /* XXX B:BEG or B:INSULT */
          || _check_hit(skill, ac))
        {
            int werewolf_hurt_max = 0;
            /* Always disturbing */
            disturb(1, 0);

            /* Hack -- Apply "protection from evil" */
            if (IS_PROT_EVIL() &&
                (r_ptr->flags3 & RF3_EVIL) &&
                !mon_save_p(m_ptr->r_idx, A_WIS) &&
                !one_in_(3))
            {
                mon_lore_3(m_ptr, RF3_EVIL);
                cmsg_format(TERM_L_BLUE, "%^s is repelled.", m_name);
                if (retaliation_hack) break;
                continue;
            }
            ht_cnt++;

            /* Describe the attack method */
            switch (blow->method)
            {
            case RBM_HIT:
                act = "hits";
                touched = TRUE;
                sound(SOUND_HIT);
                break;
            case RBM_TOUCH:
                act = "touches";
                touched = TRUE;
                sound(SOUND_TOUCH);
                break;
            case RBM_PUNCH:
                act = "punches";
                touched = TRUE;
                sound(SOUND_HIT);
                break;
            case RBM_KICK:
                act = "kicks";
                touched = TRUE;
                sound(SOUND_HIT);
                break;
            case RBM_CLAW:
                act = "claws";
                touched = TRUE;
                sound(SOUND_CLAW);
                break;
            case RBM_BITE:
                act = "bites";
                touched = TRUE;
                sound(SOUND_BITE);
                break;
            case RBM_STING:
                act = "stings";
                touched = TRUE;
                sound(SOUND_STING);
                break;
            case RBM_SLASH:
                act = "slashes";
                touched = TRUE;
                sound(SOUND_CLAW);
                break;
            case RBM_BUTT:
                act = "butts";
                touched = TRUE;
                sound(SOUND_HIT);
                break;
            case RBM_CRUSH:
                act = "crushes";
                touched = TRUE;
                sound(SOUND_CRUSH);
                break;
            case RBM_ENGULF:
                act = "engulfs";
                touched = TRUE;
                sound(SOUND_CRUSH);
                break;
            case RBM_CHARGE:
                act = "charges";
                touched = TRUE;
                sound(SOUND_BUY); /* Note! This is "charges", not "charges at". */
                break;
            case RBM_CRAWL:
                act = "crawls";
                touched = TRUE;
                sound(SOUND_SLIME);
                break;
            case RBM_DROOL:
                act = "drools";
                sound(SOUND_SLIME);
                break;
            case RBM_SPIT:
                act = "spits";
                sound(SOUND_SLIME);
                break;
            case RBM_EXPLODE:
                act = "explodes";
                explode = TRUE;
                break;
            case RBM_GAZE:
                act = "gazes";
                break;
            case RBM_WAIL:
                act = "wails";
                sound(SOUND_WAIL);
                break;
            case RBM_SPORE:
                act = "releases spores";
                sound(SOUND_SLIME);
                break;
            case RBM_XXX4:
                act = "projects XXX4's at you";
                break;
            case RBM_BEG:
                act = "begs";
                sound(SOUND_MOAN);
                break;
            case RBM_INSULT:
                act = desc_insult[randint0(m_ptr->r_idx == MON_DEBBY ? 10 : 8)];
                sound(SOUND_MOAN);
                break;
            case RBM_MOAN:
                act = desc_moan[randint0(4)];
                sound(SOUND_MOAN);
                break;
            case RBM_SHOW:
                if (m_ptr->r_idx == MON_JAIAN)
                    act = "horribly sings 'I AM GIAAAAAN. THE BOOOSS OF THE KIIIIDS.'";
                else
                {
                    if (one_in_(3))
                        act = "sings 'We are a happy family.'";
                    else
                        act = "sings 'I love you, you love me.'";
                }
                sound(SOUND_SHOW);
                break;
            }

            /* Message */
            if (act)
            {
                if (do_silly_attack)
                {
                    act = silly_attacks[randint0(MAX_SILLY_ATTACK)];
                }
                msg_format("%^s %s%s%s", m_name, act, do_silly_attack ? " you" : "", retaliation_hack ? ".<color:g>)</color>" : ".");
            }

            /* Hack -- assume all attacks are obvious */
            obvious = TRUE;

            /* XXX Wrt to explode, we now skip effect processing. When
             * the monster is killed below, monster_death() will project
             * the correct effect. This change is nastier than heng, but
             * simplifies processing (And heng relied on RBE_* codes that
             * no longer exist).  ~~~~~~~~~~~~~~~~~~~v */
            for (j = 0; j < MAX_MON_BLOW_EFFECTS && !explode; j++)
            {
                mon_effect_ptr effect = &blow->effects[j];
                int            effect_dam;

                if (!effect->effect) break;
                if (!p_ptr->playing || p_ptr->is_dead) break;
                if (nemesis_hack) break;
                if (p_ptr->leaving) break;
                if (effect->pct && randint1(100) > effect->pct) continue;

                effect_dam = damroll(effect->dd, effect->ds);
                effect_dam = effect_dam * m_ptr->mpower / 1000;
                if (stun)
                    effect_dam -= effect_dam*MIN(100, stun) / 150;

                if ( p_ptr->pclass == CLASS_DUELIST
                  && p_ptr->duelist_target_idx == m_idx )
                {
                    effect_dam -= effect_dam/3;
                }

                if ((effect_dam > 50) && (m_ptr->mflag & MFLAG_NICE)) /* Limit nice melee */
                {
                    effect_dam = 25 + (effect_dam / 2);
                }

                if (track_werewolf_dam) werewolf_hurt_max = MAX(werewolf_hurt_max, effect_dam / 2);

                switch (effect->effect)
                {
                case RBE_HURT: {
                    int pct = ac_melee_pct(ac);

                    obvious = TRUE;
                    effect_dam = effect_dam * pct / 100;
                    effect_dam = reduce_melee_dam_p(effect_dam);
                    blow_dam += take_hit(DAMAGE_ATTACK, effect_dam, ddesc);

                    break; }

                case RBE_DRAIN_CHARGES: {
                    u32b flgs[OF_ARRAY_SIZE];
                    char buf[MAX_NLEN];
                    bool drained = FALSE;
                    int  drain_amt = rlev; /* TODO: Consider using damage instead. Indeed, I nerfed this effect
                                              so all monsters should probably do some damage here as well. */

                    /* Take some damage */
                    effect_dam = reduce_melee_dam_p(effect_dam);
                    blow_dam += take_hit(DAMAGE_ATTACK, effect_dam, ddesc);

                    if (p_ptr->is_dead || CHECK_MULTISHADOW()) break;

                    /* Find an item */
                    for (k = 0; k < 10; k++)
                    {
                        slot_t  slot = pack_random_slot(NULL);
                        obj_ptr obj;

                        if (!slot) continue;
                        obj = pack_obj(slot);
                        if (!obj) continue;
                        if (!obj_is_device(obj)) continue;

                        obj_flags(obj, flgs);
                        if (have_flag(flgs, OF_HOLD_LIFE))
                        {
                            drained = TRUE; /* No food drain! */
                            break;
                        }

                        if (obj->tval == TV_ROD)
                            drain_amt /= 3;

                        if (drain_amt > device_sp(obj))
                            drain_amt = device_sp(obj);

                        obvious = TRUE;

                        if (p_ptr->no_charge_drain)
                        {
                            drained = TRUE; /* No food drain! */
                            break;
                        }

                        if (p_ptr->pclass == CLASS_DEVICEMASTER)
                        {
                            int pl = p_ptr->lev;
                            int dl = obj->activation.difficulty;

                            if (devicemaster_is_speciality(obj))
                                pl *= 2;

                            if (pl >= randint1(dl))
                            {
                                msg_print("Energy begins to drain from your pack ... But you pull it back!");
                                drained = TRUE; /* No food drain! */
                                break;
                            }
                        }

                        object_desc(buf, obj, OD_OMIT_PREFIX | OD_COLOR_CODED);
                        msg_format("Energy drains from your %s!", buf);
                        device_decrease_sp(obj, drain_amt);
                        drained = TRUE;

                        /* Heal the monster */
                        m_ptr->hp += drain_amt;
                        if (m_ptr->hp > m_ptr->maxhp)
                            m_ptr->hp = m_ptr->maxhp;

                        /* Redraw (later) if needed */
                        check_mon_health_redraw(m_idx);

                        /* Window stuff */
                        p_ptr->window |= (PW_INVEN);

                        /* Done */
                        break;
                    }

                    if ( !drained
                      && (((!(get_race()->flags & RACE_IS_NONLIVING))
                      && (!prace_is_(RACE_MON_JELLY)) ) || (prace_is_(RACE_EINHERI))))
                    {
                        msg_print("Food drains from your belly!");
                        set_food(MAX(0, MIN(p_ptr->food - 1000, p_ptr->food*2/3)));
                    }

                    break; }

                case RBE_EAT_GOLD:
                    /* Confused monsters cannot steal successfully. -LM-*/
                    if (MON_CONFUSED(m_ptr)) break;

                    if (p_ptr->is_dead || CHECK_MULTISHADOW()) break;

                    /* No stealing from lawyers - we still blink because 1) that's a signature
                     * move for thieves and 2) otherwise cockatrices become unexpectedly deadly
                     * to lawyers and ninja lawyers with no free action */
                    if ((p_ptr->pclass == CLASS_LAWYER) || (p_ptr->pclass == CLASS_NINJA_LAWYER))
                    { blinked = TRUE; obvious = TRUE; break; }

                    obvious = TRUE;

                    if (r_ptr->flags2 & RF2_THIEF)
                        mon_lore_2(m_ptr, RF2_THIEF);

                    if (!p_ptr->paralyzed &&
                        (randint0(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
                                  p_ptr->lev)))
                    {
                        msg_print("You quickly protect your money pouch!");
                        if (randint0(3)) blinked = TRUE;
                    }
                    else
                    {
                        gold = (p_ptr->au / 10) + randint1(25);
                        if (gold < 2) gold = 2;
                        if (gold > 5000) gold = (p_ptr->au / 20) + randint1(3000);
                        if (gold > p_ptr->au) gold = p_ptr->au;
                        p_ptr->au -= gold;
                        stats_on_gold_stolen(gold);
                        if (gold <= 0)
                        {
                            msg_print("Nothing was stolen.");

                        }
                        else if (p_ptr->au)
                        {
                            msg_print("Your purse feels lighter.");
                            msg_format("%d coins were stolen!", gold);
                            virtue_add(VIRTUE_SACRIFICE, 1);
                        }
                        else
                        {
                            msg_print("Your purse feels lighter.");
                            msg_print("All of your coins were stolen!");

                            virtue_add(VIRTUE_SACRIFICE, 2);
                        }

                        p_ptr->redraw |= (PR_GOLD);
                        if (prace_is_(RACE_MON_LEPRECHAUN))
                            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

                        blinked = TRUE;
                    }

                    break;

                case RBE_EAT_ITEM:

                    /* Confused monsters cannot steal successfully. -LM-*/
                    if (MON_CONFUSED(m_ptr)) break;

                    if (p_ptr->is_dead || CHECK_MULTISHADOW()) break;

                    /* No stealing from lawyers */
                    if ((p_ptr->pclass == CLASS_LAWYER) || (p_ptr->pclass == CLASS_NINJA_LAWYER)) 
                    { blinked = TRUE; obvious = TRUE; break; }

                    if (r_ptr->flags2 & RF2_THIEF)
                        mon_lore_2(m_ptr, RF2_THIEF);

                    if (p_ptr->tim_inven_prot2)
                    {
                        msg_print("Your inventory is protected!");
                        blinked = TRUE;
                        obvious = TRUE;
                        break;
                    }

                    if (!p_ptr->paralyzed &&
                        (randint0(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
                                  p_ptr->lev)))
                    {
                        msg_print("You grab hold of your backpack!");
                        blinked = TRUE;
                        obvious = TRUE;
                        break;
                    }

                    for (k = 0; k < 10; k++)
                    {
                        s16b    o_idx;
                        slot_t  slot = pack_random_slot(NULL);
                        obj_ptr obj;

                        if (!slot) continue;
                        obj = pack_obj(slot);

                        if (!obj) continue;
                        if (object_is_artifact(obj)) continue;

                        object_desc(o_name, obj, OD_OMIT_PREFIX);

                        msg_format("%sour %s %s stolen!",
                               ((obj->number > 1) ? "One of y" : "Y"),
                               o_name, have_flag(obj->flags, OF_PLURAL) ? "were" : "was");

                        virtue_add(VIRTUE_SACRIFICE, 1);

                        o_idx = o_pop();
                        if (o_idx)
                        {
                            object_type *j_ptr;
                            j_ptr = &o_list[o_idx];

                            object_copy(j_ptr, obj);

                            j_ptr->number = 1;
                            j_ptr->held_m_idx = m_idx;
                            j_ptr->next_o_idx = m_ptr->hold_o_idx;
                            m_ptr->hold_o_idx = o_idx;
                        }

                        if (((alert_device_gone) && (object_is_device(obj))) ||
                           ((alert_insc_gone) && (obj_is_inscribed(obj))))
                        {
                            msg_print(NULL); /* -more- prompt */
                        }

                        obj->number--;
                        obj_release(obj, OBJ_RELEASE_QUIET);

        
                        obvious = TRUE;
                        blinked = TRUE;
                        break;
                    }
                    break;

                case RBE_EAT_FOOD:
                    if (p_ptr->is_dead || CHECK_MULTISHADOW()) break;

                    if (r_ptr->flags2 & RF2_THIEF)
                        mon_lore_2(m_ptr, RF2_THIEF);

                    for (k = 0; k < 10; k++)
                    {
                        slot_t  slot = pack_random_slot(NULL);
                        obj_ptr obj;

                        if (!slot) continue;
                        obj = pack_obj(slot);
                        if (!obj) continue;
                        if (obj->tval != TV_FOOD && !(obj->tval == TV_CORPSE && obj->sval)) continue;
                        if (object_is_artifact(obj)) continue;

                        object_desc(o_name, obj, OD_OMIT_PREFIX | OD_NAME_ONLY | OD_COLOR_CODED);
                        msg_format("%sour %s was eaten!",
                               ((obj->number > 1) ? "One of y" : "Y"),
                               o_name);

                        obj->number--;
                        obj_release(obj, OBJ_RELEASE_QUIET);

                        obvious = TRUE;
                        break;
                    }
                    break;

                case RBE_EAT_LITE: {
                    int slot = equip_find_obj(TV_LITE, SV_ANY);

                    if (p_ptr->is_dead || CHECK_MULTISHADOW()) break;

                    if (slot)
                    {
                        o_ptr = equip_obj(slot);
                        if (o_ptr->xtra4 > 0 && !object_is_fixed_artifact(o_ptr))
                        {
                            o_ptr->xtra4 -= (250 + randint1(250));
                            if (o_ptr->xtra4 < 1) o_ptr->xtra4 = 1;

                            if (!p_ptr->blind)
                            {
                                msg_print("Your light dims.");
                                obvious = TRUE;
                            }
                            p_ptr->window |= PW_EQUIP;
                        }
                    }
                    break; }

                case RBE_LOSE_STR:
                    if (do_dec_stat(A_STR)) obvious = TRUE;
                    break;

                case RBE_LOSE_INT:
                    if (do_dec_stat(A_INT)) obvious = TRUE;
                    break;

                case RBE_LOSE_WIS:
                    if (do_dec_stat(A_WIS)) obvious = TRUE;
                    break;

                case RBE_LOSE_DEX:
                    if (do_dec_stat(A_DEX)) obvious = TRUE;
                    break;

                case RBE_LOSE_CON:
                    if (do_dec_stat(A_CON)) obvious = TRUE;
                    break;

                case RBE_LOSE_CHR:
                    if (do_dec_stat(A_CHR)) obvious = TRUE;
                    break;

                case RBE_LOSE_ALL:
                    if (do_dec_stat(A_STR)) obvious = TRUE;
                    if (do_dec_stat(A_DEX)) obvious = TRUE;
                    if (do_dec_stat(A_CON)) obvious = TRUE;
                    if (do_dec_stat(A_INT)) obvious = TRUE;
                    if (do_dec_stat(A_WIS)) obvious = TRUE;
                    if (do_dec_stat(A_CHR)) obvious = TRUE;
                    break;

                case RBE_SHATTER: {
                    int pct = ac_melee_pct(ac);

                    obvious = TRUE;
                    effect_dam = effect_dam * pct / 100;
                    effect_dam = reduce_melee_dam_p(effect_dam);
                    blow_dam += take_hit(DAMAGE_ATTACK, effect_dam, ddesc);

                    if (effect_dam > 23)
                        earthquake_aux(m_ptr->fy, m_ptr->fx, 8, m_idx);
                    break; }

                case RBE_DRAIN_EXP: {
                    int resist = 95 - effect_dam/15;
                    effect_dam += MON_DRAIN_LIFE * p_ptr->exp / 100;
                    if (effect_dam > 25000) effect_dam = 25000;
                    obvious = TRUE;
                    if (CHECK_MULTISHADOW()) break;

                    effect_dam = drain_exp(effect_dam, effect_dam/10, resist);
                    if (effect_dam)
                        mon_gain_exp(m_ptr, effect_dam);
                    break; }

                case RBE_DISEASE:
                    effect_dam = reduce_melee_dam_p(effect_dam);
                    blow_dam += take_hit(DAMAGE_ATTACK, effect_dam, ddesc);
                    if (p_ptr->is_dead || CHECK_MULTISHADOW()) break;

                    /* XXX should we do both immediate and delayed damage? */
                    {
                        int d = res_calc_dam(RES_POIS, effect_dam);
                        if (set_poisoned(p_ptr->poisoned + d, FALSE))
                            obvious = TRUE;
                    }

                    if (randint1(100) < 11 && p_ptr->prace != RACE_ANDROID)
                    {
                        bool perm = one_in_(10) && one_in_(100/MAX(1, rlev));
                        if (dec_stat(A_CON, randint1(10), perm))
                        {
                            msg_print("You feel strange sickness.");
                            obvious = TRUE;
                        }
                    }
                    else if ((one_in_(4)) && ((one_in_(4)) || (!res_save_default(RES_POIS))) 
                       && (p_ptr->prace != RACE_ANDROID))
                    {
                        if (p_ptr->unwell > UNWELL_EFFECTIVE_MAX) set_unwell(MAX(UNWELL_EFFECTIVE_MAX + 1, p_ptr->unwell - 10), TRUE);
                        else if (!p_ptr->unwell) set_unwell(75 + randint1(25), TRUE);
                    }
                    break;
                case RBE_VAMP:
                    obvious = TRUE;
                    /* Interpretation: BITE:VAMP(3d10) is still a physically
                     * damaging blow, even if the player is non-living. This
                     * effect is pure vamp, no longer combining exp drain. Do
                     * B:BITE:VAMP(3d10):DRAIN_EXP(40d6) if you want both. */
                    effect_dam = reduce_melee_dam_p(effect_dam);
                    effect_dam = take_hit(DAMAGE_ATTACK, effect_dam, ddesc);
                    blow_dam += effect_dam;
                    if (p_ptr->is_dead || CHECK_MULTISHADOW()) break;

                    if (!(get_race()->flags & RACE_IS_NONLIVING))
                    {
                        if (m_ptr->hp < m_ptr->maxhp)
                        {
                            m_ptr->hp += effect_dam;
                            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
                            check_mon_health_redraw(m_idx);
                            if (m_ptr->ml)
                                msg_format("%^s appears healthier.", m_name);
                        }
                    }
                    break;
                case RBE_CUT:
                    set_cut(p_ptr->cut + effect_dam, FALSE);
                    break;
                default: /* using GF_* ... damage 0 is OK: B:BITE:HURT(4d4):DISENCHANT */
                    effect_dam = reduce_melee_dam_p(effect_dam);
                    gf_affect_p(m_idx, effect->effect, effect_dam, GF_AFFECT_ATTACK);
                } /* switch (effect) */
                mon_lore_effect(m_ptr, effect);
            } /* for each effect */
            total_dam += blow_dam;
            if (explode)
            {
                sound(SOUND_EXPLODE);
                if (mon_take_hit(m_idx, m_ptr->hp + 1, &fear, NULL))
                {
                    blinked = FALSE;
                    alive = FALSE;
                }
            }
            else if (touched)
            {
                bool do_retaliate = FALSE;

                weaponmaster_do_readied_shot(m_ptr); /* XXX Bowmaster */

                possessor_do_auras(m_ptr);
                if (weaponmaster_get_toggle() == TOGGLE_TRADE_BLOWS)
                {
                    do_retaliate = TRUE;
                }
                else if (mystic_get_toggle() == MYSTIC_TOGGLE_RETALIATE && p_ptr->csp >= 7)
                {
                    do_retaliate = TRUE;
                }
                else if (p_ptr->sh_retaliation)
                {
                    if (p_ptr->prace == RACE_MON_SWORD) /* death scythe */
                        do_retaliate = one_in_(3);
                    else if (p_ptr->monk_lvl) /* monk */
                        do_retaliate = !mon_save_p(m_ptr->r_idx, A_DEX);
                    else /* cloak of retaliation or staffmaster */
                        do_retaliate = TRUE;
                }
                if (p_ptr->pclass == CLASS_FORCETRAINER)
                {
                    /* The Force-trainer need not see his enemy to retaliate.
                     * Indeed, it is a training excercise among novices to fight
                     * opponents while blindfolded! */
                    if (p_ptr->confused || p_ptr->paralyzed)
                        do_retaliate = FALSE;
                }
                else
                {
                    if (!m_ptr->ml || p_ptr->confused || p_ptr->blind || p_ptr->paralyzed)
                        do_retaliate = FALSE;
                }

                /* light stunning is now *very* common */
                if (p_ptr->stun && randint0(35) < p_ptr->stun)
                    do_retaliate = FALSE;

                if ( do_retaliate
                  && alive
                  && !retaliation_hack /* Otherwise, we get a retaliatory cycle!!! */
                  && !p_ptr->is_dead )
                {
                    if (weaponmaster_get_toggle() == TOGGLE_TRADE_BLOWS)
                        cmsg_print(TERM_L_UMBER, "(You trade blows:");
                    else
                        cmsg_print(TERM_L_UMBER, "(You retaliate:");

                    retaliation_hack = TRUE;
                    py_attack(m_ptr->fy, m_ptr->fx, WEAPONMASTER_RETALIATION);
                    retaliation_hack = FALSE;
                    if (!m_ptr->r_idx) /* Dead? */
                        alive = FALSE;
                    equip_learn_flag(OF_AURA_REVENGE);
                    cmsg_print(TERM_L_UMBER, ")");
                    if (mystic_get_toggle() == MYSTIC_TOGGLE_RETALIATE)
                        sp_player(-7);
                }

                if (p_ptr->tim_blood_revenge && alive && !p_ptr->is_dead && monster_living(r_ptr))
                {   /* Scale the damage based on cuts and monster deadliness */
                    int dam = blow_dam * p_ptr->cut / CUT_SEVERE;

                    /* Balance out a weak melee attack */
                    if (dam < p_ptr->cut / 10)
                        dam = p_ptr->cut / 10;

                    /* Not too powerful */
                    if (dam > 50)
                        dam = 50;

                    dam = mon_damage_mod(m_ptr, dam, FALSE);
                    if (dam > 0)
                    {
                        msg_format("%^s feels your bloody revenge!", m_name);
                        if (mon_take_hit(m_idx, dam, &fear,
                            " turns into a pool of blood."))
                        {
                            blinked = FALSE;
                            alive = FALSE;
                        }
                    }
                }

                if (p_ptr->sh_fire && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK))
                    {
                        int dam = _aura_dam_p(p_ptr->sh_fire);

                        /* Modify the damage */
                        dam = mon_damage_mod(m_ptr, dam, FALSE);

                        msg_format("%^s is <color:r>burned</color>!", m_name);

                        if (mon_take_hit(m_idx, dam, &fear,
                            " turns into a pile of ash."))

                        {
                            blinked = FALSE;
                            alive = FALSE;
                        }
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_EFF_IM_FIRE_MASK);
                    }
                }

                if (p_ptr->sh_elec && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK))
                    {
                        int dam = _aura_dam_p(p_ptr->sh_elec);

                        /* Modify the damage */
                        dam = mon_damage_mod(m_ptr, dam, FALSE);

                        msg_format("%^s is <color:b>zapped</color>!", m_name);

                        if (mon_take_hit(m_idx, dam, &fear,
                            " turns into a pile of cinder."))

                        {
                            blinked = FALSE;
                            alive = FALSE;
                        }
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_EFF_IM_ELEC_MASK);
                    }
                }

                if (p_ptr->sh_cold && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_EFF_IM_COLD_MASK))
                    {
                        int dam = _aura_dam_p(p_ptr->sh_cold);

                        /* Modify the damage */
                        dam = mon_damage_mod(m_ptr, dam, FALSE);

                        msg_format("%^s is <color:w>frozen</color>!", m_name);

                        if (mon_take_hit(m_idx, dam, &fear,
                            " was frozen."))

                        {
                            blinked = FALSE;
                            alive = FALSE;
                        }
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_EFF_IM_COLD_MASK);
                    }
                }

                /* by henkma */
                if ((p_ptr->dustrobe || p_ptr->sh_shards) && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_EFF_RES_SHAR_MASK))
                    {
                        int dam = _aura_dam_p(p_ptr->sh_shards + (p_ptr->dustrobe ? 1 : 0));

                        dam = mon_damage_mod(m_ptr, dam, FALSE);
                        msg_format("%^s is <color:u>shredded</color>!", m_name);
                        if (mon_take_hit(m_idx, dam, &fear," was torn to pieces."))
                        {
                            blinked = FALSE;
                            alive = FALSE;
                        }
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_EFF_RES_SHAR_MASK);
                    }

                    if (p_ptr->dustrobe && is_mirror_grid(&cave[py][px]))
                        teleport_player(10, 0L);
                }

                if (p_ptr->tim_sh_domination && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_RES_ALL))
                    {
                        int dam = (subjugation_power()+1)/2;
                        msg_format("%^s feels the force of your presence!", m_name);
                        gf_affect_m(GF_WHO_PLAYER, m_ptr, GF_SUBJUGATION, dam, GF_AFFECT_AURA);
                        if (MON_CSLEEP(m_ptr) || !is_hostile(m_ptr) || MON_MONFEAR(m_ptr))
                            break;
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_RES_ALL);
                    }
                }

                if (p_ptr->tim_sh_time && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_EFF_RES_TIME_MASK))
                    {
                        int dam = _aura_dam_p(1);
                        switch (randint1(3))
                        {
                        case 1:
                            msg_format("%^s gets <color:B>chronosmashed</color>!", m_name);
                            break;
                        case 2:
                            msg_format("%^s gets <color:B>flux capacitated</color>!", m_name);
                            break;
                        case 3:
                            msg_format("%^s <color:B>withers</color>!", m_name);
                            break;
                        }
                        gf_affect_m(GF_WHO_PLAYER, m_ptr, GF_TIME, dam, GF_AFFECT_AURA);
                        if (MON_PARALYZED(m_ptr)) /* frozen in time, so stop attacking me! */
                            break;
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_EFF_RES_TIME_MASK);
                    }
                }

                if (p_ptr->sh_fear && alive && !p_ptr->is_dead)
                {
                    gf_affect_m(GF_WHO_PLAYER, m_ptr, GF_TURN_ALL, 2*p_ptr->lev, GF_AFFECT_AURA);
                    if (MON_MONFEAR(m_ptr))
                        break;
                }

                if (p_ptr->tim_sh_holy && alive && !p_ptr->is_dead)
                {
                    if (r_ptr->flags3 & RF3_EVIL)
                    {
                        if (!(r_ptr->flagsr & RFR_RES_ALL))
                        {
                            int dam = _aura_dam_p(1);

                            /* Modify the damage */
                            dam = mon_damage_mod(m_ptr, dam, FALSE);

                            msg_format("%^s is injured by holy power!", m_name);

                            if (mon_take_hit(m_idx, dam, &fear,
                                " is destroyed."))
                            {
                                blinked = FALSE;
                                alive = FALSE;
                            }
                            mon_lore_3(m_ptr, RF3_EVIL);
                        }
                        else
                        {
                            mon_lore_r(m_ptr, RFR_RES_ALL);
                        }
                    }
                }

                if (p_ptr->tim_sh_touki && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_RES_ALL))
                    {
                        int dam = _aura_dam_p(1);

                        /* Modify the damage */
                        dam = mon_damage_mod(m_ptr, dam, FALSE);

                        msg_format("%^s is injured by the <color:B>Force</color>.", m_name);

                        if (mon_take_hit(m_idx, dam, &fear,
                            " is destroyed."))

                        {
                            blinked = FALSE;
                            alive = FALSE;
                        }
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_RES_ALL);
                    }
                }

                if (hex_spelling(HEX_SHADOW_CLOAK) && alive && !p_ptr->is_dead)
                {
                    if (!(r_ptr->flagsr & RFR_RES_ALL || r_ptr->flagsr & RFR_RES_DARK))
                    {
                        int dam = 1;
                        int slot, hand;

                        for (hand = 0; hand < MAX_HANDS; hand++)
                        {
                            object_type *o_ptr = NULL;
                            if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
                                o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
                            if (o_ptr)
                            {
                                int dd = o_ptr->dd + p_ptr->weapon_info[hand].to_dd;
                                int ds = o_ptr->ds + p_ptr->weapon_info[hand].to_ds;
                                dam = dd * (ds + 1) / 2 + o_ptr->to_d + p_ptr->weapon_info[hand].to_d;
                                break;
                            }
                        }
                        slot = equip_find_first(object_is_body_armour);
                        if (slot)
                        {
                            object_type *o_ptr = equip_obj(slot);
                            if (object_is_cursed(o_ptr))
                                dam *= 2;
                        }

                        dam = mon_damage_mod(m_ptr, dam, FALSE);

                        msg_format("Enveloped shadows attack %^s.", m_name);

                        if (mon_take_hit(m_idx, dam, &fear, " is destroyed."))
                        {
                            blinked = FALSE;
                            alive = FALSE;
                        }
                        else
                        {
                            for (slot = equip_find_first(object_is_cursed);
                                    slot;
                                    slot = equip_find_next(object_is_cursed, slot))
                            {
                                object_type *o_ptr = equip_obj(slot);
                                int          effect = 0;

                                switch (equip_slot_type(slot))
                                {
                                case EQUIP_SLOT_HELMET: effect = GF_OLD_CONF; break;
                                case EQUIP_SLOT_GLOVES: effect = GF_TURN_ALL; break;
                                case EQUIP_SLOT_BOOTS: effect = GF_OLD_SLOW; break;
                                default: if (object_is_shield(o_ptr)) effect = GF_OLD_SLEEP;
                                }
                                if (effect)
                                    gf_affect_m(GF_WHO_PLAYER, m_ptr, effect, p_ptr->lev*2, GF_AFFECT_AURA);
                            }
                        }
                    }
                    else
                    {
                        mon_lore_r(m_ptr, RFR_RES_ALL | RFR_RES_DARK);
                    }
                }
            }

            /* werewolf in contact with silver */
            if (werewolf_hurt_max)
            {
                werewolf_silver_effect(werewolf_hurt_max, TRUE);
            }
        }

        /* Monster missed player */
        else
        {
            /* Analyze failed attacks */
            switch (blow->method)
            {
            case RBM_HIT:
            case RBM_TOUCH:
            case RBM_PUNCH:
            case RBM_KICK:
            case RBM_CLAW:
            case RBM_BITE:
            case RBM_STING:
            case RBM_SLASH:
            case RBM_BUTT:
            case RBM_CRUSH:
            case RBM_ENGULF:
            case RBM_CHARGE:
            case RBM_CRAWL:
            case RBM_SPIT:
            case RBM_EXPLODE:
                if (m_ptr->ml)
                {
                    disturb(1, 0);
                    msg_format("%^s misses%s", m_name, retaliation_hack ? ".<color:g>)</color>" : ".");
                }
                break;
            case RBM_DROOL:
                if (m_ptr->ml)
                {
                    disturb(1, 0);
                    msg_format("%^s slobbers ineffectually%s", m_name, retaliation_hack ? ".<color:g>)</color>" : ".");
                }
                break;
            case RBM_WAIL:
                if (m_ptr->ml)
                {
                    disturb(1, 0);
                    msg_format("%^s wails ineffectually%s", m_name, retaliation_hack ? ".<color:g>)</color>" : ".");
                }
                break;
            case RBM_GAZE:
                if (m_ptr->ml)
                {
                    char tmp[MAX_NLEN];
                    disturb(1, 0);
                    monster_desc(tmp, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);
                    msg_format("You avoid %s gaze%s", tmp, retaliation_hack ? ".<color:g>)</color>" : ".");
                }
                break;
            }
        }


        {
            int options = 0;
            if (do_silly_attack) options |= MON_BLOW_SILLY;
            if (obvious) options |= MON_BLOW_OBVIOUS;
            if (blow_dam) options |= MON_BLOW_DAMAGE;
            mon_lore_blow(m_ptr, blow, options);
        }

        if (p_ptr->riding && blow_dam)
        {
            char m_name[80];
            monster_desc(m_name, &m_list[p_ptr->riding], 0);
            if (rakuba((blow_dam > 200) ? 200 : blow_dam, FALSE))
            {
                msg_format("You have fallen from %s.", m_name);
            }
        }

        if (p_ptr->special_defense & NINJA_KAWARIMI)
        {
            if (kawarimi(FALSE)) return TRUE;
        }

        if (retaliation_hack)
            break;
    }

    /* Hex - revenge damage stored */
    revenge_store(total_dam);

    if (IS_REVENGE()
        && total_dam > 0 && !p_ptr->is_dead)
    {
        char m_name_self[80];

        /* hisself */
        monster_desc(m_name_self, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE | MD_OBJECTIVE);

        /* This message can be triggered by an already dead exploder -
         * but in that case it's still pretty appropriate... */
        msg_format("%^s harms %s!", m_name, m_name_self);
        project(0, 0, m_ptr->fy, m_ptr->fx, psion_backlash_dam(total_dam), GF_MISSILE, PROJECT_KILL);
        if (p_ptr->tim_eyeeye) set_tim_eyeeye(p_ptr->tim_eyeeye-5, TRUE);
    }

    if ((p_ptr->counter || (p_ptr->special_defense & KATA_MUSOU)) && alive && !p_ptr->is_dead && m_ptr->ml && (p_ptr->csp > 7))
    {
        char m_name[80];
        monster_desc(m_name, m_ptr, 0);

        p_ptr->csp -= 7;
        msg_format("Your counterattack to %s!", m_name);
        py_attack(m_ptr->fy, m_ptr->fx, HISSATSU_COUNTER);
        fear = FALSE;

        /* Redraw mana */
        p_ptr->redraw |= (PR_MANA);
    }

    if (ht_cnt == 0 && !p_ptr->is_dead && allow_ticked_off(r_ptr) && one_in_(2))
    {
        m_ptr->anger = MIN(100, m_ptr->anger + 5 + m_ptr->anger / 3); 
    }

    /* Blink away */
    if (blinked && alive && !p_ptr->is_dead)
    {
        if (teleport_barrier(m_idx))
        {
            msg_print("The thief flees laughing...? But magic barrier obstructs it.");
        }
        else
        {
            msg_print("The thief flees laughing!");
            teleport_away(m_idx, MAX_SIGHT * 2 + 5, 0L);
        }
    }


    /* Always notice cause of death */
    if (p_ptr->is_dead && (r_ptr->r_deaths < MAX_SHORT) && !p_ptr->inside_arena)
    {
        r_ptr->r_deaths++;
    }

    if (m_ptr->ml && fear && alive && !p_ptr->is_dead)
    {
        sound(SOUND_FLEE);
        msg_format("%^s flees in terror!", m_name);

    }

    if (p_ptr->special_defense & KATA_IAI)
    {
        set_action(ACTION_NONE);
    }

    /* Assume we attacked */
    return TRUE;
}

