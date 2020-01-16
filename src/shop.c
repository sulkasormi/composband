#include "angband.h"

#include <assert.h>

bool store_hack = FALSE;

/************************************************************************
 * Data Types
 ***********************************************************************/
#define _MAX_STOCK  24
#define _MAX_OWNERS 32

struct _owner_s
{
    int  id;
    cptr name;
    int  purse;
    int  greed;
    int  race_id;
    bool active;
};
typedef struct _owner_s _owner_t, *_owner_ptr;

typedef bool (*_k_idx_p)(int k_idx);
struct _type_s
{
    int           id;
    cptr          name;
    obj_p         buy_p;
    obj_create_f  create_f;
    _owner_t      owners[_MAX_OWNERS];
};
typedef struct _type_s _type_t, *_type_ptr;

struct _last_restock_s
{
    int turn;
    int level;
    int exp;
};
typedef struct _last_restock_s _last_restock_t;

struct shop_s
{
    _type_ptr     type;
    _owner_ptr    owner;
    inv_ptr       inv;
    _last_restock_t last_restock;
};

/************************************************************************
 * Shop Types and Their Owners (names originally from CthAngband?)
 ***********************************************************************/

static bool _general_will_buy(obj_ptr obj);
static bool _general_create(obj_ptr obj, u32b mode);
static bool _armory_will_buy(obj_ptr obj);
static bool _armory_create(obj_ptr obj, u32b mode);
static bool _weapon_will_buy(obj_ptr obj);
static bool _weapon_create(obj_ptr obj, u32b mode);
static bool _temple_will_buy(obj_ptr obj);
static bool _temple_create(obj_ptr obj, u32b mode);
static bool _alchemist_will_buy(obj_ptr obj);
static bool _alchemist_create(obj_ptr obj, u32b mode);
static bool _magic_will_buy(obj_ptr obj);
static bool _magic_create(obj_ptr obj, u32b mode);
static bool _black_market_will_buy(obj_ptr obj);
static bool _black_market_create(obj_ptr obj, u32b mode);
static bool _book_will_buy(obj_ptr obj);
static bool _book_create(obj_ptr obj, u32b mode);
static bool _jeweler_will_buy(obj_ptr obj);
static bool _jeweler_create(obj_ptr obj, u32b mode);
static bool _shroomery_will_buy(obj_ptr obj);
static bool _shroomery_create(obj_ptr obj, u32b mode);
static bool _dragon_will_buy(obj_ptr obj);
static bool _dragon_create(obj_ptr obj, u32b mode);

static _type_t _types[] = 
{
    { SHOP_GENERAL, "General Store", _general_will_buy, _general_create,
        {{  1, "Bilbo the Friendly",         200, 108, RACE_HOBBIT },
         {  2, "Rincewind the Chicken",      200, 108, RACE_HUMAN },
         {  3, "Snafu the Midget",           300, 107, RACE_GNOME },
         {  4, "Lyar-el the Comely",         300, 107, RACE_DEMIGOD },
         {  5, "Falilmawen the Friendly",    250, 108, RACE_HOBBIT },
         {  6, "Voirin the Cowardly",        500, 108, RACE_HUMAN },
         {  7, "Erashnak the Midget",        750, 107, RACE_BEASTMAN },
         {  8, "Grug the Comely",           1000, 107, RACE_HALF_TITAN },
         {  9, "Forovir the Cheap",          250, 108, RACE_HUMAN },
         { 10, "Ellis the Fool",             500, 108, RACE_HUMAN },
         { 11, "Filbert the Hungry",         750, 107, RACE_VAMPIRE },
         { 12, "Fthnargl Psathiggua",       1000, 107, RACE_MIND_FLAYER },
         { 13, "Eloise Long-Dead",           250, 108, RACE_SPECTRE },
         { 14, "Fundi the Slow",             500, 108, RACE_ZOMBIE },
         { 15, "Granthus",                   750, 107, RACE_SKELETON },
         { 16, "Lorax the Suave",           1000, 107, RACE_VAMPIRE },
         { 17, "Butch",                      250, 108, RACE_SNOTLING },
         { 18, "Elbereth the Beautiful",     500, 108, RACE_HIGH_ELF },
         { 19, "Sarleth the Sneaky",         750, 107, RACE_GNOME },
         { 20, "Narlock",                   1000, 107, RACE_DWARF },
         { 21, "Haneka the Small",           250, 108, RACE_GNOME },
         { 22, "Loirin the Mad",             500, 108, RACE_HALF_GIANT },
         { 23, "Wuto Poisonbreath",          750, 107, RACE_DRACONIAN },
         { 24, "Araaka the Rotund",         1000, 107, RACE_DRACONIAN },
         { 25, "Poogor the Dumb",            250, 108, RACE_BEASTMAN },
         { 26, "Felorfiliand",               500, 108, RACE_DEMIGOD },
         { 27, "Maroka the Aged",            750, 107, RACE_GNOME },
         { 28, "Sasin the Bold",            1000, 107, RACE_HALF_GIANT },
         { 29, "Abiemar the Peasant",        250, 108, RACE_HUMAN },
         { 30, "Hurk the Poor",              500, 108, RACE_SNOTLING },
         { 31, "Soalin the Wretched",        750, 107, RACE_ZOMBIE },
         { 32, "Merulla the Humble",        1000, 107, RACE_DEMIGOD }}},
        
    { SHOP_ARMORY, "Armory", _armory_will_buy, _armory_create,
        {{  1, "Kon-Dar the Ugly",         15000, 115, RACE_SNOTLING },
         {  2, "Darg-Low the Grim",        20000, 111, RACE_HUMAN },
         {  3, "Decado the Handsome",      25000, 112, RACE_DUNADAN },
         {  4, "Wieland the Smith",        40000, 112, RACE_DWARF },
         {  5, "Stumpy the Blockhead",     15000, 113, RACE_ENT },
         {  6, "Zeppo the Blacksmith",     25000, 111, RACE_HUMAN },
         {  7, "Beamy Break-Axe",          30000, 112, RACE_ENT },
         {  8, "Elo Dragonscale",          35000, 112, RACE_DEMIGOD },
         {  9, "Delicatus",                20000, 115, RACE_SPRITE },
         { 10, "Gruce the Huge",           20000, 111, RACE_HALF_GIANT },
         { 11, "Animus",                   25000, 112, RACE_GOLEM },
         { 12, "Malvus",                   30000, 112, RACE_HALF_TITAN },
         { 13, "Selaxis",                  15000, 115, RACE_ZOMBIE },
         { 14, "Deathchill",               20000, 111, RACE_SPECTRE },
         { 15, "Drios the Faint",          25000, 112, RACE_SPECTRE },
         { 16, "Bathric the Cold",         30000, 112, RACE_VAMPIRE },
         { 17, "Vengella the Cruel",       15000, 115, RACE_HALF_TROLL },
         { 18, "Wyrana the Mighty",        20000, 111, RACE_HUMAN },
         { 19, "Yojo II",                  25000, 112, RACE_DWARF },
         { 20, "Ranalar the Sweet",        30000, 112, RACE_AMBERITE },
         { 21, "Horbag the Unclean",       15000, 115, RACE_SNOTLING },
         { 22, "Elelen the Telepath",      25000, 111, RACE_DARK_ELF },
         { 23, "Isedrelias",               30000, 112, RACE_SPRITE },
         { 24, "Vegnar One-eye",           20000, 112, RACE_CYCLOPS },
         { 25, "Rodish the Chaotic",       20000, 115, RACE_BEASTMAN },
         { 26, "Hesin Swordmaster",        25000, 111, RACE_NIBELUNG },
         { 27, "Elvererith the Cheat",     15000, 112, RACE_DARK_ELF },
         { 28, "Zzathath the Imp",         30000, 112, RACE_IMP },
         { 0 }}},

    { SHOP_WEAPON, "Weapon Smiths", _weapon_will_buy, _weapon_create,
        {{  1, "Arnold the Beastly",       15000, 115, RACE_BARBARIAN },
         {  2, "Arndal Beast-Slayer",      20000, 110, RACE_HUMAN },
         {  3, "Eddie Beast-Master",       25000, 115, RACE_SNOTLING },
         {  4, "Oglign Dragon-Slayer",     50000, 112, RACE_DWARF },
         {  5, "Drew the Skilled",         20000, 115, RACE_HUMAN },
         {  6, "Orrax Dragonson",          20000, 110, RACE_DRACONIAN },
         {  7, "Anthrax Disease-Carrier",  25000, 115, RACE_BEASTMAN },
         {  8, "Arkhoth the Stout",        35000, 112, RACE_DWARF },
         {  9, "Sarlyas the Rotten",       15000, 115, RACE_ZOMBIE },
         { 10, "Tuethic Bare-Bones",       20000, 110, RACE_SKELETON },
         { 11, "Bilious",                  25000, 115, RACE_BEASTMAN },
         { 12, "Fasgul",                   30000, 112, RACE_ZOMBIE },
         { 13, "Ellefris the Paladin",     15000, 115, RACE_BARBARIAN },
         { 14, "K'trrik'k",                20000, 110, RACE_KLACKON },
         { 15, "Drocus Spiderfriend",      25000, 115, RACE_DARK_ELF },
         { 16, "Fungus Giant-Slayer",      40000, 112, RACE_DWARF },
         { 17, "Delantha",                 20000, 115, RACE_DEMIGOD },
         { 18, "Solvistani the Ranger",    25000, 110, RACE_WOOD_ELF },
         { 19, "Xoril the Slow",           30000, 115, RACE_GOLEM },
         { 20, "Aeon Flux",                25000, 112, RACE_TONBERRY },
         { 21, "Nadoc the Strong",         20000, 115, RACE_HOBBIT },
         { 22, "Eramog the Weak",          25000, 110, RACE_KOBOLD },
         { 23, "Eowilith the Fair",        30000, 115, RACE_VAMPIRE },
         { 24, "Huimog Balrog-Slayer",     30000, 112, RACE_SNOTLING },
         { 25, "Peadus the Cruel",         15000, 115, RACE_HUMAN },
         { 26, "Vamog Slayer",             20000, 110, RACE_OGRE },
         { 27, "Hooshnak the Vicious",     25000, 115, RACE_BEASTMAN },
         { 28, "Balenn War-Dancer",        30000, 112, RACE_BARBARIAN },
         { 0 }}},

    { SHOP_TEMPLE, "Temple", _temple_will_buy, _temple_create,
        {{  1, "Ludwig the Humble",         5000, 109, RACE_DWARF },
         {  2, "Gunnar the Paladin",       10000, 110, RACE_HALF_TROLL },
         {  3, "Torin the Chosen",         25000, 107, RACE_HIGH_ELF },
         {  4, "Sarastro the Wise",        30000, 109, RACE_HUMAN },
         {  5, "Sir Parsival the Pure",    25000, 107, RACE_HIGH_ELF },
         {  6, "Asenath the Holy",         30000, 109, RACE_HUMAN },
         {  7, "McKinnon",                 10000, 109, RACE_HUMAN },
         {  8, "Mistress Chastity",        15000, 110, RACE_HIGH_ELF },
         {  9, "Hashnik the Druid",        25000, 107, RACE_HOBBIT },
         { 10, "Finak",                    30000, 109, RACE_YEEK },
         { 11, "Krikkik",                  10000, 109, RACE_KLACKON },
         { 12, "Morival the Wild",         15000, 110, RACE_DEMIGOD },
         { 13, "Hoshak the Dark",          25000, 107, RACE_IMP },
         { 14, "Atal the Wise",            30000, 109, RACE_HUMAN },
         { 15, "Ibenidd the Chaste",       10000, 109, RACE_HUMAN },
         { 16, "Eridish",                  15000, 110, RACE_HALF_TROLL },
         { 17, "Vrudush the Shaman",       25000, 107, RACE_OGRE },
         { 18, "Haob the Berserker",       30000, 109, RACE_BARBARIAN },
         { 19, "Proogdish the Youthfull",  10000, 109, RACE_OGRE },
         { 20, "Lumwise the Mad",          15000, 110, RACE_YEEK },
         { 21, "Muirt the Virtuous",       25000, 107, RACE_KOBOLD },
         { 22, "Dardobard the Weak",       30000, 109, RACE_SPECTRE },
         { 0 }}},

    { SHOP_ALCHEMIST, "Alchemy Shop", _alchemist_will_buy, _alchemist_create,
        {{  1, "Mauser the Chemist",       10000, 111, RACE_HUMAN },
         {  2, "Wizzle the Chaotic",       10000, 110, RACE_HOBBIT },
         {  3, "Midas the Greedy",         15000, 116, RACE_GNOME },
         {  4, "Ja-Far the Alchemist",     15000, 111, RACE_DEMIGOD },
         {  5, "Kakalrakakal",             15000, 116, RACE_KLACKON },
         {  6, "Jal-Eth the Alchemist",    15000, 111, RACE_DEMIGOD },
         {  7, "Fanelath the Cautious",    10000, 111, RACE_DWARF },
         {  8, "Runcie the Insane",        10000, 110, RACE_HUMAN },
         {  9, "Grumbleworth",             15000, 116, RACE_GNOME },
         { 10, "Flitter",                  15000, 111, RACE_SPRITE },
         { 11, "Xarillus",                 10000, 111, RACE_HUMAN },
         { 12, "Egbert the Old",           10000, 110, RACE_DWARF },
         { 13, "Valindra the Proud",       15000, 116, RACE_HIGH_ELF },
         { 14, "Taen the Alchemist",       15000, 111, RACE_HUMAN },
         { 15, "Cayd the Sweet",           10000, 111, RACE_VAMPIRE },
         { 16, "Fulir the Dark",           10000, 110, RACE_NIBELUNG },
         { 17, "Domli the Humble",         15000, 116, RACE_DWARF },
         { 18, "Yaarjukka Demonspawn",     15000, 111, RACE_IMP },
         { 19, "Gelaraldor the Herbmaster",10000, 111, RACE_HIGH_ELF },
         { 20, "Olelaldan the Wise",       10000, 110, RACE_BARBARIAN },
         { 21, "Fthoglo the Demonicist",   15000, 116, RACE_IMP },
         { 22, "Dridash the Alchemist",    15000, 111, RACE_SNOTLING },
         { 23, "Nelir the Strong",         10000, 111, RACE_CYCLOPS },
         { 24, "Lignus the Pungent",       10000, 110, RACE_SNOTLING },
         { 25, "Tilba",                    15000, 116, RACE_HOBBIT },
         { 26, "Myrildric the Wealthy",    15000, 111, RACE_HUMAN },
         { 27, "Igor the Mixturemaker",    15000, 112, RACE_IGOR },
         { 0 }}},

    { SHOP_MAGIC, "Magic Shop", _magic_will_buy, _magic_create,
        {{  1, "Lo Pan the Sorcerer",      20000, 110, RACE_HUMAN },
         {  2, "Buggerby the Great",       20000, 113, RACE_GNOME },
         {  3, "The Wizard of Yendor",     30000, 110, RACE_HUMAN },
         {  4, "Rjak the Necromancer",     30000, 110, RACE_DARK_ELF },
         {  5, "Skidney the Sorcerer",     15000, 110, RACE_HUMAN },
         {  6, "Kyria the Illusionist",    30000, 110, RACE_HUMAN },
         {  7, "Nikki the Necromancer",    30000, 110, RACE_DARK_ELF },
         {  8, "Solostoran",               15000, 110, RACE_SPRITE },
         {  9, "Achshe the Tentacled",     20000, 113, RACE_MIND_FLAYER },
         { 10, "Kaza the Noble",           30000, 110, RACE_HIGH_ELF },
         { 11, "Fazzil the Dark",          30000, 110, RACE_DARK_ELF },
         { 12, "Keldorn the Grand",        15000, 110, RACE_DWARF },
         { 13, "Philanthropus",            20000, 113, RACE_HOBBIT },
         { 14, "Agnar the Enchantress",    30000, 110, RACE_HUMAN },
         { 15, "Buliance the Necromancer", 30000, 110, RACE_BEASTMAN },
         { 16, "Vuirak the High-Mage",     15000, 110, RACE_BEASTMAN },
         { 17, "Madish the Smart",         20000, 113, RACE_BEASTMAN },
         { 18, "Falebrimbor",              30000, 110, RACE_HIGH_ELF },
         { 19, "Felil-Gand the Subtle",    30000, 110, RACE_DARK_ELF },
         { 20, "Thalegord the Shaman",     15000, 110, RACE_BARBARIAN },
         { 21, "Cthoaloth the Mystic",     20000, 113, RACE_MIND_FLAYER },
         { 22, "Ibeli the Illusionist",    30000, 110, RACE_SKELETON },
         { 23, "Heto the Necromancer",     30000, 110, RACE_YEEK },
         { 24, "Nils the Nifty",           30000, 110, RACE_TOMTE },
         { 0 }}},

    { SHOP_BLACK_MARKET, "Black Market", _black_market_will_buy, _black_market_create,
        {{  1, "Gary Gygaz",               20000, 150, RACE_HALF_TROLL },
         {  2, "Histor the Goblin",        20000, 150, RACE_SNOTLING },
         {  3, "Quark the Ferengi",        30000, 150, RACE_DWARF },
         {  4, "Topi the Fair(?)",         30000, 150, RACE_HUMAN },
         {  5, "Vhassa the Dead",          20000, 150, RACE_ZOMBIE },
         {  6, "Kyn the Treacherous",      20000, 150, RACE_VAMPIRE },
         {  7, "Bubonicus",                30000, 150, RACE_BEASTMAN },
         {  8, "Corpselight",              30000, 150, RACE_SPECTRE },
         {  9, "Parrish the Bloodthirsty", 20000, 150, RACE_VAMPIRE },
         { 10, "Vile",                     20000, 150, RACE_SKELETON },
         { 11, "Prentice the Trusted",     30000, 150, RACE_SKELETON },
         { 12, "Griella Humanslayer",      30000, 150, RACE_IMP },
         { 13, "Angel",                    20000, 150, RACE_VAMPIRE },
         { 14, "Flotsam the Bloated",      20000, 150, RACE_ZOMBIE },
         { 15, "Nieval",                   30000, 150, RACE_VAMPIRE },
         { 16, "Anastasia the Luminous",   30000, 150, RACE_SPECTRE },
         { 17, "Charity the Necromancer",  20000, 150, RACE_DARK_ELF },
         { 18, "Pugnacious the Pugilist",  20000, 150, RACE_SNOTLING },
         { 19, "Footsore the Lucky",       30000, 150, RACE_BEASTMAN },
         { 20, "Sidria Lightfingered",     30000, 150, RACE_HUMAN },
         { 21, "Riatho the Juggler",       20000, 150, RACE_HOBBIT },
         { 22, "Janaaka the Shifty",       20000, 150, RACE_GNOME },
         { 23, "Cina the Rogue",           30000, 150, RACE_GNOME },
         { 24, "Arunikki Greatclaw",       30000, 150, RACE_DRACONIAN },
         { 25, "Chaeand the Poor",         20000, 150, RACE_HUMAN },
         { 26, "Afardorf the Brigand",     20000, 150, RACE_BARBARIAN },
         { 27, "Lathaxl the Greedy",       30000, 150, RACE_MIND_FLAYER },
         { 28, "Falarewyn",                30000, 150, RACE_SPRITE },
         { 29, "Vosur the Wrinkled",       20000, 150, RACE_NIBELUNG },
         { 30, "Araord the Handsome",      20000, 150, RACE_AMBERITE },
         { 31, "Theradfrid the Loser",     30000, 150, RACE_HUMAN },
         { 32, "One-Legged Eroolo",        30000, 150, RACE_OGRE }}},

    { SHOP_BOOK, "Bookstore", _book_will_buy, _book_create,
        {{  1, "Dolaf the Greedy",         10000, 108, RACE_HUMAN },
         {  2, "Odnar the Sage",           15000, 105, RACE_HIGH_ELF },
         {  3, "Gandar the Neutral",       25000, 110, RACE_DARK_ELF },
         {  4, "Ro-sha the Patient",       30000, 105, RACE_DEMIGOD },
         {  5, "Randolph Carter",          15000, 108, RACE_HUMAN },
         {  6, "Sarai the Swift",          15000, 108, RACE_HUMAN },
         {  7, "Bodril the Seer",          20000, 105, RACE_HIGH_ELF },
         {  8, "Veloin the Quiet",         25000, 110, RACE_ZOMBIE },
         {  9, "Vanthylas the Learned",    30000, 105, RACE_MIND_FLAYER },
         { 10, "Ossein the Literate",      15000, 108, RACE_SKELETON },
         { 11, "Olvar Bookworm",           20000, 105, RACE_VAMPIRE },
         { 12, "Shallowgrave",             25000, 110, RACE_ZOMBIE },
         { 13, "Death Mask",               30000, 105, RACE_ZOMBIE },
         { 14, "Asuunu the Learned",       15000, 108, RACE_MIND_FLAYER },
         { 15, "Prirand the Dead",         20000, 105, RACE_ZOMBIE },
         { 16, "Ronar the Iron",           25000, 110, RACE_GOLEM },
         { 17, "Galil-Gamir",              35000, 105, RACE_HIGH_ELF },
         { 18, "Rorbag Book-Eater",         5000, 108, RACE_KOBOLD },
         { 19, "Kiriarikirk",              20000, 105, RACE_KLACKON },
         { 20, "Rilin the Quiet",          25000, 110, RACE_DWARF },
         { 21, "Isung the Lord",           30000, 105, RACE_HIGH_ELF },
         { 0 }}},

    { SHOP_JEWELER, "Jewelry Shop", _jeweler_will_buy, _jeweler_create,
        {{  1, "Dalanna the Sweet",        20000, 108, RACE_HUMAN },
         {  2, "Mesistrond",               15000, 105, RACE_DARK_ELF },
         {  3, "Mr. Biggles",              50000, 110, RACE_GNOME },
         {  4, "Snivelsby",                10000, 108, RACE_SNOTLING },
         {  5, "Grug",                     10000, 110, RACE_HALF_TROLL },
         {  6, "Raphaella",                35000, 105, RACE_ARCHON },
         {  7, "Sylphrana Lightfoot",      25000, 105, RACE_SPRITE },
         {  8, "Helen the Beautiful",      20000, 110, RACE_DEMIGOD },
         {  9, "Rattles Neverborn",        10000, 112, RACE_SKELETON },
         { 10, "Trinkles the Stinky",      30000, 110, RACE_GNOME },
         { 11, "Gaudella",                 15000, 105, RACE_HUMAN },
         { 12, "Argwynna of the Wood",     40000, 105, RACE_WOOD_ELF },
         { 13, "Mugbasha",                  5000, 120, RACE_KOBOLD },
         { 14, "Billing",                  25000, 113, RACE_DWARF },
         { 15, "Regen",                    20000, 111, RACE_NIBELUNG },
         { 16, "Aulendil",                 25000, 110, RACE_HIGH_ELF },
         { 0 }}},

	{ SHOP_SHROOMERY, "Mushroom Store", _shroomery_will_buy, _shroomery_create,
		 { { 1, "Mysticus",              50000, 110, RACE_GNOME },
		 { 2, "Martin",                10000, 108, RACE_HUMAN },
		 { 3, "Karl",                     10000, 110, RACE_HALF_TROLL },
		 { 4, "Mycella",				25000, 105, RACE_SPRITE },
		 { 5, "Gordo",					20000, 110, RACE_HOBBIT },
		 { 6, "Agaria",					40000, 105, RACE_WOOD_ELF },
		 { 7, "Dumush",                  5000, 120, RACE_KOBOLD },
		 { 0 } } },

	{ SHOP_DRAGON, "Dragonskin Emporium", _dragon_will_buy, _dragon_create,
		 { { 1, "Beowulf",              50000, 110, RACE_HUMAN },
		 { 2, "George",                10000, 108, RACE_HUMAN },
		 { 3, "Sigurd",                     10000, 110, RACE_HUMAN },
		 { 4, "Conan",				25000, 105, RACE_BARBARIAN },
		 { 0 } } },

    { SHOP_NONE }
};

static _type_ptr _get_type(int which)
{
    int i;
    for (i = 0;; i++)
    {
        _type_ptr type = &_types[i];
        if (type->id == SHOP_NONE) return NULL;
        if (type->id == which) return type;
    }
}

static bool _shop_is_basic(shop_ptr shop)
{
    switch (shop->type->id)
    {
    case SHOP_BLACK_MARKET:
    case SHOP_JEWELER:
	case SHOP_DRAGON:
        return FALSE;
    }
    return TRUE;
}

static _owner_ptr _get_owner(_type_ptr type, int which)
{
    int i;
    for (i = 0; i < _MAX_OWNERS; i++)
    {
        _owner_ptr owner = &type->owners[i];
        if (!owner->name) break;
        if (owner->id == which) return owner;
    }
    return NULL;
}

static int _count_owners(_type_ptr type)
{
    int ct = 0;
    int i;
    for (i = 0; i < _MAX_OWNERS; i++)
    {
        if (!type->owners[i].name) break;
        ct++;
    }
    return ct;
}

static bool _will_buy(obj_ptr obj)
{
    if (obj_value(obj) <= 0) return FALSE;
    if (obj->tval == TV_CORPSE) return FALSE;
    return TRUE;
}

static bool _stock_p(int k_idx)
{
    if (k_info[k_idx].gen_flags & OFG_INSTA_ART)
        return FALSE;

    if (!dun_level && p_ptr->town_num != TOWN_ZUL)
    {
        if (!(k_info[k_idx].gen_flags & OFG_TOWN))
            return FALSE;
    }
    return TRUE;
}

static int _get_k_idx(_k_idx_p p, int lvl)
{
    int k_idx;
    if (p)
    {
        get_obj_num_hook = p;
        get_obj_num_prep();
    }
    k_idx = get_obj_num(lvl);
    if (p)
    {
        get_obj_num_hook = NULL;
        get_obj_num_prep();
    }
    return k_idx;
}

static int _mod_lvl(int lvl)
{
    if (dun_level > lvl)
        return (dun_level - lvl)/3 + lvl;
    return lvl;
}

static void _discount(obj_ptr obj)
{
    int discount = 0;
    int cost = obj_value(obj);

    if (cost < 5)          discount = 0;
    else if (one_in_(25))  discount = 25;
    else if (one_in_(150)) discount = 50;
    else if (one_in_(300)) discount = 75;
    else if (one_in_(500)) discount = 90;

    if (object_is_artifact(obj))
        discount = 0;

    obj->discount = discount;
}

static bool _create(obj_ptr obj, int k_idx, int lvl, u32b mode)
{
    if (!k_idx) return FALSE;
    if (mode & AM_SHUFFLING)
    {
     if (k_info[k_idx].gen_flags & OFG_NO_SHUFFLE) return FALSE;
    }

    object_prep(obj, k_idx);
    apply_magic(obj, lvl, mode);
    if (obj->tval == TV_LITE)
	{
		if (obj->sval == SV_LITE_TORCH) obj->xtra4 = FUEL_TORCH;
		if (obj->sval == SV_LITE_LANTERN) obj->xtra4 = FUEL_LAMP;
	}

    if (object_is_cursed(obj)) return FALSE;

    obj->ident |= IDENT_STORE;
    if (obj_value(obj) <= 0) return FALSE; /* Note: requires IDENT_STORE to work!!! */

    if (mode & AM_SHUFFLING) /* Prevent shuffling for a [Sm amulet */
    {
        if ((k_info[k_idx].tval == TV_AMULET) && (have_flag(obj->flags, OF_NO_SUMMON))) return FALSE;
        if ((object_is_mushroom(obj)) && ((prace_is_(RACE_SNOTLING)) || (p_ptr->prace == RACE_SNOTLING) || (p_ptr->prace == RACE_DOPPELGANGER))) return FALSE;
    }

    /* discounts could screw up the special pricing in dragonskin emporium */
    if (k_info[k_idx].tval != TV_DRAG_ARMOR) _discount(obj);

    obj_make_pile(obj);
    return TRUE;
}

/************************************************************************
 * The General Store
 ***********************************************************************/
static bool _general_will_buy(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_POTION:
        if (obj->sval != SV_POTION_WATER) return FALSE;
    case TV_WHISTLE:
    case TV_FOOD:
    case TV_LITE:
    case TV_FLASK:
    case TV_SPIKE:
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_DIGGING:
    case TV_CLOAK:
    case TV_BOTTLE: /* 'Green', recycling Angband */
    case TV_FIGURINE:
    case TV_STATUE:
    case TV_CAPTURE:
    case TV_CARD:
        break;
    default:
        return FALSE;
    }
    return _will_buy(obj);
}

static bool _general_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;

    switch (k_info[k_idx].tval)
    {
    case TV_FLASK:
    case TV_SPIKE:
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_CAPTURE:
    case TV_FIGURINE:
    case TV_CLOAK:
    case TV_LITE:
    case TV_FOOD:
    case TV_DIGGING:
        return TRUE;
    }
    return FALSE;
}

static bool _stock_ammo_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;
    switch (k_info[k_idx].tval)
    {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
        return TRUE;
    }
    return FALSE;
}

static bool _general_create(obj_ptr obj, u32b mode)
{
    int k_idx;
    if (one_in_(50))
        k_idx = lookup_kind(TV_CAPTURE, 0);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_FOOD, SV_FOOD_RATION);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_POTION, SV_POTION_WATER);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_FLASK, SV_FLASK_OIL);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_LITE, one_in_(2) ? SV_LITE_LANTERN : SV_LITE_TORCH);
    else if (one_in_(3))
        k_idx = _get_k_idx(_stock_ammo_p, _mod_lvl(10));
    else if (one_in_(3))
        k_idx = lookup_kind(TV_SPIKE, SV_ANY);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_DIGGING, SV_SHOVEL);
    else if (one_in_(5))
        k_idx = lookup_kind(TV_DIGGING, SV_PICK);
    else
        k_idx = _get_k_idx(_general_stock_p, _mod_lvl(20));
    return _create(obj, k_idx, _mod_lvl(rand_range(1, 15)), mode);
}

/************************************************************************
 * The Armory
 ***********************************************************************/
static bool _armory_will_buy(obj_ptr obj)
{
    return obj_is_armor(obj) && _will_buy(obj);
}

static bool _armory_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;

    switch (k_info[k_idx].tval)
    {
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
    case TV_DRAG_ARMOR:
    case TV_GLOVES:
    case TV_HELM:
    case TV_BOOTS:
    case TV_SHIELD:
        return TRUE;
    }
    return FALSE;
}

static bool _armory_create(obj_ptr obj, u32b mode)
{
    int k_idx = _get_k_idx(_armory_stock_p, _mod_lvl(20));
    return _create(obj, k_idx, _mod_lvl(rand_range(1, 15)), mode);
}

/************************************************************************
 * The Weapon Smiths
 ***********************************************************************/
static bool _weapon_will_buy(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_SHOT:
    case TV_BOLT:
    case TV_ARROW:
    case TV_BOW:
    case TV_QUIVER:
    case TV_DIGGING:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_HISSATSU_BOOK:
    case TV_RAGE_BOOK:
        break;
    case TV_HAFTED:
        if(obj->sval == SV_WIZSTAFF) return FALSE;
        break;
    default:
        return FALSE;
    }
    return _will_buy(obj);
}

static bool _weapon_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;

    switch (k_info[k_idx].tval)
    {
    case TV_POLEARM:
    case TV_SWORD:
        return TRUE;
    }
    return FALSE;
}
static bool _weapon_book_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;

    switch (k_info[k_idx].tval)
    {
    case TV_HISSATSU_BOOK:
    case TV_RAGE_BOOK:
        return TRUE;
    }
    return FALSE;
}
static bool _weapon_stock_shooter_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;
    switch (k_info[k_idx].tval)
    {
    case TV_BOW:
        return TRUE;
    }
    return FALSE;
}
static bool _weapon_create(obj_ptr obj, u32b mode)
{
    int k_idx;
    int l1 = _mod_lvl(20);
    int l2 = _mod_lvl(rand_range(1, 15));
    if ((p_ptr->lev < 12) && (one_in_(3)))
    {
        static byte vuoro = 0;
        switch (vuoro % 3)
        {
            case 0: k_idx = lookup_kind(TV_SHOT, SV_PEBBLE); break;
            case 1: k_idx = lookup_kind(TV_ARROW, SV_ARROW); break;
            default: k_idx = lookup_kind(TV_BOLT, SV_BOLT); break;
        }
        if (vuoro < 100) vuoro++;
        else vuoro = 0;
    }
    else if (one_in_(3))
        k_idx = _get_k_idx(_weapon_book_p, l1);
    else if (one_in_(4))
        k_idx = _get_k_idx(_weapon_stock_shooter_p, l1);
    else if (one_in_((p_ptr->lev < 12 ? 5 : 3)))
        k_idx = _get_k_idx(_stock_ammo_p, l1);
    else if (one_in_(10))
        k_idx = lookup_kind(TV_QUIVER, 0);
    else
        k_idx = _get_k_idx(_weapon_stock_p, l1);
    if (!_create(obj, k_idx, l2, mode)) return FALSE;
    if ((object_is_ammo(obj)) && (p_ptr->lev < 12))
    {
        if ((obj->to_d > 0) && (!obj->name2) && (!one_in_(3)))
        {
            obj->to_h = 0;
            obj->to_d = 0;
        }
        obj->number -= (obj->number / 3);
    }
	if (obj->to_a < 0 || obj->to_h < 0)
	{
		if (object_is_ammo(obj) && (!obj->name2))
		{
			obj->to_h = 0;
			obj->to_d = 0;
			obj->curse_flags = 0;
			obj->known_curse_flags = 0;
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

/************************************************************************
 * The Temple
 ***********************************************************************/
static bool _temple_will_buy(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_LIFE_BOOK:
    case TV_CRUSADE_BOOK:
    case TV_SCROLL:
    case TV_POTION:
    case TV_HAFTED:
        break;
    case TV_FIGURINE:
    case TV_STATUE: {
        monster_race *r_ptr = &r_info[obj->pval];

        if (!(r_ptr->flags3 & RF3_EVIL))
        {
            if (r_ptr->flags3 & RF3_GOOD) break;
            if (r_ptr->flags3 & RF3_ANIMAL) break;
            if (strchr("?!", r_ptr->d_char)) break; /* mimics?? */
        }
        return FALSE; }
    case TV_POLEARM:
    case TV_SWORD:
        if (obj_is_blessed(obj)) break;
        return FALSE;
    default:
        return FALSE;
    }
    return _will_buy(obj);
}

static bool _temple_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;

    switch (k_info[k_idx].tval)
    {
    case TV_LIFE_BOOK:
    case TV_CRUSADE_BOOK:
        return TRUE;

    case TV_HAFTED:
        return TRUE;

    /* Scrolls and Potions are also stocked by the Alchemist */
    case TV_SCROLL:
        switch (k_info[k_idx].sval)
        {
        case SV_SCROLL_REMOVE_CURSE:
        case SV_SCROLL_BLESSING:
        case SV_SCROLL_HOLY_CHANT:
        case SV_SCROLL_WORD_OF_RECALL:
        case SV_SCROLL_STAR_REMOVE_CURSE:
        case SV_SCROLL_RUNE_OF_PROTECTION:
            return TRUE;
        }
        return FALSE;

    case TV_POTION:
        switch (k_info[k_idx].sval)
        {
        case SV_POTION_RESTORE_EXP:
        case SV_POTION_CURE_CRITICAL:
        case SV_POTION_CURE_SERIOUS:
        case SV_POTION_CURE_LIGHT:
        case SV_POTION_BOLDNESS:
        case SV_POTION_HEROISM:
        case SV_POTION_HEALING:
        case SV_POTION_STAR_HEALING:
        case SV_POTION_LIFE:
        case SV_POTION_CURING:
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

static bool _temple_create(obj_ptr obj, u32b mode)
{
    int k_idx;
    if (one_in_(4))
        k_idx = lookup_kind(TV_SCROLL, SV_SCROLL_WORD_OF_RECALL);
    else if (one_in_(7))
        k_idx = lookup_kind(TV_SCROLL, SV_SCROLL_REMOVE_CURSE);
    else if (one_in_(20))
        k_idx = lookup_kind(TV_SCROLL, SV_SCROLL_STAR_REMOVE_CURSE);
    else if (((p_ptr->lev < 40) || (no_wilderness)) && (one_in_(10)))
    {
        if (p_ptr->lev < 20) k_idx = lookup_kind(TV_POTION, SV_POTION_CURE_SERIOUS);
        else if (p_ptr->lev < 35) k_idx = lookup_kind(TV_POTION, SV_POTION_CURING);
        else k_idx = lookup_kind(TV_POTION, SV_POTION_HEROISM);
    }
    else if ((no_wilderness) && (p_ptr->lev > 20) && (one_in_(14)))
    {
        k_idx = lookup_kind(TV_POTION, SV_POTION_RESTORE_EXP);
    }
    else
        k_idx = _get_k_idx(_temple_stock_p, _mod_lvl(20));
    return _create(obj, k_idx, _mod_lvl(rand_range(1, 15)), mode);
}

/************************************************************************
 * Shops
 ***********************************************************************/
static bool _alchemist_will_buy(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_POTION:
    case TV_SCROLL:
        break;
    default:
        return FALSE;
    }
    return _will_buy(obj);
}

static bool _alchemist_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;

    switch (k_info[k_idx].tval)
    {
    /* Scrolls and Potions are also stocked by the Temple. */
    case TV_SCROLL:
    case TV_POTION:
        if (!_temple_stock_p(k_idx))
            return TRUE;
        break;
    }
    return FALSE;
}

static bool _alchemist_create(obj_ptr obj, u32b mode)
{
    int k_idx;
    if (one_in_(4))
        k_idx = lookup_kind(TV_SCROLL, SV_SCROLL_WORD_OF_RECALL);
    else if ((one_in_(22)) && (p_ptr->lev > 1))
        k_idx = lookup_kind(TV_SCROLL, SV_SCROLL_TELEPORT);
    else if ((one_in_(22)) && (p_ptr->lev > 1))
        k_idx = lookup_kind(TV_SCROLL, SV_SCROLL_PHASE_DOOR);
    else
        k_idx = _get_k_idx(_alchemist_stock_p, _mod_lvl(20));
    return _create(obj, k_idx, _mod_lvl(rand_range(1, 15)), mode);
}

/************************************************************************
 * The Magic Shop
 ***********************************************************************/
static bool _magic_will_buy(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_SORCERY_BOOK:
    case TV_NATURE_BOOK:
    case TV_CHAOS_BOOK:
    case TV_ARMAGEDDON_BOOK:
    case TV_DEATH_BOOK:
    case TV_TRUMP_BOOK:
    case TV_ARCANE_BOOK:
    case TV_CRAFT_BOOK:
    case TV_DAEMON_BOOK:
    case TV_MUSIC_BOOK:
    case TV_HEX_BOOK:
    case TV_AMULET:
    case TV_RING:
    case TV_STAFF:
    case TV_WAND:
    case TV_ROD:
    case TV_SCROLL:
    case TV_POTION:
    case TV_FIGURINE:
        break;
    case TV_HAFTED:
        if(obj->sval == SV_WIZSTAFF) break;
        else return FALSE;
    case TV_LITE:
        return obj->name1 == ART_STONE_OF_SORCERY;
    default:
        return FALSE;
    }
    return _will_buy(obj);
}

static bool _magic_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;
    switch (k_info[k_idx].tval)
    {
    case TV_WAND:
    case TV_STAFF:
    case TV_FIGURINE:
    case TV_ARCANE_BOOK:
    case TV_SORCERY_BOOK:
        return TRUE;
    }
    return FALSE;
}

static bool _magic_create(obj_ptr obj, u32b mode)
{
    int k_idx;
    if (one_in_(20))
    {
        /* Hack: Early resists are hard to find, and Archviles are so damn nasty!
           BTW, since we are cheating and not using normal ego generation code, we'll
           need to manually add to ego_type.xtra_flags. This will improve the
           player's lore experience should they purchase or examine this item
           of stock. */
        if (one_in_(5))
        {
            object_prep(obj, lookup_kind(TV_AMULET, 0));
            obj->name2 = EGO_JEWELRY_ELEMENTAL;
        }
        else
        {
            object_prep(obj, lookup_kind(TV_RING, 0));
            obj->name2 = EGO_JEWELRY_ELEMENTAL;
        }
        switch (randint1(5))
        {
        case 1: case 2:
            add_flag(obj->flags, OF_RES_COLD);
            add_flag(e_info[EGO_JEWELRY_ELEMENTAL].xtra_flags, OF_RES_COLD);
            break;
        case 3: case 4:
            add_flag(obj->flags, OF_RES_FIRE);
            add_flag(e_info[EGO_JEWELRY_ELEMENTAL].xtra_flags, OF_RES_FIRE);
            break;
        case 5:
            add_flag(obj->flags, OF_RES_ACID);
            add_flag(e_info[EGO_JEWELRY_ELEMENTAL].xtra_flags, OF_RES_ACID);
            break;
        }
        obj->ident |= IDENT_STORE;
        return TRUE;
    }
    else
        k_idx = _get_k_idx(_magic_stock_p, _mod_lvl(20));
    return _create(obj, k_idx, _mod_lvl(15), mode);
}

/************************************************************************
 * The Black Market
 ***********************************************************************/
static bool _black_market_will_buy(obj_ptr obj)
{
    return _will_buy(obj);
}

static bool _black_market_stock_p(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    if (k_ptr->gen_flags & OFG_INSTA_ART) return FALSE;

    switch (k_ptr->tval)
    {
    case TV_CHEST: return FALSE;
    case TV_WAND:
    case TV_ROD:
    case TV_STAFF: return one_in_(3);
    }

    return !(k_ptr->gen_flags & OFG_TOWN);
}

static bool _black_market_create(obj_ptr obj, u32b mode)
{
    int spread = py_prorata_level(50);
    int l1 = 20 + spread/4 + randint0(3*spread/4);
    int l2 = 20 + spread/4 + randint0(3*spread/4);
    int k_idx;

    if (one_in_(9))
    {
        k_idx = lookup_kind(TV_BURGLARY_BOOK, randint0(2));
    }
    else
    {
        for (;;)
        {
            choose_obj_kind(0);
            k_idx = _get_k_idx(get_obj_num_hook, _mod_lvl(l1));
            if (_black_market_stock_p(k_idx)) break;
        }
    }
    if (!_create(obj, k_idx, _mod_lvl(l2), mode)) return FALSE;
    if (obj_value(obj) < 10) return FALSE;
    if (object_is_nameless(obj) && !object_is_rare(obj) && object_is_wearable(obj))
    {
        if (obj->to_a <= 0 && obj->to_h <= 0 && obj->to_d <= 0)
            return FALSE;
    }
    return TRUE;
}

/************************************************************************
 * The Bookstore
 ***********************************************************************/
static bool _book_will_buy(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_SORCERY_BOOK:
    case TV_NATURE_BOOK:
    case TV_CHAOS_BOOK:
    case TV_DEATH_BOOK:
    case TV_LIFE_BOOK:
    case TV_TRUMP_BOOK:
    case TV_ARCANE_BOOK:
    case TV_CRAFT_BOOK:
    case TV_DAEMON_BOOK:
    case TV_CRUSADE_BOOK:
    case TV_NECROMANCY_BOOK:
    case TV_ARMAGEDDON_BOOK:
    case TV_LAW_BOOK:
    case TV_MUSIC_BOOK:
    case TV_HEX_BOOK:
        break;
    default:
        return FALSE;
    }
    return _will_buy(obj);
}

static bool _book_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;
    switch (k_info[k_idx].tval)
    {
    case TV_ARCANE_BOOK:
    case TV_SORCERY_BOOK:
    case TV_NATURE_BOOK:
    case TV_CHAOS_BOOK:
    case TV_DEATH_BOOK:
    case TV_TRUMP_BOOK:
    case TV_CRAFT_BOOK:
    case TV_DAEMON_BOOK:
    case TV_LAW_BOOK:
    case TV_MUSIC_BOOK:
    case TV_HEX_BOOK:
    case TV_NECROMANCY_BOOK:
    case TV_ARMAGEDDON_BOOK:
        return TRUE;
    }
    return FALSE;
}

static bool _book_create(obj_ptr obj, u32b mode)
{
    int k_idx = _get_k_idx(_book_stock_p, _mod_lvl(20));
    return _create(obj, k_idx, _mod_lvl(rand_range(1, 15)), mode);
}

/************************************************************************
 * The Jeweler
 ***********************************************************************/
static bool _jeweler_will_buy(obj_ptr obj)
{
    if (obj->tval != TV_RING && obj->tval != TV_AMULET) return FALSE;
    return _will_buy(obj);
}

static bool _jeweler_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;
    switch (k_info[k_idx].tval)
    {
    case TV_RING:
    case TV_AMULET:
        return TRUE;
    }
    return FALSE;
}

static bool _jeweler_create(obj_ptr obj, u32b mode)
{
    int spread = py_prorata_level(50);
    int l1 = 20 + spread/4 + randint0(3*spread/4);
    int l2 = 20 + spread/4 + randint0(3*spread/4);
    int k_idx = _get_k_idx(_jeweler_stock_p, _mod_lvl(l1));
    bool okei = _create(obj, k_idx, _mod_lvl(l2), mode);
    if ((okei) && (object_is_artifact(obj))) return (one_in_(6)); /* too many artifacts in jewelry shop */
    return okei;
}

/************************************************************************
* The Mushroomery
***********************************************************************/

static bool _shroomery_will_buy(obj_ptr obj)
{
	if (obj->tval != TV_FOOD) return FALSE;
	return _will_buy(obj);
}

static bool _shroomery_stock_p(int k_idx)
{
	if (!_stock_p(k_idx))
		return FALSE;
	switch (k_info[k_idx].tval)
	{
	case TV_FOOD:
		if (k_info[k_idx].sval<SV_FOOD_MAX_MUSHROOM)
			return TRUE;
	}
	return FALSE;
}

static bool _shroomery_create(obj_ptr obj, u32b mode)
{
	int k_idx = _get_k_idx(_shroomery_stock_p, _mod_lvl(20));
	return _create(obj, k_idx, _mod_lvl(rand_range(1, 15)), mode);
}

/************************************************************************
* The Dragon Emporium
***********************************************************************/

static bool _dragon_will_buy(obj_ptr obj)
{
	return obj_is_armor(obj) && _will_buy(obj);
}

static bool _dragon_stock_p(int k_idx)
{
	if (!_stock_p(k_idx))
		return FALSE;
	if (k_info[k_idx].tval == TV_DRAG_ARMOR)
		return TRUE;
	return FALSE;
}

static bool _dragon_stock_aux_p(int k_idx)
{
	if (!_stock_p(k_idx))
		return FALSE;
	if (k_info[k_idx].tval == TV_DRAG_ARMOR)
		return TRUE;
	if ((k_info[k_idx].tval == TV_CLOAK) && (k_info[k_idx].sval == SV_DRAGON_CLOAK))
		return TRUE;
	if ((k_info[k_idx].tval == TV_HELM) && (k_info[k_idx].sval == SV_DRAGON_HELM))
		return TRUE;
	if ((k_info[k_idx].tval == TV_BOOTS) && (k_info[k_idx].sval == SV_PAIR_OF_DRAGON_GREAVE))
		return TRUE;
	if ((k_info[k_idx].tval == TV_GLOVES) && (k_info[k_idx].sval == SV_SET_OF_DRAGON_GLOVES))
		return TRUE;
	if ((k_info[k_idx].tval == TV_SHIELD) && (k_info[k_idx].sval == SV_DRAGON_SHIELD))
		return TRUE;
	return FALSE;
}

static bool _dragon_create(obj_ptr obj, u32b mode)
{
	int k_idx;
	if (!one_in_(4))
	{
		k_idx = _get_k_idx(_dragon_stock_p, _mod_lvl(50));
	}
	else
	{
		k_idx = _get_k_idx(_dragon_stock_aux_p, _mod_lvl(50));
	}
	return _create(obj, k_idx, _mod_lvl(rand_range(1, 25)), mode);
}


/************************************************************************
 * Shops
 ***********************************************************************/
static void _change_owner(shop_ptr shop)
{
    int ct = _count_owners(shop->type);

    for (;;)
    {
        int        idx = randint0(ct);
        _owner_ptr owner = &shop->type->owners[idx];

        if (owner != shop->owner && !owner->active)
        {
            if (shop->owner)
            {
                msg_format("<color:U>%s</color> retires.", shop->owner->name);
                shop->owner->active = FALSE;
            }
            shop->owner = owner;
            shop->owner->active = TRUE;
            break;
        }
    }
}

shop_ptr shop_alloc(int which)
{
    shop_ptr shop = malloc(sizeof(shop_t));
    shop->type = _get_type(which);
    shop->owner = NULL;
    shop->inv = inv_alloc(shop->type->name, INV_SHOP, 0);
    _change_owner(shop);
    shop_reset(shop);
    return shop;
}

shop_ptr shop_load(savefile_ptr file)
{
    shop_ptr shop = malloc(sizeof(shop_t));
    int      tmp;
    u32b     guard;

    tmp = savefile_read_s16b(file);
    shop->type = _get_type(tmp);
    assert(shop->type);

    tmp = savefile_read_s16b(file);
    shop->owner = _get_owner(shop->type, tmp);
    assert(shop->owner);
    shop->owner->active = TRUE;

    shop->inv = inv_alloc(shop->type->name, INV_SHOP, 0);
    inv_load(shop->inv, file);

    shop->last_restock.turn = savefile_read_s32b(file);
    shop->last_restock.level = savefile_read_s16b(file);
    shop->last_restock.exp = savefile_read_s32b(file);

    guard = savefile_read_u32b(file);
    assert(guard == 0xFEEDFEED);

    return shop;
}

void shop_free(shop_ptr shop)
{
    if (shop)
    {
        inv_free(shop->inv);
        shop->inv = NULL;
        shop->type = NULL;
        shop->owner = NULL;
        free(shop);
    }
}

void shop_reset(shop_ptr shop)
{
    inv_clear(shop->inv);
    shop->last_restock.turn = 0;
    shop->last_restock.level = 0;
    shop->last_restock.exp = 0;
}

void shop_save(shop_ptr shop, savefile_ptr file)
{
    savefile_write_s16b(file, shop->type->id);
    savefile_write_s16b(file, shop->owner->id);
    inv_save(shop->inv, file);
    savefile_write_s32b(file, shop->last_restock.turn);
    savefile_write_s16b(file, shop->last_restock.level);
    savefile_write_s32b(file, shop->last_restock.exp);
    savefile_write_u32b(file, 0xFEEDFEED);
}

/************************************************************************
 * Pricing
 * Note: All functions take the point of view of the *shop*, not 
 *       the player. So _buy is the shop buying or the player
 *       selling. This is appropriate for a shop module!
 ***********************************************************************/
static int _price_factor_aux(int greed)
{
    int factor;

    factor = get_race()->shop_adjust;
    if (factor == 0)
        factor = 110;

    factor = (factor * adj_gold[p_ptr->stat_ind[A_CHR]] + 50) / 100;
    factor = (factor * (135 - MIN(200, p_ptr->fame)/4) + 50) / 100;
    factor = (factor * greed + 50) / 100;

    return factor;
}

int _price_factor(shop_ptr shop)
{
    int factor = _price_factor_aux(shop->owner->greed);

    if (prace_is_(shop->owner->race_id))
        factor = factor * 90 / 100;

    return factor;
}

static int _sell_price_aux(int price, int factor)
{
    if (factor < 100)
        factor = 100;

    if (price > 1000*1000)
        price = (price / 100) * factor;
    else
        price = (price * factor + 50) / 100;

    if (price > 1000)
        price = big_num_round(price, 3);

    return price;
}

static int _sell_price(shop_ptr shop, int price)
{
    int factor = _price_factor(shop);

    if ((shop->type->id == SHOP_DRAGON) && (price > 31000))
    {
        int ylihinta = (price - 30000);
        price += ((ylihinta / 200) * ylihinta) / 15;
    }

    price = _sell_price_aux(price, factor);
    if (shop->type->id == SHOP_BLACK_MARKET)
    {
        if (p_ptr->realm1 != REALM_BURGLARY && !mut_present(MUT_BLACK_MARKETEER))
            price = price * 2;

        price = price * (625 + virtue_current(VIRTUE_JUSTICE)) / 625;
    }
    else if (shop->type->id == SHOP_JEWELER)
        price = price * 2;

    return price;
}

static int _buy_price_aux(int price, int factor)
{
    if (factor < 105)
        factor = 105;

    if (price > 1000*1000)
        price = (price / factor) * 100;
    else
        price = (price * 100) / factor;

    if (price > 1000)
        price = big_num_round(price, 3);

    return price;
}

static int _buy_price(shop_ptr shop, int price)
{
    int factor = _price_factor(shop);

    price = _buy_price_aux(price, factor);
    if (shop->type->id == SHOP_BLACK_MARKET)
    {
        if (p_ptr->realm1 != REALM_BURGLARY && !mut_present(MUT_BLACK_MARKETEER))
            price = price / 2;

        price = price * (625 - virtue_current(VIRTUE_JUSTICE)) / 625;
    }
    else if (shop->type->id == SHOP_JEWELER)
        price = price / 2;

    if (price > shop->owner->purse)
        price = shop->owner->purse;

    return MAX(1, price);
}

int town_service_price(int price)
{
    int factor = _price_factor_aux(100);
    return _sell_price_aux(price, factor);
}

/************************************************************************
 * User Interface
 ***********************************************************************/
struct _ui_context_s
{
    shop_ptr shop;
    slot_t   top;
    int      page_size;
    doc_ptr  doc;
};
typedef struct _ui_context_s _ui_context_t, *_ui_context_ptr;

static void _display(_ui_context_ptr context);
static void _buy(_ui_context_ptr context);
static void _examine(_ui_context_ptr context);
static void _sell(_ui_context_ptr context);
static void _sellout(shop_ptr shop);
static void _reserve(_ui_context_ptr context);
static void _loop(_ui_context_ptr context);

static void _maintain(shop_ptr shop);
static int  _cull(shop_ptr shop, int target);
static int  _restock(shop_ptr shop, int target, bool is_shuffle);
static void _wizard_stock(shop_ptr shop);
static void _shuffle_stock(shop_ptr shop);
static int  _stock_base(shop_ptr shop);

void shop_ui(shop_ptr shop)
{
    _ui_context_t context = {0};

    store_hack = TRUE;
    _maintain(shop);

    context.shop = shop;
    context.top = 1;
    _loop(&context);
    store_hack = FALSE;
}

static void _loop(_ui_context_ptr context)
{
    forget_lite(); /* resizing the term would redraw the map ... sigh */
    forget_view();
    character_icky = TRUE;

    msg_line_clear();
    msg_line_init(ui_shop_msg_rect());

    Term_clear();
    context->doc = doc_alloc(MIN(80, ui_shop_rect().cx));
    for (;;)
    {
        int    max = inv_last(context->shop->inv, obj_exists);
        rect_t r = ui_shop_rect(); /* recalculate in case resize */
        int    cmd, ct;

        context->page_size = MIN(26, r.cy - 3 - 6);
        if ((context->top - 1) % context->page_size != 0) /* resize?? */
            context->top = 1;

        _display(context);

        cmd = inkey_special(TRUE);
        msg_line_clear();
        msg_boundary(); /* turn_count is unchanging while in home/museum */
        if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q') break;
        pack_lock();
        if (!shop_common_cmd_handler(cmd))
        {
            switch (cmd) /* cmd is from the player's perspective */
            {
            case 'g': case 'b': case 'p': _sell(context); break;
            case 'B': _sellout(context->shop); break;
            case 'd': case 's': _buy(context); break;
            case 'x': _examine(context); break;
            case 'S': _shuffle_stock(context->shop); break;
            case 'R': _reserve(context); break;
            case KTRL('A'):
                if (p_ptr->wizard) _wizard_stock(context->shop);
            case KTRL('S'):
                if (p_ptr->wizard) inv_sort(context->shop->inv);
                break;
            case KTRL('O'):
                if (p_ptr->wizard) _change_owner(context->shop);
                break;
            case '?':
                doc_display_help("context_shop.txt", NULL);
                Term_clear_rect(ui_shop_msg_rect());
                break;
            case SKEY_PGDOWN: case '3': case ' ':
                if (context->top + context->page_size - 1 < max)
                    context->top += context->page_size;
                break;
            case SKEY_PGUP: case '9': case '-':
                if (context->top > context->page_size)
                    context->top -= context->page_size;
                break;
            default:
                if (cmd < 256 && isprint(cmd))
                {
                    msg_format("Unrecognized command: <color:R>%c</color>. "
                               "Press <color:keypress>?</color> for help.", cmd);
                }
                else if (KTRL('A') <= cmd && cmd <= KTRL('Z'))
                {
                    cmd |= 0x40;
                    msg_format("Unrecognized command: <color:R>^%c</color>. "
                               "Press <color:keypress>?</color> for help.", cmd);
                }
            }
            ct = inv_count_slots(context->shop->inv, obj_exists);
            if (!ct)
            {
                _restock(context->shop, _stock_base(context->shop), TRUE);
                context->top = 1;
                if (one_in_(20)) _change_owner(context->shop);
                msg_format("<color:U>%s</color> brings out some new stock.", context->shop->owner->name);
            }
            else
            {
                max = inv_last(context->shop->inv, obj_exists);
                while (context->top > max)
                    context->top -= context->page_size;
                if (context->top < 1) context->top = 1;
            }
        }
        pack_unlock();
        notice_stuff(); /* PW_INVEN and PW_PACK ... */
        handle_stuff(); /* Plus 'C' to view character sheet */
        if ((shop_exit_hack) || (pack_overflow_count() > ((pack_is_full()) ? 0 : 1)))
        {
            if (shop_exit_hack) msg_print("It's time for you to leave!");
            else msg_print("<color:v>Your pack is overflowing!</color> It's time for you to leave!");
            msg_print(NULL);
            shop_exit_hack = FALSE;
            break;
        }
    }
    character_icky = FALSE;
    energy_use = 100;
    msg_line_clear();
    msg_line_init(ui_msg_rect());

    Term_clear();
    do_cmd_redraw();

    doc_free(context->doc);
}

static void _display_inv(doc_ptr doc, shop_ptr shop, slot_t top, int page_size);
static void _display(_ui_context_ptr context)
{
    rect_t   r = ui_shop_rect();
    doc_ptr  doc = context->doc;
    shop_ptr shop = context->shop;
    int      ct = strlen(shop->type->name) + 10; /* " (20000gp)" */
    char     buf[10];

    doc_clear(doc);
    doc_insert(doc, "<style:table>");
    doc_printf(doc, "    <color:U>%s (%s)</color>",
        shop->owner->name, get_race_aux(shop->owner->race_id, 0)->name);
    doc_printf(doc, "<tab:%d><color:G>%s</color> (<color:r>%d</color>)\n\n",
        doc_width(doc) - ct, shop->type->name, shop->owner->purse);

    _display_inv(doc, shop, context->top, context->page_size);
    
    {
        slot_t max = inv_last(shop->inv, obj_exists);
        slot_t bottom = context->top + context->page_size - 1;

        if (context->top > 1 || bottom < max)
        {
            int page_count = (max - 1) / context->page_size + 1;
            int page_current = (context->top - 1) / context->page_size + 1;

            doc_printf(doc, "<color:B>(Page %d of %d)</color>\n", page_current, page_count);
        }
    }

    big_num_display(p_ptr->au, buf);
    doc_printf(doc, "Gold Remaining: <color:y>%s</color>\n\n", buf);
    doc_insert(doc, "<color:keypress>b</color> to buy. ");
    if (no_selling)
        doc_insert(doc, "<color:keypress>s</color> to give. ");
    else
        doc_insert(doc, "<color:keypress>s</color> to sell. ");
    doc_insert(doc, 
        "<color:keypress>x</color> to begin examining items.\n"
        "<color:keypress>B</color> to buyout inventory. "
		"<color:keypress>S</color> to shuffle stock. "
        "<color:keypress>R</color> to reserve an item.");
    doc_newline(doc);

    doc_insert(doc,
        "<color:keypress>Esc</color> to exit. "
        "<color:keypress>?</color> for help.");
    doc_insert(doc, "</style>");

    Term_clear_rect(r);
    doc_sync_term(doc,
        doc_range_top_lines(doc, r.cy),
        doc_pos_create(r.x, r.y));
}

static int _add_obj(shop_ptr shop, obj_ptr obj, bool is_restock);
static bool _buy_aux(shop_ptr shop, obj_ptr obj)
{
    char       name[MAX_NLEN];
    string_ptr s = string_alloc();
    char       c;
    int        price = obj_value(obj);

    if (!price)
    {
        msg_print("I have no interest in your junk!");
        return FALSE;
    }
    if ((obj->tval == TV_CAPTURE) && (obj->pval > 0) && (r_info[obj->pval].ball_num))
    {
        msg_print("I wouldn't take that if you paid me!");
        return FALSE;
    }
    price = _buy_price(shop, price);
    price *= obj->number;

    object_desc(name, obj, OD_COLOR_CODED);
    if (no_selling)
        string_printf(s, "Really give %s? <color:y>[y/n]</color>", name);
    else
        string_printf(s, "Really sell %s for <color:R>%d</color> gp? <color:y>[y/n]</color>", name, price);
    c = msg_prompt(string_buffer(s), "ny", PROMPT_YES_NO);
    string_free(s);
    if (c == 'n') return FALSE;

    if (!no_selling)
    {
        p_ptr->au += price;
        stats_on_gold_selling(price);

        p_ptr->redraw |= PR_GOLD;
        if (prace_is_(RACE_MON_LEPRECHAUN))
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
    }

    obj->inscription = 0;
    obj->feeling = FEEL_NONE;
    obj->marked &= ~OM_WORN;
    obj->timeout = 0;

    obj_identify_fully(obj);
    stats_on_purchase(obj);

    /* This message may seem like spam, but it is not. Selling an
     * un-identified potion of augmentation, for example. */
    object_desc(name, obj, OD_COLOR_CODED);

    if (no_selling)
        msg_format("You gave %s.", name);
    else
        msg_format("You sold %s for <color:R>%d</color> gold.", name, price);

    if (shop->type->id == SHOP_BLACK_MARKET)
        virtue_add(VIRTUE_JUSTICE, -1);

    if (obj->tval == TV_BOTTLE)
        virtue_add(VIRTUE_NATURE, 1);

    if (object_is_(obj, TV_POTION, SV_POTION_BLOOD))
    {
        msg_print("The potion goes sour.");
        obj->sval = SV_POTION_SALT_WATER;
        obj->k_idx = lookup_kind(TV_POTION, SV_POTION_SALT_WATER);
        object_origins(obj, ORIGIN_BLOOD);
        obj->mitze_type = 0;
    }

    price = obj_value(obj); /* correctly handle unidentified items */
    if (price > 0 && _add_obj(shop, obj, FALSE))
        inv_sort(shop->inv);
    else
        obj->number = 0;
    return TRUE;
}

static void _buy(_ui_context_ptr context)
{
    obj_prompt_t prompt = {0};
    int          amt = 1;

    if (no_selling)
    {
        prompt.prompt = "Give which item?";
        prompt.error = "You have nothing to give.";
    }
    else
    {
        prompt.prompt = "Sell which item?";
        prompt.error = "You have nothing to sell.";
    }
    prompt.filter = context->shop->type->buy_p;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_QUIVER;

    command_cmd = 's'; /* Hack for !s inscriptions */
    obj_prompt(&prompt);
    command_cmd = '\0';
    if (!prompt.obj) return;

    if (prompt.obj->number > 1)
    {
        amt = prompt.obj->number;
        if (!msg_input_num("Quantity", &amt, 1, prompt.obj->number)) return;
    }

    if (amt < prompt.obj->number)
    {
        obj_t copy = *prompt.obj;
        int vakuutettu = 0;
        if (prompt.obj->insured)
        {
            vakuutettu = MAX(0, (prompt.obj->insured % 100) - prompt.obj->number + amt);
            if (vakuutettu) copy.insured = (prompt.obj->insured / 100) * 100 + vakuutettu;
            else copy.insured = 0;
        }
        copy.number = amt;
        if (_buy_aux(context->shop, &copy))
        {
            obj_identify_fully(prompt.obj);
            prompt.obj->number -= amt;
            if (vakuutettu) obj_dec_insured(prompt.obj, vakuutettu);
            prompt.obj->marked |= OM_DELAYED_MSG;
            p_ptr->notice |= PN_CARRY;
            if (prompt.obj->loc.where == INV_QUIVER)
                p_ptr->notice |= PN_OPTIMIZE_QUIVER;
            else if (prompt.obj->loc.where == INV_PACK)
                p_ptr->notice |= PN_OPTIMIZE_PACK;
        }
    }
    else
        _buy_aux(context->shop, prompt.obj);

    obj_release(prompt.obj, OBJ_RELEASE_QUIET);
}

static void _examine(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;

        if (!msg_command("<color:y>Examine which item <color:w>(<color:keypress>Esc</color> when done)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->shop->inv, slot);
        if (!obj) continue;

        obj_learn_store(obj);
        obj_display(obj);
    }
}

static void _reserve_aux(shop_ptr shop, obj_ptr obj)
{
    int        cost = _sell_price(shop, MIN(10000, obj_value(obj) / 2));
    string_ptr s;
    char       c;
    char       name[MAX_NLEN];

    object_desc(name, obj, OD_COLOR_CODED);
    s = string_alloc_format("Reserve %s for <color:R>%d</color> gp? <color:y>[y/n]</color>", name, cost);
    c = msg_prompt(string_buffer(s), "ny", PROMPT_YES_NO);
    string_free(s);
    if (c == 'n') return;
    if (cost > p_ptr->au)
    {
        msg_print("You don't have enough gold.");
        return;
    }
    p_ptr->au -= cost;
    stats_on_gold_services(cost);

    p_ptr->redraw |= PR_GOLD;
    if (prace_is_(RACE_MON_LEPRECHAUN))
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

    obj->marked |= OM_RESERVED;
    msg_format("Done! I'll hold on to %s for you. You may come back at any time to purchase it.", name);
}

static void _reserve(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;

        if (!msg_command("<color:y>Reserve which item <color:w>(<color:keypress>Esc</color> when done)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->shop->inv, slot);
        if (!obj) continue;

        if (obj->marked & OM_RESERVED)
        {
            msg_print("You have already reserved that item. Choose another.");
            continue;
        }
        _reserve_aux(context->shop, obj);
        break;
    }
}

static bool _sell_aux(shop_ptr shop, obj_ptr obj)
{
    char       name[MAX_NLEN];
    string_ptr s = string_alloc();
    char       c;
    int        price = obj_value(obj);

    price = _sell_price(shop, price);
    price *= obj->number;

    object_desc(name, obj, OD_COLOR_CODED);
    string_printf(s, "Really buy %s for <color:R>%d</color> gp? <color:y>[y/n]</color>", name, price);
    c = msg_prompt(string_buffer(s), "ny", PROMPT_YES_NO);
    string_free(s);
    if (c == 'n') return FALSE;

    if (price > p_ptr->au)
    {
        msg_print("You do not have enough gold.");
        return FALSE;
    }
    p_ptr->au -= price;
    stats_on_gold_buying(price);

    p_ptr->redraw |= PR_GOLD;
    if (prace_is_(RACE_MON_LEPRECHAUN))
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

    obj->ident &= ~IDENT_STORE;
    obj->inscription = 0;
    obj->feeling = FEEL_NONE;
    obj->marked &= ~OM_RESERVED;

    /* Almost all origins are marked on item creation, but origin_store is
     * marked on purchase to avoid message spam and misleading messages
     * ("bought from store" for items that have not yet been bought).
     * However, we may be repurchasing an item that we previously sold, so
     * we need to avoid overwriting such items' original origins */
    if ((!obj->origin_type) || (obj->origin_type == ORIGIN_MIXED)) object_origins(obj, ORIGIN_STORE);

    obj_identify_fully(obj);
    stats_on_purchase(obj);

    if (shop->type->id == SHOP_BLACK_MARKET)
        virtue_add(VIRTUE_JUSTICE, -1);

    if (obj->tval == TV_BOTTLE)
        virtue_add(VIRTUE_NATURE, -1);

    pack_carry(obj);
    /*msg_format("You have %s.", name);*/
    return TRUE;
}

static void _sell(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;
        int     amt = 1;

		//ban obvious thing
		if (context->shop->type->id == SHOP_SHROOMERY)
		{
			if (prace_is_(RACE_SNOTLING) || prace_is_(RACE_DOPPELGANGER)) {
				msg_print("We don't serve your kind here.");
				return;
			}
			if ((p_ptr->prace == RACE_SNOTLING) || (p_ptr->prace == RACE_DOPPELGANGER)) {
				msg_print("I'm wise to your tricks. You can't have my mushrooms.");
				return;
			}
		}

        if (!msg_command("<color:y>Buy which item <color:w>(<color:keypress>Esc</color> "
                         "to cancel)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->shop->inv, slot);
        if (!obj) continue;

        if (obj->number > 1)
        {
            if (!msg_input_num("Quantity", &amt, 1, obj->number)) continue;
        }

        if (amt < obj->number)
        {
            obj_t copy = *obj;
            int vakuutettu = 0;
            if (obj->insured)
            {
                vakuutettu = MIN((obj->insured % 100), amt);
                if (vakuutettu) copy.insured = (obj->insured / 100) * 100 + vakuutettu;
                else copy.insured = 0;
            }
            copy.number = amt;
            if (_sell_aux(context->shop, &copy))
            {
                obj->number -= amt;
                if (vakuutettu) obj_dec_insured(obj, vakuutettu);
            }
        }
        else
        {
            _sell_aux(context->shop, obj);
            if (!obj->number)
            {
                inv_remove(context->shop->inv, slot);
                inv_sort(context->shop->inv);
            }
        }
        break;
    }
}

static void _sellout(shop_ptr shop)
{
    slot_t slot, max = inv_last(shop->inv, obj_exists);
    int    price, total_price = 0;

	//ugh I copy/pasted this
	if (shop->type->id == SHOP_SHROOMERY)
	{
		if (prace_is_(RACE_SNOTLING) || prace_is_(RACE_DOPPELGANGER)) {
			msg_print("We don't serve your kind here.");
			return;
		}
		if (p_ptr->prace == RACE_DOPPELGANGER) {
			msg_print("I'm wise to your tricks. You can't have my mushrooms.");
			return;
		}
	}

    if (1 && !get_check("Are you sure you want to buy the entire inventory of this store? "))
        return;

    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(shop->inv, slot);
        if (!obj) continue;
        price = _sell_price(shop, obj_value(obj));
        price *= obj->number;
        if (price <= p_ptr->au)
        {
            bool destroy = FALSE;
            int auto_pick_idx = is_autopick(obj);

            if (auto_pick_idx >= 0 && autopick_list[auto_pick_idx].action & DO_AUTODESTROY)
                destroy = TRUE;

            obj->ident &= ~IDENT_STORE;
            obj->inscription = 0;
            obj->feeling = FEEL_NONE;
            obj->marked &= ~OM_RESERVED;

            obj_identify_fully(obj);

            if (!destroy)
            {
                pack_carry(obj);
                stats_on_purchase(obj);
            }
            if (shop->type->id == SHOP_BLACK_MARKET)
                virtue_add(VIRTUE_JUSTICE, -1);

            if (obj->tval == TV_BOTTLE)
                virtue_add(VIRTUE_NATURE, -1);

            obj->number = 0;
            inv_remove(shop->inv, slot);

            p_ptr->au -= price;
            total_price += price;
            stats_on_gold_buying(price);

            p_ptr->redraw |= PR_GOLD;
        }
        else
        {
            msg_print("You have run out of gold.");
            break;
        }
    }

    msg_format("You spent <color:R>%d</color> gp.", total_price);
    if (prace_is_(RACE_MON_LEPRECHAUN))
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

    inv_sort(shop->inv);
}


/************************************************************************
 * Stocking
 ***********************************************************************/
#define _STOCK_LO   6
#define _STOCK_BASE 15
#define _STOCK_HI   24

static int _stock_base(shop_ptr shop)
{
    if (shop->type->id == SHOP_GENERAL)
        return 10 - 2 + randint1(4);
	if (shop->type->id == SHOP_SHROOMERY)
		return 6 + randint1(4);
    if ((shop->type->id == SHOP_WEAPON) && (p_ptr->lev < 12)) return 18 + randint1(5);
    return _STOCK_BASE - 4 + randint1(8);
}

static void _maintain(shop_ptr shop)
{
    int  num;
    int  i;
    bool allow_restock = TRUE;

    /* Always initialize an empty shop */
    if (!inv_count_slots(shop->inv, obj_exists))
    {
        _restock(shop, _stock_base(shop), FALSE);
        return;
    }

    /* Shops maintain once per day */
    num = MIN(10, (game_turn - shop->last_restock.turn) / TOWN_DAWN);
    if (!num) return;

    /* Limit shop scumming (ie resting in town or on DL1 for BM wares) */
    if (!_shop_is_basic(shop))
    {
        if (shop->last_restock.turn)
        {
            int xp = shop->last_restock.exp;
            xp += MIN(MAX(xp / 20, 1000), 100000);
            if ( ((coffee_break) || (!ironman_downward))
              && p_ptr->max_plv <= shop->last_restock.level
              && p_ptr->max_exp <= xp
              && p_ptr->prace != RACE_ANDROID )
            {
                allow_restock = FALSE;
            }
        }
    }

    /* Maintain the shop for each day since last visit */
    for (i = 0; i < num; i++)
    {
        int ct = inv_count_slots(shop->inv, obj_exists);
        if (ct < _STOCK_LO) _restock(shop, _stock_base(shop), FALSE);
        else if (ct > _STOCK_HI) _cull(shop, _stock_base(shop));
        else
        {
            ct = _cull(shop, isompi(_STOCK_LO, ct - randint1(9)));
            if (allow_restock)
                ct = _restock(shop, pienempi(_STOCK_HI, ct + randint1(9)), FALSE);
        }
    }
}

static bool _can_cull(obj_ptr obj)
{
    if (obj && !(obj->marked & OM_RESERVED)) return TRUE;
    return FALSE;
}
static int _cull(shop_ptr shop, int target)
{
    int ct = inv_count_slots(shop->inv, obj_exists);
    int attempt;
    int old_num;

    assert(ct >= target);
    for (attempt = 1; ct > target && attempt < 100; attempt++)
    {
        slot_t  slot = inv_random_slot(shop->inv, _can_cull);
        obj_ptr obj;

        if (!slot) break; /* nothing but 'Reserved' objects remain */

        obj = inv_obj(shop->inv, slot);
        assert(obj->number > 0);
        assert (!(obj->marked & OM_RESERVED));

        old_num = obj->number;

        if (one_in_(2))
            obj->number = (obj->number + 1)/2;
        else if (one_in_(2))
            obj->number--;
        else
            obj->number = 0;

        if (obj->insured) /* Ye gods */
        {
            int poisto = old_num - obj->number;
            obj->number = old_num;
            obj_dec_number(obj, poisto, TRUE);
        }

        if (!obj->number)
        {
            inv_remove(shop->inv, slot);
            ct--;
        }
    }
    inv_sort(shop->inv);
    assert(ct == inv_count_slots(shop->inv, obj_exists));
    return ct;
}

static int _add_obj(shop_ptr shop, obj_ptr obj, bool is_restock) /* return number of new slots used (0 or 1) */
{
    slot_t slot, max = inv_last(shop->inv, obj_exists);
    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr dest = inv_obj(shop->inv, slot);
        if (!dest) continue;
        if (obj_can_combine(dest, obj, INV_SHOP))
        {
            if ((is_restock) && (dest->marked & OM_RESERVED)) return 0;
            obj_combine(dest, obj, INV_SHOP);
            obj->number = 0; /* forget spillover */
            return 0;
        }
    }
    inv_add(shop->inv, obj);
    return 1;
}

static void _wizard_stock(shop_ptr shop)
{
    u32b mode = AM_NO_FIXED_ART;

    if (!dun_level && _shop_is_basic(shop))
        mode |=  AM_STOCK_TOWN;

    if (shop->type->id == SHOP_BLACK_MARKET)
        mode |= AM_STOCK_BM;

    wiz_create_objects(shop->type->create_f, mode);
    Term_clear_rect(ui_shop_msg_rect());
}

static int _restock(shop_ptr shop, int target, bool is_shuffle)
{
    int ct = inv_count_slots(shop->inv, obj_exists);
    int attempt = 0;
    u32b mode = AM_NO_FIXED_ART;

    if (!dun_level && _shop_is_basic(shop))
        mode |=  AM_STOCK_TOWN;

    if (shop->type->id == SHOP_BLACK_MARKET)
        mode |= AM_STOCK_BM;

    if (is_shuffle) mode |= AM_SHUFFLING;

    assert(ct <= target);
    for (attempt = 1; ct < target && attempt < 100; attempt++)
    {
        obj_t forge = {0};
        if (shop->type->create_f(&forge, mode))
        {
            assert(obj_value(&forge) > 0);
            ct += _add_obj(shop, &forge, TRUE);
        }
    }
    inv_sort(shop->inv);
    assert(ct == inv_count_slots(shop->inv, obj_exists));
    shop->last_restock.turn = game_turn;
    shop->last_restock.level = p_ptr->max_plv;
    shop->last_restock.exp = p_ptr->max_exp;
    return ct;
}

static void _shuffle_stock(shop_ptr shop)
{
    if (!p_ptr->wizard)
    {
        int        cost = _sell_price(shop, 5000);
        string_ptr s;
        char       c;
        if (shop->type->id == SHOP_BLACK_MARKET) cost *= 2;
        s = string_alloc_format("Shuffle stock for <color:R>%d</color> gp? <color:y>[y/n]</color>", cost);
        c = msg_prompt(string_buffer(s), "ny", PROMPT_YES_NO);
        string_free(s);
        if (c == 'n') return;
        if (cost > p_ptr->au)
        {
            msg_print("You don't have enough gold.");
            return;
        }
        p_ptr->au -= cost;
        stats_on_gold_services(cost);

        p_ptr->redraw |= PR_GOLD;
        if (prace_is_(RACE_MON_LEPRECHAUN))
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
    }
    _cull(shop, 0);
    _restock(shop, _stock_base(shop), TRUE);
}

/************************************************************************
 * User Interface Helpers
 ***********************************************************************/
bool shop_common_cmd_handler(int cmd)
{
    switch (cmd)
    {
    case '\r':
        return TRUE;
    case 'w':
        if (pack_overflow_count()) return FALSE;
        equip_wield_ui();

        /* This can happen if we equipped from a pile */
        if (pack_overflow_count() > ((pack_is_full()) ? 0 : 1))
        {
            shop_exit_hack = TRUE;
        }

        pack_overflow();
        return TRUE;
    case 't': case 'T':
        equip_takeoff_ui();
        return TRUE;
    case 'k': case KTRL('D'):
        obj_destroy_ui();
        return TRUE;
    case 'e':
        equip_ui();
        return TRUE;
    case 'i':
        pack_ui();
        return TRUE;
    case 'I':
        obj_inspect_ui();
        return TRUE;
    case KTRL('I'):
        toggle_inven_equip();
        return TRUE;
    case '{':
        obj_inscribe_ui();
        return TRUE;
    case '}':
        obj_uninscribe_ui();
        return TRUE;
    case 'C':
        py_display();
        return TRUE;
    case KTRL('W'):
        show_weights = !show_weights;
        return TRUE;
    case KTRL('G'):
        show_item_graph = !show_item_graph;
        return TRUE;
    }
    return FALSE;
}

static void _display_inv(doc_ptr doc, shop_ptr shop, slot_t top, int page_size)
{
    slot_t  slot;
    int     xtra = 0;
    char    name[MAX_NLEN];
    inv_ptr inv = shop->inv;
    bool    show_prices = inv_loc(inv) == INV_SHOP;
    bool    show_values = inv_loc(inv) != INV_SHOP || p_ptr-> wizard;

    if (show_weights)
        xtra += 10;  /* " 123.0 lbs" */
    if (show_prices)
        xtra += 7;
    if (show_values)
        xtra += 7;

    doc_insert(doc, "    Item Description");
    if (xtra)
    {
        doc_printf(doc, "<tab:%d>", doc_width(doc) - xtra);
        if (show_weights)
            doc_printf(doc, " %9.9s", "Weight");
        if (show_prices)
            doc_printf(doc, " %6.6s", "Price");
        if (show_values)
            doc_printf(doc, " %6.6s", "Score");
    }
    doc_newline(doc);

    for (slot = top; slot < top + page_size; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
        {
            obj_ptr     next = inv_obj(inv, slot + 1);
            doc_style_t style = *doc_current_style(doc);
            char        label_color = 'w';

            if (next && obj_cmp(obj, next) > 0)
                label_color = 'r';

            object_desc(name, obj, OD_COLOR_CODED);

            doc_printf(doc, " <color:%c>%c</color>) ", label_color, slot_label(slot - top + 1));
            if (show_item_graph)
            {
                doc_insert_char(doc, object_attr(obj), object_char(obj));
                doc_insert(doc, " ");
            }
            if (xtra)
            {
                style.right = doc_width(doc) - xtra;
                doc_push_style(doc, &style);
            }
            doc_printf(doc, "%s", name);
            if (xtra)
            {
                doc_pop_style(doc);
                doc_printf(doc, "<tab:%d>", doc_width(doc) - xtra);

                if (show_weights)
                {
                    int wgt = obj->weight; /* single object only for home/shops */
                    doc_printf(doc, " %3d.%d lbs", wgt/10, wgt%10);
                }
                if (show_prices || show_values)
                {
                    int value = obj_value(obj);

                    if (show_prices)
                    {
                        int price = _sell_price(shop, value);
                        if (price >= 1000000)
                        {
                            char tmp[10];
                            big_num_display(price, tmp);
                            doc_printf(doc, " <color:%c>%6s</color>", price <= p_ptr->au ? 'w' : 'D', tmp);
                        }
                        else doc_printf(doc, " <color:%c>%6d</color>", price <= p_ptr->au ? 'w' : 'D', price);
                    }
                    if (show_values)
                    {
                        if (value >= 1000000)
                        {
                            char tmp[10];
                            big_num_display(value, tmp);
                            doc_printf(doc, " %6s", tmp);
                        }
                        else doc_printf(doc, " %6d", value);
                    }
                }
            }
            doc_newline(doc);
        }
        else
            doc_newline(doc);
    }
}
void shop_display_inv(doc_ptr doc, inv_ptr inv, slot_t top, int page_size)
{
    shop_t hack = {0};
    hack.inv = inv;
    _display_inv(doc, &hack, top, page_size);
}

/************************************************************************
 * Town
 ***********************************************************************/
static cptr _names[] = { "Wilderness", "Outpost", "Telmora", "Morivant", "Angwil", "Anambar", "Thalos", "Zul", "Dungeon" };

struct town_s
{
   int         id;
   cptr        name;
   bool        visited;
   int_map_ptr shops;
};

static town_ptr _town_alloc(int which, cptr name)
{
    town_ptr town = malloc(sizeof(town_t));
    town->id = which;
    town->name = name;
    town->visited = FALSE;
    town->shops = int_map_alloc((int_map_free_f)shop_free);
    return town;
}

static town_ptr _town_load(savefile_ptr file)
{
    town_ptr town = malloc(sizeof(town_t));
    int      ct, i;

    town->id = savefile_read_s16b(file);
    assert(0 < town->id && town->id <= TOWN_RANDOM);
    town->name = _names[town->id];
    town->visited = savefile_read_bool(file);
    town->shops = int_map_alloc((int_map_free_f)shop_free);

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        shop_ptr shop = shop_load(file);
        int_map_add(town->shops, shop->type->id, shop);
    }
    return town;
}

static void _town_free(town_ptr town)
{
    if (town)
    {
        int_map_free(town->shops);
        town->shops = NULL;
        free(town);
    }
}

static void _town_save(town_ptr town, savefile_ptr file)
{
    int_map_iter_ptr iter;

    savefile_write_s16b(file, town->id);
    savefile_write_bool(file, town->visited);
    savefile_write_s16b(file, int_map_count(town->shops));

    for (iter = int_map_iter_alloc(town->shops);
            int_map_iter_is_valid(iter);
            int_map_iter_next(iter))
    {
        shop_ptr shop = int_map_iter_current(iter);
        shop_save(shop, file);
    }
    int_map_iter_free(iter);
}

shop_ptr town_get_shop(town_ptr town, int which)
{
    shop_ptr shop = int_map_find(town->shops, which);
    if (!shop)
    {
        shop = shop_alloc(which);
        int_map_add(town->shops, which, shop);
    }
    assert(shop);
    return shop;
}

bool town_visited(int which)
{
    return towns_get_town(which)->visited;
}

void town_on_visit(int which)
{
    towns_get_town(which)->visited = TRUE;
}

cptr town_name(int which)
{
    return _names[which];
}
/************************************************************************
 * Towns
 ***********************************************************************/
static int_map_ptr _towns = NULL;

void towns_init(void)
{
    int_map_free(_towns);
    _towns = int_map_alloc((int_map_free_f) _town_free);
}

town_ptr towns_current_town(void)
{
    if (dun_level)
        return towns_get_town(TOWN_RANDOM);
    else if (p_ptr->town_num)
        return towns_get_town(p_ptr->town_num);
    return towns_get_town(TOWN_RANDOM); /* wilderness encounter */
}


town_ptr towns_get_town(int which)
{
    town_ptr town = int_map_find(_towns, which);
    assert(0 < which && which <= TOWN_RANDOM);
    if (!town)
    {
        town = _town_alloc(which, _names[which]);
        int_map_add(_towns, which, town);
    }
    assert(town);
    return town;
}

void towns_save(savefile_ptr file)
{
    int_map_iter_ptr iter;

    savefile_write_s16b(file, int_map_count(_towns));

    for (iter = int_map_iter_alloc(_towns);
            int_map_iter_is_valid(iter);
            int_map_iter_next(iter))
    {
        town_ptr town = int_map_iter_current(iter);
        _town_save(town, file);
    }
    int_map_iter_free(iter);
}

void towns_load(savefile_ptr file)
{
    int i, ct;

    int_map_clear(_towns);

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        town_ptr town = _town_load(file);
        int_map_add(_towns, town->id, town);
    }
}

void towns_on_turn_overflow(int rollback_turns)
{
    int town_id;
    for (town_id = TOWN_MIN; town_id <= TOWN_MAX; town_id++)
    {
        town_ptr town = int_map_find(_towns, town_id);
        int_map_iter_ptr iter;
        if (!town) continue;
        for (iter = int_map_iter_alloc(town->shops);
                int_map_iter_is_valid(iter);
                int_map_iter_next(iter))
        {
            shop_ptr shop = int_map_iter_current(iter);
            if (shop->last_restock.turn > rollback_turns)
                shop->last_restock.turn -= rollback_turns;
            else
                shop->last_restock.turn = 0;
        }
        int_map_iter_free(iter);
    }
}

void _town_add_shop_item(town_ptr town, int which, int k_idx, int ct)
{
    shop_ptr shop = town_get_shop(town, which);
    int i;
    if (!k_idx) return;
    if (!inv_count_slots(shop->inv, obj_exists)) _restock(shop, _stock_base(shop), TRUE);
    for (i = 0; i < ct; i++)
    {
        obj_t forge = {0};
        if (_create(&forge, k_idx, _mod_lvl(rand_range(1, 15)), AM_STOCK_TOWN))
        {
            (void)_add_obj(shop, &forge, TRUE);
        }
    }
    inv_sort(shop->inv);
}

void birth_shop_items(void)
{
    town_ptr town = towns_get_town(TOWN_BIRTH);
    if (TOWN_BIRTH != TOWN_ZUL) _town_add_shop_item(town, SHOP_GENERAL, lookup_kind(TV_LITE, SV_LITE_LANTERN), 1);
    _town_add_shop_item(town, SHOP_TEMPLE, lookup_kind(TV_POTION, SV_POTION_CURE_SERIOUS), 1);
    _town_add_shop_item(town, SHOP_ALCHEMIST, lookup_kind(TV_SCROLL, SV_SCROLL_PHASE_DOOR), 1);
    _town_add_shop_item(town, SHOP_ALCHEMIST, lookup_kind(TV_SCROLL, SV_SCROLL_TELEPORT), 1);
    _town_add_shop_item(town, SHOP_WEAPON, lookup_kind(TV_QUIVER, 0), 1);
}

/************************************************************************
 * Towns: Parsing
 * TODO: Redo buildings? I've kept the old parser/syntax for now.
 * TODO: Redesign t_info.txt like q_info.txt. town_t.file will give
 *       the correct file to parse and town_t.buildings will store
 *       the buildings.
 ***********************************************************************/
static errr _parse_building(char *buf, int options) /* moved w/o change from init1.c */
{
    int i;
    char *zz[128];
    int index;
    char *s;

    s = buf + 2;
    /* Get the building number */
    index = atoi(s);

    /* Find the colon after the building number */
    s = my_strchr(s, ':');

    /* Verify that colon */
    if (!s) return (1);

    /* Nuke the colon, advance to the sub-index */
    *s++ = '\0';

    /* Paranoia -- require a sub-index */
    if (!*s) return (1);

    /* Building definition sub-index */
    switch (s[0])
    {
        /* Building name, owner, race */
        case 'N':
        {
            if (tokenize(s + 2, 3, zz, 0) == 3)
            {
                /* Name of the building */
                strcpy(building[index].name, zz[0]);

                /* Name of the owner */
                strcpy(building[index].owner_name, zz[1]);

                /* Race of the owner */
                strcpy(building[index].owner_race, zz[2]);

                break;
            }

            return (PARSE_ERROR_TOO_FEW_ARGUMENTS);
        }

        /* Building Action */
        case 'A':
        {
            if (tokenize(s + 2, 8, zz, 0) >= 7)
            {
                /* Index of the action */
                int action_index = atoi(zz[0]);

                /* Name of the action */
                strcpy(building[index].act_names[action_index], zz[1]);

                /* Cost of the action for members */
                building[index].member_costs[action_index] = atoi(zz[2]);

                /* Cost of the action for non-members */
                building[index].other_costs[action_index] = atoi(zz[3]);

                /* Letter assigned to the action */
                building[index].letters[action_index] = zz[4][0];

                /* Action code */
                building[index].actions[action_index] = atoi(zz[5]);

                /* Action restriction */
                building[index].action_restr[action_index] = atoi(zz[6]);

                break;
            }

            return (PARSE_ERROR_TOO_FEW_ARGUMENTS);
        }

        /* Building Classes
            The old way:
            B:7:C:2:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:2:0:0:2:2:2:0:0:0:0:0:2:0:0:2:0:2:2:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0

            The new way:
            B:7:C:*:None to set a default
            B:7:C:Warrior:Owner to set an owner
            B:7:C:Ranger:Member to set a member
            (You probably should always specify a default first since I am unsure if
            code cleans up properly.)
        */
        case 'C':
        {
            if (tokenize(s + 2, 2, zz, 0) == 2)
            {
                int c = get_bldg_member_code(zz[1]);

                if (c < 0)
                    return PARSE_ERROR_GENERIC;

                if (strcmp(zz[0], "*") == 0)
                {
                    for (i = 0; i < MAX_CLASS; i++)
                        building[index].member_class[i] = c;
                }
                else
                {
                    int idx = lookup_class_idx(zz[0]);
                    if (idx < 0 || idx >= MAX_CLASS)
                        return PARSE_ERROR_GENERIC;
                    building[index].member_class[idx] = c;
                }
                break;
            }

            return (PARSE_ERROR_TOO_FEW_ARGUMENTS);
        }

        /* Building Races
            Same as with classes ...
        */
        case 'R':
        {
            if (tokenize(s + 2, 2, zz, 0) == 2)
            {
                int c = get_bldg_member_code(zz[1]);

                if (c < 0)
                    return PARSE_ERROR_GENERIC;

                if (strcmp(zz[0], "*") == 0)
                {
                    for (i = 0; i < MAX_RACES; i++)
                        building[index].member_race[i] = c;
                }
                else
                {
                    int idx = get_race_idx(zz[0]);
                    if (idx < 0 || idx >= MAX_RACES)
                        return PARSE_ERROR_GENERIC;
                    building[index].member_race[idx] = c;
                    if (idx == RACE_VAMPIRE) /* We have 2 races with the same name! */
                        building[index].member_race[RACE_MON_VAMPIRE] = c;
                }
                break;
            }

            return (PARSE_ERROR_TOO_FEW_ARGUMENTS);
        }

        /* Building Realms */
        case 'M':
        {
            if (tokenize(s + 2, 2, zz, 0) == 2)
            {
                int c = get_bldg_member_code(zz[1]);

                if (c < 0)
                    return PARSE_ERROR_GENERIC;

                if (strcmp(zz[0], "*") == 0)
                {
                    for (i = 0; i <= MAX_REALM; i++)
                        building[index].member_realm[i] = c;
                }
                else
                {
                    int idx = get_realm_idx(zz[0]);
                    if (idx < 0 || idx > MAX_REALM)
                        return PARSE_ERROR_GENERIC;
                    building[index].member_realm[idx] = c;
                }
                break;
            }

            return (PARSE_ERROR_TOO_FEW_ARGUMENTS);
        }

        case 'Z':
        {
            /* Ignore scripts */
            break;
        }

        default:
        {
            return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
        }
    }

    return (0);
}
static room_ptr _temp_room;
static errr _parse_town(char *line, int options)
{
    if (line[0] == 'B' && line[1] == ':')
        return _parse_building(line, options);
    if (_temp_room)
        return parse_room_line(_temp_room, line, options);
    return 0;
}
room_ptr towns_get_map(void)
{
    room_ptr room = room_alloc("NotSureYet");
    _temp_room = room;
    if (parse_edit_file("t_info.txt", _parse_town, 0) != ERROR_SUCCESS)
    {
        room_free(room);
        room = NULL;
    }
    _temp_room = NULL;
    return room;
}

void towns_init_buildings(void)
{
    parse_edit_file("t_info.txt", _parse_town, 0);
}

