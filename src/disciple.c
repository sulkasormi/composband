#include "angband.h"

static cptr _desc =
    "Trusted followers of mysterious ancient spirits known as the 'Purples', "
    "the Disciples roam the world running errands for their masters, "
    "learning mighty spells and receiving rare items as rewards for their "
    "work. There are three main flavors of Purple Disciples - disciples "
    "of Karrot, disciples of Yeqrezh, and disciples of a group of "
    "Purples known as the Troika. Each of the Purples expects different "
    "things of their Disciples, and each helps them achieve their ends in "
    "very different ways.";
    /* "For more information, see <link:Disciples.txt>.";*/

void disciple_birth(void)
{
    mut_unlock(MUT_CHAOS_GIFT);
    mut_lose(MUT_CHAOS_GIFT);
    mut_gain(MUT_PURPLE_GIFT);
    mut_lock(MUT_PURPLE_GIFT);
}

/* Basically, this is the level the player would be if he were a Dunadan */
int karrot_level(void)
{
    int real_xp_factor = p_ptr->expfact;
    int i, sopiva = 1;
    p_ptr->expfact = 200;
    for (i = 1; i < PY_MAX_LEVEL; i++)
    {
        if (p_ptr->max_max_exp >= exp_requirement(i))
        {
            sopiva = i + 1;
        }
        else break;
    }
    p_ptr->expfact = real_xp_factor;
    return sopiva;
}

void disciple_feeling(void)
{
    if (disciple_is_(DISCIPLE_YEQREZH)) return;
    if (!py_in_dungeon()) return;
    if (p_ptr->max_plv < 3) return;
    if (disciple_is_(DISCIPLE_KARROT))
    {
        point_t tbl[3] = { {24, 124}, {34, 134}, {50, 166} };
        int verrokki1 = karrot_level() + 5;
        int verrokki2 = (karrot_level() * interpolate(karrot_level(), tbl, 3) / 100) - 2;
        if ((dun_level < verrokki1) || (dun_level < verrokki2))
        {
             msg_print("Karrot considers this depth <color:y>uninteresting.</color>");
        }
        else if (dun_level < verrokki2 + 2)
        {
             msg_print("Karrot considers this depth <color:v>mildly interesting.</color>");
        }
        else
        {
             cmsg_print(TERM_VIOLET, "Karrot sees this as a worthy depth for you.");
        }
        return;
    }
    else
    {
        point_t tbl[3] = { {26, 134}, {50, 182} };
        int verrokki1 = p_ptr->max_plv + 5;
        int verrokki2 = (p_ptr->max_plv * interpolate(p_ptr->max_plv, tbl, 2) / 100) - 2;
        if ((dun_level < verrokki1) || (dun_level < verrokki2))
        {
             msg_print("Sohoglyth considers this depth <color:y>uninteresting.</color>");
        }
        else if (dun_level < verrokki2 + 2)
        {
             msg_print("Sohoglyth considers this depth <color:v>mildly interesting.</color>");
        }
        else
        {
             cmsg_print(TERM_VIOLET, "Sohoglyth sees this as a worthy depth for you.");
        }
        return;
    }
}

quest_ptr disciple_get_quest(int dungeon, int quest)
{
    if (p_ptr->pclass != CLASS_DISCIPLE) return NULL;
    switch (p_ptr->psubclass)
    {
        case DISCIPLE_KARROT: return karrot_get_quest(dungeon, quest);
        case DISCIPLE_TROIKA: return troika_get_quest(dungeon, quest);
        default: return NULL;
    }
}

class_t *disciple_get_class(int psubclass)
{
    class_t *result = NULL;
    switch (psubclass)
    {
    case DISCIPLE_KARROT:
        result = karrot_get_class();
        break;
    case DISCIPLE_YEQREZH:
        result = yeqrezh_get_class();
        break;
    case DISCIPLE_TROIKA:
        result = troika_get_class();
        break;
    default: /* ?? */
        result = karrot_get_class();
    }

    if ((psubclass >= MIN_PURPLE_PATRON) && (psubclass < MAX_PURPLE_PATRON)) result->subname = chaos_patrons[psubclass];
    else
    {
        result->subname = "";
    }
    result->name = "Disciple";
    result->desc = _desc;

    return result;
}

