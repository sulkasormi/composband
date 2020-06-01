/*
 * Mutations that cannot be activated as normal spells.
 * These mutations might be timed effects, or just things
 * like "Horns" that you simply have. They might be augmentations
 * like "Super Human He-man".
 *
 * We are still implementing all mutations as spells for
 * uniformity.
 *
 * Again, spells are (stateless) objects, implemented as functions.
 */

#include "angband.h"

/*
void foo_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "");
        break;
    case SPELL_GAIN_MUT:
        msg_print("");
        break;
    case SPELL_LOSE_MUT:
        msg_print("");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CALC_BONUS:
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
*/


void albino_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Albino");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You turn into an albino! You feel frail...");
        mut_lose(MUT_RESILIENT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer an albino!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are albino (-4 CON).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void alcohol_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Alcohol");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your body starts producing alcohol!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your body stops producing alcohol!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your body produces alcohol.");
        break;
    case SPELL_PROCESS:
        if (randint1(6400) == 321)
        {
            if (!res_save_default(RES_CONF) && !res_save_default(RES_CHAOS))
            {
                disturb(0, 0);
                p_ptr->redraw |= PR_EXTRA;
                msg_print("You feel a SSSCHtupor cOmINg over yOu... *HIC*!");
            }

            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint0(20) + 15, FALSE);

            if (!res_save_default(RES_CHAOS))
            {
                if (one_in_(20))
                {
                    msg_print(NULL);
                    if (one_in_(3)) lose_all_info();
                    else wiz_dark();
                    teleport_player_aux(100, TELEPORT_NONMAGICAL | TELEPORT_PASSIVE);
                    wiz_dark();
                    msg_print("You wake up somewhere with a sore head...");
                    msg_print("You can't remember a thing, or how you got here!");
                }
                else if (one_in_(3))
                {
                    msg_print("Thishcischs GooDSChtuff!");
                    set_image(p_ptr->image + randint0(15) + 15, FALSE);
                }
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void ambidexterity_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ambidexterity");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel like dual wielding.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel like dual wielding.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are ambidextrous.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives a bonus equivalent to 15 percentage points of dual-wielding skill.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void arcane_mastery_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Arcane Mastery");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain arcane insights.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel your arcane mastery slipping away.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have arcane mastery.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Reduces spell failure chances by 3 percentage points (maximum and minimum failure chances are still respected).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void arthritis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Arthritis");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your joints suddenly hurt.");
        mut_lose(MUT_LIMBER);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your joints stop hurting.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your joints ache constantly (-3 DEX).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void astral_guide_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Astral Guide");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You teleport quickly!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer teleport quickly!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are an astral guide (Teleport costs less energy).");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Reduces time needed for player-induced teleports by 67% to 70%.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void attract_animal_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Attract Animals");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start attracting animals.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop attracting animals.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You attract animals.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(7000))
        {
            bool pet = one_in_(3);
            u32b mode = PM_ALLOW_GROUP;

            if (pet) mode |= PM_FORCE_PET;
            else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

            if (summon_specific((pet ? -1 : 0), py, px, dun_level, SUMMON_ANIMAL, mode))
            {
                msg_print("You have attracted an animal!");
                disturb(0, 0);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void attract_demon_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Attract Demons");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start attracting demons.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop attracting demons.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You attract demons.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && (randint1(6666) == 666))
        {
            bool pet = one_in_(6);
            u32b mode = PM_ALLOW_GROUP;

            if (pet) mode |= PM_FORCE_PET;
            else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

            if (summon_specific((pet ? -1 : 0), py, px,
                        dun_level, SUMMON_DEMON, mode))
            {
                msg_print("You have attracted a demon!");
                disturb(0, 0);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void attract_dragon_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Attract Dragon");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start attracting dragons.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop attracting dragons.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You attract dragons.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(3000))
        {
            bool pet = one_in_(5);
            u32b mode = PM_ALLOW_GROUP;

            if (pet) mode |= PM_FORCE_PET;
            else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

            if (summon_specific((pet ? -1 : 0), py, px, dun_level, SUMMON_DRAGON, mode))
            {
                msg_print("You have attracted a dragon!");
                disturb(0, 0);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void black_marketeer_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Black Marketeer");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become an agent of the black market!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer an agent of the black market.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are an agent of the black market.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives you favorable pricing in the black market (all items half-price).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void blank_face_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blank Face");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your face becomes completely featureless!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your facial features return.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your face is featureless (-1 CHR).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void bad_luck_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Black Aura");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel a strange kinship with Dennis the rodmaster!");
        mut_lose(MUT_GOOD_LUCK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your black aura swirls and fades.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "There is a black aura surrounding you.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void beak_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Beak");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your mouth turns into a sharp, powerful beak!");
        mut_lose(MUT_TRUNK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your mouth reverts to normal!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have a beak.");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 2;
        a.ds = 4;
        a.weight = 30;
        a.blows = 100;
        a.msg = "You peck.";
        a.name = "Beak";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void berserk_rage_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Berserk Rage");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become subject to fits of berserk rage!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer subject to fits of berserk rage!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are subject to berserker fits.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->shero && one_in_(3000))
        {
            disturb(0, 0);
            cast_berserk();
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cerebral_pultitis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cerebral Pultitis");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your brain feels like porridge...");
        mut_lose(MUT_HYPER_INT);
        mut_lose(MUT_MORONIC);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your brain no longer feels like porridge.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You suffer from cerebral pultitis (-3 INT).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void chaos_deity_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Chaos Deity");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You attract the notice of a chaos deity!");
        /* In case it isn't obvious, every character has a chaos deity assigned at birth. */
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the attention of the chaos deities.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Chaos deities give you gifts.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cowardice_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cowardice");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become an incredible coward!");
        mut_lose(MUT_FEARLESS);
        mut_lose(MUT_NO_INHIBITIONS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer an incredible coward!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are subject to cowardice.");
        break;
    case SPELL_PROCESS:
        if (!res_save_default(RES_FEAR) && (randint1(3000) == 13))
        {
            disturb(0, 0);
            msg_print("It's so dark... so scary!");
            fear_add_p(FEAR_SCARED);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cult_of_personality_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cult of Personality");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain control over your enemy's summons!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose control over your enemy's summons!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Summoned monsters are sometimes friendly.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Summoned hostile monsters have an up to 50% chance to become friendly or even pets, depending on their level and the player's charisma.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->cult_of_personality = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void demonic_grasp_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Demonic Grasp");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You have a firm grasp on your magical devices.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose your firm grasp on your magical devices.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You resist charge draining.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives immunity to charge draining.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_breath_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Deadly Breath");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You breathe more powerfully.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Your breath will become more deadly.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_kin_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_MUT_DESC:
        var_set_string(res, "You can summon kin.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You will be able to summon aid from dragons.");
        break;
    default:
        summon_kin_spell(cmd, res);
        break;
    }
}

void draconian_lore_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ancient Knowledge");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Items automatically identify as you pick them up.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Items will automatically identify as you pick them up.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->auto_id = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_magic_resistance_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Resistance");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are resistant to magic.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You will gain an improved saving throw versus magical attacks.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.sav += 15 + p_ptr->lev / 5;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_metamorphosis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Metamorphosis");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have metamorphosed into a Dragon!");
        break;
    case SPELL_HELP_DESC:
        if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
            var_set_string(res, "Your body will transform into a dragon (e.g. 6 ring slots and no weapons). WARNING: You will lose access to martial arts!");
        else
            var_set_string(res, "Your body will transform into a dragon (e.g. 6 ring slots and no weapons)");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_regen_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Regeneration");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are regenerating extremely quickly.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You will regenerate much more quickly.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->regen += 150;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_resistance_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resistance");
        break;
    case SPELL_MUT_DESC:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            var_set_string(res, "You gain extra fire resistance.");
            break;
        case DRACONIAN_WHITE:
            var_set_string(res, "You gain extra cold resistance.");
            break;
        case DRACONIAN_BLUE:
            var_set_string(res, "You gain extra electricity resistance.");
            break;
        case DRACONIAN_BLACK:
            var_set_string(res, "You gain extra acid resistance.");
            break;
        case DRACONIAN_GREEN:
            var_set_string(res, "You gain extra poison resistance.");
            break;
        case DRACONIAN_BRONZE:
            var_set_string(res, "You gain extra confusion resistance.");
            break;
        case DRACONIAN_GOLD:
            var_set_string(res, "You gain extra sound resistance.");
            break;
        case DRACONIAN_CRYSTAL:
            var_set_string(res, "You gain extra shards resistance.");
            break;
        case DRACONIAN_SHADOW:
            var_set_string(res, "You gain extra nether resistance.");
            break;
        }
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You will gain extra resistance of the appropriate type (e.g. Fire or Cold).");
        break;
    case SPELL_CALC_BONUS:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            res_add(RES_FIRE);
            break;
        case DRACONIAN_WHITE:
            res_add(RES_COLD);
            break;
        case DRACONIAN_BLUE:
            res_add(RES_ELEC);
            break;
        case DRACONIAN_BLACK:
            res_add(RES_ACID);
            break;
        case DRACONIAN_GREEN:
            res_add(RES_POIS);
            break;
        case DRACONIAN_BRONZE:
            res_add(RES_CONF);
            break;
        case DRACONIAN_GOLD:
            res_add(RES_SOUND);
            break;
        case DRACONIAN_CRYSTAL:
            res_add(RES_SHARDS);
            break;
        case DRACONIAN_SHADOW:
            res_add(RES_NETHER);
            break;
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_shield_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dragon Skin");
        break;
    case SPELL_MUT_DESC:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            var_set_string(res, "Dragon Skin: You gain +15 AC and an aura of fire");
            break;
        case DRACONIAN_WHITE:
            var_set_string(res, "Dragon Skin: You gain +15 AC and an aura of cold");
            break;
        case DRACONIAN_BLUE:
            var_set_string(res, "Dragon Skin: You gain +15 AC and an aura of electricity");
            break;
        case DRACONIAN_CRYSTAL:
            var_set_string(res, "Dragon Skin: You gain +10 AC and an aura of shards");
            break;
        default:
            var_set_string(res, "Dragon Skin: You gain +25 AC");
        }
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You will gain extra armor class and a defensive aura if available.");
        break;
    case SPELL_CALC_BONUS:
    {
        int amt = 25;
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            p_ptr->sh_fire++;
            amt = 15;
            break;
        case DRACONIAN_WHITE:
            p_ptr->sh_cold++;
            amt = 15;
            break;
        case DRACONIAN_BLUE:
            p_ptr->sh_elec++;
            amt = 15;
            break;
        case DRACONIAN_CRYSTAL:
            p_ptr->sh_shards++;
            amt = 10;
            break;
        }
        p_ptr->to_a += amt;
        p_ptr->dis_to_a += amt;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_strike_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dragon Strike");
        break;
    case SPELL_DESC:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED: var_set_string(res, "Attack an adjacent opponent with a fiery blow."); break;
        case DRACONIAN_WHITE: var_set_string(res, "Attack an adjacent opponent with an icy blow."); break;
        case DRACONIAN_BLUE: var_set_string(res, "Attack an adjacent opponent with a shocking blow."); break;
        case DRACONIAN_BLACK: var_set_string(res, "Attack an adjacent opponent with a corrosive blow."); break;
        case DRACONIAN_GREEN: var_set_string(res, "Attack an adjacent opponent with a poisoned blow."); break;
        case DRACONIAN_GOLD: var_set_string(res, "Attack an adjacent opponent with a stunning blow."); break;
        case DRACONIAN_BRONZE: var_set_string(res, "Attack an adjacent opponent with a confusing blow."); break;
        case DRACONIAN_CRYSTAL: var_set_string(res, "Attack an adjacent opponent with a shredding blow."); break;
        case DRACONIAN_SHADOW: var_set_string(res, "Attack an adjacent opponent with a vampiric blow."); break;
        }
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have the power of Dragon Strike.");
        break;
    case SPELL_HELP_DESC:
        if (spoiler_hack)
            var_set_string(res, "You will be able to attack an adjacent opponent with a special blow (Fire, Cold, etc).");
        else
        {
            switch (p_ptr->psubrace)
            {
            case DRACONIAN_RED: var_set_string(res, "You will be able to attack an adjacent opponent with a fiery blow."); break;
            case DRACONIAN_WHITE: var_set_string(res, "You will be able to attack an adjacent opponent with an icy blow."); break;
            case DRACONIAN_BLUE: var_set_string(res, "You will be able to attack an adjacent opponent with a shocking blow."); break;
            case DRACONIAN_BLACK: var_set_string(res, "You will be able to attack an adjacent opponent with a corrosive blow."); break;
            case DRACONIAN_GREEN: var_set_string(res, "You will be able to attack an adjacent opponent with a poisoned blow."); break;
            case DRACONIAN_GOLD: var_set_string(res, "You will be able to attack an adjacent opponent with a stunning blow."); break;
            case DRACONIAN_BRONZE: var_set_string(res, "You will be able to attack an adjacent opponent with a confusing blow."); break;
            case DRACONIAN_CRYSTAL: var_set_string(res, "You will be able to attack an adjacent opponent with a shredding blow."); break;
            case DRACONIAN_SHADOW: var_set_string(res, "You will be able to attack an adjacent opponent with a vampiric blow."); break;
            }
        }
        break;
    case SPELL_CAST:
    {
        int mode = 0;
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED: mode = DRACONIAN_STRIKE_FIRE; break;
        case DRACONIAN_WHITE: mode = DRACONIAN_STRIKE_COLD; break;
        case DRACONIAN_BLUE: mode = DRACONIAN_STRIKE_ELEC; break;
        case DRACONIAN_BLACK: mode = DRACONIAN_STRIKE_ACID; break;
        case DRACONIAN_GREEN: mode = DRACONIAN_STRIKE_POIS; break;
        case DRACONIAN_GOLD: mode = DRACONIAN_STRIKE_STUN; break;
        case DRACONIAN_BRONZE: mode = DRACONIAN_STRIKE_CONF; break;
        case DRACONIAN_CRYSTAL: mode = PY_ATTACK_VORPAL; break;
        case DRACONIAN_SHADOW: mode = PY_ATTACK_VAMP; break;
        }
        var_set_bool(res, do_blow(mode));
        break;
    }
    case SPELL_COST_EXTRA:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
        case DRACONIAN_WHITE:
        case DRACONIAN_BLUE:
        case DRACONIAN_BLACK:
        case DRACONIAN_GREEN:
            var_set_int(res, 15);
            break;
        case DRACONIAN_GOLD:
        case DRACONIAN_BRONZE:
            var_set_int(res, 20);
            break;
        case DRACONIAN_CRYSTAL:
            var_set_int(res, 12);
            break;
        case DRACONIAN_SHADOW:
            var_set_int(res, 7);
            break;
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void easy_tiring_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Easy Tiring");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You suddenly feel out of shape.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel out of shape.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Physical combat exhausts you.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void easy_tiring_II_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Very Easy Tiring");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You suddenly feel incredibly tired.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel incredibly tired.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Using magic or ranged weapons exhausts you.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void eat_light_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Eat Light");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel a strange kinship with Ungoliant.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel the world's a brighter place.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You sometimes feed off of the light around you.");
        break;
    case SPELL_PROCESS:
        if (one_in_(3000))
        {
            int slot = equip_find_obj(TV_LITE, SV_ANY);

            msg_print("A shadow passes over you.");
            msg_print(NULL);

            if ((cave[py][px].info & (CAVE_GLOW | CAVE_MNDK)) == CAVE_GLOW)
            {
                hp_player(10);
            }

            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if (!object_is_fixed_artifact(o_ptr) && (o_ptr->xtra4 > 0))
                {
                    hp_player(o_ptr->xtra4 / 20);
                    o_ptr->xtra4 /= 2;

                    msg_print("You absorb energy from your light!");
                    notice_lite_change(o_ptr);
                }
            }
            unlite_area(50, 10);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void einstein_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Living Computer");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your brain evolves into a living computer!");
        mut_lose(MUT_MORONIC);
        mut_lose(MUT_PULTITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your brain reverts to normal.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your brain is a living computer (+4 INT/WIS).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void elec_aura_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Electric Aura");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Electricity starts running through you!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Electricity stops running through you.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Electricity is running through your veins.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->sh_elec++;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void evasion_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Evasion");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the power of evasion.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the power of evasion.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can avoid being crushed by earthquakes and you dodge monster breath attacks.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Reduces damage from all enemy breath attacks, rockets and rocks by between 11% and 20%, and gives a 50% chance to avoid earthquakes.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void extra_eyes_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Extra Eyes");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You grow an extra pair of eyes!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your extra eyes vanish!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have an extra pair of eyes (+15 search).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.fos += 15;
        p_ptr->skills.srh += 15;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void extra_legs_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Extra Legs");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You grow an extra pair of legs!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your extra legs disappear!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have an extra pair of legs (+3 speed).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed += 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void extra_noise_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Extra Noise");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start making strange noise!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop making strange noise!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You make a lot of strange noise (-3 stealth).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.stl -= 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fantastic_frenzy_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fantastic Frenzy");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel a fantastic frenzy...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel a fantastic frenzy.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have the power of Fantastic Frenzy.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Saves the energy left over when a monster is killed in less than one turn, and also gives the power of Whirlwind Attack, which allows you to attack all adjacent monsters at a cost of 50 SP or HP.");
        break;
    default:
        massacre_spell(cmd, res);
        break;
    }
}

void fast_learner_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fast Learner");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You learn things quickly...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You revert to you normal dull self!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are a fast learner.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You gain 20% more experience for each monster slain.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fat_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Extra Fat");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become sickeningly fat!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You benefit from a miracle diet!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are extremely fat (+2 CON, -2 speed).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed -= 2;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fearless_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fearless");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become completely fearless.");
        mut_lose(MUT_COWARDICE);
        mut_lose(MUT_NO_INHIBITIONS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("You begin to feel fear again.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are fearless.");
        break;
    case SPELL_CALC_BONUS:
        res_add(RES_FEAR);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fell_sorcery_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fell Sorcery");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your spells are more powerful.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel your magic grow more powerful.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel your magic return to normal.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Your spells will grow more powerful (+15% spellpower), at the cost of a slight penalty to your strength, dexterity and constitution.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->spell_power += 2;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fire_aura_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire Aura");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your body is enveloped in flames!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your body is no longer enveloped in flames.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your body is enveloped in flames.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->sh_fire++;
        p_ptr->lite = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void flatulence_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Flatulence");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become subject to uncontrollable flatulence.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer subject to uncontrollable flatulence.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are subject to uncontrollable flatulence.");
        break;
    case SPELL_PROCESS:
        if (randint1(3000) == 13)
        {
            disturb(0, 0);
            /* Seriously, this the best mutation!  Ever!! :D */
            msg_print("BRRAAAP! Oops.");
            msg_print(NULL);
            fire_ball(GF_POIS, 0, p_ptr->lev, 3);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fleet_of_foot_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fleet of Foot");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel fleet of foot!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel like your old plodding self!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are fleet of foot (Movement costs less energy).");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Walking costs 40% less energy.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fumbling_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fumbling");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your feet grow to four times their former size.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your feet shrink to their former size.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You occasionally stumble and drop things.");
        break;
    case SPELL_PROCESS:
        if (one_in_(10000))
        {
            int slot = equip_random_slot(object_is_melee_weapon);

            disturb(0, 0);
            msg_print("You trip over your own feet!");
            take_hit(DAMAGE_NOESCAPE, randint1(150 / 6), "tripping");
            msg_print(NULL);

            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if (equip_can_takeoff(o_ptr))
                {
                    cmsg_print(TERM_VIOLET, "You drop your weapon!");
                    equip_drop(o_ptr);
                    msg_print("Press 'Y' to continue.");
                    flush();
                    for (;;)
                    {
                        char ch = inkey();
                        if (ch == 'Y') break;
                    }
                    msg_line_clear();
                }
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void good_luck_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "White Aura");
        break;
    case SPELL_GAIN_MUT:
        msg_print("There is a benevolent white aura surrounding you...");
        mut_lose(MUT_BAD_LUCK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your white aura shimmers and fades.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "There is a white aura surrounding you.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hallucination_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hallucination");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You are afflicted by a hallucinatory insanity!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer afflicted by a hallucinatory insanity!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have a hallucinatory insanity.");
        break;
    case SPELL_PROCESS:
        if (!res_save_default(RES_CHAOS) && randint1(6400) == 42 && !res_save_default(RES_CHAOS))
        {
            disturb(0, 0);
            p_ptr->redraw |= PR_EXTRA;
            set_image(p_ptr->image + randint0(50) + 20, FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void he_man_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "He-man");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You turn into a superhuman he-man!");
        mut_lose(MUT_PUNY);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your muscles revert to normal.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are superhumanly strong (+4 STR).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void horns_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Horns");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Horns pop forth into your forehead!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your horns vanish from your forehead!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have horns.");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 2;
        a.ds = 6;
        a.weight = 150;
        a.blows = 100;
        a.msg = "You impale.";
        a.name = "Horns";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_chr_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Overconfidence");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become overconfident!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer overconfident.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Overconfidence makes you sloppy.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Highly charismatic you may be; but getting too caught up in your own hype results in overconfidence and sloppy work. Lack of sufficient concentration gives you penalties to melee and shooting accuracy and to device and spell failure rates, similar to those from the Lazy personality.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.dev -= 10;
        p_ptr->skills.thn -= 16;
        p_ptr->skills.thb -= 10;
        p_ptr->to_m_chance += 10;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_con_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Pushed to the Limits");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your body is pushed to its limits.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your body is no longer pushed to its limits.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You suffer from frequent bouts of unwellness.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Your constitution and endurance are extraordinary; but as with many highly athletic people, your immune system has trouble keeping up. You suffer from frequent bouts of unwellness which temporarily decrease your Constitution and Dexterity.");
        break;
    case SPELL_PROCESS:
        if ((!p_ptr->unwell) && (one_in_(200)))
        {
            disturb(0, 0);
            set_unwell(50, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_dex_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nimbler than Your Body");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You are too agile for your body.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer too agile for your body.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You occasionally sprain a muscle in combat.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Agility is your strongest point; but there is only so much you can do in a human body. You sometimes sprain a muscle while hitting monsters, dodging enemy blows or firing projectiles; this temporarily reduces your speed by 10.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_int_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Overactive Imagination");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your imagination becomes overactive.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your imagination is no longer overactive.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have an overactive imagination.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Your brain is highly developed - for a human - but perhaps sometimes you can have too much of a good thing? Your overactive imagination makes you vulnerable to fear, and reduces your effective character level by 10 for all fear-related player/monster interactions.");
        break;
    case SPELL_CALC_BONUS:
        res_add_vuln(RES_FEAR);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_str_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Unbalancing Strikes");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You can no longer control your own strength.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You can control your own strength again.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "The power of your strongest hits throws you off-balance.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Strength is your greatest asset; but you now hit with too much power to control. Every critical hit you score in melee will unbalance you; you cannot score further criticals in that round, and since you need extra time to regain your balance, such a round will consume 1.2 turns instead of 1.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_wis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Attuned to God");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You no longer feel any connection to evil creatures.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer disconnected from evil minds.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You feel no connection to creatures of evil.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You are extremely wise, and have spent so much time in meditation that your mind is now fully attuned to the divine; but as a consequence, you feel no connection to evil monsters. You cannot detect any evil monster telepathically, even with spells or equipment of telepathy.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hypochondria_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Acute Hypochondria");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel very worried about your health.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer worry about your health.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are a hypochondriac.");
        break;
    case SPELL_PROCESS:
        if (one_in_(1815))
        {
            if (one_in_(2))
            {
                disturb(0, 0);
                msg_print("You feel incredibly worried!");
                fear_add_p(FEAR_SCARED);
            }
            else
            {
                disturb(0, 0);
                set_unwell(50, TRUE);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void illusion_normal_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Reassuring Image");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start projecting a reassuring image.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop projecting a reassuring image.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your appearance is masked with illusion.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void impotence_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Impotence");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your magic device suddenly feels limp.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your magic device no longer feels limp.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are impotent.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void infernal_deal_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Infernal Deal");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You make a pact with the devil!");
        mut_lose(MUT_ARTHRITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your infernal pact is broken.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have made an infernal deal.");
        break;
    case SPELL_HELP_DESC:
        if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_SAMURAI) || (p_ptr->pclass == CLASS_MYSTIC) || ((p_ptr->lev > 18) && (p_ptr->msp == 0))) var_set_string(res, "You will regain HP whenever a nearby enemy monster is slain, the amount equalling two-thirds of the monster's native level.");
        else var_set_string(res, "You will regain HP and SP whenever a nearby enemy monster is slain, the amount equalling four-ninths of the monster's native level for HP and half that for SP.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void infravision_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Infravision");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your infravision is improved.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your infravision is degraded.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have remarkable infravision (+3).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->see_infra += 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void inspired_smithing_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Inspired Smithing");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the ability to inspire smiths!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You can no longer inspire smiths.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your presence inspires smiths.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You will receive better results from item reforging (10% average value improvement).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void invulnerability_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Invulnerability");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You are blessed with fits of invulnerability.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer blessed with fits of invulnerability.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You occasionally feel invincible.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(5000))
        {
            disturb(0, 0);
            msg_print("You feel invincible!");

            msg_print(NULL);
            set_invuln(randint1(8) + 8, FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void limber_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Limber");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your muscles become limber.");
        mut_lose(MUT_ARTHRITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your muscles stiffen.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your body is very limber (+3 DEX).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void limp_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Limp");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start limping.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer walk with a limp.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You walk with a limp (-10% walking speed).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void loremaster_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Loremaster");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel quite knowledgeable.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You know longer know so much.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are a Loremaster.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Automatically identifies items as you pick them up.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->auto_id = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void magic_resistance_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Resistance");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become resistant to magic.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You become susceptible to magic again.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are resistant to magic.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.sav += (15 + (p_ptr->lev / 5));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void merchants_friend_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Merchant's Friend");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel an intense desire to shop!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel like shopping.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are a Merchant's Friend.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "You will gain new powers of shopping.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void moron_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Moron");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your brain withers away...");
        mut_lose(MUT_HYPER_INT);
        mut_lose(MUT_PULTITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your brain reverts to normal.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are moronic (-4 INT/WIS).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void motion_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Motion");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You move with new assurance.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You move with less assurance.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your movements are precise and forceful (FA; +1 STL).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->free_act++;
        p_ptr->skills.stl += 1;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void nausea_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nausea");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your stomach starts to roil nauseously.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your stomach stops roiling.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have a seriously upset stomach.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->slow_digest && one_in_(9000))
        {
            disturb(0, 0);

            msg_print("Your stomach roils, and you lose your lunch!");
            msg_print(NULL);

            set_food(PY_FOOD_WEAK);

            stop_mouth();
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void no_inhibitions_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "No Inhibitions");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You suddenly feel free to do anything.");
        mut_lose(MUT_COWARDICE);
        mut_lose(MUT_FEARLESS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel incredibly free.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have no inhibitions.");
        break;
    case SPELL_CALC_BONUS:
        res_add(RES_FEAR);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void normality_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Normality");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel strangely normal.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel normally strange.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You may be mutated, but you're recovering.");
        break;
    case SPELL_PROCESS:
        if (one_in_(5000))
        {
            if (mut_lose_random(NULL))
                msg_print("You feel oddly normal.");
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void one_with_magic_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "One with Magic");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel one with magic.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel one with magic.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have a chance of resisting Dispel Magic and Antimagic.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives a 77% chance to resist Antimagic and Dispel Magic (lower while polymorphed or in wraithform).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void peerless_sniper_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Peerless Sniper");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel distant monsters relax...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel distant monsters return to their normal grouchy selves.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your missiles no longer anger monsters.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Damaging a monster with a missile weapon no longer provokes a retaliation.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void peerless_tracker_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Peerless Tracker");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel able to track anything...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You can no longer track so well.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are a peerless tracker.");
        break;
    case SPELL_DESC:
        var_set_string(res, "Maps nearby area. Detects all monsters, traps, doors and stairs.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives the ability to map your surroundings and detect nearby monsters.");
        break;
    case SPELL_CAST:
    {
        int rad1 = DETECT_RAD_MAP;
        int rad2 = DETECT_RAD_DEFAULT;
        map_area(rad1);
        detect_traps(rad2, TRUE);
        detect_doors(rad2);
        detect_stairs(rad2);
        detect_monsters_normal(rad2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_wounds_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Polymorph Wounds");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel forces of chaos entering your old scars.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel forces of chaos departing your old scars.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your health is subject to chaotic forces.");
        break;
    case SPELL_PROCESS:
        if (one_in_(3000))
            do_poly_wounds();
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void potion_chugger_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Potion Chugger");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel like chugging a six pack of healing potions.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel like chugging your potions.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You chug potions faster than normal.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Allows you to drink potions twice as fast.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void produce_mana_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Produce Mana");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start producing magical energy uncontrollably.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop producing magical energy uncontrollably.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are producing magical energy uncontrollably.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(9000))
        {
            int dir = 0;
            disturb(0, 0);
            msg_print("Magical energy flows through you! You must release it!");
            flush();
            msg_print(NULL);
            (void)get_hack_dir(&dir);
            fire_ball(GF_MANA, dir, p_ptr->lev * 2, 3);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void puny_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Puny");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your muscles wither away...");
        mut_lose(MUT_HYPER_STR);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your muscles revert to normal.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are puny (-4 STR).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void purple_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Purple Patron");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You attract the attention of a Purple!");
        /* In case it isn't obvious, every character has a chaos deity assigned at birth. */
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the attention of the Purples.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are the disciple of a Purple.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void random_banish_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Random Banish");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel a terrifying power lurking behind you.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel a terrifying power lurking behind you.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You sometimes cause nearby creatures to vanish.");
        break;
    case SPELL_PROCESS:
        if (one_in_(9000))
        {
            disturb(0, 0);
            msg_print("You suddenly feel almost lonely.");

            banish_monsters(100);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void random_teleport_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Random Teleportation");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your position seems very uncertain...");
        mut_lose(MUT_TELEPORT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your position seems more certain.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are teleporting randomly.");
        break;
    case SPELL_PROCESS:
        if (!res_save_default(RES_NEXUS) && !p_ptr->anti_tele && (randint1(5000) == 88))
        {
            disturb(0, 0);
            msg_print("Your position suddenly seems very uncertain...");
            msg_print(NULL);
            teleport_player(40, TELEPORT_PASSIVE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void raw_chaos_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Raw Chaos");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel the universe is less stable around you.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel the universe is more stable around you.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You occasionally are surrounded with raw chaos.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(8000))
        {
            disturb(0, 0);
            msg_print("You feel the world warping around you!");
            msg_print(NULL);
            fire_ball(GF_CHAOS, 0, p_ptr->lev, 8);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void regeneration_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Regeneration");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start regenerating.");
        mut_lose(MUT_FLESH_ROT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop regenerating.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are regenerating.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->regen += 100;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resilient_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resilient");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become extraordinarily resilient.");
        mut_lose(MUT_ALBINO);
        break;
    case SPELL_LOSE_MUT:
        msg_print("You become ordinarily resilient again.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are very resilient (+4 CON).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void rotting_flesh_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rotting Flesh");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your flesh is afflicted by a rotting disease!");
        mut_lose(MUT_STEEL_SKIN);
        mut_lose(MUT_REGEN);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your flesh is no longer afflicted by a rotting disease!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your flesh is rotting (-2 CON, -1 CHR, Slow Regen).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->regen -= 80;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sacred_vitality_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sacred Vitality");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the power of Sacred Vitality!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the power of Sacred Vitality!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You gain a bonus to all healing effects.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives a 20% bonus to all healing effects.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void scales_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Scales");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your skin turns into black scales!");
        mut_lose(MUT_STEEL_SKIN);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your scales vanish!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your skin has turned into scales (-1 CHR, +10 AC).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void scorpion_tail_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Scorpion Tail");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You grow a scorpion tail!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose your scorpion tail!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have a scorpion tail.");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 3;
        a.ds = 7;
        a.weight = 50;
        a.blows = 100;
        a.effect[0] = GF_POIS;
        a.msg = "You lash.";
        a.name = "Tail";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void sensitive_eyes_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sensitive Eyes");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your eyes suddenly feel very sensitive.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your eyes no longer feel sensitive.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your eyes are very sensitive.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->see_infra += 4;
        res_add_vuln(RES_BLIND);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void shadow_walk_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shadow Walk");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel like reality is as thin as paper.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel like you're trapped in reality.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You occasionally stumble into other shadows.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(12000) && !p_ptr->inside_arena)
            alter_reality();
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void short_legs_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Short Legs");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your legs turn into short stubs!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your legs lengthen to normal.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your legs are short stubs (-3 speed).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed -= 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void silly_voice_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Silly Voice");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your voice turns into a ridiculous squeak!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your voice returns to normal.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your voice is a silly squeak (-4 CHR).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


void speed_flux_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Speed Flux");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become manic-depressive.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer manic-depressive.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You move faster or slower randomly.");
        break;
    case SPELL_PROCESS:
        if (one_in_(6000))
        {
            disturb(0, 0);
            if (one_in_(2))
            {
                msg_print("You feel less energetic.");
                if (p_ptr->fast > 0)
                    set_fast(0, TRUE);
                else
                {
                    if ((!p_ptr->slow) && (one_in_(2))) set_slow(randint1(30) + 10, FALSE);
                    else (void)p_inc_minislow(10);
                }
            }
            else
            {
                msg_print("You feel more energetic.");
                if ((p_ptr->slow > 0) || (p_ptr->minislow > 0))
                {
                    if (p_ptr->slow > 0) set_slow(0, TRUE);
                    if (p_ptr->minislow > 0) (void)p_inc_minislow(-10);
                }
                else
                    set_fast(randint1(30) + 10, FALSE);
            }
            msg_print(NULL);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void speed_reader_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Speed Reader");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel like reading a long novel.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer read so fast.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You read scrolls faster than normal.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Allows you to read scrolls twice as fast.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void steel_skin_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Steel Skin");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your skin turns to steel!");
        mut_lose(MUT_SCALES);
        mut_lose(MUT_WARTS);
        mut_lose(MUT_FLESH_ROT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your skin reverts to flesh!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your skin is made of steel (-1 DEX, +25 AC).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 25;
        p_ptr->dis_to_a += 25;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void strong_mind_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Strong Mind");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel in full control of your mind!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel in full control of your mind.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are immune to mana draining.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Makes you immune to mana draining.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void subtle_casting_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Subtle Casting");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel distant monsters relax...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel distant monsters return to their normal grouchy selves.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your distance attack spells no longer anger monsters.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Damaging a monster with distance magic no longer provokes a retaliation.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void telepathy_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Telepathy");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You develop a telepathic ability!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose your telepathic ability!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are telepathic.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->telepathy = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void tentacles_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Tentacles");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Evil-looking tentacles sprout from your sides.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your tentacles vanish from your sides.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have evil looking tentacles.");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 2;
        a.ds = 5;
        a.weight = 50;
        a.blows = 100;
        a.msg = "You hit with your tentacles.";
        a.name = "Tentacles";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void tread_softly_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Tread Softly");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your stealth is increased.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel your stealth increase.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel your stealth return to normal.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Increases your stealth by 3.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.stl += 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void trunk_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Elephantine Trunk");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your nose grows into an elephant-like trunk.");
        mut_lose(MUT_BEAK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your nose returns to a normal length.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have an elephantine trunk.");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 1;
        a.ds = 4;
        a.weight = 200;
        a.blows = 100;
        a.msg = "You hit with your trunk.";
        a.name = "Trunk";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void untouchable_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Untouchable");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel untouchable!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel touchable!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are untouchable and gain a bonus to AC.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives +20 to armor class.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 20;
        p_ptr->dis_to_a += 20;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void unyielding_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Unyielding");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You will never yield!!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Heck!  Might as well give up ...");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are unyielding and gain extra hp.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives a bonus to hit points (+1 HP per character level).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vortex_melee_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Deadly Whirl");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start whirling with deadly power!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer whirl with deadly power.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You whirl with deadly power.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives one extra damage side and +25 to-hit/to-dam.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vortex_speed_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Tornado Speed");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start moving at tornado speed!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer move at tornado speed.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You move at tornado speed (+8 speed).");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Gives +8 speed.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed += 8;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vortex_control_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breath Control");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You learn to control your breathing.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer control your breathing.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You control your breathing.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Increases the cost of breathing and decreases breath power for some elements, but allows you to select the element breathed.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vulnerability_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Vulnerability");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel strangely exposed.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel less exposed.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are susceptible to damage from the elements.");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void warning_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Warning");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You suddenly feel paranoid.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel paranoid.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You receive warnings about your foes.");
        break;
    case SPELL_PROCESS:
        if (one_in_(1000))
        {
            int danger_amount = 0;
            int monster;

            for (monster = 0; monster < m_max; monster++)
            {
                monster_type    *m_ptr = &m_list[monster];
                monster_race    *r_ptr = &r_info[m_ptr->r_idx];

                /* Skip dead monsters */
                if (!m_ptr->r_idx) continue;

                if (r_ptr->level >= p_ptr->lev)
                {
                    danger_amount += r_ptr->level - p_ptr->lev + 1;
                }
            }

            if (danger_amount > 100)
                msg_print("You feel utterly terrified!");

            else if (danger_amount > 50)
                msg_print("You feel terrified!");

            else if (danger_amount > 20)
                msg_print("You feel very worried!");

            else if (danger_amount > 10)
                msg_print("You feel paranoid!");

            else if (danger_amount > 5)
                msg_print("You feel almost safe.");

            else
                msg_print("You feel lonely.");
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void warts_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Warts");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Disgusting warts appear everywhere on you!");
        mut_lose(MUT_STEEL_SKIN);
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your warts disappear!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your skin is covered with warts (-2 CHR, +5 AC).");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 5;
        p_ptr->dis_to_a += 5;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void wasting_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Horrible Wasting");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You suddenly contract a horrible wasting disease.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are cured of the horrible wasting disease!");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have a horrible wasting disease.");
        break;
    case SPELL_PROCESS:
        if (one_in_(3000))
        {
            int which_stat = randint0(6);
            int sustained = FALSE;

            switch (which_stat)
            {
            case A_STR:
                if (p_ptr->sustain_str) sustained = TRUE;
                break;
            case A_INT:
                if (p_ptr->sustain_int) sustained = TRUE;
                break;
            case A_WIS:
                if (p_ptr->sustain_wis) sustained = TRUE;
                break;
            case A_DEX:
                if (p_ptr->sustain_dex) sustained = TRUE;
                break;
            case A_CON:
                if (p_ptr->sustain_con) sustained = TRUE;
                break;
            case A_CHR:
                if (p_ptr->sustain_chr) sustained = TRUE;
                break;
            default:
                msg_print("Invalid stat chosen!");
                sustained = TRUE;
                break;
            }

            if (!sustained)
            {
                disturb(0, 0);
                msg_print("You can feel yourself wasting away!");
                msg_print(NULL);
                dec_stat(which_stat, randint1(6) + 6, one_in_(6));
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void waybread_into_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Waybread Intolerance");
        break;
    case SPELL_GAIN_MUT:
        msg_print("The idea of lembas suddenly disgusts you!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are cured of your waybread intolerance.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have a waybread intolerance.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->levitation = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void weapon_skills_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Weapon Versatility");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel you may master anything...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel so masterful.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You may master any weapon.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Allows you to reach full proficiency with any weapon (but does not make icky weapons non-icky).");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void random_telepathy_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Random Telepathy");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your thoughts suddenly take off in strange directions.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your thoughts return to boring paths.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your mind randomly expands and contracts.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(3000))
        {
            if (p_ptr->tim_esp > 0)
            {
                msg_print("Your mind feels cloudy!");
                set_tim_esp(0, TRUE);
            }
            else
            {
                msg_print("Your mind expands!");
                set_tim_esp(p_ptr->lev, FALSE);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void weird_mind_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Weird Mind");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You are no longer bothered by the Eldritch Horror!");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel susceptible to the Eldritch Horror again.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your weird mind is unaffected by the Eldritch Horror and Hallucination.");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "Protects your mind from hallucination and Eldritch Horrors.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->no_eldritch = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void wings_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wings");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You grow a pair of wings.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your wings fall off.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You have wings.");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->levitation = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void wraith_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wraithform");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start to fade in and out of the physical world.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are firmly in the physical world.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You fade in and out of physical reality.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(3000))
        {
            disturb(0, 0);
            msg_print("You feel insubstantial!");
            msg_print(NULL);
            set_wraith_form(randint1(p_ptr->lev / 2) + (p_ptr->lev / 2), FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

