#include "angband.h"

void magic_mapping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Mapping");
        break;
    case SPELL_DESC:
        var_set_string(res, "Maps the dungeon in your vicinity.");
        break;
    case SPELL_CAST:
        map_area(DETECT_RAD_MAP);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_magic_mapping(void) { return cast_spell(magic_mapping_spell); }

void magic_missile_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Missile");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a weak bolt of unresistable magic.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(3 + ((p_ptr->lev - 1) / 5)), 4, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dice = 3 + (p_ptr->lev - 1) / 5;
        int sides = 4;
        int dir = 0;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance() - 10,
            GF_MISSILE,
            dir,
            spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_magic_missile(void) { return cast_spell(magic_missile_spell); }

void mana_branding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Branding");
        break;
    case SPELL_DESC:
        var_set_string(res, "Makes current weapon some elemental branded. You must wield weapons.");
        break;
    case SPELL_CAST:
        var_set_bool(res, choose_ele_attack());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_bolt_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of pure mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(p_ptr->lev * 7 / 2), spell_power(p_ptr->lev + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You cast a mana bolt.");
        fire_bolt(
            GF_MANA,
            dir,
            spell_power(randint1(p_ptr->lev * 7 / 2) + p_ptr->lev + p_ptr->to_d_spell)
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_bolt_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a powerful bolt of pure mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(p_ptr->lev * 7), spell_power(p_ptr->lev*2 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You cast a mana bolt.");
        fire_bolt(
            GF_MANA,
            dir,
            spell_power(randint1(p_ptr->lev * 7) + p_ptr->lev*2 + p_ptr->to_d_spell)
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_storm_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a large ball of pure mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 5 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You cast a mana storm.");
        fire_ball(GF_MANA, dir, spell_power(p_ptr->lev * 5 + damroll(10, 10) + p_ptr->to_d_spell), 4);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_storm_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a large ball of pure mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 8 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You cast a mana storm.");
        fire_ball(GF_MANA, dir, spell_power(p_ptr->lev * 8 + 50 + damroll(10, 10) + p_ptr->to_d_spell), 4);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void massacre_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Massacre");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack all adjacent monsters in a fit of wild, uncontrollable fury.");
        break;
    case SPELL_CAST:
    {
        int              dir, x, y;
        cave_type       *c_ptr;
        monster_type    *m_ptr;

        for (dir = 0; dir < 8; dir++)
        {
            y = py + ddy_ddd[dir];
            x = px + ddx_ddd[dir];
            c_ptr = &cave[y][x];

            m_ptr = &m_list[c_ptr->m_idx];

            if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
                py_attack(y, x, 0);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mind_blast_spell(int cmd, variant *res)
{
    int dice = 3 + (p_ptr->lev - 1)/5;
    int sides = 3;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mind Blast");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to blast your opponent with psionic energy.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the power of Mind Blast.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the power of Mind Blast.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can Mind Blast your enemies.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            msg_print("You concentrate...");
            fire_bolt(
                GF_PSI,
                dir,
                spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
            );
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_mind_blast(void) { return cast_spell(mind_blast_spell); }

void nature_awareness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nature Awareness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Maps nearby area. Detects all monsters, traps, doors and stairs.");
        break;
    case SPELL_CAST:
        map_area(DETECT_RAD_MAP);
        detect_traps(DETECT_RAD_DEFAULT, TRUE);
        detect_doors(DETECT_RAD_DEFAULT);
        detect_stairs(DETECT_RAD_DEFAULT);
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void nether_ball_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 / 2 + 100 + p_ptr->to_d_spell);
    int rad = spell_power(p_ptr->lev / 20 + 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nether Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of nether.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_NETHER, dir, dam, rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void nether_bolt_spell(int cmd, variant *res)
{
    int dd = 8 + (p_ptr->lev - 5) / 4;
    int ds = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nether Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt or beam of nether.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_NETHER,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void orb_of_entropy_spell(int cmd, variant *res)
{
    int base;

    if (p_ptr->pclass == CLASS_MAGE || p_ptr->pclass == CLASS_BLOOD_MAGE || p_ptr->pclass == CLASS_HIGH_MAGE || p_ptr->pclass == CLASS_SORCERER || p_ptr->pclass == CLASS_YELLOW_MAGE || p_ptr->pclass == CLASS_GRAY_MAGE)
        base = p_ptr->lev + p_ptr->lev / 2;
    else
        base = p_ptr->lev + p_ptr->lev / 4;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Orb of Entropy");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball which damages living monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3, spell_power(6), spell_power(base + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        int rad = (p_ptr->lev < 30) ? 2 : 3;

        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_OLD_DRAIN, dir, spell_power(damroll(3, 6) + base + p_ptr->to_d_spell), rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void panic_hit_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Panic Hit");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent monster and attempt a getaway.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You suddenly understand how thieves feel.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel jumpy.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can run for your life after hitting something.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int x, y;

        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) break;
        y = py + ddy[dir];
        x = px + ddx[dir];
        if (cave[y][x].m_idx)
        {
            py_attack(y, x, 0);
            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("You failed to teleport.");
            else
                teleport_player(30, 0L);

            var_set_bool(res, TRUE);
        }
        else
        {
            msg_print("You don't see any monster in this direction");
            msg_print(NULL);
            /* No Charge for this Action ... */
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_panic_hit(void) { return cast_spell(panic_hit_spell); }

void paralyze_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Paralyze");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to freeze a monster.");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        stasis_monster(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void pattern_mindwalk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Pattern Mindwalking");
        break;
    case SPELL_DESC:
        var_set_string(res, "Walk the pattern in your mind. Restores life and stats.");
        break;
    case SPELL_CAST:
        msg_print("You picture the Pattern in your mind and walk it...");

        set_poisoned(0, TRUE);
        set_image(0, TRUE);
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        set_blind(0, TRUE);
        fear_clear_p();
        do_res_stat(A_STR);
        do_res_stat(A_INT);
        do_res_stat(A_WIS);
        do_res_stat(A_DEX);
        do_res_stat(A_CON);
        do_res_stat(A_CHR);
        restore_level();
        lp_player(1000);

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void perception_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Perception");
        break;
    default:
        identify_spell(cmd, res);
        break;
    }
}

void phase_door_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Phase Door");
        break;
    case SPELL_DESC:
        var_set_string(res, "A short range teleport.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the power of minor teleportation.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the power of minor teleportation.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can teleport yourself short distances.");
        break;
    case SPELL_CAST:
        teleport_player(10, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, 30);
            break;
        } /* Fall through */
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_phase_door(void) { return cast_spell(phase_door_spell); }

void plasma_ball_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 / 2 + 80 + p_ptr->to_d_spell);
    int rad = spell_power(2 + p_ptr->lev / 40);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Plasma Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of plasma.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_PLASMA, dir, dam, rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void plasma_bolt_spell(int cmd, variant *res)
{
    int dd = 11 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Plasma Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt or beam of plasma.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_PLASMA,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void poison_dart_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Poison Dart");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a poison dart at a single foe.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You throw a dart of poison.");
        fire_bolt(GF_POIS, dir, p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polish_shield_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Polish Shield");
        break;
    case SPELL_DESC:
        var_set_string(res, "Makes your shield reflect missiles and bolt spells.");
        break;
    case SPELL_CAST:
        var_set_bool(res, polish_shield());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_polish_shield(void) {    return cast_spell(polish_shield_spell); }

void polymorph_colossus_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Polymorph Colossus");
        break;
    case SPELL_DESC:
        var_set_string(res, "Mimic a Colossus for a while. Loses abilities of original race and gets great abilities as a colossus.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(15), spell_power(15)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(15);
        set_mimic(base + randint1(base), MIMIC_COLOSSUS, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Polymorph Demon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Mimic a demon for a while. Loses abilities of original race and gets abilities as a demon.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(15), spell_power(15)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(10 + p_ptr->lev / 2);
        set_mimic(base + randint1(base), MIMIC_DEMON, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_demonlord_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Polymorph Demonlord");
        break;
    case SPELL_DESC:
        var_set_string(res, "Mimic a demon lord for a while. Loses abilities of original race and gets great abilities as a demon lord. Even hard walls can't stop your walking.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(15), spell_power(15)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(15);
        set_mimic(base + randint1(base), MIMIC_DEMON_LORD, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_self_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Polymorph");
        break;
    case SPELL_DESC:
        var_set_string(res, "Mutates yourself. This can be dangerous!");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your body seems mutable.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your body seems stable.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can polymorph yourself at will.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (get_check("You will polymorph yourself. Are you sure? "))
        {
            do_poly_self();
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_polymorph_self(void) { return cast_spell(polymorph_self_spell); }

void polymorph_vampire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Polymorph Vampire");
        break;
    case SPELL_DESC:
        var_set_string(res, "Mimic a powerful vampire for a while. Loses abilities of original race and gets abilities as a vampire.");
        break;
    case SPELL_INFO:
    {
        int base = spell_power(10 + p_ptr->lev / 2);
        var_set_string(res, info_duration(base, base));
        break;
    }
    case SPELL_CAST:
    {
        int base = spell_power(10 + p_ptr->lev / 2);
        set_mimic(base + randint1(base), MIMIC_VAMPIRE, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void power_throw_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Throw Object");
        break;
    case SPELL_DESC:
        var_set_string(res, "Hurl an object with great force.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your throwing arm feels much stronger.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your throwing arm feels much weaker.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can hurl objects with great force.");
        break;
    case SPELL_CAST: /* This is a hack for wild-talent use only */
    {
        bool old_mt = p_ptr->mighty_throw;
        if (p_ptr->pclass != CLASS_WILD_TALENT) break;
        p_ptr->mighty_throw = TRUE;
        if (!p_ptr->wild_mode)
        {
            py_throw_t context = {0};
            py_throw(&context);
        }
        p_ptr->mighty_throw = old_mt;
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_CALC_BONUS:
        p_ptr->mighty_throw = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_power_throw(void) { return cast_spell(power_throw_spell); }

void probing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Probe Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Determines the abilities, strengths and weaknesses of nearby monsters.");
        break;
    case SPELL_CAST:
        probing();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_probing(void) { return cast_spell(probing_spell); }

void protection_from_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Protection from Evil");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to prevent evil monsters from attacking you. When a weak evil monster melees you, it may be repelled by the forces of good.");
        break;
    case SPELL_CAST:
        set_protevil(randint1(3 * p_ptr->lev) + 25, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_protection_from_evil(void) { return cast_spell(protection_from_evil_spell); }

void punishment_spell(int cmd, variant *res)
{
    int dd = 3 + (p_ptr->lev - 1)/5;
    int ds = 4;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Punishment");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt or beam of lightning.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance() - 10,
            GF_ELEC,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void radiation_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Radiation Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of radiation.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(10), 6, spell_power(p_ptr->lev*2)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_NUKE, dir, spell_power(damroll(10, 6) + p_ptr->lev*2), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void radiation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Emit Radiation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a huge ball of radiation centered on you.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("You start emitting hard radiation.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You stop emitting hard radiation.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can emit hard radiation at will.");
        break;
    case SPELL_CAST:
        msg_print("Radiation flows from your body!");
        fire_ball(GF_NUKE, 0, spell_power(p_ptr->lev * 2), 3 + (p_ptr->lev / 20));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_radiation(void) { return cast_spell(radiation_spell); }

void ray_of_sunlight_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ray of Sunlight");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a beam of light which damages to light-sensitive monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(6, 8, 0));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        lite_line(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void recall_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Recall");
        break;
    case SPELL_DESC:
        var_set_string(res, "Travel back and forth between the town and the dungeon.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel briefly homesick, but it passes.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel briefly homesick.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can travel between town and the depths.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (word_of_recall(TRUE))
            var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_recall(void) { return cast_spell(recall_spell); }

void recharging_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Recharging");
        break;
    case SPELL_DESC:
        if (p_ptr->prace == RACE_MON_LEPRECHAUN)
            var_set_string(res, "It attempts to recharge a device using your gold for power.");
        else if (!p_ptr->msp)
            var_set_string(res, "It attempts to recharge a device using another device for power.");
        else
            var_set_string(res, "It attempts to recharge a device using your mana for power.");
        break;
    case SPELL_CAST:
        if (p_ptr->prace == RACE_MON_LEPRECHAUN)
            var_set_bool(res, recharge_from_player(2 * p_ptr->lev));
        else if (!p_ptr->msp)
            var_set_bool(res, recharge_from_device(3 * p_ptr->lev));
        else
            var_set_bool(res, recharge_from_player(3 * p_ptr->lev));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_recharging(void) { return cast_spell(recharging_spell); }

void remove_curse_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Remove Curse");
        break;
    case SPELL_DESC:
        var_set_string(res, "Uncurses an item so that you may remove it.");
        break;
    case SPELL_CAST:
        if (remove_curse())
            msg_print("You feel as if someone is watching over you.");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_remove_curse_I(void) { return cast_spell(remove_curse_I_spell); }

void remove_curse_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "*Remove Curse*");
        break;
    case SPELL_DESC:
        var_set_string(res, "Uncurses an item so that you may remove it. Even heavily cursed items can be removed.");
        break;
    case SPELL_CAST:
        if (remove_all_curse())
            msg_print("You feel as if someone is watching over you.");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_remove_curse_II(void) { return cast_spell(remove_curse_II_spell); }

void remove_fear_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Remove Fear");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        fear_clear_p();
        var_set_bool(res, TRUE);
        break;
    case SPELL_FLAGS:
        var_set_int(res, PWR_AFRAID);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_remove_fear(void) { return cast_spell(remove_fear_spell); }

void resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resistance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives resistance to fire, cold, electricity, acid and poison for a while.");
        break;
    case SPELL_CAST:
    {
        int base = spell_power(20);

        set_oppose_base(randint1(base) + base, FALSE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_elements_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resist Elements");
        break;
    case SPELL_DESC:
        var_set_string(res, "Protect yourself from the ravages of the elements.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel like you can protect yourself.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel like you might be vulnerable.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can harden yourself to the ravages of the elements.");
        break;
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        if (p_ptr->lev >= 20)
            n += 5;
        if (p_ptr->lev >= 30)
            n += 5;
        if (p_ptr->lev >= 40)
            n += 5;
        if (p_ptr->lev >= 50)
            n += 5;
        var_set_int(res, n);
        break;
    }
    case SPELL_CAST:
    {
        int num = p_ptr->lev / 10;
        int dur = randint1(20) + 20;

        if (randint0(5) < num)
        {
            set_oppose_acid(dur, FALSE);
            num--;
        }
        if (randint0(4) < num)
        {
            set_oppose_elec(dur, FALSE);
            num--;
        }
        if (randint0(3) < num)
        {
            set_oppose_fire(dur, FALSE);
            num--;
        }
        if (randint0(2) < num)
        {
            set_oppose_cold(dur, FALSE);
            num--;
        }
        if (num)
        {
            set_oppose_pois(dur, FALSE);
            num--;
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_resist_elements(void) { return cast_spell(resist_elements_spell); }

void resist_environment_spell(int cmd, variant *res)
{
    int base = spell_power(20);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resist Environment");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives resistance to fire, cold and electricity for a while.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_oppose_cold(randint1(base) + base, FALSE);
        set_oppose_fire(randint1(base) + base, FALSE);
        set_oppose_elec(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_fire_spell(int cmd, variant *res)
{
    int base = spell_power(20);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resist Fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives extra resistance to fire for a bit.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_oppose_fire(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_heat_cold_spell(int cmd, variant *res)
{
    int base = spell_power(20);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resist Heat and Cold");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives resistance to fire and cold.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_oppose_cold(randint1(base) + base, FALSE);
        set_oppose_fire(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_poison_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Resist Poison");
        break;
    case SPELL_DESC:
        var_set_string(res, "Provides temporary resistance to poison.");
        break;
    case SPELL_CAST:
        set_oppose_pois(randint1(20) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void restoration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Restoration");
        break;
    case SPELL_DESC:
        var_set_string(res, "Restores all stats and experience.");
        break;
    case SPELL_CAST:
        do_res_stat(A_STR);
        do_res_stat(A_INT);
        do_res_stat(A_WIS);
        do_res_stat(A_DEX);
        do_res_stat(A_CON);
        do_res_stat(A_CHR);
        restore_level();
        lp_player(1000);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void restore_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Restore Life");
        break;
    case SPELL_DESC:
        var_set_string(res, "Regain all lost experience.");
        break;
    case SPELL_CAST:
        restore_level();
        lp_player(150);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_restore_life(void) { return cast_spell(restore_life_spell); }

void rocket_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Rocket I");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a magic rocket.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(120 + p_ptr->lev * 2 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(120 + p_ptr->lev * 2 + p_ptr->to_d_spell);
        int rad = 2;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You launch a rocket!");
        fire_rocket(GF_ROCKET, dir, dam, rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void rocket_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Rocket II");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a magic rocket of unsurpassable fire power.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(500 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(500 + p_ptr->to_d_spell);
        int rad = 2;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You launch a rocket!");
        fire_rocket(GF_ROCKET, dir, dam, rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void rush_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rush Attack");
        break;
    case SPELL_DESC:
        var_set_string(res, "Charge a nearby monster and attack with your weapons.");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(5, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

