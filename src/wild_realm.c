#include "angband.h"

/* TODO: Player counters in general could use a revamp. I'm too lazy to fix everything
   since there are literally hundreds of existing counters, so I'll just hack in more crud ... */

typedef void(*_wild_fn)(void);

typedef struct {
    int type;
    _wild_fn on_fn;    /* Called before the counter is turned on */
    _wild_fn off_fn; /* Called after the counter is turned off */
    s32b redraw_flags;
    s32b update_flags;
} _counter_type;

/* The trouble with status messages is that you can get multiple sources of a power, so we need
   to check all other possibilites. Annoying ... Note that many of the current set_tim_foo()
   routines now report incorrectly. They are coded obtusely, so I won't even attempt to fix. 
   
   Note: It wasn't clear to me whether wild buffs should clobber normal buff. If so, this means
   that Haste and Double Resist are pretty hard to come by!  For now, they will clobber (i.e., when
   wild double resist shuts off, so too does normal double resist), but we can change this later
   if desired. */
void _wild_infravision_on(void) {
    if (!IS_TIM_INFRA())
        msg_print("Your eyes begin to tingle!");
}

void _wild_infravision_off(void) {
    if (p_ptr->tim_infra)
        set_tim_infra(0, TRUE);
    if (!IS_TIM_INFRA())
        msg_print("Your eyes stop tingling.");
}

void _wild_bless_on(void) {
    if (!IS_BLESSED())
        msg_print("You feel righteous!");
}

void _wild_bless_off(void) {
    if (p_ptr->blessed)
        set_blessed(0, TRUE);
    if (!IS_BLESSED())
        msg_print("The prayer has expired.");
}

void _wild_berserk_on(void) {
    if (!IS_SHERO())
        msg_print("You feel like a killing machine!");
}

void _wild_berserk_off(void) {
    if (p_ptr->shero)
        set_shero(0, TRUE);
    if (!IS_SHERO())
        msg_print("You feel less Berserk.");
}

void _wild_speed_on(void) {
    if (!IS_FAST() && !IS_LIGHT_SPEED())
        msg_print("You feel yourself moving much faster!");
}

void _wild_speed_off(void) {
    if (p_ptr->fast)
        set_fast(0, TRUE);
    if (!IS_FAST() && !IS_LIGHT_SPEED())
        msg_print("You feel yourself slow down.");
}

void _wild_esp_on(void) {
    if (!IS_TIM_ESP())
        msg_print("You feel your consciousness expand!");
}

void _wild_esp_off(void) {
    if (p_ptr->tim_esp)
        set_tim_esp(0, TRUE);
    if (!IS_TIM_ESP())
        msg_print("Your consciousness contracts again.");
}

void _wild_prot_evil_on(void) {
    if (!IS_PROT_EVIL())
        msg_print("You feel safe from evil!");
}

void _wild_prot_evil_off(void) {
    if (p_ptr->protevil)
        set_protevil(0, TRUE);
    if (!IS_PROT_EVIL())
        msg_print("You no longer feel safe from evil.");
}

void _wild_magic_resist_on(void) {
    if (!IS_RESIST_MAGIC())
        msg_print("You feel resistant to magic!");
}

void _wild_magic_resist_off(void) {
    if (p_ptr->resist_magic)
        set_resist_magic(0, TRUE);
    if (!IS_RESIST_MAGIC())
        msg_print("You no longer feel resistant to magic.");
}

void _wild_resist_on(void) {
    if (!IS_RESIST())
        msg_print("You feel resistant to the elements!");
}

void _wild_resist_off(void) {
    if (p_ptr->oppose_acid)
        set_oppose_acid(0, TRUE);
    if (p_ptr->oppose_cold)
        set_oppose_cold(0, TRUE);
    if (p_ptr->oppose_elec)
        set_oppose_elec(0, TRUE);
    if (p_ptr->oppose_fire)
        set_oppose_fire(0, TRUE);
    if (p_ptr->oppose_pois)
        set_oppose_pois(0, TRUE);
    if (!IS_RESIST())
        msg_print("You no longer feel resistant to the elements.");
}

void _wild_stone_skin_on(void) {
    if (!IS_STONE_SKIN())
        msg_print("Your skin turns to stone!");
}

void _wild_stone_skin_off(void) {
    if (p_ptr->shield)
        set_shield(0, TRUE);
    if (!IS_STONE_SKIN())
        msg_print("Your skin turns to normal.");
}

void _wild_passwall_on(void) {
    if (!IS_PASSWALL())
        msg_print("You take on an ethereal form!");
}

void _wild_passwall_off(void) {
    if (p_ptr->kabenuke)
        set_kabenuke(0, TRUE);
    if (!IS_PASSWALL())
        msg_print("You are no longer ethereal.");
}

void _wild_revenge_on(void) {
    if (!IS_REVENGE())
        msg_print("You feel like a keeper of commandments!");
}

void _wild_revenge_off(void) {
    if (p_ptr->tim_eyeeye)
        set_tim_eyeeye(0, TRUE);
    if (!IS_REVENGE())
        msg_print("You feel like letting bygones be bygones.");
}

void _wild_invulnerability_on(void) {
    if (!IS_INVULN())
        msg_print("Invulnerability!");
}

void _wild_invulnerability_off(void) {
    if (p_ptr->invuln)
        set_invuln(0, TRUE);
    if (!IS_INVULN())
        msg_print("The invulnerability wears off.");    /* no energy penalty */
}

void _wild_wraith_on(void) {
    if (!IS_WRAITH())
        msg_print("You leave the physical world and turn into a wraith-being!");
}

void _wild_wraith_off(void) {
    if (p_ptr->wraith_form)
        set_wraith_form(0, TRUE);
    if (!IS_WRAITH())
        msg_print("You feel opaque.");
}

_counter_type _types[] = {
    { WILD_INFRAVISION, _wild_infravision_on, _wild_infravision_off, PR_STATUS, PU_BONUS | PU_MONSTERS },
    { WILD_BLESS, _wild_bless_on, _wild_bless_off, PR_STATUS, PU_BONUS },
    { WILD_BERSERK, _wild_berserk_on, _wild_berserk_off, PR_STATUS, PU_BONUS | PU_HP },
    { WILD_SPEED, _wild_speed_on, _wild_speed_off, PR_STATUS, PU_BONUS },
    { WILD_ESP, _wild_esp_on, _wild_esp_off, PR_STATUS, PU_BONUS | PU_MONSTERS },
    { WILD_PROT_EVIL, _wild_prot_evil_on, _wild_prot_evil_off, PR_STATUS, 0 },
    { WILD_MAGIC_RESIST, _wild_magic_resist_on, _wild_magic_resist_off, PR_STATUS, PU_BONUS },
    { WILD_RESIST, _wild_resist_on, _wild_resist_off, PR_STATUS, PU_BONUS },
    { WILD_STONE_SKIN, _wild_stone_skin_on, _wild_stone_skin_off, PR_STATUS, PU_BONUS },
    { WILD_PASSWALL, _wild_passwall_on, _wild_passwall_off, PR_STATUS, PU_BONUS },
    { WILD_REVENGE, _wild_revenge_on, _wild_revenge_off, PR_STATUS, 0 },
    { WILD_INVULN, _wild_invulnerability_on, _wild_invulnerability_off,  PR_STATUS | PR_MAP, PU_BONUS | PU_MONSTERS },
    { WILD_WRAITH, _wild_wraith_on, _wild_wraith_off,  PR_STATUS, PU_BONUS | PU_HP },
    { WILD_LIGHT_SPEED, _wild_speed_on, _wild_speed_off, PR_STATUS, PU_BONUS },
    { -1, 0, 0, 0, 0 },
};

int _find_type(int type)
{
    int i;
    for (i = 0; ; i++)
    {
        if (_types[i].type < 0) break;
        if (_types[i].type == type) return i;
    }
    return -1;
}

int _find_counter(int power)
{
    int i;
    for (i = 0; i < MAX_WILD_COUNTERS; i++)
    {
        if (p_ptr->wild_counters[i].type == power) return i;
    }
    return -1;
}

int _get_free_counter(void)
{
    int i;
    for (i = 0; i < MAX_WILD_COUNTERS; i++)
    {
        if (p_ptr->wild_counters[i].type == 0) return i;
    }
    return -1;
}

int _get_random_counter(void)
{
    return randint0(MAX_WILD_COUNTERS);
}

void _reset_counter(int i)
{
    int j;
    if (p_ptr->wild_counters[i].type == 0) return;

    j = _find_type(p_ptr->wild_counters[i].type);
    p_ptr->wild_counters[i].type = 0;
    p_ptr->wild_counters[i].counter = 0;
    
    if (j >= 0)
    {
        _types[j].off_fn();
        p_ptr->redraw |= _types[j].redraw_flags;
        p_ptr->update |= _types[j].update_flags;
        if (disturb_state) disturb(0, 0);
        handle_stuff();
    }
}

int _get_counter(void)
{
    int i = _get_free_counter();
    if (i < 0)
    {
        i = _get_random_counter();
        _reset_counter(i);
    }
    return i;
}

bool wild_has_power(int power)
{
    return _find_counter(power) >= 0;
}

int _get_random_power(void)
{
    int power = 0;

    for (;;)
    {
        int n = randint0(100);
        if (n < 10) power = WILD_INFRAVISION;
        else if (n < 20) power = WILD_BLESS;
        else if (n < 30) power = WILD_BERSERK;
        else if (n < 39) power = WILD_SPEED;
        else if (n < 48) power = WILD_ESP;
        else if (n < 57) power = WILD_PROT_EVIL;
        else if (n < 65) power = WILD_MAGIC_RESIST;
        else if (n < 73) power = WILD_RESIST;
        else if (n < 80) power = WILD_STONE_SKIN;
        else if (n < 86) power = WILD_PASSWALL;
        else if (n < 91) power = WILD_REVENGE;
        else if (n < 95) power = WILD_INVULN;
        else if (n < 98) power = WILD_WRAITH;
        else power = WILD_LIGHT_SPEED;

        if (!wild_has_power(power)) break;
    }

    return power;
}

void wild_weapon_strike(void)
{
    /* Note, if we get a counter slot first, then you can just regain what you lost.
       So getting the power first assures you are getting something new */
    int power = _get_random_power();
    int slot = _get_counter();
    int info = _find_type(power);

    if (info >= 0)
        _types[info].on_fn();

    p_ptr->wild_counters[slot].type = power;
    p_ptr->wild_counters[slot].counter = 2;

    if (info >= 0)
    {
        p_ptr->redraw |= _types[info].redraw_flags;
        p_ptr->update |= _types[info].update_flags;
        if (disturb_state) disturb(0, 0);
        handle_stuff();
    }
}

void wild_decrement_counters(void)
{
    int i;
    for (i = 0; i < MAX_WILD_COUNTERS; i++)
    {
        if (p_ptr->wild_counters[i].counter)
        {
            p_ptr->wild_counters[i].counter--;
            if (p_ptr->wild_counters[i].counter <= 0)
                _reset_counter(i);
        }
    }
}

void wild_reset_counters(void)
{
    int i;
    for (i = 0; i < MAX_WILD_COUNTERS; i++)
    {
        p_ptr->wild_counters[i].type = 0;
        p_ptr->wild_counters[i].counter = 0;
    }
}

void wild_dispel_player(void)
{
    int i;
    for (i = 0; i < MAX_WILD_COUNTERS; i++)
        _reset_counter(i);
}

void wild_reset_counter(int power)
{
    int i = _find_counter(power);
    if (i >= 0) _reset_counter(i);
}
