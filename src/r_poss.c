#include "angband.h"

#include <assert.h>

/***********************************************************************
 Possession History: Track the most recently assumed forms for the Possessor
 and Mimic races. Report these on the character dump.

 TODO: Free memory on quit().
 ***********************************************************************/
#define _HISTORY_MAX 1000

/* Special Codes for dungeon_type */
#define DUNGEON_WILD   0
#define DUNGEON_TOWN  -1   /* lvl gives which town */
#define DUNGEON_QUEST -2   /* lvl gives which quest */

typedef struct _history_s _history_t;
typedef struct _history_s *_history_ptr;

struct _history_s
{
    int r_idx;
    int d_idx;
    int d_lvl;
    int p_lvl;
    int turn;

    _history_ptr next;
};

static _history_ptr _history = NULL;

static int _history_count(void)
{
    int          ct = 0;
    _history_ptr p = _history;
    
    while (p)
    {
        ct++;
        p = p->next;
    }
    
    return ct;
}

static void _history_clear(void)
{
    _history_ptr p = _history;
    while (p)
    {
        _history = p->next;
        free(p);
        p = _history;
    }
}

static void _history_on_birth(void)
{
    _history_clear();
}

static void _history_on_possess(int r_idx)
{
    _history_ptr p = malloc(sizeof(_history_t));
    if (p)
    {
        p->r_idx = r_idx;

        if (!py_in_dungeon())
        {
            if (py_in_town())
            {
                p->d_idx = DUNGEON_TOWN;
                p->d_lvl = p_ptr->town_num;
            }
            else if (py_on_surface())
            {
                p->d_idx = DUNGEON_WILD;
                p->d_lvl = wilderness_level(p_ptr->wilderness_x, p_ptr->wilderness_y);
            }
            else if (quests_get_current())
            {
                p->d_idx = DUNGEON_QUEST;
                p->d_lvl = quests_get_current()->id;
            }
            else /* ??? */
            {
                p->d_idx = DUNGEON_WILD;
                p->d_lvl = wilderness_level(p_ptr->wilderness_x, p_ptr->wilderness_y);
            }
        }
        else
        {
            p->d_idx = dungeon_type;
            p->d_lvl = dun_level;
        }

        p->p_lvl = p_ptr->lev;
        p->turn = game_turn;

        p->next = _history;
        _history = p;
    }
}

static void _history_on_load(savefile_ptr file)
{
    int          ct = savefile_read_s32b(file);
    int          i;
    _history_ptr c, t, lst = NULL;

    _history_clear();
    for (i = 0; i < ct; i++)
    {
        c = malloc(sizeof(_history_t));
        assert(c);

        c->r_idx = savefile_read_s32b(file);
        c->d_idx = savefile_read_s32b(file);
        c->d_lvl = savefile_read_s32b(file);
        c->p_lvl = savefile_read_s32b(file);
        c->turn  = savefile_read_s32b(file);

        c->next = lst;
        lst = c;
    }

    /* Reverse List */
    while (lst)
    {
        t = lst;
        lst = lst->next;
        t->next = _history;
        _history = t;
    }
}

static void _history_on_save(savefile_ptr file)
{
    _history_ptr p = _history;
    int          ct = MIN(_HISTORY_MAX, _history_count());
    int          i = 0;

    savefile_write_s32b(file, ct);
    while (i < ct)
    {
        assert(p);
        savefile_write_s32b(file, p->r_idx);
        savefile_write_s32b(file, p->d_idx);
        savefile_write_s32b(file, p->d_lvl);
        savefile_write_s32b(file, p->p_lvl);
        savefile_write_s32b(file, p->turn);
        p = p->next;
        i++;
    }
}

/***********************************************************************
 ...
 ***********************************************************************/
static int _calc_level(int l)
{
    return l + l*l*l/2500;
}

void possessor_on_birth(void)
{
    _history_on_birth();
}

static void _birth(void) 
{ 
    object_type forge;

    possessor_on_birth();

    p_ptr->current_r_idx = MON_POSSESSOR_SOUL;
    equip_on_change_race();

    object_prep(&forge, lookup_kind(TV_WAND, SV_ANY));
    if (device_init_fixed(&forge, EFFECT_BOLT_COLD))
        py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 3;
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

static int _get_toggle(void)
{
    return p_ptr->magic_num1[0];
}

int possessor_get_toggle(void)
{
    int result = TOGGLE_NONE;
    if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
        result = _get_toggle();
    return result;
}

static void _player_action(int energy_use)
{
    if (_get_toggle() == LEPRECHAUN_TOGGLE_BLINK)
        teleport_player(10, TELEPORT_LINE_OF_SIGHT);
}

int possessor_max_plr_lvl(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];
    point_t       tbl[5] = { {10, 15}, {20, 30}, {30, 37}, { 40, 45}, { 50, 50} };
    return        interpolate(r_ptr->level, tbl, 5);
}

static int _max_lvl(void)
{
    return possessor_max_plr_lvl(p_ptr->current_r_idx);
}

/**********************************************************************
 * Attacks
 **********************************************************************/
bool possessor_can_attack(void)
{
    if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
        return p_ptr->current_r_idx != 0;
    return FALSE;
}

static cptr _hit_msg(mon_blow_ptr blow)
{
    switch (blow->method) /* XXX table this up ... monsters need this too! */
    {
    case RBM_HIT: return "hit";
    case RBM_TOUCH: return "touch";
    case RBM_PUNCH: return "punch";
    case RBM_KICK: return "kick";
    case RBM_CLAW: return "claw";
    case RBM_BITE: return "bite";
    case RBM_STING: return "sting";
    case RBM_SLASH: return "slash";
    case RBM_BUTT: return "butt";
    case RBM_CRUSH: return "crush";
    case RBM_ENGULF: return "engulf";
    case RBM_CHARGE: return "charge";
    case RBM_CRAWL: return "crawl on";
    case RBM_DROOL: return "drool on";
    case RBM_SPIT: return "spit on";
    case RBM_GAZE: return "gaze at";
    case RBM_WAIL: return "wail at";
    case RBM_SPORE: return "release spores on";
    case RBM_BEG: return "beg to";
    case RBM_INSULT: return "insult";
    case RBM_MOAN: return "moan at";
    case RBM_SHOW: return "sing to";
    case RBM_EXPLODE: return "explode at";
    }
    return "hit";
}
static bool _touched(mon_blow_ptr blow)
{
    switch (blow->method) /* XXX table this up ... monsters need this too! */
    {
    case RBM_DROOL: /* XXX */
    case RBM_SPIT:
    case RBM_GAZE:
    case RBM_WAIL:
    case RBM_SPORE:
    case RBM_BEG:
    case RBM_INSULT:
    case RBM_MOAN:
    case RBM_SHOW:
    case RBM_EXPLODE: return FALSE;
    }
    return TRUE;
}
static mon_ptr _get_mon(point_t where)
{
    int m_idx = cave[where.y][where.x].m_idx;
    if (!m_idx) return NULL;
    return &m_list[m_idx];
}
static bool _blow_is_masked(mon_blow_ptr blow)
{
    switch (blow->method)
    {
    case RBM_GAZE:
        if (p_ptr->blind)
            return TRUE;
        break;

    case RBM_HIT:
    case RBM_TOUCH:
    case RBM_PUNCH:
    case RBM_CLAW:
    case RBM_SLASH:
    case RBM_BEG:
        if (!equip_is_valid_hand(0)) /* No hands, so can't be blocked */
            return FALSE;

        if (p_ptr->weapon_ct > 0) /* Wielding a weapon blocks hand based attacks */
            return TRUE;

        if (!equip_find_empty_hand()) /* So does shield, capture ball, etc. */
            return TRUE;

        break;
    }

    return FALSE;
}
static int _dam_boost(int method, int rlev)
{
    /* Most early monsters with innate attacks aren't worth possessing as
       their damage is just too low ... Heck, a Mean Looking Mercenary with
       a good longsword is usually a much better option! */
    switch (method)
    {
    case RBM_HIT:
    case RBM_PUNCH:
    case RBM_KICK:
    case RBM_CLAW:
    case RBM_BITE:
    case RBM_STING:
    case RBM_SLASH:
    case RBM_BUTT:
    case RBM_CRUSH:
        return  2 + (rlev + 4) / 5;
    }
    return 0;
}
static int _vorpal_pct(mon_blow_ptr blow)
{
    int i;
    /* Assume CUT is never the first effect ... */
    for (i = 1; i < MAX_MON_BLOW_EFFECTS; i++)
    {
        if (blow->effects[i].effect == RBE_CUT)
        {
            int chance = blow->effects[i].pct;
            if (!chance) chance = 100;
            return chance;
        }
    }
    return 0;
}
static bool _skip_effect(int which)
{
    switch (which)
    {
    case RBE_CUT:
    case RBE_DRAIN_EXP:
    case RBE_LOSE_STR: case RBE_LOSE_INT: case RBE_LOSE_WIS:
    case RBE_LOSE_DEX: case RBE_LOSE_CON: case RBE_LOSE_CHR:
    case RBE_LOSE_ALL:
    case RBE_EAT_LITE:
        return TRUE;
    }
    return FALSE;
}
void possessor_attack(point_t where, bool *fear, bool *mdeath, int mode)
{
    mon_race_ptr body, foe_race;
    mon_ptr      foe = _get_mon(where);
    int          i, j, ac, skill, steal_ct = 0;
    bool         delay_quake = FALSE;
    char         m_name_subject[MAX_NLEN], m_name_object[MAX_NLEN];

    if (!possessor_can_attack()) return;
    if (!foe) return;

    set_monster_csleep(foe->id, 0);
    monster_desc(m_name_subject, foe, MD_PRON_VISIBLE);
    monster_desc(m_name_object, foe, MD_PRON_VISIBLE | MD_OBJECTIVE);

    body = &r_info[p_ptr->current_r_idx];
    foe_race = mon_race(foe);
    ac = mon_ac(foe);

    for (i = 0; i < MAX_MON_BLOWS; i++)
    {
        mon_blow_ptr blow = &body->blows[i];
        if (mode == WEAPONMASTER_RETALIATION) /* mystics */
        {
            for (;;)
            {
                i = randint0(4);
                blow = &body->blows[i];
                if (blow->method) break;
            }
        }

        if (*mdeath) break;
        if (!blow->method) break;
        if (_blow_is_masked(blow)) continue;
        if (foe->fx != where.x || foe->fy != where.y) break; /* teleport effect? */
        if (p_ptr->afraid && !fear_allow_melee(foe->id))
        {
            if (foe->ml)
                cmsg_format(TERM_VIOLET, "You are too afraid to attack %s!", m_name_object);
            else
                cmsg_format(TERM_VIOLET, "There is something scary in your way!");
            return;
        }

        skill = p_ptr->skills.thn + (p_ptr->to_h_m * BTH_PLUS_ADJ);
        skill += blow->power;
        if (p_ptr->stun)
            skill -= skill * MIN(100, p_ptr->stun) / 150;
        if (test_hit_norm(skill, ac, foe->ml))
        {
            msg_format("You %s %s.", _hit_msg(blow), m_name_object);
            for (j = 0; j < MAX_MON_BLOW_EFFECTS; j++)
            {
                mon_effect_ptr effect = &blow->effects[j];
                int            dam;

                if (*mdeath) break;
                if (!effect->effect) break;
                if (_skip_effect(effect->effect)) continue;
                if (effect->pct && randint1(100) > effect->pct) continue;

                /* The first effect is the base damage, and gets boosted by combat boosting
                 * magic (e.g. rings of combat, weaponmaster, et. al.) */
                if (j == 0)
                {
                    dam = damroll(effect->dd + p_ptr->innate_attack_info.to_dd, effect->ds);
                    /* Translate CUT into vorpal since monsters cannot bleed the way players can */
                    if (randint0(100) < _vorpal_pct(blow))
                    {
                        int m = 2;
                        while (one_in_(4)) m++;
                        dam *= m;
                        switch (m)
                        {
                        case 2: msg_format("You <color:U>gouge</color> %s!", m_name_object); break;
                        case 3: msg_format("You <color:y>maim</color> %s!", m_name_object); break;
                        case 4: msg_format("You <color:R>carve</color> %s!", m_name_object); break;
                        case 5: msg_format("You <color:r>cleave</color> %s!", m_name_object); break;
                        case 6: msg_format("You <color:v>smite</color> %s!", m_name_object); break;
                        case 7: msg_format("You <color:v>eviscerate</color> %s!", m_name_object); break;
                        default: msg_format("You <color:v>shred</color> %s!", m_name_object); break;
                        }
                    }
                    dam += p_ptr->to_d_m; /* XXX Need to subtract out later for non-damage effects */
                    dam += _dam_boost(blow->method, body->level);
                }
                /* Subsequent effects are add-ons, and should not be damage boosted */
                else
                    dam = damroll(effect->dd, effect->ds);

                if (p_ptr->stun)
                    dam -= dam*MIN(100, p_ptr->stun) / 150;
                if (blow->method == RBM_EXPLODE)
                {
                    possessor_explode(dam);
                    return;
                }
                switch (effect->effect)
                {
                case RBE_SHATTER:
                    if (dam > 23) delay_quake = TRUE;
                case RBE_HURT:
                    dam = mon_damage_mod(foe, dam, FALSE);
                    if (dam > 0)
                        anger_monster(foe);
                    *mdeath = mon_take_hit(foe->id, dam, fear, NULL);
                    break;
                case RBE_EAT_GOLD:
                case RBE_EAT_ITEM:
                case RBE_EAT_FOOD:
                    if (leprechaun_steal(foe->id))
                        steal_ct++;
                    break;
                case RBE_DISEASE:
                    if (dam)
                        gf_affect_m(GF_WHO_PLAYER, foe, GF_POIS, dam, GF_AFFECT_ATTACK);
                    break;
                case RBE_DRAIN_CHARGES:
                    gf_affect_m(GF_WHO_PLAYER, foe, GF_DRAIN_MANA,
                        (dam ? dam : p_ptr->lev), GF_AFFECT_ATTACK);
                    break;
                case RBE_VAMP:
                    if (monster_living(foe_race) && gf_affect_m(GF_WHO_PLAYER, foe, GF_OLD_DRAIN, dam, GF_AFFECT_ATTACK))
                    {
                        msg_format("You <color:D>drain life</color> from %s!", m_name_object);
                        hp_player(dam);
                        /* XXX limit amt per turn? */
                    }
                    break;
                default:
                    gf_affect_m(GF_WHO_PLAYER, foe, effect->effect, dam, GF_AFFECT_ATTACK);
                }
                *mdeath = (foe->r_idx == 0);
            }
            if (_touched(blow) && !*mdeath)
                touch_zap_player(foe->id);
        }
        else
        {
            msg_print("You miss.");
        }
        if (mode == WEAPONMASTER_RETALIATION) break;
    }
    if (delay_quake)
        earthquake(py, px, 10);
    if (steal_ct && !*mdeath)
    {
        if (mon_save_p(foe->r_idx, A_DEX))
            msg_print("You fail to run away!");
        else
            teleport_player(25 + p_ptr->lev/2, 0);
    }
}

/**********************************************************************
 * Spells
 **********************************************************************/
void possessor_cast(void)
{
    mon_race_ptr race = &r_info[(p_ptr->pclass == CLASS_BLUE_MAGE) ? MON_SEXY_SWIMSUIT : p_ptr->current_r_idx];
    if (!race->spells)
    {
        if (p_ptr->pclass == CLASS_BLUE_MAGE) msg_print("You have not learned any spells yet.");
        else msg_print("Your current body has no spells.");
        return;
    }
    if (p_ptr->confused)
    {
        msg_print("You are too confused.");
        return;
    }
    if (pelko()) return;
    if (mon_spell_cast_possessor(race))
        energy_use = 100;
}

/**********************************************************************
 * Possession
 **********************************************************************/
static bool _obj_can_possess(object_type *o_ptr)
{
    return o_ptr->tval == TV_CORPSE && o_ptr->sval == SV_CORPSE;
}
static void _possess_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Possess");
        break;
    case SPELL_DESC:
        var_set_string(res, "Enter the corpse of a new body, gaining the powers and abilities of that form.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Lvl %d", _calc_level(p_ptr->max_plv) + 5));
        break;
    case SPELL_CAST:
    {
        obj_prompt_t  prompt = {0};
        monster_race *r_ptr;
        char          name[MAX_NLEN];

        var_set_bool(res, FALSE);

        if (p_ptr->current_r_idx != MON_POSSESSOR_SOUL)
        {
            msg_print("You must leave your current body first. Be careful!");
            return;
        }

        /*if ( p_ptr->current_r_idx != MON_POSSESSOR_SOUL 
          && !get_check("Your current body may be destroyed. Are you sure? ") )
        {
            return;
        }*/

        prompt.prompt = "Possess which corpse?";
        prompt.error = "You have nothing to possess.";
        prompt.filter = _obj_can_possess;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj) return;
        r_ptr = &r_info[prompt.obj->pval];

        object_desc(name, prompt.obj, OD_NAME_ONLY | OD_SINGULAR);
        if (r_ptr->level > _calc_level(p_ptr->max_plv) + 5)
        {
            msg_format("You are not powerful enough to possess %s (Lvl %d).",
                name, r_ptr->level);
            return;
        }

        msg_format("You possess %s.", name);
        if (p_ptr->current_r_idx != MON_POSSESSOR_SOUL)
        {
            if (p_ptr->lev <= 10 || one_in_(3))
            {
                object_type forge;
                object_prep(&forge, lookup_kind(TV_CORPSE, SV_CORPSE));
                apply_magic(&forge, object_level, AM_NO_FIXED_ART);
                forge.pval = p_ptr->current_r_idx;
                forge.weight = MIN(500*10, MAX(40, r_info[p_ptr->current_r_idx].weight * 10));
                drop_near(&forge, -1, py, px);
            }
            else
                msg_print("Your previous body quickly decays!");
        }

        possessor_set_current_r_idx(prompt.obj->pval);
        prompt.obj->number--;
        obj_release(prompt.obj, 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _unpossess_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Unpossess");
        break;
    case SPELL_DESC:
        var_set_string(res, "Leave your current body, returning to your native form. Your current body may be destroyed in the process.");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (p_ptr->current_r_idx == MON_POSSESSOR_SOUL) return; /* paranoia */

        if (get_check("Your current body may be destroyed. Are you sure? "))
        {
            int old_r_idx = p_ptr->current_r_idx;
            monster_race *old_r_ptr = &r_info[old_r_idx];

            msg_print("You leave your current body!");
            if (p_ptr->lev <= 10 || one_in_(3))
            {
                object_type forge;
                object_prep(&forge, lookup_kind(TV_CORPSE, SV_CORPSE));
                apply_magic(&forge, object_level, AM_NO_FIXED_ART);
                forge.pval = old_r_idx;
                forge.weight = MIN(500*10, MAX(40, old_r_ptr->weight * 10));
                drop_near(&forge, -1, py, px);
            }
            else
                msg_print("Your previous body quickly decays!");

            possessor_set_current_r_idx(MON_POSSESSOR_SOUL);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _add_power(spell_info* spell, int lvl, int cost, int fail, ang_spell fn, int stat_idx)
{
    int l = MIN(_max_lvl(), lvl); /* It's frustrating when a corpse can *never* use a given power ... */
    spell->level = l;
    spell->cost = cost;
    spell->fail = calculate_fail_rate(l, fail, stat_idx); 
    spell->fn = fn;
}

int possessor_get_powers(spell_info* spells, int max)
{
    mon_race_ptr race = &r_info[p_ptr->current_r_idx];
    int          ct = 0;
    if (ct < max && (race->flags1 & RF1_TRUMP))
        _add_power(&spells[ct++], 1, 0, 0, blink_toggle_spell, p_ptr->stat_ind[A_DEX]);
    if (ct < max && (race->body.class_idx == CLASS_MAGE || race->body.class_idx == CLASS_HIGH_MAGE || race->body.class_idx == CLASS_SORCERER))
        _add_power(&spells[ct++], 25, 1, 90, eat_magic_spell, p_ptr->stat_ind[A_INT]);
    return ct;
}

static int _get_powers(spell_info* spells, int max)
{
    int ct = 0;

    if (/*p_ptr->current_r_idx == MON_POSSESSOR_SOUL &&*/ ct < max)
        _add_power(&spells[ct++], 1, 0, 0, _possess_spell, p_ptr->stat_ind[A_DEX]);
    if (p_ptr->current_r_idx != MON_POSSESSOR_SOUL && ct < max)
        _add_power(&spells[ct++], 1, 0, 0, _unpossess_spell, p_ptr->stat_ind[A_DEX]);

    ct += possessor_get_powers(spells + ct, max - ct);
    return ct;
}

caster_info *possessor_caster_info(void)
{
    static caster_info info = {0};
    monster_race      *r_ptr = &r_info[p_ptr->current_r_idx];

    /* This is a hack since the mimic's default class
       normally lacks mana. But if we do this, then every time the
       mimic assumes a magical form, they will start with 0sp! */
    if (p_ptr->current_r_idx == MON_MIMIC)
    {
        info.which_stat = r_ptr->body.spell_stat;
        info.magic_desc = "power";
        info.options = 0;
        info.encumbrance.max_wgt = 450;
        info.encumbrance.weapon_pct = 0;
        info.encumbrance.enc_wgt = 800;
        return &info;
    }


    if (r_ptr->body.class_idx)
    {
        class_t *class_ptr = get_class_aux(r_ptr->body.class_idx, 0);
        if (class_ptr && class_ptr->caster_info)
        {
            info = *class_ptr->caster_info();
            info.which_stat = r_ptr->body.spell_stat; /* r_info can now override the default spell stat */
            /*XXX Why? I suppose that many giant forms have HEALING
             * and cast as 'Maulers'. Giving a HP based healing spell
             * would probably be broken! */
            if (info.options & CASTER_USE_HP)
            {
                info.options &= ~CASTER_USE_HP;
                /* Most CASTER_USE_HP info ignores encumbrance */
                info.encumbrance.max_wgt = 3000;
                info.encumbrance.weapon_pct = 0;
                info.encumbrance.enc_wgt = 1200;
            }
            if (info.options & CASTER_SUPERCHARGE_MANA) /* many forms cast as 'Mystics', but cannot 'Concentrate'! */
                info.options &= ~CASTER_SUPERCHARGE_MANA;
            return &info;
        }
    }
    return NULL;
}

/**********************************************************************
 * Bonuses
 * TODO: Someday, we should have a unified flag system ... sigh.
 **********************************************************************/
static int ac_percent;

static void _ac_bonus_imp(int slot)
{
    object_type *o_ptr = equip_obj(slot);
    if (o_ptr)
    {
        switch (equip_slot_type(slot))
        {
        case EQUIP_SLOT_BODY_ARMOR:
            ac_percent -= 25;
            break;
        case EQUIP_SLOT_CLOAK:
            ac_percent -= 5;
            break;
        case EQUIP_SLOT_WEAPON_SHIELD:
            if (object_is_shield(o_ptr))
                ac_percent -= 15;
            break;
        case EQUIP_SLOT_HELMET:
            ac_percent -= 7;
            break;
        case EQUIP_SLOT_GLOVES:
            ac_percent -= 5;
            break;
        case EQUIP_SLOT_BOOTS:
            ac_percent -= 5;
            break;
        }
    }
}

int possessor_r_speed(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];
    int           sp;
    int           r_lvl = MAX(1, r_ptr->level);
    int           p_lvl = _calc_level(p_ptr->lev);

    if (r_ptr->body.speed)
        sp = r_ptr->body.speed;
    else
    {
        sp = (int)r_ptr->speed - 110;
        if (sp > 0)
        {
            int i;
            equip_template_ptr body = &b_info[r_ptr->body.body_idx];
            bool humanoid = FALSE;

            for (i = 1; i <= body->max; i++)
            {
                if (body->slots[i].type == EQUIP_SLOT_WEAPON_SHIELD)
                {
                    humanoid = TRUE;
                    break;
                }
            }

            if (humanoid)
            {
                int factor = 35;
                int tsp = sp * 10;
                sp = 0;
                while (tsp > 0)
                {
                    if (tsp >= 100)
                        sp += factor;
                    else
                        sp += tsp * factor / 100;
 
                    factor /= 2;
                    tsp -= 100;
                }
                sp = sp/10;
            }
        }
    }
    sp = sp * MIN(p_lvl, r_lvl) / r_lvl;
    return sp;
}

int possessor_r_ac(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];
    int           ac = 0;
    int           r_lvl = MAX(1, r_ptr->level);
    int           p_lvl = _calc_level(p_ptr->lev);

    if (r_ptr->flags9 & RF9_POS_GAIN_AC)
    {
        ac = r_ptr->ac * MIN(p_lvl, r_lvl) / r_lvl;

        /* Reduce AC bonus a bit depending on what armor slots are available.
           For example, Wahha-man has AC200 yet can also wear a full complement of armor! */        
        ac_percent = 100;
        equip_for_each_slot(_ac_bonus_imp);
        ac = ac * ac_percent / 100;
    }
    return MAX(0, ac);
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    monster_race *r_ptr = &r_info[p_ptr->current_r_idx];
    if (r_ptr->body.blows_calc.max)
        info_ptr->blows_calc.max = r_ptr->body.blows_calc.max;
    if (r_ptr->body.blows_calc.wgt)
        info_ptr->blows_calc.max = r_ptr->body.blows_calc.wgt;
    if (r_ptr->body.blows_calc.mult)
        info_ptr->blows_calc.max = r_ptr->body.blows_calc.mult;
}

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    if (p_ptr->current_r_idx && !p_ptr->shooter_info.heavy_shoot)
    {
        monster_race *r_ptr = &r_info[p_ptr->current_r_idx];

        if ( r_ptr->body.class_idx == CLASS_RANGER
          && p_ptr->shooter_info.tval_ammo != TV_ARROW )
        {
            p_ptr->shooter_info.base_shot = 100;
        }
        if ( r_ptr->body.class_idx == CLASS_ROGUE
          && p_ptr->shooter_info.tval_ammo != TV_SHOT )
        {
            p_ptr->shooter_info.base_shot = 100;
        }
    }
}

void possessor_calc_bonuses(void) 
{
    monster_race *r_ptr = &r_info[p_ptr->current_r_idx];

    if (!p_ptr->current_r_idx) /* Birth hack ... we haven't been "born" yet! */
        return;

    p_ptr->psex = p_ptr->start_sex;

    if ((r_ptr->flags1 & RF1_FEMALE) && p_ptr->psex != SEX_FEMALE)
    {
        p_ptr->psex = SEX_FEMALE;
    }

    if ((r_ptr->flags1 & RF1_MALE) && p_ptr->psex != SEX_MALE)
    {
        p_ptr->psex = SEX_MALE;
    }

    if (!equip_can_wield_kind(TV_LITE, SV_LITE_FEANOR))
        p_ptr->see_nocto = TRUE;

    {
        int to_a = possessor_r_ac(p_ptr->current_r_idx);
        p_ptr->to_a += to_a;
        p_ptr->dis_to_a += to_a;
    }

    p_ptr->pspeed += possessor_r_speed(p_ptr->current_r_idx);

    if (r_ptr->flags3 & RF3_GOOD)
        p_ptr->align += 200;
    if (r_ptr->flags3 & RF3_EVIL)
        p_ptr->align -= 200;

    if (r_ptr->flags9 & RF9_POS_HOLD_LIFE)
        p_ptr->hold_life++;
    /*if (r_ptr->flags1 & (RF1_RAND_25 | RF1_RAND_50))
        p_ptr->move_random = TRUE;*/
    if (r_ptr->flags9 & RF9_POS_TELEPATHY)
        p_ptr->telepathy = TRUE;
    if (r_ptr->flags9 & RF9_POS_SEE_INVIS)
        p_ptr->see_inv++;
    if (r_ptr->flags2 & RF2_INVISIBLE)
        p_ptr->see_inv++;
    if (r_ptr->flags9 & RF9_POS_BACKSTAB)
        p_ptr->ambush = TRUE;

    if (r_ptr->flags9 & RF9_POS_SUST_STR)
        p_ptr->sustain_str = TRUE;
    if (r_ptr->flags9 & RF9_POS_SUST_INT)
        p_ptr->sustain_int = TRUE;
    if (r_ptr->flags9 & RF9_POS_SUST_WIS)
        p_ptr->sustain_wis = TRUE;
    if (r_ptr->flags9 & RF9_POS_SUST_DEX)
        p_ptr->sustain_dex = TRUE;
    if (r_ptr->flags9 & RF9_POS_SUST_CON)
        p_ptr->sustain_con = TRUE;
    if (r_ptr->flags9 & RF9_POS_SUST_CHR)
        p_ptr->sustain_chr = TRUE;

    if (r_ptr->flags2 & RF2_REFLECTING)
        p_ptr->reflect = TRUE;
    if (r_ptr->flags2 & RF2_REGENERATE)
        p_ptr->regen += 100;
    if ((r_ptr->flags2 & RF2_ELDRITCH_HORROR) || strchr("GLUVW", r_ptr->d_char))
        p_ptr->no_eldritch = TRUE;
    if (r_ptr->flags2 & RF2_AURA_FIRE)
        p_ptr->sh_fire++;
    if (r_ptr->flags2 & RF2_AURA_ELEC)
        p_ptr->sh_elec++;
    if (r_ptr->flags3 & RF3_AURA_COLD)
        p_ptr->sh_cold++;
    if (r_ptr->flags2 & RF2_PASS_WALL)
    {
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (r_ptr->flags2 & RF2_KILL_WALL)
        p_ptr->kill_wall = TRUE;
    if (r_ptr->flags2 & RF2_AURA_REVENGE)
        p_ptr->sh_retaliation = TRUE;
    if (r_ptr->flags2 & RF2_AURA_FEAR)
        p_ptr->sh_fear = TRUE;

    if (r_ptr->flags3 & RF3_HURT_LITE)
        res_add_vuln(RES_LITE);
    if (r_ptr->flags3 & RF3_HURT_FIRE)
        res_add_vuln(RES_FIRE);
    if (r_ptr->flags3 & RF3_HURT_COLD)
        res_add_vuln(RES_COLD);
    if (r_ptr->flags3 & RF3_NO_FEAR)
        res_add(RES_FEAR);
    if (r_ptr->flags3 & RF3_NO_STUN)
        p_ptr->no_stun = TRUE;
    if (r_ptr->flags3 & RF3_NO_CONF)
        res_add(RES_CONF);
    if (r_ptr->flags3 & RF3_NO_SLEEP)
        p_ptr->free_act++;

    if (r_ptr->flags7 & RF7_CAN_FLY)
        p_ptr->levitation = TRUE;

    if (r_ptr->flagsr & RFR_IM_ACID)
        res_add_immune(RES_ACID);
    if (r_ptr->flagsr & RFR_IM_ELEC)
        res_add_immune(RES_ELEC);
    if (r_ptr->flagsr & RFR_IM_FIRE)
        res_add_immune(RES_FIRE);
    if (r_ptr->flagsr & RFR_IM_COLD)
        res_add_immune(RES_COLD);
    if (r_ptr->flagsr & RFR_IM_POIS)
        res_add_immune(RES_POIS);

    if (r_ptr->flagsr & RFR_RES_ACID)
        res_add(RES_ACID);
    if (r_ptr->flagsr & RFR_RES_ELEC)
        res_add(RES_ELEC);
    if (r_ptr->flagsr & RFR_RES_FIRE)
        res_add(RES_FIRE);
    if (r_ptr->flagsr & RFR_RES_COLD)
        res_add(RES_COLD);
    if (r_ptr->flagsr & RFR_RES_POIS)
        res_add(RES_POIS);
    if (r_ptr->flagsr & RFR_RES_LITE)
        res_add(RES_LITE);
    if (r_ptr->flagsr & RFR_RES_DARK)
        res_add(RES_DARK);
    if (r_ptr->flagsr & RFR_RES_NETH)
        res_add(RES_NETHER);
    if (r_ptr->flagsr & RFR_RES_SHAR)
        res_add(RES_SHARDS);
    if (r_ptr->flagsr & RFR_RES_SOUN)
        res_add(RES_SOUND);
    if (r_ptr->flagsr & RFR_RES_CHAO)
        res_add(RES_CHAOS);
    if (r_ptr->flagsr & RFR_RES_NEXU)
        res_add(RES_NEXUS);
    if (r_ptr->flagsr & RFR_RES_DISE)
        res_add(RES_DISEN);
    if (r_ptr->flagsr & RFR_RES_TIME)
        res_add(RES_TIME);
    if (r_ptr->flagsr & RFR_RES_TELE)
        res_add(RES_TELEPORT);
    if (r_ptr->flagsr & RFR_RES_ALL)
    {
        res_add_all();
        if (p_ptr->current_r_idx == MON_SPELLWARP_AUTOMATON)
            p_ptr->magic_resistance = 35;
        else
            p_ptr->magic_resistance = 95;
    }

    if (strchr("sGLVWz", r_ptr->d_char))
        p_ptr->no_cut = TRUE;
    if (strchr("sg", r_ptr->d_char))
        p_ptr->no_stun = TRUE;
}

void possessor_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    monster_race *r_ptr = &r_info[p_ptr->current_r_idx];

    if (r_ptr->speed != 110)
        add_flag(flgs, OF_SPEED);

    if (r_ptr->flags9 & RF9_POS_HOLD_LIFE)
        add_flag(flgs, OF_HOLD_LIFE);
    if (r_ptr->flags9 & RF9_POS_TELEPATHY)
        add_flag(flgs, OF_TELEPATHY);
    if (r_ptr->flags9 & RF9_POS_SEE_INVIS)
        add_flag(flgs, OF_SEE_INVIS);
    if (r_ptr->flags9 & RF9_POS_SUST_STR)
        add_flag(flgs, OF_SUST_STR);
    if (r_ptr->flags9 & RF9_POS_SUST_INT)
        add_flag(flgs, OF_SUST_INT);
    if (r_ptr->flags9 & RF9_POS_SUST_WIS)
        add_flag(flgs, OF_SUST_WIS);
    if (r_ptr->flags9 & RF9_POS_SUST_DEX)
        add_flag(flgs, OF_SUST_DEX);
    if (r_ptr->flags9 & RF9_POS_SUST_CON)
        add_flag(flgs, OF_SUST_CON);
    if (r_ptr->flags9 & RF9_POS_SUST_CHR)
        add_flag(flgs, OF_SUST_CHR);

    if (r_ptr->flags2 & RF2_REFLECTING)
        add_flag(flgs, OF_REFLECT);
    if (r_ptr->flags2 & RF2_REGENERATE)
        add_flag(flgs, OF_REGEN);
    if (r_ptr->flags2 & RF2_AURA_FIRE)
        add_flag(flgs, OF_AURA_FIRE);
    if (r_ptr->flags2 & RF2_AURA_ELEC)
        add_flag(flgs, OF_AURA_ELEC);

    if (r_ptr->flags3 & RF3_AURA_COLD)
        add_flag(flgs, OF_AURA_COLD);
    if (r_ptr->flags3 & RF3_NO_FEAR)
        add_flag(flgs, OF_RES_FEAR);
    if (r_ptr->flags3 & RF3_NO_CONF)
        add_flag(flgs, OF_RES_CONF);
    if (r_ptr->flags3 & RF3_NO_SLEEP)
        add_flag(flgs, OF_FREE_ACT);
    if (r_ptr->flags2 & RF2_AURA_REVENGE)
        add_flag(flgs, OF_AURA_REVENGE);

    if (r_ptr->flags7 & RF7_CAN_FLY)
        add_flag(flgs, OF_LEVITATION);

    if (r_ptr->flagsr & RFR_RES_ACID)
        add_flag(flgs, OF_RES_ACID);
    if (r_ptr->flagsr & RFR_RES_ELEC)
        add_flag(flgs, OF_RES_ELEC);
    if (r_ptr->flagsr & RFR_RES_FIRE)
        add_flag(flgs, OF_RES_FIRE);
    if (r_ptr->flagsr & RFR_RES_COLD)
        add_flag(flgs, OF_RES_COLD);
    if (r_ptr->flagsr & RFR_RES_POIS)
        add_flag(flgs, OF_RES_POIS);
    if (r_ptr->flagsr & RFR_RES_LITE)
        add_flag(flgs, OF_RES_LITE);
    if (r_ptr->flagsr & RFR_RES_DARK)
        add_flag(flgs, OF_RES_DARK);
    if (r_ptr->flagsr & RFR_RES_NETH)
        add_flag(flgs, OF_RES_NETHER);
    if (r_ptr->flagsr & RFR_RES_SHAR)
        add_flag(flgs, OF_RES_SHARDS);
    if (r_ptr->flagsr & RFR_RES_SOUN)
        add_flag(flgs, OF_RES_SOUND);
    if (r_ptr->flagsr & RFR_RES_CHAO)
        add_flag(flgs, OF_RES_CHAOS);
    if (r_ptr->flagsr & RFR_RES_NEXU)
        add_flag(flgs, OF_RES_NEXUS);
    if (r_ptr->flagsr & RFR_RES_DISE)
        add_flag(flgs, OF_RES_DISEN);
    if (r_ptr->flagsr & RFR_RES_TIME)
        add_flag(flgs, OF_RES_TIME);
    if (r_ptr->flagsr & RFR_RES_ALL)
    {
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_LITE);
        add_flag(flgs, OF_RES_DARK);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_RES_NEXUS);
        add_flag(flgs, OF_RES_SOUND);
        add_flag(flgs, OF_RES_SHARDS);
        add_flag(flgs, OF_RES_CHAOS);
        add_flag(flgs, OF_RES_DISEN);
        add_flag(flgs, OF_RES_TIME);
    }

    if (r_ptr->flagsr & RFR_IM_ACID)
        add_flag(flgs, OF_IM_ACID);
    if (r_ptr->flagsr & RFR_IM_ELEC)
        add_flag(flgs, OF_IM_ELEC);
    if (r_ptr->flagsr & RFR_IM_FIRE)
        add_flag(flgs, OF_IM_FIRE);
    if (r_ptr->flagsr & RFR_IM_COLD)
        add_flag(flgs, OF_IM_COLD);
    if (r_ptr->flagsr & RFR_IM_POIS)
        add_flag(flgs, OF_IM_POIS);

    if (r_ptr->flags3 & RF3_HURT_LITE)
        add_flag(flgs, OF_VULN_LITE);
    if (r_ptr->flags3 & RF3_HURT_FIRE)
        add_flag(flgs, OF_VULN_FIRE);
    if (r_ptr->flags3 & RF3_HURT_COLD)
        add_flag(flgs, OF_VULN_COLD);
}

/**********************************************************************
 * Public
 **********************************************************************/
void possessor_init_race_t(race_t *race_ptr, int default_r_idx)
{
    static int last_r_idx = -1;
    int        r_idx = p_ptr->current_r_idx, i;

    if (!r_idx) /* Birthing menus. p_ptr->prace not chosen yet. _birth() not called yet. */
        r_idx = default_r_idx; 

    if (r_idx != last_r_idx)
    {
        monster_race *r_ptr;
    
        if (p_ptr->current_r_idx == r_idx) /* Birthing menus. current_r_idx = 0 but r_idx = default_r_idx. */
            last_r_idx = r_idx;            /* BTW, the game really needs a "current state" concept ... */

        r_ptr = &r_info[r_idx];

        race_ptr->base_hp = 15;

        race_ptr->get_spells = NULL;
        race_ptr->caster_info = NULL;
        if (r_ptr->body.spell_stat != A_NONE)
        {
            /*race_ptr->get_spells = possessor_get_spells;*/
            race_ptr->caster_info = possessor_caster_info;
        }

        race_ptr->infra = r_ptr->body.infra;

        race_ptr->life = r_ptr->body.life;
        if (!race_ptr->life)
            race_ptr->life = 100;
    
        race_ptr->equip_template = mon_get_equip_template();

        for (i = 0; i < MAX_STATS; i++)
            race_ptr->stats[i] = r_ptr->body.stats[i];

        race_ptr->skills = r_ptr->body.skills;
        race_ptr->extra_skills = r_ptr->body.extra_skills;

        race_ptr->pseudo_class_idx = r_ptr->body.class_idx;

        race_ptr->subname = mon_name(r_idx);

        race_ptr->flags = RACE_IS_MONSTER;
        if (r_ptr->flags3 & RF3_UNDEAD)
            race_ptr->flags |= RACE_IS_UNDEAD | RACE_IS_NONLIVING;
        if (r_ptr->flags3 & RF3_NONLIVING)
            race_ptr->flags |= RACE_IS_NONLIVING;
        if (r_ptr->flags3 & RF3_DEMON)
            race_ptr->flags |= RACE_IS_DEMON | RACE_IS_NONLIVING;
    }
    if (birth_hack || spoiler_hack)
    {
        race_ptr->subname = NULL;
        race_ptr->subdesc = NULL;
    }
}
race_t *mon_possessor_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;

    if (!init)
    {
        me.name = "Possessor";
        me.desc = "The Possessor is an odd creature, completely harmless in its natural form. However, they "
                    "are capable of possessing the corpses of monsters they have slain, and gain powers and "
                    "abilities based on their current body. As such, they can become quite powerful indeed! "
                    "Unfortunately, not every type of monster will drop a corpse, and getting suitable corspes "
                    "to inhabit can be difficult. If the possessor ever leaves their current body then all of "
                    "their equipment will be removed (except a "
                    "light source) and they will temporarily be in their native, vulnerable state. Finally, "
                    "leaving their current body will destroy that corpse most of the time, so the possessor "
                    "should only do so if they have a better corpse on hand (and also only if there are no "
                    "monsters nearby!).\n \n"
                    "Possessors are monsters and do not choose a normal class. Their stats, skills, resistances "
                    "and spells are completely determined by the body they inhabit. Their current body also "
                    "determines their spell stat (e.g. a novice priest uses wisdom, a novice mage uses intelligence). "
                    "Their current body may offer innate powers (e.g. breath weapons or rockets) in addition to or in lieu "
                    "of magical powers (e.g. mana storms and frost bolts). Be sure to check both the racial power "
                    "command ('U') and the magic command ('m') after possessing a new body.";

        me.exp = 250;
        me.shop_adjust = 110; /* Really should depend on current form */

        me.birth = _birth;

        me.get_powers = _get_powers;

        me.calc_bonuses = possessor_calc_bonuses;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.get_flags = possessor_get_flags;
        me.player_action = _player_action;
        me.save_player = possessor_on_save;
        me.load_player = possessor_on_load;
        me.character_dump = possessor_character_dump;
        
        init = TRUE;
    }

    possessor_init_race_t(&me, MON_POSSESSOR_SOUL);
    return &me;
}

bool possessor_can_gain_exp(void)
{
    int max = _max_lvl();
    if (max < PY_MAX_LEVEL && p_ptr->lev >= max)
        return FALSE;
    return TRUE;
}

s32b possessor_max_exp(void)
{
    int max = _max_lvl();
    if (max < PY_MAX_LEVEL)
        return exp_requirement(max) - 1;
    else
        return 99999999;
}

void possessor_on_take_hit(void)
{
    /* Getting too wounded may eject the possessor! */
    if ( p_ptr->chp < p_ptr->mhp/4
      && p_ptr->current_r_idx != MON_POSSESSOR_SOUL )
    {
        if (one_in_(66))
        {
            int old_r_idx = p_ptr->current_r_idx;
            monster_race *old_r_ptr = &r_info[old_r_idx];

            msg_print("You can no longer maintain your current body!");
            if (one_in_(3))
            {
                object_type forge;
                object_prep(&forge, lookup_kind(TV_CORPSE, SV_CORPSE));
                apply_magic(&forge, object_level, AM_NO_FIXED_ART);
                forge.pval = old_r_idx;
                forge.weight = MIN(500*10, MAX(40, old_r_ptr->weight * 10));
                drop_near(&forge, -1, py, px);
            }
            else
                msg_print("Your previous body quickly decays!");

            possessor_set_current_r_idx(MON_POSSESSOR_SOUL);
            p_ptr->chp = p_ptr->mhp; /* Be kind. This effect is nasty! */
            p_ptr->chp_frac = 0;
        }
        else
        {
            msg_print("You struggle to maintain possession of your current body!");
        }
    }
}

void possessor_set_current_r_idx(int r_idx)
{
    if (r_idx != p_ptr->current_r_idx)
    {
        int mana_ratio = p_ptr->csp * 100 / MAX(1, p_ptr->msp);

        p_ptr->magic_num1[0] = 0; /* Blinking Death ... */
        p_ptr->current_r_idx = r_idx;
        lore_do_probe(r_idx);
        if (!r_info[r_idx].r_sights) r_info[r_idx].r_sights = 1;

        if (p_ptr->exp > possessor_max_exp())
        {
            p_ptr->exp = possessor_max_exp();
            check_experience();
        }
        else
            restore_level();

        p_ptr->update |= PU_BONUS | PU_HP | PU_MANA;
        p_ptr->redraw |= PR_MAP | PR_BASIC | PR_MANA | PR_EXP | PR_EQUIPPY;

        /* Apply the new body type to our equipment */
        equip_on_change_race();

        /* Mimic's shift alot. Try to preserve the old mana ratio if possible. */
        if (p_ptr->prace == RACE_MON_MIMIC)
        {
            handle_stuff();

            p_ptr->csp = p_ptr->msp * mana_ratio / 100;
            p_ptr->csp_frac = 0;
            p_ptr->redraw |= PR_MANA;
            p_ptr->window |= PW_SPELL;

            if (p_ptr->current_r_idx != MON_MIMIC)
                _history_on_possess(r_idx);
        }
        else if (p_ptr->current_r_idx != MON_POSSESSOR_SOUL)
            _history_on_possess(r_idx);    
    }
}

void possessor_do_auras(mon_ptr mon)
{
    mon_race_ptr race;
    int          i;
    if (p_ptr->prace != RACE_MON_POSSESSOR && p_ptr->prace != RACE_MON_MIMIC) return;
    race = &r_info[p_ptr->current_r_idx];
    for (i = 0; i < MAX_MON_AURAS; i++)
    {
        mon_effect_ptr aura = &race->auras[i];
        int            dam;
        if (!aura->effect) continue;
        if (aura->pct && randint1(100) > aura->pct) continue;
        dam = damroll(aura->dd, aura->ds);
        if (!dam) continue;
        gf_affect_m(GF_WHO_PLAYER, mon, aura->effect, dam, GF_AFFECT_AURA);
    }
}

void possessor_explode(int dam)
{
    if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
    {
        int           i;
        monster_race *r_ptr = &r_info[p_ptr->current_r_idx];

        for (i = 0; i < MAX_MON_BLOWS; i++)
        {
            if (r_ptr->blows[i].method == RBM_EXPLODE)
            {
                int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
                int typ = r_ptr->blows[i].effects[0].effect;
                project(0, 3, py, px, dam, typ, flg);
                break;
            }
        }

        if (p_ptr->prace == RACE_MON_MIMIC)
            possessor_set_current_r_idx(MON_MIMIC);
        else
            possessor_set_current_r_idx(MON_POSSESSOR_SOUL);

        take_hit(DAMAGE_NOESCAPE, dam, "Exploding");
        set_stun(p_ptr->stun + 10, FALSE);
    }
}

void possessor_character_dump(doc_ptr doc)
{
    _history_ptr p = _history;
    int          ct = 0;
    char         lvl[80];
    char         loc[255];
    
    if (p_ptr->current_r_idx != MON_MIMIC && p_ptr->current_r_idx != MON_POSSESSOR_SOUL)
    {
        mon_race_ptr race = &r_info[p_ptr->current_r_idx];
        bool old_use_graphics = use_graphics;
        use_graphics = FALSE;
        doc_printf(doc, "<topic:CurrentForm>================================ <color:keypress>C</color>urrent Form =================================\n\n");
        mon_display_possessor(race, doc);
        doc_newline(doc);
        use_graphics = old_use_graphics;
    }
    doc_printf(doc, "<topic:RecentForms>================================ <color:keypress>R</color>ecent Forms =================================\n\n");
    doc_insert(doc, "<style:table>");
    doc_printf(doc, "<color:G>%-33.33s CL Day  Time  DL %-28.28s</color>\n", "Most Recent Forms", "Location");

    while (p && ct < 100)
    {
        int day, hour, min;
        extract_day_hour_min_imp(p->turn, &day, &hour, &min);

        switch (p->d_idx)
        {
        case DUNGEON_QUEST:
            sprintf(loc, "%s", quests_get(p->d_lvl)->name);
            sprintf(lvl, "%3d", quests_get(p->d_lvl)->level);
            break;
        case DUNGEON_TOWN:
            sprintf(loc, "%s", town_name(p->d_lvl));
            sprintf(lvl, "%s", "   ");
            break;
        default: /* DUNGEON_WILD is present in the pref file d_info.txt with the name: Wilderness */
            sprintf(loc, "%s", d_name + d_info[p->d_idx].name);
            if (p->d_lvl)
                sprintf(lvl, "%3d", p->d_lvl);
            else
                sprintf(lvl, "%s", "   ");
            break;
        }
        doc_printf(doc, "%-33.33s %2d %3d %2d:%02d %s %-25.25s\n",
            mon_name(p->r_idx), 
            p->p_lvl, 
            day, hour, min,
            lvl, loc
        );
        p = p->next;
        ct++;
    }
    doc_insert(doc, "</style>");
    doc_newline(doc);
}

void possessor_on_save(savefile_ptr file)
{
    _history_on_save(file);
}

void possessor_on_load(savefile_ptr file)
{
    _history_on_load(file);
}

