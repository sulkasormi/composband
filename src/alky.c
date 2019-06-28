/* Adapted from PosChengband-R */

#include "angband.h"

#include <assert.h>

/** Based on Magic Eater code, if it wasn't obvious.**/

#define _MAX_INF_SLOTS 12
#define _INVALID_SLOT -1
#define _INFUSION_CAP 99
#define _MAX_CHEM 35000

#define _CTIER0 0
#define _CTIER1 1
#define _CTIER2 2
#define _CTIER_MAX 3

static cptr _tiername[_CTIER_MAX] = {
	"tier 0",
	"tier 1",
	"tier 2"
};

static object_type _infusions[_MAX_INF_SLOTS];
static bool _infusions_init = FALSE;

static int _CHEM[] = {0,0,0};

static u16b _alchemist_hero = 0;
static u16b _alchemist_shero = 0;

int _AlchemistSkill(void) {
	return MAX(1, ((p_ptr->lev * 6 / 5) + (2 * adj_mag_stat[p_ptr->stat_ind[A_INT]])));
}

int alchemist_infusion_energy_use(void) {
	return (90 - (_AlchemistSkill() / 2));
}

typedef struct {
	int  sval;
	int  cost;
	int minLv;
	int ctier;
} _formula_info_t, *_formula_info_ptr;


// a single potion is about 1/3 of the cost ( similar to alchemy spell )
// empties for sake of being consistent with the ids, so I can just do quick lookup with _formulas[SV_POTION_WATER] etc. 
// svals might be bit excessive then, but eh... Easier on eye, perhaps. Could be checked for safety, ex (if(i!=sval) search_through ). 

static _formula_info_t _formulas[POTION_MAX+1] = {
{ SV_POTION_WATER,					10, 1, _CTIER0 },
{ SV_POTION_APPLE_JUICE,			10, 1, _CTIER0 },
{ SV_POTION_SLIME_MOLD,				60, 1, _CTIER0 }, 
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_SLOWNESS,				20, 1, _CTIER0 },
{ SV_POTION_SALT_WATER,				20, 1, _CTIER0 },
{ SV_POTION_POISON,					30, 1, _CTIER0 },
{ SV_POTION_BLINDNESS,				30, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_CONFUSION,				30, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_SLEEP,					60, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_LOSE_MEMORIES,			80, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_RUINATION,				100, 55, _CTIER1 },
{ SV_POTION_DEC_STR,				40, 1, _CTIER0 },
{ SV_POTION_DEC_INT,				40, 1, _CTIER0 },
{ SV_POTION_DEC_WIS,				40, 1, _CTIER0 },
{ SV_POTION_DEC_DEX,				40, 1, _CTIER0 },
{ SV_POTION_DEC_CON,				40, 1, _CTIER0 },
{ SV_POTION_DEC_CHR,				40, 1, _CTIER0 },
{ SV_POTION_DETONATIONS,			100, 55, _CTIER1 },
{ SV_POTION_DEATH,					160, 55, _CTIER1 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_SIGHT,				75, 15, _CTIER0 },
{ SV_POTION_CURE_POISON,			75, 15, _CTIER0 },
{ SV_POTION_BOLDNESS,				60, 5, _CTIER1 },
{ SV_POTION_SPEED,				150, 20, _CTIER0 },
{ SV_POTION_THERMAL,				75, 20, _CTIER1 },
{ SV_POTION_VIGOR,				75, 20, _CTIER1 },
{ SV_POTION_HEROISM,				60, 10, _CTIER0 },
{ SV_POTION_BERSERK_STRENGTH,			120, 35, _CTIER1 },
{ SV_POTION_CURE_LIGHT,				30, 1, _CTIER0 },
{ SV_POTION_CURE_SERIOUS,			60, 20, _CTIER0 },
{ SV_POTION_CURE_CRITICAL,			120, 35, _CTIER1 },
{ SV_POTION_HEALING,				240, 55, _CTIER2 },
{ SV_POTION_STAR_HEALING,			480, 75, _CTIER2 },
{ SV_POTION_LIFE,				720, 90, _CTIER2 },
{ SV_POTION_RESTORE_MANA,			160, 50, _CTIER1 },
{ SV_POTION_RESTORE_EXP,			60, 30, _CTIER1 },
{ SV_POTION_RES_STR,				30, 20, _CTIER1 },
{ SV_POTION_RES_INT,				30, 20, _CTIER1 },
{ SV_POTION_RES_WIS,				30, 20, _CTIER1 },
{ SV_POTION_RES_DEX,				30, 20, _CTIER1 },
{ SV_POTION_RES_CON,				30, 20, _CTIER1 },
{ SV_POTION_RES_CHR,				30, 20, _CTIER1 },
{ SV_POTION_INC_STR,				360, 50, _CTIER2 },
{ SV_POTION_INC_INT,				360, 50, _CTIER2 },
{ SV_POTION_INC_WIS,				360, 50, _CTIER2 },
{ SV_POTION_INC_DEX,				360, 50, _CTIER2 },
{ SV_POTION_INC_CON,				360, 50, _CTIER2 },
{ SV_POTION_INC_CHR,				240, 50, _CTIER2 },
{ SV_POTION_RES_ALL,				120, 40, _CTIER1 },
{ SV_POTION_AUGMENTATION,			1000, 999, _CTIER2 },
{ SV_POTION_ENLIGHTENMENT,			120, 40, _CTIER1 },
{ SV_POTION_STAR_ENLIGHTENMENT,			800, 60, _CTIER2 },
{ SV_POTION_SELF_KNOWLEDGE,			120, 40, _CTIER1 },
{ SV_POTION_EXPERIENCE,				1000, 999, _CTIER2 },
{ SV_POTION_RESISTANCE,				120, 30, _CTIER1 },
{ SV_POTION_CURING,					90, 30, _CTIER1 },
{ SV_POTION_INVULNERABILITY,		450, 90, _CTIER2 },
{ SV_POTION_NEW_LIFE,				150, 30, _CTIER2 },
{ SV_POTION_NEO_TSUYOSHI,			60, 1, _CTIER0 },
{ SV_POTION_TSUYOSHI,				30, 1, _CTIER0 },
{ SV_POTION_POLYMORPH,				80, 10, _CTIER1 },
{ SV_POTION_BLOOD,					120, 1, _CTIER0 },
{ SV_POTION_GIANT_STRENGTH,			120, 40, _CTIER1 },
{ SV_POTION_STONE_SKIN,				120, 40, _CTIER1 },
{ SV_POTION_CLARITY,				90, 25, _CTIER0 },
{ SV_POTION_GREAT_CLARITY,			120, 40, _CTIER1 },

{ -1, -1, 999 , _CTIER0 }, // null entry
};


static void _birth(void)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		memset(&_infusions[i], 0, sizeof(object_type));
	}
	_infusions_init = TRUE;
	for (i = 0; i < _CTIER_MAX; i++)
	{
		_CHEM[i] = 0;
	}
	_CHEM[_CTIER0] = 240; 
	_CHEM[_CTIER1] = 120;

	_alchemist_hero = 0;
	_alchemist_shero = 0;

	py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
	py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
	py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
}

static object_type *_which_obj(int tval, int slot)
{
	assert(0 <= slot && slot < _MAX_INF_SLOTS);
	return _infusions + slot;
}

static _formula_info_t *_FindFormula(int sval){
	if (sval<0 || sval > POTION_MAX) return _formulas + POTION_MAX;

	if (sval == _formulas[sval].sval) return _formulas + sval;

	for (int i = 0; i < POTION_MAX; i++){
		if (sval == _formulas[i].sval) return _formulas + sval;
	}
	return _formulas + POTION_MAX;
}

static void _displayInfusions(rect_t display)
{
	char    buf[MAX_NLEN];
	int     i;
	point_t pos = rect_topleft(display);
	int     padding, max_o_len = 20;
	doc_ptr doc = NULL;
	object_type *list = _infusions;
	int alcskil = _AlchemistSkill();
	int DC = 1, tier = 0;

	padding = 5;   /* leading " a) " + trailing " " */
	padding += 12; /* " Count " */

	/* Measure */
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = list + i;
		if (o_ptr->k_idx)
		{
			int len;
			object_desc(buf, o_ptr, 0);
			len = strlen(buf);
			if (len > max_o_len)
				max_o_len = len;
		}
	}

	if (max_o_len + padding > display.cx)
		max_o_len = display.cx - padding;

	/* Display */
	doc = doc_alloc(display.cx);
	doc_insert(doc, "<style:table>");
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = list + i;

		doc_printf(doc, " %c) ", I2A(i));

		if (o_ptr->k_idx)
		{
			DC = _FindFormula(o_ptr->sval)->minLv;
			tier = _FindFormula(o_ptr->sval)->ctier;
			object_desc(buf, o_ptr, OD_COLOR_CODED);
			doc_insert(doc, buf);

			if (DC <= alcskil){ // can do
				 doc_printf(doc, "<tab:%d>Cost: %4d (%s)  Lvl: <color:G>%3d</color>\n", display.cx - 32, _FindFormula(o_ptr->sval)->cost, _tiername[tier], DC);
			} 
			else if (DC == 999){ // can't do ever
				doc_printf(doc, "<tab:%d><color:r>Irreproducible</color>\n",display.cx - 22);
			}
			else {
				doc_printf(doc, "<tab:%d>Cost: %4d (%s)  Lvl: <color:R>%3d</color>\n", display.cx - 32, _FindFormula(o_ptr->sval)->cost, _tiername[tier], DC);
			}

			/*doc_printf(doc, "<tab:%d>SP: %3d.%2.2d\n", display.cx - 12, o_ptr->xtra5 / 100, o_ptr->xtra5 % 100);*/
		}
		else
			doc_insert_text(doc, TERM_L_DARK, "(None)\n");
	}


	doc_printf(doc, "\nAlchemist skill: <color:y>%3d%</color> \n", _AlchemistSkill());
	doc_printf(doc, "Energy cost: <color:B>%7d%</color> \n", alchemist_infusion_energy_use());
	doc_printf(doc, "\nChemical stock: \n");
		doc_printf(doc, "<tab:4><color:U>%s: %5d</color> \n", _tiername[_CTIER0], _CHEM[_CTIER0]);
		doc_printf(doc, "<tab:4><color:o>%s: %5d</color> \n", _tiername[_CTIER1], _CHEM[_CTIER1]);
		doc_printf(doc, "<tab:4><color:v>%s: %5d</color> \n", _tiername[_CTIER2], _CHEM[_CTIER2]);
	doc_printf(doc, "\n");
	doc_insert(doc, "</style>");
	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
	doc_free(doc);
}

#define _ALLOW_EMPTY    0x01 /* Absorb */
#define _ALLOW_SWITCH   0x02 /* Browse/Use */
#define _ALLOW_EXCHANGE 0x04

object_type *_chooseInfusion(cptr verb, int tval, int options)
{
	object_type *result = NULL;
	int          slot = 0;
	int          cmd, komento = 0;
	rect_t       display = ui_menu_rect();
	int          which_tval = tval;
	string_ptr   prompt = NULL;
	bool         done = FALSE;
	bool         exchange = FALSE;
	bool         toisto = FALSE;
	int          slot1 = _INVALID_SLOT, slot2 = _INVALID_SLOT;

	if (display.cx > 80)
		display.cx = 80;

	which_tval = TV_POTION;
	prompt = string_alloc();
	screen_save();

	if ((REPEAT_PULL(&komento)) && ('a' <= komento && komento < 'a' + _MAX_INF_SLOTS))
	{ /* use pulled cmd */
		cmd = komento;
		toisto = TRUE;
	}

	while (!done)
	{
		string_clear(prompt);

		if (exchange)
		{
			if (slot1 == _INVALID_SLOT)
				string_printf(prompt, "Select the first %s:", "infusion");
			else
				string_printf(prompt, "Select the second %s:", "infusion");
		}
		else
		{
			string_printf(prompt, "%s which %s", verb, "infusion");
			if (options & _ALLOW_SWITCH)
			{
				if (options & _ALLOW_EXCHANGE){
					string_append_s(prompt, ", ['X' to Exchange");
					string_append_s(prompt, "]:");
				}
			}
			else
				string_append_c(prompt, ':');
		}
		prt(string_buffer(prompt), 0, 0);
		_displayInfusions(display);

		if ((komento) && (cmd == komento)) /* use pulled command once */
		{
			komento = 0;
		}
		else cmd = inkey_special(FALSE);

		if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q')
			done = TRUE;

		if (options & _ALLOW_EXCHANGE)
		{
			if (!exchange && (cmd == 'x' || cmd == 'X'))
			{
				exchange = TRUE;
				slot1 = slot2 = _INVALID_SLOT;
			}
		}

		if ('a' <= cmd && cmd < 'a' + _MAX_INF_SLOTS)
		{
			slot = A2I(cmd);
			if (exchange)
			{
				if (slot1 == _INVALID_SLOT)
					slot1 = slot;
				else
				{
					slot2 = slot;
					if (slot1 != slot2)
					{
						object_type  tmp = *_which_obj(which_tval, slot1);
						object_type *obj1 = _which_obj(which_tval, slot1);
						object_type *obj2 = _which_obj(which_tval, slot2);

						*obj1 = *obj2;
						*obj2 = tmp;
					}
					exchange = FALSE;
					slot1 = slot2 = _INVALID_SLOT;
				}
			}
			else 
			{
				object_type *o_ptr = _which_obj(which_tval, slot);
				if (o_ptr->k_idx || (options & _ALLOW_EMPTY))
				{
					result = o_ptr;
					done = TRUE;
				}
			}
		}
	}

	screen_load();
	string_free(prompt);

	if (result)
	{
		REPEAT_PUSH(I2A(slot));
		if ((toisto) && (result->number > 0) && ((streq("Use", verb)) || (streq("Reproduce", verb))))
		{ /* Inform player */
			char o_name[MAX_NLEN];
			result->number--;
			object_desc(o_name, result, OD_COLOR_CODED);
			msg_format("You have %s.", o_name);
			result->number++;
		}
	}

	return result;
}

void _use_infusion(object_type* o_ptr, int overdose)
{
	int  boost = device_power(100) - 100;
	u32b flgs[OF_ARRAY_SIZE];
	cptr used = NULL;
	int  uses = 1;

	energy_use = alchemist_infusion_energy_use(); // at best 40!

	obj_flags(o_ptr, flgs);
	
	if (o_ptr->number < uses || o_ptr->number == 0)
	{
		if (flush_failure) flush();
		msg_print("You do not have enough infusions.");
		return;
	}

	if (o_ptr->sval == SV_POTION_DEATH || o_ptr->sval == SV_POTION_BOOZE
		|| o_ptr->sval == SV_POTION_RUINATION
		|| o_ptr->sval == SV_POTION_DETONATIONS){

		char prompt[255];
		char o_name[255];
		object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NO_PLURAL | OD_OMIT_PREFIX);
		sprintf(prompt, "Really use %s? <color:y>[y/n]</color>", o_name);
		if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
			return;
		else{
			sprintf(prompt, "Really, REALLY use %s? <color:y>[y/n]</color>", o_name);
			if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n') return;
		}
	}
	/*
	if (o_ptr->activation.type == EFFECT_IDENTIFY)
		device_available_charges = device_sp(o_ptr) / o_ptr->activation.cost;
		**/
	sound(SOUND_QUAFF);
	
	used = do_device(o_ptr, SPELL_CAST, boost);
	if (used)
	{
		if (strlen(used)) msg_print(used); // get the message of effect.

		o_ptr->number -= uses;
		stats_on_use(o_ptr, uses);

	}
	else
		energy_use = 0;
}

void alchemist_browse(void)
{
	object_type *o_ptr = _chooseInfusion("Browse", TV_POTION, _ALLOW_SWITCH | _ALLOW_EXCHANGE);
	if (o_ptr)
		obj_display(o_ptr);
}

void alchemist_cast(int tval)
{
	object_type *o_ptr;

	if (!fear_allow_magic())
	{
		msg_print("You are too scared!");
		energy_use = alchemist_infusion_energy_use();
		return;
	}

	if (!tval)
		tval = TV_POTION;

	o_ptr = _chooseInfusion("Use", tval, _ALLOW_SWITCH | _ALLOW_EXCHANGE);
	if (o_ptr)
	{
		_use_infusion(o_ptr, 1);
	}

}

static bool create_infusion(void)
{
	int infct = 1;
	object_type *dest_ptr;
	char o_name[MAX_NLEN];
	bool already_slotted;
	obj_prompt_t prompt = {0};

	prompt.prompt = "Create infusion from which potions?";
	prompt.error = "You have nothing to create infusions from.";
	prompt.filter = object_is_potion;
	prompt.where[0] = INV_PACK;
	prompt.where[1] = INV_FLOOR;

	obj_prompt(&prompt);

	if (!prompt.obj) return FALSE;

	already_slotted = FALSE;
	
	for (int i = 0; i < _MAX_INF_SLOTS; i++){ // auto-assign if there is already one.
		if (_infusions[i].sval == prompt.obj->sval && (prompt.obj->tval == TV_POTION && _infusions[i].tval == TV_POTION) && _infusions[i].number < _INFUSION_CAP){ 
			dest_ptr = &_infusions[i]; already_slotted = TRUE; break; }
	}

	if (!already_slotted){
		dest_ptr = _chooseInfusion("Replace", prompt.obj->tval, _ALLOW_EMPTY);
	}

	if (!dest_ptr)
		return FALSE;

	if ((dest_ptr->k_idx) && (dest_ptr->number >= _INFUSION_CAP) && (dest_ptr->sval == prompt.obj->sval)) {
		object_desc(o_name, dest_ptr, OD_OMIT_PREFIX);
		msg_format("This slot is already full of %s.", o_name);
		return FALSE;
	}

	if (dest_ptr->k_idx && dest_ptr->sval != prompt.obj->sval)
	{
		char prompt[255];
		object_desc(o_name, dest_ptr, OD_COLOR_CODED);
		sprintf(prompt, "Really replace %s? <color:y>[y/n]</color>", o_name);
		if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
			return FALSE;
	}

	if (prompt.obj->number > 1)
	{
		infct = get_quantity_aux(NULL, MIN(prompt.obj->number, _INFUSION_CAP), MIN(prompt.obj->number, _INFUSION_CAP));
	}

	if (infct <= 0) { msg_format("You do nothing.");  return FALSE; }
	

	if (dest_ptr->sval == prompt.obj->sval && dest_ptr->number>0){ 
		// we already got one, so just increment them! And check that there is -something-, because water has sval of 0
		bool capped = FALSE;

		if (dest_ptr->number + infct > _INFUSION_CAP){ infct = _INFUSION_CAP - dest_ptr->number; capped = TRUE; }
		if (infct < 0) infct = 0;
	
			dest_ptr->number += infct;

		if (capped == FALSE){
			msg_format("You create %d additional infusion%s.", infct, ((infct == 1) ? "" : "s"));
		}
		else {
			msg_format("You create %d additional infusion%s, reaching the limit.", infct, ((infct == 1) ? "" : "s"));
		}
	}
	// limit the infusions
	else {
		int oldct = prompt.obj->number;
		prompt.obj->number = infct;
		object_desc(o_name, prompt.obj, OD_COLOR_CODED);
		msg_format("You create %s from %s.", infct == 1 ? "an infusion" : "infusions", o_name);
		prompt.obj->number = oldct;

		*dest_ptr = *prompt.obj;
		dest_ptr->discount = 0;
		dest_ptr->number = infct;
		dest_ptr->inscription = 0;
		obj_identify_fully(dest_ptr);
		stats_on_identify(dest_ptr);
	}
	obj_dec_number(prompt.obj, infct, TRUE);
	obj_release(prompt.obj, 0);
	return TRUE;
}

static void _create_infusion_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Create Infusion");
		break;
	case SPELL_DESC:
		var_set_string(res, "Alchemically processes a potion to create an infusion - faster to use, and resistant to damage.");
		break;
	case SPELL_CAST:
		var_set_bool(res, create_infusion());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

bool _evaporate_aux(object_type *o_ptr){

	int blastType = 0;
	int power = 0;
	int dam = -1;
	int minPow = -1; int maxPow = -1;
	int rad = 2;
	int dir = 0;
	int plev = p_ptr->lev;
	cptr desc = "";
	int oldct = o_ptr->number;

	char o_name[MAX_NLEN];
	if (o_ptr->tval == TV_POTION){
		switch (o_ptr->sval){
		case SV_POTION_APPLE_JUICE:
		case SV_POTION_SALT_WATER:		// Not GF_WATER because it is a goddamn beast.
		case SV_POTION_WATER:			blastType = GF_ACID;  dam = 10 + damroll(plev / 5, 10);		    break;
		case SV_POTION_BLINDNESS:		blastType = GF_DARK_WEAK; dam = (5 + plev) * 2;					break;
		case SV_POTION_DETONATIONS:		blastType = GF_ROCKET; dam = plev * 12; rad = 3;				break; // give it bit boost for being that rare.
		case SV_POTION_DEATH:			blastType = GF_DEATH_RAY; dam = plev * 200; rad = 1;			break; // powerful as shit
		case SV_POTION_RUINATION:		blastType = GF_TIME; dam = plev * 5;  minPow = 100;				break;
		case SV_POTION_LIFE:			blastType = GF_DISP_UNDEAD;	dam = (plev * 35) + 44; maxPow = 1694;	    break; // should be super-powerful. Niche use.
		case SV_POTION_HEALING:			blastType = GF_DISP_UNDEAD;	dam = plev * 10; maxPow = 450;		break;
		case SV_POTION_STAR_HEALING:	blastType = GF_DISP_UNDEAD;	dam = plev * 15; maxPow = 850;	    break;
		case SV_POTION_SLEEP:			blastType = GF_OLD_SLEEP; dam = 25 + 7 * (plev / 10);		    break;

		case SV_POTION_DEC_CHR:
		case SV_POTION_DEC_STR:
		case SV_POTION_DEC_DEX:
		case SV_POTION_DEC_INT:
		case SV_POTION_DEC_WIS:
		case SV_POTION_DEC_CON:
			blastType = GF_TIME; dam = (5 + plev) * 2; 							break;
		case SV_POTION_CONFUSION:		blastType = GF_CONFUSION; dam = plev;							break;
		case SV_POTION_SLOWNESS:		blastType = GF_OLD_SLOW; dam = plev;							break;
		case SV_POTION_LOSE_MEMORIES:	blastType = GF_AMNESIA;	dam = plev;								break;
		case SV_POTION_BLOOD:			blastType = GF_BLOOD; dam = (30 + plev) * 2;					break;
		case SV_POTION_POISON:			blastType = GF_POIS; dam = (20 + plev) * 2; rad = 3;			break;
		case SV_POTION_BERSERK_STRENGTH:blastType = GF_BRAIN_SMASH;	dam = damroll(12, 12);				break;
		case SV_POTION_STONE_SKIN:		blastType = GF_PARALYSIS; dam = 25 + plev;						break; // turns to 'stone'
		case SV_POTION_SLIME_MOLD:		blastType = GF_STUN; dam = (5 + plev) * 2;						break;
		case SV_POTION_RESTORE_MANA:    blastType = GF_MANA; dam = (30 + plev) * 2 + 50;			    break;
		default: blastType = -1; // Other potions cannot be evaporated.
		}

		switch (o_ptr->sval){
		case SV_POTION_APPLE_JUICE:     desc = "It produces a blast of... apple juice?";	break;
		case SV_POTION_SALT_WATER:      desc = "It produces a blast of mist, and some salt.";	break;
		case SV_POTION_WATER:			desc = "It produces a blast of mist!";	break;
		case SV_POTION_BLINDNESS:		desc = "It produces a cloud of darkness";	break;
		case SV_POTION_DETONATIONS:		desc = "It produces an explosion!";	break;
		case SV_POTION_DEATH:			desc = "It produces a cloud of death!";	break;
		case SV_POTION_RUINATION:		desc = "It produces an eerie white mist!";	break;
		case SV_POTION_LIFE:			desc = "It produces a massive pillar of raw life energy!";	break;
		case SV_POTION_HEALING:			desc = "It produces a cloud of raw life energy!";	break;
		case SV_POTION_STAR_HEALING:	desc = "It produces a blast of raw life energy!";	break;
		case SV_POTION_SLEEP:			desc = "It produces a cloud of sleeping gas!";	break;
		case SV_POTION_DEC_CHR:
		case SV_POTION_DEC_STR:
		case SV_POTION_DEC_DEX:
		case SV_POTION_DEC_INT:
		case SV_POTION_DEC_WIS:
		case SV_POTION_DEC_CON:
			desc = "It a cloud of brown mist!";	break;
		case SV_POTION_CONFUSION:		desc = "It produces a scintillating cloud!";	break;
		case SV_POTION_SLOWNESS:		desc = "It produces a slow cloud!";	break;
		case SV_POTION_LOSE_MEMORIES:	desc = "It produces an ominous mist!";	break;
		case SV_POTION_BLOOD:			desc = "It produces a blast of blood!";	break;
		case SV_POTION_POISON:			desc = "It produces a cloud of poison!";	break;
		case SV_POTION_BERSERK_STRENGTH:desc = "It produces a cloud of fury!";	break;
		case SV_POTION_STONE_SKIN:		desc = "It produces a cloud of petrification!";	break;
		case SV_POTION_SLIME_MOLD:		desc = "It produces a blast of slime!";	break;
		case SV_POTION_RESTORE_MANA:    desc = "It produces a blast of raw mana!";	break;
		default: blastType = -1; // no allowing others.
		}
	}
	else if (o_ptr->tval == TV_FLASK){
		if (o_ptr->sval == SV_FLASK_OIL){
			desc = "The flask explodes into a cloud of fire!";
			dam = 12 + damroll(plev / 5, 12);	
			blastType = GF_FIRE; rad = 3;
		}
	}

	// Descriptions for sake of formatting.


	if (blastType < 0) { msg_format("You cannot evaporate this potion."); return FALSE; }
	if (!get_aim_dir(&dir)){ return FALSE; }

	// some special stuff.
	if (p_ptr->lev > 40 && blastType == GF_AMNESIA)  plev += (p_ptr->lev - 40) * 2;
	if (minPow >= 0 && minPow > dam) dam = minPow;
	if (maxPow >= 0 && maxPow < dam) dam = maxPow;
	power = spell_power(dam);

	o_ptr->number = 1;
	object_desc(o_name, o_ptr, OD_COLOR_CODED);
	msg_format("You evaporate %s. %s", o_name, desc);
	o_ptr->number = oldct;

	fire_ball_aux(
		blastType,
		dir,
		power,
		rad,
		PROJECT_FULL_DAM
		);


	return TRUE;
}

bool _object_is_evaporable(object_type *o_ptr)
{
	return (k_info[o_ptr->k_idx].tval == TV_POTION) || (k_info[o_ptr->k_idx].tval == TV_FLASK);
}

static bool evaporate(void){

	object_type *o_ptr;
	bool EvapInf = TRUE;
	bool success = FALSE;
	int komento = 0;

	char kehote[80];
	sprintf(kehote, "Evaporate [Q] Potion or [m] Infusion?\n");

	if ((REPEAT_PULL(&komento)) && ((komento == 'm') || (komento == 'M'))) {}
	else if (msg_prompt(kehote, "qm", PROMPT_DEFAULT) == 'q')
		EvapInf = FALSE;

	if (EvapInf == FALSE){
		obj_prompt_t prompt = {0};
		prompt.prompt = "Evaporate which potion?";
		prompt.error = "You have nothing to evaporate.";
		prompt.filter = _object_is_evaporable;
		prompt.where[0] = INV_PACK;
		prompt.where[1] = INV_FLOOR;

		obj_prompt(&prompt);

		if (!prompt.obj) return FALSE;

		success = _evaporate_aux(prompt.obj);

		if (success == TRUE) {
			prompt.obj->number--;
			obj_release(prompt.obj, 0);
		}
		else return FALSE;
	} 
	else {
		REPEAT_PUSH('m');
		o_ptr = _chooseInfusion("Evaporate", TV_POTION, 0);
		if (!o_ptr){ return FALSE; }

		if (o_ptr->number < 1){
			msg_format("There's nothing to evaporate.");
			return FALSE;
		}

		success = _evaporate_aux(o_ptr);
		if (success == TRUE){
			o_ptr->number--;
		}

	}
	energy_use = 100;
	return TRUE;
}

static void _evaporate_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Evaporate");
		break;
	case SPELL_DESC:
		var_set_string(res, "Evaporates a potion, creating a burst that may harm or benefit creatures.");
		break;
	case SPELL_CAST:
		var_set_bool(res, evaporate());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

bool alchemist_break_down_aux(object_type *o_ptr, int ct){
	char o_name[MAX_NLEN];
	int formulaCost = _FindFormula(o_ptr->sval)->cost;
	int newchem, cost = (ct * formulaCost) / 2;
	int tier = _FindFormula(o_ptr->sval)->ctier;
	bool singular = (o_ptr->number == 1);
	if (_CHEM[tier] + cost > _MAX_CHEM){
		char prompt[255];
		object_desc(o_name, o_ptr, OD_OMIT_PREFIX);
		sprintf(prompt, "Really break down %s? %s chemicals will be lost due to lack of room. <color:y>[y/n]</color>", o_name, _CHEM[tier] == _MAX_CHEM ? "All" : "Some of the");
		if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
			return FALSE;
	}

	newchem = MIN(cost, _MAX_CHEM - _CHEM[tier]);
	_CHEM[tier] += newchem;

	if (newchem) msg_format("You break down the potion%s and gain %d %s chemicals, for a total of %d.", (singular ? "" : "s"), newchem, _tiername[tier], _CHEM[tier]);
	else msg_format("You break down the potion%s, losing all the chemicals in %s.", singular ? "" : "s", singular ? "it" : "them");
	return TRUE;
}

static bool break_down_potion(void){
	int ct;
	obj_prompt_t prompt = {0};
	bool success = FALSE;

	prompt.prompt = "Break down which potions?";
	prompt.error = "You have nothing to break down.";
	prompt.filter = object_is_potion;
	prompt.where[0] = INV_PACK;
	prompt.where[1] = INV_FLOOR;

	obj_prompt(&prompt);

	if (!prompt.obj) return FALSE;

	ct = (prompt.obj->number == 1 ? 1 : get_quantity_aux(NULL, prompt.obj->number, prompt.obj->number));
	if (ct <= 0) return FALSE;

	if (prompt.obj->sval == SV_POTION_WATER) {
		if(randint0(100)<8) msg_print("It's just H2O, funny guy.");
		else if (one_in_(12)) msg_print("You attempt to break down the dihydrogen monoxide, but fail.");
		else msg_print("It's just plain water.");

		return FALSE;
	}

	success = alchemist_break_down_aux(prompt.obj, ct);
	
	if (success) {
		obj_dec_number(prompt.obj, ct, TRUE);
		obj_release(prompt.obj, 0);
	}
	return TRUE;
}

static void _break_down_potion_spell(int cmd, variant *res){
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Break Down Potion");
		break;
	case SPELL_DESC:
		var_set_string(res, "Breaks down a potion to its base chemicals.");
		break;
	case SPELL_CAST:
		var_set_bool(res, break_down_potion());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

void _reproduceInf(object_type* o_ptr){
	
	u32b flgs[OF_ARRAY_SIZE];
	int infct, cost, tier, limit = _INFUSION_CAP;
	char o_name[MAX_NLEN];
	char prompt[80];

	energy_use = 100;

	obj_flags(o_ptr, flgs);

	if (o_ptr->number >= _INFUSION_CAP)
	{
		if (flush_failure) flush();
		msg_print("This slot is already full."); // perhaps make it sound more flavourful...
		return;
	}
	else if (!o_ptr){
		if (flush_failure) flush();
		msg_print("There's nothing to reproduce."); 
		return;
	}

	sound(SOUND_QUAFF); // what the hell would be the sound anyway?


	limit = _INFUSION_CAP - o_ptr->number;
	if (limit < 0) limit = 0;

	infct = get_quantity(NULL, MIN(_INFUSION_CAP, limit));
	if (infct <= 0) return;

	cost = infct * _FindFormula(o_ptr->sval)->cost;
	tier = _FindFormula(o_ptr->sval)->ctier;
	if (_FindFormula(o_ptr->sval)->minLv > _AlchemistSkill()){
		msg_format("This infusion is beyond your skills to reproduce.");
		return;
	}
	
	if (infct < 1 || tier<0 || tier>2){ // do notting
		return;
	}

	object_desc(o_name, o_ptr, OD_COLOR_CODED);
	sprintf(prompt, "Reproduction would cost %d chemicals. Are you sure? <color:y>[y/n]</color>", cost);
	if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
		return;

	if (cost > _CHEM[tier]){ 
		msg_format("You do not have enough %s chemicals.", _tiername[tier]);
		return;
	}

	if (infct > 0)
	{
		msg_format("You recreate %d infusion%s.", infct, ((infct == 1) ? "" : "s"));
		o_ptr->number += infct;
		_CHEM[tier] -= cost;
	}
	else
		energy_use = 0;

}

static bool reproduceInfusion(void){
	object_type *o_ptr;
	o_ptr = _chooseInfusion("Reproduce", TV_POTION, 0);
	if (o_ptr){ _reproduceInf(o_ptr); return TRUE; }
	return FALSE;
}

static void _reproduce_infusion_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Reproduce Infusion");
		break;
	case SPELL_DESC:
		var_set_string(res, "Uses chemicals to create an infusion based on an existing infusion.");
		break;
	case SPELL_CAST:
		var_set_bool(res, reproduceInfusion());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

/* Note that we don't check here whether p_ptr->pclass == CLASS_ALCHEMIST.
 * The game probably won't crash if we call alchemist_set_hero() from a
 * non-alchemist, but it's still a good idea to check first... */
void alchemist_set_hero(bool *notice, int uus_arvo, bool normal_hero)
{
    u16b *kohde = (normal_hero ? &_alchemist_hero : &_alchemist_shero);
    int tulos;
    bool oli = ((*kohde > 0) && (notice)); /* checks for NULL notice */
    if (uus_arvo < 1)
    {
        if (oli) *notice = TRUE;
        *kohde = 0;
        return;
    }
    tulos = *kohde;
    tulos += uus_arvo;
    tulos -= (normal_hero ? p_ptr->hero : p_ptr->shero);
    if (tulos < 0) tulos = 0;
    if ((!tulos) && (oli)) *notice = TRUE;
    if ((tulos > 0) && (!oli) && (notice)) *notice = TRUE;
    if (tulos > 200) tulos = 200;
    tulos = MIN(tulos, uus_arvo);
    *kohde = (u16b)tulos;
}

void alchemist_super_potion_effect(int sval){

	switch (sval)
	{
		case SV_POTION_ENLIGHTENMENT: set_tim_esp(100 + randint0(p_ptr->lev * 2), FALSE); break;
		case SV_POTION_STAR_ENLIGHTENMENT: set_tim_esp(300 + randint0(p_ptr->lev * 6), FALSE); break;
		case SV_POTION_CLARITY: set_confused(0, TRUE); break;
		case SV_POTION_GREAT_CLARITY: set_confused(0, TRUE); set_stun(0, TRUE); set_image(0, TRUE); break;
	}

}

static void _calc_bonuses(void) {
	int boost = 0;
	if (p_ptr->lev >= 20) 
	{
		add_flag(p_ptr->weapon_info[0].flags, OF_BRAND_POIS);
		add_flag(p_ptr->weapon_info[1].flags, OF_BRAND_POIS);
		add_flag(p_ptr->shooter_info.flags, OF_BRAND_POIS);
	}
	if (p_ptr->lev >= 40)
	{
		add_flag(p_ptr->weapon_info[0].flags, OF_BRAND_ACID);
		add_flag(p_ptr->weapon_info[1].flags, OF_BRAND_ACID);
		add_flag(p_ptr->shooter_info.flags, OF_BRAND_ACID);
	}

	if (_alchemist_shero){ // extra benefits from things.
		boost = 2 + p_ptr->lev / 5;

		p_ptr->weapon_info[0].xtra_blow += py_prorata_level_aux(75, 1, 1, 1);
		p_ptr->weapon_info[1].xtra_blow += py_prorata_level_aux(75, 1, 1, 1);

		p_ptr->to_h_m += boost;
		p_ptr->to_d_m += boost;

		p_ptr->pspeed += 4;
		p_ptr->shooter_info.xtra_shot += p_ptr->lev * 120 / 80;
	}
	else if (_alchemist_hero){
		boost = 1 + p_ptr->lev / 10;
		p_ptr->pspeed += 2;
		p_ptr->to_h_m += boost;
		p_ptr->to_d_m += boost;
		p_ptr->shooter_info.xtra_shot += p_ptr->lev * 60 / 80;
	}

}

static bool _on_destroy_object(object_type *o_ptr)
{
	if (object_is_potion(o_ptr))
	{
		char o_name[MAX_NLEN];
		object_desc(o_name, o_ptr, OD_COLOR_CODED);
		msg_format("You attempt to break down %s. ", o_name);
		alchemist_break_down_aux(o_ptr, o_ptr->number);
		return TRUE;
	}
	return FALSE;
}



static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
	if (p_ptr->lev >= 20)
		add_flag(flgs, OF_BRAND_POIS);
	if (p_ptr->lev >= 40)
		add_flag(flgs, OF_BRAND_ACID);
}

static void _load_list(savefile_ptr file)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = _infusions + i;
		memset(o_ptr, 0, sizeof(object_type));
	}
	_infusions_init = TRUE;

	while (1)
	{
		object_type *o_ptr;
		i = savefile_read_u16b(file);
		if (i == 0xFFFF) break;
		assert(0 <= i && i < _MAX_INF_SLOTS);
		o_ptr = _infusions + i;
		rd_item(file, o_ptr);
		assert(o_ptr->k_idx);
	}

	for (i = 0; i < _CTIER_MAX; i++){
		_CHEM[i] = savefile_read_s32b(file);
	}
	if (savefile_is_older_than(file, 7, 0, 9, 0)) return;
	_alchemist_hero = savefile_read_u16b(file);
	_alchemist_shero = savefile_read_u16b(file);
}

static void _load_player(savefile_ptr file)
{
	_load_list(file);
}

static void _save_list(savefile_ptr file)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = _infusions + i;
		if (o_ptr->k_idx)
		{
			savefile_write_u16b(file, (u16b)i);
			wr_item(file, o_ptr);
		}
	}
	savefile_write_u16b(file, 0xFFFF); /* sentinel */
	for (i = 0; i < _CTIER_MAX; i++){
		savefile_write_s32b(file, (s32b)_CHEM[i]); // save chemical count.
	}
}

static void _save_player(savefile_ptr file)
{
	_save_list(file);
	savefile_write_u16b(file, _alchemist_hero);
	savefile_write_u16b(file, _alchemist_shero);
}

/* Character Dump */
static void _dump_list(doc_ptr doc)
{
	int i;
	char o_name[MAX_NLEN];
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = _infusions + i;
		if (o_ptr->k_idx)
		{
			object_desc(o_name, o_ptr, OD_COLOR_CODED);
			doc_printf(doc, "%c) %s\n", I2A(i), o_name);
		}
		else
			doc_printf(doc, "%c) (Empty)\n", I2A(i));
	}
	doc_newline(doc);
}

static void _character_dump(doc_ptr doc)
{
	doc_printf(doc, "<topic:Alchemist>============================= Created <color:keypress>I</color>nfusions ============================\n\n");

	_dump_list(doc);

	doc_newline(doc);
}

/* Weight of the infusions */
int _calc_extra_weight(obj_p p)
{
    int i, laskuri = 0;
    if (!_infusions_init) return 0;
    for (i = 0; i < _MAX_INF_SLOTS; i++)
    {
        object_type *o_ptr = _which_obj(TV_POTION, i);
        if ((!o_ptr) || (!o_ptr->k_idx) || (!o_ptr->number)) continue;
        laskuri += o_ptr->number;
    }

    return (laskuri / 2);
}

static caster_info * _caster_info(void)
{
	static caster_info me = { 0 };
	static bool init = FALSE;
	if (!init)
	{
		me.magic_desc = "alchemy";
		me.encumbrance.max_wgt = 1000;
		me.encumbrance.weapon_pct = 20;
		me.encumbrance.enc_wgt = 1200;
		init = TRUE;
	}
	me.which_stat = A_INT;
	return &me;
}

/* Class Info */
static int _get_powers(spell_info* spells, int max)
{
	int ct = 0;

	spell_info* spell = &spells[ct++];
	spell->level = 1;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _create_infusion_spell;

	spell = &spells[ct++];
	spell->level = 1;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _reproduce_infusion_spell;

	spell = &spells[ct++];
	spell->level = 1;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _break_down_potion_spell;

	spell = &spells[ct++];
	spell->level = 5;
	spell->cost = 8;
	spell->fail = calculate_fail_rate(spell->level, 25, p_ptr->stat_ind[A_INT]); 
	spell->fn = _evaporate_spell;

	spell = &spells[ct++];
	spell->level = 20;
	spell->cost = 5;
	spell->fail = calculate_fail_rate(spell->level, 30, p_ptr->stat_ind[A_INT]);
	spell->fn = alchemy_spell;

	return ct;
}

class_t *alchemist_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;

	/* static info never changes */
	if (!init)
	{           
		/* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 30, 30, 34, 6, 50, 24, 52, 52 };
		skills_t xs = { 15, 9, 10, 0, 0, 0, 17, 17 };

		me.name = "Alchemist";
		me.desc = "Alchemists are masters of tinctures, concoctions and "
			"infusions. They can prepare themselves set infusions from "
			"potions that replicate the effect - without consuming inventory "
			"space or being in danger of shattering. They are reasonably good "
			"at melee, especially with the right potions, though they cannot "
			"rival melee specialists. "
			"Even bad potions are useful in their hands, as weapons or "
			"ingredients. Their other abilities include creating copies "
			"of potions and turning items to gold. They require intelligence "
			"for some of their abilities.\n";

		me.stats[A_STR] = 0;
		me.stats[A_INT] = 2;
		me.stats[A_WIS] = -1;
		me.stats[A_DEX] = 2;
		me.stats[A_CON] = 0;
		me.stats[A_CHR] = -2;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 101;
		me.base_hp = 12;
		me.exp = 135;
		me.pets = 30;

		me.birth = _birth;
		me.get_powers = _get_powers;
		me.calc_bonuses = _calc_bonuses;
		me.character_dump = _character_dump;
		me.get_flags = _get_flags;

		me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG |
			CLASS_SENSE2_STRONG;

		me.caster_info = _caster_info;
		me.load_player = _load_player;
		me.save_player = _save_player;
		me.destroy_object = _on_destroy_object;
		me.calc_extra_weight = _calc_extra_weight;

		init = TRUE;
	}

	return &me;
}
