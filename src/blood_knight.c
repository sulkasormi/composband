#include "angband.h"

void _blood_flow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Flow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Cuts yourself.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Gives player Light Cut (10) status, or increases current cut status by 20%, whichever is greater.");
        break;
    case SPELL_CAST:
    {
        int cut = p_ptr->cut;
        cut += cut/5;
        if (cut < CUT_LIGHT)
            cut = CUT_LIGHT;

        set_cut(cut, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_sight_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Sight");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects living creatures in the vicinity.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Detects living creatures in the vicinity. At L30, gives temporary ESP Living for 30+d30 rounds.");
        break;
    case SPELL_CAST:
    {
        if (p_ptr->lev < 30)
            detect_monsters_living(DETECT_RAD_DEFAULT, "You sense potential blood!");
        else
            set_tim_blood_sight(randint1(30) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_spray_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Spray");
        break;
    case SPELL_DESC:
        var_set_string(res, "Cuts yourself, splattering nearby enemies.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Generates a radius 3 blood ball centered on the player for 2*(3d5+L+L/4) damage. Radius is increased to 4 at L30.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3, 5, p_ptr->lev + p_ptr->lev/4));
        break;
    case SPELL_CAST:
    {
        int dice = 3;
        int sides = 5;
        int rad = (p_ptr->lev < 30) ? 3 : 4;
        int base = p_ptr->lev + p_ptr->lev/4;

        project(0, rad, py, px, 2*(damroll(dice, sides) + base), GF_BLOOD, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_bath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Bath");
        break;
    case SPELL_DESC:
        var_set_string(res, "Restores constitution and reduces poisoning.");
        break;
    case SPELL_CAST:
    {
        bool chg = FALSE;
        if (do_res_stat(A_CON)) chg = TRUE;
        if (set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE)) chg = TRUE;
        if (!chg) msg_print("You don't need a bath just yet.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_shield_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Shield");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives bonus to AC depending on how wounded you are. Grants reflection if you are really hurting.");
        break;

    case SPELL_SPOIL_DESC:
        var_set_string(res, "Player gains 100*(MHP-CHP)/MHP to AC. If more than 60% wounded, player also gains reflection.");
        break;
    case SPELL_CAST:
    {
        set_tim_blood_shield(randint1(20) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_seeking_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Seeking");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives slay living to your weapon.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "For 30+d30 rounds, the player's weapon will Slay Living (x2).");
        break;
    case SPELL_CAST:
    {
        set_tim_blood_seek(randint1(30) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Rage");
        break;
    case SPELL_DESC:
        var_set_string(res, "Enter a blood frenzy. Gives speed and big bonuses to hit and damage.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "For L/2+d(L/2) rounds, player is hasted and berserk.");
        break;
    case SPELL_CAST:
    {
        int dur = randint1(p_ptr->lev/2) + p_ptr->lev/2;
        set_fast(dur, FALSE);
        set_shero(dur, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_feast_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Feast");
        break;
    case SPELL_DESC:
        var_set_string(res, "You begin to feast on your opponents blood, doing extra damage but at a cost to your own health.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "For 25+d25 rounds, each melee strike does +35 damage, but player takes 15 damage per strike.");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (p_ptr->tim_blood_feast)
        {
            if (!get_check("Cancel the Blood Feast? ")) return;
            set_tim_blood_feast(0, TRUE);
        }
        else
        {
            set_tim_blood_feast(randint1(25) + 25, FALSE);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_revenge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Revenge");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives an aura of bloody revenge. Monsters take damaged based on your cut status.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "For 5+d5 rounds, any foe that does X melee damage to the player takes X*C/100 damage in revenge, where C is the player's current cut status. However, this retaliatory damage is bounded between C/10 and 50 per strike.");
        break;
    case SPELL_CAST:
    {
        set_tim_blood_revenge(randint1(5) + 5, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _is_blood_potion(obj_ptr obj)
    { return obj->tval == TV_POTION && obj->sval == SV_POTION_BLOOD; }
static int _count_blood_potions(void)
    { return pack_count(_is_blood_potion); }

void _blood_pool_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Pool");
        break;
    case SPELL_DESC:
        var_set_string(res, "Creates a macabre Potion of Healing made of your own blood.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Create a potion of blood. Player is limited to 30 such potions, and may neither "
                                "drop, throw nor sell them. Quaffing a potion of blood heals 100hp and cures "
                                "blindness, confusion, poison and stuns.");
        break;
    case SPELL_CAST:
    {
        object_type forge;
        int ct = _count_blood_potions();

        if (ct >= 30)
        {
            msg_print("You have too many blood potions at the moment. Why not drink some?");
            var_set_bool(res, FALSE);
            return;
        }

        msg_print("You feel light headed.");
        object_prep(&forge, lookup_kind(TV_POTION, SV_POTION_BLOOD));
        if (!object_is_known(&forge)) obj_identify(&forge);
        object_origins(&forge, ORIGIN_BLOOD);
        pack_carry(&forge);
        msg_print("You store your blood for future use.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_explosion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blood Explosion");
        break;
    case SPELL_DESC:
        var_set_string(res, "Damages all living creatures in sight at tremendous cost to your own health.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "All living creatures in player's line of sight take 500 damage.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, 500));
        break;
    case SPELL_CAST:
    {
        msg_print("You cut too deep ... Your blood explodes!");
        dispel_living(500);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _cauterize_wounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cauterize Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Cures cuts");
        break;
    case SPELL_CAST:
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _powers[] =
{
    { A_CON, {30, 20, 50, _cauterize_wounds_spell} }, 
    { -1, { -1, -1, -1, NULL} }
};

static spell_info _spells[] = 
{
    /*lvl cst fail spell */
    {  1,   1, 20, _blood_flow_spell },
    {  5,  5,  30, _blood_sight_spell},
    { 10, 10,  30, _blood_spray_spell},
    { 15, 20,  30, _blood_bath_spell},
    { 20, 30,  30, _blood_shield_spell},
    { 25, 50,  40, _blood_seeking_spell},
    { 30, 60,  40, _blood_rage_spell},
    { 40, 60,  50, _blood_feast_spell},
    { 42, 60,   0, _blood_revenge_spell},
    { 45,200,   0, _blood_pool_spell},
    { 50,500,  60, _blood_explosion_spell},
    { -1, -1,  -1, NULL}
}; 

static int _get_spells(spell_info* spells, int max)
{
    return get_spells_aux(spells, max, _spells);
}

static int _get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _powers);
}

static void _character_dump(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = _get_spells(spells, MAX_SPELLS);

    py_display_spells(doc, spells, ct);
}

static void _calc_bonuses(void)
{
    p_ptr->regen += 100 + 2*p_ptr->lev;
    if (p_ptr->lev >= 30) res_add(RES_FEAR);

    if (p_ptr->cut > 0)
    {
        int to_h = 0;
        int to_d = 0;
        int to_stealth = 0;
        if (p_ptr->cut >= CUT_MORTAL_WOUND)
        {
            to_h = 25;
            to_d = 25;
            to_stealth = -10;
        }
        else if (p_ptr->cut >= CUT_DEEP_GASH)
        {
            to_h = 15;
            to_d = 15;
            to_stealth = -3;
        }
        else if (p_ptr->cut >= CUT_SEVERE)
        {
            to_h = 8;
            to_d = 8;
            to_stealth = -2;
        }
        else if (p_ptr->cut >= CUT_NASTY)
        {
            to_h = 6;
            to_d = 6;
            to_stealth = -2;
        }
        else if (p_ptr->cut >= CUT_BAD)
        {
            to_h = 4;
            to_d = 4;
            to_stealth = -1;
        }
        else if (p_ptr->cut >= CUT_LIGHT)
        {
            to_h = 2;
            to_d = 2;
            to_stealth = -1;
        }
        else
        {
            to_h = 1;
            to_d = 1;
            to_stealth = -1;
        }
        p_ptr->weapon_info[0].to_h += to_h;
        p_ptr->weapon_info[1].to_h += to_h;
        p_ptr->to_h_m  += to_h;
        p_ptr->weapon_info[0].dis_to_h += to_h;
        p_ptr->weapon_info[1].dis_to_h += to_h;

        p_ptr->weapon_info[0].to_d += to_d;
        p_ptr->weapon_info[1].to_d += to_d;
        p_ptr->to_d_m  += to_d;
        p_ptr->weapon_info[0].dis_to_d += to_d;
        p_ptr->weapon_info[1].dis_to_d += to_d;

        p_ptr->skills.stl += to_stealth;
    }

    if (p_ptr->tim_blood_shield)
    {
        int amt = 100 * (p_ptr->mhp - p_ptr->chp) / p_ptr->mhp; 
        p_ptr->to_a += amt;
        p_ptr->dis_to_a += amt;    
        if (amt > 60)
            p_ptr->reflect = TRUE;
    }

    if (p_ptr->tim_blood_feast)
    {
        p_ptr->weapon_info[0].to_d += 35;
        p_ptr->weapon_info[1].to_d += 35;
        p_ptr->to_d_m  += 35; /* Tentacles, beak, etc. */
        p_ptr->weapon_info[0].dis_to_d += 35;
        p_ptr->weapon_info[1].dis_to_d += 35;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_REGEN);
    if (p_ptr->lev >= 30) add_flag(flgs, OF_RES_FEAR);
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    int frac = p_ptr->chp * 100 / p_ptr->mhp;
    static point_t taulukko[7] = { {0, 0}, {1, 50}, {20, 128}, {40, 242}, {60, 440}, {80, 666}, {100, 1000} };
    if (p_ptr->chp < p_ptr->mhp) info_ptr->xtra_blow += interpolate(MIN(100 - frac, (p_ptr->lev * 5 / 3) + MAX(5, p_ptr->lev / 2 - 8)), taulukko, 7);
}

static void _on_cast(const spell_info *spell)
{
    set_cut(p_ptr->cut + spell->level, FALSE);
    p_ptr->update |= PU_BONUS;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "bloodcraft";
        me.options = CASTER_USE_HP;
        me.which_stat = A_CON;
        me.on_cast = _on_cast;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_CHAIN_MAIL, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CURE_CRITICAL, rand_range(2, 5));
}

class_t *blood_knight_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  32,   2,  16,   6,  70,  20};
    skills_t xs = { 12,   7,  10,   0,   0,   0,  23,  15};

        me.name = "Blood-Knight";
        me.desc = "Blood-Knights are fighters who have delved into the dark arts and can perform "
                  "some special offensive effects using their own health. In addition to the "
                  "HP cost, using such an ability causes bleeding, with an amount proportional "
                  "to the cost of the ability. These special abilities rely on Constitution.\n \n"
                  "Blood-Knights are very unusual in that cuts and low health make them more powerful; at "
                  "full health their melee is nothing special, but the more damaged and wounded they are, "
                  "the faster and deadlier their attacks become. On the "
                  "brink of death they are the strongest fighters imaginable, and stories of their "
                  "legendary feats abound; but with great power comes great risk of death, "
                  "and you don't recall ever meeting one of these heroes of legend in person!\n \n"
                  "Since a Blood-Knight relies on their own blood for their power, this class is restricted "
                  "to certain races; no non-living race may walk the red path.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  2;
        
        me.base_skills = bs;
        me.extra_skills = xs;
        
        me.life = 120;
        me.base_hp = 20;
        me.exp = 150;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}

