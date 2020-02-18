#include "angband.h"

/* Check for valid equipment. Note, the pointer we return
   will point to a descriptive error message indefinitely.
   We don't use format(...). Also, different messages will
   have different addresses, so that _calc_bonuses() can
   keep the user up to date as to why their powers don't work. */
cptr duelist_equip_error(void)
{
    int wgt = equip_weight(object_is_armour);

    if (wgt > (120 + (p_ptr->lev * 3)))
        return "The weight of your equipment is disrupting your talents.";

    if (equip_find_obj(TV_SHIELD, SV_ANY) || equip_find_obj(TV_CAPTURE, SV_ANY))
        return "Your shield is disrupting your talents.";

    if (p_ptr->weapon_ct > 1)
        return "Dual wielding is disrupting your talents.";

    if (equip_find_obj(TV_SWORD, SV_POISON_NEEDLE))
        return "The Poison Needle is not an honorable dueling weapon.";

    if (p_ptr->anti_magic)
        return "An anti-magic barrier disrupts your talents.";

    return NULL;
}

cptr duelist_current_challenge(void)
{
    static char current_challenge[200];

    /* paranoia ... this only seems to happen with wizard mode summoned monsters
       after a save and restore, so probably the wizard 'n' command is broken */
    if (p_ptr->duelist_target_idx && !m_list[p_ptr->duelist_target_idx].r_idx)
        p_ptr->duelist_target_idx = 0;

    if (p_ptr->duelist_target_idx)
    {
        monster_desc(current_challenge, &m_list[p_ptr->duelist_target_idx], MD_ASSUME_VISIBLE);
        return current_challenge;
    }
    if (duelist_equip_error())
        return "Talents Disrupted";

    return "No Current Challenge";
}

int duelist_skill_sav(int m_idx)
{
    int result = p_ptr->skills.sav;
    if ( p_ptr->pclass == CLASS_DUELIST
      && m_idx > 0
      && p_ptr->duelist_target_idx == m_idx )
    {
        result = result + 15 + p_ptr->lev;
    }
    return result;
}

bool duelist_can_challenge(void)
{
    int i;
    for (i = 1; i < max_m_idx; i++)
    {
        mon_ptr mon = & m_list[i];
        if (!mon->r_idx) continue;
        if (is_pet(mon)) continue;
        if (!mon->ml) continue;
        if (mon->cdis > MAX_RANGE) continue; /* wizard mode */
        return TRUE;
    }
    return FALSE;
}

bool duelist_issue_challenge(void)
{
    bool result = FALSE;
    int m_idx = 0;

    if (target_set(TARGET_MARK))
    {
        if (target_who > 0)
            m_idx = target_who;
        else
            m_idx = cave[target_row][target_col].m_idx;
    }

    if (m_idx)
    {
        if (m_idx == p_ptr->duelist_target_idx)
            msg_format("%^s has already been challenged.", duelist_current_challenge());
        else
        {
            /* of course, we must first set the target index before duelist_current_challenge()
               will return the correct text */
            p_ptr->duelist_target_idx = m_idx;
            msg_format("You challenge %s to a duel!", duelist_current_challenge());
            set_monster_csleep(m_idx, 0);
            set_hostile(&m_list[m_idx]);
            result = TRUE;
        }
    }
    else if (p_ptr->duelist_target_idx)
    {
        p_ptr->duelist_target_idx = 0;
        msg_print("You cancel your current challenge!");
    }

    p_ptr->redraw |= PR_STATUS;
    return result;
}


/*
 * I spiked the Ninja/Samurai rush_attack() ... it was not quite what I need.
 */

typedef enum { 
    _rush_cancelled,  /* Don't charge player energy ... they made a dumb request */
    _rush_failed,     /* Rush to foe was blocked by another monster, or foe out of range */
    _rush_succeeded   /* Got him! */
} _rush_result;

typedef enum {
    _rush_normal,     /* Attacks first monster in the way */
    _rush_acrobatic,   /* Displaces intervening monsters (waking them up) */
    _rush_phase,
} _rush_type;

_rush_result _rush_attack(int rng, _rush_type type)
{
    _rush_result result = _rush_cancelled;
    int tx, ty;
    int tm_idx = 0;
    u16b path_g[32];
    int path_n, i;
    bool moved = FALSE;
    int flg = 0;
    int dis = 0;

    if (type == _rush_normal)
        flg = PROJECT_STOP | PROJECT_KILL;
    else if (type == _rush_acrobatic)
        flg = PROJECT_THRU | PROJECT_KILL;
    else
        flg = PROJECT_DISI | PROJECT_THRU;

    if (!p_ptr->duelist_target_idx)
    {
        msg_print("You need to select a foe first (Mark Target).");
        return result;
    }

    tm_idx = p_ptr->duelist_target_idx;
    tx = m_list[tm_idx].fx;
    ty = m_list[tm_idx].fy;

    dis = distance(ty, tx, py, px);

    /* Foe must be visible. For all charges except the phase charge, the
       foe must also be in your line of sight */
    if (!m_list[p_ptr->duelist_target_idx].ml ||
        (type != _rush_phase && !los(ty, tx, py, px)))
    {
        msg_format("%^s is not in your line of sight.", duelist_current_challenge());
        return result;
    }

    if (dis > rng)
    {
        msg_format("Your foe is out of range (%d vs %d).", dis, rng);
        if (!get_check("Charge anyway? ")) return result;
    }

    project_length = rng;
    path_n = project_path(path_g, project_length, py, px, ty, tx, flg);
    project_length = 0;

    if (!path_n) return result;

    result = _rush_failed;

    /* Use ty and tx as to-move point */
    ty = py;
    tx = px;

    /* Scrolling the cave would invalidate our path! */
    if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
        wilderness_scroll_lock = TRUE;

    /* Project along the path */
    for (i = 0; i < path_n; i++)
    {
        monster_type *m_ptr;
        cave_type *c_ptr;
        bool can_enter = FALSE;
        bool old_pass_wall = p_ptr->pass_wall;

        int ny = GRID_Y(path_g[i]);
        int nx = GRID_X(path_g[i]);
        c_ptr = &cave[ny][nx];

        switch (type)
        {
        case _rush_normal:
            can_enter = cave_empty_bold(ny, nx) && player_can_enter(c_ptr->feat, 0);
            break;

        case _rush_acrobatic:
            can_enter = !c_ptr->m_idx && player_can_enter(c_ptr->feat, 0);
            break;
        
        case _rush_phase:
            p_ptr->pass_wall = TRUE;
            can_enter = !c_ptr->m_idx && player_can_enter(c_ptr->feat, 0);
            p_ptr->pass_wall = old_pass_wall;
            break;
        }

        if (can_enter)
        {
            ty = ny;
            tx = nx;
            continue;
        }

        if (!c_ptr->m_idx)
        {
            msg_print("Failed!");
            break;
        }

        /* Move player before updating the monster */
        if (!player_bold(ty, tx)) move_player_effect(ty, tx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
        moved = TRUE;

        /* Update the monster */
        update_mon(c_ptr->m_idx, TRUE);

        /* Found a monster */
        m_ptr = &m_list[c_ptr->m_idx];

        /* But it is not the monster we seek! */
        if (tm_idx != c_ptr->m_idx)
        {
            /* Acrobatic Charge attempts to displace monsters on route */
            if (type == _rush_acrobatic)
            {
                /* Swap position of player and monster */
                set_monster_csleep(c_ptr->m_idx, 0);
                move_player_effect(ny, nx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
                ty = ny;
                tx = nx;
                continue;
            }
            /* Normal Charge just attacks first monster on route */
            else
                msg_format("There is %s in the way!", m_ptr->ml ? (tm_idx ? "another monster" : "a monster") : "someone");
        }

        /* Attack the monster */
        if (tm_idx == p_ptr->duelist_target_idx) result = _rush_succeeded;
        py_attack(ny, nx, 0);
        break;
    }

    if (!moved && !player_bold(ty, tx)) move_player_effect(ty, tx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
    if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
    {
        wilderness_scroll_lock = FALSE;
        wilderness_move_player(px, py);
    }
    return result;
}

/****************************************************************
 * Private Spells
 ****************************************************************/
static void _acrobatic_charge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Acrobatic Charge");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move up to 7 squares and attack your marked foe, displacing any monsters in your way.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _rush_attack(7, _rush_acrobatic) != _rush_cancelled);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _charge_target_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Charge");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move up to 5 squares and attack your marked foe.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _rush_attack(5, _rush_normal) != _rush_cancelled);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _darting_duel_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Darting Duel");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move up to 5 squares and attack your marked foe. Strafe if you attack your foe.");
        break;
    case SPELL_CAST:
        {
            int tmp = p_ptr->duelist_target_idx;
            _rush_result r = _rush_attack(5, _rush_normal);
            if (r == _rush_cancelled)
                var_set_bool(res, FALSE);
            else 
            {
                var_set_bool(res, TRUE);
                if (r == _rush_succeeded && tmp == p_ptr->duelist_target_idx)
                {
                    monster_type *m_ptr = &m_list[p_ptr->duelist_target_idx];
                    mon_anger(m_ptr);
                    teleport_player(10, TELEPORT_LINE_OF_SIGHT);
                }
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _disengage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Disengage");
        break;
    case SPELL_DESC:
        var_set_string(res, "You teleport (range 100), and prevent your marked foe from following, even if it's a monster that can normally follow teleportation. After the teleport, your foe is no longer marked.");
        break;
    case SPELL_CAST:
        if (!p_ptr->duelist_target_idx)
        {
            msg_print("You need to mark your target first.");
            var_set_bool(res, FALSE);
        }
        else if (!m_list[p_ptr->duelist_target_idx].ml)
        {
            msg_print("You may not disengage unless your foe is visible.");
            var_set_bool(res, FALSE);
        }
        else
        {
            teleport_player(100, TELEPORT_DISENGAGE);
            p_ptr->duelist_target_idx = 0;
            msg_print("You disengage from your current challenge.");
            p_ptr->redraw |= PR_STATUS;
        
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _isolation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Isolation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to teleport away all monsters in Line of Sight other than your marked foe.");
        break;
    case SPELL_CAST:
        if (!p_ptr->duelist_target_idx)
        {
            msg_print("You need to mark your target first.");
            var_set_bool(res, FALSE);
        }
        else
        {
            project_hack(GF_ISOLATION, p_ptr->lev * 4);
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mark_target_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mark Target");
        break;
    case SPELL_DESC:
        var_set_string(res, "Mark selected monster as designated foe. You may only mark a single target at a time, and receive great benefits when fighting this target.");
        break;

    case SPELL_INFO:
        var_set_string(res, format("%^s", duelist_current_challenge()));
        break;

    case SPELL_CAST:
        var_set_bool(res, duelist_issue_challenge());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _phase_charge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Phase Charge");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move up to 10 squares and attack your marked foe. Functions even if there are walls or closed doors between you and your target.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _rush_attack(10, _rush_phase) != _rush_cancelled);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void strafing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Strafing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Blink to a new location in the line of sight of your current location.");
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
            var_set_int(res, 30);
        else
            default_spell(cmd, res);
        break;
    case SPELL_CAST:
        teleport_player(10, TELEPORT_LINE_OF_SIGHT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table
 ****************************************************************/

#define MAX_DUELIST_SPELLS    9

static spell_info _spells[MAX_DUELIST_SPELLS] = 
{
    /*lvl cst fail spell */
    {  1,   0,  0, _mark_target_spell },
    {  8,  10,  0, _charge_target_spell },
    { 16,   8,  0, strafing_spell },
    { 24,  25,  0, _disengage_spell },
    { 32,  30,  0, _acrobatic_charge_spell },
    { 40,  60,  0, _isolation_spell },
    { 45,  60,  0, _darting_duel_spell },
    { 48,  80,  0, _phase_charge_spell },
    {-1,  -1,  -1, NULL}
}; 

static int _get_spells(spell_info* spells, int max)
{
    cptr msg = duelist_equip_error();

    if (msg)
    {
        msg_print(msg);
        return 0;
    }
    
    return get_spells_aux(spells, MIN(max, MAX_DUELIST_SPELLS), _spells);
}

static void _calc_bonuses(void)
{
    static cptr last_msg = NULL;
    cptr msg = duelist_equip_error();

    p_ptr->to_a -= 50;
    p_ptr->dis_to_a -= 50;

    if (!msg)
    {
        int x = (p_ptr->stat_ind[A_INT] + 3);
        int l = p_ptr->lev;
        int to_a = x/2 + x*l/50;
        p_ptr->to_a += to_a;
        p_ptr->dis_to_a += to_a;
    }

    if (msg != last_msg)
    {
        last_msg = msg;
        if (msg)
        {
            msg_print(msg);
            if (p_ptr->duelist_target_idx)
            {
                msg_format("%^s is no longer your target.", duelist_current_challenge());
                p_ptr->duelist_target_idx = 0;
                p_ptr->redraw |= PR_STATUS;
            }
        }
        else
            msg_print("You regain your talents.");
    }

    p_ptr->redraw |= PR_STATUS;
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    int to_d = (p_ptr->stat_ind[A_DEX] + 3 - 10) + p_ptr->lev/2 - o_ptr->weight/10;

    if (!duelist_equip_error())
    {
        info_ptr->to_d += to_d;
        info_ptr->dis_to_d += to_d;

        /* Blows should always be 1 ... even with Quickthorn and Shiva's Jacket! 
           But, don't make Tonberry gloves a gimme. Negative attacks now are 0 attacks!
        */
        if (info_ptr->base_blow + info_ptr->xtra_blow > 100)
        {
            info_ptr->base_blow = 100;
            info_ptr->xtra_blow = 0;
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "challenge";
        me.options = CASTER_USE_HP;
        me.which_stat = A_DEX;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_RAPIER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
}

class_t *duelist_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  21,  23,   3,  22,  16,  50,   0};
    skills_t xs = { 10,  10,  10,   0,   0,   0,  14,   0};

        me.name = "Duelist";
        me.desc = "The duelist is the ultimate one-on-one fighter, but finds himself at a severe "
                  "disadvantage when facing numerous strong foes at once. To start a duel, the duelist "
                  "first issues a challenge to his intended foe; of course, this will wake the monster "
                  "up, as there is no honor in dueling a sleeping enemy! And while the duelist will "
                  "honor the fight as a one-on-one affair, many monsters have no such scruples.\n \n"
                  "Against a challenged foe the duelist is extremely strong, gaining bonuses to saving "
                  "throws, armor class, damage reduction and combat prowess. On the other hand, due "
                  "to the single-mindedness of their focus, the duelist is quite vulnerable to unchallenged "
                  "opponents. Most of the special techniques of this class aim at enforcing the sanctity of the "
                  "duel.\n \n"
                  "The duelist only ever gains a single attack in combat, but they make the most of this "
                  "blow by gaining enhanced effects as they gain experience. Able to wound, stun, and even "
                  "hamstring their foes, the prowess of the duelist in a one-on-one encounter is legendary!\n \n"
                  "Duelists favor light armors and weapons, and cannot equip a shield. They gain no extra bonus for "
                  "wielding a weapon with both hands. For their techniques, the duelist relies on Dexterity.";
        me.stats[A_STR] =  2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] = -3;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 150;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        init = TRUE;
    }

    return &me;
}

