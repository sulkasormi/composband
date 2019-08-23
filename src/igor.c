#include "angband.h"

#define _MAX_PACK_SLOTS 6

#define _IB_HEAD 1
#define _IB_EYES 2
#define _IB_EARS 3
#define _IB_STOMACH 4
#define _IB_HEART 5
#define _IB_HANDS 6
#define _IB_LEGS 7
#define _IB_MAX_ACTIVE 7

static inv_ptr _igor_cold_pack = NULL;
static inv_ptr _igor_body = NULL;
static bool _pack_initialized = FALSE;
static bool _do_init_pack = FALSE;
static bool _igor_birth_hack = FALSE;

/* Why did I make ears be sval 8? They'd all be neatly in sequence otherwise */
static int _ibtosval(int ib)
{
    switch (ib)
    {
        case _IB_HEAD: return SV_BODY_HEAD;
        case _IB_EYES: return SV_BODY_EYES;
        case _IB_EARS: return SV_BODY_EARS;
        case _IB_STOMACH: return SV_BODY_STOMACH;
        case _IB_HEART: return SV_BODY_HEART;
        case _IB_HANDS: return SV_BODY_HANDS;
        case _IB_LEGS: return SV_BODY_LEGS;
        default: return 0;
    }
}

void _igor_pack_init(void)
{
    if ((_pack_initialized) && (!_do_init_pack)) return;
    inv_free(_igor_cold_pack);
    inv_free(_igor_body);
    _igor_cold_pack = inv_alloc("Ice Bag", INV_SPECIAL1, _MAX_PACK_SLOTS);
    _igor_body = inv_alloc("Body", INV_SPECIAL3, _IB_MAX_ACTIVE);
    _pack_initialized = TRUE;
    _do_init_pack = FALSE;
}

static bool _object_is_corpse(object_type *o_ptr)
{
    return ((object_is_(o_ptr, TV_CORPSE, SV_CORPSE)) && (o_ptr->pval > 0) && (o_ptr->pval < max_r_idx));
}

static void _igor_set_origins(object_type *o_ptr)
{
    object_origins(o_ptr, ORIGIN_DROP);
    if ((o_ptr->sval < SV_BODY_HEAD) || (o_ptr->sval == SV_BODY_EARS)) o_ptr->origin_xtra = o_ptr->pval;
    else o_ptr->origin_xtra = o_ptr->xtra4;
}

static void _ears_of(int kuka)
{
    object_type *q_ptr, forge;
    q_ptr = &forge;

    object_prep(q_ptr, lookup_kind(TV_CORPSE, SV_BODY_EARS));
    q_ptr->pval = kuka;
    _igor_set_origins(q_ptr);
    obj_identify(q_ptr);
    pack_carry(q_ptr);
}

static slot_t _igor_object_body_slot(object_type *o_ptr)
{
    if ((!o_ptr) || (!o_ptr->k_idx)) return 0;
    if (o_ptr->name1 == ART_EYE_OF_VECNA) return _IB_EYES;
    if (o_ptr->name1 == ART_ALL_SEEING_EYE) return _IB_EYES;
    if (o_ptr->name1 == ART_HYPNO) return _IB_EYES;
    if (o_ptr->name1 == ART_HAND_OF_VECNA) return _IB_HANDS;
    if (o_ptr->tval != TV_CORPSE) return 0;
    if (o_ptr->sval > SV_BODY_EARS) return 0;
    if (o_ptr->sval < SV_BODY_HEAD) return 0;
    if (o_ptr->sval <= SV_BODY_EYES) return (o_ptr->sval - SV_BODY_HEAD + 1);
    if (o_ptr->sval == SV_BODY_EARS) return _IB_EARS;
    return (o_ptr->sval - SV_BODY_HEAD + 2);
}

static bool _object_is_body_part(object_type *o_ptr)
{
    if ((!o_ptr) || (!o_ptr->k_idx)) return 0;
//    if ((_allow_ears_hack) && (object_is_(o_ptr, TV_CORPSE, SV_BODY_EARS))) return TRUE;
    return (_igor_object_body_slot(o_ptr) > 0);
}

static bool _freeze_carry(object_type *o_ptr, bool selita)
{
    slot_t slot;
    char o_name[MAX_NLEN];
    object_desc(o_name, o_ptr, OD_COLOR_CODED);
    slot = inv_add(_igor_cold_pack, o_ptr);
    if (slot)
        msg_format("You freeze %s.", o_name);
    else if (selita)
    {
        msg_format("There is no room in your ice bag.");
        return FALSE;
    }
    else
    {
        pack_carry(o_ptr);
    }

    if ((o_ptr) && (o_ptr->k_idx))
    {
        o_ptr->number = 0;
        obj_release(o_ptr, OBJ_RELEASE_QUIET);
    }
    return TRUE;
}

static bool _igor_carry(object_type *o_ptr)
{
    slot_t slot = _igor_object_body_slot(o_ptr);
    object_type *ulos;
//    msg_format("Sopiva paikka: %d", slot);
    if ((slot < 1) || (slot > _IB_MAX_ACTIVE)) return FALSE;
    ulos = inv_obj(_igor_body, slot);
    if ((ulos) && (ulos->k_idx))
    {
        if (object_is_cursed(ulos))
        {
            char o_name[MAX_NLEN];
            object_desc(o_name, ulos, OD_COLOR_CODED);
            msg_format("You cannot replace your cursed %s!", o_name);
            return FALSE;
        }
        _freeze_carry(ulos, FALSE);
        inv_remove(_igor_body, slot);
    }
    if (!_igor_birth_hack)
    {
        static const char *kuvaukset[_IB_MAX_ACTIVE] = {"head", "eyes", "ears", "stomach", "heart", "hands", "legs"};
        char o_name[MAX_NLEN];
        object_desc(o_name, o_ptr, OD_COLOR_CODED);
        msg_format("You replace your %s with %s!", kuvaukset[slot - 1], o_name);
    }
    inv_add_at(_igor_body, o_ptr, slot);
    
    if (!_igor_birth_hack) obj_release(o_ptr, OBJ_RELEASE_QUIET);

    if (_igor_birth_hack) return TRUE;

    p_ptr->update |= (PU_BONUS | PU_TORCH | PU_MANA | PU_HP);
    p_ptr->redraw |= (PR_EQUIPPY | PR_EFFECTS | PR_STATUS);
    p_ptr->window |= (PW_INVEN | PW_EQUIP);

    handle_stuff();
    return TRUE;
}

static bool _equip_body_part(bool surgery)
{
    obj_prompt_t prompt = {0};
    if (surgery)
    {
        prompt.prompt = "Use which body part?";
        prompt.error = "You have no available body parts.";
    }
    else
    {
        prompt.prompt = "Freeze which body part?";
        prompt.error = "You have no body parts to freeze.";
    }
    prompt.filter = _object_is_body_part;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;
    if (!surgery) prompt.where[2] = INV_SPECIAL1;
    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;
    if (surgery) return _igor_carry(prompt.obj);
    else
    {
        return _freeze_carry(prompt.obj, TRUE);
    }
}

bool mon_matches_obj_res(monster_race *r_ptr, int which)
{
    if ((!r_ptr) || (!r_ptr->name)) return FALSE;
    if (r_ptr->flagsr & RFR_RES_ALL) return TRUE;
    switch (which)
    {
        case OF_RES_ACID: return (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK) ? TRUE : FALSE;
        case OF_RES_ELEC: return (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK) ? TRUE : FALSE;
        case OF_RES_FIRE: return (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK) ? TRUE : FALSE;
        case OF_RES_COLD: return (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK) ? TRUE : FALSE;
        case OF_RES_POIS: return (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK) ? TRUE : FALSE;
        case OF_RES_DARK: return (r_ptr->flagsr & RFR_EFF_RES_DARK_MASK) ? TRUE : FALSE;
        case OF_RES_SHARDS: return (r_ptr->flagsr & RFR_EFF_RES_SHAR_MASK) ? TRUE : FALSE;
        case OF_RES_CHAOS: return (r_ptr->flagsr & RFR_EFF_RES_CHAO_MASK) ? TRUE : FALSE;
        case OF_RES_NEXUS: return (r_ptr->flagsr & RFR_EFF_RES_NEXU_MASK) ? TRUE : FALSE;
        case OF_RES_TIME: return (r_ptr->flagsr & RFR_EFF_RES_TIME_MASK) ? TRUE : FALSE;
        case OF_RES_NETHER: return ((r_ptr->flagsr & RFR_RES_NETH) || (r_ptr->flags3 & RF3_UNDEAD)) ? TRUE : FALSE;
        case OF_RES_LITE: return (r_ptr->flagsr & RFR_RES_LITE) ? TRUE : FALSE;
        case OF_RES_DISEN: return (r_ptr->flagsr & RFR_RES_DISE) ? TRUE : FALSE;
        case OF_RES_SOUND: return (r_ptr->flagsr & RFR_RES_SOUN) ? TRUE : FALSE;
        case OF_RES_CONF: return (r_ptr->flags3 & RF3_NO_CONF) ? TRUE : FALSE;
        case OF_RES_FEAR: return (r_ptr->flags3 & RF3_NO_FEAR) ? TRUE : FALSE;
        default: break;
    }
    return FALSE;
}

static bool _igor_prob(int _in, bool unique, int taito)
{
    static bool lippu = FALSE;
    s32b prob;
    if (!_in)
    {
        lippu = FALSE;
        return FALSE;
    }
    if (_in < 0) return FALSE;
    prob = 4620 / _in;
    if (unique) prob += (prob / 6);
    if ((!lippu) && (taito > 40)) prob += (prob / (600 / taito));
    if (randint0(4620) < prob)
    {
        if (!unique) lippu = TRUE;
        return TRUE;
    }
    return FALSE;
}

static bool _dissect_corpse(void)
{
    obj_prompt_t prompt = {0};
    bool _wanted = FALSE;
    byte sopiva = 0, tyyppi = 0;
    monster_race *r_ptr;
    equip_template_ptr et_ptr;
    bool _okei[_IB_MAX_ACTIVE] = {0};
    int slot;
    int taito = MIN(p_ptr->lev + 50, (p_ptr->stat_ind[A_DEX] + 2) * 5 / 2);
    bool unique, living;

    prompt.prompt = "Dissect which corpse?";
    prompt.error = "You have no corpses to dissect.";
    prompt.filter = _object_is_corpse;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;
    r_ptr = &r_info[prompt.obj->pval];
    if ((!r_ptr) || (!r_ptr->name)) return FALSE; /* paranoia */
    et_ptr = &b_info[r_ptr->body.body_idx];

    _wanted = object_is_shoukinkubi(prompt.obj);
    unique = (r_ptr->flags1 & RF1_UNIQUE) ? TRUE : FALSE;
    living = monster_living(r_ptr);

    if ((_wanted) && (!unique))
    {
        if (!get_check("Really dissect a wanted corpse? ")) return FALSE;
    }

    for (slot = 1; slot <= et_ptr->max; slot++)
    {
        if (et_ptr->slots[slot].type == EQUIP_SLOT_HELMET)
        {
            _okei[_IB_HEAD - 1] = TRUE;
            if (living) _okei[_IB_EARS - 1] = TRUE;
        }
        if (et_ptr->slots[slot].type == EQUIP_SLOT_GLOVES) _okei[_IB_HANDS - 1] = TRUE;
        if (((et_ptr->slots[slot].type == EQUIP_SLOT_BODY_ARMOR) ||
             (et_ptr->slots[slot].type == EQUIP_SLOT_CLOAK)) && (living))
        {
            _okei[_IB_STOMACH - 1] = TRUE;
            _okei[_IB_HEART - 1] = TRUE;
        }
        if (et_ptr->slots[slot].type == EQUIP_SLOT_BOOTS) _okei[_IB_LEGS - 1] = TRUE;
    }
    _okei[_IB_EYES - 1] = TRUE;

    for (slot = 0; slot < _IB_MAX_ACTIVE; slot++)
    {
        if ((_okei[slot]) && ((unique) || (one_in_(4))))
        {
            if ((slot == _IB_EARS - 1) && (prompt.obj->pval != MON_DONKEY) && (!one_in_(4))) continue;
            sopiva++;
            if (one_in_(sopiva)) tyyppi = slot + 1;
        }
    }

    if ((!tyyppi) || (randint1(100) > taito))
    {
        char o_name[MAX_NLEN];
        int _pval = prompt.obj->pval;
        object_desc(o_name, prompt.obj, OD_OMIT_PREFIX | OD_COLOR_CODED | OD_SINGULAR);

        if ((_wanted) && (!unique)) _wanted = FALSE;

        msg_format("You dissect the %s, but find nothing %s.", o_name, _wanted ? "useful" : "worth keeping");
        prompt.obj->number--;
        obj_release(prompt.obj, 0);
        if (_wanted) _ears_of(_pval); /* Keep the ears so we can turn them in */
        p_ptr->window |= (PW_INVEN);
    }
    else /* Successfully salvaged a body part */
    {
        object_type *q_ptr, forge;
        int voima = r_ptr->level / 2 + 5;
        int _my_pval = 0;
        q_ptr = &forge;

        object_prep(q_ptr, lookup_kind(TV_CORPSE, _ibtosval(tyyppi)));
        q_ptr->xtra4 = prompt.obj->pval; /* Careful - the "pval" parameter might go to an actual pval! */
        q_ptr->xtra5 = 0;
        q_ptr->pval = 0;
        q_ptr->to_h = 0;
        q_ptr->to_d = 0;
        q_ptr->to_a = 0;
        (void)_igor_prob(0, 0, 0);

        switch (tyyppi)
        {
            case _IB_HEAD:
            {
                bool lippu = FALSE;
                if ((one_in_(12)) && (randint0(600) < taito))
                {
                    int _int = r_ptr->body.stats[A_INT];
                    if (_int)
                    {
                        bool merkki = (_int > 0) ? TRUE : FALSE;
                        _int = ABS(_int);
                        _my_pval = m_bonus(_int, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 3, merkki ? voima : 70);
                        while (_my_pval > 0)
                        {
                            if ((merkki) && (one_in_(4)))
                            {
                                add_flag(q_ptr->flags, OF_SUST_INT);
                                break;
                            }
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki)
                            {
                                add_flag(q_ptr->flags, OF_INT);
                                if (one_in_(8)) add_flag(q_ptr->flags, OF_SUST_INT);
                            }
                            else add_flag(q_ptr->flags, OF_DEC_INT);
                            lippu = TRUE;
                            break;
                        }
                    }
                }
                if ((randint0(200) < taito) && (_igor_prob(4, unique, taito)))
                {
                    int _wis = r_ptr->body.stats[A_WIS];
                    if (_wis)
                    {
                        bool merkki = (_wis > 0) ? TRUE : FALSE;
                        _wis = ABS(_wis);
                        _my_pval = m_bonus(_wis, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 3, merkki ? voima : 70);
                        while (_my_pval > 0)
                        {
                            if ((merkki) && (one_in_(4)))
                            {
                                add_flag(q_ptr->flags, OF_SUST_WIS);
                                break;
                            }
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki)
                            {
                                add_flag(q_ptr->flags, OF_WIS);
                                if (one_in_(8)) add_flag(q_ptr->flags, OF_SUST_WIS);
                            }
                            else add_flag(q_ptr->flags, OF_DEC_WIS);
                            lippu = TRUE;
                            break;
                        }
                    }
                }
                if (_igor_prob(2, unique, taito))
                {
                    int i, _chr = r_ptr->body.stats[A_CHR];
                    for (i = 0; i < 5; i++)
                    {
                        if (randint0(100) < taito) _chr--;
                    }
                    if (_chr)
                    {
                        bool merkki = (_chr > 0) ? TRUE : FALSE;
                        _chr = ABS(_chr);
                        _my_pval = m_bonus(_chr, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 3, merkki ? voima : 70);
                        while (_my_pval > 0)
                        {
                            if ((merkki) && (one_in_(4)))
                            {
                                add_flag(q_ptr->flags, OF_SUST_CHR);
                                break;
                            }
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki)
                            {
                                add_flag(q_ptr->flags, OF_CHR);
                                if (one_in_(8)) add_flag(q_ptr->flags, OF_SUST_CHR);
                            }
                            else add_flag(q_ptr->flags, OF_DEC_CHR);
                            lippu = TRUE;
                            break;
                        }
                    }
                }
                if ((r_ptr->ac > 100) && (randint1(363) < taito))
                {
                    int _ac = (r_ptr->ac - 91) / 10;
                    q_ptr->to_a = m_bonus(_ac, MIN(taito, voima + 45));
                    if (q_ptr->to_a) lippu = TRUE;
                }
                if (mon_race_has_spell_type(r_ptr, MST_BREATH))
                {
                    mon_spell_group_ptr spells = r_ptr->spells->groups[MST_BREATH];
                    int i;
                    sopiva = 0;
                    for (i = 0; i < spells->count; i++)
                    {
                        if (!(spells->spells[i].id.effect)) continue;
                        sopiva++;
                        if (one_in_(sopiva)) q_ptr->xtra5 = spells->spells[i].id.effect;
                    }
                    if (q_ptr->xtra5) lippu = TRUE;
                }
                if ((r_ptr->flags9 & RF9_POS_TELEPATHY) && (randint0(1500) < voima) && (_igor_prob(3, unique, taito)))
                {
                    if (randint0(600) < voima)
                    {
                        add_esp_strong(q_ptr);
                        if (!one_in_(3)) break;
                    }
                    add_esp_weak(q_ptr, FALSE);
                    lippu = TRUE;
                }
                if ((voima + taito > 100) && (((!lippu) && (_igor_prob(4, unique, taito))) || (one_in_(44))))
                {
                    int i;
                    while (1)
                    {
                        int sopiva = 0, tarjolla = 0, valittu = 0;
                        for (i = OF_RES_START; i <= OF_RES_END; i++)
                        {
                            if (mon_matches_obj_res(r_ptr, i))
                            {
                                if (have_flag(q_ptr->flags, i)) continue;
                                tarjolla++;
                                if ((i == OF_RES_TIME) && (!one_in_(7))) continue;
                                if ((i == OF_RES_LITE) && (!one_in_(7))) continue;
                                sopiva++;
                                if (one_in_(sopiva)) valittu = i;
                            }
                        }
                        if (valittu) add_flag(q_ptr->flags, valittu);
                        if (!one_in_(8)) break;
                        if (!tarjolla) break;
                    }
                }
                break;
            }
            case _IB_EYES:
            {
                if (r_ptr->flags3 & (RF3_HURT_LITE))
                {
                    add_flag(q_ptr->flags, OF_VULN_LITE);
                    if (randint0(2000) < voima) add_flag(q_ptr->flags, OF_NIGHT_VISION);
                    if ((r_ptr->flagsr & RFR_RES_DARK) && (_igor_prob(4, unique, taito))) add_flag(q_ptr->flags, OF_RES_DARK);
                }
                else if ((r_ptr->flagsr & RFR_RES_DARK) && (_igor_prob(7, unique, taito))) add_flag(q_ptr->flags, OF_RES_DARK);
                if (r_ptr->flagsr & (RFR_RES_LITE))
                {
                    if ((randint0(120) < voima) && (randint0(120) < taito))
                    {
                        bool blind_okay = FALSE, blind_lis = FALSE;
                        if (r_ptr->flags3 & RF3_NO_CONF) blind_okay = TRUE;
                        if ((blind_okay) && (one_in_(2))) blind_lis = TRUE;
                        add_flag(q_ptr->flags, blind_lis ? OF_RES_BLIND : OF_RES_LITE);
                        if ((blind_lis) && (randint0(240) < voima) && (randint0(240) < taito))
                            add_flag(q_ptr->flags, OF_RES_LITE);
                    }
                }
                if (_igor_prob(4, unique, taito))
                {
                    int _inf = r_ptr->body.infra;
                    if (_inf > 0)
                    {
                        _my_pval = m_bonus(_inf, MIN(taito + 20, voima + 45));
                        if (_my_pval > 3) _my_pval = trim(_my_pval, 3, 6, voima);
                        if (_my_pval > 0)
                        {
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            add_flag(q_ptr->flags, OF_INFRA);
                        }
                    }
                }
                if ((r_ptr->d_char == 'e') && (r_ptr->flags9 & RF9_POS_TELEPATHY) && (randint0(2000) < voima))
                {
                    if (randint0(600) < voima)
                    {
                        add_esp_strong(q_ptr);
                        if (!one_in_(3)) break;
                    }
                    add_esp_weak(q_ptr, FALSE);
                }
                if ((strpos(" eye", r_name + r_ptr->name) == (strlen(r_name + r_ptr->name) - 3)) || (strpos("yclops", r_name + r_ptr->name)))
                {
                    remove_flag(q_ptr->flags, OF_PLURAL);
                }
                break;
            }
            case _IB_STOMACH:
            {
                if ((r_ptr->flagsr & RFR_IM_POIS) || ((r_ptr->flagsr & RFR_RES_POIS) && (one_in_(6))))
                {
                    if (!one_in_(3)) add_flag(q_ptr->flags, OF_RES_POIS);
                }
                if ((randint0(100) < taito) && (randint0(100) < voima) && (_igor_prob((r_ptr->d_char == 'J') ? 5 : 10, unique, taito)))
                {
                    add_flag(q_ptr->flags, OF_SLOW_DIGEST);
                }
                if ((r_ptr->flags2 & RF2_REGENERATE) && (randint0(100) < voima) && (randint0(100) < taito) && (_igor_prob(4, unique, taito)))
                {
                    add_flag(q_ptr->flags, OF_REGEN);
                }
                if (mon_race_has_spell_type(r_ptr, MST_BREATH))
                {
                    mon_spell_group_ptr spells = r_ptr->spells->groups[MST_BREATH];
                    int i;
                    sopiva = 0;
                    for (i = 0; i < spells->count; i++)
                    {
                        if (!(spells->spells[i].id.effect)) continue;
                        sopiva++;
                        if (one_in_(sopiva)) q_ptr->xtra5 = spells->spells[i].id.effect;
                    }
                }
                break;
            }
            case _IB_HEART:
            {
                if (_igor_prob(5, unique, taito))
                {
                    int _con = r_ptr->body.stats[A_CON];
                    if (_con)
                    {
                        bool merkki = (_con > 0) ? TRUE : FALSE;
                        _con = ABS(_con);
                        _my_pval = m_bonus(_con, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 3, merkki ? voima : 70);
                        while (_my_pval > 0)
                        {
                            if ((merkki) && (one_in_(4)))
                            {
                                add_flag(q_ptr->flags, OF_SUST_CON);
                                break;
                            }
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki)
                            {
                                add_flag(q_ptr->flags, OF_CON);
                                if (one_in_(8)) add_flag(q_ptr->flags, OF_SUST_CON);
                            }
                            else add_flag(q_ptr->flags, OF_DEC_CON);
                            break;
                        }
                    }
                }
                else if ((taito > 66) && (one_in_(12)))
                {
                    int _life = (r_ptr->body.life + 2) / 4 - 25;
                    if (_life)
                    {
                        bool merkki = (_life > 0) ? TRUE : FALSE;
                        _life = ABS(_life);
                        _my_pval = m_bonus(_life, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 2, merkki ? voima : 70);
                        if ((merkki) && (one_in_(2))) _my_pval = MIN(_my_pval, 1);
                        while (_my_pval > 0)
                        {
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki) add_flag(q_ptr->flags, OF_LIFE);
                            else add_flag(q_ptr->flags, OF_DEC_LIFE);
                            break;
                        }
                    }
                }
                if ((r_ptr->flags2 & RF2_REGENERATE) && (randint0(100) < voima) && (randint0(100) < taito))
                {
                    add_flag(q_ptr->flags, OF_REGEN);
                }
                if ((r_ptr->flagsr & RFR_IM_ELEC) || ((r_ptr->flagsr & RFR_RES_ELEC) && (!(r_ptr->flags2 & RF2_HUMAN)) && (one_in_(8))))
                {
                    if (_igor_prob(2, unique, taito)) add_flag(q_ptr->flags, OF_RES_ELEC);
                }
                if ((r_ptr->flags3 & RF3_NO_SLEEP) && (randint0(100) < voima) && (_igor_prob(5, unique, taito)))
                {
                    add_flag(q_ptr->flags, OF_FREE_ACT);
                }
                break;
            }
            case _IB_HANDS:
            {
                if (_igor_prob(4, unique, taito))
                {
                    int _dex = r_ptr->body.stats[A_DEX];
                    if (_dex)
                    {
                        bool merkki = (_dex > 0) ? TRUE : FALSE;
                        _dex = ABS(_dex);
                        _my_pval = m_bonus(_dex, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 3, merkki ? voima : 70);
                        while (_my_pval > 0)
                        {
                            if ((merkki) && (_igor_prob(7, unique, taito)))
                            {
                                add_flag(q_ptr->flags, OF_SUST_DEX);
                                break;
                            }
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki)
                            {
                                add_flag(q_ptr->flags, OF_DEX);
                                if (one_in_(6)) add_flag(q_ptr->flags, OF_SUST_DEX);
                            }
                            else add_flag(q_ptr->flags, OF_DEC_DEX);
                            break;
                        }
                    }
                }
                if (_igor_prob(7, unique, taito))
                {
                    int _str = r_ptr->body.stats[A_STR];
                    if (_str)
                    {
                        bool merkki = (_str > 0) ? TRUE : FALSE;
                        _str = ABS(_str);
                        _my_pval = m_bonus(_str, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 3, merkki ? voima : 70);
                        while (_my_pval > 0)
                        {
                            if ((merkki) && (_igor_prob(7, unique, taito)))
                            {
                                add_flag(q_ptr->flags, OF_SUST_STR);
                                break;
                            }
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki)
                            {
                                add_flag(q_ptr->flags, OF_STR);
                                if (one_in_(6)) add_flag(q_ptr->flags, OF_SUST_STR);
                            }
                            else add_flag(q_ptr->flags, OF_DEC_STR);
                            break;
                        }
                    }
                }
                if (_igor_prob(6, unique, taito))
                {
                    int _thn = r_ptr->body.skills.thn + (((r_ptr->level + 10) / 20) * r_ptr->body.extra_skills.thn);
                    int i, _thd = r_ptr->body.stats[A_STR] - 4;
                    bool merkki;
                    _thn += r_ptr->body.skills.thb + (((r_ptr->level + 10) / 20) * r_ptr->body.extra_skills.thb);
                    _thn -= 150;
                    merkki = (_thn > 0);
                    if (_thn)
                    {
                        _thn /= 10;
                        _my_pval = m_bonus(_thn, merkki ? MIN(taito, voima + 35) : 80);
                        _my_pval = trim(_my_pval, 8, 16, merkki ? voima : 70);
                        q_ptr->to_h = merkki ? _my_pval : (0 - _my_pval);
                    }
                    if (r_ptr->flags1 & RF1_NEVER_BLOW) _thd -= 5;
                    else
                    {
                        int metodi, bonus = 0;
                        for (i = 0; i < MAX_MON_BLOWS; i++)
                        {
                            metodi = r_ptr->blows[i].method;
                            if (!metodi) break;
                            if (metodi == RBM_HIT) bonus = MAX(bonus, 4);
                            if (metodi == RBM_PUNCH)
                            {
                                bonus = 8;
                                break;
                            }
                        }
                        _thd += bonus;
                    }
                    if (_thd)
                    {
                        _my_pval = m_bonus(_thd, merkki ? MIN(taito, voima + 35) : 80);
                        _my_pval = trim(_my_pval, 5, 10, merkki ? voima : 70);
                        q_ptr->to_d = merkki ? _my_pval : (0 - _my_pval);
                    }
                }
                break;
            }
            case _IB_EARS: /* Ears were originally intended to exclusively be
                            * a wanted trophy. They can now be "worn", but
                            * have no purpose besides making for funny combos
                            * (and the original purpose of a wanted trophy) */
                q_ptr->pval = q_ptr->xtra4;
                q_ptr->xtra4 = 0;
                break;
            default: /* Legs */
            {
                q_ptr->sval = SV_BODY_LEGS; /* paranoia */
                q_ptr->k_idx = lookup_kind(TV_CORPSE, SV_BODY_LEGS);

                if (_igor_prob((r_ptr->d_char == 'h') ? 3 : 6, unique, taito))
                {
                    int _stl = r_ptr->body.skills.stl + (((r_ptr->level + 10) / 20) * r_ptr->body.extra_skills.stl);
                    if (_stl)
                    {
                        bool merkki = (_stl > 0) ? TRUE : FALSE;
                        _stl = ABS(_stl);
                        _my_pval = m_bonus(_stl, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 2) _my_pval = trim(_my_pval, 2, 4, merkki ? voima : 70);
                        if (_my_pval > 0)
                        {
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki) add_flag(q_ptr->flags, OF_STEALTH);
                            else add_flag(q_ptr->flags, OF_DEC_STEALTH);
                        }
                    }
                }
                if ((randint1(100) < r_ptr->level) && (randint1(unique ? 130 : 150) < taito))
                {
                    int _spd = r_ptr->speed - 110;

                    if (_spd)
                    {
                        bool merkki = (_spd > 0) ? TRUE : FALSE;
                        _spd = ABS(_spd);
                        if (_spd > 10) _spd = trim(_spd, 10, 20, voima);
                        _my_pval = m_bonus(_spd, merkki ? MIN(taito + 15, voima + 25) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 3, merkki ? voima : 70);
                        if (_my_pval > 0)
                        {
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki) add_flag(q_ptr->flags, OF_SPEED);
                            else add_flag(q_ptr->flags, OF_DEC_SPEED);
                        }
                    }
                }
                if ((r_ptr->ac > 100) && (randint1(unique ? 242 : 363) < taito))
                {
                    int _ac = (r_ptr->ac - 91) / 10;
                    q_ptr->to_a = m_bonus(_ac, MIN(taito, voima + 45));
                }
                if ((r_ptr->flagsr & RFR_IM_COLD) || ((r_ptr->flagsr & RFR_RES_COLD) && (r_ptr->flags3 & RF3_ANIMAL) && (one_in_(5))))
                {
                    if (!one_in_(3)) add_flag(q_ptr->flags, OF_RES_COLD);
                    if (one_in_(3)) add_flag(q_ptr->flags, OF_RES_FEAR);
                }
                if (_igor_prob(q_ptr->pval ? 10 : 4, unique, 0))
                {
                    int _con = r_ptr->body.stats[A_CON];
                    if (_con)
                    {
                        bool merkki = (_con > 0) ? TRUE : FALSE;
                        _con = ABS(_con);
                        _my_pval = m_bonus(_con, merkki ? MIN(taito + 20, voima + 45) : 100);
                        if (_my_pval > 1) _my_pval = trim(_my_pval, 1, 2, merkki ? voima : 70);
                        if (_my_pval > 0)
                        {
                            if ((merkki) && (_igor_prob(4, unique, taito)))
                            {
                                add_flag(q_ptr->flags, OF_SUST_CON);
                                break; /* Watch out! */
                            }
                            if (!q_ptr->pval) q_ptr->pval = _my_pval;
                            else q_ptr->pval = MIN(q_ptr->pval, _my_pval);
                            if (merkki)
                            {
                                add_flag(q_ptr->flags, OF_CON);
                                if (one_in_(8)) add_flag(q_ptr->flags, OF_SUST_CON);
                            }
                            else add_flag(q_ptr->flags, OF_DEC_CON);
                        }
                    }
                }
                break;
            }
        }
        prompt.obj->number--;
        obj_release(prompt.obj, 0);
        if ((_wanted) && (unique) && (tyyppi != _IB_HEAD) && (tyyppi != _IB_EARS)) _ears_of(q_ptr->xtra4);
        _igor_set_origins(q_ptr);
        obj_identify_fully(q_ptr);
        pack_carry(q_ptr);
        p_ptr->window |= (PW_INVEN);
    }
    return TRUE;
}

static void _birth(void)
{
    int i;

    _do_init_pack = TRUE;
    _igor_pack_init();

    if (spoiler_hack) return;

    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Claw", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    py_birth_food();
    py_birth_light();
    _igor_birth_hack = TRUE;
    for (i = 0; i < _IB_MAX_ACTIVE; i++)
    {
        object_type forge, *q_ptr = &forge;
        object_prep(q_ptr, lookup_kind(TV_CORPSE, _ibtosval(i + 1)));
        q_ptr->xtra4 = 0;
        q_ptr->number = 1;
        if (i == (_IB_HANDS - 1))
        {
            q_ptr->pval = 2;
            add_flag(q_ptr->flags, OF_DEX);
        }
        object_origins(q_ptr, ORIGIN_BIRTH);
        q_ptr->loc.where = INV_TMP_ALLOC;
        obj_identify_fully(q_ptr);
        (void)_igor_carry(q_ptr);
        /* No need to call obj_release() to free the memory (indeed, it causes crashes on Linux) */
    }
    mut_gain(MUT_LIMP);
    mut_lock(MUT_LIMP);
    _igor_birth_hack = FALSE;
}

void _igor_calc_innate_attacks(void)
{
    int l = p_ptr->lev;
    int to_h = l/6 - 10;

    if (!_pack_initialized) return;

    while (1)
    {
        static innate_attack_t _bite = {0};
        static int _oldh = 0;
        static bool _exists = FALSE;
        object_type *o_ptr = inv_obj(_igor_body, _IB_HEAD);

        if ((o_ptr) && (o_ptr->xtra4 != _oldh))
        {
            monster_race *r_ptr;
            if ((!o_ptr->xtra4) || (o_ptr->xtra4 >= max_r_idx))
            {
                _oldh = o_ptr->xtra4;
                _exists = FALSE;
                break;
            }
            r_ptr = &r_info[o_ptr->xtra4];
            _oldh = o_ptr->xtra4;
            _exists = FALSE;
            if ((r_ptr->name) && (!(r_ptr->flags1 & RF1_NEVER_BLOW)))
            {
                int i, j, metodi, maksi = 0, _md = 0, _ms = 0;
                bool venom = FALSE;
                for (i = 0; i < MAX_MON_BLOWS; i++)
                {
                    metodi = r_ptr->blows[i].method;
                    if (!metodi) break;
                    if (metodi != RBM_BITE) continue;
                    {
                        for (j = 0; j < MAX_MON_BLOW_EFFECTS; j++)
                        {
                            mon_effect_t e = r_ptr->blows[i].effects[j];
                            if (!e.effect) break;
                            if (e.effect > RBE_HURT) continue;
                            if (e.effect == GF_POIS) venom = TRUE;
                            if (((e.dd + 1) * e.ds) > maksi)
                            {
                                maksi = MAX(maksi, (e.dd + 1) * e.ds);
                                _md = e.dd;
                                _ms = e.ds;
                            }
                        }
                    }
                }
                if (maksi >= 10)
                {
                    _exists = TRUE;
                    _bite.dd = _md;
                    _bite.ds = _ms;
                    _bite.to_d += 2;
                    _bite.to_h += to_h;

                    _bite.weight = 60;
                    calc_innate_blows(&_bite, 100);
                    _bite.msg = "You bite.";
                    _bite.name = "Bite";
                    _bite.effect[0] = GF_MISSILE;
                    if (venom) _bite.effect[1] = GF_POIS;
                    if (psion_combat()) psion_combat_innate_blows(&_bite);
                }
            }
        }

        if (_exists) p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = _bite;
        break;
    }

    while (1)
    {
        static innate_attack_t _claw = {0};
        static int _oldg = 0;
        static bool _exists = FALSE;
        object_type *o_ptr = inv_obj(_igor_body, _IB_HANDS);

        if ((o_ptr) && (o_ptr->xtra4 != _oldg))
        {
            monster_race *r_ptr;
            if ((!o_ptr->xtra4) || (o_ptr->xtra4 >= max_r_idx))
            {
                _oldg = o_ptr->xtra4;
                _exists = FALSE;
                break;
            }
            r_ptr = &r_info[o_ptr->xtra4];
            _oldg = o_ptr->xtra4;
            _exists = FALSE;
            if ((r_ptr->name) && (!(r_ptr->flags1 & RF1_NEVER_BLOW)))
            {
                int i, j, metodi, maksi = 0, _md = 0, _ms = 0;
                bool venom = FALSE;
                for (i = 0; i < MAX_MON_BLOWS; i++)
                {
                    metodi = r_ptr->blows[i].method;
                    if (!metodi) break;
                    if (metodi != RBM_CLAW) continue;
                    {
                        for (j = 0; j < MAX_MON_BLOW_EFFECTS; j++)
                        {
                            mon_effect_t e = r_ptr->blows[i].effects[j];
                            if (!e.effect) break;
                            if (e.effect > RBE_HURT) continue;
                            if (e.effect == GF_POIS) venom = TRUE;
                            if (((e.dd + 1) * e.ds) > maksi)
                            {
                                maksi = MAX(maksi, (e.dd + 1) * e.ds);
                                _md = e.dd;
                                _ms = e.ds;
                            }
                        }
                    }
                }
                if (maksi >= 10)
                {
                    _exists = TRUE;
                    _claw.dd = _md;
                    _claw.ds = _ms;
                    _claw.to_d += 2;
                    _claw.to_h += to_h;

                    _claw.weight = 60;
                    calc_innate_blows(&_claw, 100);
                    _claw.msg = "You claw.";
                    _claw.name = "Claw";
                    _claw.effect[0] = GF_MISSILE;
                    if (venom) _claw.effect[1] = GF_POIS;
                    if (psion_combat()) psion_combat_innate_blows(&_claw);
                }
            }
        }

        if (_exists) p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = _claw;
        break;
    }
}

static void _igor_head_breathe_spell(int cmd, variant *res)
{
    object_type *nuppi = inv_obj(_igor_body, _IB_HEAD);
    static gf_info_ptr gf = NULL;
    static char elly_upper[32] = "Software Bugs", elly_lower[32] = "software bugs";
    static int mika = 0;
    int pow = 1;
    /* This is ugly, but some elements have multi-part names that are hard to
     * (un)capitalize */
    if (mika != nuppi->xtra5) gf = gf_lookup(nuppi->xtra5);
    if ((gf) && (gf->name) && (mika != nuppi->xtra5))
    {
        strcpy(elly_upper, gf->name);
        strcpy(elly_lower, gf->name);
        str_tolower(elly_lower);
        mika = nuppi->xtra5;
    }
    if ((cmd == SPELL_INFO) || (cmd == SPELL_CAST))
    {
        int voima = 0;
        if ((nuppi->xtra4 > 0) && (nuppi->xtra4 < max_r_idx))
        {
            voima = r_info[nuppi->xtra4].level;
        }
        pow = MAX(1, voima + (p_ptr->chp / 6));
        switch (nuppi->xtra5)
        {
            case GF_ACID:
            case GF_ELEC:
            case GF_COLD:
            case GF_FIRE:
                  pow = pow * 2;
                  break;
            case GF_POIS:
                  pow = pow * 4 / 3;
                  break;
            default:
                  break;
        }
        pow /= 6;
    }
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Breathe %^s", elly_upper));
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s at your opponent.", elly_lower));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, pow));
        break;
    case SPELL_COST_EXTRA:
    {
        int l = p_ptr->lev;
        int cst = l*l/150;
        var_set_int(res, cst);
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            msg_format("You breathe <color:%c>%s.</color>", gf ? attr_to_attr_char(gf->color) : 'w', elly_upper);
            fire_ball(nuppi->xtra5, dir, pow, -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _igor_stomach_breathe_spell(int cmd, variant *res)
{
    object_type *tummy = inv_obj(_igor_body, _IB_STOMACH);
    static gf_info_ptr gf = NULL;
    static char elly_upper[32] = "Software Bugs", elly_lower[32] = "software bugs";
    static int mika = 0;
    int pow = 1;
    /* This is ugly, but some elements have multi-part names that are hard to
     * (un)capitalize */
    if (mika != tummy->xtra5) gf = gf_lookup(tummy->xtra5);
    if ((gf) && (mika != tummy->xtra5))
    {
        strcpy(elly_upper, gf->name);
        strcpy(elly_lower, gf->name);
        str_tolower(elly_lower);
        mika = tummy->xtra5;
    }
    if ((cmd == SPELL_INFO) || (cmd == SPELL_CAST))
    {
        byte head_support = 2;
        object_type *nuppi = inv_obj(_igor_body, _IB_HEAD);
        int voima = 0;
        
        if ((nuppi) && (nuppi->xtra5 == tummy->xtra5))
        {
            head_support = 3;
            if (nuppi->xtra4 == tummy->xtra4) head_support = 4;
            if ((nuppi->xtra4 > 0) && (tummy->xtra4 > 0) && (nuppi->xtra4 < max_r_idx) && (nuppi->xtra5 < max_r_idx)) voima = (r_info[tummy->xtra4].level * 2 + r_info[nuppi->xtra4].level + 2) / 3;
        }
        else if ((tummy->xtra4 > 0) && (tummy->xtra4 < max_r_idx))
        {
            voima = r_info[tummy->xtra4].level;
        }
        pow = MAX(1, voima + (p_ptr->chp / 6));
        switch (tummy->xtra5)
        {
            case GF_ACID:
            case GF_ELEC:
            case GF_COLD:
            case GF_FIRE:
                  pow = pow * 2;
                  break;
            case GF_POIS:
                  pow = pow * 4 / 3;
                  break;
            default:
                  break;
        }
        pow *= head_support;
        pow /= 6;
    }
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Breathe %^s", elly_upper));
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s at your opponent.", elly_lower));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, pow));
        break;
    case SPELL_COST_EXTRA:
    {
        int l = p_ptr->lev;
        int cst = l*l/150;
        var_set_int(res, cst);
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            msg_format("You breathe <color:%c>%s.</color>", gf ? attr_to_attr_char(gf->color) : 'w', elly_upper);
            fire_ball(tummy->xtra5, dir, pow, -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _dissect_corpse_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dissect Corpse");
        break;
    case SPELL_DESC:
        var_set_string(res, "Cuts open a dead body, looking for a part worth salvaging. (Don't be too clumsy!)");
        break;
    case SPELL_CAST:
        var_set_bool(res, _dissect_corpse());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void _replace_body_part_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Surgery");
        break;
    case SPELL_DESC:
        var_set_string(res, "Replaces one of your body parts with another. (This requires several turns...)");
        break;
    case SPELL_CAST:
        var_set_bool(res, _equip_body_part(TRUE));
        break;
    case SPELL_ENERGY:
    {
        int hinta = 700, slot = equip_find_first(object_is_gloves);
        if (slot)
        {
            u32b         flgs[OF_ARRAY_SIZE];
            object_type *o_ptr = equip_obj(slot);

            if ((o_ptr) && (o_ptr->k_idx)) /* paranoia */
            {
                obj_flags(o_ptr, flgs);
                if ((have_flag(flgs, OF_FREE_ACT)) ||
                    (have_flag(flgs, OF_MAGIC_MASTERY)) ||
                    ((have_flag(flgs, OF_DEX)) && (o_ptr->pval > 0)))
                {
                    hinta = 500;
                }
            }
        }
        var_set_int(res, hinta);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _freeze_body_part_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Freeze Body Part");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places an organ in your ice bag for later use.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _equip_body_part(FALSE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _igor_powers[] = {
    { A_DEX, {  1,  0,  0, _dissect_corpse_spell}},
    { A_DEX, {  1,  0,  0, _replace_body_part_spell}},
    { A_DEX, {  1,  0,  0, _freeze_body_part_spell}},
    { A_INT, { 10,  5,  20, phase_door_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static power_info _igor_head_spell[] = {
    { A_CON, {  1, 10,  0, _igor_head_breathe_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static power_info _igor_stomach_spell[] = {
    { A_CON, {  1, 10,  0, _igor_stomach_breathe_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static int _get_powers(spell_info* spells, int max) {
    int ct = get_powers_aux(spells, max, _igor_powers);
    object_type *o1_ptr, *o2_ptr;
    if (!_pack_initialized) return ct;
    o1_ptr = inv_obj(_igor_body, _IB_STOMACH);
    if ((o1_ptr) && (o1_ptr->xtra5)) ct += get_powers_aux(spells + ct, max - ct, _igor_stomach_spell);
    o2_ptr = inv_obj(_igor_body, _IB_HEAD);
    if ((o2_ptr) && (o2_ptr->xtra5) && ((!o1_ptr) || (o2_ptr->xtra5 != o1_ptr->xtra5))) ct += get_powers_aux(spells + ct, max - ct, _igor_head_spell);
    return ct;
}

static slot_t _igor_fake_slot(slot_t slot)
{
    static slot_t vastineet[_IB_MAX_ACTIVE] = {0};
    static bool _init = FALSE;
    if (!_init)
    {
        equip_template_ptr _my_body = &b_info[0];
        int i;
        for (i = 1; i < _my_body->max; i++)
        {
            if (_my_body->slots[i].type == EQUIP_SLOT_HELMET) vastineet[_IB_HEAD - 1] = i;
            if (_my_body->slots[i].type == EQUIP_SLOT_GLOVES) vastineet[_IB_HANDS - 1] = i;
            if (_my_body->slots[i].type == EQUIP_SLOT_BOOTS) vastineet[_IB_LEGS - 1] = i;
            if (_my_body->slots[i].type == EQUIP_SLOT_LITE) /* why the heck not */
            {
                vastineet[_IB_HEART - 1] = i;
                vastineet[_IB_EYES - 1] = i;
                vastineet[_IB_STOMACH - 1] = i;
            }
        }
        for (i = 0; i < _IB_MAX_ACTIVE; i++)
        {
            if (!vastineet[i]) vastineet[i] = 8; /* paranoia - even more fake slot */
        }
        _init = TRUE;
    }
    if ((slot < 1) || (slot > _IB_MAX_ACTIVE)) return 0;
    return vastineet[slot - 1];
}

void igor_body_bonuses(void)
{
    int slot;
    if (!_pack_initialized) return;
    else
    {
        for (slot = 1; slot <= _IB_MAX_ACTIVE; slot++)
        {
            object_type *obj = inv_obj(_igor_body, slot);
            if ((!obj) || (!obj->k_idx)) continue;
            object_calc_bonuses(obj, _igor_fake_slot(slot));
        }
    }
}

static void _calc_stats(s16b stats[MAX_STATS])
{
    int i, j;
    if (!_pack_initialized) return;
    for (i = _IB_HEAD; i <= _IB_MAX_ACTIVE; i++)
    {
        object_type *o_ptr = inv_obj(_igor_body, i);
        if (i == _IB_EARS) continue; /* save time */
        if ((!o_ptr) || (!o_ptr->k_idx) || (!o_ptr->pval)) continue;
        for (j = A_STR; j < MAX_STATS; j++)
        {
            if (have_flag(o_ptr->flags, OF_STR + j - A_STR)) stats[j] += o_ptr->pval;
            if (have_flag(o_ptr->flags, OF_DEC_STR + j - A_STR)) stats[j] -= o_ptr->pval;
        }
    }
}

static void _calc_bonuses(void)
{
    p_ptr->pspeed -= 1;
    res_add(RES_ELEC);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    int i, j;
    add_flag(flgs, OF_RES_ELEC);
    add_flag(flgs, OF_DEC_SPEED);
    if (!_pack_initialized) return;
    for (i = _IB_HEAD; i <= _IB_MAX_ACTIVE; i++)
    {
        object_type *o_ptr;
        if (i == _IB_EARS) continue;
        o_ptr = inv_obj(_igor_body, i);
        if ((!o_ptr) || (!o_ptr->k_idx)) continue;
        for (j = 0; j < OF_ARRAY_SIZE; j++)
        {
            flgs[j] |= o_ptr->flags[j];
        }
    }
}

static void _igor_save(savefile_ptr file)
{
    inv_save(_igor_cold_pack, file);
    inv_save(_igor_body, file);
}

static void _igor_load(savefile_ptr file)
{
    _igor_pack_init();
    inv_load(_igor_cold_pack, file);
    inv_load(_igor_body, file);
}

static void _dump_body(doc_ptr doc)
{
    slot_t slot;

    char o_name[MAX_NLEN];

    if (!_pack_initialized) return;

    doc_insert(doc, "<topic:Body>===================================== <color:keypress>B</color>ody ====================================\n\n");
    for (slot = 1; slot <= _IB_MAX_ACTIVE; slot++)
    {
        object_type *o_ptr = inv_obj(_igor_body, slot);
        if ((!o_ptr) || (!o_ptr->k_idx)) continue;

        object_desc(o_name, o_ptr, OD_COLOR_CODED);
        doc_printf(doc, " %c) <indent><style:indent>%s</style></indent>\n", slot - 1 + 'a', o_name);
        if (((always_dump_origins) || ((final_dump_origins) && ((p_ptr->total_winner) || (p_ptr->is_dead))))
         && (o_ptr->origin_type != ORIGIN_NONE) && (o_ptr->origin_type != ORIGIN_MIXED))
        {
            doc_printf(doc, "    <indent><style:indent><color:W>");
            (void)display_origin(o_ptr, doc);
            doc_printf(doc, "</color></style></indent>\n");
        }
    }
    doc_newline(doc);
}

static void _dump_cold_pack(doc_ptr doc)
{
    int laskuri = 0;
    slot_t slot;

    if (!_pack_initialized) return;

    for (slot = 1; slot <= _MAX_PACK_SLOTS; slot++)
    {
        obj_ptr src = inv_obj(_igor_cold_pack, slot);
        if (!src) continue;
        laskuri++;
    }

    if (!laskuri) return;
    else
    {
        char o_name[MAX_NLEN];

        doc_insert(doc, "<topic:Ice Bag>=================================== <color:keypress>I</color>ce Bag ===================================\n\n");
        for (slot = 1; slot <= _MAX_PACK_SLOTS; slot++)
        {
            object_type *o_ptr = inv_obj(_igor_cold_pack, slot);
            if (!o_ptr) continue;

            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            doc_printf(doc, " %c) <indent><style:indent>%s</style></indent>\n", slot - 1 + 'a', o_name);
            if (((always_dump_origins) || ((final_dump_origins) && ((p_ptr->total_winner) || (p_ptr->is_dead))))
              && (o_ptr->origin_type != ORIGIN_NONE) && (o_ptr->origin_type != ORIGIN_MIXED))
            {
                doc_printf(doc, "    <indent><style:indent><color:W>");
                (void)display_origin(o_ptr, doc);
                doc_printf(doc, "</color></style></indent>\n");
            }
        }
        doc_newline(doc);
    }
}

static void _dump_extras(doc_ptr doc)
{
    _dump_body(doc);
    _dump_cold_pack(doc);
}


inv_ptr _igor_get_pack(void)
{
    if (!_pack_initialized) return NULL;
    return _igor_cold_pack;
}

inv_ptr _igor_get_body(void)
{
    if (!_pack_initialized) return NULL;
    return _igor_body;
}

/* Account for the weight of the items in the ice bag */
int _igor_pack_weight(obj_p p)
{
    if (!_igor_get_pack()) return 50; /* the weight of the ice bag itself */

    return (inv_weight(_igor_cold_pack, p) + 50);
}

/* Igor body parts can sometimes be activated! */
static void _igor_recharge_body_aux(object_type *o_ptr)
{
    if (o_ptr->timeout > 0)
    {
        o_ptr->timeout--;
        if (!o_ptr->timeout) recharged_notice(o_ptr, '!');
    }
}

static void _igor_recharge_body(void)
{
    if (!_pack_initialized) return;
    inv_for_each(_igor_body, _igor_recharge_body_aux);
}

race_t *igor_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;

    if (!init)
    {
        me.name = "Igor";
        me.desc = "The epitome of a self-made man, an Igor is generally easily recognizable by "
                  "the ring of stitches around his neck, the tendency to limp with perfectly "
                  "healthy legs, and the mysterious ability to appear suddenly "
                  "and then disappear when the mob comes looking. An Igor is "
                  "at home with lightning, and always carries a bag full of ice in case he "
                  "should find something that needs to be kept cold.";

        me.exp = 145;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers = _get_powers;
        me.get_flags = _get_flags;
        me.birth = _birth;
        me.load_player = _igor_load;
        me.save_player = _igor_save;
        me.character_dump = _dump_extras;
        me.calc_extra_weight = _igor_pack_weight;
        me.stats[A_STR] = -3;
        me.stats[A_INT] = 0;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] = -1;
        me.life = 97;
        me.shop_adjust = 110;
        me.base_hp = 15;

        me.skills.dis = 2;
        me.skills.dev = 0;
        me.skills.sav = 0;
        me.skills.stl = 0;
        me.skills.srh = 0;
        me.skills.fos = 10;
        me.skills.thn = -16;
        me.skills.thb = -15;
        me.calc_innate_attacks = _igor_calc_innate_attacks;
        me.calc_stats = _calc_stats;
        me.flags = RACE_NO_POLY;
        me.process_world = _igor_recharge_body;
        init = TRUE;
    }

    me.bonus_pack = _igor_get_pack();
    me.bonus_pack2 = _igor_get_body();

    return &me;
}
