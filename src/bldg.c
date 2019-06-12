/* File: bldg.c */

/*
 * Purpose: Building commands
 * Created by Ken Wigle for Kangband - a variant of Angband 2.8.3
 * -KMW-
 *
 * Rewritten for Kangband 2.8.3i using Kamband's version of
 * bldg.c as written by Ivan Tkatchev
 *
 * Changed for ZAngband by Robert Ruehlmann
 */

#include "angband.h"
#include "equip.h"
#include <assert.h>

/* hack as in leave_store in store.c */
static bool leave_bldg = FALSE;
static bool paivita = FALSE;
static bool paivitys_no_inkey_hack = FALSE;

int get_bldg_member_code(cptr name)
{
    if (strcmp(name, "None") == 0)
        return BUILDING_NON_MEMBER;

    if (strcmp(name, "Owner") == 0)
        return BUILDING_OWNER;

    if (strcmp(name, "Member") == 0)
        return BUILDING_MEMBER;

    return -1;
}

static bool is_owner(building_type *bldg)
{
    int race_idx = (p_ptr->mimic_form != MIMIC_NONE) ? p_ptr->mimic_form : p_ptr->prace;

    if (bldg->member_class[p_ptr->pclass] == BUILDING_OWNER)
    {
        return (TRUE);
    }

    if (race_idx < MAX_RACES && bldg->member_race[race_idx] == BUILDING_OWNER)
    {
        return (TRUE);
    }

    if ((is_magic(p_ptr->realm1) && (bldg->member_realm[p_ptr->realm1] == BUILDING_OWNER)) ||
        (is_magic(p_ptr->realm2) && (bldg->member_realm[p_ptr->realm2] == BUILDING_OWNER)))
    {
        return (TRUE);
    }

    return (FALSE);
}


static bool is_member(building_type *bldg)
{
    int race_idx = (p_ptr->mimic_form != MIMIC_NONE) ? p_ptr->mimic_form : p_ptr->prace;

    if (bldg->member_class[p_ptr->pclass])
    {
        return (TRUE);
    }

    if (race_idx < MAX_RACES && bldg->member_race[race_idx])
    {
        return (TRUE);
    }

    if ((is_magic(p_ptr->realm1) && bldg->member_realm[p_ptr->realm1]) ||
        (is_magic(p_ptr->realm2) && bldg->member_realm[p_ptr->realm2]))
    {
        return (TRUE);
    }


    if (p_ptr->pclass == CLASS_SORCERER)
    {
        int i;
        bool OK = FALSE;
        for (i = 0; i < MAX_MAGIC; i++)
        {
            if (bldg->member_realm[i+1]) OK = TRUE;
        }
        return OK;
    }
    return (FALSE);
}


/*
 * Clear the building information
 */
static void clear_bldg(int min_row, int max_row)
{
    int   i;

    for (i = min_row; i <= max_row; i++)
        prt("", i, 0);
}

static void building_prt_gold(void)
{
    char tmp[10];
    char tmp_str[80];

    prt("Gold Remaining: ", 23, 53);

    big_num_display(p_ptr->au, tmp);
    sprintf(tmp_str, "%6.6s", tmp);
    prt(tmp_str, 23, 68);
}


/*
 * Display a building.
 */
static void show_building(building_type* bldg)
{
    char buff[20];
    int i;
    byte action_color;
    char tmp_str[80];

    Term_clear();
    sprintf(tmp_str, "%s (%s) %35s", bldg->owner_name, bldg->owner_race, bldg->name);
    prt(tmp_str, 3, 1);


    for (i = 0; i < 8; i++)
    {
        if (bldg->letters[i])
        {
            int member_cost = town_service_price(bldg->member_costs[i]);
            int other_cost = town_service_price(bldg->other_costs[i]);
            bool owner = is_owner(bldg);
            bool member = is_member(bldg);
            int cost = owner ? member_cost : other_cost;

            if (bldg->action_restr[i] == 0)
            {
                if (cost == 0)
                {
                    action_color = TERM_WHITE;
                    buff[0] = '\0';
                }
                else
                {
                    action_color = TERM_YELLOW;
                    sprintf(buff, "(%dgp)", cost);
                }
            }
            else if (bldg->action_restr[i] == 1)
            {
                if (!member)
                {
                    action_color = TERM_L_DARK;
                    strcpy(buff, "(closed)");

                }
                else if (cost == 0)
                {
                    action_color = TERM_WHITE;
                    buff[0] = '\0';
                }
                else
                {
                    action_color = TERM_YELLOW;
                    sprintf(buff, "(%dgp)", cost);
                }
            }
            else
            {
                if (!owner)
                {
                    action_color = TERM_L_DARK;
                    strcpy(buff, "(closed)");

                }
                else if (cost == 0)
                {
                    action_color = TERM_WHITE;
                    buff[0] = '\0';
                }
                else
                {
                    action_color = TERM_YELLOW;
                    sprintf(buff, "(%dgp)", cost);
                }
            }

            if (cost > p_ptr->au)
                action_color = TERM_L_DARK;

            sprintf(tmp_str," %c) %s %s", bldg->letters[i], bldg->act_names[i], buff);
            c_put_str(action_color, tmp_str, 19+(i/2), 35*(i%2));
        }
    }

    prt(" ESC) Exit building", 23, 0);
    paivita = FALSE;
}

/*
 * arena commands
 */
static void arena_comm(int cmd)
{
    monster_race    *r_ptr;
    cptr            name;


    switch (cmd)
    {
        case BACT_ARENA:
            if (p_ptr->arena_number == MAX_ARENA_MONS)
            {
                clear_bldg(5, 19);
                prt("               Arena Victor!", 5, 0);
                prt("Congratulations!  You have defeated all before you.", 7, 0);
                prt("For that, receive the prize: 1,000,000 gold pieces", 8, 0);

                prt("", 10, 0);
                prt("", 11, 0);
                p_ptr->au += 1000000;
                stats_on_gold_winnings(1000000);
                msg_prompt("Press the space bar to continue", " ", PROMPT_NEW_LINE | PROMPT_FORCE_CHOICE);
                p_ptr->arena_number++;
            }
            else if (p_ptr->arena_number > MAX_ARENA_MONS)
            {
                if (p_ptr->arena_number < MAX_ARENA_MONS+2)
                {
                    msg_print("The strongest challenger is waiting for you.");
                    if (get_check("Do you fight? "))
                    {
                        p_ptr->exit_bldg = FALSE;
                        reset_tim_flags();

                        /* Save the surface floor as saved floor */
                        prepare_change_floor_mode(CFM_SAVE_FLOORS);

                        p_ptr->inside_arena = TRUE;
                        p_ptr->leaving = TRUE;
                        leave_bldg = TRUE;
                    }
                    else
                    {
                        msg_print("We are disappointed.");
                    }
                }
                else
                {
                    msg_print("You enter the arena briefly and bask in your glory.");
                }
            }
            else if (p_ptr->riding && p_ptr->pclass != CLASS_BEASTMASTER && p_ptr->pclass != CLASS_CAVALRY && p_ptr->prace != RACE_MON_RING)
            {
                msg_print("You don't have permission to enter with pet.");
            }
            else
            {
                p_ptr->exit_bldg = FALSE;
                reset_tim_flags();

                /* Save the surface floor as saved floor */
                prepare_change_floor_mode(CFM_SAVE_FLOORS);

                p_ptr->inside_arena = TRUE;
                p_ptr->leaving = TRUE;
                leave_bldg = TRUE;
            }
            break;
        case BACT_POSTER:
            if (p_ptr->arena_number == MAX_ARENA_MONS)
                msg_print("You are victorious. Enter the arena for the ceremony.");

            else if (p_ptr->arena_number > MAX_ARENA_MONS)
            {
                msg_print("You have won against all foes.");
            }
            else
            {
                r_ptr = &r_info[arena_info[p_ptr->arena_number].r_idx];
                name = (r_name + r_ptr->name);
                msg_format("Do I hear any challenges against: %s", name);
            }
            break;
        case BACT_ARENA_RULES:

            /* Save screen */
            screen_save();

            /* Peruse the arena help file */
            (void)show_file(TRUE, "arena.txt", NULL, 0, 0);


            /* Load screen */
            screen_load();

            break;
    }
}


/*
 * display fruit for dice slots
 */
static void display_fruit(int row, int col, int fruit)
{
    switch (fruit)
    {
        case 0: /* lemon */
            c_put_str(TERM_YELLOW, "   ####.", row, col);
            c_put_str(TERM_YELLOW, "  #    #", row + 1, col);
            c_put_str(TERM_YELLOW, " #     #", row + 2, col);
            c_put_str(TERM_YELLOW, "#      #", row + 3, col);
            c_put_str(TERM_YELLOW, "#      #", row + 4, col);
            c_put_str(TERM_YELLOW, "#     # ", row + 5, col);
            c_put_str(TERM_YELLOW, "#    #  ", row + 6, col);
            c_put_str(TERM_YELLOW, ".####   ", row + 7, col);
            prt(                   " Lemon  ", row + 8, col);

            break;
        case 1: /* orange */
            c_put_str(TERM_ORANGE, "   ##   ", row, col);
            c_put_str(TERM_ORANGE, "  #..#  ", row + 1, col);
            c_put_str(TERM_ORANGE, " #....# ", row + 2, col);
            c_put_str(TERM_ORANGE, "#......#", row + 3, col);
            c_put_str(TERM_ORANGE, "#......#", row + 4, col);
            c_put_str(TERM_ORANGE, " #....# ", row + 5, col);
            c_put_str(TERM_ORANGE, "  #..#  ", row + 6, col);
            c_put_str(TERM_ORANGE, "   ##   ", row + 7, col);
            prt(                   " Orange ", row + 8, col);

            break;
        case 2: /* sword */
            c_put_str(TERM_SLATE, "   /\\   " , row, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 1, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 2, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 3, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 4, col);
            c_put_str(TERM_SLATE, "   ##   " , row + 5, col);
            c_put_str(TERM_UMBER, " ###### " , row + 6, col);
            c_put_str(TERM_UMBER, "   ##   " , row + 7, col);
            prt(                  " Sword  " , row + 8, col);

            break;
        case 3: /* shield */
            c_put_str(TERM_SLATE, " ###### ", row, col);
            c_put_str(TERM_SLATE, "#      #", row + 1, col);
            c_put_str(TERM_SLATE, "# ++++ #", row + 2, col);
            c_put_str(TERM_SLATE, "# +==+ #", row + 3, col);
            c_put_str(TERM_SLATE, "#  ++  #", row + 4, col);
            c_put_str(TERM_SLATE, " #    # ", row + 5, col);
            c_put_str(TERM_SLATE, "  #  #  ", row + 6, col);
            c_put_str(TERM_SLATE, "   ##   ", row + 7, col);
            prt(                  " Shield ", row + 8, col);

            break;
        case 4: /* plum */
            c_put_str(TERM_VIOLET, "   ##   ", row, col);
            c_put_str(TERM_VIOLET, " ###### ", row + 1, col);
            c_put_str(TERM_VIOLET, "########", row + 2, col);
            c_put_str(TERM_VIOLET, "########", row + 3, col);
            c_put_str(TERM_VIOLET, "########", row + 4, col);
            c_put_str(TERM_VIOLET, " ###### ", row + 5, col);
            c_put_str(TERM_VIOLET, "  ####  ", row + 6, col);
            c_put_str(TERM_VIOLET, "   ##   ", row + 7, col);
            prt(                   "  Plum  ", row + 8, col);

            break;
        case 5: /* cherry */
            c_put_str(TERM_RED, "      ##", row, col);
            c_put_str(TERM_RED, "   ###  ", row + 1, col);
            c_put_str(TERM_RED, "  #..#  ", row + 2, col);
            c_put_str(TERM_RED, "  #..#  ", row + 3, col);
            c_put_str(TERM_RED, " ###### ", row + 4, col);
            c_put_str(TERM_RED, "#..##..#", row + 5, col);
            c_put_str(TERM_RED, "#..##..#", row + 6, col);
            c_put_str(TERM_RED, " ##  ## ", row + 7, col);
            prt(                " Cherry ", row + 8, col);

            break;
    }
}

/*
 * kpoker no (tyuto-hannpa na)pakuri desu...
 * joker ha shineru node haitte masen.
 *
 * TODO: donataka! tsukutte!
 *  - agatta yaku no kiroku (like DQ).
 *  - kakkoii card no e.
 *  - sousa-sei no koujyo.
 *  - code wo wakariyasuku.
 *  - double up.
 *  - Joker... -- done.
 *
 * 9/13/2000 --Koka
 * 9/15/2000 joker wo jissou. soreto, code wo sukosi kakikae. --Habu
 */
#define SUIT_OF(card)  ((card) / 13)
#define NUM_OF(card)   ((card) % 13)
#define IS_JOKER(card) ((card) == 52)

static int cards[5]; /* tefuda no card */

static void reset_deck(int deck[])
{
    int i;
    for (i = 0; i < 53; i++) deck[i] = i;

    /* shuffle cards */
    for (i = 0; i < 53; i++){
        int tmp1 = randint0(53 - i) + i;
        int tmp2 = deck[i];
        deck[i] = deck[tmp1];
        deck[tmp1] = tmp2;
    }
}

static bool have_joker(void)
{
    int i;

    for (i = 0; i < 5; i++){
      if(IS_JOKER(cards[i])) return TRUE;
    }
    return FALSE;
}

static bool find_card_num(int num)
{
    int i;
    for (i = 0; i < 5; i++)
        if (NUM_OF(cards[i]) == num && !IS_JOKER(cards[i])) return TRUE;
    return FALSE;
}

static bool yaku_check_flush(void)
{
    int i, suit;
    bool joker_is_used = FALSE;

    suit = IS_JOKER(cards[0]) ? SUIT_OF(cards[1]) : SUIT_OF(cards[0]);
    for (i = 0; i < 5; i++){
        if (SUIT_OF(cards[i]) != suit){
          if(have_joker() && !joker_is_used)
            joker_is_used = TRUE;
          else
            return FALSE;
        }
    }

    return TRUE;
}

static int yaku_check_straight(void)
{
    int i, lowest = 99;
    bool joker_is_used = FALSE;

    /* get lowest */
    for (i = 0; i < 5; i++)
    {
        if (NUM_OF(cards[i]) < lowest && !IS_JOKER(cards[i]))
            lowest = NUM_OF(cards[i]);
    }
    
    if (yaku_check_flush())
    {
      if( lowest == 0 ){
        for (i = 0; i < 4; i++)
        {
            if (!find_card_num(9 + i)){
                if( have_joker() && !joker_is_used )
                  joker_is_used = TRUE;
                else
                  break;
            }
        }
        if (i == 4) return 3; /* Wow! Royal Flush!!! */
      }
      if( lowest == 9 ){
        for (i = 0; i < 3; i++)
        {
            if (!find_card_num(10 + i))
                break;
        }
        if (i == 3 && have_joker()) return 3; /* Wow! Royal Flush!!! */
      }
    }

    joker_is_used = FALSE;
    for (i = 0; i < 5; i++)
    {
        if (!find_card_num(lowest + i)){
          if( have_joker() && !joker_is_used )
            joker_is_used = TRUE;
          else
            return 0;
        }
    }
    
    if (yaku_check_flush())
        return 2; /* Straight Flush */

    return 1;
}

/*
 * 0:nopair 1:1 pair 2:2 pair 3:3 cards 4:full house 6:4cards
 */
static int yaku_check_pair(void)
{
    int i, i2, matching = 0;

    for (i = 0; i < 5; i++)
    {
        for (i2 = i+1; i2 < 5; i2++)
        {
            if (IS_JOKER(cards[i]) || IS_JOKER(cards[i2])) continue;
            if (NUM_OF(cards[i]) == NUM_OF(cards[i2]))
                matching++;
        }
    }

    if(have_joker()){
      switch(matching){
      case 0:
        matching = 1;
        break;
      case 1:
        matching = 3;
        break;
      case 2:
        matching = 4;
        break;
      case 3:
        matching = 6;
        break;
      case 6:
        matching = 7;
        break;
      default:
        /* don't reach */
        break;
      }
    }

    return matching;
}

#define ODDS_5A 3000
#define ODDS_5C 400
#define ODDS_RF 200
#define ODDS_SF 80
#define ODDS_4C 16
#define ODDS_FH 12
#define ODDS_FL 8
#define ODDS_ST 4
#define ODDS_3C 1
#define ODDS_2P 1

static int yaku_check(void)
{
    prt("                            ", 4, 3);

    switch(yaku_check_straight()){
    case 3: /* RF! */
        c_put_str(TERM_YELLOW, "Royal Flush",  4,  3);
        return ODDS_RF;
    case 2: /* SF! */
        c_put_str(TERM_YELLOW, "Straight Flush",  4,  3);
        return ODDS_SF;
    case 1:
        c_put_str(TERM_YELLOW, "Straight",  4,  3);
        return ODDS_ST;
    default:
        /* Not straight -- fall through */
        break;
    }

    if (yaku_check_flush())
    {

    c_put_str(TERM_YELLOW, "Flush",  4,  3);
        return ODDS_FL;
    }

    switch (yaku_check_pair())
    {
    case 1:
        c_put_str(TERM_YELLOW, "One pair",  4,  3);
        return 0;
    case 2:
        c_put_str(TERM_YELLOW, "Two pair",  4,  3);
        return ODDS_2P;
    case 3:
        c_put_str(TERM_YELLOW, "Three of a kind",  4,  3);
        return ODDS_3C;
    case 4:
        c_put_str(TERM_YELLOW, "Full house",  4,  3);
        return ODDS_FH;
    case 6:
        c_put_str(TERM_YELLOW, "Four of a kind",  4,  3);
        return ODDS_4C;
    case 7:
        if (!NUM_OF(cards[0]) || !NUM_OF(cards[1]))
        {
            c_put_str(TERM_YELLOW, "Five ace",  4,  3);
            return ODDS_5A;
        }
        else
        {
            c_put_str(TERM_YELLOW, "Five of a kind",  4,  3);
            return ODDS_5C;
        }
    default:
        break;
    }
    return 0;
}

static void display_kaeruka(int hoge, int kaeruka[])
{
    int i;
    char col = TERM_WHITE;
    for (i = 0; i < 5; i++)
    {
        if (i == hoge) col = TERM_YELLOW;
        else if(kaeruka[i]) col = TERM_WHITE;
        else col = TERM_L_BLUE;
        if(kaeruka[i])
            c_put_str(col, "Change", 14,  5+i*16);
        else
            c_put_str(col, " Stay ", 14,  5+i*16);
    }
    if (hoge > 4) col = TERM_YELLOW;
    else col = TERM_WHITE;
    c_put_str(col, "Sure", 16,  38);

    /* Hilite current option */
    if (hoge < 5) move_cursor(14, 5+hoge*16);
    else move_cursor(16, 38);
}


static void display_cards(void)
{
    int i, j;
    char suitcolor[4] = {TERM_YELLOW, TERM_L_RED, TERM_L_BLUE, TERM_L_GREEN};

    cptr suit[4] = {"[]", "qp", "<>", "db"};
    cptr card_grph[13][7] = {{"A    %s     ",
                  "     He     ",
                  "     ng     ",
                  "     ba     ",
                  "     nd     ",
                  "     %s     ",
                  "           A"},
                 {"2           ",
                  "     %s     ",
                  "            ",
                  "            ",
                  "            ",
                  "     %s     ",
                  "           2"},
                 {"3           ",
                  "     %s     ",
                  "            ",
                  "     %s     ",
                  "            ",
                  "     %s     ",
                  "           3"},
                 {"4           ",
                  "   %s  %s   ",
                  "            ",
                  "            ",
                  "            ",
                  "   %s  %s   ",
                  "           4"},
                 {"5           ",
                  "   %s  %s   ",
                  "            ",
                  "     %s     ",
                  "            ",
                  "   %s  %s   ",
                  "           5"},
                 {"6           ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "           6"},
                 {"7           ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "           7"},
                 {"8           ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "           8"},
                 {"9  %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s  9"},
                 {"10 %s  %s   ",
                  "     %s     ",
                  "   %s  %s   ",
                  "            ",
                  "   %s  %s   ",
                  "     %s     ",
                  "   %s  %s 10"},
                 {"J    /\\     ",
                  "%s   ||     ",
                  "     ||     ",
                  "     ||     ",
                  "     ||     ",
                  "   |=HH=| %s",
                  "     ][    J"},
                 {"Q  ######   ",
                  "%s#      #  ",
                  "  # ++++ #  ",
                  "  # +==+ #  ",
                  "   # ++ #   ",
                  "    #  #  %s",
                  "     ##    Q"},
                 {"K           ",
                  "%s _'~~`_   ",
                  "   jjjjj$&  ",
                  "   q q uu   ",
                  "   c    &   ",
                  "    v__/  %s",
                  "           K"}};
    cptr joker_grph[7] = {    "            ",
                  "     J      ",
                  "     O      ",
                  "     K      ",
                  "     E      ",
                  "     R      ",
                  "            "};

    for (i = 0; i < 5; i++)
    {
        prt(" +------------+ ",  5,  i*16);
    }

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 7; j++)
        {
            prt(" |",  j+6,  i*16);
            if(IS_JOKER(cards[i]))
                c_put_str(TERM_VIOLET, joker_grph[j],  j+6,  2+i*16);
            else
                c_put_str(suitcolor[SUIT_OF(cards[i])], format(card_grph[NUM_OF(cards[i])][j], suit[SUIT_OF(cards[i])], suit[SUIT_OF(cards[i])]),  j+6,  2+i*16);
            prt("| ",  j+6,  i*16+14);
        }
    }
    for (i = 0; i < 5; i++)
    {
        prt(" +------------+ ", 13,  i*16);
    }
}

static int do_poker(void)
{
    int i, k = 2;
    char cmd;
    int deck[53]; /* yamafuda : 0...52 */
    int deck_ptr = 0;
    int kaeruka[5]; /* 0:kaenai 1:kaeru */

    bool done = FALSE;
    bool kettei = TRUE;
    bool kakikae = TRUE;

    reset_deck(deck);

    for (i = 0; i < 5; i++)
    {
        cards[i] = deck[deck_ptr++];
        kaeruka[i] = 0; /* default:nokosu */
    }
    
#if 0
    /* debug:RF */
    cards[0] = 12;
    cards[1] = 0;
    cards[2] = 9;
    cards[3] = 11;
    cards[4] = 10;
#endif
#if 0
    /* debug:SF */
    cards[0] = 3;
    cards[1] = 2;
    cards[2] = 4;
    cards[3] = 6;
    cards[4] = 5;
#endif
#if 0
    /* debug:Four Cards */
    cards[0] = 0;
    cards[1] = 0 + 13 * 1;
    cards[2] = 0 + 13 * 2;
    cards[3] = 0 + 13 * 3;
    cards[4] = 51;
#endif
#if 0
    /* debug:Straight */
    cards[0] = 1;
    cards[1] = 0 + 13;
    cards[2] = 3;
    cards[3] = 2 + 26;
    cards[4] = 4;
#endif
#if 0
    /* debug */
    cards[0] = 52;
    cards[1] = 0;
    cards[2] = 1;
    cards[3] = 2;
    cards[4] = 3;
#endif

    /* suteruno wo kimeru */
    prt("Stay witch? ", 0, 0);

    display_cards();
    yaku_check();

    while (!done)
    {
        if (kakikae) display_kaeruka(k+kettei*5, kaeruka);
        kakikae = FALSE;
        cmd = inkey();
        switch (cmd)
        {
        case '6': case 'l': case 'L': case KTRL('F'):
            if (!kettei) k = (k+1)%5;
            else {k = 0;kettei = FALSE;}
            kakikae = TRUE;
            break;
        case '4': case 'h': case 'H': case KTRL('B'):
            if (!kettei) k = (k+4)%5;
            else {k = 4;kettei = FALSE;}
            kakikae = TRUE;
            break;
        case '2': case 'j': case 'J': case KTRL('N'):
            if (!kettei) {kettei = TRUE;kakikae = TRUE;}
            break;
        case '8': case 'k': case 'K': case KTRL('P'):
            if (kettei) {kettei = FALSE;kakikae = TRUE;}
            break;
        case ' ': case '\r':
            if (kettei) done = TRUE;
            else {kaeruka[k] = !kaeruka[k];kakikae = TRUE;}
            break;
        default:
            break;
        }
    }
    
    prt("",0,0);

    for (i = 0; i < 5; i++)
        if (kaeruka[i] == 1) cards[i] = deck[deck_ptr++]; /* soshite toru */

    display_cards();
    
    return yaku_check();
}
#undef SUIT_OF
#undef NUM_OF
#undef IS_JOKER
/* end of poker codes --Koka */

/*
 * gamble_comm
 */
static bool gamble_comm(int cmd)
{
    int i;
    int roll1, roll2, roll3, choice, odds, win;
    s32b wager;
    s32b maxbet;
    s32b oldgold;

    char out_val[160], tmp_str[80], again;
    cptr p;

    screen_save();

    if (cmd == BACT_GAMBLE_RULES)
    {
        /* Peruse the gambling help file */
        (void)show_file(TRUE, "gambling.txt", NULL, 0, 0);

    }
    else
    {
        /* No money */
        if (p_ptr->au < 1)
        {
            msg_print("Hey! You don't have gold - get out of here!");
            msg_print(NULL);
            screen_load();
            return FALSE;
        }

        clear_bldg(5, 23);

        maxbet = p_ptr->lev * 200;

        /* We can't bet more than we have */
        maxbet = MIN(maxbet, p_ptr->au);

        /* Get the wager */
        strcpy(out_val, "");
        sprintf(tmp_str,"Your wager (1-%d) ? ", maxbet);

        /*
         * Use get_string() because we may need more than
         * the s16b value returned by get_quantity().
         */
        if (get_string(tmp_str, out_val, 32))
        {
            /* Strip spaces */
            for (p = out_val; *p == ' '; p++);

            /* Get the wager */
            wager = atol(p);

            if (wager > p_ptr->au)
            {
                msg_print("Hey! You don't have the gold - get out of here!");

                msg_print(NULL);
                screen_load();
                return (FALSE);
            }
            else if (wager > maxbet)
            {
                msg_format("I'll take %d gold of that. Keep the rest.", maxbet);
                wager = maxbet;
            }
            else if (wager < 1)
            {
                msg_print("Ok, we'll start with 1 gold.");


                wager = 1;
            }
            msg_print(NULL);
            win = FALSE;
            odds = 0;
            oldgold = p_ptr->au;

            sprintf(tmp_str, "Gold before game: %9d", oldgold);
            prt(tmp_str, 20, 2);

            sprintf(tmp_str, "Current Wager:    %9d", wager);
            prt(tmp_str, 21, 2);

            /* Prevent savefile-scumming of the casino */
/*            Rand_quick = TRUE; */
            Rand_value = (u32b)time(NULL);

            do
            {
                p_ptr->au -= wager;
                stats_on_gold_services(wager);

                switch (cmd)
                {
                 case BACT_IN_BETWEEN: /* Game of In-Between */
                    c_put_str(TERM_GREEN, "In Between", 5, 2);

                    odds = 4;
                    win = FALSE;
                    roll1 = randint1(10);
                    roll2 = randint1(10);
                    choice = randint1(10);
                    sprintf(tmp_str, "Black die: %d       Black Die: %d", roll1, roll2);

                    prt(tmp_str, 8, 3);
                    sprintf(tmp_str, "Red die: %d", choice);

                    prt(tmp_str, 11, 14);
                    if (((choice > roll1) && (choice < roll2)) ||
                        ((choice < roll1) && (choice > roll2)))
                        win = TRUE;
                    break;
                case BACT_CRAPS:  /* Game of Craps */
                    c_put_str(TERM_GREEN, "Craps", 5, 2);

                    win = 3;
                    odds = 2;
                    roll1 = randint1(6);
                    roll2 = randint1(6);
                    roll3 = roll1 +  roll2;
                    choice = roll3;
                    sprintf(tmp_str, "First roll: %d %d    Total: %d", roll1,

                         roll2, roll3);
                    prt(tmp_str, 7, 5);
                    if ((roll3 == 7) || (roll3 == 11))
                        win = TRUE;
                    else if ((roll3 == 2) || (roll3 == 3) || (roll3 == 12))
                        win = FALSE;
                    else
                        do
                        {
                            msg_print("Hit any key to roll again");

                            msg_print(NULL);
                            roll1 = randint1(6);
                            roll2 = randint1(6);
                            roll3 = roll1 +  roll2;

                            sprintf(tmp_str, "Roll result: %d %d   Total:     %d",

                                 roll1, roll2, roll3);
                            prt(tmp_str, 8, 5);
                            if (roll3 == choice)
                                win = TRUE;
                            else if (roll3 == 7)
                                win = FALSE;
                        } while ((win != TRUE) && (win != FALSE));
                    break;

                case BACT_SPIN_WHEEL:  /* Spin the Wheel Game */
                    win = FALSE;
                    odds = 9;
                    c_put_str(TERM_GREEN, "Wheel", 5, 2);

                    prt("0  1  2  3  4  5  6  7  8  9", 7, 5);
                    prt("--------------------------------", 8, 3);
                    strcpy(out_val, "");
                    get_string("Pick a number (0-9): ", out_val, 32);

                    for (p = out_val; isspace(*p); p++);
                    choice = atol(p);
                    if (choice < 0)
                    {
                        msg_print("I'll put you down for 0.");

                        choice = 0;
                    }
                    else if (choice > 9)
                    {
                        msg_print("Ok, I'll put you down for 9.");

                        choice = 9;
                    }
                    msg_print(NULL);
                    roll1 = randint0(10);
                    sprintf(tmp_str, "The wheel spins to a stop and the winner is %d",

                        roll1);
                    prt(tmp_str, 13, 3);
                    prt("", 9, 0);
                    prt("*", 9, (3 * roll1 + 5));
                    if (roll1 == choice)
                        win = TRUE;
                    break;

                case BACT_DICE_SLOTS: /* The Dice Slots */
                    c_put_str(TERM_GREEN, "Dice Slots", 5, 2);

                    win = FALSE;
                    roll1 = randint1(21);
                    for (i=6;i>0;i--)
                    {
                        if ((roll1-i) < 1)
                        {
                            roll1 = 7-i;
                            break;
                        }
                        roll1 -= i;
                    }
                    roll2 = randint1(21);
                    for (i=6;i>0;i--)
                    {
                        if ((roll2-i) < 1)
                        {
                            roll2 = 7-i;
                            break;
                        }
                        roll2 -= i;
                    }
                    choice = randint1(21);
                    for (i=6;i>0;i--)
                    {
                        if ((choice-i) < 1)
                        {
                            choice = 7-i;
                            break;
                        }
                        choice -= i;
                    }
                    put_str("/--------------------------\\", 7, 2);
                    prt("\\--------------------------/", 17, 2);
                    display_fruit(8,  3, roll1 - 1);
                    display_fruit(8, 12, roll2 - 1);
                    display_fruit(8, 21, choice - 1);
                    if ((roll1 == roll2) && (roll2 == choice))
                    {
                        win = TRUE;
                        switch(roll1)
                        {
                        case 1:
                            odds = 5;break;
                        case 2:
                            odds = 10;break;
                        case 3:
                            odds = 20;break;
                        case 4:
                            odds = 50;break;
                        case 5:
                            odds = 200;break;
                        case 6:
                            odds = 1000;break;
                        }
                    }
                    else if ((roll1 == 1) && (roll2 == 1))
                    {
                        win = TRUE;
                        odds = 2;
                    }
                    break;
                case BACT_POKER:
                    win = FALSE;
                    odds = do_poker();
                    if (odds) win = TRUE;
                    break;
                }

                if (win)
                {
                    prt("YOU WON", 16, 37);

                    p_ptr->au += odds * wager;
                    stats_on_gold_winnings(odds * wager);
                    sprintf(tmp_str, "Payoff: %d", odds);

                    prt(tmp_str, 17, 37);
                }
                else
                {
                    prt("You Lost", 16, 37);
                    prt("", 17, 37);
                }
                sprintf(tmp_str, "Current Gold:     %9d", p_ptr->au);

                prt(tmp_str, 22, 2);
                prt("Again(Y/N)?", 18, 37);

                move_cursor(18, 52);
                again = inkey();
                prt("", 16, 37);
                prt("", 17, 37);
                prt("", 18, 37);
                if (wager > p_ptr->au)
                {
                    msg_print("Hey! You don't have the gold - get out of here!");
                    msg_print(NULL);

                    /* Get out here */
                    break;
                }
            } while ((again == 'y') || (again == 'Y'));

            /* Switch back to complex RNG */
            Rand_quick = FALSE;

            prt("", 18, 37);
            if (p_ptr->au >= oldgold)
            {
                msg_print("You came out a winner! We'll win next time, I'm sure.");
                virtue_add(VIRTUE_CHANCE, 3);
            }
            else
            {
                msg_print("You lost gold! Haha, better head home.");
                virtue_add(VIRTUE_CHANCE, -3);
            }
        }
        msg_print(NULL);
    }
    screen_load();
    return (TRUE);
}

static bool _has_attack_spells(mon_race_ptr race)
{
    if (!race->spells) return FALSE;
    if (race->spells->groups[MST_BREATH]) return TRUE;
    if (race->spells->groups[MST_BALL]) return TRUE;
    if (race->spells->groups[MST_BOLT]) return TRUE;
    if (race->spells->groups[MST_BEAM]) return TRUE;
    if (race->spells->groups[MST_CURSE]) return TRUE;
    return FALSE;
}
static bool vault_aux_battle(int r_idx)
{
    int i, j;
    int dam = 0;

    monster_race *r_ptr = &r_info[r_idx];

    /* Decline town monsters */
/*    if (!mon_hook_dungeon(r_idx)) return FALSE; */

    /* Decline unique monsters */
/*    if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE); */
/*    if (r_ptr->flags7 & (RF7_NAZGUL)) return (FALSE); */

    if (r_ptr->flags1 & (RF1_NEVER_MOVE)) return (FALSE);
    if (r_ptr->flags2 & (RF2_MULTIPLY)) return (FALSE);
    if (r_ptr->flags2 & (RF2_QUANTUM)) return (FALSE);
    if (r_ptr->flags7 & (RF7_AQUATIC)) return (FALSE);
    if (r_ptr->flags7 & (RF7_CHAMELEON)) return (FALSE);

    for (i = 0; i < MAX_MON_BLOWS; i++)
    {
        if (r_ptr->blows[i].method == RBM_EXPLODE) return (FALSE);
        for (j = 0; j < MAX_MON_BLOW_EFFECTS; j++)
        {
            if (r_ptr->blows[i].effects[j].effect != GF_DRAIN_MANA)
                dam += r_ptr->blows[i].effects[j].dd;
        }
    }
    if (!dam && !_has_attack_spells(r_ptr))
        return FALSE;

    return TRUE;
}

void battle_monsters(void)
{
    int total, i;
    int max_dl = 0;
    int mon_level;
    int power[4];
    bool tekitou;
    bool old_inside_battle = p_ptr->inside_battle;

    for (i = 0; i < max_d_idx; i++)
        if (max_dl < max_dlv[i]) max_dl = max_dlv[i];

    mon_level = randint1(MIN(max_dl, 122))+5;
    if (randint0(100) < 60)
    {
        i = randint1(MIN(max_dl, 122))+5;
        mon_level = MAX(i, mon_level);
    }
    if (randint0(100) < 30)
    {
        i = randint1(MIN(max_dl, 122))+5;
        mon_level = MAX(i, mon_level);
    }

    while (1)
    {
        total = 0;
        tekitou = FALSE;
        
        for(i=0;i<4;i++)
        {
            int r_idx, j;
            int attempt = 0;
            while (1)
            {
                attempt++;
                get_mon_num_prep(vault_aux_battle, NULL);
                p_ptr->inside_battle = TRUE;
                r_idx = get_mon_num(mon_level);
                p_ptr->inside_battle = old_inside_battle;
                if (!r_idx) continue;
                if (attempt > 1000) break;

                if ((r_info[r_idx].flags1 & RF1_UNIQUE) || (r_info[r_idx].flags7 & RF7_UNIQUE2))
                {
                    if ((r_info[r_idx].level + 10) > mon_level) continue;
                }

                for (j = 0; j < i; j++)
                    if(r_idx == battle_mon[j]) break;
                if (j<i) continue;

                break;
            }
            battle_mon[i] = r_idx;
            if (r_info[r_idx].level < 45) tekitou = TRUE;
        }

        /*
        battle_mon[0] = MON_SERPENT;
        battle_mon[1] = MON_TALOS;
        battle_mon[2] = MON_SOFTWARE_BUG;
        battle_mon[3] = MON_SOFTWARE_BUG;
        */

        for (i=0;i<4;i++)
        {
            monster_race *r_ptr = &r_info[battle_mon[i]];
            int num_taisei = count_bits(r_ptr->flagsr & (RFR_IM_ACID | RFR_IM_ELEC | RFR_IM_FIRE | RFR_IM_COLD | RFR_IM_POIS));

            if (r_ptr->flags1 & RF1_FORCE_MAXHP)
                power[i] = r_ptr->hdice * r_ptr->hside * 2;
            else
                power[i] = r_ptr->hdice * (r_ptr->hside + 1);
            power[i] = power[i] * (100 + r_ptr->level) / 100;
            if (r_ptr->speed > 110)
                power[i] = power[i] * (r_ptr->speed * 2 - 110) / 100;
            if (r_ptr->speed < 110)
                power[i] = power[i] * (r_ptr->speed - 20) / 100;
            if (num_taisei > 2)
                power[i] = power[i] * (num_taisei*2+5) / 10;
            else if (mon_race_has_invulnerability(r_ptr))
                power[i] = power[i] * 4 / 3;
            else if (mon_race_has_healing(r_ptr))
                power[i] = power[i] * 4 / 3;
            else if (mon_race_has_drain_mana(r_ptr))
                power[i] = power[i] * 11 / 10;
            if (r_ptr->flags1 & RF1_RAND_25)
                power[i] = power[i] * 9 / 10;
            if (r_ptr->flags1 & RF1_RAND_50)
                power[i] = power[i] * 9 / 10;

            switch (battle_mon[i])
            {
                case MON_GREEN_G:
                case MON_THAT_BAT:
                case MON_GHOST_Q:
                    power[i] /= 4;
                    break;
                case MON_LOST_SOUL:
                case MON_GHOST:
                    power[i] /= 2;
                    break;
                case MON_UND_BEHOLDER:
                case MON_SANTACLAUS:
                case MON_ULT_BEHOLDER:
                case MON_UNGOLIANT:
                case MON_ATLACH_NACHA:
                case MON_Y_GOLONAC:
                    power[i] = power[i] * 3 / 5;
                    break;
                case MON_ROBIN_HOOD:
                case MON_RICH:
                case MON_LICH:
                case MON_COLOSSUS:
                case MON_CRYPT_THING:
                case MON_MASTER_LICH:
                case MON_DREADMASTER:
                case MON_DEMILICH:
                case MON_SHADOWLORD:
                case MON_ARCHLICH:
                case MON_BLEYS:
                case MON_CAINE:
                case MON_JULIAN:
                case MON_VENOM_WYRM:
                case MON_MASTER_MYS:
                case MON_G_MASTER_MYS:
                    power[i] = power[i] * 3 / 4;
                    break;
                case MON_VORPAL_BUNNY:
                case MON_SHAGRAT:
                case MON_GORBAG:
                case MON_LOGRUS_MASTER:
                case MON_JURT:
                case MON_GRAV_HOUND:
                case MON_SHIMMERING_VORTEX:
                case MON_JUBJUB:
                case MON_CLUB_DEMON:
                case MON_LLOIGOR:
                case MON_NIGHTCRAWLER:
                case MON_NIGHTWALKER:
                case MON_RAPHAEL:
                case MON_SHAMBLER:
                case MON_SKY_DRAKE:
                case MON_GERARD:
                case MON_G_CTHULHU:
                case MON_SPECT_WYRM:
                case MON_BAZOOKER:
                case MON_GCWADL:
                case MON_KIRIN:
                case MON_FENGHUANG:
                    power[i] = power[i] * 4 / 3;
                    break;
                case MON_UMBER_HULK:
                case MON_FIRE_VORTEX:
                case MON_WATER_VORTEX:
                case MON_COLD_VORTEX:
                case MON_ENERGY_VORTEX:
                case MON_GACHAPIN:
                case MON_REVENANT:
                case MON_NEXUS_VORTEX:
                case MON_PLASMA_VORTEX:
                case MON_TIME_VORTEX:
                case MON_MANDOR:
                case MON_KAVLAX:
                case MON_RINALDO:
                case MON_STORMBRINGER:
                case MON_TIME_HOUND:
                case MON_PLASMA_HOUND:
                case MON_TINDALOS:
                case MON_CHAOS_VORTEX:
                case MON_AETHER_VORTEX:
                case MON_AETHER_HOUND:
                case MON_CANTORAS:
                case MON_GODZILLA:
                case MON_TARRASQUE:
                case MON_DESTROYER:
                case MON_MORGOTH:
                case MON_SERPENT:
                case MON_OROCHI:
                case MON_D_ELF_SHADE:
                case MON_MANA_HOUND:
                case MON_SHARD_VORTEX:
                case MON_BANORLUPART:
                case MON_BOTEI:
                case MON_JAIAN:
                case MON_BAHAMUT:
                case MON_WAHHA:
                    power[i] = power[i] * 3 / 2;
                    break;
                case MON_ROLENTO:
                case MON_CYBER:
                case MON_CYBER_KING:
                case MON_UNICORN_ORD:
                    power[i] = power[i] * 5 / 3;
                    break;
                case MON_ARCH_VILE:
                case MON_PHANTOM_B:
                case MON_WYRM_POWER:
                    power[i] *= 2;
                    break;
                case MON_NODENS:
                case MON_CULVERIN:
                    power[i] *= 3;
                    break;
                case MON_ECHIZEN:
                    power[i] *= 9;
                    break;
                case MON_HAGURE:
                    power[i] *= 100000;
                    break;
                default:
                    break;
            }
            total += power[i];
        }
        for (i=0;i<4;i++)
        {
            power[i] = total*60/power[i];
            if (tekitou && ((power[i] < 160) || power[i] > 1500)) break;
            if ((power[i] < 160) && randint0(20)) break;
            if (power[i] < 101) power[i] = 100 + randint1(5);
            mon_odds[i] = power[i];
        }
        if (i == 4) break;
    }
}

static bool kakutoujou(void)
{
    s32b maxbet;
    s32b wager;
    char out_val[160], tmp_str[80];
    cptr p;

    if ((game_turn - old_battle) > TURNS_PER_TICK*250)
    {
        battle_monsters();
        old_battle = game_turn;
    }

    screen_save();

    /* No money */
    if (p_ptr->au < 1)
    {
        msg_print("Hey! You don't have gold - get out of here!");

        msg_print(NULL);
        screen_load();
        return FALSE;
    }
    else
    {
        int i;

        clear_bldg(4, 10);

        prt("Monsters                                                       Odds", 4, 4);
        for (i=0;i<4;i++)
        {
            char buf[80];
            monster_race *r_ptr = &r_info[battle_mon[i]];

            sprintf(buf,"%d) %-58s  %4u.%02u", i+1, format("%s%s", r_name + r_ptr->name, (r_ptr->flags1 & RF1_UNIQUE) ? " (clone)" : ""), mon_odds[i]/100, mon_odds[i]%100);
            prt(buf, 5+i, 1);
        }

        prt("Which monster: ", 0, 0);
        while(1)
        {
            i = inkey();

            if (i == ESCAPE)
            {
                screen_load();
                return FALSE;
            }
            if (i >= '1' && i <= '4')
            {
                sel_monster = i-'1';
                battle_odds = mon_odds[sel_monster];
                break;
            }
            else bell();
        }

        clear_bldg(4,4);
        for (i=0;i<4;i++)
            if (i !=sel_monster) clear_bldg(i+5,i+5);

        maxbet = p_ptr->lev * 200;

        /* We can't bet more than we have */
        maxbet = MIN(maxbet, p_ptr->au);

        /* Get the wager */
        strcpy(out_val, "");
        sprintf(tmp_str,"Your wager (1-%d) ? ", maxbet);

        /*
         * Use get_string() because we may need more than
         * the s16b value returned by get_quantity().
         */
        if (get_string(tmp_str, out_val, 32))
        {
            /* Strip spaces */
            for (p = out_val; *p == ' '; p++);

            /* Get the wager */
            wager = atol(p);

            if (wager > p_ptr->au)
            {
                msg_print("Hey! You don't have the gold - get out of here!");

                msg_print(NULL);
                screen_load();
                return (FALSE);
            }
            else if (wager > maxbet)
            {
                msg_format("I'll take %d gold of that. Keep the rest.", maxbet);
                wager = maxbet;
            }
            else if (wager < 1)
            {
                msg_print("Ok, we'll start with 1 gold.");


                wager = 1;
            }
            msg_print(NULL);
            battle_odds = MAX(wager+1, wager * battle_odds / 100);
            kakekin = wager;
            p_ptr->au -= wager;
            stats_on_gold_services(wager);

            reset_tim_flags();

            /* Save the surface floor as saved floor */
            prepare_change_floor_mode(CFM_SAVE_FLOORS);

            p_ptr->inside_battle = TRUE;
            p_ptr->leaving = TRUE;

            leave_bldg = TRUE;
            screen_load();

            return (TRUE);
        }
    }
    screen_load();

    return (FALSE);
}

static void today_target(void)
{
    char buf[160];
    monster_race *r_ptr = &r_info[today_mon];

    clear_bldg(4,18);
    prt("Wanted monster that changes from day to day", 5, 10);
    sprintf(buf,"target: %s",r_name + r_ptr->name);
    c_put_str(TERM_YELLOW, buf, 6, 10);
    sprintf(buf,"corpse   ---- $%d",r_ptr->level * 50 + 100);
    prt(buf, 8, 10);
    sprintf(buf,"skeleton ---- $%d",r_ptr->level * 30 + 60);
    prt(buf, 9, 10);
    p_ptr->today_mon = today_mon;
}

static void tsuchinoko(void)
{
    clear_bldg(4,18);
c_put_str(TERM_YELLOW, "Big chance to quick money!!!", 5, 10);
c_put_str(TERM_YELLOW, "target: the rarest animal 'Tsuchinoko'", 6, 10);
c_put_str(TERM_WHITE, "catch alive ---- $1,000,000", 8, 10);
c_put_str(TERM_WHITE, "corpse      ----   $200,000", 9, 10);
c_put_str(TERM_WHITE, "bones       ----   $100,000", 10, 10);
}

static void shoukinkubi(void)
{
    int i;
    int y = 0;

    clear_bldg(4,18);

    prt("Offer a prize when you bring a wanted monster's corpse",4 ,10);
    c_put_str(TERM_YELLOW, "Wanted monsters", 6, 10);

    for (i = 0; i < MAX_KUBI; i++)
    {
        byte color;
        cptr done_mark;
        int  id = kubi_r_idx[i];
        mon_race_ptr race;

        if (!id) continue; /* SUPPRESSED by reduce_uniques options */

        if (id > 10000)
        {
            color = TERM_RED;
            done_mark = "(done)";
            id -= 10000;
        }
        else
        {
            color = TERM_WHITE;
            done_mark = "";
        }
        assert(0 < id && id < max_r_idx);
        race = &r_info[id];

        c_prt(color, format("%s %s", r_name + race->name, done_mark), y+7, 10);

        y = (y+1) % 10;
        if (!y && (i < MAX_KUBI -1))
        {
            prt("Hit any key.", 0, 0);
            (void)inkey();
            prt("", 0, 0);
            clear_bldg(7,18);
        }
    }
}


/* List of prize object */
static struct {
    s16b tval;
    s16b sval;
} prize_list[MAX_KUBI] = 
{
    {TV_POTION, SV_POTION_SPEED},
    {TV_POTION, SV_POTION_SPEED},
    {TV_POTION, SV_POTION_SPEED},
    {TV_POTION, SV_POTION_RESISTANCE},
    {TV_POTION, SV_POTION_ENLIGHTENMENT},

    {TV_POTION, SV_POTION_HEALING},
    {TV_POTION, SV_POTION_RESTORE_MANA},
    {TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION},
    {TV_POTION, SV_POTION_STAR_ENLIGHTENMENT},
    {TV_SCROLL, SV_SCROLL_CRAFTING},

    {TV_SCROLL, SV_SCROLL_GENOCIDE},
    {TV_POTION, SV_POTION_STAR_HEALING},
    {TV_POTION, SV_POTION_STAR_HEALING},
    {TV_POTION, SV_POTION_NEW_LIFE},
    {TV_SCROLL, SV_SCROLL_MASS_GENOCIDE},

    {TV_POTION, SV_POTION_LIFE},
    {TV_POTION, SV_POTION_LIFE},
    {TV_POTION, SV_POTION_AUGMENTATION},
    {TV_POTION, SV_POTION_INVULNERABILITY},
    {TV_SCROLL, SV_SCROLL_ARTIFACT},
};


/* Get prize */
static int _prize_count = 0;
static bool _is_captured_tsuchinoko(obj_ptr obj)
{
    if (obj->tval == TV_CAPTURE && obj->pval == MON_TSUCHINOKO)
        return TRUE;
    return FALSE;
}
static bool _is_corpse_tsuchinoko(obj_ptr obj)
{
    if (obj->tval == TV_CORPSE && obj->pval == MON_TSUCHINOKO)
        return TRUE;
    return FALSE;
}
static int _tsuchinoko_amt(obj_ptr obj)
{
    if (obj->tval == TV_CAPTURE)
        return 1000000 * obj->number;
    if (obj->tval == TV_CORPSE && obj->sval == SV_CORPSE)
        return 200000 * obj->number;
    if (obj->tval == TV_CORPSE && obj->sval == SV_SKELETON)
        return 100000 * obj->number;
    return 0;
}
static void _obj_reward(obj_ptr obj, int amt)
{
    msg_format("You get %dgp.", amt);
    p_ptr->au += amt;
    stats_on_gold_winnings(amt);
    obj->number = 0;
    obj_release(obj, 0);
    p_ptr->redraw |= PR_GOLD;
    p_ptr->notice |= PN_OPTIMIZE_PACK;
}
static void _process_tsuchinoko(obj_ptr obj)
{
    char name[MAX_NLEN];
    char buf[MAX_NLEN+30];
    object_desc(name, obj, OD_COLOR_CODED);
    sprintf(buf, "Convert %s into money? ", name);
    if (get_check(buf))
        _obj_reward(obj, _tsuchinoko_amt(obj));
    ++_prize_count;
}
static bool _is_todays_prize(obj_ptr obj)
{
    if (obj->tval == TV_CORPSE)
    {
        if (streq(r_name + r_info[obj->pval].name,
                  r_name + r_info[today_mon].name) )
            return TRUE;
    }
    return FALSE;
}
static void _process_todays_prize(obj_ptr obj)
{
    char name[MAX_NLEN];
    char buf[MAX_NLEN+30];
    object_desc(name, obj, OD_COLOR_CODED);
    sprintf(buf, "Convert %s into money? ", name);
    if (get_check(buf))
    {
        int mult = obj->sval == SV_CORPSE ? 50 : 30;
        int amt = ((r_info[today_mon].level + 2) * mult) * obj->number;
        _obj_reward(obj, amt);
    }
    ++_prize_count;
}
static int _wanted_monster_idx(int r_idx)
{
    int i;
    for (i = 0; i < MAX_KUBI; i++)
    {
        if (kubi_r_idx[i] == r_idx)
            return i;
    }
    return -1;
}
static bool _is_wanted_monster(int r_idx)
{
    int idx = _wanted_monster_idx(r_idx);
    if (idx >= 0) return TRUE;
    return FALSE;
}
static void _forge_wanted_monster_prize(obj_ptr obj, int r_idx)
{
    int idx = _wanted_monster_idx(r_idx);
    int tval = prize_list[idx].tval;
    int sval = prize_list[idx].sval;

    object_prep(obj, lookup_kind(tval, sval));
    apply_magic(obj, object_level, AM_NO_FIXED_ART);
    object_origins(obj, ORIGIN_WANTED);
    obj->origin_xtra = r_idx;
    obj_make_pile(obj);
    obj_identify_fully(obj);
}
static bool _is_wanted_corpse(obj_ptr obj)
{
    if (obj->tval == TV_CORPSE && _is_wanted_monster(obj->pval))
        return TRUE;
    return FALSE;
}
static bool _is_wanted_captureball(obj_ptr obj)
{
    if (obj->tval == TV_CAPTURE && _is_wanted_monster(obj->pval))
        return TRUE;
    return FALSE;
}
static void _process_wanted_corpse(obj_ptr obj)
{
    char  name[MAX_NLEN];
    char  buf[MAX_NLEN+30];
    obj_t prize;
    int   r_idx = obj->pval;
    int   num, k, kubi_idx = _wanted_monster_idx(r_idx);

    ++_prize_count;

    object_desc(name, obj, OD_COLOR_CODED);
    sprintf(buf, "Hand %s over? ", name);
    if (!get_check(buf)) return;

    _forge_wanted_monster_prize(&prize, r_idx);
    if (obj->tval == TV_CAPTURE)
    {
        r_info[r_idx].max_num = 0;
        r_info[r_idx].ball_num = 0;
        empty_capture_ball(obj);
    }
    else
    {
        obj->number = 0;
    }
    obj_release(obj, 0);

    virtue_add(VIRTUE_JUSTICE, 5);
    p_ptr->fame++;
    kubi_r_idx[kubi_idx] += 10000;

    /* Count number of unique corpses already handed */
    for (num = 0, k = 0; k < MAX_KUBI; k++)
    {
        if (kubi_r_idx[k] >= 10000) num++;
    }

    msg_format("You earned %d point%s total.", num, num > 1 ? "s" : "");

    object_desc(name, &prize, OD_COLOR_CODED);
    /*msg_format("You get %s.", name);*/
    pack_carry(&prize);
}
static bool kankin(void)
{
    _prize_count = 0;
    equip_for_each_that(_process_tsuchinoko, _is_captured_tsuchinoko);
    pack_for_each_that(_process_tsuchinoko, _is_captured_tsuchinoko);
    pack_for_each_that(_process_tsuchinoko, _is_corpse_tsuchinoko);
    pack_for_each_that(_process_todays_prize, _is_todays_prize);
    pack_for_each_that(_process_wanted_corpse, _is_wanted_corpse);
    equip_for_each_that(_process_wanted_corpse, _is_wanted_captureball);
    pack_for_each_that(_process_wanted_corpse, _is_wanted_captureball);
    if (!_prize_count)
    {
        msg_print("You have nothing that I want.");
        return FALSE;
    }
    return TRUE;
}

bool get_nightmare(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    /* Require eldritch horrors */
    if (!(r_ptr->flags2 & (RF2_ELDRITCH_HORROR))) return (FALSE);

    /* Require high level */
    if (r_ptr->level <= p_ptr->lev) return (FALSE);

    /* Accept this monster */
    return (TRUE);
}


void have_nightmare(int r_idx)
{
    bool happened = FALSE;
    monster_race *r_ptr = &r_info[r_idx];
    int power = r_ptr->level + 10;
    char m_name[80];
    cptr desc = r_name + r_ptr->name;

    /* Describe it */
    if (!(r_ptr->flags1 & RF1_UNIQUE))
        sprintf(m_name, "%s %s", (is_a_vowel(desc[0]) ? "an" : "a"), desc);
    else
        sprintf(m_name, "%s", desc);

    if (!(r_ptr->flags1 & RF1_UNIQUE))
    {
        if (r_ptr->flags1 & RF1_FRIENDS) power /= 2;
    }
    else power *= 2;

    if (saving_throw(p_ptr->skills.sav * 100 / power))
    {
        msg_format("%^s chases you through your dreams.", m_name);

        /* Safe */
        return;
    }

    if (p_ptr->image)
    {
        /* Something silly happens... */
        msg_format("You behold the %s visage of %s!",

                      funny_desc[randint0(MAX_SAN_FUNNY)], m_name);

        if (one_in_(3))
        {
            msg_print(funny_comments[randint0(MAX_SAN_COMMENT)]);
            p_ptr->image = p_ptr->image + randint1(r_ptr->level);
        }

        /* Never mind; we can't see it clearly enough */
        return;
    }

    /* Something frightening happens... */
    msg_format("You behold the %s visage of %s!",

                  horror_desc[randint0(MAX_SAN_HORROR)], desc);

    mon_lore_aux_2(r_ptr, RF2_ELDRITCH_HORROR);

    if (p_ptr->mimic_form != MIMIC_NONE)
    {
        switch (p_ptr->prace)
        {
        /* Demons may make a saving throw */
        case RACE_IMP:
        case RACE_BALROG:
            if (saving_throw(20 + p_ptr->lev)) return;
            break;
        /* Undead may make a saving throw */
        case RACE_SKELETON:
        case RACE_ZOMBIE:
        case RACE_SPECTRE:
        case RACE_VAMPIRE:
            if (saving_throw(10 + p_ptr->lev)) return;
            break;
        }
    }
    else
    {
        race_t *race_ptr = get_race();
        /* Demons may make a saving throw */
        if (race_ptr->flags & RACE_IS_DEMON)
        {
            if (saving_throw(20 + p_ptr->lev)) return;
        }
        /* Undead may make a saving throw */
        else if (race_ptr->flags & RACE_IS_UNDEAD)
        {
            if (saving_throw(10 + p_ptr->lev)) return;
        }
    }

    if (p_ptr->no_eldritch) return;

    /* Mind blast */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        if (!res_save_default(RES_CONF))
        {
            (void)set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
        }
        if (!res_save_default(RES_CHAOS) && one_in_(3))
        {
            (void)set_image(p_ptr->image + randint0(250) + 150, FALSE);
        }
        return;
    }

    /* Lose int & wis */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        do_dec_stat(A_INT);
        do_dec_stat(A_WIS);
        return;
    }

    /* Brain smash */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        if (!res_save_default(RES_CONF))
        {
            (void)set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
        }
        if (!free_act_save_p(power))
        {
            (void)set_paralyzed(randint1(4), FALSE);
        }
        while ((!saving_throw(p_ptr->skills.sav)) && (p_ptr->stat_cur[A_INT] > 3))
        {
            (void)do_dec_stat(A_INT);
            if (p_ptr->sustain_int) break;
        }
        while ((!saving_throw(p_ptr->skills.sav)) && (p_ptr->stat_cur[A_WIS] > 3))
        {
            (void)do_dec_stat(A_WIS);
            if (p_ptr->sustain_wis) break;
        }
        if (!res_save_default(RES_CHAOS))
        {
            (void)set_image(p_ptr->image + randint0(250) + 150, FALSE);
        }
        return;
    }


    /* Amnesia */
    if (!saving_throw(p_ptr->skills.sav * 100 / power))
    {
        if (lose_all_info())
        {
            msg_print("You forget everything in your utmost terror!");

        }
        return;
    }

    /* Else gain permanent insanity */
    if (mut_present(MUT_MORONIC) && mut_present(MUT_BERS_RAGE) &&
        (mut_present(MUT_COWARDICE) || res_save_default(RES_FEAR)) &&
        (mut_present(MUT_HALLUCINATION) || res_save_default(RES_CHAOS)))
    {
        /* The poor bastard already has all possible insanities! */
        return;
    }

    while (!happened)
    {
        switch (randint1(4))
        {
            case 1:
            {
                if (!mut_present(MUT_MORONIC))
                {
                    mut_gain(MUT_MORONIC);
                    happened = TRUE;
                }
                break;
            }
            case 2:
            {
                if (!mut_present(MUT_COWARDICE) && !res_save_default(RES_FEAR))
                {
                    mut_gain(MUT_COWARDICE);
                    happened = TRUE;
                }
                break;
            }
            case 3:
            {
                if (!mut_present(MUT_HALLUCINATION) && !res_save_default(RES_CHAOS))
                {
                    mut_gain(MUT_HALLUCINATION);
                    happened = TRUE;
                }
                break;
            }
            default:
            {
                if (!mut_present(MUT_BERS_RAGE))
                {
                    mut_gain(MUT_BERS_RAGE);
                    happened = TRUE;
                }
                break;
            }
        }
    }

    p_ptr->update |= PU_BONUS;
    handle_stuff();
}

static void _recharge_player_items(void)
{
    slot_t slot;
    if (p_ptr->pclass == CLASS_MAGIC_EATER)
        magic_eater_restore_all();

    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = pack_obj(slot);
        if (obj && object_is_device(obj))
        device_regen_sp_aux(obj, 1000);
    }
    for (slot = 1; slot <= equip_max(); slot++)
    {
        obj_ptr obj = equip_obj(slot);
        if (obj && obj->timeout > 0)
        obj->timeout = 0;
    }
}

/*
 * inn commands
 * Note that resting for the night was a perfect way to avoid player
 * ghosts in the town *if* you could only make it to the inn in time (-:
 * Now that the ghosts are temporarily disabled in 2.8.X, this function
 * will not be that useful. I will keep it in the hopes the player
 * ghost code does become a reality again. Does help to avoid filthy urchins.
 * Resting at night is also a quick way to restock stores -KMW-
 */
static bool inn_comm(int cmd)
{
    switch (cmd)
    {
        case BACT_FOOD: /* Buy food & drink */
            msg_print("The barkeep gives you some gruel and a beer.");

            (void)set_food(PY_FOOD_MAX - 1);
            break;

        case BACT_REST: /* Rest for the night */
            if ((p_ptr->poisoned) || (p_ptr->cut))
            {
                msg_print("You need a healer, not a room.");
                msg_print("Sorry, but I don't want anyone dying in here.");
            }
            else
            {
                s32b oldturn = game_turn;
                int prev_day, prev_hour, prev_min;

                extract_day_hour_min(&prev_day, &prev_hour, &prev_min);
                game_turn = (game_turn / (TURNS_PER_TICK*TOWN_DAWN/2) + 1) * (TURNS_PER_TICK*TOWN_DAWN/2);
                if (dungeon_turn < dungeon_turn_limit)
                {
                    dungeon_turn += MIN(game_turn - oldturn, TURNS_PER_TICK*250);
                    if (dungeon_turn > dungeon_turn_limit) dungeon_turn = dungeon_turn_limit;
                }

                prevent_turn_overflow();

                p_ptr->chp = p_ptr->mhp;
                reset_tim_flags();

                if (ironman_nightmare)
                {
                    msg_print("Horrible visions flit through your mind as you sleep.");
                    get_mon_num_prep(get_nightmare, NULL);
                    while(1)
                    {
                        have_nightmare(get_mon_num(MAX_DEPTH));
                        if (!one_in_(3)) break;
                    }

                    get_mon_num_prep(NULL, NULL);
                    msg_print("You awake screaming.");
                }
                else
                {
                    p_ptr->chp = p_ptr->mhp;
                    if ((p_ptr->pclass != CLASS_RUNE_KNIGHT) && (!elemental_is_(ELEMENTAL_WATER)))
                        p_ptr->csp = p_ptr->msp;

                    _recharge_player_items();

                    if (prev_hour >= 6 && prev_hour <= 17)
                        msg_print("You awake refreshed for the evening.");
                    else
                        msg_print("You awake refreshed for the new day.");
                }
                /* Update daily wanted */
                if (prev_hour >= 18) /* Proxy for date change */
                {
                    determine_today_mon(FALSE);
                    if (p_ptr->prace == RACE_WEREWOLF) werewolf_check_midnight();
                }
            }
            break;

        case BACT_RUMORS: /* Listen for rumors */
            {
                char Rumor[1024];

                if (!get_rnd_line("rumors.txt", 0, Rumor))

                    msg_format("%s", Rumor);
                break;
            }
    }

    return (TRUE);
}

/*
 * Refresh buildings (epic hack)
 */
static void refresh_buildings(void)
{
    room_ptr dummy;
    init_buildings();
    dummy = towns_get_map();
    if (dummy)
    {
        int y, x;
        bool loytyi = FALSE;
        cave_type *ruutu = &cave[py][px];
        s16b etsittava = ruutu->feat;
        for (y = 0; y < dummy->height && !loytyi; y++)
        {
            cptr line = vec_get(dummy->map, y);
            for (x = 0; x < dummy->width && !loytyi; x++)
            {
                char letter = line[x];
                room_grid_ptr my_grid = int_map_find(dummy->letters, letter);
                if (!my_grid) my_grid = int_map_find(room_letters, letter);
                if (!my_grid) continue;
                if (etsittava == conv_dungeon_feat(my_grid->cave_feat))
                {
                    loytyi = TRUE;

                    /* Update the quest indicator for the building we're in */
                    ruutu->special = my_grid->extra;
                }
            }
        }
        room_free(dummy);
    }
    paivita = TRUE;
}

/*
 * Request a quest from the Lord.
 */
static void castle_quest(void)
{
    int             quest_id = 0;
    quest_ptr       quest;


    clear_bldg(4, 18);

    /* Current quest of the building */
    quest_id = cave[py][px].special;

    /* Is there a quest available at the building? */
    if (!quest_id)
    {
        put_str("I don't have a quest for you at the moment.", 8, 0);
        return;
    }

    quest = quests_get(quest_id);

    if (quest->status == QS_COMPLETED)
    {
        if (strpos("Eddies", quest->name)) town_on_visit(TOWN_ZUL);
        quest_reward(quest);
        reinit_wilderness = TRUE;
        refresh_buildings();
    }
    else if (quest->status == QS_FAILED)
    {
        string_ptr s = quest_get_description(quest);
        msg_format("<color:R>%s</color> (<color:U>Level %d</color>): %s",
            quest->name, quest->danger_level, string_buffer(s));
        string_free(s);
        quest->status = QS_FAILED_DONE;
        reinit_wilderness = TRUE;
        refresh_buildings();
    }
    else if (quest->status == QS_TAKEN)
    {
        put_str("You have not completed your current quest yet!", 8, 0);
        put_str("Use CTRL-Q to check the status of your quest.", 9, 0);
        put_str("Return when you have completed your quest.", 12, 0);
    }
    else if (quest->status == QS_UNTAKEN)
    {
        quest_take(quest);
        reinit_wilderness = TRUE;
    }
}


/*
 * Display town history
 */
static void town_history(void)
{
    /* Save screen */
    screen_save();

    /* Peruse the building help file */
    (void)show_file(TRUE, "bldg.txt", NULL, 0, 0);


    /* Load screen */
    screen_load();
}


/*
 * Hook to specify "ammo"
 */
static bool item_tester_hook_ammo(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


/*
 * Evaluate AC
 *
 * Calculate and display the dodge-rate and the protection-rate
 * based on AC
 */
static bool eval_ac(int iAC)
{
    const char memo[] =
        "'Protection Rate' means how much damage is reduced by your armor.\n"
        "Note that the Protection rate is effective only against normal "
        "'attack' and 'shatter' type melee attacks, "
        "and has no effect against any other types such as 'poison'.\n \n"
        "'Dodge Rate' indicates the success rate on dodging the "
        "monster's melee attacks. "
        "It is depend on the level of the monster and your AC.\n \n"
        "'Average Damage' indicates the expected amount of damage "
        "when you are attacked by normal melee attacks with power=60.";

    int protection;
    int col, row = 2;
    int lvl;
    char buf[80*20], *t;

    /* AC lower than zero has no effect */
    if (iAC < 0) iAC = 0;

    protection = 100 - ac_melee_pct(iAC);

    screen_save();
    clear_bldg(0, 22);

    put_str(format("Your current AC : %3d", iAC), row++, 0);
    put_str(format("Protection rate : %3d%%", protection), row++, 0);
    row++;

    put_str("Level of Monster:", row + 0, 0);
    put_str("Dodge Rate      :", row + 1, 0);
    put_str("Average Damage  :", row + 2, 0);
    
    for (col = 17 + 1, lvl = 0; lvl <= 100; lvl += 10, col += 5)
    {
        int quality = 60 + lvl * 3; /* attack quality with power 60 */
        int dodge;
        int average;

        put_str(format("%3d", lvl), row + 0, col);

        dodge = 5 + (MIN(100, 100 * (iAC * 3 / 4) / quality) * 9 + 5) / 10;
        put_str(format("%3d%%", dodge), row + 1, col);

        average = (100 - dodge) * (100 - protection) / 100;
        put_str(format("%3d", average), row + 2, col);
    }

    /* Display note */
    roff_to_buf(memo, 70, buf, sizeof(buf));
    for (t = buf; t[0]; t += strlen(t) + 1)
        put_str(t, (row++) + 6, 4);

    prt("Defense abilities from your current Armor Class are evaluated below.", 0, 0);
  
    flush();
    (void)inkey();
    screen_load();

    /* Done */
    return (TRUE);
}

typedef struct _gamble_shop_s {
    int tval;
    int sval;
    int prob;
} _gamble_shop_t;

const _gamble_shop_t _gamble_shop_potions[] = {
    { TV_POTION, SV_POTION_SPEED, 50},
    { TV_POTION, SV_POTION_CURING, 38},
    { TV_POTION, SV_POTION_RESISTANCE, 30},
    { TV_POTION, SV_POTION_HEALING, 30},
    { TV_POTION, SV_POTION_STAR_HEALING, 10},
    { TV_POTION, SV_POTION_RESTORE_MANA, 10},
    { TV_POTION, SV_POTION_INC_STR, 3},
    { TV_POTION, SV_POTION_INC_INT, 3},
    { TV_POTION, SV_POTION_INC_WIS, 3},
    { TV_POTION, SV_POTION_INC_DEX, 3},
    { TV_POTION, SV_POTION_INC_CON, 3},
    { TV_POTION, SV_POTION_INC_CHR, 3},
    { TV_POTION, SV_POTION_POLYMORPH, 3},
    { TV_POTION, SV_POTION_NEW_LIFE, 3},
    { TV_POTION, SV_POTION_LIFE, 3},
    { TV_POTION, SV_POTION_STONE_SKIN, 1},
    { TV_POTION, SV_POTION_INVULNERABILITY, 1},
    { TV_POTION, SV_POTION_AUGMENTATION, 1},
    { TV_POTION, SV_POTION_HEROISM, 50},
    { TV_POTION, SV_POTION_BOLDNESS, 50},
    { TV_POTION, SV_POTION_CURE_LIGHT, 50},
    { TV_POTION, SV_POTION_CURE_SERIOUS, 50},
    { TV_POTION, SV_POTION_CURE_CRITICAL, 50},
    { TV_POTION, SV_POTION_RESTORE_EXP, 50},
    { TV_POTION, SV_POTION_RES_ALL, 50},
    { TV_POTION, SV_POTION_RES_STR, 50},
    { TV_POTION, SV_POTION_RES_INT, 50},
    { TV_POTION, SV_POTION_RES_WIS, 50},
    { TV_POTION, SV_POTION_RES_DEX, 50},
    { TV_POTION, SV_POTION_RES_CON, 50},
    { TV_POTION, SV_POTION_RES_CHR, 50},
    { TV_POTION, SV_POTION_CLARITY, 50},
    { TV_POTION, SV_POTION_THERMAL, 50},
    { TV_POTION, SV_POTION_VIGOR, 50},
    { 0, 0, 0}
};

const _gamble_shop_t _gamble_shop_scrolls[] = {
    { TV_SCROLL, SV_SCROLL_TELEPORT_LEVEL, 39},
    { TV_SCROLL, SV_SCROLL_RUNE_OF_PROTECTION, 30},
    { TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION, 30},
    { TV_SCROLL, SV_SCROLL_ACQUIREMENT, 15},
    { TV_SCROLL, SV_SCROLL_MASS_GENOCIDE, 20},
    { TV_SCROLL, SV_SCROLL_GENOCIDE, 20},
    { TV_SCROLL, SV_SCROLL_STAR_ACQUIREMENT, 5},
    { TV_SCROLL, SV_SCROLL_CRAFTING, 10},
    { TV_SCROLL, SV_SCROLL_MADNESS, 10},
    { TV_SCROLL, SV_SCROLL_ARTIFACT, 1},
    { TV_SCROLL, SV_SCROLL_PHASE_DOOR, 50},
    { TV_SCROLL, SV_SCROLL_TELEPORT, 50},
    { TV_SCROLL, SV_SCROLL_REMOVE_CURSE, 50},
    { TV_SCROLL, SV_SCROLL_STAR_ENCHANT_WEAPON, 50},
    { TV_SCROLL, SV_SCROLL_STAR_ENCHANT_ARMOR, 50},
    { TV_SCROLL, SV_SCROLL_DETECT_ITEM, 50},
    { TV_SCROLL, SV_SCROLL_DETECT_GOLD, 50},
    { TV_SCROLL, SV_SCROLL_LIGHT, 50},
    { TV_SCROLL, SV_SCROLL_MAPPING, 50},
    { TV_SCROLL, SV_SCROLL_MONSTER_CONFUSION, 50},
    { TV_SCROLL, SV_SCROLL_RECHARGING, 50},
    { TV_SCROLL, SV_SCROLL_BLESSING, 50},
    { TV_SCROLL, SV_SCROLL_HOLY_PRAYER, 50},
    { TV_SCROLL, SV_SCROLL_PROTECTION_FROM_EVIL, 50},
    { TV_SCROLL, SV_SCROLL_DETECT_INVIS, 50},
    { TV_SCROLL, SV_SCROLL_DISPEL_UNDEAD, 50},
    { 0, 0, 0}
};

static int _gamble_shop_roll(const _gamble_shop_t *choices)
{
    int tot = 0, roll;
    int i = 0;

    for (i = 0; ; i++)
    {
        const _gamble_shop_t *entry = choices + i;
        if (!entry->prob)
            break;
        tot += entry->prob;
    }

    roll = randint1(tot);

    for (i = 0; ; i++)
    {
        const _gamble_shop_t *entry = choices + i;
        if (!entry->prob)
            break;

        roll -= entry->prob;
        if (roll <= 0)
            return i;
    }

    return -1;
}

static bool _gamble_shop_aux(object_type *o_ptr)
{
    char buf[MAX_NLEN];
    int auto_pick_idx;

    obj_identify_fully(o_ptr);
    stats_on_identify(o_ptr);
    object_desc(buf, o_ptr, OD_COLOR_CODED);
    msg_format("You win %s.", buf);

    auto_pick_idx = is_autopick(o_ptr);
    if (auto_pick_idx >= 0)
    {
        if (autopick_list[auto_pick_idx].action & DO_AUTODESTROY)
        {
            race_t  *race_ptr = get_race();
            class_t *class_ptr = get_class();
            bool     handled = FALSE;
            if (!handled && race_ptr->destroy_object)
                handled = race_ptr->destroy_object(o_ptr);
            if (!handled && class_ptr->destroy_object)
                handled = class_ptr->destroy_object(o_ptr);
            if (!handled)
                msg_format("You destroy %s.", buf);
            return TRUE;
        }
    }

    pack_carry(o_ptr);
    return TRUE;
}

static bool _gamble_shop(const _gamble_shop_t *choices)
{
    object_type forge = {0};
    int k_idx;
    int choice = _gamble_shop_roll(choices);

    if (choice < 0)
    {
        msg_print("Oops! Something went wrong.");
        return FALSE;
    }

    k_idx = lookup_kind(choices[choice].tval, choices[choice].sval);
    object_prep(&forge, k_idx);
    object_origins(&forge, ORIGIN_GAMBLE);

    return _gamble_shop_aux(&forge);
}

static bool _gamble_shop_object(object_p pred)
{
    object_type forge = {0};
    int lvl = 40 + randint0(60);
    int k_idx;
    
    for (;;)
    {
        k_idx = get_obj_num(lvl);
        object_prep(&forge, k_idx);
        if (pred && !pred(&forge))
            continue;
        apply_magic(&forge, lvl, AM_GOOD);
        object_origins(&forge, ORIGIN_GAMBLE);
        switch (forge.tval)
        {
            case TV_SPIKE:
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
            {
                if (!forge.name1 && !forge.name3)
                    forge.number = (byte)damroll(6, 7);
            }
        }
        break;
    }

    return _gamble_shop_aux(&forge);
}

static bool _gamble_shop_device(int tval)
{
    object_type forge = {0};
    int lvl = 20;
    int k_idx;

    while (one_in_(lvl/10))
        lvl += 20;

    for (;;)
    {
        k_idx = lookup_kind(tval, SV_ANY);
        object_prep(&forge, k_idx);
        if (device_init(&forge, lvl, 0))
        {
            object_origins(&forge, ORIGIN_GAMBLE);
            break;
        }
    }

    return _gamble_shop_aux(&forge);
}

static bool _gamble_shop_artifact(void)
{
    object_type forge = {0};
    int lvl = 70 + randint0(30);
    int k_idx;
    
    for (;;)
    {
        k_idx = get_obj_num(lvl);
        object_prep(&forge, k_idx);
        if (!object_is_weapon(&forge) && !object_is_armour(&forge))
            continue;
        apply_magic(&forge, lvl, AM_GOOD | AM_GREAT | AM_SPECIAL);
        if (!forge.art_name)
            continue;
        object_origins(&forge, ORIGIN_GAMBLE);

        break;
    }

    return _gamble_shop_aux(&forge);
}

static obj_ptr _get_reforge_src(int max_power)
{
    char buf[255];
    char buf2[255];
    obj_prompt_t prompt = {0};

    sprintf(buf, "Use what artifact for reforging (Max Power = %d)? ", max_power);
    sprintf(buf2, "You have no artifacts to reforge. (Max Power = %d) ", max_power);
    prompt.prompt = buf;
    prompt.error = buf2;
    prompt.filter = object_is_artifact;
    prompt.where[0] = INV_PACK;
    prompt.flags = INV_SHOW_VALUE;

    obj_prompt(&prompt);
    return prompt.obj;
}

static obj_ptr _get_reforge_dest(int max_power)
{
    char buf[255];
    obj_prompt_t prompt = {0};

    sprintf(buf, "Reforge which object (Max Power = %d)? ", max_power);
    prompt.prompt = buf;
    prompt.error = "You have nothing to reforge.";
    prompt.filter = item_tester_hook_nameless_weapon_armour;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.flags = INV_SHOW_VALUE;

    obj_prompt(&prompt);
    return prompt.obj;
}

static int _calculate_reforge_cost(int value, int slot_weight)
{
    int cost = MAX(10000, MIN(value, 1125L * slot_weight)) * (coffee_break ? 7 : 8);
    if (prace_is_(RACE_MON_PUMPKIN)) cost -= (cost / 5);
    cost -= cost % 1000;
    return cost;
}

static bool _reforge_artifact_exit(void)
{
    if (paivita) (void)inkey();
    return FALSE;
}

int reforge_limit(void)
{
    int f = ((p_ptr->fame <= 128) || (p_ptr->fame >= 224)) ? p_ptr->fame : (((p_ptr->fame - 128) * 3) / 4) + 128;
    /* 90K max kind of removed - stronger objects are allowed but get scaled down */
    if (coffee_break) /* Accelerated reforging */
    {
        int i;
        byte failed_quests = 0;
        vec_ptr v = quests_get_random();
        for (i = 0; i < vec_length(v); i++)
        {
            quest_ptr q = vec_get(v, i);
            if (q->status == QS_FAILED || q->status == QS_FAILED_DONE)
            {
                failed_quests++;
            }
        }
        vec_free(v);

        if (failed_quests < 2) f += ((p_ptr->lev * 3 / 2) / (1 + failed_quests));
    }
    return (f*150 + f*f*3/2);
}

static bool _reforge_artifact(void)
{
    int  cost, extra_cost = 0, value;
    char o_name[MAX_NLEN];
    object_type *src, *dest;
    int src_max_power = reforge_limit();
    int dest_max_power = 0;
    int src_weight = 80;
    int new_day, prev_day, prev_hour, prev_min;

    if (p_ptr->prace == RACE_MON_SWORD || p_ptr->prace == RACE_MON_RING || p_ptr->prace == RACE_MON_ARMOR)
    {
        msg_print("Go enchant yourself!");
        return FALSE;
    }

    src = _get_reforge_src(src_max_power);
    if (!src) return FALSE;

    if (!object_is_artifact(src)) /* paranoia */
    {
        msg_print("You must choose an artifact for reforging.");
        return FALSE;
    }
    if (obj_value_real(src) > src_max_power)
    {
        msg_print("You are not famous enough to reforge that item.");
        return FALSE;
    }

    value = obj_value_real(src);
    src_weight = get_slot_weight(src);
    
    dest_max_power = MIN(1125L * src_weight / 2, value / 2);
    if (dest_max_power < 1000) /* Reforging won't try to power match weak stuff ... */
        dest_max_power = 1000;

    cost = _calculate_reforge_cost(value, src_weight);

    msg_format("Reforging will cost you %d gold.", cost);
    if (p_ptr->au < cost)
    {
        msg_print("You do not have that much.");
        return FALSE;
    }
    else if ((dest_max_power < value / 2) && (src_weight < 80))
    {
        msg_print("(Additional costs may be incurred reforging this item into certain equipment slots.)");
        paivita = TRUE; /* The long message wipes the storeminder's name, so we need to redraw it */
        paivitys_no_inkey_hack = TRUE;
    }

    object_desc(o_name, src, OD_NAME_ONLY | OD_COLOR_CODED);
    if (!get_check(format("Really use %s? (It will be destroyed!) ", o_name))) 
        return FALSE;

    dest = _get_reforge_dest(dest_max_power);
    if (!dest) return FALSE;

    if (dest->number > 1)
    {
        msg_print("Don't be greedy! You may only reforge a single object.");
        return _reforge_artifact_exit();
    }

    if (object_is_artifact(dest))
    {
        msg_print("This item is already an artifact!");
        return _reforge_artifact_exit();
    }

    if (dest->tval == TV_QUIVER)
    {
        msg_print("I am unable to reforge quivers.");
        return _reforge_artifact_exit();
    }

    if (object_is_ammo(dest))
    {
        msg_print("I'm a weaponsmith not a fletcher. Perhaps you should check elsewhere?");
        return _reforge_artifact_exit();
    }

    if (have_flag(dest->flags, OF_NO_REMOVE))
    {
        msg_print("You cannot be reforged!");
        return _reforge_artifact_exit();
    }

    if (object_is_ego(dest))
    {
        msg_print("This item is already an ego item!");
        return _reforge_artifact_exit();
    }

    if (!equip_first_slot(dest))
    {
        msg_print("This item cannot be reforged.");
        return _reforge_artifact_exit();
    }

    if (obj_value_real(dest) > dest_max_power)
    {
        msg_print("This item is too powerful for the source artifact you have chosen.");
        return _reforge_artifact_exit();
    }

    /* Items like Vilya count for the full 90K when reforged into e.g.
     * the armour slot, so the reforge should also cost accordingly */
    if (dest_max_power < value / 2)
    {
        int dest_weight = get_dest_weight(dest);
        if (dest_weight > src_weight)
        {
            extra_cost = _calculate_reforge_cost(value, dest_weight) - cost;
            if (extra_cost > 0)
            {
                if (p_ptr->au < cost + extra_cost)
                {
                    msg_format("This reforge will cost an additional %d gold. You do not have that much.", extra_cost);
                    return _reforge_artifact_exit();
                }
                if (!get_check(format("This reforge will cost an additional %d gold. Proceed?", extra_cost))) return FALSE;
            }
        }
    }

    if (reforge_details)
    {
        int dest_weight, min_power, max_power, voima, avg_bp, arvio, normi, minpwg, maxpwg, buflen, adjustment = 0, adjustment2 = 0;
        char src_name[MAX_NLEN], dest_name[MAX_NLEN], buf[10];
        int arvo = obj_value_real(dest) * 4 / 3;
        static point_t tbl[12] = { {1, 0}, {28, 2}, {45, 4}, {57, 6}, {64, 10}, {70, 18}, {75, 30}, {80, 45}, {85, 69}, {95, 105}, {105, 120}, {120, 170} };
        if ((object_is_gloves(dest)) && (arvo < 2000)) arvo = arvo / 3 + 1333;

        get_reforge_powers(TRUE, src, dest, &src_weight, &dest_weight, &min_power, &max_power, &avg_bp, p_ptr->fame);
        object_desc(src_name, src, (OD_NAME_ONLY));
        object_desc(dest_name, dest, (OD_NAME_ONLY));
        prt("Source:", 3, 1);
        c_put_str(tval_to_attr[src->tval], format("%-30.30s", src_name), 3, 9);
        put_str(format("Power: %-5d  Type strength: %d", MIN(value, MAX(src_weight, dest_weight) * 1125L), src_weight), 3, 41);
        put_str("Target:", 4, 1);
        c_put_str(tval_to_attr[dest->tval], format("%-30.30s", dest_name), 4, 9);
        put_str(format("Score: %-5d  Type strength: %d", obj_value(dest), dest_weight), 4, 41);

        voima = MIN(value * MIN(src_weight, dest_weight) / src_weight, dest_weight * 1125L);
        put_str("Reforge power after adjusting for item types:", 6, 1);
        c_put_str(TERM_L_BLUE, format("%d", voima), 6, 47);
        if (mut_present(MUT_INSPIRED_SMITHING)) avg_bp += avg_bp / 10; /* Eyeballing */
        put_str("Reforge power after adjusting for inspiration level:", 7, 1);
        c_put_str(TERM_L_BLUE, format("%d", avg_bp), 7, 54);
        put_str("Range of possible scores for resulting artifact:", 8, 1);
        strcpy(buf, format("%d", min_power));
        c_put_str(TERM_L_BLUE, buf, 8, 50);
        buflen = strlen(buf);
        put_str("to", 8, 51 + buflen);
        c_put_str(TERM_L_BLUE, format("%d", max_power), 8, 54 + buflen);
        put_str("Estimated average score:", 9, 1);
        if (max_power < arvo * 7 / 5)
        {
            c_put_str(TERM_RED, "Not recommended - no room for improvements", 10, 1);
        }
        else if ((src_weight < dest_weight * 9 / 10) && (value < dest_weight * 1000L))
        {
            c_put_str(TERM_RED, "Not recommended - source item has lower type strength", 10, 1);
        }
        else if ((src_weight == 80) && (value < 39000L) && ((dest_weight >= 45) || (src_max_power >= 45000L)))
        {
            c_put_str(TERM_RED, "Not recommended - cheap source item of high type strength", 10, 1);
        }
        /* Hack - there's no actual way to calculate a real expected score,
         * apart from generating a very large number of test artifacts and
         * calculating the average; but using that approach can cause the
         * game to become stuck, and still gives an unreliable estimate.
         * So we do evil voodoo instead
         * Calculating expected scores for dragon auxes is very difficult
         * and should be improved somehow
         * Anyone with any understanding of mathematics and probability is
         * advised to skip this section of the code, for their own sanity */
        normi = dest_weight * 460;
        if ((object_is_cloak(dest)) || (object_is_boots(dest)) || (object_is_gloves(dest)) || (object_is_shield(dest))) normi += arvo / 2;
        if (object_is_melee_weapon(dest))
        { /* This makes no sense at all, but somehow it works
           * Let's pray it keeps working and not think about it too closely */
            static point_t taulu[14] = { {1, 20}, {25, 22}, {40, 25}, {55, 30}, {65, 38}, {72, 44}, {76, 50}, {80, 60}, {90, 84}, {95, 100}, {100, 120}, {105, 150}, {110, 195}, {115, 270} };
            int paino5 = dest->ds * dest->ds * (dest->dd + 1);
            int paino3 = min_power / ((paino5 * 2) + (dest_weight * 2) + 240);
            int paino2 = interpolate(paino3, taulu, 14);
            s32b paino4 = (dest->ds * 10) * (dest->ds + 9) * 24 * (dest->dd * 10 - 5 + (40 * paino2 / 100)) / (35 + 40 * paino2 / 100) / 75;
            s32b paino = MAX(-100, ((paino4 / 10 + MIN(paino4, 700) * 9 / 10) * (paino2 + 5) / 3 + ((56 + dest_weight / 2) * (95 - paino2))) / 100);
            int dmult = 60 + MAX(0, paino);
            int n_arvo = 200L * (dest_weight + paino * 5 / 6);
            int t_arvo = (((n_arvo / 2) - (dest_weight * 35)) - (ABS(min_power - n_arvo)));
            if (t_arvo < 0) t_arvo /= 2;
            adjustment += t_arvo / 484 + 24 - ((ABS(dest_weight - 53)) / 3);
            if (object_is_(dest, TV_SWORD, SV_DIAMOND_EDGE)) adjustment -= ((min_power / 2 + 15000L) - (ABS(min_power - 32000L))) / 750;
            normi = (56 + (dest_weight * 5 / 2) + (MAX(0, paino) * 3 / 2)) * dmult;
            adjustment += dest_weight + 16 - paino;
//            put_str(format("P1/P2/P3/P4/P5: %d/%d/%d/%d/%d", paino, paino2, paino3, paino4, paino5), 12, 1);
        }
        if (object_is_shield(dest)) adjustment = -12; /* Shields tend to be fairly good compared to what they "ought" to be like */
        if (object_is_cloak(dest)) adjustment = -8;
        if (object_is_boots(dest)) adjustment = -10;

        /* Adjust for valuable source item */
        adjustment2 = interpolate(arvo * (object_is_melee_weapon(dest) ? 800L : 480L) / (min_power * 3 + max_power), tbl, 12);

//        put_str(format("Early adj2: %d", adjustment2), 10, 1);

        if (min_power < dest_weight * 200)
        {
            if (object_is_melee_weapon(dest))
                 adjustment2 += 126 * (dest_weight * 200 - min_power) / (dest_weight * 200);
            else adjustment2 += 18 * (dest_weight * 200 - min_power) / (dest_weight * 200);
        }

        /* Adjustments for "Bilbo sweet spot"
         * The generic formula tends to underestimate values in a range
         * roughly centered around a Bilbo-esque 58K weight-80 source */

        if (dest->tval == TV_AMULET)
        { /* It has to be true if empirical data says so */
            adjustment -= MAX(0, (6600 - ABS(min_power - 18000L)) / 300);
        }

        if (object_is_gloves(dest))
        {
            int nval = 16500L + arvo / 2;
            adjustment -= MAX(0, ((nval / 2) - ABS(min_power - nval)) / 480);
        }

        if (object_is_helmet(dest))
        {
            int nval = 23500L;
            adjustment2 -= (adjustment2 / 3);
            adjustment -= MAX(0, ((nval / 2) - ABS(min_power - nval)) / 600);
            adjustment -= MAX(0L, (32000L + MAX(0, arvo / 2 - 3500) - min_power)) / 100 * arvo / 12000 * (MIN(4800, MAX(100, min_power - arvo - 2000))) / 12000;
        }

        if (object_is_body_armour(dest))
        {
            int nval = MAX(0, 9000L - ABS(14000L - arvo)) * 4 / 3;
            adjustment -= MAX(0, nval - ABS(min_power - 32000L)) / 220;
        }

        if (object_is_shield(dest))
        {
            int nval = 17500L + arvo / 2;
            adjustment -= MAX(0, ((nval / 2) - ABS(min_power - nval)) / 720);
        }

        if (object_is_(dest, TV_HAFTED, SV_WIZSTAFF))
        {
            int muutos = ((8500 - ABS(min_power - 14000)) / 100);
            if (muutos < 0) muutos /= 4;
            adjustment2 -= muutos;
        }

        if (object_is_bow(dest)) /* All bows are unique, so we need to estimate them one by one */
        {
            switch (dest->sval)
            {
                case SV_SLING:
                    if (min_power > normi) adjustment2 -= (min_power - normi) * 50 / min_power;
                    break;
                case SV_SHORT_BOW:
                    adjustment2 += MAX(0, 12000L - ABS(27000L - min_power)) / 600;
                    if (min_power > normi) adjustment2 -= (min_power - normi) * 30 / min_power;
                    break;
                case SV_LONG_BOW:
                    {
                       int lisays = (12000L - ABS(27000L - min_power)) / 440;
                       if ((min_power < 25000) && (lisays < 0)) lisays = 0;
                       adjustment2 += lisays;
                       break;
                    }
                case SV_LIGHT_XBOW:
                    adjustment2 += MAX(0, 16000L - ABS(27000L - min_power)) / 440;
                    break;
                case SV_HEAVY_XBOW:
                    adjustment2 += MAX(0, 15000L - ABS(30000L - min_power)) / 330;
                    break;
                case SV_NAMAKE_BOW:
                    adjustment2 -= interpolate(min_power / 425, tbl, 12) / 2;
                    adjustment2 -= 17;
                    adjustment2 += MAX(0, 12000L - ABS(27000L - min_power)) / 480;
                    break;
                default: /* harps, guns */
                    adjustment2 += MAX(0, 7500L - ABS(18000L - min_power)) / 340;
                    break;
            }
        }

        minpwg = 448 - (((ABS(normi - min_power)) * 400L / (normi + min_power)));
        maxpwg = 448 - (((ABS(normi - max_power)) * 400L / (normi + max_power)));
//        put_str(format("Early minpwg/maxpwg: %d/%d Normi: %d", minpwg, maxpwg, normi), 13, 1);
        minpwg = (minpwg * 400L / (minpwg + maxpwg)) + 52 + adjustment - adjustment2;
        if (object_is_melee_weapon(dest)) /* Min-power is less important with melee weapons */
        {
            minpwg -= minpwg / 3;
            minpwg += 85;
        }
        minpwg = MAX(30, MIN(370, minpwg));

        arvio = (min_power * minpwg / 400) + (max_power * (400L - minpwg) / 400);
        if (dest->tval == TV_LITE) arvio += arvio / 75;
        else if (dest->tval == TV_RING) /* even worse */
        {
            arvio += arvio / 100;
            arvio += (arvio / 200) * (arvio / 200) / 100;
        }
        else if (object_is_(dest, TV_BOW, SV_HARP))
        {
            arvio += (arvio / 200) * (arvio / 200) / 100;
        }

//        put_str(format("Arvo: %d Adj: %d Adj2: %d Minpwg: %d", arvo, adjustment, adjustment2, minpwg), 14, 1);
         
        c_put_str(TERM_YELLOW, format("%d", arvio), 9, 26);
        paivita = TRUE;
        paivitys_no_inkey_hack = TRUE;
        if (!get_check("Proceed with this reforge? ")) return FALSE;
    }

    if (src->name1 == ART_STING)
    {
        char komento;
        paivita = TRUE;
        paivitys_no_inkey_hack = TRUE;
        msg_print("You have chosen to <color:R>reforge Sting</color>, certified as a <color:y>Grade A1 (Premium) Very Bad Idea</color> by the Ministry of Silly Plans. Experts have concluded there are <color:G>no circumstances</color> in which reforging Sting would be a good decision.\nProceed anyway? <color:y>[y/n]</color>");
        while (1)
        {
            bool lopeta = FALSE;
            komento = inkey_special(FALSE);
            switch (komento)
            {
                case ESCAPE:
                case 'n':
                case 'N':
                case 'q':
                case 'Q':
                     msg_print("You wisely decide not to use Sting as a reforge source.");
                     return FALSE;
                case 'y':
                case 'Y':
                     lopeta = TRUE;
                     break;
                default: break;
            }
            if (lopeta) break;
        };
        if (!p_ptr->wizard)
        {
            msg_print("Very well, you asked for it! (In fact, you've asked for it repeatedly! Remember, it was YOUR bad idea.)");
            (void)inkey();
        }
    }

    if (p_ptr->wizard)
    {
        doc_ptr doc = doc_alloc(80);
        int     i;
        int     ct = 100;
        int     base = obj_value_real(src);
        int     total = 0;
        char    buf[MAX_NLEN];

        object_desc(buf, src, OD_COLOR_CODED);
        doc_printf(doc, "Reforging %s (%d):\n", buf, base);
        if (strpos("test", player_name)) ct = 1000;
        for (i = 0; i < ct; i++)
        {
            obj_t forge = *dest;
            int   score = 0;
            reforge_artifact(src, &forge, p_ptr->fame);
            obj_identify_fully(&forge);
            score = obj_value_real(&forge);
            total += score;
            object_desc(buf, &forge, OD_COLOR_CODED);
            doc_printf(doc, "%d) <indent><style:indent>%s (%d%%)</style></indent>\n",
                i + 1, buf, score * 100 / base);
        }
        doc_printf(doc, "\n\n<color:B>Average Reforge: <color:R>%d%%</color> (Score: <color:R>%d</color>)</color>\n",
            total / base, total / ct);
        doc_display(doc, "Reforging", 0);
        doc_free(doc);
        return FALSE;
    }
    if (!reforge_artifact(src, dest, p_ptr->fame))
    {
        msg_print("The reforging failed!");
        return _reforge_artifact_exit();
    }

    src->number = 0;
    obj_release(src, OBJ_RELEASE_QUIET);
    src = NULL;

    if (extra_cost > 0) cost += extra_cost;
    p_ptr->au -= cost;
    stats_on_gold_services(cost);

    msg_print("After several hours, you are presented your new artifact...");
    extract_day_hour_min(&prev_day, &prev_hour, &prev_min);
    game_turn += rand_range(5000, 15000);
    prevent_turn_overflow();
    extract_day_hour_min(&new_day, &prev_hour, &prev_min);
    if (new_day != prev_day)
    {
        determine_today_mon(FALSE);
        if (p_ptr->prace == RACE_WEREWOLF) werewolf_check_midnight();
        if (ironman_nightmare)
        {
            msg_print("You fall asleep as you wait, and horrible visions flit through your mind...");
            get_mon_num_prep(get_nightmare, NULL);
            while(1)
            {
                have_nightmare(get_mon_num(MAX_DEPTH));
                if (!one_in_(3)) break;
            }

            get_mon_num_prep(NULL, NULL);
            msg_print("You awake screaming.");
        }
    }

    /* The items we carried recharged during those hours */
    _recharge_player_items();
    reset_tim_flags(); /* this can miraculously cure the player of cuts or
                        * poisoning, but given the cost of reforging that's
                        * probably only fair... */
    p_ptr->chp = p_ptr->mhp;
    if ((p_ptr->pclass != CLASS_RUNE_KNIGHT) && (!elemental_is_(ELEMENTAL_WATER)))
        p_ptr->csp = p_ptr->msp;

    /* Update discovery details to apply to the reforged item
     * We do this after updating the turn to get the correct time */
    dest->mitze_type = 0;
    object_mitze(dest, MITZE_REFORGE);

    obj_identify_fully(dest);
    gear_notice_enchant(dest);
    handle_stuff();

    obj_display(dest);
    return TRUE;
}

typedef struct _enchant_choice_s { int amt; int cost; } _enchant_choice_t;
static void _enchant_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    _enchant_choice_t *ptr = (_enchant_choice_t *)cookie;
    ptr += which;
    switch (cmd)
    {
    case MENU_TEXT:
    {
        char tmp[255];
        if (ptr->cost >= 10000)
            big_num_display(ptr->cost, tmp);
        else
            sprintf(tmp, "%d", ptr->cost);

        var_set_string(res, format("+%2d %9.9s", ptr->amt, tmp));
        break;
    }
    case MENU_COLOR:
        if (ptr->cost > p_ptr->au)
        {
            var_set_int(res, TERM_L_DARK);
            break;
        }
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static bool enchant_item(obj_p filter, int cost, int to_hit, int to_dam, int to_ac, bool is_guild)
{
    obj_prompt_t prompt = {0};
    int          i;
    bool         okay = FALSE;
    int          maxenchant;
    char         tmp_str[MAX_NLEN];

    if (cost == 0)
        cost = town_service_price(1500);

    if ((p_ptr->prace == RACE_MON_SWORD) || (p_ptr->prace == RACE_MON_ARMOR))
    {
        msg_print("Go enchant yourself!");
        return FALSE;
    }

    clear_bldg(4, 18);

    prompt.prompt = "Improve which item?";
    prompt.error = "You have nothing to improve.";
    prompt.filter = filter;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;

    if (is_guild)
        maxenchant = 5 + p_ptr->lev/5;
    else
        maxenchant = 2 + p_ptr->lev/5;

    /* Streamline. Nothing is more fun than enchanting Twilight (-40,-60)->(+10, +10), I
       admit. But other players might not share my love of carpal tunnel syndrome! */
    {
        int idx = -1;
        int old_cost;
        int unit_cost_sum = 0;
        _enchant_choice_t choices[25];
        object_type copy = {0};
        menu_t menu = { "Enchant How Much?", NULL, 
                        "Amt      Cost", _enchant_menu_fn,
                        choices, 25, 0 };

        object_copy(&copy, prompt.obj);
        copy.curse_flags = 0;
        remove_flag(copy.flags, OF_AGGRAVATE);
        remove_flag(copy.flags, OF_NO_TELE);
        remove_flag(copy.flags, OF_NO_MAGIC);
        remove_flag(copy.flags, OF_DRAIN_EXP);
        remove_flag(copy.flags, OF_TY_CURSE);
        old_cost = new_object_cost(&copy, COST_REAL);
                
        for (i = 0; i < 25; i++) /* TODO: Option for max. But +25 a pop is enough perhaps? */
        {
            bool ok = FALSE;
            int  v = 0, m = 5;
            
            choices[i].amt = i+1;

            if (to_hit && copy.to_h < maxenchant) {copy.to_h++; ok = TRUE; v = MAX(v, copy.to_h);}
            if (to_dam && copy.to_d < maxenchant) {copy.to_d++; ok = TRUE; v = MAX(v, copy.to_d);}
            if (to_ac && copy.to_a < maxenchant) {copy.to_a++; ok = TRUE; v = MAX(v, copy.to_a);}

            if (v > 10)
            {
                if (to_ac)
                {
                    int j;
                    for (j = 10; j < v; j++)
                        m = m * 5 / 3;
                }
                else
                    m += v - 10;
            }

            if (object_is_artifact(prompt.obj))
                m *= 3;

            if (!ok) break;

            if (prompt.obj->tval == TV_ARROW || prompt.obj->tval == TV_BOLT || prompt.obj->tval == TV_SHOT)
            {
                choices[i].cost = (i+1)*cost*prompt.obj->number;
            }
            else
            {
                int new_cost = new_object_cost(&copy, COST_REAL);
                int unit_cost_add = new_cost - old_cost;
                int min_cost = (i+1)*cost;
                int unit_cost;
                old_cost = new_cost;

                unit_cost_add *= m;
                unit_cost_sum += unit_cost_add;
                unit_cost = unit_cost_sum;

                unit_cost = town_service_price(unit_cost);

                if (unit_cost < min_cost)
                    unit_cost = min_cost;

                if (is_guild)
                    unit_cost = (unit_cost + 1)/2;

                if (unit_cost >= 10000)
                    choices[i].cost = big_num_round(unit_cost, 3);
                else
                    choices[i].cost = unit_cost;
            }
        }
        if (!i)
        {
            object_desc(tmp_str, prompt.obj, OD_COLOR_CODED);
            msg_format("%^s can not be further improved.", tmp_str);
            return FALSE;
        }

        menu.count = i;
        idx = menu_choose(&menu);
        if (idx < 0) return FALSE;
        
        if (to_hit) to_hit = choices[idx].amt;
        if (to_dam) to_dam = choices[idx].amt;
        if (to_ac) to_ac = choices[idx].amt;
        cost = choices[idx].cost;
    }

    /* Check if the player has enough money */
    if (p_ptr->au < cost)
    {
        object_desc(tmp_str, prompt.obj, OD_NAME_ONLY | OD_COLOR_CODED);
        msg_format("You do not have the gold to improve %s!", tmp_str);
        return FALSE;
    }

    /* Enchant to hit */
    for (i = 0; i < to_hit; i++)
    {
        if (prompt.obj->to_h < maxenchant)
        {
            if (enchant(prompt.obj, 1, (ENCH_TOHIT | ENCH_FORCE)))
                okay = TRUE;
        }
    }

    /* Enchant to damage */
    for (i = 0; i < to_dam; i++)
    {
        if (prompt.obj->to_d < maxenchant)
        {
            if (enchant(prompt.obj, 1, (ENCH_TODAM | ENCH_FORCE)))
                okay = TRUE;
        }
    }

    /* Enchant to AC */
    for (i = 0; i < to_ac; i++)
    {
        if (prompt.obj->to_a < maxenchant)
        {
            if (enchant(prompt.obj, 1, (ENCH_TOAC | ENCH_FORCE)))
                okay = TRUE;
        }
    }

    if (!okay)
    {
        if (flush_failure) flush();
        msg_print("The improvement failed.");
        return (FALSE);
    }
    else
    {
        object_desc(tmp_str, prompt.obj, OD_NAME_AND_ENCHANT | OD_COLOR_CODED);
        msg_format("Improved %s for %d gold.", tmp_str, cost);

        p_ptr->au -= cost;
        stats_on_gold_services(cost);
        obj_release(prompt.obj, OBJ_RELEASE_ENCHANT);
        return (TRUE);
    }
}

bool tele_town(void)
{
    int i, x, y;
    int num = 0;
    int town_id = 0;

    if (dun_level)
    {
        msg_print("This spell can only be used on the surface!");
        return FALSE;
    }

    if (p_ptr->inside_arena || p_ptr->inside_battle)
    {
        msg_print("This spell can only be used outside!");
        return FALSE;
    }

    screen_save();
    clear_bldg(4, 10);

    for (i = TOWN_MIN; i <= TOWN_MAX_STD; i++)
    {
        char buf[80];

        if (i == p_ptr->town_num) continue;
        if ((num) && (easy_thalos) && (i == TOWN_THALOS)) town_on_visit(i); /* Make people not hate me */
        if (!town_visited(i) && !p_ptr->wizard) continue;

        sprintf(buf,"%c) %-20s", I2A(i-1), town_name(i));
        prt(buf, 5+i, 5);
        num++;
    }

    if (!num)
    {
        msg_print("You have not yet visited any town.");
        msg_print(NULL);
        screen_load();
        return FALSE;
    }

    prt("Which town you go: ", 0, 0);
    while(1)
    {
        i = inkey();

        if (i == ESCAPE)
        {
            screen_load();
            return FALSE;
        }
        else if ((i < 'a') || (i > ('a'+TOWN_MAX_STD-1))) continue;

        town_id = A2I(i) + 1;
        if (town_id == p_ptr->town_num) continue;
        if (!p_ptr->wizard && !town_visited(town_id)) continue;
        break;
    }

    for (y = 0; y < max_wild_y; y++)
    {
        for (x = 0; x < max_wild_x; x++)
        {
            if(wilderness[y][x].town == town_id)
            {
                p_ptr->wilderness_y = y;
                p_ptr->wilderness_x = x;

                p_ptr->wilderness_dx = 0;
                p_ptr->wilderness_dy = 0;
            }
        }
    }

    p_ptr->leaving = TRUE;
    leave_bldg = TRUE;
    p_ptr->teleport_town = TRUE;
    screen_load();
    return TRUE;
}


/*
 *  research_mon
 *  -KMW-
 */
static bool research_mon(void)
{
    int i, n, r_idx;
    char sym, query;
    char buf[128];

    bool notpicked;

    bool recall = FALSE;

    u16b why = 0;

    u16b    *who;

    /* XTRA HACK WHATSEARCH */
    bool    all = FALSE;
    bool    uniq = FALSE;
    bool    norm = FALSE;
    char temp[80] = "";

    /* XTRA HACK REMEMBER_IDX */
    static int old_sym = '\0';
    static int old_i = 0;


    /* Save the screen */
    screen_save();

    /* Get a character, or abort */
    if (!get_com("Enter character to be identified(^A:All,^U:Uniqs,^N:Non uniqs,^M:Name): ", &sym, FALSE))

    {
        /* Restore */
        screen_load();

        return (FALSE);
    }

    /* Find that character info, and describe it */
    for (i = 0; ident_info[i]; ++i)
    {
        if (sym == ident_info[i][0]) break;
    }

        /* XTRA HACK WHATSEARCH */
    if (sym == KTRL('A'))
    {
        all = TRUE;
        strcpy(buf, "Full monster list.");
    }
    else if (sym == KTRL('U'))
    {
        all = uniq = TRUE;
        strcpy(buf, "Unique monster list.");
    }
    else if (sym == KTRL('N'))
    {
        all = norm = TRUE;
        strcpy(buf, "Non-unique monster list.");
    }
    else if (sym == KTRL('M'))
    {
        all = TRUE;
        if (!get_string("Enter name:",temp, 70))
        {
            temp[0]=0;

            /* Restore */
            screen_load();

            return FALSE;
        }
        sprintf(buf, "Monsters with a name \"%s\"",temp);
    }
    else if (ident_info[i])
    {
        sprintf(buf, "%c - %s.", sym, ident_info[i] + 2);
    }
    else
    {
        sprintf(buf, "%c - %s.", sym, "Unknown Symbol");

    }

    /* Display the result */
    prt(buf, 16, 10);


    /* Allocate the "who" array */
    C_MAKE(who, max_r_idx, u16b);

    /* Collect matching monsters */
    for (n = 0, i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Empty monster */
        if (!r_ptr->name) continue;

        if (r_ptr->flagsx & RFX_SUPPRESS) continue;

        /* XTRA HACK WHATSEARCH */
        /* Require non-unique monsters if needed */
        if (norm && (r_ptr->flags1 & (RF1_UNIQUE))) continue;

        /* Require unique monsters if needed */
        if (uniq && !(r_ptr->flags1 & (RF1_UNIQUE))) continue;

        if (temp[0])
        {
            int xx;
            char temp2[80];

            for (xx = 0; temp[xx] && xx < 80; xx++)
            {
                if (isupper(temp[xx])) temp[xx] = tolower(temp[xx]);
            }
  
            strcpy(temp2, r_name + r_ptr->name);
            for (xx = 0; temp2[xx] && xx < 80; xx++)
                if (isupper(temp2[xx])) temp2[xx] = tolower(temp2[xx]);

            if (my_strstr(temp2, temp))
                who[n++] = i;
        }
        else if (all || (r_ptr->d_char == sym)) who[n++] = i;
    }

    /* Nothing to recall */
    if (!n)
    {
        /* Free the "who" array */
        C_KILL(who, max_r_idx, u16b);

        /* Restore */
        screen_load();

        return (FALSE);
    }

    /* Sort by level */
    why = 2;
    query = 'y';

    /* Sort if needed */
    if (why)
    {
        /* Select the sort method */
        ang_sort_comp = ang_sort_comp_hook;
        ang_sort_swap = ang_sort_swap_hook;

        /* Sort the array */
        ang_sort(who, &why, n);
    }


    /* Start at the end */
    /* XTRA HACK REMEMBER_IDX */
    if (old_sym == sym && old_i < n) i = old_i;
    else i = n - 1;

    notpicked = TRUE;

    /* Scan the monster memory */
    while (notpicked)
    {
        /* Extract a race */
        r_idx = who[i];

        /* Hack -- Begin the prompt */
        roff_top(r_idx);

        /* Hack -- Complete the prompt */
        Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC, space to continue]");


        /* Interact */
        while (1)
        {
            /* Recall */
            if (recall)
            {
                /*** Recall on screen ***/

                /* Mark Monster as Seen for Monster Knowledge Screens */
                if (!r_info[r_idx].r_sights)
                    r_info[r_idx].r_sights = 1;

                /* Get maximal info about this monster */
                lore_do_probe(r_idx);

                /* Save this monster ID */
                monster_race_track(r_idx);

                /* Hack -- Handle stuff */
                handle_stuff();

                /* know every thing mode */
                mon_display(&r_info[r_idx]);
                notpicked = FALSE;

                /* XTRA HACK REMEMBER_IDX */
                old_sym = sym;
                old_i = i;
            }

            /* Command */
            query = inkey();

            /* Normal commands */
            if (query != 'r') break;

            /* Toggle recall */
            recall = !recall;
        }

        /* Stop scanning */
        if (query == ESCAPE) break;

        /* Move to "prev" monster */
        if (query == '-')
        {
            if (++i == n)
            {
                i = 0;
                if (!expand_list) break;
            }
        }

        /* Move to "next" monster */
        else
        {
            if (i-- == 0)
            {
                i = n - 1;
                if (!expand_list) break;
            }
        }
    }


    /* Re-display the identity */
    /* prt(buf, 5, 5);*/

    /* Free the "who" array */
    C_KILL(who, max_r_idx, u16b);

    /* Restore */
    screen_load();

    return (!notpicked);
}

static bool _object_is_photograph(object_type *o_ptr)
{
    return (object_is_(o_ptr, TV_STATUE, SV_PHOTO));
}

/* Loosely based on _buy and _buy_aux from shop.c */
static void _sell_photo_local_aux(object_type *o_ptr)
{
    int amt = o_ptr->number, tarjous = 0, jaljella = 0;
    bool myy = FALSE;
    char name[MAX_NLEN];
    if (o_ptr->number > 1)
    {
        if (!msg_input_num("Quantity", &amt, 1, o_ptr->number)) return;
    }
    if (o_ptr->pval == MON_BARNEY)
    {
        msg_format("Thanks, but we already have more than enough photographs of him.");
        return;
    }
    tarjous = tourist_sell_photo_aux(o_ptr, amt, FALSE);
    if (tarjous == -1)
    {
        msg_format("We're not interested in %s.", (amt == 1) ? "this photograph" : "these photographs");
        return;
    }
    myy = ((tarjous > 0) && (!no_selling));
    if (tarjous > 2000)
    {
        if (o_ptr->pval == MON_TSUCHINOKO) msg_print("Hey, nice find!");
        else msg_print("Is that... BIGFOOT????");
        myy = TRUE;
        if (no_selling) tarjous = (tarjous / 1000) * 1000;
    }
    if (!myy) tarjous = 0;
    jaljella = o_ptr->number - amt;
    o_ptr->number = amt;
    object_desc(name, o_ptr, OD_COLOR_CODED);
    if (!tarjous)
    {
        if (!get_check(format("Really give %s? ", name)))
        {
            o_ptr->number = amt + jaljella;
            return;
        }
    }
    else
    {
        if (!get_check(format("Really sell %s for <color:R>%d</color> gp? ", name, tarjous)))
        {
            o_ptr->number = amt + jaljella;
            return;
        }
        p_ptr->au += tarjous;
        stats_on_gold_selling(tarjous);
        p_ptr->redraw |= PR_GOLD;
        if (prace_is_(RACE_MON_LEPRECHAUN))
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
    }

    object_desc(name, o_ptr, OD_COLOR_CODED);
    if (!tarjous)
        msg_format("You gave %s.", name);
    else
        msg_format("You sold %s for <color:R>%d</color> gold.", name, tarjous);
    (void)tourist_sell_photo_aux(o_ptr, amt, TRUE);
    o_ptr->number = jaljella;
    p_ptr->notice |= (PN_CARRY | PN_OPTIMIZE_PACK);
}

static void _sell_photo(void)
{
    obj_prompt_t prompt = {0};
    if (p_ptr->pclass != CLASS_TOURIST) return;

    if (no_selling)
    {
        prompt.prompt = "Give which photograph?";
    }
    else
    {
        prompt.prompt = "Sell which photograph?";
    }
    prompt.error = "You don't have any photographs.";
    prompt.filter = _object_is_photograph;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_QUIVER;

    obj_prompt(&prompt);
    if (!prompt.obj) return;
    _sell_photo_local_aux(prompt.obj);
    obj_release(prompt.obj, OBJ_RELEASE_QUIET);
}

/*
 * Execute a building command
 */
static void bldg_process_command(building_type *bldg, int i)
{
    int bact = bldg->actions[i];
    int bcost;
    bool paid = FALSE;
    int amt;
    bool is_guild = FALSE;

    /* Flush messages XXX XXX XXX */
    msg_line_clear();

    if (is_owner(bldg))
    {
        bcost = bldg->member_costs[i];
        is_guild = TRUE;
    }
    else
        bcost = bldg->other_costs[i];

    /* Adjst price for race, charisma and fame */
    bcost = town_service_price(bcost);

    /* action restrictions */
    if (((bldg->action_restr[i] == 1) && !is_member(bldg)) ||
        ((bldg->action_restr[i] == 2) && !is_owner(bldg)))
    {
        msg_print("You have no right to choose that!");
        return;
    }

    if (bcost > p_ptr->au)
    {
        msg_print("You do not have the gold!");
        return;
    }

    switch (bact)
    {
    case BACT_NOTHING:
        /* Do nothing */
        break;
    case BACT_RESEARCH_ITEM:
        paid = identify_fully(NULL);
        break;
    case BACT_TOWN_HISTORY:
        town_history();
        break;
    case BACT_RACE_LEGENDS:
        /*race_legends();*/
        break;
    case BACT_QUEST:
        castle_quest();
        break;
    case BACT_KING_LEGENDS:
    case BACT_ARENA_LEGENDS:
    case BACT_LEGENDS:
        /*show_highclass();*/
        break;
    case BACT_POSTER:
    case BACT_ARENA_RULES:
    case BACT_ARENA:
        arena_comm(bact);
        break;
    case BACT_IN_BETWEEN:
    case BACT_CRAPS:
    case BACT_SPIN_WHEEL:
    case BACT_DICE_SLOTS:
    case BACT_GAMBLE_RULES:
    case BACT_POKER:
        gamble_comm(bact);
        break;
    case BACT_REST:
    case BACT_RUMORS:
    case BACT_FOOD:
        paid = inn_comm(bact);
        break;
    case BACT_RESEARCH_MONSTER:
        paid = research_mon();
        break;
    case BACT_COMPARE_WEAPONS:
    /*    paid = compare_weapons(); */
        break;
    case BACT_ENCHANT_WEAPON:
        enchant_item(object_allow_enchant_melee_weapon, bcost, 1, 1, 0, is_guild);
        break;
    case BACT_ENCHANT_ARMOR:
        enchant_item(object_allow_enchant_armour, bcost, 0, 0, 1, is_guild);
        break;
    case BACT_RECHARGE:
        msg_print("My apologies, but that service is no longer available!");
        break;
    case BACT_RECHARGE_ALL:
        msg_print("My apologies, but that service is no longer available!");
        break;
    case BACT_IDENTS: /* needs work */
        if (!get_check("Pay to have all your possessions identified? ")) break;
        identify_pack();
        msg_print("Your possessions have been identified.");

        paid = TRUE;
        break;
    case BACT_IDENT_ONE: /* needs work */
        paid = ident_spell(NULL);
        break;
    case BACT_LEARN:
        do_cmd_study();
        break;
    case BACT_HEALING: /* needs work */
        hp_player(200);
        set_poisoned(0, TRUE);
        set_blind(0, TRUE);
        set_confused(0, TRUE);
        set_cut(0, TRUE);
        set_stun(0, TRUE);

        if (p_ptr->riding)
        {
            monster_type *m_ptr = &m_list[p_ptr->riding];
            int           amt = 500;

            if (m_ptr->hp < 30000) m_ptr->hp += amt;
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
            p_ptr->redraw |= PR_HEALTH_BARS;
        }

        paid = TRUE;
        break;
    case BACT_RESTORE: /* needs work */
        if (do_res_stat(A_STR)) paid = TRUE;
        if (do_res_stat(A_INT)) paid = TRUE;
        if (do_res_stat(A_WIS)) paid = TRUE;
        if (do_res_stat(A_DEX)) paid = TRUE;
        if (do_res_stat(A_CON)) paid = TRUE;
        if (do_res_stat(A_CHR)) paid = TRUE;
        if (restore_level()) paid = TRUE;
        if (lp_player(1000)) paid = TRUE;
        break;
    case BACT_ENCHANT_ARROWS:
        enchant_item(item_tester_hook_ammo, bcost, 1, 1, 0, is_guild);
        break;
    case BACT_ENCHANT_BOW:
        enchant_item(object_is_bow, bcost, 1, 1, 0, is_guild);
        break;
    case BACT_RECALL:
        if (recall_player(1, FALSE)) paid = TRUE;
        break;
    case BACT_TELEPORT_LEVEL:
    {
        int select_dungeon;
        int max_depth;

        clear_bldg(4, 20);
        select_dungeon = choose_dungeon("teleport", 4, 0);
        show_building(bldg);
        if (!select_dungeon) return;

        max_depth = d_info[select_dungeon].maxdepth;

        /* Limit depth in Angband */
        if (select_dungeon == DUNGEON_ANGBAND)
        {
            if (quests_get(QUEST_OBERON)->status != QS_FINISHED) max_depth = 98;
            else if(quests_get(QUEST_SERPENT)->status != QS_FINISHED) max_depth = 99;
        }

        amt = get_quantity(format("Teleport to which level of %s? ", d_name + d_info[select_dungeon].name), max_depth);
        if (amt > 0)
        {
            p_ptr->word_recall = 1;
            p_ptr->recall_dungeon = select_dungeon;
            max_dlv[p_ptr->recall_dungeon] = ((amt > d_info[select_dungeon].maxdepth) ? d_info[select_dungeon].maxdepth : ((amt < d_info[select_dungeon].mindepth) ? d_info[select_dungeon].mindepth : amt));
            msg_print("The air about you becomes charged...");

            paid = TRUE;
            p_ptr->redraw |= (PR_STATUS);
        }
        break;
    }
    case BACT_LOSE_MUTATION:
        if (mut_count(mut_unlocked_pred))
        {
            paid = mut_lose_random(NULL);
        }
        else
        {
            msg_print("You have no mutations that I can cure.");
        }
        break;
    case BACT_BATTLE:
        kakutoujou();
        break;
    case BACT_TSUCHINOKO:
        tsuchinoko();
        break;
    case BACT_KUBI:
        shoukinkubi();
        break;
    case BACT_TARGET:
        today_target();
        break;
    case BACT_KANKIN:
        kankin();
        break;
    case BACT_HEIKOUKA:
        msg_print("You received an equalization ritual.");
        virtue_init();
        paid = TRUE;
        break;
    case BACT_TELE_TOWN:
        paid = tele_town();
        break;
    case BACT_EVAL_AC:
        paid = eval_ac(p_ptr->dis_ac + p_ptr->dis_to_a);
        break;
    case BACT_GAMBLE_SHOP_POTION:
        paid = _gamble_shop(_gamble_shop_potions);
        break;
    case BACT_GAMBLE_SHOP_SCROLL:
        paid = _gamble_shop(_gamble_shop_scrolls);
        break;
    case BACT_GAMBLE_SHOP_STAFF:
        paid = _gamble_shop_device(TV_STAFF);
        break;
    case BACT_GAMBLE_SHOP_WAND:
        paid = _gamble_shop_device(TV_WAND);
        break;
    case BACT_GAMBLE_SHOP_ROD:
        paid = _gamble_shop_device(TV_ROD);
        break;
    case BACT_GAMBLE_SHOP_ARMOR:
        paid = _gamble_shop_object(object_is_armour);
        break;
    case BACT_GAMBLE_SHOP_WEAPON:
        paid = _gamble_shop_object(object_is_weapon);
        break;
    case BACT_GAMBLE_SHOP_ARTIFACT:
        paid = _gamble_shop_artifact();
        break;
    case BACT_REFORGE_ARTIFACT:
        paid = _reforge_artifact();
        break;
    case BACT_CHANGE_NAME:
        if (p_ptr->pclass == CLASS_POLITICIAN)
        {
            msg_print("No way! Think of your name recognition.");
        }
        else if (py_get_name())
        {
            process_player_name(FALSE);
            paid = TRUE;
        }
        break;
    case BACT_REPUTATION:
        if (p_ptr->fame <= 0)
            cmsg_print(TERM_WHITE, "Who the hell are you?");
        else if (p_ptr->fame < 20)
            cmsg_print(TERM_WHITE, "I've never even heard of you!");
        else if (p_ptr->fame < 40)
            cmsg_print(TERM_L_UMBER, "Hmmm ... You've done a few minor notable deeds, but hardly anything worth bragging about!");
        else if (p_ptr->fame < 60)
            cmsg_print(TERM_YELLOW, "Yes, I've heard of you. The townfolk are talking!");
        else if (p_ptr->fame < 80)
            cmsg_print(TERM_ORANGE, "Ah, good sir. 'Tis an honor to see you again!");
        else if (p_ptr->fame < 100)
            cmsg_print(TERM_L_RED, "You are a true hero!");
        else if (p_ptr->fame < 150)
            cmsg_print(TERM_RED, "You are the stuff of legends!");
        else
            cmsg_print(TERM_VIOLET, "The bards doth sing of ye: Heroic ballads both far 'n wide!");
        paid = TRUE;
        break;
    case BACT_SELL_PHOTO:
        _sell_photo();
        break;
    case BACT_LOAN:
    case BACT_DEPOSIT:
    case BACT_INSURANCE:
    case BACT_CORNY_CASH_IN:
    case BACT_VIEW_POLICY:
    case BACT_VIEW_POSTER:
        cornucopia_do_command(bldg, bact);
        break;
    }

    if (paid)
    {
        p_ptr->au -= bcost;
        stats_on_gold_services(bcost);
        if (prace_is_(RACE_MON_LEPRECHAUN))
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
    }
}


/*
 * Enter quest level
 */
void do_cmd_quest(void)
{
    energy_use = 100;

    if (!cave_have_flag_bold(py, px, FF_QUEST_ENTER))
    {
        msg_print("You see no quest level here.");
        return;
    }
    else
    {
        int quest_id = cave[py][px].special;
        int danger = quests_get_level(quest_id);
        msg_format("This is the entrance to the quest: <color:B>%s</color>.",
            quests_get_name(quest_id));
        if (((danger < 23) && (p_ptr->lev < danger - 4)) ||
            ((danger < 34) && (p_ptr->lev < danger - 7)) ||
            ((danger > 33) && (danger < 46) && (p_ptr->lev < (danger / 2) + 11)) ||
            ((danger > 45) && (p_ptr->lev < (danger * 4 / 5) - 3) && (p_ptr->lev < 46)))
        {
            msg_format("\n<color:R>WARNING:</color> This is a level <color:o>%d</color> quest.\n", danger);
            sound(SOUND_WARN);
        }
        if (!get_check("Do you enter? ")) return;

        /* Player enters a new quest XXX */
        p_ptr->oldpy = py;
        p_ptr->oldpx = px;

        enter_quest = quest_id;
        p_ptr->leaving = TRUE;
    }
}


/*
 * Do building commands
 */
void do_cmd_bldg(void)
{
    int             i, which;
    char            command;
    bool            validcmd;
    building_type   *bldg;

    energy_use = 100;

    if (!cave_have_flag_bold(py, px, FF_BLDG))
    {
        msg_print("You see no building here.");

        return;
    }

    which = f_info[cave[py][px].feat].subtype;

    bldg = &building[which];

    /* Don't re-init the wilderness */
    reinit_wilderness = FALSE;

    if ((no_wilderness) && (p_ptr->lev < 10) && (which == 6))
    {
        msg_print("Get some adventuring under your belt before you start gambling!");
        return;
    }
    else if ((which == 2) && (p_ptr->arena_number < 0))
    {
        msg_print("'There's no place here for a LOSER like you!'");
        return;
    }
    else if ((which == 2) && p_ptr->inside_arena)
    {
        if (!p_ptr->exit_bldg)
        {
            prt("The gates are closed. The monster awaits!", 0, 0);
        }
        else
        {
            /* Player is not victorious unless they manage to leave the
             * arena alive! */
            if (p_ptr->arena_number > MAX_ARENA_MONS) p_ptr->arena_number++;
            p_ptr->arena_number++;

            /* Don't save the arena as saved floor */
            prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_NO_RETURN);

            p_ptr->inside_arena = FALSE;
            p_ptr->leaving = TRUE;

            /* Re-enter the arena */
            command_new = SPECIAL_KEY_BUILDING;

            /* Refresh buildings (otherwise bad things happen if we saved and loaded inside the arena) */
            refresh_buildings();

            /* No energy needed to re-enter the arena */
            energy_use = 0;
        }

        return;
    }
    else if (prace_is_(RACE_MON_LEPRECHAUN) && strpos("Cornucopia", bldg->name))
    {
        msg_print("We don't serve your kind in here!");
        return;
    }
    else if (p_ptr->inside_battle)
    {
        /* Don't save the arena as saved floor */
        prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_NO_RETURN);

        p_ptr->leaving = TRUE;
        p_ptr->inside_battle = FALSE;

        /* Re-enter the monster arena */
        command_new = SPECIAL_KEY_BUILDING;

        /* No energy needed to re-enter the arena */
        energy_use = 0;

        return;
    }
    else
    {
        p_ptr->oldpy = py;
        p_ptr->oldpx = px;
    }

    /* Forget the lite */
    forget_lite();

    /* Forget the view */
    forget_view();

    /* Hack -- Increase "icky" depth */
    character_icky++;

    command_arg = 0;
    command_rep = 0;
    command_new = 0;

    leave_bldg = FALSE;
    show_building(bldg);

    msg_line_init(ui_shop_msg_rect());
    store_hack = TRUE;

    while (!leave_bldg)
    {
        validcmd = FALSE;

        building_prt_gold();

        command = inkey();

        if (command == ESCAPE)
        {
            leave_bldg = TRUE;
            p_ptr->inside_arena = FALSE;
            p_ptr->inside_battle = FALSE;
            break;
        }

        for (i = 0; i < 8; i++)
        {
            if (bldg->letters[i])
            {
                if (bldg->letters[i] == command)
                {
                    validcmd = TRUE;
                    break;
                }
            }
        }

        if (validcmd)
        {
            bldg_process_command(bldg, i);
        }

        if (pack_overflow_count())
        {
            msg_print("<color:v>Your pack is overflowing!</color> It's time for you to leave!");
            msg_print(NULL);
            leave_bldg = TRUE;
        }

        /* Notice stuff */
        notice_stuff();

        /* Handle stuff */
        handle_stuff();

        if (paivita)
        {
            if (leave_bldg) paivita = FALSE;
            else
            {
                if (!paivitys_no_inkey_hack) inkey();
                paivitys_no_inkey_hack = FALSE;
                show_building(bldg);
            }
        }
    }

    store_hack = FALSE;
    msg_line_init(ui_msg_rect());

    /* Force reinit if which == 2 (or bad things happen if we saved and loaded inside the arena) */
    if (which == 2) reinit_wilderness = TRUE;

    /* Reinit wilderness to activate quests ... */
    if (reinit_wilderness)
    {
        p_ptr->leaving = TRUE;
        quest_reward_drop_hack = TRUE;
    }

    /* Hack -- Decrease "icky" depth */
    character_icky--;

    /* Clear the screen */
    Term_clear();

    /* Update the visuals */
    p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_BONUS | PU_LITE | PU_MON_LITE);

    /* Redraw entire screen */
    p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_EQUIPPY | PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
}


