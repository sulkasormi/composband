#include "angband.h"

bool spoiler_hack = FALSE;

#ifdef ALLOW_SPOILERS

typedef void(*_file_fn)(FILE*);
static void _help_file(cptr name, _file_fn fn)
{
    FILE    *fp = NULL;
    char    buf[1024];

    path_build(buf, sizeof(buf), ANGBAND_DIR_HELP, name);
    fp = my_fopen(buf, "w");

    if (!fp)
    {
        path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
        fp = my_fopen(buf, "w");

        if (!fp)
        {
            prt("Failed!", 0, 0);
            (void)inkey();
            return;
        }
    }

    fn(fp);
    fprintf(fp, "\n\n<color:s>Automatically generated for FrogComposband %d.%d.%s.</color>\n",
            VER_MAJOR, VER_MINOR, VER_PATCH);

    my_fclose(fp);
    if (character_dungeon) msg_format("Created %s", buf);
}

static void _csv_file(cptr name, _file_fn fn)
{
    FILE    *fp = NULL;
    char    buf[1024];

    path_build(buf, sizeof(buf), ANGBAND_DIR_HELP, name);
    fp = my_fopen(buf, "w");

    if (!fp)
    {
        path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
        fp = my_fopen(buf, "w");

        if (!fp)
        {
            prt("Failed!", 0, 0);
            (void)inkey();
            return;
        }
    }

    fn(fp);

    my_fclose(fp);
    if (character_dungeon) msg_format("Created %s", buf);
}

/******************************************************************************
 * Skill Descriptions
 * Rather then displaying a meaningless and perhaps spoilerish number to the user,
 * let's display a description instead. These descriptions are only for comparison
 * purposes. For example, Warriors have "Bad" device skills while a Mage is "Superb".
 * Of course, a CL50 Warrior's Character Sheet might list their device skill as
 * "Superb", but it really is "Bad" ... Trust me!
 *
 * TODO: I copied this to skills.c ... please rewrite here (cf wiz_doc.c)
 ******************************************************************************/
static cptr _skill_desc(int amt, int div)
{
    static char buf[255];
    skill_desc_t desc = skills_describe(amt, div);
    sprintf(buf, "<color:%c>%-13.13s</color>", attr_to_attr_char(desc.color), desc.desc);
    return buf;
}

/* Disarming */
static cptr _dis_skill_desc(int base, int xtra) { return _skill_desc(base + 5*xtra - 40, 8); }
static cptr _class_dis_skill_desc(class_t *class_ptr) { return _dis_skill_desc(class_ptr->base_skills.dis, class_ptr->extra_skills.dis); }
static cptr _mon_race_dis_skill_desc(race_t *race_ptr) { return _dis_skill_desc(race_ptr->skills.dis, race_ptr->extra_skills.dis); }

static cptr _dis_skill_desc2(int base) { return _skill_desc(base + 5, 2); }
static cptr _race_dis_skill_desc(race_t *race_ptr) { return _dis_skill_desc2(race_ptr->skills.dis); }
static cptr _pers_dis_skill_desc(personality_ptr pers_ptr) { return _dis_skill_desc2(pers_ptr->skills.dis*2); }

/* Devices */
static cptr _dev_skill_desc(int base, int xtra) { return _skill_desc(base + 5*xtra - 50, 6); }
static cptr _class_dev_skill_desc(class_t *class_ptr) { return _dev_skill_desc(class_ptr->base_skills.dev, class_ptr->extra_skills.dev); }
static cptr _mon_race_dev_skill_desc(race_t *race_ptr) { return _dev_skill_desc(race_ptr->skills.dev, race_ptr->extra_skills.dev); }

static cptr _dev_skill_desc2(int base) { return _skill_desc(base + 5, 2); }
static cptr _race_dev_skill_desc(race_t *race_ptr) { return _dev_skill_desc2(race_ptr->skills.dev); }
static cptr _pers_dev_skill_desc(personality_ptr pers_ptr) { return _dev_skill_desc2(pers_ptr->skills.dev*2); }

/* Saving Throws */
static cptr _sav_skill_desc(int base, int xtra) { return _skill_desc(base + 5*xtra - 65, 5); }
static cptr _class_sav_skill_desc(class_t *class_ptr) { return _sav_skill_desc(class_ptr->base_skills.sav, class_ptr->extra_skills.sav); }
static cptr _mon_race_sav_skill_desc(race_t *race_ptr) { return _sav_skill_desc(race_ptr->skills.sav, race_ptr->extra_skills.sav); }

static cptr _sav_skill_desc2(int base) { return _skill_desc(base + 5, 2); }
static cptr _race_sav_skill_desc(race_t *race_ptr) { return _sav_skill_desc2(race_ptr->skills.sav); }
static cptr _pers_sav_skill_desc(personality_ptr pers_ptr) { return _sav_skill_desc2(pers_ptr->skills.sav*2); }

/* Melee */
static cptr _thn_skill_desc(int base, int xtra) { return _skill_desc(base + 5*xtra - 70, 12); }
static cptr _class_thn_skill_desc(class_t *class_ptr) { return _thn_skill_desc(class_ptr->base_skills.thn, class_ptr->extra_skills.thn); }
static cptr _mon_race_thn_skill_desc(race_t *race_ptr) { return _thn_skill_desc(race_ptr->skills.thn, race_ptr->extra_skills.thn); }

static cptr _thn_skill_desc2(int base) { return _skill_desc(base + 5, 2); }
static cptr _race_thn_skill_desc(race_t *race_ptr) { return _thn_skill_desc2(race_ptr->skills.thn); }
static cptr _pers_thn_skill_desc(personality_ptr pers_ptr) { return _thn_skill_desc2(pers_ptr->skills.thn*2); }

/* Bows */
static cptr _thb_skill_desc(int base, int xtra) { return _skill_desc(base + 5*xtra - 60, 12); }
static cptr _class_thb_skill_desc(class_t *class_ptr) { return _thb_skill_desc(class_ptr->base_skills.thb, class_ptr->extra_skills.thb); }
static cptr _mon_race_thb_skill_desc(race_t *race_ptr) { return _thb_skill_desc(race_ptr->skills.thb, race_ptr->extra_skills.thb); }

static cptr _thb_skill_desc2(int base) { return _skill_desc(base + 9, 2); }
static cptr _race_thb_skill_desc(race_t *race_ptr) { return _thb_skill_desc2(race_ptr->skills.thb); }
static cptr _pers_thb_skill_desc(personality_ptr pers_ptr) { return _thb_skill_desc2(pers_ptr->skills.thb*2); }

/* Stealth */
static cptr _stl_skill_desc(int base, int xtra) { return _skill_desc(base + 5*xtra + 2, 1); }
static cptr _class_stl_skill_desc(class_t *class_ptr) { return _stl_skill_desc(class_ptr->base_skills.stl, class_ptr->extra_skills.stl); }
static cptr _mon_race_stl_skill_desc(race_t *race_ptr) { return _stl_skill_desc(race_ptr->skills.stl, race_ptr->extra_skills.stl); }
static cptr _race_stl_skill_desc(race_t *race_ptr) { return _stl_skill_desc(race_ptr->skills.stl, 0); }
static cptr _pers_stl_skill_desc(personality_ptr pers_ptr) { return _stl_skill_desc(pers_ptr->skills.stl, 0); }

/******************************************************************************
 * Racial Help
 ******************************************************************************/
/* TODO: This is copied/duplicated in birth.txt ... Spoiler generation is a convenience
   hack, so I'll turn a blind eye for now :) */
#define _MAX_RACES_PER_GROUP 23
#define _MAX_RACE_GROUPS      9
typedef struct _race_group_s {
    cptr name;
    int ids[_MAX_RACES_PER_GROUP];
} _race_group_t;
static _race_group_t _race_groups[_MAX_RACE_GROUPS] = {
    { "Humans",
        {RACE_AMBERITE, RACE_BARBARIAN, RACE_DEMIGOD, RACE_DUNADAN, RACE_HUMAN, RACE_IGOR, -1} },
    { "Elves",
        {RACE_DARK_ELF, RACE_HIGH_ELF, RACE_TOMTE, RACE_WOOD_ELF, -1} },
    { "Hobbits/Dwarves",
        {RACE_DWARF, RACE_GNOME, RACE_HOBBIT, RACE_NIBELUNG, -1} },
    { "Fairies",
        {RACE_SHADOW_FAIRY, RACE_SPRITE, -1} },
    { "Angels/Demons",
        {RACE_ARCHON, RACE_BALROG, RACE_IMP, -1} },
    { "Orcs/Trolls/Giants",
        {RACE_CYCLOPS, RACE_HALF_GIANT, RACE_OGRE, RACE_HALF_ORC,
         RACE_HALF_TITAN, RACE_HALF_TROLL, RACE_KOBOLD, RACE_SNOTLING, -1} },
    { "Shapeshifters",
        {RACE_BEORNING, RACE_DOPPELGANGER, RACE_WEREWOLF, -1} },
    { "The Undead",
        {RACE_EINHERI, RACE_SKELETON, RACE_SPECTRE, RACE_VAMPIRE, RACE_ZOMBIE, -1} },
    { "Other Races",
        {RACE_ANDROID, RACE_BEASTMAN, RACE_BOIT, RACE_CENTAUR, RACE_DRACONIAN, RACE_ENT,
         RACE_GOLEM, RACE_KLACKON, RACE_KUTAR, RACE_MIND_FLAYER, RACE_TONBERRY, RACE_YEEK,-1 } },
};

static void _race_help_table(FILE *fp, race_t *race_ptr)
{
    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     %+3d        Disarming   %s\n",
        race_ptr->stats[A_STR],
        _race_dis_skill_desc(race_ptr));

    fprintf(fp, "Intelligence %+3d        Device      %s\n",
        race_ptr->stats[A_INT],
        _race_dev_skill_desc(race_ptr));

    fprintf(fp, "Wisdom       %+3d        Save        %s\n",
        race_ptr->stats[A_WIS],
        _race_sav_skill_desc(race_ptr));

    fprintf(fp, "Dexterity    %+3d        Stealth     %s\n",
        race_ptr->stats[A_DEX],
        _race_stl_skill_desc(race_ptr));

    fprintf(fp, "Constitution %+3d        Searching   %s\n",
        race_ptr->stats[A_CON],
        _skill_desc(race_ptr->skills.srh, 1));

    fprintf(fp, "Charisma     %+3d        Perception  %s\n",
        race_ptr->stats[A_CHR],
        _skill_desc(race_ptr->skills.fos, 1));

    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        race_ptr->life,
        _race_thn_skill_desc(race_ptr));

    fprintf(fp, "Base HP      %3d        Bows        %s\n",
        race_ptr->base_hp,
        _race_thb_skill_desc(race_ptr));

    fprintf(fp, "Experience   %3d%%       Infravision %d'\n", race_ptr->exp, race_ptr->infra*10);
    fputs("</style></indent>\n", fp);
}

static void _race_help(FILE *fp, int idx)
{
    race_t *race_ptr = get_race_aux(idx, 0);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->name, race_ptr->name);
    fprintf(fp, "%s\n\n", race_ptr->desc);
    switch(idx)
    {
    case RACE_DEMIGOD:
        fputs("See <link:Demigods.txt> for more details on demigod parentage.\n\n", fp);
        break;
    case RACE_DRACONIAN:
        fputs("See <link:Draconians.txt> for more details on draconians.\n\n", fp);
        break;
    }

    _race_help_table(fp, race_ptr);
}

static void _races_help(FILE* fp)
{
    int i, j;

    fputs("<style:title>The Races</style>\n", fp);
    fputs("There are many races in the world, each, for the most part, with both "
          "strengths and weaknesses. Humans are the base race, and serve as the benchmark "
          "of comparison for all the others. In general, the stronger a race is relative to "
          "humans, the higher the <color:keyword>Experience Penalty</color> and the longer "
          "it will take to gain levels.\n\n"
          "For details on the <color:keyword>Primary Statistics</color>, see "
          "<link:birth.txt#PrimaryStats>. For information about the various <color:keyword>Skills</color>, see "
          "<link:birth.txt#PrimarySkills>. "
          "The skill descriptions in this document are for comparison purposes only. "
          "For example, your fledgling high-elf will not be born with <color:v>Legendary[2]</color> "
          "device skill. In general, skills are influenced by level, race, class, stats and equipment. "
          "To compare the various races, you might want to take a look "
          "at <link:Races.txt#Tables> the race tables below.\n\n", fp);
    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", _race_groups[i].name);
        for (j = 0; ; j++)
        {
            int race_idx = _race_groups[i].ids[j];
            if (race_idx == -1) break;
            _race_help(fp, race_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Race Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = _race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
                race_ptr->name,
                race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
                race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
                race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
            );
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Race Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = _race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _race_dis_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_dev_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_sav_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_stl_skill_desc(race_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Race Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "Searching", "Perception", "Melee", "Bows", "Infra");
    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = _race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _skill_desc(race_ptr->skills.srh, 1));
            fprintf(fp, " %s", _skill_desc(race_ptr->skills.fos, 1));
            fprintf(fp, " %s", _race_thn_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_thb_skill_desc(race_ptr));
            fprintf(fp, " %4d'", race_ptr->infra * 10);
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);
}

struct _name_desc_s { string_ptr name; string_ptr desc; };
typedef struct _name_desc_s _name_desc_t, *_name_desc_ptr;
static int _compare_name_desc(const _name_desc_ptr left, const _name_desc_ptr right) {
    return string_compare(left->name, right->name);
}
static void _name_desc_free(_name_desc_ptr p) {
    string_free(p->name);
    string_free(p->desc);
    free(p);
}
static _name_desc_ptr _name_desc_alloc(void) {
    _name_desc_ptr result = malloc(sizeof(_name_desc_t));
    result->name = string_alloc();
    result->desc = string_alloc();
    return result;
}

static void _demigods_help(FILE* fp)
{
    int i;

    fputs("<style:title>Demigod Parentage</style>\n\n", fp);
    fputs(get_race_aux(RACE_DEMIGOD, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);

        _race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Demigod Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Demigod Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Demigod Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "Searching", "Perception", "Melee", "Bows", "Infra");
    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.srh, 1));
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.fos, 1));
        fprintf(fp, " %s", _race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    {
        vec_ptr vec = vec_alloc((vec_free_f)_name_desc_free);

        fputs("<topic:Powers><style:heading>Table 4 - Demigod Special Powers</style>\n\n", fp);
        fputs("All demigods have access to special powers. When they reach level 20, they may choose "
                    "a single power from the following list. When they reach level, 40, they may choose another. "
                    "These powers can never be removed or changed, so you might want to study this list to "
                    "decide which powers you will choose for your character.\n\n", fp);

        for (i = 0; i < MAX_MUTATIONS; i++)
        {
            if (mut_demigod_pred(i))
            {
                char buf[1024];
                _name_desc_ptr nd = _name_desc_alloc();

                mut_name(i, buf);
                string_append_s(nd->name, buf);

                mut_help_desc(i, buf);
                string_append_s(nd->desc, buf);
                vec_add(vec, nd);
            }
        }

        vec_sort(vec, (vec_cmp_f)_compare_name_desc);

        for (i = 0; i < vec_length(vec); i++)
        {
            _name_desc_ptr nd = vec_get(vec, i);
            /*fprintf(fp, "<color:G>%s: </color>%s\n",*/
            fprintf(fp, "  <indent><color:G>%s</color>\n%s</indent>\n\n",
                string_buffer(nd->name), string_buffer(nd->desc));
        }

        vec_free(vec);
    }
    fputs("\n\n", fp);
}

static void _draconians_help(FILE* fp)
{
    int i;

    fputs("<style:title>Draconians</style>\n\n", fp);
    fputs(get_race_aux(RACE_DRACONIAN, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);

        _race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Draconian Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Draconian Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Draconian Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "Searching", "Perception", "Melee", "Bows", "Infra");
    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.srh, 1));
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.fos, 1));
        fprintf(fp, " %s", _race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    {
        vec_ptr vec = vec_alloc((vec_free_f)_name_desc_free);

        fputs("<topic:Powers><style:heading>Table 4 - Draconian Special Powers</style>\n\n", fp);
        fputs("All draconians have access to special powers. When they reach level 35, they may choose "
                "a single power from the following list. "
                "These powers can never be removed or changed, so you might want to study this list to "
                "decide which power you will choose for your character.\n\n", fp);

        for (i = 0; i < MAX_MUTATIONS; i++)
        {
            if (mut_draconian_pred(i))
            {
                char buf[1024];
                _name_desc_ptr nd = _name_desc_alloc();

                mut_name(i, buf);
                string_append_s(nd->name, buf);

                mut_help_desc(i, buf);
                string_append_s(nd->desc, buf);
                vec_add(vec, nd);
            }
        }

        vec_sort(vec, (vec_cmp_f)_compare_name_desc);

        for (i = 0; i < vec_length(vec); i++)
        {
            _name_desc_ptr nd = vec_get(vec, i);
            /*fprintf(fp, "<color:G>%s: </color>%s\n",*/
            fprintf(fp, "  <indent><color:G>%s</color>\n%s</indent>\n\n",
                string_buffer(nd->name), string_buffer(nd->desc));
        }

        vec_free(vec);
    }
    fputs("\n\n", fp);
}

/******************************************************************************
 * Monster Mode Help
 ******************************************************************************/
#define _MAX_MON_RACE_GROUPS      12
static _race_group_t _mon_race_groups[_MAX_MON_RACE_GROUPS] = {
    { "Animal",
        {/*RACE_MON_ANT, RACE_MON_BEETLE, RACE_MON_BIRD, RACE_MON_CAT,*/ RACE_MON_CENTIPEDE,
            RACE_MON_HOUND, /*RACE_MON_HORSE, */ RACE_MON_HYDRA, RACE_MON_SPIDER, -1} },
    { "Angel/Demon",
        {RACE_MON_ANGEL, RACE_MON_DEMON, -1} },
    { "Beholder",
        {RACE_MON_BEHOLDER, -1} },
    { "Dragon",
        {RACE_MON_DRAGON, -1} },
    { "Elemental/Vortex",
        {RACE_MON_ELEMENTAL, RACE_MON_VORTEX, -1} },
    { "Golem",
        {RACE_MON_GOLEM, -1} },
    { "Jelly",
        {RACE_MON_JELLY, /*RACE_MON_MOLD,*/ RACE_MON_QUYLTHULG, -1} },
    { "Leprechaun",
        {RACE_MON_LEPRECHAUN, -1} },
    { "Mimic/Possessor",
        {RACE_MON_SWORD, RACE_MON_ARMOR, RACE_MON_MIMIC, RACE_MON_POSSESSOR, RACE_MON_RING, -1} },
    { "Orc/Troll/Giant",
        {RACE_MON_GIANT, /*RACE_MON_KOBOLD,*/ RACE_MON_ORC, RACE_MON_TROLL, -1} },
    { "Undead",
        {/*RACE_MON_GHOST,*/ RACE_MON_LICH, RACE_MON_VAMPIRE, /*RACE_MON_WRAITH, RACE_MON_ZOMBIE,*/ -1 } },
    { "Other",
        {RACE_MON_PUMPKIN, RACE_MON_XORN, -1} },
};

static void _mon_race_help_table(FILE *fp, race_t *race_ptr)
{
    caster_info *caster_ptr = NULL;

    if (race_ptr->caster_info)
        caster_ptr = race_ptr->caster_info();

    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     <color:%c>%+3d</color>        Disarming   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_STR) ? 'v' : 'w',
        race_ptr->stats[A_STR],
        _mon_race_dis_skill_desc(race_ptr));
    fprintf(fp, "Intelligence <color:%c>%+3d</color>        Device      %s\n",
        (caster_ptr && caster_ptr->which_stat == A_INT) ? 'v' : 'w',
        race_ptr->stats[A_INT],
        _mon_race_dev_skill_desc(race_ptr));
    fprintf(fp, "Wisdom       <color:%c>%+3d</color>        Save        %s\n",
        (caster_ptr && caster_ptr->which_stat == A_WIS) ? 'v' : 'w',
        race_ptr->stats[A_WIS],
        _mon_race_sav_skill_desc(race_ptr));
    fprintf(fp, "Dexterity    <color:%c>%+3d</color>        Stealth     %s\n",
        (caster_ptr && caster_ptr->which_stat == A_DEX) ? 'v' : 'w',
        race_ptr->stats[A_DEX],
        _mon_race_stl_skill_desc(race_ptr));
    fprintf(fp, "Constitution <color:%c>%+3d</color>        Searching   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CON) ? 'v' : 'w',
        race_ptr->stats[A_CON],
        _skill_desc(race_ptr->skills.srh + 5*race_ptr->extra_skills.srh, 6));
    fprintf(fp, "Charisma     <color:%c>%+3d</color>        Perception  %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CHR) ? 'v' : 'w',
        race_ptr->stats[A_CHR],
        _skill_desc(race_ptr->skills.fos + 5*race_ptr->extra_skills.fos, 6));
    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        race_ptr->life,
        _mon_race_thn_skill_desc(race_ptr));
    fprintf(fp, "Base HP      %3d        Bows        %s\n",
        race_ptr->base_hp,
        _mon_race_thb_skill_desc(race_ptr));
    fprintf(fp, "Experience   %3d%%       Infravision %d'\n", race_ptr->exp, race_ptr->infra*10);
    fputs("</style></indent>\n", fp);
}

static void _mon_race_help(FILE *fp, int idx)
{
    race_t *race_ptr = get_race_aux(idx, 0);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->name, race_ptr->name);
    fprintf(fp, "%s\n\n", race_ptr->desc);
    switch(idx)
    {
    case RACE_MON_RING:
        fputs("See <link:rings.txt> for more details on rings.\n\n", fp);
        break;
    case RACE_MON_DRAGON:
        fputs("See <link:Dragons.txt> for more details on dragons.\n", fp);
        fputs("See <link:DragonRealms.txt> for more details on dragon realms.\n\n", fp);
        break;
    case RACE_MON_ORC:
        fputs("See <link:Orcs.txt> for more details on orcs.\n\n", fp);
        break;
    case RACE_MON_DEMON:
        fputs("See <link:Demons.txt> for more details on demons.\n\n", fp);
        return;
    }

    _mon_race_help_table(fp, race_ptr);
}


static void _monster_races_help(FILE* fp)
{
    int i, j;

    fprintf(fp, "<style:title>Monster Races</style>\n\n");
    fputs("So, you have decided to play as a monster. There are many options "
            "to choose from and the various types of monsters are loosely grouped "
            "by type below: Animals, Dragons, Demons, etc.\n\n"
            "As a monster, you won't be able to choose a class the way normal "
            "players do. Rather, most monster types gain abilities and powers "
            "as they gain experience. For example, dragons can breathe fire as "
            "well as access magical abilities. So you will want to check out both "
            "the magic command (<color:keypress>m</color>) as well as the racial "
            "power command (<color:keypress>U</color> or <color:keypress>O</color>) "
            "to see what powers are available as you play.\n\n"
            "In addition, most monsters have custom body types, and this will "
            "severely constrain the amount and kind of equipment you may wear. "
            "For example, a beholder cannot wear armor or wield a sword ... That "
            "would be an odd sight indeed! Instead, they may wear rings on their "
            "numerous eyestalks. Details of this kind should be described below.\n\n"
            "Finally, all monsters evolve. When they gain enough experience, they "
            "will assume a more powerful form. To continue with our example of dragons, "
            "you might evolve from Baby to Young to Mature and finally Ancient forms, "
            "becoming vastly more powerful in the process. The stats and skills listed "
            "in this document only apply to the starting form, which is usually "
            "very weak.\n\n"
            "For details on the <color:keyword>Primary Statistics</color>, see "
            "<link:birth.txt#PrimaryStats>. For information about the various "
            "<color:keyword>Skills</color>, see <link:birth.txt#PrimarySkills>. "
            "To compare the various races at a glance, you might want to take a "
            "look at <link:MonsterRaces.txt#Tables> the race tables below.\n\n", fp);

    for (i = 0; i < _MAX_MON_RACE_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", _mon_race_groups[i].name);
        for (j = 0; ; j++)
        {
            int race_idx = _mon_race_groups[i].ids[j];
            if (race_idx == -1) break;
            _mon_race_help(fp, race_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Race Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-12.12s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < _MAX_MON_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = _mon_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            if (race_idx == RACE_MON_DEMON) continue;

            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
                race_ptr->name,
                race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
                race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
                race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
            );
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Race Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < _MAX_MON_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = _mon_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            if (race_idx == RACE_MON_DEMON) continue;

            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Race Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "Searching", "Perception", "Melee", "Bows", "Infra");
    for (i = 0; i < _MAX_MON_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = _mon_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            if (race_idx == RACE_MON_DEMON) continue;

            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _skill_desc(race_ptr->skills.srh + 5*race_ptr->extra_skills.srh, 6));
            fprintf(fp, " %s", _skill_desc(race_ptr->skills.fos + 5*race_ptr->extra_skills.fos, 6));
            fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
            fprintf(fp, " %4d'", race_ptr->infra * 10);
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);
}

static void _demons_help(FILE* fp)
{
    int i;
    fputs("<style:title>Demons</style>\n\n", fp);
    fputs(get_race_aux(RACE_MON_DEMON, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);
        _mon_race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Demon Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Demon Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Demon Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "Searching", "Perception", "Melee", "Bows", "Infra");
    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.srh + 5*race_ptr->extra_skills.srh, 6));
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.fos + 5*race_ptr->extra_skills.fos, 6));
        fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _dragons_help(FILE* fp)
{
    int i;
    fputs("<style:title>Dragons</style>\n\n", fp);
    fputs(get_race_aux(RACE_MON_DRAGON, 0)->desc, fp);
    fputs("\n\n", fp);
    fputs("For more information on <color:keyword>Dragon Realms</color>, see <link:DragonRealms.txt>.\n\n", fp);

    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);
        _mon_race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Dragon Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Dragon Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Dragon Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "Searching", "Perception", "Melee", "Bows", "Infra");
    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.srh + 5*race_ptr->extra_skills.srh, 6));
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.fos + 5*race_ptr->extra_skills.fos, 6));
        fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _dragon_realms_help(FILE* fp)
{
    int i, j;
    fputs("<style:title>Dragon Realms</style>\n\n", fp);
    fputs("Dragons are magical creatures and may choose to learn a particular branch of "
           "dragon magic. Unlike normal spell casters, dragons do not need spell books to "
           "cast or learn powers. Instead, they simply gain spells as they mature. Each "
           "realm of dragon magic has a direct impact on the player's stats and skills, and "
           "each realm also requires a different stat for casting purposes.\n\n", fp);
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "<topic:%s><color:o>%s</color>\n", realm->name, realm->name);
        fputs(realm->desc, fp);
        fputs("\n\n", fp);

        fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
        fprintf(fp, "Strength     <color:%c>%+3d</color>        Disarming   %s\n",
            (realm->spell_stat == A_STR) ? 'v' : 'w',
            realm->stats[A_STR],
            _skill_desc(realm->skills.dis + 10, 2));
        fprintf(fp, "Intelligence <color:%c>%+3d</color>        Device      %s\n",
            (realm->spell_stat == A_INT) ? 'v' : 'w',
            realm->stats[A_INT],
            _skill_desc(realm->skills.dev + 5, 1));
        fprintf(fp, "Wisdom       <color:%c>%+3d</color>        Save        %s\n",
            (realm->spell_stat == A_WIS) ? 'v' : 'w',
            realm->stats[A_WIS],
            _skill_desc(realm->skills.sav + 5, 1));
        fprintf(fp, "Dexterity    <color:%c>%+3d</color>        Stealth     %s\n",
            (realm->spell_stat == A_DEX) ? 'v' : 'w',
            realm->stats[A_DEX],
            _skill_desc(realm->skills.stl + 4, 1));
        fprintf(fp, "Constitution <color:%c>%+3d</color>        Searching   %s\n",
            (realm->spell_stat == A_CON) ? 'v' : 'w',
            realm->stats[A_CON],
            _skill_desc(realm->skills.srh, 1));
        fprintf(fp, "Charisma     <color:%c>%+3d</color>        Perception  %s\n",
            (realm->spell_stat == A_CHR) ? 'v' : 'w',
            realm->stats[A_CHR],
            _skill_desc(realm->skills.fos, 1));
        fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
            realm->life,
            _skill_desc(realm->skills.thn + 10, 2));
        fprintf(fp, "Experience   %3d%%       Bows        %s\n",
            realm->exp,
            _skill_desc(realm->skills.thb + 10, 2));
        fprintf(fp, "Attack       %3d%%\n", realm->attack);
        fprintf(fp, "Breath       %3d%%\n", realm->breath);
        fputs("</style></indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Dragon Realm Statistic Bonus Table</style>\n\n", fp);
    fprintf(fp, "<style:table><color:G>%-14.14s STR  INT  WIS  DEX  CON  CHR  Life  Exp  Attack  Breath</color>\n", "");
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        char             line[255];
        char             tmp[255];

        sprintf(line, "%-14.14s", realm->name);
        for (j = 0; j < 6; j++)
        {
            if (j == realm->spell_stat)
                sprintf(tmp, "<color:v> %+3d </color>", realm->stats[j]);
            else
                sprintf(tmp, " %+3d ", realm->stats[j]);
            strcat(line, tmp);
        }
        sprintf(tmp, " %3d%%  %3d%% %5d%%  %5d%%", realm->life, realm->exp, realm->attack, realm->breath);
        strcat(line, tmp);
        fprintf(fp, "%s\n", line);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Dragon Realm Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-14.14s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "%-14.14s", realm->name);
        fprintf(fp, " %s", _skill_desc(realm->skills.dis + 10, 2));
        fprintf(fp, " %s", _skill_desc(realm->skills.dev + 5, 1));
        fprintf(fp, " %s", _skill_desc(realm->skills.sav + 5, 1));
        fprintf(fp, " %s", _skill_desc(realm->skills.stl + 4, 1));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Dragon Realm Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-14.14s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Searching", "Perception", "Melee", "Bows");
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "%-14.14s", realm->name);
        fprintf(fp, " %s", _skill_desc(realm->skills.srh, 1));
        fprintf(fp, " %s", _skill_desc(realm->skills.fos, 1));
        fprintf(fp, " %s", _skill_desc(realm->skills.thn + 10, 2));
        fprintf(fp, " %s", _skill_desc(realm->skills.thb + 10, 2));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _orcs_help(FILE* fp)
{
    int i;
    fputs("<style:title>Orcs</style>\n\n", fp);
    fputs(get_race_aux(RACE_MON_ORC, ORC_FIGHTER)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);
        _mon_race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Orc Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Orc Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Orc Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "Searching", "Perception", "Melee", "Bows", "Infra");
    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.srh + 5*race_ptr->extra_skills.srh, 6));
        fprintf(fp, " %s", _skill_desc(race_ptr->skills.fos + 5*race_ptr->extra_skills.fos, 6));
        fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

/******************************************************************************
 * Class Help
 ******************************************************************************/
static void _class_help_table(FILE *fp, class_t *class_ptr)
{
    caster_info *caster_ptr = NULL;

    if (class_ptr->caster_info && !streq(class_ptr->name, "Psion") && !streq(class_ptr->name, "Wild-Talent"))
        caster_ptr = class_ptr->caster_info();

    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     <color:%c>%+3d</color>        Disarming   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_STR) ? 'v' : 'w',
        class_ptr->stats[A_STR],
        _class_dis_skill_desc(class_ptr));
    fprintf(fp, "Intelligence <color:%c>%+3d</color>        Device      %s\n",
        (caster_ptr && caster_ptr->which_stat == A_INT) ? 'v' : 'w',
        class_ptr->stats[A_INT],
        _class_dev_skill_desc(class_ptr));
    fprintf(fp, "Wisdom       <color:%c>%+3d</color>        Save        %s\n",
        (caster_ptr && caster_ptr->which_stat == A_WIS) ? 'v' : 'w',
        class_ptr->stats[A_WIS],
        _class_sav_skill_desc(class_ptr));
    fprintf(fp, "Dexterity    <color:%c>%+3d</color>        Stealth     %s\n",
        (caster_ptr && caster_ptr->which_stat == A_DEX) ? 'v' : 'w',
        class_ptr->stats[A_DEX],
        _class_stl_skill_desc(class_ptr));
    fprintf(fp, "Constitution <color:%c>%+3d</color>        Searching   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CON) ? 'v' : 'w',
        class_ptr->stats[A_CON],
        _skill_desc(class_ptr->base_skills.srh + 5*class_ptr->extra_skills.srh, 6));
    fprintf(fp, "Charisma     <color:%c>%+3d</color>        Perception  %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CHR) ? 'v' : 'w',
        class_ptr->stats[A_CHR],
        _skill_desc(class_ptr->base_skills.fos + 5*class_ptr->extra_skills.fos, 6));
    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        class_ptr->life,
        _class_thn_skill_desc(class_ptr));
    fprintf(fp, "Base HP      %3d        Bows        %s\n",
        class_ptr->base_hp,
        _class_thb_skill_desc(class_ptr));
    fprintf(fp, "Experience   %3d%%\n", class_ptr->exp);
    fputs("</style></indent>\n", fp);
}

static void _class_help(FILE *fp, int idx)
{
    class_t *class_ptr = get_class_aux(idx, 0);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->name, class_ptr->name);
    fputs(class_ptr->desc, fp);
    fputs("\n\n", fp);

    switch(idx)
    {
    case CLASS_WARLOCK:
        fputs("See <link:Warlocks.txt> for more details on warlock pacts.\n\n", fp);
        break;
    case CLASS_WEAPONMASTER:
        fputs("See <link:Weaponmasters.txt> for more details on weaponmasters.\n\n", fp);
        break;
    case CLASS_SKILLMASTER:
        fputs("See <link:Skillmasters.txt> for more details on skillmasters.\n\n", fp);
        break;
    case CLASS_DISCIPLE:
        fputs("See <link:Disciples.txt> for more details on disciples.\n\n", fp);
        break;
    case CLASS_RUNE_KNIGHT:
        fputs("See <link:Runeknights.txt> for more details on rune knights.\n\n", fp);
        break;
    }

    _class_help_table(fp, class_ptr);
}

#define _MAX_CLASSES_PER_GROUP 20
#define _MAX_CLASS_GROUPS      11
typedef struct _class_group_s {
    cptr name;
    int ids[_MAX_CLASSES_PER_GROUP];
} _class_group_t;
static _class_group_t _class_groups[_MAX_CLASS_GROUPS] = {
    { "Melee", {CLASS_BERSERKER, CLASS_BLOOD_KNIGHT, CLASS_DUELIST, CLASS_MAULER,
                    CLASS_RUNE_KNIGHT, CLASS_SAMURAI, CLASS_WARRIOR, CLASS_WEAPONMASTER,
                    CLASS_WEAPONSMITH, -1} },
    { "Archery", {CLASS_ARCHER, CLASS_SNIPER, -1} },
    { "Martial Arts", {CLASS_FORCETRAINER, CLASS_MONK, CLASS_MYSTIC, -1} },
    { "Magic", {CLASS_BLOOD_MAGE, CLASS_GRAY_MAGE, CLASS_HIGH_MAGE, CLASS_MAGE,
                    CLASS_NECROMANCER, CLASS_SORCERER, CLASS_YELLOW_MAGE, -1} },
    { "Devices", {CLASS_ALCHEMIST, CLASS_DEVICEMASTER, CLASS_MAGIC_EATER, -1} },
    { "Prayer", {CLASS_PRIEST, -1} },
    { "Stealth", {CLASS_NINJA, CLASS_ROGUE, CLASS_SCOUT, -1} },
    { "Hybrid", {CLASS_CHAOS_WARRIOR, CLASS_DISCIPLE, CLASS_NINJA_LAWYER, CLASS_PALADIN,
                    CLASS_RANGER, CLASS_RED_MAGE, CLASS_WARRIOR_MAGE,  -1} },
    { "Riding", {CLASS_BEASTMASTER, CLASS_CAVALRY, -1} },
    { "Mind", {CLASS_MINDCRAFTER, CLASS_MIRROR_MASTER, CLASS_PSION,
                    CLASS_TIME_LORD, CLASS_WARLOCK, -1} },
    { "Other", {CLASS_ARCHAEOLOGIST, CLASS_BARD, CLASS_LAWYER, CLASS_POLITICIAN,
                    CLASS_RAGE_MAGE, CLASS_SKILLMASTER, CLASS_TOURIST, CLASS_WILD_TALENT, -1} },
};

static void _classes_help(FILE* fp)
{
    int i, j, k;

    fputs("<style:title>The Classes</style>\n\n", fp);
    fputs("No decision is so important as which class to play. Below, the many "
            "available classes are loosely grouped by their principle playstyle: "
            "Melee, Archery, Martial Arts, Magic, Devices, etc. The hybrid classes "
            "generally combine melee with a bit of magic and are a good option for "
            "a balanced playstyle. In this document, the primary spell stat for "
            "each class is <color:v>highlighted</color>.\n\n"
            "For details on the <color:keyword>Primary Statistics</color>, see "
            "<link:birth.txt#PrimaryStats>. For information about the various "
            "<color:keyword>Skills</color>, see <link:birth.txt#PrimarySkills>. "
            "The skill descriptions in this document are for comparison purposes only. "
            "For example, your fledgling berserker will not be born with <color:v>Legendary[32]</color> "
            "melee skill. In general, skills are influenced by level, race, class, stats and equipment. "
            "To compare the various classes at a glance, you might want to take "
            "a look at <link:Classes.txt#Tables> the class tables below.\n\n", fp);

    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", _class_groups[i].name);
        for (j = 0; ; j++)
        {
            int class_idx = _class_groups[i].ids[j];
            if (class_idx == -1) break;
            _class_help(fp, class_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Class Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "%-13.13s <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int          class_idx = _class_groups[i].ids[j];
            class_t     *class_ptr;
            caster_info *caster_ptr = NULL;
            char         line[255];
            char         tmp[255];

            if (class_idx == -1) break;
            class_ptr = get_class_aux(class_idx, 0);
            if (class_ptr->caster_info)
                caster_ptr = class_ptr->caster_info();

            sprintf(line, "%-13.13s", class_ptr->name);
            for (k = 0; k < 6; k++)
            {
                if (caster_ptr && k == caster_ptr->which_stat && class_idx != CLASS_PSION && class_idx != CLASS_WILD_TALENT)
                    sprintf(tmp, "<color:v> %+3d </color>", class_ptr->stats[k]);
                else
                    sprintf(tmp, " %+3d ", class_ptr->stats[k]);
                strcat(line, tmp);
            }
            sprintf(tmp, " %3d%%  %+3d  %3d%%", class_ptr->life, class_ptr->base_hp, class_ptr->exp);
            strcat(line, tmp);
            fprintf(fp, "%s\n", line);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Class Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-13.13s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     class_idx = _class_groups[i].ids[j];
            class_t *class_ptr;

            if (class_idx == -1) break;
            class_ptr = get_class_aux(class_idx, 0);
            fprintf(fp, "%-13.13s", class_ptr->name);
            fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Class Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-13.13s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Searching", "Perception", "Melee", "Bows");
    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     class_idx = _class_groups[i].ids[j];
            class_t *class_ptr;

            if (class_idx == -1) break;
            class_ptr = get_class_aux(class_idx, 0);
            fprintf(fp, "%-13.13s", class_ptr->name);
            fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.srh + 5*class_ptr->extra_skills.srh, 6));
            fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.fos + 5*class_ptr->extra_skills.fos, 6));
            fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);
}

static void _disciples_help(FILE *fp)
{
    int i;
    fputs("<style:title>Disciples</style>\n\n", fp);
    fputs(get_class_aux(CLASS_DISCIPLE, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->subname, class_ptr->subname);
        fprintf(fp, "%s\n\n", class_ptr->subdesc);
        _class_help_table(fp, class_ptr);
    }

    fprintf(fp, "<color:B>TIP</color>: The special random quests for Karrot and Troika disciples can only appear ");
    fprintf(fp, "if you are sufficiently deep for the quest to be challenging. Pressing <color:y>CTRL+F</color> ");
    fprintf(fp, "reveals your patron's opinion of your current depth. The Purples also like you to visit ");
    fprintf(fp, "many different dungeons; do not expect a large number of assignments in a single dungeon, unless ");
    fprintf(fp, "you are playing with no wilderness.\n\n");

    fputs("<topic:Tables><style:heading>Table 1 - Disciple Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            class_ptr->subname,
            class_ptr->stats[A_STR], class_ptr->stats[A_INT], class_ptr->stats[A_WIS],
            class_ptr->stats[A_DEX], class_ptr->stats[A_CON], class_ptr->stats[A_CHR],
            class_ptr->life, class_ptr->base_hp, class_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Disciple Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Disciple Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Searching", "Perception", "Melee", "Bows");
    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.srh + 5*class_ptr->extra_skills.srh, 6));
        fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.fos + 5*class_ptr->extra_skills.fos, 6));
        fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
    yeqrezh_help(fp);
    fputs("\n</style>\n", fp);
}

static void _weaponmasters_help(FILE *fp)
{
    int i;
    fputs("<style:title>Weaponmasters</style>\n\n", fp);
    fputs(get_class_aux(CLASS_WEAPONMASTER, WEAPONMASTER_AXES)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->subname, class_ptr->subname);
        fprintf(fp, "%s\n\n", class_ptr->subdesc);
        _class_help_table(fp, class_ptr);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Weaponmaster Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            class_ptr->subname,
            class_ptr->stats[A_STR], class_ptr->stats[A_INT], class_ptr->stats[A_WIS],
            class_ptr->stats[A_DEX], class_ptr->stats[A_CON], class_ptr->stats[A_CHR],
            class_ptr->life, class_ptr->base_hp, class_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Weaponmaster Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Weaponmaster Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Searching", "Perception", "Melee", "Bows");
    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.srh + 5*class_ptr->extra_skills.srh, 6));
        fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.fos + 5*class_ptr->extra_skills.fos, 6));
        fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _warlocks_help(FILE *fp)
{
    int i;
    fputs("<style:title>Warlocks</style>\n\n", fp);
    fputs(get_class_aux(CLASS_WARLOCK, WARLOCK_UNDEAD)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->subname, class_ptr->subname);
        fprintf(fp, "%s\n\n", class_ptr->subdesc);
        _class_help_table(fp, class_ptr);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Warlock Statistic Bonus Table</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            class_ptr->subname,
            class_ptr->stats[A_STR], class_ptr->stats[A_INT], class_ptr->stats[A_WIS],
            class_ptr->stats[A_DEX], class_ptr->stats[A_CON], class_ptr->stats[A_CHR],
            class_ptr->life, class_ptr->base_hp, class_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Warlock Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Warlock Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Searching", "Perception", "Melee", "Bows");
    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.srh + 5*class_ptr->extra_skills.srh, 6));
        fprintf(fp, " %s", _skill_desc(class_ptr->base_skills.fos + 5*class_ptr->extra_skills.fos, 6));
        fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

/******************************************************************************
 * Personality Help
 ******************************************************************************/
static void _personality_help(FILE *fp, int idx)
{
    personality_ptr pers_ptr = get_personality_aux(idx);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", pers_ptr->name, pers_ptr->name);
    fprintf(fp, "%s\n\n", pers_ptr->desc);

    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     %+3d        Disarming   %s\n",
        pers_ptr->stats[A_STR],
        _pers_dis_skill_desc(pers_ptr));

    fprintf(fp, "Intelligence %+3d        Device      %s\n",
        pers_ptr->stats[A_INT],
        _pers_dev_skill_desc(pers_ptr));

    fprintf(fp, "Wisdom       %+3d        Save        %s\n",
        pers_ptr->stats[A_WIS],
        _pers_sav_skill_desc(pers_ptr));

    fprintf(fp, "Dexterity    %+3d        Stealth     %s\n",
        pers_ptr->stats[A_DEX],
        _pers_stl_skill_desc(pers_ptr));

    fprintf(fp, "Constitution %+3d        Searching   %s\n",
        pers_ptr->stats[A_CON],
        _skill_desc(pers_ptr->skills.srh + 3, 1));

    fprintf(fp, "Charisma     %+3d        Perception  %s\n",
        pers_ptr->stats[A_CHR],
        _skill_desc(pers_ptr->skills.fos + 3, 1));

    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        pers_ptr->life,
        _pers_thn_skill_desc(pers_ptr));

    fprintf(fp, "Experience   %3d%%       Bows        %s\n",
        pers_ptr->exp,
        _pers_thb_skill_desc(pers_ptr));
    fputs("</style></indent>\n", fp);
}

static void _personalities_help(FILE* fp)
{
    int i;

    fprintf(fp, "<style:title>The Personalities</style>\n\n");
    fputs("Your personality is the way you see and act in the world, and has "
            "a small but significant effect on your stats and skills. In general "
            "you should pick a personality that complements the strengths and "
            "weaknesses inherent in your race and class. Your choice of personality "
            "is the least important decision you make when creating your "
            "character (after gender).\n\n"
            "For details on the <color:keyword>Primary Statistics</color>, see "
            "<link:birth.txt#PrimaryStats>. For information about the various <color:keyword>Skills</color>, see "
            "<link:birth.txt#PrimarySkills>. "
            "The skill descriptions in this document are for comparison purposes only. "
            "For example, your fledgling munchkin will not be born with <color:v>Legendary[20]</color> "
            "melee skill. In general, skills are also influenced by level, race, class, stats and equipment. "
            "To compare the various personalities, you might want to take a look "
            "at <link:Personalities.txt#Tables> the personality tables below.\n\n", fp);
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (pers_ptr->flags & DEPRECATED) continue;
        _personality_help(fp, i);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Personality Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp</color>\n", fp);

    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);

        if (pers_ptr->flags & DEPRECATED) continue;
        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%%\n",
            pers_ptr->name,
            pers_ptr->stats[0], pers_ptr->stats[1], pers_ptr->stats[2],
            pers_ptr->stats[3], pers_ptr->stats[4], pers_ptr->stats[5],
            pers_ptr->life, pers_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>Table 2 - Personality Skill Bonus Table I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Disarming", "Device", "Save", "Stealth");
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (pers_ptr->flags & DEPRECATED) continue;
        fprintf(fp, "%-12.12s", pers_ptr->name);
        fprintf(fp, " %s", _pers_dis_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_dev_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_sav_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_stl_skill_desc(pers_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>Table 3 - Personality Skill Bonus Table II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "Searching", "Perception", "Melee", "Bows");
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (pers_ptr->flags & DEPRECATED) continue;
        fprintf(fp, "%-12.12s", pers_ptr->name);
        fprintf(fp, " %s", _skill_desc(pers_ptr->skills.srh + 3, 1));
        fprintf(fp, " %s", _skill_desc(pers_ptr->skills.fos + 3, 1));
        fprintf(fp, " %s", _pers_thn_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_thb_skill_desc(pers_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

/******************************************************************************
 * Spoilers: All the various possessor body types
 ******************************************************************************/
static void _possessor_stats_table(FILE* fp)
{
    int i;
    fprintf(fp, "Name,Idx,Lvl,Speed,AC,Attacks,Dam,Body,Str,Int,Wis,Dex,Con,Chr,Life,Disarm,Device,Save,Stealth,Search,Perception,Melee,Bows\n");
    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /*XXX if (r_ptr->flags9 & RF9_DROP_CORPSE)*/
        {
            int ac = 0, dam = 0, attacks = 0/*, j*/;

            if (r_ptr->flags9 & RF9_POS_GAIN_AC)
                ac = r_ptr->ac;

            #if 0
            for (j = 0; j < 4; j++)
            {
                if (!r_ptr->blow[j].effect) continue;
                if (r_ptr->blow[j].method == RBM_EXPLODE) continue;

                dam += r_ptr->blow[j].d_dice * (r_ptr->blow[j].d_side + 1) / 2;
                attacks++;
            }
            #endif

            fprintf(fp, "\"%s\",%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,=\"%d+%d\",=\"%d+%d\",=\"%d+%d\",%d,%d,%d,=\"%d+%d\",=\"%d+%d\"\n",
                i == MON_ECHIZEN ? "Combat Echizen" : r_name + r_ptr->name, i, r_ptr->level,
                r_ptr->speed - 110, ac, attacks, dam,
                b_name + b_info[r_ptr->body.body_idx].name,
                r_ptr->body.stats[A_STR], r_ptr->body.stats[A_INT], r_ptr->body.stats[A_WIS],
                r_ptr->body.stats[A_DEX], r_ptr->body.stats[A_CON], r_ptr->body.stats[A_CHR],
                r_ptr->body.life,
                r_ptr->body.skills.dis, r_ptr->body.extra_skills.dis,
                r_ptr->body.skills.dev, r_ptr->body.extra_skills.dev,
                r_ptr->body.skills.sav, r_ptr->body.extra_skills.sav,
                r_ptr->body.skills.stl,
                r_ptr->body.skills.srh,
                r_ptr->body.skills.fos,
                r_ptr->body.skills.thn, r_ptr->body.extra_skills.thn,
                r_ptr->body.skills.thb, r_ptr->body.extra_skills.thb
            );
        }
    }
}

/******************************************************************************
 * Spoilers: Skill Tables with Actual Numbers for Design Purposes
 ******************************************************************************/
static void _skills_race_table(FILE* fp)
{
    int i,j;
    fputs("Race,Dis,Dev,Sav,Stl,Srh,Fos,Thn,Thb,Stats,Exp\n", fp);
    for (i = 0; i < MAX_RACES; i++)
    {
        int max_j = 1;
        if (i == RACE_DEMIGOD)
            max_j = DEMIGOD_MAX;
        else if (i == RACE_DRACONIAN)
            max_j = DRACONIAN_MAX;

        for (j = 0; j < max_j; j++)
        {
            race_t *race_ptr = get_race_aux(i, j);
            int     stats = 0, k;

            if (race_ptr->flags & RACE_IS_MONSTER) continue;

            for (k = 0; k < MAX_STATS; k++)
                stats += race_ptr->stats[k];

            if (race_ptr->subname && strlen(race_ptr->subname))
                fprintf(fp, "\"%s:%s\",", race_ptr->name, race_ptr->subname);
            else
                fprintf(fp, "\"%s\",", race_ptr->name);
            fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                race_ptr->skills.dis,
                race_ptr->skills.dev,
                race_ptr->skills.sav,
                race_ptr->skills.stl,
                race_ptr->skills.srh,
                race_ptr->skills.fos,
                race_ptr->skills.thn,
                race_ptr->skills.thb,
                stats,
                race_ptr->exp
            );
        }
    }
}

static void _skills_class_table(FILE* fp)
{
    int i,j;
    fputs("Class,Dis,Dev,Sav,Stl,Srh,Fos,Thn,Thb,Dis2,Dev2,Sav2,Thn2,Thb2,Life,BHP,Riding,DualWielding\n", fp);
    for (i = 0; i < MAX_CLASS; i++)
    {
        int max_j = 1;

        if (i == CLASS_MONSTER || i == CLASS_XXX12 || i == CLASS_XXX21)
            continue;
        else if (i == CLASS_WEAPONMASTER)
            max_j = WEAPONMASTER_MAX;
        else if (i == CLASS_WARLOCK)
            max_j = WARLOCK_MAX;

        for (j = 0; j < max_j; j++)
        {
            class_t *class_ptr = get_class_aux(i, j);

            if (class_ptr->subname && strlen(class_ptr->subname))
                fprintf(fp, "\"%s:%s\",", class_ptr->name, class_ptr->subname);
            else
                fprintf(fp, "\"%s\",", class_ptr->name);
            fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d+%d,%d+%d,%d+%d,%d+%d,%d+%d,%d,%d,%d,%d\n",
                class_ptr->base_skills.dis + 5*class_ptr->extra_skills.dis,
                class_ptr->base_skills.dev + 5*class_ptr->extra_skills.dev,
                class_ptr->base_skills.sav + 5*class_ptr->extra_skills.sav,
                class_ptr->base_skills.stl + 5*class_ptr->extra_skills.stl,
                class_ptr->base_skills.srh,
                class_ptr->base_skills.fos,
                class_ptr->base_skills.thn + 5*class_ptr->extra_skills.thn,
                class_ptr->base_skills.thb + 5*class_ptr->extra_skills.thb,
                class_ptr->base_skills.dis, class_ptr->extra_skills.dis,
                class_ptr->base_skills.dev, class_ptr->extra_skills.dev,
                class_ptr->base_skills.sav, class_ptr->extra_skills.sav,
                class_ptr->base_skills.thn, class_ptr->extra_skills.thn,
                class_ptr->base_skills.thb, class_ptr->extra_skills.thb,
                class_ptr->life, class_ptr->base_hp,
                s_info[i].s_max[SKILL_RIDING],
                s_info[i].s_max[SKILL_DUAL_WIELDING]
            );
        }
    }
}

static void _spells_table(FILE* fp) /*m_info.txt*/
{
    int class_idx, realm_idx, spell_idx;
    fputs("Class,Realm,Index,Name,Level,Cost,Fail\n", fp);
    for (class_idx = 0; class_idx < MAX_CLASS; class_idx++)
    {
        class_t      *class_ptr;
        player_magic *magic_ptr;
        if (class_idx == CLASS_XXX12 || class_idx == CLASS_XXX21) continue;
        class_ptr = get_class_aux(class_idx, 0);
        magic_ptr = &m_info[class_idx];
        for (realm_idx = REALM_LIFE; realm_idx <= MAX_MAGIC; realm_idx++)
        {
            for (spell_idx = 0; spell_idx < 32; spell_idx++)
            {
                magic_type *spell_ptr = &magic_ptr->info[realm_idx-1][spell_idx];
                if (0 < spell_ptr->slevel && spell_ptr->slevel <= PY_MAX_LEVEL)
                {
                    fprintf(fp, "\"%s\",\"%s\",%d,\"%s\",%d,%d,%d\n",
                        class_ptr->name,
                        realm_names[realm_idx],
                        spell_idx+1,
                        do_spell(realm_idx, spell_idx, SPELL_NAME),
                        spell_ptr->slevel,
                        spell_ptr->smana,
                        spell_ptr->sfail
                    );
                }
            }
        }
    }
}

/******************************************************************************
 * Auto-generate HTML and TEXT files from the help system
 ******************************************************************************/
typedef struct {
    string_ptr dir;
    string_ptr base;
    string_ptr ext;
} _file_parts_t, *_file_parts_ptr;
static _file_parts_ptr _file_parts_alloc(void)
{
    _file_parts_ptr result = malloc(sizeof(_file_parts_t));
    result->dir = string_alloc();
    result->base = string_alloc();
    result->ext = string_alloc();
    return result;
}
static void _file_parts_free(_file_parts_ptr fp)
{
    if (fp)
    {
        string_free(fp->dir);
        string_free(fp->base);
        string_free(fp->ext);
        free(fp);
    }
}
static string_ptr _file_parts_build_fullname(_file_parts_ptr fp)
{
    string_ptr result = string_copy(fp->dir);

    string_append_s(result, PATH_SEP);
    string_append(result, fp->base);
    if (string_length(fp->ext))
    {
        string_append_c(result, '.');
        string_append(result, fp->ext);
    }
    return result;
}
static void _file_parts_change_name(_file_parts_ptr fp, cptr filename)
{
    string_ptr s = string_copy_s(filename);
    int        pos = string_last_chr(s, '.');

    string_clear(fp->base);
    string_clear(fp->ext);

    if (pos >= 0)
    {
        string_append_sn(fp->base, string_buffer(s), pos);
        string_append_s(fp->ext, string_buffer(s) + pos + 1);
    }
    else
    {
        string_append(fp->base, s);
    }
    string_free(s);
}
static void _file_parts_change_extension(_file_parts_ptr fp, cptr ext)
{
    string_clear(fp->ext);
    string_append_s(fp->ext, ext);
}
static void _file_parts_extend_path(_file_parts_ptr fp, cptr dirname)
{
    char buf[1024];
    path_build(buf, sizeof(buf), string_buffer(fp->dir), dirname);
    string_clear(fp->dir);
    string_append_s(fp->dir, buf);
}

static void _generate_html_help_aux(cptr name, str_map_ptr prev, int format)
{
    if (!str_map_contains(prev, name))
    {
        int              i;
        doc_ptr          doc;
        FILE            *fff;
        _file_parts_ptr  dfp;
        string_ptr       dest_path;
        char             src_path[1024];
        vec_ptr          links;

        /* Read Source Document*/
        path_build(src_path, sizeof(src_path), ANGBAND_DIR_HELP, name);
        str_map_add(prev, name, 0); /* optimism */

        fff = my_fopen(src_path, "r");
        if (!fff)
            return;

        doc = doc_alloc(80);
        doc_read_file(doc, fff);
        my_fclose(fff);

        /* Output Dest Document */
        dfp = _file_parts_alloc();
        string_append_s(dfp->dir, ANGBAND_DIR_HELP);
        if (format == DOC_FORMAT_HTML)
        {
            _file_parts_extend_path(dfp, "html");
            _file_parts_change_name(dfp, name);
            _file_parts_change_extension(dfp, "html");
        }
        else
        {
            _file_parts_extend_path(dfp, "text");
            _file_parts_change_name(dfp, name);
            _file_parts_change_extension(dfp, "txt");
        }
        dest_path = _file_parts_build_fullname(dfp);

        fff = my_fopen(string_buffer(dest_path), "w");
        if (fff)
        {
            doc_write_file(doc, fff, format);
            my_fclose(fff);
        }
        _file_parts_free(dfp);
        string_free(dest_path);

        /* Recurse On Links */
        links = doc_get_links(doc);
        for (i = 0; i < vec_length(links); i++)
        {
            doc_link_ptr link = vec_get(links, i);
            _generate_html_help_aux(string_buffer(link->file), prev, format);
        }
        vec_free(links);

        doc_free(doc);
    }
}

static void _generate_html_help(void)
{
    str_map_ptr prev = str_map_alloc(NULL);
    _generate_html_help_aux("start.txt", prev, DOC_FORMAT_HTML);
    str_map_free(prev);
}

static void _generate_text_help(void)
{
    str_map_ptr prev = str_map_alloc(NULL);
    _generate_html_help_aux("start.txt", prev, DOC_FORMAT_TEXT);
    str_map_free(prev);
}

void generate_spoilers(void)
{
    int taso = p_ptr->lev;
    p_ptr->lev = 35; /* stats may vary based on the player's level, we use 35 to at least be consistent */
    spoiler_hack = TRUE;

    _help_file("Races.txt", _races_help);
    _help_file("Demigods.txt", _demigods_help);
    _help_file("Draconians.txt", _draconians_help);

    _help_file("Classes.txt", _classes_help);
    _help_file("Weaponmasters.txt", _weaponmasters_help);
    _help_file("Warlocks.txt", _warlocks_help);
    _help_file("Disciples.txt", _disciples_help);

    _help_file("Personalities.txt", _personalities_help);

    _help_file("MonsterRaces.txt", _monster_races_help);
    _help_file("Demons.txt", _demons_help);
    _help_file("Dragons.txt", _dragons_help);
    _help_file("DragonRealms.txt", _dragon_realms_help);
    _help_file("Orcs.txt", _orcs_help);

    _csv_file("PossessorStats.csv", _possessor_stats_table);
    _csv_file("Skills-Racial.csv", _skills_race_table);
    _csv_file("Skills-Class.csv", _skills_class_table);
    /*_csv_file("Skills-Monster.csv", _skills_mon_table);*/
    _csv_file("Spells.csv", _spells_table);

    _generate_html_help();
    _generate_text_help();
    spoiler_hack = FALSE;
    p_ptr->lev = taso;
}

#endif
