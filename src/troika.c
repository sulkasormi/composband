#include "angband.h"

typedef struct
{
    u16b goal_idx;
    u16b dungeon;
    u16b level;
    u16b danger_level;
    byte goal_ct;
    byte killed;
    byte start_lev;
    byte completed_lev;
    u32b completed_turn;
} dpl_quest;

#define MAX_TK_QUEST 15
#define MAX_TK_SPELL 20
#define MAX_TK_TIMEOUT 18
#define _MAX_PACK_SLOTS 12

static byte _q_idx = 0;
static dpl_quest _tk_quests[MAX_TK_QUEST];
static byte _my_spell_taso[2][MAX_TK_SPELL];
static byte _slay_timeouts[MAX_TK_TIMEOUT];
static char *_troika_names[3] = {"Guntujant", "Uxip", "Sohoglyth"};
static byte _sohoglyth_reward_level = 0;

#define REW_WRATH       1
#define REW_TY_CURSE    2
#define REW_HURT_LOT    3
#define REW_PISS_OFF    4
#define REW_CURSE_WP    5
#define REW_CURSE_AR    6
#define REW_LOSE_ABL    7
#define REW_RUIN_ABL    8
#define REW_LOSE_EXP    9
#define REW_SUMMON_M    10
#define REW_H_SUMMON    11
#define REW_AFC_WP      12
#define REW_STICKY      13
#define REW_BY_CURSE    14
#define REW_CURSE_EQ    15

#define REW_TROIKA_W    16
#define REW_HEAL_FUL    17
#define REW_DISPEL_C    18
#define REW_POLY_SLF    19
#define REW_GAIN_EXP    20
#define REW_POLY_WND    21
#define REW_DO_HAVOC    22
#define REW_DESTRUCT    23
#define REW_GENOCIDE    24
#define REW_MASS_GEN    25
#define REW_NO_BUFFS    26

#define REW_AUGM_ABL    27
#define REW_GOOD_OBJ    28
#define REW_SHARP_WP    29
#define REW_GREA_OBJ    30
#define REW_GOOD_OBS    31
#define REW_GREA_OBS    32
#define REW_GAIN_ABL    33

#define _MAX_PUN 22
#define _MAX_RND 17
#define _MAX_REW 15

int _troika_punishments[_MAX_PUN] =
{
    REW_WRATH, REW_WRATH, REW_TY_CURSE, REW_HURT_LOT, REW_PISS_OFF,
    REW_CURSE_AR, REW_CURSE_WP, REW_CURSE_AR, REW_CURSE_WP, REW_CURSE_AR,
    REW_CURSE_WP, REW_CURSE_WP, REW_CURSE_AR, REW_RUIN_ABL, REW_LOSE_ABL,
    REW_LOSE_EXP, REW_H_SUMMON, REW_SUMMON_M, REW_AFC_WP, REW_CURSE_EQ,
    REW_BY_CURSE, REW_STICKY
};
int _troika_random[_MAX_RND] =
{
    REW_SUMMON_M, REW_TROIKA_W, REW_POLY_WND, REW_POLY_WND, REW_NO_BUFFS,
    REW_POLY_SLF, REW_HEAL_FUL, REW_HEAL_FUL, REW_GAIN_ABL,
    REW_GAIN_EXP, REW_LOSE_EXP, REW_GOOD_OBJ, REW_DESTRUCT,
    REW_GENOCIDE, REW_MASS_GEN, REW_MASS_GEN, REW_DISPEL_C
};
int _troika_rewards[_MAX_REW] =
{
    REW_GOOD_OBJ, REW_GOOD_OBS, REW_TROIKA_W, REW_GREA_OBJ, REW_AUGM_ABL,
    REW_GREA_OBS, REW_GAIN_ABL, REW_GOOD_OBJ, REW_GOOD_OBS, REW_TROIKA_W,
    REW_GREA_OBJ, REW_AUGM_ABL, REW_GREA_OBS, REW_GAIN_ABL, REW_SHARP_WP
};

static int _slay_flags[MAX_TK_TIMEOUT] =
{
    OF_SLAY_ORC, OF_SLAY_ANIMAL, OF_SLAY_TROLL, OF_SLAY_HUMAN, OF_BRAND_FIRE,
    OF_BRAND_COLD, OF_BRAND_ACID, OF_BRAND_ELEC, OF_SLAY_DRAGON, OF_SLAY_GIANT,
    OF_SLAY_DEMON, OF_SLAY_UNDEAD, OF_VORPAL, OF_SLAY_EVIL, OF_BRAND_VAMP,
    OF_BRAND_CHAOS, OF_STUN, OF_BRAND_MANA
};

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_STR;
        me.encumbrance.max_wgt = 500;
        me.encumbrance.weapon_pct = 20;
        me.encumbrance.enc_wgt = 1200;
        me.min_fail = 0;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

static void _troika_ini_quests(void)
{
    int i;
    _q_idx = 0;
    _sohoglyth_reward_level = 0;
    for (i = 0; i < MAX_TK_QUEST; i++)
    {
        _tk_quests[i].goal_idx = 0;
        _tk_quests[i].dungeon = 0;
        _tk_quests[i].level = 0;
        _tk_quests[i].danger_level = 0;
        _tk_quests[i].goal_ct = 0;
        _tk_quests[i].killed = 0;
        _tk_quests[i].start_lev = 0;
        _tk_quests[i].completed_lev = 0;
        _tk_quests[i].completed_turn = 0;
    }
}

static void _troika_ini_spells(void)
{
    int i;
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        _my_spell_taso[0][i] = 0;
        _my_spell_taso[1][i] = 0;
    }
}

static int _locate_slay_flag(int which)
{
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if (_slay_flags[i] == which) return i;
    }
    return -1;
}

bool troika_timeout_flag(int which)
{
    if (_slay_timeouts[_locate_slay_flag(which)] > 0) return TRUE;
    return FALSE;
}

static void _inc_purple_timeout(int which, int delta)
{
    int vanha, uusi, mika = _locate_slay_flag(which);
    if (mika < 0) return;
    vanha = _slay_timeouts[mika];
    uusi = vanha + delta;
    if (uusi < 0) uusi = 0;
    else if (uusi > 150) uusi = 150;
    _slay_timeouts[mika] = uusi;
    if ((vanha > 0) && (uusi > 0)) return;
    if ((!vanha) && (!uusi)) return;
    if (uusi)
    {
        switch (which)
        {
             case OF_SLAY_ORC: msg_print("You become a great bane of orcs!"); break;
             case OF_SLAY_ANIMAL: msg_print("You become a great bane of animals!"); break;
             case OF_SLAY_TROLL: msg_print("You become a great bane of trolls!"); break;
             case OF_SLAY_HUMAN: msg_print("You become a great bane of humans!"); break;
             case OF_SLAY_GIANT: msg_print("You become a great bane of giants!"); break;
             case OF_SLAY_DRAGON: msg_print("You become a great bane of dragons!"); break;
             case OF_SLAY_DEMON: msg_print("You become a great bane of demons!"); break;
             case OF_SLAY_UNDEAD: msg_print("You become a great bane of the undead!"); break;
             case OF_SLAY_EVIL: msg_print("You smite evil monsters with great power!"); break;
             case OF_BRAND_FIRE: msg_print("Your weapons are coated in flames!"); break;
             case OF_BRAND_COLD: msg_print("Your weapons are coated in frost!"); break;
             case OF_BRAND_ACID: msg_print("Your weapons are coated in acid!"); break;
             case OF_BRAND_POIS: msg_print("Your weapons are coated in venom!"); break;
             case OF_BRAND_ELEC: msg_print("Your weapons crackle with electricity!"); break;
             case OF_BRAND_VAMP: msg_print("Your weapons become vampiric!"); break;
             case OF_BRAND_CHAOS: msg_print("Your weapons become chaotic!"); break;
             case OF_VORPAL: msg_print("Your weapons suddenly feel very sharp!"); break;
             case OF_STUN: msg_print("Your weapons become stunningly stunning!"); break;
             default: break; /* ?? */
        }
    }
    else
    {
        switch (which)
        {
             case OF_SLAY_ORC: msg_print("You are no longer a great bane of orcs."); break;
             case OF_SLAY_ANIMAL: msg_print("You are no longer a great bane of animals."); break;
             case OF_SLAY_TROLL: msg_print("You are no longer a great bane of trolls."); break;
             case OF_SLAY_HUMAN: msg_print("You are no longer a great bane of humans."); break;
             case OF_SLAY_GIANT: msg_print("You are no longer a great bane of giants."); break;
             case OF_SLAY_DRAGON: msg_print("You are no longer a great bane of dragons."); break;
             case OF_SLAY_DEMON: msg_print("You are no longer a great bane of demons."); break;
             case OF_SLAY_UNDEAD: msg_print("You are no longer a great bane of the undead."); break;
             case OF_SLAY_EVIL: msg_print("You no longer smite evil monsters with great power."); break;
             case OF_BRAND_FIRE: msg_print("The flames around your weapons disappear."); break;
             case OF_BRAND_COLD: msg_print("Your weapons are no longer coated in frost."); break;
             case OF_BRAND_ACID: msg_print("Your weapons are no longer coated in acid."); break;
             case OF_BRAND_POIS: msg_print("Your weapons are no longer coated in venom."); break;
             case OF_BRAND_ELEC: msg_print("Your weapons no longer crackle with electricity."); break;
             case OF_BRAND_VAMP: msg_print("Your weapons are no longer vampiric."); break;
             case OF_BRAND_CHAOS: msg_print("Your weapons are no longer chaotic."); break;
             case OF_VORPAL: msg_print("Your weapons suddenly feel less sharp."); break;
             case OF_STUN: msg_print("Your weapons no longer stun enemies."); break;
             default: break; /* ?? */
        }
    }
    p_ptr->redraw |= PR_STATUS;
}

void troika_wipe_timeouts(void)
{
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++) _slay_timeouts[i] = 0;
}

static void _birth(void)
{
    disciple_birth(); 
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_CHAIN_MAIL, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CONFUSION, 2);
    _troika_ini_quests();
    _troika_ini_spells();
    troika_wipe_timeouts();
}

static void _make_troika_weapon(int sval)
{
    int k_idx = lookup_kind(TV_SWORD, sval);
    object_type forge;
    if (k_idx < 1) return;
    object_prep(&forge, k_idx);
    apply_magic_ego = EGO_WEAPON_TROIKA;
    apply_magic(&forge, (p_ptr->max_plv * 5 / 3) + 8, AM_GOOD | AM_GREAT | AM_FORCE_EGO);
    object_origins(&forge, ORIGIN_PATRON);
    drop_near(&forge, -1, py, px);
}

static void _troika_event(int effect)
{
    char wrath_reason[24] = "the Wrath of Guntujant";
    int count = 0, dummy;

    if (p_ptr->is_dead) return;

    switch (effect)
    {
        case REW_POLY_SLF:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thou needst a new form, mortal!'");

            do_poly_self();
            break;
        case REW_GAIN_EXP:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Well done, mortal! Lead on!'");
            if (p_ptr->prace == RACE_ANDROID)
                msg_print("But, nothing happens.");
            else if (p_ptr->exp < PY_MAX_EXP)
            {
                s32b ee = (p_ptr->exp / 2) + 10;
                if (ee > 100000L) ee = 100000L;
                msg_print("You feel more experienced.");
                gain_exp(ee);
            }
            break;
        case REW_LOSE_EXP:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thou didst not deserve that, slave.'");

            if (p_ptr->prace == RACE_ANDROID)
            {
                nonlethal_ty_substitute(TRUE);
            }
            else
            {
                lose_exp(p_ptr->exp / 6);
            }
            break;
        case REW_GOOD_OBJ:
            msg_format("The voice of %s whispers:",_troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Use my gift wisely.'");
            acquirement(py, px, 1, FALSE, FALSE, ORIGIN_PATRON);
            break;
        case REW_GREA_OBJ:
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Use my gift wisely.'");

            acquirement(py, px, 1, TRUE, FALSE, ORIGIN_PATRON);
            break;
        case REW_TROIKA_W:
        {
            int dummy2;
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thy deed hath earned thee a worthy blade.'");
            switch (randint1(p_ptr->lev - (p_ptr->lev / 5)))
            {
                case 0: case 1:
                    dummy2 = SV_DAGGER;
                    break;
                case 2: case 3:
                    dummy2 = SV_MAIN_GAUCHE;
                    break;
                case 4:
                    dummy2 = SV_TANTO;
                    break;
                case 5: case 6:
                    dummy2 = SV_RAPIER;
                    break;
                case 7: case 8:
                    dummy2 = SV_SMALL_SWORD;
                    break;
                case 9: case 10:
                    dummy2 = SV_BASILLARD;
                    break;
                case 11: case 12: case 13:
                    dummy2 = SV_SHORT_SWORD;
                    break;
                case 14: case 15:
                    dummy2 = SV_SABRE;
                    break;
                case 16: case 17:
                    dummy2 = SV_CUTLASS;
                    break;
                case 18:
                    dummy2 = SV_WAKIZASHI;
                    break;
                case 19:
                    dummy2 = SV_KHOPESH;
                    break;
                case 20:
                    dummy2 = SV_TULWAR;
                    break;
                case 21:
                    dummy2 = SV_BROAD_SWORD;
                    break;
                case 22: case 23:
                    dummy2 = SV_LONG_SWORD;
                    break;
                case 24: case 25:
                    dummy2 = SV_SCIMITAR;
                    break;
                case 26:
                    dummy2 = SV_NINJATO;
                    break;
                case 27:
                    dummy2 = SV_KATANA;
                    break;
                case 28: case 29:
                    dummy2 = SV_BASTARD_SWORD;
                    break;
                case 30:
                    dummy2 = SV_GREAT_SCIMITAR;
                    break;
                case 31:
                    dummy2 = SV_CLAYMORE;
                    break;
                case 32:
                    dummy2 = SV_ESPADON;
                    break;
                case 33:
                    dummy2 = SV_TWO_HANDED_SWORD;
                    break;
                case 34:
                    dummy2 = SV_FLAMBERGE;
                    break;
                case 35:
                    dummy2 = SV_NO_DACHI;
                    break;
                case 36:
                    dummy2 = SV_EXECUTIONERS_SWORD;
                    break;
                case 37:
                    dummy2 = SV_ZWEIHANDER;
                    break;
                case 38:
                    dummy2 = SV_HAYABUSA;
                    break;
                case 39:
                    dummy2 = SV_BLADE_OF_CHAOS;
                    break;
                default:
                    dummy2 = (one_in_(3)) ? SV_DIAMOND_EDGE : SV_BLADE_OF_CHAOS;
                    break;
            }
            _make_troika_weapon(dummy2);
            break;
        }
        case REW_GOOD_OBS:
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thy deed hath earned thee a worthy reward.'");

            acquirement(py, px, randint1(2) + 1, FALSE, FALSE, ORIGIN_PATRON);
            break;
        case REW_GREA_OBS:
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Behold, mortal, how generously I reward thy loyalty.'");

            acquirement(py, px, randint1(2) + 1, TRUE, FALSE, ORIGIN_PATRON);
            break;
        case REW_TY_CURSE:
            msg_format("The voice of %s thunders:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thou art growing arrogant, mortal.'");

            if (p_ptr->lev < 30) nonlethal_ty_substitute(TRUE);
            else activate_ty_curse(FALSE, &count);
            break;
        case REW_BY_CURSE:
            msg_format("The voice of %s thunders:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thou art growing arrogant, mortal.'");

            nonlethal_ty_substitute(TRUE);
            break;
        case REW_SUMMON_M:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'My pets, destroy the arrogant mortal!'");
            for (dummy = 0; dummy < randint1(5) + 1; dummy++)
                summon_specific(0, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
            break;
        case REW_H_SUMMON:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thou needst worthier opponents!'");
            activate_hi_summon(py, px, FALSE);
            break;
        case REW_DO_HAVOC:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Death and destruction! This pleaseth me!'");
            call_chaos(100);
            break;
        case REW_GAIN_ABL:
            msg_format("The voice of %s rings out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Stay, mortal, and let me mold thee.'");
                do_inc_stat(randint0(6));
            break;
        case REW_LOSE_ABL:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'I grow tired of thee, mortal.'");
                do_dec_stat(randint0(6));
            break;
        case REW_RUIN_ABL:
            msg_format("The voice of %s thunders:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Thou needst a lesson in humility, mortal!'");
            msg_print("You feel less powerful!");

            for (dummy = 0; dummy < 6; dummy++)
                dec_stat(dummy, 10 + randint1(15), TRUE);
            break;
        case REW_POLY_WND:
            msg_format("You feel the power of %s touch you.", _troika_names[0]);
            do_poly_wounds();
            break;
        case REW_AUGM_ABL:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Receive this modest gift from me!'");
            for (dummy = 0; dummy < 6; dummy++)
                do_inc_stat(dummy);
            break;
        case REW_HURT_LOT:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Suffer, pathetic fool!'");
            fire_ball(GF_DISINTEGRATE, 0, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), 4);
            take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), wrath_reason);
            break;
       case REW_HEAL_FUL:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Rise, my servant!'");
            restore_level();
            set_poisoned(0, TRUE);
            set_blind(0, TRUE);
            set_confused(0, TRUE);
            set_image(0, TRUE);
            set_stun(0, TRUE);
            set_cut(0, TRUE);
            hp_player(5000);
            for (dummy = 0; dummy < 6; dummy++)
                do_res_stat(dummy);
            break;
        case REW_CURSE_WP:
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot)
            {
                msg_format("The voice of %s booms out:",
                    _troika_names[0]);
                cmsg_print(TERM_VIOLET, "'Thou reliest too much on thy weapon.'");
                curse_weapon(FALSE, slot);
            }
            else nonlethal_ty_substitute(TRUE);
            break;
        }
        case REW_SHARP_WP:
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot)
            {
                if (one_in_(4))
                {
                    object_type *o_ptr = equip_obj(slot);
                    if ((o_ptr) && (o_ptr->k_idx))
                    {
                         u32b flgs[OF_ARRAY_SIZE];
                         obj_flags(o_ptr, flgs);
                         if (!have_flag(flgs, OF_VORPAL))
                         {
                             msg_format("The voice of %s booms out:", _troika_names[0]);
                             cmsg_print(TERM_VIOLET, "'Thou needst a sharper weapon!'");
                             add_flag(o_ptr->flags, OF_VORPAL);
                             add_flag(o_ptr->known_flags, OF_VORPAL);
                         }
                    }
                }
                else
                {
                    msg_format("The voice of %s booms out:", _troika_names[0]);
                    cmsg_print(TERM_VIOLET, "'Thou needst a sharper weapon!'");
                    _inc_purple_timeout(OF_VORPAL, 50);
                }
            }
            break;
        }
        case REW_AFC_WP:
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if ((o_ptr) && (o_ptr->k_idx) && (!(o_ptr->curse_flags & OFC_TY_CURSE)))
                {
                    bool _perm = (one_in_(4));
                    if (_perm)
                    {
                        add_flag(o_ptr->flags, OF_TY_CURSE);
                    }
                    o_ptr->curse_flags |= (OFC_HEAVY_CURSE | OFC_TY_CURSE | OFC_CURSED);
                    o_ptr->known_curse_flags |= (OFC_HEAVY_CURSE | OFC_TY_CURSE | OFC_CURSED);
                    msg_format("The voice of %s booms out:", _troika_names[0]);
                    cmsg_format(TERM_VIOLET, "'I pronounce this curse of blood upon thy weapon; that it shall %s a beacon of hope to thine enemies, and betray thee in thy need.'", _perm ? "always be" : "be");
                    p_ptr->update |= PU_BONUS;
                }
            }
            break;
        }
        case REW_STICKY:
        {
            int slot = equip_random_slot(obj_exists);
            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if ((o_ptr) && (o_ptr->k_idx) && (!(o_ptr->curse_flags & OFC_PERMA_CURSE)))
                {
                    char o_name[MAX_NLEN];
                    strip_name(o_name, o_ptr->k_idx);
                    if ((o_ptr->name2 == EGO_SPECIAL_BLASTED) && (!one_in_(13))) /* Let's be nice and not perma-curse blasted equipment... */
                    {
                        msg_format("The voice of %s booms out:", _troika_names[0]);
                        cmsg_print(TERM_VIOLET, "'Suffer, pathetic mortal!'");
                        take_hit(DAMAGE_LOSELIFE, MIN(p_ptr->mhp * 2 / 3, p_ptr->lev * 4), wrath_reason);
                        nonlethal_ty_substitute(TRUE);
                        break;
                    }
                    o_ptr->curse_flags |= (OFC_PERMA_CURSE | OFC_HEAVY_CURSE | OFC_CURSED);
                    msg_format("The voice of %s booms out:", _troika_names[0]);
                    cmsg_format(TERM_VIOLET, "'I pronounce this curse upon thee, mortal: that thou shalt %s this same %s all thy life, even unto the end of thy days, and mayest never unequip it.'", object_is_weapon(o_ptr) ? "wield" : "wear", o_name);
                    p_ptr->update |= PU_BONUS;
                }
            }
            break;
        }
        case REW_CURSE_AR:
        {
            int slot = equip_random_slot(object_is_armour);
            if (slot)
            {
                msg_format("The voice of %s booms out:",
                    _troika_names[0]);
                cmsg_print(TERM_VIOLET, "'Thou reliest too much on thine equipment.'");
                curse_armor(slot);
            }
            else nonlethal_ty_substitute(TRUE);
            break;
        }
        case REW_CURSE_EQ:
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'A curse be upon thine equipment!'");
            curse_equipment(100, 50);
            break;
        case REW_PISS_OFF:
            msg_format("The voice of %s whispers:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Now thou shalt pay for annoying me.'");
            switch (randint1(4))
            {
                case 1:
                    if ((p_ptr->lev < 39) || (one_in_(2))) nonlethal_ty_substitute(TRUE);
                    else activate_ty_curse(FALSE, &count);
                    break;
                case 2:
                    activate_hi_summon(py, px, FALSE);
                    break;
                case 3:
                    if (one_in_(2))
                    {
                        int slot = equip_random_slot(object_is_melee_weapon);
                        if (slot)
                            curse_weapon(FALSE, slot);
                        else
                            nonlethal_ty_substitute(TRUE);
                    }
                    else
                    {
                        int slot = equip_random_slot(object_is_armour);
                        if (slot)
                            curse_armor(slot);
                        else
                            nonlethal_ty_substitute(TRUE);
                    }
                    break;
                default:
                    for (dummy = 0; dummy < 6; dummy++)
                        dec_stat(dummy, 10 + randint1(15), TRUE);
                    break;
            }
            break;
        case REW_WRATH:
            msg_format("The voice of %s thunders:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Die, mortal!'");

            take_hit(DAMAGE_LOSELIFE, MIN(p_ptr->mhp * 2 / 3, p_ptr->lev * 4), wrath_reason);
            if (p_ptr->chp < 0) break; /* We've probably done enough */
            for (dummy = 0; dummy < 6; dummy++)
                dec_stat(dummy, 10 + randint1(15), FALSE);
            activate_hi_summon(py, px, FALSE);
            if ((p_ptr->lev < 39) || (!one_in_(8))) nonlethal_ty_substitute(TRUE);
            else activate_ty_curse(FALSE, &count);
            if (one_in_(2))
            {
                int slot = equip_random_slot(object_is_melee_weapon);
                if (slot)
                    curse_weapon(FALSE, slot);
            }
            if (one_in_(2))
            {
                int slot = equip_random_slot(object_is_armour);
                if (slot)
                    curse_armor(slot);
            }
            break;
        case REW_NO_BUFFS:
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Normality! This pleaseth me!'");
            reset_tim_flags();
            break;
        case REW_DESTRUCT:
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Death and destruction! This pleaseth me!'");
            destroy_area(py, px, 25, 3 * p_ptr->lev);
            break;
        case REW_GENOCIDE:
            msg_format("The voice of %s booms out:",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Let me relieve thee of thine oppressors!'");
            symbol_genocide(0, FALSE);
            break;
        case REW_MASS_GEN:
            msg_format("The voice of %s booms out:", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "'Let me relieve thee of thine oppressors!'");
            mass_genocide(0, FALSE);
            break;
        case REW_DISPEL_C:
            msg_format("You feel the power of %s assault your enemies!", _troika_names[0]);
            dispel_monsters(p_ptr->lev * 4);
            break;
/*        case REW_IGNORE:
            msg_format("%s ignores you.", _troika_names[0]);
            break;*/
        default:
            msg_format("The voice of %s stammers:", _troika_names[0]);
            msg_format("'Uh... uh... the answer's %d, what's the question?'", effect);
            break;
    }
}

static void _troika_punish(void)
{
    int type = randint0(_MAX_PUN);
    if ((type == _MAX_PUN - 1) && (one_in_(2))) type = randint0(_MAX_PUN);
    _troika_event(_troika_punishments[type]);
}
static void _troika_random_effect(void)
{
    int type = randint0(_MAX_RND);
    _troika_event(_troika_random[type]);
}

static void _troika_reward(void)
{
    int type = randint0(_MAX_REW * 5 / 3);
    if (p_ptr->is_dead) return;
    if (type >= _MAX_REW)
    {
        int count;
        count = mut_count(mut_unlocked_pred);
        if (count>randint1(5))
        {
            if (one_in_(3))
            {
                int newmuts = 1;
                cmsg_format(TERM_VIOLET, "%s rewards you with new mutations!", _troika_names[0]);
                mut_gain_random(NULL);
                while (one_in_(newmuts)) 
                {
                    mut_lose_random(NULL);
                    mut_gain_random(NULL);
                    newmuts++;
                }
            }
            else
            {
                cmsg_format(TERM_VIOLET, "%s rewards you with a new mutation!", _troika_names[0]);
                mut_gain_random(NULL);
            }
        }
        else
        {
            {
                cmsg_format(TERM_VIOLET, "%s rewards you with a mutation!", _troika_names[0]);
                mut_gain_random(NULL);
            }
        }
    }
    else 
    {
        _troika_event(_troika_rewards[type]);
    }
}

void troika_effect(int reason)
{
    int punish_chance = 0;
    int reward_chance = 0;
    if (reason) switch (reason)
    {
    case TROIKA_HIT:
        if (one_in_(343)) /* 343 */
        {
            punish_chance = 39; /* 39 */
            reward_chance = 21; /* 21 */
        }
        break;
    case TROIKA_KILL_WEAK:
        if (one_in_(216))
        {
            punish_chance = 13;
            reward_chance = 169;
        }
        break;
    case TROIKA_KILL:
        if (one_in_(81))
        {
            punish_chance = 39;
            reward_chance = 21;
        }
        break;
    case TROIKA_KILL_UNIQUE:
        if (one_in_(7))
        {
            punish_chance = 39;
            reward_chance = 7;
        }
        break;
    case TROIKA_KILL_FAMOUS:
        if (one_in_(2))
        {
            punish_chance = 666;
            reward_chance = 3;
        }
        break;
    case TROIKA_KILL_GOOD:
        if (one_in_(7))
        {
            punish_chance = 666;
            reward_chance = 3;
        }
        break;
    case TROIKA_KILL_DEMON:
        if (one_in_(27))
        {
            punish_chance = 13;
            reward_chance = 7;
        }
        break;
    case TROIKA_CAST:
        if (one_in_(131))
        {
            punish_chance = 39;
            reward_chance = 21;
        }
        break;
    case TROIKA_VILLAINY:
        if (one_in_(21))
        {
            punish_chance = 39;
            reward_chance = 7;
        }
        break;
    case TROIKA_CHANCE:
        if (one_in_(14))
        {
            punish_chance = 39;
            reward_chance = 7;
        }
        break;
    case TROIKA_TAKE_HIT:
        if (one_in_(7))
        {
            punish_chance = 39;
            reward_chance = 21;
        }
        break;
    case TROIKA_TELEPORT:
        if ((one_in_(7)) && (p_ptr->chp > p_ptr->mhp / 4) && (p_ptr->chp != p_ptr->mhp))
        {
            punish_chance = (randint1(100) < (p_ptr->chp * 100 / p_ptr->mhp)) ? 2 : 77;
            reward_chance = 28;
        }
        break;
    default:
        break;
    }
    if (punish_chance && reward_chance) 
    {
        if (one_in_(punish_chance))
        {
            _troika_punish();
        }
        else if (one_in_(reward_chance))
        {
            _troika_reward();
        }
        else
        {
            _troika_random_effect();
        }
    }
}

static void _gain_level(int new_level)
{
    if (new_level > 1)
    {
        if (one_in_(2))
        {
            _troika_reward();
        }
        else if (one_in_(13))
        {
            _troika_punish();
        }
        else
        {
            _troika_random_effect();
        }
    }
}

static void _troika_quest_cleanup(quest_ptr q)
{
    q->status = QS_UNTAKEN;
    q->level = 0;
    q->completed_lev = 0;
    q->completed_turn = 0;
    q->goal_current = 0;
    q->goal_idx = 0;
    q->goal_count = 0;
}

void troika_punish_quest_fail(void)
{
    msg_print("A distant voice roars:");
    cmsg_print(TERM_VIOLET, "'Such cowardice is an abomination unto Uxip!'");
    take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 6, p_ptr->mhp / 2), "the Wrath of Uxip");
    switch (randint0(5))
    {
        case 0:
        case 1:
            _troika_event(REW_CURSE_WP);
            break;
        case 2:
        case 3:
            _troika_event(REW_CURSE_AR);
            break;
        default:
            _troika_event(REW_STICKY);
            break;
    }
}

static void _give_reward(void)
{
    int voitot = 0, tappiot = 0, hyvaksy, i;
    for (i = 0; i < _q_idx - 1; i++)
    {
        if ((_tk_quests[i].completed_turn) && (_tk_quests[i].killed == _tk_quests[i].goal_ct)) voitot++;
        else tappiot++;
    }
    hyvaksy = (_q_idx / 5) - tappiot;
    if (hyvaksy > _sohoglyth_reward_level)
    {
        _sohoglyth_reward_level++;
        msg_format("The voice of %s fills the dungeon:", _troika_names[2]);
        switch (_sohoglyth_reward_level)
        {
            case 1:
                cmsg_print(TERM_VIOLET, "'Thou hast proved thyself in combat this day, and faced great dangers in our service and prevailed. Behold now thy reward: I shall grant thee divine sight, that thou mightest perceive the vile spirits that do inhabit this dungeon, even as they approach, and no enemy can take thee by surprise.'");
                break;
            case 2:
                if (p_ptr->personality != PERS_MUNCHKIN) cmsg_print(TERM_VIOLET, "'Thou art worthy among our servants, and thy deeds this day have earned thee a great reward. Behold, this is my gift to thee: thou canst now perceive the quality of items without touch, even from the far side of the cave, to all the world as if thou wert a Munchkin.'");
                else cmsg_print(TERM_VIOLET, "'We had a great reward and a nice flowery speech prepared for you, but you don't really need it, do you?'");
                break;
            case 3:
                if (!prace_is_(RACE_SPECTRE)) cmsg_print(TERM_VIOLET, "'Truly, thou art our true Disciple, and stand alone in thy greatness among all mortals, a hero of legend. This, then, shall be thy reward; that thou needst suffer no longer the limitations of mortal life, but mayest pass through solid rock, e'en as if thou wert an insubstantial ghost, and never be held back unless it be by permanent walls.'");
                else cmsg_print(TERM_VIOLET, "'We had a great reward and a nice flowery speech prepared for you, but you don't really need it, do you?");
                break;
            default:
                cmsg_print(TERM_VIOLET, "'Something seems to be wrong...'");
                break;
        }
        p_ptr->update |= PU_BONUS;
    }
    else
    {
        int maali = (_sohoglyth_reward_level + 1) * 5 + (tappiot * 4);
        int tarve = MAX(1, maali - voitot - 1);
        if (maali > MAX_TK_QUEST - tappiot) return;
        msg_format("The voice of %s rings out:", _troika_names[2]);
        if (tarve == 1)
        {
            cmsg_print(TERM_VIOLET, "'Well done, mortal! Complete one more mission, and I shall give thee a great reward.'");
        }
        else cmsg_format(TERM_VIOLET, "'Well done, mortal! Complete now %d more quests, and thou wilt be greatly rewarded.'", tarve);
    }
}

void troika_quest_finished(quest_ptr q, bool success)
{
    int _tq = _q_idx - 1;
    _tk_quests[_tq].killed = q->goal_current;
    if (success) _tk_quests[_tq].killed = q->goal_count; /* paranoia */
    _tk_quests[_tq].completed_lev = (prace_is_(RACE_ANDROID)) ? p_ptr->lev : p_ptr->max_plv;
    _tk_quests[_tq].completed_turn = game_turn;
    _troika_quest_cleanup(q);
    if (!success)
    {
        msg_format("The voice of %s whispers: <color:v>Didst thou fail thy task on purpose? We have set thee above all other mortals, we have given thee might and powers for which most men would give their souls... is this how thou showest thy gratitude?</color>", _troika_names[2]);
        return;
    }
    else
    {
        /* No need to create stairs because purple quests allow normal stair generation
         * (indeed, generating stairs would be risky because purple quests can appear
         * at the bottom of a dungeon) */
        cmsg_print(TERM_L_BLUE, "You just completed your quest!");
        msg_add_tiny_screenshot(50, 24);
        p_ptr->redraw |= PR_DEPTH;
        _give_reward();
    }
}

quest_ptr troika_get_quest(int dungeon, int level)
{
    point_t tbl[2] = { {26, 134}, {50, 182} };
    int mahis = (no_wilderness ? 5 : 9);
    int verrokki = isompi(p_ptr->max_plv + 5, (p_ptr->max_plv * interpolate(p_ptr->max_plv, tbl, 2) / 100) + randint1(mahis) - 3);
    quest_ptr q;

    /* No quest if level too low */
    if (level < verrokki) return NULL;

    /* Not too early */
    if (p_ptr->max_plv < 3) return NULL;

    /* Not too many quests */
    if (_q_idx >= MAX_TK_QUEST) return NULL;

    /* Paranoia */
    if (!dungeon) return NULL;
    if ((level == d_info[dungeon].maxdepth) && (!dungeon_conquered(dungeon))) return NULL;

    /* Not in Chameleon Cave */
    if (dungeon == DUNGEON_CHAMELEON) return NULL;

    if (_q_idx > 2)
    {
        int i, monesko = 0;
        for (i = 0; i < _q_idx; i++)
        {
            if ((_tk_quests[i].completed_turn) && (_tk_quests[i].killed != _tk_quests[i].goal_ct)) monesko++;
        }
        if (monesko > 2 - _sohoglyth_reward_level) return NULL; /* Side-quests stop appearing once it's impossible to gain further rewards */
    }

    /* Not too many quests in one dungeon */
    if (!no_wilderness)
    {
        int i, osumat = 0, limit = (dungeon == DUNGEON_ANGBAND) ? 5 : 2;
        for (i = 0; i < _q_idx; i++)
        {
            if (_tk_quests[i].dungeon == dungeon) osumat++;
            if (osumat >= limit) break;
        }
        if (osumat >= limit) return NULL;
        /* Never guarantee a quest */
        if (magik(60))
        {
            return NULL;
        }
    }

    /* Not too many quests at this clvl, especially if the danger level is decreasing */
    {
        int i, osumat = 0, huippu = 0;
        for (i = 0; i < _q_idx; i++)
        {
            if (_tk_quests[i].start_lev == p_ptr->max_plv)
            {
                osumat++;
                if (_tk_quests[i].level > huippu) huippu = _tk_quests[i].level;
            }
        }
        if (osumat >= 4) return NULL;
        if ((osumat >= 1) && (level < huippu + osumat * 3)) return NULL;
    }

    /* Roll a new quest */
    q = quests_get(PURPLE_QUEST);
    q->level = MIN(88, (MAX(p_ptr->max_plv + 5, (p_ptr->max_plv * interpolate(p_ptr->max_plv, tbl, 2) / 100)) + level) / 2);
    q->dungeon = dungeon;
    get_purple_questor(q);
    q->goal = QG_KILL_MON;
    q->status = QS_IN_PROGRESS;
    q->level = level;
    q->danger_level = level;

    /* Store the details permanently */
    _tk_quests[_q_idx].goal_idx = q->goal_idx;
    _tk_quests[_q_idx].goal_ct = q->goal_count;
    _tk_quests[_q_idx].dungeon = q->dungeon;
    _tk_quests[_q_idx].start_lev = p_ptr->max_plv;
    _tk_quests[_q_idx].killed = 0;
    _tk_quests[_q_idx].completed_lev = 0;
    _tk_quests[_q_idx].completed_turn = 0;
    _tk_quests[_q_idx].level = level;
    _tk_quests[_q_idx].danger_level = r_info[q->goal_idx].level;

    _q_idx++;

    return q;
}

static void _plasma_bolt_spell(int cmd, variant *res)
{
    int dice = 8 + (p_ptr->lev / 6);
    int sides = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Plasma Bolt");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of plasma.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_PLASMA, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _chaos_bolt_spell(int cmd, variant *res)
{
    int dice = 8 + (p_ptr->lev / 6);
    int sides = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Chaos Bolt");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of chaos.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_CHAOS, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_burst_spell(int cmd, variant *res)
{
    int dice = 3;
    int sides = 5;
    int rad = spell_power((p_ptr->lev < 30) ? 2 : 3);
    int base = p_ptr->lev + p_ptr->lev / 3;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Burst");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell + base)));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of magic.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_MISSILE, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell + p_ptr->lev + base), rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _triple_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Triple Attack");
        break;
    case SPELL_DESC:
        var_set_string(res, "Simultaneously attacks in three adjacent directions.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int cdir;
        int y, x;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) break;
        if (dir == 5) break;

        for (cdir = 0;cdir < 8; cdir++)
        {
            if (cdd[cdir] == dir) break;
        }

        if (cdir == 8) break;

        y = py + ddy_cdd[cdir];
        x = px + ddx_cdd[cdir];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("You attack the empty air.");
        y = py + ddy_cdd[(cdir + 7) % 8];
        x = px + ddx_cdd[(cdir + 7) % 8];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("You attack the empty air.");
        y = py + ddy_cdd[(cdir + 1) % 8];
        x = px + ddx_cdd[(cdir + 1) % 8];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("You attack the empty air.");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wave_of_death_spell(int cmd, variant *res)
{
    int sides = p_ptr->lev * 3;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wave of Death");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(sides), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "Damages all living monsters in sight.");
        break;
    case SPELL_CAST:
    {
        dispel_living(spell_power(randint1(sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _berserk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Berserk Frenzy");
        break;
    case SPELL_DESC:
        var_set_string(res, "Drives you into a berserk frenzy, gaining great combat bonuses, but losing the ability to think clearly. Also blesses you, and briefly hastes you at high levels.");
        break;
    case SPELL_CAST:
    {
        int pituus = 10 + randint1(p_ptr->lev);
        int pituus2 = p_ptr->lev - 32;
        msg_print("Raaagh! You feel like hitting something.");
        set_shero(pituus, FALSE);
        set_blessed(pituus, FALSE);
        if (pituus2 > 0) set_fast(p_ptr->fast + pituus2, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rock_smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rock Smash");
        break;
    case SPELL_DESC:
        var_set_string(res, "Destroys a wall, or powerfully attacks a monster made of stone.");
        break;
    case SPELL_CAST:
    {
        int dir, y, x;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) break;
        if (dir == 5) break;

        y = py + ddy[dir];
        x = px + ddx[dir];

        if (cave[y][x].m_idx)
        {
            py_attack(y, x, HISSATSU_HAGAN);
            var_set_bool(res, TRUE);
        }
    
        if (!cave_have_flag_bold(y, x, FF_HURT_ROCK)) break;

        cave_alter_feat(y, x, FF_HURT_ROCK);
        p_ptr->update |= (PU_FLOW);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_vamp_spell(int cmd, variant *res)
{
    int dam = spell_power(70 + p_ptr->to_d_spell/3);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Purple Vampirism");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires 3 bolts. Each of the bolts absorbs some hit points from a living monster and gives them to you.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("dam 3*%d", dam));
        break;
    case SPELL_CAST:
    {
        int i, dir;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) break;

        virtue_add(VIRTUE_SACRIFICE, -1);
        virtue_add(VIRTUE_VITALITY, -1);

        for (i = 0; i < 3; i++)
        {
            if (drain_life(dir, dam) && p_ptr->pclass != CLASS_BLOOD_MAGE)
                 vamp_player(dam);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wall_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wall of Stone");
        break;
    case SPELL_DESC:
        var_set_string(res, "Creates walls on all open, surrounding squares.");
        break;
    case SPELL_CAST:
        wall_stone();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slaughter_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Purple Slaughter");
        break;
    case SPELL_DESC:
        var_set_string(res, "Performs a series of rush attacks, continuing while monsters are killed and you still have SP.");
        break;
    case SPELL_CAST:
    {
        const int mana_cost_per_monster = 8;
        bool uusi = TRUE;
        bool mdeath;
        var_set_bool(res, FALSE);
        do
        {
            if (!rush_attack(5, &mdeath)) break;
            if (uusi)
            {
                /* Reserve needed mana point */
                p_ptr->csp -= calculate_cost(40);
                uusi = FALSE;
            }
            else
                p_ptr->csp -= mana_cost_per_monster;

            if (!mdeath) break;
            command_dir = 0;

            p_ptr->redraw |= PR_MANA;
            handle_stuff();
        }
        while (p_ptr->csp > mana_cost_per_monster);

        if (uusi) break;
    
        /* Restore reserved mana */
        p_ptr->csp += calculate_cost(40);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _nether_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nether Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a large ball of nether.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 9 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_NETHER, dir, spell_power(p_ptr->lev * 9 + 50 + p_ptr->to_d_spell), 3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a large ball of pure mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 6 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You cast a mana storm.");
        fire_ball(GF_MANA, dir, spell_power(p_ptr->lev * 6 + 50 + damroll(10, 10) + p_ptr->to_d_spell), 4);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_orc_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Orcs");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay orcs.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_ORC, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_animal_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Animals");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay animals.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_ANIMAL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_troll_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Trolls");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay trolls.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_TROLL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_human_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Humans");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay humans.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_HUMAN, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Flame Tongue");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily be branded with fire.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_FIRE, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_cold_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Winter's Chill");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily be branded with frost.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_COLD, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_acid_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Caustic Coating");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily be branded with acid.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_ACID, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_elec_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Thunder Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily be branded with lightning.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_ELEC, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_giant_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Giants");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay giants.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_GIANT, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Dragons");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay dragons.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_DRAGON, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _long_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Long Attack");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attacks a monster two squares away.");
        break;
    case SPELL_CAST:
        {
            int dir = 5;
            bool b = FALSE;

            project_length = 2;
            if (get_fire_dir(&dir))
            {
                project_hook(GF_ATTACK, dir, HISSATSU_2, PROJECT_STOP | PROJECT_KILL);
                b = TRUE;
            }
            var_set_bool(res, b);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rush_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rush Attack");
        break;
    case SPELL_DESC:
        var_set_string(res, "Steps closer to a monster and attacks it.");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(2, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Demons");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay demons.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_DEMON, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slay Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay undead.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_UNDEAD, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_sharp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Purple Sharpness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily be sharp. (Weapons that already Sharp or *Sharp* are not further improved.)");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_VORPAL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _smite_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Smite Evil");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily slay evil.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_EVIL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _vampirism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Vampiric Drain");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily turn vampiric.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_VAMP, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mark_chaos_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mark of Chaos");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily turn chaotic.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_CHAOS, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_stun_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Purple Stunning");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily stun monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_STUN, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_brand_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Brand");
        break;
    case SPELL_DESC:
        var_set_string(res, "Your melee weapons will temporarily receive extra power from your mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_MANA, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _troika_spells[2][MAX_TK_SPELL] =
{
    { /* Guntujant's spells */
    /* id cost fail spell (note that level is determined on learning) */
    { 1, 3, 25, heroism_spell},
    { 2, 1, 25, magic_missile_spell},
    { 3, 5, 30, _mana_burst_spell},
    { 4, 10, 30, _triple_attack_spell},
    { 5, 5, 35, light_area_spell},
    { 6, 5, 35, detect_monsters_spell},
    { 7, 10, 40, _berserk_spell},
    { 8, 10, 35, building_up_spell},
    { 9, 10, 40, wonder_spell},
    { 10, 10, 40, _rock_smash_spell},
    { 11, 13, 45, _plasma_bolt_spell},
    { 12, 13, 45, _chaos_bolt_spell},
    { 13, 25, 50, nether_ball_spell},
    { 14, 30, 50, _wave_of_death_spell},
    { 15, 50, 55, _purple_vamp_spell},
    { 16, 45, 55, drain_mana_spell},
    { 17, 40, 60, _slaughter_spell},
    { 18, 50, 60, _wall_spell},
    { 19, 60, 65, _nether_storm_spell},
    { 20, 60, 65, _mana_storm_spell}

    },

    { /* Uxip's spells */
    { 1, 5, 25, _slay_orc_spell},
    { 2, 10, 25, _slay_animal_spell},
    { 3, 10, 25, _slay_troll_spell},
    { 4, 20, 35, _slay_human_spell},
    { 5, 20, 40, _brand_fire_spell},
    { 6, 20, 40, _brand_cold_spell},
    { 7, 25, 45, _brand_acid_spell},
    { 8, 25, 45, _brand_elec_spell},
    { 9, 35, 55, _slay_dragon_spell},
    { 10, 25, 50, _slay_giant_spell},
    { 11, 10, 55, _long_attack_spell},
    { 12, 20, 55, _rush_attack_spell},
    { 13, 55, 60, _slay_demon_spell},
    { 14, 55, 60, _slay_undead_spell},
    { 15, 55, 65, _purple_sharp_spell},
    { 16, 55, 65, _smite_evil_spell},
    { 17, 75, 65, _vampirism_spell},
    { 18, 75, 65, _mark_chaos_spell},
    { 19, 75, 65, _purple_stun_spell},
    { 20, 50, 65, _mana_brand_spell}
    }
};

void troika_bonus_flags(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    if ((!o_ptr) || (!o_ptr->k_idx)) return; /* paranoia */
    if (o_ptr->loc.where != INV_EQUIP) return;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if ((_slay_timeouts[i] > 0) && (!have_flag(flgs, _slay_flags[i]))) add_flag(flgs, _slay_flags[i]);
    }
}

static int _troika_get_spells_learned(spell_info* spells, int alku, int kumpi)
{
    int ct = 0, i;
    bool dummy = (!spells) ? TRUE : FALSE;
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        if (_my_spell_taso[kumpi][i] == 0) continue;
        if (dummy)
        {
            ct++;
            continue;
        }
        else
        {
            spell_info *src, *dest;
            src = &_troika_spells[kumpi][i];
            dest = &spells[alku + (ct++)];
            dest->level = _my_spell_taso[kumpi][i];
            dest->cost = src->cost;
            dest->fail = src->fail;
            dest->fn = src->fn;
        }
    }
    return ct;
}

void troika_learn_spell(monster_race *r_ptr)
{
    bool unique = (r_ptr->flags1 & RF1_UNIQUE) ? TRUE : FALSE;
    byte kumpi, opittu = 0, uusi;
    int taso = MAX(10, MIN(r_ptr->level, p_ptr->max_plv * 2));
    s32b imp = MIN(400, (r_ptr->hdice * r_ptr->hside) * 100L / (taso * taso));
    if (taso < p_ptr->max_plv + 4) return;
    if (randint1(555) > imp) return;
    kumpi = randint0(2);
    opittu = _troika_get_spells_learned(NULL, 0, kumpi);
    if (opittu >= (MAX_TK_SPELL / 2)) return;
    if ((opittu > 3) && (taso < p_ptr->max_plv * 4 / 3)) return;
    if (randint1(MAX(6, r_ptr->level)) < MIN(p_ptr->max_plv, opittu * 5 + 5)) return;
    if ((!unique) && (randint1(847 + (242 * opittu)) > imp)) return;
    if (r_ptr->level < (opittu * 8) + 8) return;
    else
    {
        uusi = (opittu * 2) + randint0(2);
        _my_spell_taso[kumpi][uusi] = p_ptr->max_plv;
        msg_format("%s teaches you the spell of <color:v>%s</color>.", _troika_names[kumpi], get_spell_name(_troika_spells[kumpi][uusi].fn));
        msg_print(NULL);
    }
}

static void _dump_quests(doc_ptr doc)
{
    int i;
    doc_insert(doc, "<topic:Purple Quests>================================ <color:keypress>P</color>urple Quests ================================\n\n");
    for (i = 0; i < _q_idx; i++)
    {
        int vari = TERM_L_GREEN;
        int day = 0, hour = 0, min = 0;
        monster_race *r_ptr = &r_info[_tk_quests[i].goal_idx];
        if (!r_ptr) continue; /* paranoia */
        if ((_tk_quests[i].start_lev) && (!_tk_quests[i].completed_lev)) vari = TERM_YELLOW;
        else if (_tk_quests[i].killed < _tk_quests[i].goal_ct) vari = TERM_RED;
        if (_tk_quests[i].goal_ct > 1)
        {
            char name[MAX_NLEN];
            strcpy(name, r_name + r_ptr->name);
            plural_aux(name);
            doc_printf(doc, "%2d) <indent><style:indent><color:%c>%s, Level %d - Kill %d %s\n", i + 1, attr_to_attr_char(vari), d_name + d_info[_tk_quests[i].dungeon].name, _tk_quests[i].level, _tk_quests[i].goal_ct, name);
        }
        else
            doc_printf(doc, "%2d) <indent><style:indent><color:%c>%s, Level %d - Kill %s\n", i + 1, attr_to_attr_char(vari), d_name + d_info[_tk_quests[i].dungeon].name, _tk_quests[i].level, r_name + r_ptr->name);
        switch (vari)
        {
            case TERM_YELLOW:
                doc_printf(doc, "In Progress\n");
                break;
            case TERM_RED:
                extract_day_hour_min_imp(_tk_quests[i].completed_turn, &day, &hour, &min);
                doc_printf(doc, "Failed: Day %d, %d:%02d, at CL %d", day, hour, min, _tk_quests[i].completed_lev);
                if (_tk_quests[i].goal_ct > 1) doc_printf(doc, " (%d kills)", _tk_quests[i].killed);
                doc_printf(doc, "\n");
                break;
            default:
                extract_day_hour_min_imp(_tk_quests[i].completed_turn, &day, &hour, &min);
                doc_printf(doc, "Completed: Day %d, %d:%02d, at CL %d\n", day, hour, min, _tk_quests[i].completed_lev);
                break;
        }
        doc_printf(doc, "</color></style></indent>\n");
    }
}

static void _calc_bonuses(void)
{
    int hand;
    if (_sohoglyth_reward_level > 0) p_ptr->telepathy = TRUE;
    if (_sohoglyth_reward_level > 1) p_ptr->munchkin_pseudo_id = TRUE;
    if (_sohoglyth_reward_level > 2)
    {
        p_ptr->pass_wall = TRUE;
        p_ptr->levitation = TRUE;
    }
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if (p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS)
        {
            p_ptr->weapon_info[hand].to_d += (p_ptr->lev / 3) + (_sohoglyth_reward_level * 3);
            p_ptr->weapon_info[hand].dis_to_d += (p_ptr->lev / 3) + (_sohoglyth_reward_level * 3);
        }
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (_sohoglyth_reward_level > 0) add_flag(flgs, OF_TELEPATHY);
    if (_sohoglyth_reward_level > 2) add_flag(flgs, OF_LEVITATION);
}

static void _troika_load(savefile_ptr file)
{
    int i;
    _troika_ini_quests();
    _troika_ini_spells();
    troika_wipe_timeouts();
    _sohoglyth_reward_level = savefile_read_byte(file);
    _q_idx = savefile_read_byte(file);
    for (i = 0; i < _q_idx; i++)
    {
        int j = MIN(i, MAX_TK_QUEST - 1); /* paranoia */
        _tk_quests[j].goal_idx = savefile_read_u16b(file);
        _tk_quests[j].dungeon = savefile_read_u16b(file);
        _tk_quests[j].level = savefile_read_u16b(file);
        _tk_quests[j].danger_level = savefile_read_u16b(file);
        _tk_quests[j].goal_ct = savefile_read_byte(file);
        _tk_quests[j].killed = savefile_read_byte(file);
        _tk_quests[j].start_lev = savefile_read_byte(file);
        _tk_quests[j].completed_lev = savefile_read_byte(file);
        _tk_quests[j].completed_turn = savefile_read_u32b(file);
    }
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        _slay_timeouts[i] = savefile_read_byte(file);
    }
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        _my_spell_taso[0][i] = savefile_read_byte(file);
        _my_spell_taso[1][i] = savefile_read_byte(file);
    }
}

static void _troika_save(savefile_ptr file)
{
    int i;
    savefile_write_byte(file, _sohoglyth_reward_level);
    savefile_write_byte(file, _q_idx);
    for (i = 0; i < _q_idx; i++)
    {
        savefile_write_u16b(file, _tk_quests[i].goal_idx);
        savefile_write_u16b(file, _tk_quests[i].dungeon);
        savefile_write_u16b(file, _tk_quests[i].level);
        savefile_write_u16b(file, _tk_quests[i].danger_level);
        savefile_write_byte(file, _tk_quests[i].goal_ct);
        savefile_write_byte(file, _tk_quests[i].killed);
        savefile_write_byte(file, _tk_quests[i].start_lev);
        savefile_write_byte(file, _tk_quests[i].completed_lev);
        savefile_write_u32b(file, _tk_quests[i].completed_turn);
    }
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        savefile_write_byte(file, _slay_timeouts[i]);
    }
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        savefile_write_byte(file, _my_spell_taso[0][i]);
        savefile_write_byte(file, _my_spell_taso[1][i]);
    }
}

static spell_info *_get_spells(void)
{
    static spell_info spells[MAX_SPELLS];
    int laskuri = _troika_get_spells_learned(spells, 0, 0);
    troika_spell_hack = laskuri;
    laskuri += _troika_get_spells_learned(spells, laskuri, 1);
    spells[laskuri].fn = NULL;
    return spells;
}

static void _troika_dump(doc_ptr doc)
{
    _dump_quests(doc);
    py_dump_spells(doc);
}

bool troika_dispel_timeouts(void)
{
    bool tulos = FALSE;
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if ((_slay_timeouts[i] > 0) && (one_in_(2)))
        {
            _inc_purple_timeout(_slay_flags[i], -500);
            tulos = TRUE;
        }
    }
    return tulos;
}

void troika_reduce_timeouts(void)
{
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if (_slay_timeouts[i] > 0) _inc_purple_timeout(_slay_flags[i], -1);
    }
}

bool troika_allow_equip_item(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if ((!o_ptr) || (!o_ptr->k_idx)) return FALSE;
    if (!disciple_is_(DISCIPLE_TROIKA)) return TRUE; /* paranoia */
    if (o_ptr->tval == TV_SHIELD)
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("A distant voice bellows:");
            cmsg_print(TERM_VIOLET, "'The equipping of a shield is an abomination unto Uxip!'");
        }
        else msg_print("Shields are an abomination unto Uxip!");
        viesti = TRUE;
        return FALSE;
    }
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_NO_MAGIC))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("The voice of Uxip roars:");
            cmsg_print(TERM_VIOLET, "'Seekest thou to suppress the magic we have given thee? Thou shouldst embrace it, and be grateful.'");
        }
        else msg_print("Anti-magic items are an abomination unto Uxip!");
        obj_learn_flag(o_ptr, OF_NO_MAGIC);
        viesti = TRUE;
        return FALSE;
    }
    if ((object_is_helmet(o_ptr)) && (o_ptr->name2 == EGO_ARMOR_SEEING) && (p_ptr->max_plv < 50))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("The voice of Uxip bellows:");
            cmsg_print(TERM_VIOLET, "'Thou art unworthy of such a powerful item!'");
        }
        else msg_print("Uxip considers only level 50 characters to be worthy of Helms of Seeing. For you to wear one would be an abomination.");
        viesti = TRUE;
        return FALSE;
    }
    return TRUE;
}

bool troika_allow_use_device(object_type *o_ptr)
{
    if ((!o_ptr) || (!o_ptr->k_idx) || (!object_is_device(o_ptr))) return FALSE;
    if (!disciple_is_(DISCIPLE_TROIKA)) return TRUE; /* paranoia */
    if (!o_ptr->activation.type) return TRUE;
    if ((o_ptr->tval == TV_WAND) && (o_ptr->activation.type == EFFECT_DRAIN_LIFE) && (p_ptr->max_plv < 40))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("A distant voice bellows:");
            cmsg_print(TERM_VIOLET, "'It is an abomination unto Uxip that such a weakling should use such a wand!'");
        }
        else
            msg_print("It is an abomination unto Uxip that you should do anything cool before reaching level 40.");
        viesti = TRUE;
        return FALSE;
    }
    if ((o_ptr->tval == TV_ROD) && (o_ptr->activation.type == EFFECT_DETECT_ALL) && (p_ptr->max_plv < 40))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("The voice of Uxip bellows:");
            cmsg_print(TERM_VIOLET, "'It is an abomination unto Uxip that such a weakling should use such a rod!'");
        }
        else
            msg_print("It is an abomination unto Uxip that you should use such an overpowered item before reaching level 40.");
        viesti = TRUE;
        return FALSE;
    }
    return TRUE;
}

class_t *troika_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  17,  30,   3,  16,  20,  64,  42};
    skills_t xs = { 12,   7,   8,   0,   0,   0,  20,  15};

        me.name = "Troika";
 me.subdesc = "The Troika is a group of three closely related Purples working together - "
 "Guntujant, Uxip and Sohoglyth. Guntujant is a capricious Purple who follows "
 "the careers of his servants very closely, and rewards and punishes them "
 "seemingly at random; the powers gained by his followers are unpredictable, "
 "but generally quite strong. Uxip delights in combat, and grants Troika "
 "disciples their prodigious endurance and skill with two-handed weapons; "
 "however, a number of common items are an abomination unto Uxip, and cannot "
 "be used by followers of the Troika. Sohoglyth is a mysterious spirit, but "
 "a profoundly powerful one. She occasionally sets bonus quests for Troika "
 "disciples; if enough of the quests are completed with no failures, she will "
 "reward the questor with the power of telepathy, remote object quality "
 "sensing, or even walking through walls.\n\n"
 "Troika Disciples are in general very strong, especially in combat but also "
 "sorcery, yet the limitations placed upon them by Uxip and Guntujant also make "
 "them in some ways vulnerable. In particular, they frequently suffer random "
 "punishments, risk being punished if they teleport from a fight while not at low health, "
 "cannot wear a shield or cure poison with mushrooms, and must earn the right to "
 "use a rod of Detection. The spells of Troika disciples are unique in that they "
 "are not learned at any particular character level; instead, Guntujant and Uxip "
 "will teach them as a reward for killing dangerous enemies, especially uniques. "
 "Strength determines the spellcasting ability of a Troika disciple.";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.base_hp = 23;
        me.exp = 150;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.gain_level = _gain_level;
        me.character_dump = _troika_dump;
        me.get_spells_fn = _get_spells;
        me.load_player = _troika_load;
        me.save_player = _troika_save;
        init = TRUE;
    }
    me.life = 100 + (p_ptr->lev / 4);

    return &me;
}

