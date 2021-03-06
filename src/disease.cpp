#include "rng.h"
#include "game.h"
#include "monster.h"
#include "bodypart.h"
#include "disease.h"
#include "weather.h"
#include "translations.h"
#include "martialarts.h"
#include "monstergenerator.h"
#include "messages.h"
#include <stdlib.h>
#include <sstream>
#include <algorithm>

// Used only internally for fast lookups.
enum dis_type_enum {
 DI_NULL,
// Weather
// Temperature, the order is important (dependent on bodypart.h)
 DI_COLD,
 DI_FROSTBITE,
 DI_HOT,
 DI_BLISTERS,
// Diseases
 DI_INFECTION,
 DI_COMMON_COLD, DI_FLU, DI_RECOVER, DI_TAPEWORM, DI_BLOODWORMS, DI_BRAINWORM, DI_PAINCYSTS,
 DI_TETANUS,
// Fields - onfire moved to effects
 DI_CRUSHED, DI_BOULDERING,
// Monsters
 DI_SAP, DI_SPORES, DI_FUNGUS, DI_SLIMED,
 DI_LYING_DOWN, DI_SLEEP, DI_ALARM_CLOCK,
 DI_PARALYZEPOISON, DI_BLEED, DI_BADPOISON, DI_FOODPOISON, DI_SHAKES,
 DI_DERMATIK, DI_FORMICATION,
 DI_WEBBED,
 DI_RAT, DI_BITE,
// Food & Drugs
 DI_PKILL1, DI_PKILL2, DI_PKILL3, DI_PKILL_L, DI_DRUNK, DI_CIG, DI_HIGH, DI_WEED_HIGH,
  DI_HALLU, DI_VISUALS, DI_IODINE, DI_TOOK_XANAX, DI_TOOK_PROZAC,
  DI_TOOK_FLUMED, DI_ADRENALINE, DI_JETINJECTOR, DI_ASTHMA, DI_GRACK, DI_METH, DI_VALIUM,
// Traps
 DI_BEARTRAP, DI_LIGHTSNARE, DI_HEAVYSNARE, DI_IN_PIT, DI_STUNNED, DI_DOWNED,
// Other
 DI_AMIGARA, DI_STEMCELL_TREATMENT, DI_TELEGLOW, DI_ATTENTION, DI_EVIL,
// Bite wound infected (dependent on bodypart.h)
 DI_INFECTED,
// Inflicted by an NPC
 DI_ASKED_TO_FOLLOW, DI_ASKED_TO_LEAD, DI_ASKED_FOR_ITEM,
 DI_ASKED_TO_TRAIN, DI_ASKED_PERSONAL_INFO,
// Martial arts-related buffs
 DI_MA_BUFF,
// NPC-only
 DI_CATCH_UP,
 // Lack/sleep
 DI_LACKSLEEP,
 // Grabbed (from MA or monster)
 DI_GRABBED
};

std::map<std::string, dis_type_enum> disease_type_lookup;

// Todo: Move helper functions into a DiseaseHandler Class.
// Should standardize parameters so we can make function pointers.
static void manage_fungal_infection(player& p, disease& dis);
static void manage_sleep(player& p, disease& dis);

static void handle_alcohol(player& p, disease& dis);
static void handle_bite_wound(player& p, disease& dis);
static void handle_infected_wound(player& p, disease& dis);
static void handle_recovery(player& p, disease& dis);
static void handle_cough(player& p, int intensity = 1, int volume = 4, bool harmful = false);
static void handle_deliriant(player& p, disease& dis);
static void handle_evil(player& p, disease& dis);
static void handle_insect_parasites(player& p, disease& dis);

static bool will_vomit(player& p, int chance = 1000);

void game::init_diseases() {
    // Initialize the disease lookup table.

    disease_type_lookup["null"] = DI_NULL;
    disease_type_lookup["cold"] = DI_COLD;
    disease_type_lookup["frostbite"] = DI_FROSTBITE;
    disease_type_lookup["hot"] = DI_HOT;
    disease_type_lookup["blisters"] = DI_BLISTERS;
    disease_type_lookup["infection"] = DI_INFECTION;
    disease_type_lookup["common_cold"] = DI_COMMON_COLD;
    disease_type_lookup["flu"] = DI_FLU;
    disease_type_lookup["recover"] = DI_RECOVER;
    disease_type_lookup["tapeworm"] = DI_TAPEWORM;
    disease_type_lookup["bloodworms"] = DI_BLOODWORMS;
    disease_type_lookup["brainworm"] = DI_BRAINWORM;
    disease_type_lookup["tetanus"] = DI_TETANUS;
    disease_type_lookup["paincysts"] = DI_PAINCYSTS;
    disease_type_lookup["crushed"] = DI_CRUSHED;
    disease_type_lookup["bouldering"] = DI_BOULDERING;
    disease_type_lookup["sap"] = DI_SAP;
    disease_type_lookup["spores"] = DI_SPORES;
    disease_type_lookup["fungus"] = DI_FUNGUS;
    disease_type_lookup["slimed"] = DI_SLIMED;
    disease_type_lookup["lying_down"] = DI_LYING_DOWN;
    disease_type_lookup["sleep"] = DI_SLEEP;
    disease_type_lookup["alarm_clock"] = DI_ALARM_CLOCK;
    disease_type_lookup["bleed"] = DI_BLEED;
    disease_type_lookup["badpoison"] = DI_BADPOISON;
    disease_type_lookup["paralyzepoison"] = DI_PARALYZEPOISON;
    disease_type_lookup["foodpoison"] = DI_FOODPOISON;
    disease_type_lookup["shakes"] = DI_SHAKES;
    disease_type_lookup["dermatik"] = DI_DERMATIK;
    disease_type_lookup["formication"] = DI_FORMICATION;
    disease_type_lookup["webbed"] = DI_WEBBED;
    disease_type_lookup["rat"] = DI_RAT;
    disease_type_lookup["bite"] = DI_BITE;
    disease_type_lookup["pkill1"] = DI_PKILL1;
    disease_type_lookup["pkill2"] = DI_PKILL2;
    disease_type_lookup["pkill3"] = DI_PKILL3;
    disease_type_lookup["pkill_l"] = DI_PKILL_L;
    disease_type_lookup["drunk"] = DI_DRUNK;
    disease_type_lookup["valium"] = DI_VALIUM;
    disease_type_lookup["cig"] = DI_CIG;
    disease_type_lookup["high"] = DI_HIGH;
    disease_type_lookup["hallu"] = DI_HALLU;
    disease_type_lookup["visuals"] = DI_VISUALS;
    disease_type_lookup["iodine"] = DI_IODINE;
    disease_type_lookup["took_xanax"] = DI_TOOK_XANAX;
    disease_type_lookup["took_prozac"] = DI_TOOK_PROZAC;
    disease_type_lookup["took_flumed"] = DI_TOOK_FLUMED;
    disease_type_lookup["adrenaline"] = DI_ADRENALINE;
    disease_type_lookup["jetinjector"] = DI_JETINJECTOR;
    disease_type_lookup["asthma"] = DI_ASTHMA;
    disease_type_lookup["grack"] = DI_GRACK;
    disease_type_lookup["meth"] = DI_METH;
    disease_type_lookup["beartrap"] = DI_BEARTRAP;
    disease_type_lookup["lightsnare"] = DI_LIGHTSNARE;
    disease_type_lookup["heavysnare"] = DI_HEAVYSNARE;
    disease_type_lookup["in_pit"] = DI_IN_PIT;
    disease_type_lookup["stunned"] = DI_STUNNED;
    disease_type_lookup["downed"] = DI_DOWNED;
    disease_type_lookup["amigara"] = DI_AMIGARA;
    disease_type_lookup["stemcell_treatment"] = DI_STEMCELL_TREATMENT;
    disease_type_lookup["teleglow"] = DI_TELEGLOW;
    disease_type_lookup["attention"] = DI_ATTENTION;
    disease_type_lookup["evil"] = DI_EVIL;
    disease_type_lookup["infected"] = DI_INFECTED;
    disease_type_lookup["asked_to_follow"] = DI_ASKED_TO_FOLLOW;
    disease_type_lookup["asked_to_lead"] = DI_ASKED_TO_LEAD;
    disease_type_lookup["asked_for_item"] = DI_ASKED_FOR_ITEM;
    disease_type_lookup["asked_to_train"] = DI_ASKED_TO_TRAIN;
    disease_type_lookup["asked_personal_info"] = DI_ASKED_PERSONAL_INFO;
    disease_type_lookup["catch_up"] = DI_CATCH_UP;
    disease_type_lookup["weed_high"] = DI_WEED_HIGH;
    disease_type_lookup["ma_buff"] = DI_MA_BUFF;
    disease_type_lookup["lack_sleep"] = DI_LACKSLEEP;
    disease_type_lookup["grabbed"] = DI_GRABBED;
}

bool dis_msg(dis_type type_string) {
    dis_type_enum type = disease_type_lookup[type_string];
    switch (type) {
    case DI_COMMON_COLD:
        add_msg(m_bad, _("You feel a cold coming on..."));
        g->u.add_memorial_log(pgettext("memorial_male", "Caught a cold."),
                              pgettext("memorial_female", "Caught a cold."));
        break;
    case DI_FLU:
        add_msg(m_bad, _("You feel a flu coming on..."));
        g->u.add_memorial_log(pgettext("memorial_male", "Caught the flu."),
                              pgettext("memorial_female", "Caught the flu."));
        break;
    case DI_CRUSHED:
        add_msg(m_bad, _("The ceiling collapses on you!"));
        break;
    case DI_BOULDERING:
        add_msg(m_warning, _("You are slowed by the rubble."));
        break;
    case DI_SAP:
        add_msg(m_bad, _("You're coated in sap!"));
        break;
    case DI_SLIMED:
        add_msg(m_bad, _("You're covered in thick goo!"));
        break;
    case DI_LYING_DOWN:
        add_msg(_("You lie down to go to sleep..."));
        break;
    case DI_FORMICATION:
        add_msg(m_bad, _("Your skin feels extremely itchy!"));
        break;
    case DI_WEBBED:
        add_msg(m_bad, _("You're covered in webs!"));
        break;
    case DI_DRUNK:
    case DI_HIGH:
    case DI_WEED_HIGH:
        add_msg(m_warning, _("You feel lightheaded."));
        break;
    case DI_ADRENALINE:
        add_msg(m_good, _("You feel a surge of adrenaline!"));
        break;
    case DI_JETINJECTOR:
        add_msg(_("You feel a rush as the chemicals flow through your body!"));
        break;
    case DI_ASTHMA:
        add_msg(m_bad, _("You can't breathe... asthma attack!"));
        break;
    case DI_STUNNED:
        add_msg(m_bad, _("You're stunned!"));
        break;
    case DI_DOWNED:
        add_msg(m_bad, _("You're knocked to the floor!"));
        break;
    case DI_AMIGARA:
        add_msg(m_bad, _("You can't look away from the faultline..."));
        break;
    case DI_STEMCELL_TREATMENT:
        add_msg(m_good, _("You receive a pureed bone & enamel injection into your eyeball."));
        if (!(g->u.has_trait("NOPAIN"))) {
            add_msg(m_bad, _("It is excruciating."));
        }
        break;
    case DI_BITE:
        add_msg(m_bad, _("The bite wound feels really deep..."));
        g->u.add_memorial_log(pgettext("memorial_male", "Received a deep bite wound."),
                              pgettext("memorial_female", "Received a deep bite wound."));
        break;
    case DI_INFECTED:
        add_msg(m_bad, _("Your bite wound feels infected."));
        g->u.add_memorial_log(pgettext("memorial_male", "Contracted an infection."),
                              pgettext("memorial_female", "Contracted an infection."));
        break;
    case DI_LIGHTSNARE:
        add_msg(m_bad, _("You are snared."));
        break;
    case DI_HEAVYSNARE:
        add_msg(m_bad, _("You are snared."));
        break;
    case DI_LACKSLEEP:
        add_msg(m_warning, _("You are too tired to function well."));
        break;
    case DI_GRABBED:
        add_msg(m_bad, _("You have been grabbed."));
        break;
    default:
        return false;
        break;
    }

    return true;
}

void weed_msg(player *p) {
    int howhigh = p->disease_duration("weed_high");
    int smarts = p->get_int();
    if(howhigh > 125 && one_in(7)) {
        int msg = rng(0,5);
        switch(msg) {
        case 0: // Freakazoid
            p->add_msg_if_player(_("The scariest thing in the world would be... if all the air in the world turned to WOOD!"));
            return;
        case 1: // Simpsons
            p->add_msg_if_player(_("Could Jesus microwave a burrito so hot, that he himself couldn't eat it?"));
            p->hunger += 2;
            return;
        case 2:
            if(smarts > 8) { // Timothy Leary
                p->add_msg_if_player(_("Science is all metaphor."));
            } else if(smarts < 3){ // It's Always Sunny in Phildelphia
                p->add_msg_if_player(_("Science is a liar sometimes."));
            } else { // Durr
                p->add_msg_if_player(_("Science is... wait, what was I talking about again?"));
            }
            return;
        case 3: // Dazed and Confused
            p->add_msg_if_player(_("Behind every good man there is a woman, and that woman was Martha Washington, man."));
            if(one_in(2)) {
                p->add_msg_if_player(_("Every day, George would come home, and she would have a big fat bowl waiting for him when he came in the door, man."));
                if(one_in(2)) {
                    p->add_msg_if_player(_("She was a hip, hip, hip lady, man."));
                }
            }
            return;
        case 4:
            if(p->has_amount("money_bundle", 1)) { // Half Baked
                p->add_msg_if_player(_("You ever see the back of a twenty dollar bill... on weed?"));
                if(one_in(2)) {
                    p->add_msg_if_player(_("Oh, there's some crazy shit, man. There's a dude in the bushes. Has he got a gun? I dunno!"));
                    if(one_in(3)) {
                        p->add_msg_if_player(_("RED TEAM GO, RED TEAM GO!"));
                    }
                }
            } else if(p->has_amount("holybook_bible", 1)) {
                p->add_msg_if_player(_("You have a sudden urge to flip your bible open to Genesis 1:29..."));
            } else { // Big Lebowski
                p->add_msg_if_player(_("That rug really tied the room together..."));
            }
            return;
        case 5:
            p->add_msg_if_player(_("I used to do drugs...  I still do, but I used to, too."));
        default:
            return;
        }
    } else if(howhigh > 100 && one_in(5)) {
        int msg = rng(0, 5);
        switch(msg) {
        case 0: // Bob Marley
            p->add_msg_if_player(_("The herb reveals you to yourself."));
            return;
        case 1: // Freakazoid
            p->add_msg_if_player(_("Okay, like, the scariest thing in the world would be... if like you went to grab something and it wasn't there!"));
            return;
        case 2: // Simpsons
            p->add_msg_if_player(_("They call them fingers, but I never see them fing."));
            if(smarts > 2 && one_in(2)) {
                p->add_msg_if_player(_("... oh, there they go."));
            }
            return;
        case 3: // Bill Hicks
            p->add_msg_if_player(_("You suddenly realize that all matter is merely energy condensed to a slow vibration, and we are all one consciousness experiencing itself subjectively."));
            return;
        case 4: // Steve Martin
            p->add_msg_if_player(_("I usually only smoke in the late evening."));
            if(one_in(4)) {
                p->add_msg_if_player(_("Oh, occasionally the early evening, but usually the late evening, or the mid-evening."));
            }
            if(one_in(4)) {
                p->add_msg_if_player(_("Just the early evening, mid-evening and late evening."));
            }
            if(one_in(4)) {
                p->add_msg_if_player(_("Occasionally, early afternoon, early mid-afternoon, or perhaps the late mid-afternoon."));
            }
            if(one_in(4)) {
                p->add_msg_if_player(_("Oh, sometimes the early-mid-late-early-morning."));
            }
            if(smarts > 2) {
                p->add_msg_if_player(_("...But never at dusk."));
            }
            return;
        case 5:
        default:
            return;
        }
    } else if(howhigh > 50 && one_in(3)) {
        int msg = rng(0, 5);
        switch(msg) {
        case 0: // Cheech and Chong
            p->add_msg_if_player(_("Dave's not here, man."));
            return;
        case 1: // Real Life
            p->add_msg_if_player(_("Man, a cheeseburger sounds SO awesome right now."));
            p->hunger += 4;
            if(p->has_trait("VEGETARIAN")) {
               p->add_msg_if_player(_("Eh... maybe not."));
            } else if(p->has_trait("LACTOSE")) {
                p->add_msg_if_player(_("I guess, maybe, without the cheese... yeah."));
            }
            return;
        case 2: // Dazed and Confused
            p->add_msg_if_player( _("Walkin' down the hall, by myself, smokin' a j with fifty elves."));
            return;
        case 3: // Half Baked
            p->add_msg_if_player(_("That weed was the shiz-nittlebam snip-snap-sack."));
            return;
        case 4:
            weed_msg(p); // re-roll
        case 5:
        default:
            return;
        }
    }
}

void dis_end_msg(player &p, disease &dis)
{
    switch (disease_type_lookup[dis.type]) {
    case DI_SLEEP:
        p.add_msg_if_player(_("You wake up."));
        break;
    default:
        break;
    }
}

void dis_remove_memorial(dis_type type_string) {

  dis_type_enum type = disease_type_lookup[type_string];

  switch(type) {
    case DI_COMMON_COLD:
      g->u.add_memorial_log(pgettext("memorial_male", "Got over the cold."),
                            pgettext("memorial_female", "Got over the cold."));
      break;
    case DI_FLU:
      g->u.add_memorial_log(pgettext("memorial_male", "Got over the flu."),
                            pgettext("memorial_female", "Got over the flu."));
      break;
    case DI_FUNGUS:
      g->u.add_memorial_log(pgettext("memorial_male", "Cured the fungal infection."),
                            pgettext("memorial_female", "Cured the fungal infection."));
      break;
    case DI_BITE:
      g->u.add_memorial_log(pgettext("memorial_male", "Recovered from a bite wound."),
                            pgettext("memorial_female", "Recovered from a bite wound."));
      break;
    case DI_INFECTED:
      g->u.add_memorial_log(pgettext("memorial_male", "Recovered from an infection... this time."),
                            pgettext("memorial_female", "Recovered from an infection... this time."));
      break;

    default:
        break;
  }

}

void dis_effect(player &p, disease &dis)
{
    bool sleeping = p.has_disease("sleep");
    bool tempMsgTrigger = one_in(400);
    int bonus;
    dis_type_enum disType = disease_type_lookup[dis.type];
    int grackPower = 500;
    int BMB = 0; // Body Mass Bonus
    if (p.has_trait("FAT")) {
        BMB += 25;
    }
    if (p.has_trait("LARGE") || p.has_trait("LARGE_OK")) {
        BMB += 100;
    }
    if (p.has_trait("HUGE") || p.has_trait("HUGE_OK")) {
        BMB += 200;
    }
    bool inflictBadPsnPain = (!p.has_trait("POISRESIST") && one_in(100 + BMB)) ||
                                (p.has_trait("POISRESIST") && one_in(500 + BMB));
    switch(disType) {
        case DI_COLD:
            switch(dis.bp) {
                case bp_head:
                    switch(dis.intensity) {
                        case 3:
                            p.mod_int_bonus(-2);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(_("Your thoughts are unclear."));
                            }
                        case 2:
                            p.mod_int_bonus(-1);
                        default:
                            break;
                    }
                    break;
                case bp_mouth:
                    switch(dis.intensity) {
                        case 3:
                            p.mod_per_bonus(-2);
                        case 2:
                            p.mod_per_bonus(-1);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your face is stiff from the cold."));
                            }
                        default:
                            break;
                    }
                    break;
                case bp_torso:
                    switch(dis.intensity) {
                        case 3:
                            // Speed -20
                            p.mod_dex_bonus(-2);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your torso is freezing cold. \
                                     You should put on a few more layers."));
                            }
                        case 2:
                            p.mod_dex_bonus(-2);
                    }
                    break;
                case bp_arms:
                    switch(dis.intensity) {
                        case 3:
                            p.mod_dex_bonus(-2);
                        case 2:
                            p.mod_dex_bonus(-1);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your arms are shivering."));
                            }
                        default:
                            break;
                    }
                    break;
                case bp_hands:
                    switch(dis.intensity) {
                        case 3:
                            p.mod_dex_bonus(-2);
                        case 2:
                            p.mod_dex_bonus(-1);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your hands feel like ice."));
                            }
                        default:
                            break;
                    }
                    break;
                case bp_legs:
                    switch(dis.intensity) {
                        case 3:
                            p.mod_dex_bonus(-1);
                            p.mod_str_bonus(-1);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your legs tremble against the relentless cold."));
                            }
                        case 2:
                            p.mod_dex_bonus(-1);
                            p.mod_str_bonus(-1);
                        default:
                            break;
                    }
                    break;
                case bp_feet:
                    switch(dis.intensity) {
                        case 3:
                            p.mod_dex_bonus(-1);
                            p.mod_str_bonus(-1);
                            break;
                        case 2:
                            p.mod_dex_bonus(-1);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your feet feel frigid."));
                            }
                        default:
                            break;
                    }
                    break;
                case bp_eyes:// Eyes are not susceptible to this disease.
                case num_bp: // Suppress compiler warning [-Wswitch]
                    break;
            }
            break;

        case DI_FROSTBITE:
            switch(dis.bp) {
                case bp_hands:
                    switch(dis.intensity) {
                        case 2:
                            p.mod_dex_bonus(-3);
                        case 1:
                            if ((p.temp_cur[bp_hands] > BODYTEMP_COLD && p.pain < 40)) {
                                p.mod_pain(1);
                            }
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your fingers itch."));
                            }
                        default:
                            break;
                    }
                    break;
                case bp_feet:
                    switch(dis.intensity) {
                        case 2:
                            if ((p.temp_cur[bp_feet] > BODYTEMP_COLD && p.pain < 40)) {
                                p.mod_pain(1);
                            }
                        case 1:
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your toes itch."));
                            }
                        default:
                            break;
                    }
                    break;
                case bp_mouth:
                    switch(dis.intensity) {
                        case 2:
                            p.mod_per_bonus(-2);
                            if ((p.temp_cur[bp_mouth] > BODYTEMP_COLD && p.pain < 40)) {
                                p.mod_pain(1);
                            }
                        case 1:
                            p.mod_per_bonus(-1);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your face feels numb."));
                            }
                        default:
                            break;
                    }
                    break;
                default: // Suppress compiler warnings [-Wswitch]
                    break;
            }
            break;

        case DI_BLISTERS:
            switch(dis.bp) {
                case bp_hands:
                    p.mod_dex_bonus(-1);
                    if ( p.pain < 35 ) {
                        p.mod_pain(1);
                    }
                    if (one_in(2)) {
                        p.hp_cur[hp_arm_r]--;
                    } else {
                        p.hp_cur[hp_arm_l]--;
                    }
                    break;
                case bp_feet:
                    p.mod_str_bonus(-1);
                    if (p.pain < 35) {
                        p.mod_pain(1);
                    }
                    if (one_in(2)) {
                        p.hp_cur[hp_leg_r]--;
                    } else {
                        p.hp_cur[hp_leg_l]--;
                    }
                    break;
                case bp_mouth:
                    p.mod_per_bonus(-1);
                    p.hp_cur[hp_head]--;
                    if (p.pain < 35) {
                        p.mod_pain(1);
                    }
                    break;
                default: // Suppress compiler warnings [-Wswitch]
                    break;
            }
            break;

        case DI_HOT:
            switch(dis.bp) {
                case bp_head:
                    switch(dis.intensity) {
                        case 3:
                            if (int(calendar::turn) % 150 == 0) {
                                p.thirst++;
                            }
                            if (p.pain < 40) {
                                p.mod_pain(1);
                            }
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your head is pounding from the heat."));
                            }
                        case 2:
                            if (int(calendar::turn) % 300 == 0) {
                                p.thirst++;
                            }
                            // Hallucinations handled in game.cpp
                            if (one_in(std::min(14500, 15000 - p.temp_cur[bp_head]))) {
                                p.vomit();
                            }
                            if (p.pain < 20) {
                                p.mod_pain(1);
                            }
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("The heat is making you see things."));
                            }
                    }
                    break;
                case bp_mouth:
                    switch(dis.intensity) {
                        case 3:
                            if (int(calendar::turn) % 150 == 0) {
                                p.thirst++;
                            }
                            if (p.pain < 30) {
                                p.mod_pain(1);
                            }
                        case 2:
                            if (int(calendar::turn) % 300 == 0) {
                                p.thirst++;
                            }
                    }
                    break;
                case bp_torso:
                    switch(dis.intensity) {
                        case 3:
                            if (int(calendar::turn) % 150 == 0) {
                                p.thirst++;
                            }
                            p.mod_str_bonus(-1);
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("You are sweating profusely."));
                            }
                        case 2:
                            if (int(calendar::turn) % 300 == 0) {
                                p.thirst++;
                            }
                            p.mod_str_bonus(-1);
                        default:
                            break;
                    }
                    break;
                case bp_arms:
                    switch(dis.intensity) {
                        case 3 :
                            if (int(calendar::turn) % 150 == 0) {
                                p.thirst++;
                            }
                            if (p.pain < 30) {
                                p.mod_pain(1);
                            }
                        case 2:
                            if (int(calendar::turn) % 300 == 0) {
                                p.thirst++;
                            }
                        default:
                            break;
                    }
                    break;
                case bp_hands:
                    switch(dis.intensity) {
                        case 3:
                            p.mod_dex_bonus(-1);
                        case 2:
                            p.mod_dex_bonus(-1);
                        default:
                            break;
                    }
                    break;
                case bp_legs:
                    switch (dis.intensity) {
                        case 3 :
                            if (int(calendar::turn) % 150 == 0) {
                                p.thirst++;
                            }
                            if (p.pain < 30) {
                                p.mod_pain(1);
                            }
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your legs are cramping up."));
                            }
                        case 2:
                            if (int(calendar::turn) % 300 == 0) {
                                p.thirst++;
                            }
                    }
                    break;
                case bp_feet:
                    switch (dis.intensity) {
                        case 3 :
                            if (p.pain < 30) {
                                p.mod_pain(1);
                            }
                            if (!sleeping && tempMsgTrigger) {
                                add_msg(m_bad, _("Your feet are swelling in the heat."));
                            }
                    }
                    break;
                case bp_eyes:// Eyes are not susceptible by this disease.
                case num_bp: // Suppress compiler warning [-Wswitch]
                    break;
            }
            break;
        case DI_COMMON_COLD:
            if (int(calendar::turn) % 300 == 0) {
                p.thirst++;
            }
            if (int(calendar::turn) % 50 == 0) {
                p.fatigue++;
            }
            if (p.has_disease("took_flumed")) {
            p.mod_str_bonus(-1);
            p.mod_int_bonus(-1);
            } else {
                p.mod_str_bonus(-3);
                p.mod_dex_bonus(-1);
                p.mod_int_bonus(-2);
                p.mod_per_bonus(-1);
            }

            if (one_in(300)) {
                handle_cough(p);
            }
            break;

        case DI_FLU:
            if (int(calendar::turn) % 300 == 0) {
                p.thirst++;
            }
            if (int(calendar::turn) % 50 == 0) {
                p.fatigue++;
            }
            if (p.has_disease("took_flumed")) {
                p.mod_str_bonus(-2);
                p.mod_int_bonus(-1);
                } else {
                    p.mod_str_bonus(-4);
                    p.mod_dex_bonus(-2);
                    p.mod_int_bonus(-2);
                    p.mod_per_bonus(-1);
                    if (p.pain < 15) {
                        p.mod_pain(1);
                    }
                }
            if (one_in(300)) {
                handle_cough(p);
            }
            if (!p.has_disease("took_flumed") || one_in(2)) {
                if (one_in(3600) || will_vomit(p)) {
                    p.vomit();
                }
            }
            break;

        case DI_CRUSHED:
            p.hurtall(10);
            /*  This could be developed on later, for instance
                to deal different damage amounts to different body parts and
                to account for helmets and other armor
            */
            break;

        case DI_BOULDERING:
            switch(dis.intensity) {
                case 3:
                    p.mod_dex_bonus(-2);
                case 2:
                    p.mod_dex_bonus(-2);
                case 1:
                    p.mod_dex_bonus(-1);
                    break;
                default:
                    debugmsg("Something went wrong with DI_BOULDERING.");
                    debugmsg("Check disease.cpp");
            }
            if (p.get_dex() < 1) {
                // Add to dexterity current + 1 so it's at least 1
                p.mod_dex_bonus(abs(p.get_dex())+1);
            }
            break;

        case DI_SAP:
            p.mod_dex_bonus(-3);
            break;

        case DI_SPORES:
            // Equivalent to X in 150000 + health * 1000
            if (one_in(100) && x_in_y(dis.intensity, 150 + p.health)) {
                p.add_disease("fungus", 3601, false, 1, 1, 0, -1);
                g->u.add_memorial_log(pgettext("memorial_male", "Contracted a fungal infection."),
                                      pgettext("memorial_female", "Contracted a fungal infection."));
            }
            break;

        case DI_FUNGUS:
            manage_fungal_infection(p, dis);
            break;

        case DI_SLIMED:
            p.mod_dex_bonus(-2);
            if (will_vomit(p, 2100)) {
                p.vomit();
            } else if (one_in(4800)) {
                p.add_msg_if_player(m_bad, _("You gag and retch."));
            }
            break;

        case DI_LYING_DOWN:
            p.moves = 0;
            if (p.can_sleep()) {
                dis.duration = 1;
                p.add_msg_if_player(_("You fall asleep."));
                // Communicate to the player that he is using items on the floor
                std::string item_name = p.is_snuggling();
                if (item_name == "many") {
                    if (one_in(15) ) {
                        add_msg(_("You nestle your pile of clothes for warmth."));
                    } else {
                        add_msg(_("You use your pile of clothes for warmth."));
                    }
                } else if (item_name != "nothing") {
                    if (one_in(15)) {
                        add_msg(_("You snuggle your %s to keep warm."), item_name.c_str());
                    } else {
                        add_msg(_("You use your %s to keep warm."), item_name.c_str());
                    }
                }
                if (p.has_trait("HIBERNATE") && (p.hunger < -60)) {
                    p.add_memorial_log(pgettext("memorial_male", "Entered hibernation."),
                                       pgettext("memorial_female", "Entered hibernation."));
                    // 10 days' worth of round-the-clock Snooze.  Cata seasons default to 14 days.
                    p.fall_asleep(144000);
                }
                // If you're not fatigued enough for 10 days, you won't sleep the whole thing.
                // In practice, the fatigue from filling the tank from (no msg) to Time For Bed
                // will last about 8 days.
                if (p.hunger >= -60) {
                p.fall_asleep(6000); //10 hours, default max sleep time.
                }
            }
            if (dis.duration == 1 && !p.has_disease("sleep")) {
                p.add_msg_if_player(_("You try to sleep, but can't..."));
            }
            break;

        case DI_ALARM_CLOCK:
            {
                if (p.has_disease("sleep")) {
                    if (dis.duration == 1) {
                        if(p.has_bionic("bio_watch")) {
                            // Normal alarm is volume 12, tested against (2/3/6)d15 for
                            // normal/HEAVYSLEEPER/HEAVYSLEEPER2.
                            //
                            // It's much harder to ignore an alarm inside your own skull,
                            // so this uses an effective volume of 20.
                            const int volume = 20;
                            if ((!(p.has_trait("HEAVYSLEEPER") ||
                                   p.has_trait("HEAVYSLEEPER2")) && dice(2, 15) < volume) ||
                                (p.has_trait("HEAVYSLEEPER") && dice(3, 15) < volume) ||
                                (p.has_trait("HEAVYSLEEPER2") && dice(6, 15) < volume)) {
                                p.rem_disease("sleep");
                                add_msg(_("Your internal chronometer wakes you up."));
                            } else {
                                // 10 minute cyber-snooze
                                dis.duration += 100;
                            }
                        } else {
                            if(!g->sound(p.posx, p.posy, 12, _("beep-beep-beep!"))) {
                                // 10 minute automatic snooze
                                dis.duration += 100;
                            } else {
                                add_msg(_("You turn off your alarm-clock."));
                            }
                        }
                    }
                } else if (!p.has_disease("lying_down")) {
                    // Turn the alarm-clock off if you woke up before the alarm
                    dis.duration = 1;
                }
            }

        case DI_SLEEP:
            manage_sleep(p, dis);
            break;

        case DI_STEMCELL_TREATMENT:
            // slightly repair broken limbs. (also nonbroken limbs (unless they're too healthy))
            for (int i = 0; i < num_hp_parts; i++) {
                if (one_in(6)) {
                    if (p.hp_cur[i] < rng(0, 40)) {
                        add_msg(m_good, _("Your bones feel like rubber as they melt and remend."));
                        p.hp_cur[i]+= rng(1,8);
                    } else if (p.hp_cur[i] > rng(10, 2000)) {
                        add_msg(m_bad, _("Your bones feel like they're crumbling."));
                        p.hp_cur[i] -= rng(0,8);
                    }
                }
            }
            break;

        case DI_PKILL1:
            if (dis.duration <= 70 && dis.duration % 7 == 0 && p.pkill < 15) {
                p.pkill++;
            }
            break;

        case DI_PKILL2:
            if (dis.duration % 7 == 0 && (one_in(p.addiction_level(ADD_PKILLER)*2))) {
                p.pkill += 2;
            }
            break;

        case DI_PKILL3:
            if (dis.duration % 2 == 0 && (one_in(p.addiction_level(ADD_PKILLER)*2))) {
                p.pkill++;
            }
            break;

        case DI_PKILL_L:
            {
                bool painkillerOK = p.pkill < 40 && (one_in(p.addiction_level(ADD_PKILLER)*2));
                if (dis.duration % 20 == 0 && painkillerOK) {
                    p.pkill++;
                }
            }
            break;

        case DI_IODINE:
            if (p.radiation > 0 && one_in(16)) {
                p.radiation--;
            }
            break;

        case DI_TOOK_XANAX:
            if (dis.duration % 25 == 0 && (p.stim > 0 || one_in(2))) {
                p.stim--;
            }
            break;

        case DI_DRUNK:
            handle_alcohol(p, dis);
            break;

        case DI_VALIUM:
            if (dis.duration % 25 == 0 && (p.stim > 0 || one_in(2))) {
                p.stim--;
            }
            break;

        case DI_CIG:
            if (dis.duration >= 600) { // Smoked too much
                p.mod_str_bonus(-1);
                p.mod_dex_bonus(-1);
                if (dis.duration >= 1200 && (one_in(50) || will_vomit(p, 10))) {
                    p.vomit();
                }
            } else {
                // p.dex_cur++;
                p.mod_int_bonus(1);
                p.mod_per_bonus(1);
            }
            break;

        case DI_HIGH:
            p.mod_int_bonus(-1);
            p.mod_per_bonus(-1);
            break;

        case DI_WEED_HIGH:
            p.mod_str_bonus(-1);
            p.mod_dex_bonus(-1);
            p.mod_per_bonus(-1);
            break;

        case DI_BLEED:
            // Presuming that during the first-aid process you're putting pressure
            // on the wound or otherwise suppressing the flow. (Kits contain either
            // quikclot or bandages per the recipe.)
            if ( (one_in(6 / dis.intensity)) && (!(p.activity.type == ACT_FIRSTAID)) ) {
                p.add_msg_player_or_npc(m_bad, _("You lose some blood."),
                                               _("<npcname> loses some blood.") );
                p.mod_pain(1);
                p.hurt(dis.bp, dis.side == -1 ? 0 : dis.side, 1);
                p.mod_per_bonus(-1);
                p.mod_str_bonus(-1);
                g->m.add_field(p.posx, p.posy, p.playerBloodType(), 1);
            }
            break;

        case DI_BADPOISON:
            if( inflictBadPsnPain && !p.has_trait("NOPAIN") ) {
                p.add_msg_if_player(m_bad, _("You're suddenly wracked with pain!"));
                p.mod_pain( 2 );
                p.hurt(bp_torso, -1, rng(0, 2));
            }
            p.mod_per_bonus(-2);
            p.mod_dex_bonus(-2);
            if (!p.has_trait("POISRESIST")) {
                p.mod_str_bonus(-3);
            } else {
                p.mod_str_bonus(-1);
            }
            break;

        case DI_PARALYZEPOISON:
            p.mod_dex_bonus(-(dis.intensity / 3));
            break;

        case DI_FOODPOISON:
            bonus = 0;
            p.mod_str_bonus(-3);
            p.mod_dex_bonus(-1);
            p.mod_per_bonus(-1);
            if (p.has_trait("POISRESIST")) {
                bonus += 600;
                p.mod_str_bonus(2);
            }
            if ((one_in(300 + bonus)) && (!(p.has_trait("NOPAIN")))) {
                p.add_msg_if_player(m_bad, _("You're suddenly wracked with pain and nausea!"));
                p.hurt(bp_torso, -1, 1);
            }
            if (will_vomit(p, 100+bonus) || one_in(600 + bonus)) {
                p.vomit();
            }
            break;

        case DI_TAPEWORM:
            if (p.has_trait("PARAIMMUNE") || p.has_trait("EATHEALTH")) {
               p.rem_disease("tapeworm");
            } else {
                if(one_in(512)) {
                    p.hunger++;
                }
            }
            break;

        case DI_TETANUS:
            if (p.has_trait("INFIMMUNE")) {
               p.rem_disease("tetanus");
            }
            if (!p.has_disease("valium")) {
            p.mod_dex_bonus(-4);
            if (one_in(512)) {
                add_msg(m_bad, "Your muscles spasm.");
                p.add_effect("downed",rng(1,4));
                p.add_effect("stunned",rng(1,4));
                if (one_in(10)) {
                    p.mod_pain(rng(1, 10));
                }
            }
            }
            break;

        case DI_BLOODWORMS:
            if (p.has_trait("PARAIMMUNE")) {
               p.rem_disease("bloodworms");
            } else {
                if(one_in(512)) {
                    p.health--;
                }
            }
            break;

        case DI_BRAINWORM:
            if (p.has_trait("PARAIMMUNE")) {
               p.rem_disease("brainworm");
            } else {
                if((one_in(512)) && (!p.has_trait("NOPAIN"))) {
                    add_msg(m_bad, _("Your head hurts."));
                    p.mod_pain(rng(2, 8));
                }
                if(one_in(1024)) {
                    p.health--;
                    p.hurt(bp_head, -1, rng(0, 1));
                    if (!p.has_disease("visuals")) {
                    add_msg(m_bad, _("Your vision is getting fuzzy."));
                    p.add_disease("visuals", rng(10, 600));
                  }
                }
                if(one_in(4096)) {
                    p.health--;
                    p.hurt(bp_head, -1, rng(1, 2));
                    if (!p.has_effect("blind")) {
                    p.add_msg_if_player(m_bad, _("You can't see!"));
                    p.add_effect("blind", rng(5, 20));
                  }
                }
            }
            break;

        case DI_PAINCYSTS:
            if (p.has_trait("PARAIMMUNE")) {
               p.rem_disease("paincysts");
            } else {
                if((one_in(256)) && (!p.has_trait("NOPAIN"))) {
                    add_msg(m_bad, _("Your joints ache."));
                    p.mod_pain(rng(1, 4));
                }
                if(one_in(256)) {
                    p.fatigue++;
                }
            }
            break;

        case DI_SHAKES:
            if (p.has_disease("valium")) {
               p.rem_disease("shakes");
            }
            p.mod_dex_bonus(-4);
            p.mod_str_bonus(-1);
            break;

        case DI_DERMATIK:
            if (p.has_trait("PARAIMMUNE")) {
               p.rem_disease("dermatik");
            } else {
                handle_insect_parasites(p, dis);
            }
            break;

        case DI_WEBBED:
            p.mod_str_bonus(-2);
            p.mod_dex_bonus(-4);
            break;

        case DI_RAT:
            p.mod_int_bonus(-(int(dis.duration / 20)));
            p.mod_str_bonus(-(int(dis.duration / 50)));
            p.mod_per_bonus(-(int(dis.duration / 25)));
            if (rng(0, 100) < dis.duration / 10) {
                if (!one_in(5)) {
                    p.mutate_category("MUTCAT_RAT");
                    dis.duration /= 5;
                } else {
                    p.mutate_category("MUTCAT_TROGLOBITE");
                    dis.duration /= 3;
                }
            } else if (rng(0, 100) < dis.duration / 8) {
                if (one_in(3)) {
                    p.vomit();
                    dis.duration -= 10;
                } else {
                    add_msg(m_bad, _("You feel nauseous!"));
                    dis.duration += 3;
                }
            }
            break;

        case DI_FORMICATION:
            p.mod_int_bonus(-(dis.intensity));
            p.mod_str_bonus(-(int(dis.intensity / 3)));
            if (x_in_y(dis.intensity, 100 + 50 * p.get_int())) {
                if (!p.is_npc()) {
                    //~ %s is bodypart in accusative.
                    add_msg(m_warning, _("You start scratching your %s!"),
                            body_part_name_accusative(dis.bp, dis.side).c_str());
                    g->cancel_activity();
                } else if (g->u_see(p.posx, p.posy)) {
                    //~ 1$s is NPC name, 2$s is bodypart in accusative. 
                    add_msg(_("%1$s starts scratching their %2$s!"), p.name.c_str(),
                            body_part_name_accusative(dis.bp, dis.side).c_str());
                }
                p.moves -= 150;
                p.hurt(dis.bp, dis.side, 1);
            }
            break;

        case DI_HALLU:
            handle_deliriant(p, dis);
        break;

        case DI_ADRENALINE:
            if (dis.duration > 150) {
                // 5 minutes positive effects
                p.mod_str_bonus(5);
                p.mod_dex_bonus(3);
                p.mod_int_bonus(-8);
                p.mod_per_bonus(1);
            } else if (dis.duration == 150) {
                // 15 minutes come-down
                p.add_msg_if_player(m_bad, _("Your adrenaline rush wears off.  You feel AWFUL!"));
            } else {
                p.mod_str_bonus(-2);
                p.mod_dex_bonus(-1);
                p.mod_int_bonus(-1);
                p.mod_per_bonus(-1);
            }
            break;

        case DI_JETINJECTOR:
            if (dis.duration > 50) {
                // 15 minutes positive effects
                p.mod_str_bonus(1);
                p.mod_dex_bonus(1);
                p.mod_per_bonus(1);
            } else if (dis.duration == 50) {
                // 5 minutes come-down
                p.add_msg_if_player(m_bad, _("The jet injector's chemicals wear off.  You feel AWFUL!"));
            } else {
                p.mod_str_bonus(-1);
                p.mod_dex_bonus(-2);
                p.mod_int_bonus(-1);
                p.mod_per_bonus(-2);
            }
            break;

        case DI_ASTHMA:
            if (dis.duration > 1200) {
                p.add_msg_if_player(m_bad, _("Your asthma overcomes you.\nYou asphyxiate."));
                g->u.add_memorial_log(pgettext("memorial_male", "Succumbed to an asthma attack."),
                                      pgettext("memorial_female", "Succumbed to an asthma attack."));
                p.hurtall(500);
            } else if (dis.duration > 700) {
                if (one_in(20)) {
                    p.add_msg_if_player(m_bad, _("You wheeze and gasp for air."));
                }
            }
            p.mod_str_bonus(-2);
            p.mod_dex_bonus(-3);
            break;

        case DI_GRACK:
            p.mod_str_bonus(grackPower);
            p.mod_dex_bonus(grackPower);
            p.mod_int_bonus(grackPower);
            p.mod_per_bonus(grackPower);
            break;

        case DI_METH:
            if (dis.duration > 200) {
                p.mod_str_bonus(1);
                p.mod_dex_bonus(2);
                p.mod_int_bonus(2);
                p.mod_per_bonus(3);
            } else {
                p.mod_str_bonus(-3);
                p.mod_dex_bonus(-2);
                p.mod_int_bonus(-1);
                p.mod_per_bonus(-2);
                if (one_in(150)) {
                    p.add_msg_if_player(m_bad, _("You feel paranoid. They're watching you."));
                    p.mod_pain(1);
                    p.fatigue += dice(1,6);
                } else if (one_in(500)) {
                    p.add_msg_if_player(m_bad, _("You feel like you need less teeth. You pull one out, and it is rotten to the core."));
                    p.mod_pain(1);
                } else if (one_in(500)) {
                    p.add_msg_if_player(m_bad, _("You notice a large abscess. You pick at it."));
                    body_part bp = random_body_part(true);
                    int side = random_side(bp);
                    p.add_disease("formication", 600, false, 1, 3, 0, 1, bp, side, true);
                    p.mod_pain(1);
                } else if (one_in(500)) {
                    p.add_msg_if_player(m_bad, _("You feel so sick, like you've been poisoned, but you need more. So much more."));
                    p.vomit();
                    p.fatigue += dice(1,6);
                }
                p.fatigue += 1;
            }
            if (will_vomit(p, 2000)) {
                p.vomit();
            }
            break;

        case DI_TELEGLOW:
            // Default we get around 300 duration points per teleport (possibly more
            // depending on the source).
            // TODO: Include a chance to teleport to the nether realm.
            // TODO: This with regards to NPCS
            if(&p != &(g->u)) {
                // NO, no teleporting around the player because an NPC has teleglow!
                return;
            }
            if (dis.duration > 6000) {
                // 20 teles (no decay; in practice at least 21)
                if (one_in(1000 - ((dis.duration - 6000) / 10))) {
                    if (!p.is_npc()) {
                        add_msg(_("Glowing lights surround you, and you teleport."));
                        g->u.add_memorial_log(pgettext("memorial_male", "Spontaneous teleport."),
                                              pgettext("memorial_female", "Spontaneous teleport."));
                    }
                    g->teleport();
                    if (one_in(10)) {
                        p.rem_disease("teleglow");
                    }
                }
                if (one_in(1200 - ((dis.duration - 6000) / 5)) && one_in(20)) {
                    if (!p.is_npc()) {
                        add_msg(m_bad, _("You pass out."));
                    }
                    p.fall_asleep(1200);
                    if (one_in(6)) {
                        p.rem_disease("teleglow");
                    }
                }
            }
            if (dis.duration > 3600) {
                // 12 teles
                if (one_in(4000 - int(.25 * (dis.duration - 3600)))) {
                    MonsterGroupResult spawn_details = MonsterGroupManager::GetResultFromGroup("GROUP_NETHER");
                    monster beast(GetMType(spawn_details.name));
                    int x, y;
                    int tries = 0;
                    do {
                        x = p.posx + rng(-4, 4);
                        y = p.posy + rng(-4, 4);
                        tries++;
                        if (tries >= 10) {
                            break;
                        }
                    } while (((x == p.posx && y == p.posy) || g->mon_at(x, y) != -1));
                    if (tries < 10) {
                        if (g->m.move_cost(x, y) == 0) {
                            g->m.ter_set(x, y, t_rubble);
                        }
                        beast.spawn(x, y);
                        g->add_zombie(beast);
                        if (g->u_see(x, y)) {
                            g->cancel_activity_query(_("A monster appears nearby!"));
                            add_msg(m_warning, _("A portal opens nearby, and a monster crawls through!"));
                        }
                        if (one_in(2)) {
                            p.rem_disease("teleglow");
                        }
                    }
                }
                if (one_in(3500 - int(.25 * (dis.duration - 3600)))) {
                    p.add_msg_if_player(m_bad, _("You shudder suddenly."));
                    p.mutate();
                    if (one_in(4))
                    p.rem_disease("teleglow");
                }
            } if (dis.duration > 2400) {
                // 8 teleports
                if (one_in(10000 - dis.duration) && !p.has_disease("valium")) {
                    p.add_disease("shakes", rng(40, 80));
                }
                if (one_in(12000 - dis.duration)) {
                    p.add_msg_if_player(m_bad, _("Your vision is filled with bright lights..."));
                    p.add_effect("blind", rng(10, 20));
                    if (one_in(8)) {
                        p.rem_disease("teleglow");
                    }
                }
                if (one_in(5000) && !p.has_disease("hallu")) {
                    p.add_disease("hallu", 3600);
                    if (one_in(5)) {
                        p.rem_disease("teleglow");
                    }
                }
            }
            if (one_in(4000)) {
                p.add_msg_if_player(m_bad, _("You're suddenly covered in ectoplasm."));
                p.add_disease("boomered", 100);
                if (one_in(4)) {
                    p.rem_disease("teleglow");
                }
            }
            if (one_in(10000)) {
                p.add_disease("fungus", 3601, false, 1, 1, 0, -1);
                p.rem_disease("teleglow");
            }
            break;

        case DI_ATTENTION:
            if (one_in(100000 / dis.duration) && one_in(100000 / dis.duration) && one_in(250)) {
                MonsterGroupResult spawn_details = MonsterGroupManager::GetResultFromGroup("GROUP_NETHER");
                monster beast(GetMType(spawn_details.name));
                int x, y;
                int tries = 0;
                do {
                    x = p.posx + rng(-4, 4);
                    y = p.posy + rng(-4, 4);
                    tries++;
                } while (((x == p.posx && y == p.posy) || g->mon_at(x, y) != -1) && tries < 10);
                if (tries < 10) {
                    if (g->m.move_cost(x, y) == 0) {
                        g->m.ter_set(x, y, t_rubble);
                    }
                    beast.spawn(x, y);
                    g->add_zombie(beast);
                    if (g->u_see(x, y)) {
                        g->cancel_activity_query(_("A monster appears nearby!"));
                        add_msg(m_warning, _("A portal opens nearby, and a monster crawls through!"));
                    }
                    dis.duration /= 4;
                }
            }
            break;

        case DI_EVIL:
            handle_evil(p, dis);
            break;

        case DI_BITE:
            handle_bite_wound(p, dis);
            break;

        case DI_LIGHTSNARE:
            p.moves = -500;
            if(one_in(10)) {
                add_msg(_("You attempt to free yourself from the snare."));
            }
            break;

        case DI_HEAVYSNARE:
            p.moves = -500;
            if(one_in(20)) {
                add_msg(_("You attempt to free yourself from the snare."));
            }
            break;

        case DI_INFECTED:
            handle_infected_wound(p, dis);
            break;

        case DI_RECOVER:
            handle_recovery(p, dis);
            break;

        case DI_MA_BUFF:
            if (ma_buffs.find(dis.buff_id) != ma_buffs.end()) {
              ma_buff b = ma_buffs[dis.buff_id];
              if (b.is_valid_player(p)) {
                b.apply_player(p);
              }
              else {
                p.rem_disease(dis.type);
              }
            }
            break;

        case DI_LACKSLEEP:
            p.mod_str_bonus(-1);
            p.mod_dex_bonus(-1);
            p.mod_int_bonus(-2);
            p.mod_per_bonus(-2);
            break;

        case DI_GRABBED:
            p.blocks_left -= dis.intensity;
            p.dodges_left = 0;
            p.rem_disease(dis.type);
            break;
        default: // Other diseases don't have any effects. Suppress warning.
            break;
    }
}

int disease_speed_boost(disease dis)
{
    dis_type_enum type = disease_type_lookup[dis.type];
    switch (type) {
        case DI_COLD:
            switch (dis.bp) {
                case bp_torso:
                    switch (dis.intensity) {
                        case 1 : return  -2;
                        case 2 : return  -5;
                        case 3 : return -20;
                    }
                case bp_legs:
                    switch (dis.intensity) {
                        case 1 : return  -2;
                        case 2 : return  -5;
                        case 3 : return -20;
                    }
                default:
                    return 0;
            }
            break;
        case DI_FROSTBITE:
            switch (dis.bp) {
                case bp_feet:
                    switch (dis.intensity) {
                        case 2 : return -4;
                    }
                default:
                    return 0;
            }
            break;
        case DI_HOT:
            switch(dis.bp) {
                case bp_head:
                    switch (dis.intensity) {
                        case 1 : return  -2;
                        case 2 : return  -5;
                        case 3 : return -20;
                    }
                case bp_torso:
                    switch (dis.intensity) {
                        case 1 : return  -2;
                        case 2 : return  -5;
                        case 3 : return -20;
                    }
                default:
                    return 0;
            }
        break;

        case DI_SPORES:
            switch (dis.bp) {
                case bp_head:
                    switch (dis.intensity) {
                        case 1: return -10;
                        case 2: return -15;
                        case 3: return -20;
                    }
                case bp_torso:
                    switch (dis.intensity) {
                        case 1: return -15;
                        case 2: return -20;
                        case 3: return -25;
                    }
                case bp_arms:
                    switch (dis.intensity) {
                        case 1: return -5;
                        case 2: return -10;
                        case 3: return -15;
                    }
                case bp_legs:
                    switch (dis.intensity) {
                        case 1: return -5;
                        case 2: return -10;
                        case 3: return -15;
                    }
                default:
                    return 0;
            }

        case DI_PARALYZEPOISON:
            return dis.intensity * -5;

        case DI_INFECTION:  return -80;
        case DI_SAP:        return -25;
        case DI_SLIMED:     return -25;
        case DI_BADPOISON:  return -10;
        case DI_FOODPOISON: return -20;
        case DI_WEBBED:     return -25;
        case DI_ADRENALINE: return (dis.duration > 150 ? 40 : -10);
        case DI_ASTHMA:     return 0 - int(dis.duration / 5);
        case DI_GRACK:      return +20000;
        case DI_METH:       return (dis.duration > 600 ? 50 : -40);
        case DI_BOULDERING: return ( 0 - (dis.intensity * 10));
        case DI_LACKSLEEP:  return -5;
        case DI_GRABBED:    return -25;
        default:            break;
    }
    return 0;
}

std::string dis_name(disease& dis)
{
    // Maximum length of returned string is 26 characters
    dis_type_enum type = disease_type_lookup[dis.type];
    switch (type) {
    case DI_NULL: return "";
    case DI_COLD:
        switch (dis.bp) {
            case bp_head:
                switch (dis.intensity) {
                case 1: return _("Chilly head");
                case 2: return _("Cold head!");
                case 3: return _("Freezing head!!");}
            case bp_mouth:
                switch (dis.intensity) {
                case 1: return _("Chilly face");
                case 2: return _("Cold face!");
                case 3: return _("Freezing face!!");}
            case bp_torso:
                switch (dis.intensity) {
                case 1: return _("Chilly torso");
                case 2: return _("Cold torso!");
                case 3: return _("Freezing torso!!");}
            case bp_arms:
                switch (dis.intensity) {
                case 1: return _("Chilly arms");
                case 2: return _("Cold arms!");
                case 3: return _("Freezing arms!!");}
            case bp_hands:
                switch (dis.intensity) {
                case 1: return _("Chilly hands");
                case 2: return _("Cold hands!");
                case 3: return _("Freezing hands!!");}
            case bp_legs:
                switch (dis.intensity) {
                case 1: return _("Chilly legs");
                case 2: return _("Cold legs!");
                case 3: return _("Freezing legs!!");}
            case bp_feet:
                switch (dis.intensity) {
                case 1: return _("Chilly feet");
                case 2: return _("Cold feet!");
                case 3: return _("Freezing feet!!");}
            case bp_eyes: // Eyes are not susceptible by this disease.
            case num_bp: // Suppress compiler warning [-Wswitch]
                break; // function return "" in this case
        }

    case DI_FROSTBITE:
        switch(dis.bp) {
            case bp_hands:
                switch (dis.intensity) {
                case 1: return _("Frostnip - hands");
                case 2: return _("Frostbite - hands");}
            case bp_feet:
                switch (dis.intensity) {
                case 1: return _("Frostnip - feet");
                case 2: return _("Frostbite - feet");}
            case bp_mouth:
                switch (dis.intensity) {
                case 1: return _("Frostnip - face");
                case 2: return _("Frostbite - face");}
            default: // Suppress compiler warning [-Wswitch]
                break; // function return "" in this case
        }

    case DI_HOT:
        switch (dis.bp) {
            case bp_head:
                switch (dis.intensity) {
                case 1: return _("Warm head");
                case 2: return _("Hot head!");
                case 3: return _("Scorching head!!");}
            case bp_mouth:
                switch (dis.intensity) {
                case 1: return _("Warm face");
                case 2: return _("Hot face!");
                case 3: return _("Scorching face!!");}
            case bp_torso:
                switch (dis.intensity) {
                case 1: return _("Warm torso");
                case 2: return _("Hot torso!");
                case 3: return _("Scorching torso!!");}
            case bp_arms:
                switch (dis.intensity) {
                case 1: return _("Warm arms");
                case 2: return _("Hot arms!");
                case 3: return _("Scorching arms!!");}
            case bp_hands:
                switch (dis.intensity) {
                case 1: return _("Warm hands");
                case 2: return _("Hot hands!");
                case 3: return _("Scorching hands!!");}
                break;
            case bp_legs:
                switch (dis.intensity) {
                case 1: return _("Warm legs");
                case 2: return _("Hot legs!");
                case 3: return _("Scorching legs!!");}
            case bp_feet:
                switch (dis.intensity) {
                case 1: return _("Warm feet");
                case 2: return _("Hot feet!");
                case 3: return _("Scorching feet!!");}
            case bp_eyes: // Eyes are not susceptible by this disease.
            case num_bp: // Suppress compiler warning [-Wswitch]
                break; // function return "" in this case
        }

    case DI_BLISTERS:
        switch(dis.bp) {
            case bp_mouth:
                return _("Blisters - face");
            case bp_torso:
                return _("Blisters - torso");
            case bp_arms:
                return _("Blisters - arms");
            case bp_hands:
                return _("Blisters - hands");
            case bp_legs:
                return _("Blisters - legs");
            case bp_feet:
                return _("Blisters - feet");
            default: // Suppress compiler warning [-Wswitch]
                break;
        }

    case DI_COMMON_COLD: return _("Common Cold");
    case DI_FLU: return _("Influenza");
    case DI_SAP: return _("Sap-coated");

    case DI_SPORES:
    {
        std::string status = "";
        switch (dis.intensity) {
        case 1: status = _("Spore dusted - "); break;
        case 2: status = _("Spore covered - "); break;
        case 3: status = _("Spore coated - "); break;
        }
        switch (dis.bp) {
            case bp_head:
                status += _("Head");
                break;
            case bp_torso:
                status += _("Torso");
                break;
            case bp_arms:
                if (dis.side == 0) {
                    status += _("Left Arm");
                } else if (dis.side == 1) {
                    status += _("Right Arm");
                }
                break;
            case bp_legs:
                if (dis.side == 0) {
                    status += _("Left Leg");
                } else if (dis.side == 1) {
                    status += _("Right Leg");
                }
                break;
            default: // Suppress compile warning [-Wswitch]
                break;
        }
        return status;
    }

    case DI_SLIMED: return _("Slimed");
    case DI_STUNNED: return _("Stunned");
    case DI_DOWNED: return _("Downed");
    case DI_BLEED:
    {
        std::string status = "";
        switch (dis.intensity) {
        case 1: status = _("Bleeding - "); break;
        case 2: status = _("Bad Bleeding - "); break;
        case 3: status = _("Heavy Bleeding - "); break;
        }
        switch (dis.bp) {
            case bp_head:
                status += _("Head");
                break;
            case bp_torso:
                status += _("Torso");
                break;
            case bp_arms:
                if (dis.side == 0) {
                    status += _("Left Arm");
                } else if (dis.side == 1) {
                    status += _("Right Arm");
                }
                break;
            case bp_legs:
                if (dis.side == 0) {
                    status += _("Left Leg");
                } else if (dis.side == 1) {
                    status += _("Right Leg");
                }
                break;
            default: // Suppress compiler warning [-Wswitch]
                break;
        }
        return status;
    }
    case DI_PARALYZEPOISON:
    {
        if (dis.intensity > 15) {
                return _("Completely Paralyzed");
        } else if (dis.intensity > 10) {
                return _("Partially Paralyzed");
        } else if (dis.intensity > 5) {
                return _("Sluggish");
        } else {
                return _("Slowed");
        }
    }

    case DI_BADPOISON: return _("Badly Poisoned");
    case DI_FOODPOISON: return _("Food Poisoning");
    case DI_SHAKES: return _("Shakes");
    case DI_FORMICATION:
    {
        std::string status = "";
        switch (dis.intensity) {
        case 1: status = _("Itchy skin - "); break;
        case 2: status = _("Writhing skin - "); break;
        case 3: status = _("Bugs in skin - "); break;
        }
        switch (dis.bp) {
            case bp_head:
                status += _("Head");
                break;
            case bp_torso:
                status += _("Torso");
                break;
            case bp_arms:
                if (dis.side == 0) {
                    status += _("Left Arm");
                } else if (dis.side == 1) {
                    status += _("Right Arm");
                }
                break;
            case bp_legs:
                if (dis.side == 0) {
                    status += _("Left Leg");
                } else if (dis.side == 1) {
                    status += _("Right Leg");
                }
                break;
            default: // Suppress compiler warning [-Wswitch]
                break;
        }
        return status;
    }
    case DI_WEBBED: return _("Webbed");
    case DI_RAT: return _("Ratting");
    case DI_DRUNK:
        if (dis.duration > 2200) return _("Wasted");
        if (dis.duration > 1400) return _("Trashed");
        if (dis.duration > 800)  return _("Drunk");
        else return _("Tipsy");

    case DI_CIG: return _("Nicotine");
    case DI_HIGH: return _("High");
    case DI_VISUALS: return _("Hallucinating");

    case DI_ADRENALINE:
        if (dis.duration > 150) return _("Adrenaline Rush");
        else return _("Adrenaline Comedown");

    case DI_JETINJECTOR:
        if (dis.duration > 150) return _("Chemical Rush");
        else return _("Chemical Comedown");

    case DI_ASTHMA:
        if (dis.duration > 800) return _("Heavy Asthma");
        else return _("Asthma");

    case DI_GRACK: return _("RELEASE THE GRACKEN!!!!");

    case DI_METH:
        if (dis.duration > 200) return _("High on Meth");
        else return _("Meth Comedown");

    case DI_IN_PIT: return _("Stuck in Pit");
    case DI_BOULDERING: return _("Clambering Over Rubble");

    case DI_STEMCELL_TREATMENT: return _("Stem cell treatment");
    case DI_BITE:
    {
        std::string status = "";
        if ((dis.duration > 2401) || (g->u.has_trait("INFIMMUNE"))) {status = _("Bite - ");
        } else { status = _("Painful Bite - ");
        }
        switch (dis.bp) {
            case bp_head:
                status += _("Head");
                break;
            case bp_torso:
                status += _("Torso");
                break;
            case bp_arms:
                if (dis.side == 0) {
                    status += _("Left Arm");
                } else if (dis.side == 1) {
                    status += _("Right Arm");
                }
                break;
            case bp_legs:
                if (dis.side == 0) {
                    status += _("Left Leg");
                } else if (dis.side == 1) {
                    status += _("Right Leg");
                }
                break;
            default: // Suppress compiler warning [-Wswitch]
                break;
        }
        return status;
    }
    case DI_INFECTED:
    {
        std::string status = "";
        if (dis.duration > 8401) {
            status = _("Infected - ");
        } else if (dis.duration > 3601) {
            status = _("Badly Infected - ");
        } else {
            status = _("Pus Filled - ");
        }
        switch (dis.bp) {
            case bp_head:
                status += _("Head");
                break;
            case bp_torso:
                status += _("Torso");
                break;
            case bp_arms:
                if (dis.side == 0) {
                    status += _("Left Arm");
                } else if (dis.side == 1) {
                    status += _("Right Arm");
                }
                break;
            case bp_legs:
                if (dis.side == 0) {
                    status += _("Left Leg");
                } else if (dis.side == 1) {
                    status += _("Right Leg");
                }
                break;
            default: // Suppress compiler warning [-Wswitch]
                break;
        }
        return status;
    }
    case DI_RECOVER: return _("Recovering From Infection");

    case DI_MA_BUFF:
        if (ma_buffs.find(dis.buff_id) != ma_buffs.end()) {
            std::stringstream buf;
            if (ma_buffs[dis.buff_id].max_stacks > 1) {
                buf << ma_buffs[dis.buff_id].name << " (" << dis.intensity << ")";
                return buf.str();
            } else {
                buf << ma_buffs[dis.buff_id].name.c_str();
                return buf.str();
            }
        } else
            return "Invalid martial arts buff";

    case DI_LACKSLEEP: return _("Lacking Sleep");
    case DI_GRABBED: return _("Grabbed");
    default: break;
    }
    return "";
}

std::string dis_combined_name(disease& dis)
{
    // Maximum length of returned string is 19 characters
    dis_type_enum type = disease_type_lookup[dis.type];
    switch (type) {
        case DI_SPORES:
            return _("Spore covered");
        case DI_COLD:
            return _("Cold");
        case DI_FROSTBITE:
            return _("Frostbite");
        case DI_HOT:
            return _("Hot");
        default: // Suppress compiler warnings [-Wswitch]
            break;
    }
    return "";
}

std::string dis_description(disease& dis)
{
    int strpen, dexpen, intpen, perpen, speed_pen;
    std::stringstream stream;
    dis_type_enum type = disease_type_lookup[dis.type];
    switch (type) {

    case DI_NULL:
        return _("None");

    case DI_COLD:
        switch(dis.bp) {
            case bp_head:
                switch (dis.intensity) {
                case 1: return _("Your head is exposed to the cold.");
                case 2: return _("Your head is very exposed to the cold. It is hard to concentrate.");
                case 3: return _("Your head is extremely cold.  You can barely think straight.");
                }
            case bp_mouth:
                switch (dis.intensity) {
                case 1: return _("Your face is exposed to the cold.");
                case 2: return _("Your face is very exposed to the cold.");
                case 3: return _("Your face is dangerously cold.");
                }
            case bp_torso:
                switch (dis.intensity) {
                case 1: return _("Your torso is exposed to the cold.");
                case 2: return _("Your torso is very cold, and your actions are uncoordinated.");
                case 3: return _("Your torso is dangerously cold. Your actions are very uncoordinated.");
                }
            case bp_arms:
                switch (dis.intensity) {
                case 1: return _("Your arms are exposed to the cold.");
                case 2: return _("Your arms are very exposed to the cold. Your arms are shivering.");
                case 3: return _("Your arms are dangerously cold. Your arms are shivering uncontrollably");
                }
            case bp_hands:
                switch (dis.intensity) {
                case 1: return _("Your hands are exposed to the cold.");
                case 2: return _("Your hands are shivering from the cold.");
                case 3: return _("Your hands are shivering uncontrollably from the extreme cold.");
                }
            case bp_legs:
                switch (dis.intensity) {
                case 1: return _("Your legs are exposed to the cold.");
                case 2: return _("Your legs are very exposed to the cold. Your strength is sapped.");
                case 3: return _("Your legs are dangerously cold. Your strength is sapped.");
                }
            case bp_feet:
                switch (dis.intensity) {
                case 1: return _("Your feet are exposed to the cold.");
                case 2: return _("Your feet are very exposed to the cold. Your strength is sapped.");
                case 3: return _("Your feet are dangerously cold. Your strength is sapped.");
                }
            case bp_eyes:// Eyes are not susceptible by this disease.
            case num_bp: // Suppress compiler warning [-Wswitch]
                break;
        }

    case DI_FROSTBITE:
        if (g->u.has_trait("NOPAIN")) {
            switch(dis.bp) {
            case bp_hands:
                switch (dis.intensity) {
                case 1: return _("\
Your hands are frostnipped from the prolonged exposure to the cold and have gone numb.");
                case 2: return _("\
Your hands are frostbitten from the prolonged exposure to the cold. The tissues in your hands are frozen.");
                }
            case bp_feet:
                switch (dis.intensity) {
                case 1: return _("\
Your feet are frostnipped from the prolonged exposure to the cold and have gone numb.");
                case 2: return _("\
Your feet are frostbitten from the prolonged exposure to the cold. The tissues in your feet are frozen.");
                }
            case bp_mouth:
                switch (dis.intensity) {
                case 1: return _("\
Your face is frostnipped from the prolonged exposure to the cold and has gone numb.");
                case 2: return _("\
Your face is frostbitten from the prolonged exposure to the cold. The tissues in your face are frozen.");
                }
            case bp_torso:
                return _("\
Your torso is frostbitten from prolonged exposure to the cold.");
            case bp_arms:
                return _("\
Your arms are frostbitten from prolonged exposure to the cold.");
            case bp_legs:
                return _("\
Your legs are frostbitten from prolonged exposure to the cold.");
            default: // Suppress compiler warning [-Wswitch]
                break;
          }
        } else {
            switch(dis.bp) {
            case bp_hands:
                switch (dis.intensity) {
                case 1: return _("\
Your hands are frostnipped from the prolonged exposure to the cold and have gone numb. When the blood begins to flow, it will be painful.");
                case 2: return _("\
Your hands are frostbitten from the prolonged exposure to the cold. The tissues in your hands are frozen.");
                }
            case bp_feet:
                switch (dis.intensity) {
                case 1: return _("\
Your feet are frostnipped from the prolonged exposure to the cold and have gone numb. When the blood begins to flow, it will be painful.");
                case 2: return _("\
Your feet are frostbitten from the prolonged exposure to the cold. The tissues in your feet are frozen.");
                }
            case bp_mouth:
                switch (dis.intensity) {
                case 1: return _("\
Your face is frostnipped from the prolonged exposure to the cold and has gone numb. When the blood begins to flow, it will be painful.");
                case 2: return _("\
Your face is frostbitten from the prolonged exposure to the cold. The tissues in your face are frozen.");
                }
            case bp_torso:
                return _("\
Your torso is frostbitten from prolonged exposure to the cold. It is extremely painful.");
            case bp_arms:
                return _("\
Your arms are frostbitten from prolonged exposure to the cold. It is extremely painful.");
            case bp_legs:
                return _("\
Your legs are frostbitten from prolonged exposure to the cold. It is extremely painful.");
            default: // Suppress compiler warning [-Wswitch]
                break;
            }
        }

    case DI_HOT:
        switch (dis.bp) {
            case bp_head:
                switch (dis.intensity) {
                case 1: return _("Your head feels warm.");
                case 2: return _("Your head is sweating from the heat. You feel nauseated.");
                case 3: return _("Your head is sweating profusely. You feel very nauseated.");
                }
            case bp_mouth:
                switch (dis.intensity) {
                case 1: return _("Your face feels warm.");
                case 2: return _("Your face is sweating from the heat, making it hard to see.");
                case 3: return _("Your face is sweating profusely, making it hard to see.");
                }
            case bp_torso:
                switch (dis.intensity) {
                case 1: return _("Your torso feels warm.");
                case 2: return _("Your torso is sweating from the heat. You feel weak.");
                case 3: return _("Your torso is sweating profusely. You feel very weak.");
                }
            case bp_arms:
                switch (dis.intensity) {
                case 1: return _("Your arms feel warm.");
                case 2: return _("Your arms are sweating from the heat.");
                case 3: return _("Your arms are sweating profusely. Your muscles are cramping.");
                }
            case bp_hands:
                switch (dis.intensity) {
                case 1: return _("Your hands feel warm.");
                case 2: return _("Your hands feel hot and uncoordinated.");
                case 3: return _("Your hands feel disgustingly hot and are very uncoordinated.");
                }
            case bp_legs:
                switch (dis.intensity) {
                case 1: return _("Your legs feel warm.");
                case 2: return _("Your legs are sweating from the heat.");
                case 3: return _("Your legs are sweating profusely. Your muscles are cramping.");
                }
            case bp_feet:
                switch (dis.intensity) {
                case 1: return _("Your feet feel warm.");
                case 2: return _("Your feet are swollen due to the heat.");
                case 3: return _("Your feet are swollen due to the heat.");
                }
            case bp_eyes:// Eyes are not susceptible by this disease.
            case num_bp: // Suppress compiler warning [-Wswitch]
                break;
        }

    case DI_BLISTERS:
      if (g->u.has_trait("NOPAIN")) {
        switch (dis.bp) {
            case bp_mouth:
                return _("\
Your face is blistering from the intense heat.");
            case bp_torso:
                return _("\
Your torso is blistering from the intense heat.");
            case bp_arms:
                return _("\
Your arms are blistering from the intense heat.");
            case bp_hands:
                return _("\
Your hands are blistering from the intense heat.");
            case bp_legs:
                return _("\
Your legs are blistering from the intense heat.");
            case bp_feet:
                return _("\
Your feet are blistering from the intense heat.");
            default: // Suppress compiler warning [-Wswitch]
                break;
        }
      } else {
          switch (dis.bp) {
            case bp_mouth:
                return _("\
Your face is blistering from the intense heat. It is extremely painful.");
            case bp_torso:
                return _("\
Your torso is blistering from the intense heat. It is extremely painful.");
            case bp_arms:
                return _("\
Your arms are blistering from the intense heat. It is extremely painful.");
            case bp_hands:
                return _("\
Your hands are blistering from the intense heat. It is extremely painful.");
            case bp_legs:
                return _("\
Your legs are blistering from the intense heat. It is extremely painful.");
            case bp_feet:
                return _("\
Your feet are blistering from the intense heat. It is extremely painful.");
            default: // Suppress compiler warning [-Wswitch]
                break;
          }
      }

    case DI_COMMON_COLD:
        return _(
        "Increased thirst;   Frequent coughing\n"
        "Strength - 3;   Dexterity - 1;   Intelligence - 2;   Perception - 1\n"
        "Symptoms alleviated by medication (cough syrup).");

    case DI_FLU:
        return _(
        "Increased thirst;   Frequent coughing;   Occasional vomiting\n"
        "Strength - 4;   Dexterity - 2;   Intelligence - 2;   Perception - 1\n"
        "Symptoms alleviated by medication (cough syrup).");

    case DI_CRUSHED: return "If you're seeing this, there is a bug in disease.cpp!";

    case DI_BOULDERING:
        switch (dis.intensity){
        case 1:
            stream << _(
            "Dexterity - 1;   Speed -10%\n"
            "You are being slowed by climbing over a pile of rubble.");
        case 2:
            stream << _(
            "Dexterity - 3;   Speed -20%\n"
            "You are being slowed by climbing over a heap of rubble.");
        case 3:
            stream << _(
            "Dexterity - 5;   Speed -30%\n"
            "You are being slowed by climbing over a mountain of rubble.");
        }
        return stream.str();

    case DI_STEMCELL_TREATMENT: return _("Your insides are shifting in strange ways as the treatment takes effect.");

    case DI_SAP:
        return _("Dexterity - 3;   Speed - 25");

    case DI_SPORES:
        speed_pen = disease_speed_boost(dis);
        stream << string_format(_(
                "Speed %d%%\n"
                "You can feel the tiny spores sinking directly into your flesh."), speed_pen);
        return stream.str();

    case DI_SLIMED:
        return _("Speed -25%;   Dexterity - 2");

    case DI_STUNNED: return _("Your movement is randomized.");

    case DI_DOWNED: return _("You're knocked to the ground.  You have to get up before you can move.");

    case DI_BLEED:
        switch (dis.intensity) {
            case 1:
                return _("You are slowly losing blood.");
            case 2:
                return _("You are losing blood.");
            case 3:
                return _("You are rapidly loosing blood.");
        }

    case DI_PARALYZEPOISON:
        dexpen = int(dis.intensity / 3);
        if (dexpen > 0) {
            stream << string_format(_("Dexterity - %d"), dexpen);
        }
        return stream.str();

    case DI_BADPOISON:
        if (g->u.has_trait("NOPAIN")) {
            return _("Perception - 2;   Dexterity - 2;\n"
                     "Strength - 3 IF not resistant, -1 otherwise\n"
                     "Frequent damage.");
        } else {
            return _("Perception - 2;   Dexterity - 2;\n"
                     "Strength - 3 IF not resistant, -1 otherwise\n"
                     "Frequent pain and/or damage.");
        }

    case DI_FOODPOISON:
        if (g->u.has_trait("NOPAIN")) {
            return _("Speed - 35%;   Strength - 3;   Dexterity - 1;   Perception - 1\n"
                     "Your stomach is extremely upset, and you are quite nauseous.");
        } else {
            return _("Speed - 35%;   Strength - 3;   Dexterity - 1;   Perception - 1\n"
                     "Your stomach is extremely upset, and you keep having pangs of pain and nausea.");
        }

    case DI_SHAKES:
        return _("Strength - 1;   Dexterity - 4");

    case DI_FORMICATION:
    {
        intpen = int(dis.intensity);
        strpen = int(dis.intensity / 3);
        stream << _("You stop to scratch yourself frequently; high intelligence helps you resist\n"
        "this urge.\n");
        if (intpen > 0) {
            stream << string_format(_("Intelligence - %d;   "), intpen);
        }
        if (strpen > 0) {
            stream << string_format(_("Strength - %d;   "), strpen);
        }
        return stream.str();
    }

    case DI_WEBBED:
        return _(
        "Strength - 1;   Dexterity - 4;   Speed - 25");

    case DI_RAT:
    {
        intpen = int(dis.duration / 20);
        perpen = int(dis.duration / 25);
        strpen = int(dis.duration / 50);
        stream << _("You feel nauseated and rat-like.\n");
        if (intpen > 0) {
            stream << string_format(_("Intelligence - %d;   "), intpen);
        }
        if (perpen > 0) {
            stream << string_format(_("Perception - %d;   "), perpen);
        }
        if (strpen > 0) {
            stream << string_format(_("Strength - %d;   "), strpen);
        }
        return stream.str();
    }

    case DI_DRUNK:
    {
        perpen = int(dis.duration / 1000);
        dexpen = int(dis.duration / 1000);
        intpen = int(dis.duration /  700);
        strpen = int(dis.duration / 1500);
        if (strpen > 0) {
            stream << string_format(_("Strength - %d;   "), strpen);
        }
        else if (dis.duration <= 600)
            stream << _("Strength + 1;    ");
        if (dexpen > 0) {
            stream << string_format(_("Dexterity - %d;   "), dexpen);
        }
        if (intpen > 0) {
            stream << string_format(_("Intelligence - %d;   "), intpen);
        }
        if (perpen > 0) {
            stream << string_format(_("Perception - %d;   "), perpen);
        }
        return stream.str();
    }

    case DI_CIG:
        if (dis.duration >= 600)
            return _(
            "Strength - 1;   Dexterity - 1\n"
            "You smoked too much.");
        else
            return _(
            "Dexterity + 1;   Intelligence + 1;   Perception + 1");

    case DI_HIGH:
        return _("Intelligence - 1;   Perception - 1");

    case DI_VISUALS: return _("You can't trust everything that you see.");

    case DI_ADRENALINE:
        if (dis.duration > 150)
            return _(
            "Speed +80;   Strength + 5;   Dexterity + 3;\n"
            "Intelligence - 8;   Perception + 1");
        else
            return _(
            "Strength - 2;   Dexterity - 1;   Intelligence - 1;   Perception - 1");

    case DI_JETINJECTOR:
        if (dis.duration > 50)
            return _(
            "Strength + 1;   Dexterity + 1; Perception + 1");
        else
            return _(
            "Strength - 1;   Dexterity - 2;   Intelligence - 1;   Perception - 2");

    case DI_ASTHMA:
        return string_format(_("Speed - %d%%;   Strength - 2;   Dexterity - 3"), int(dis.duration / 5));

    case DI_GRACK: return _("Unleashed the Gracken.");

    case DI_METH:
        if (dis.duration > 200)
            return _(
            "Speed +50;   Strength + 2;   Dexterity + 2;\n"
            "Intelligence + 3;   Perception + 3");
        else
            return _(
            "Speed -40;   Strength - 3;   Dexterity - 2;   Intelligence - 2");

    case DI_IN_PIT: return _("You're stuck in a pit.  Sight distance is limited and you have to climb out.");

    case DI_BITE: return _("You have a nasty bite wound.");
    case DI_INFECTED: return _("You have an infected wound.");
    case DI_RECOVER: return _("You are recovering from an infection.");

    case DI_MA_BUFF:
        if (ma_buffs.find(dis.buff_id) != ma_buffs.end())
          return ma_buffs[dis.buff_id].description.c_str();
        else
          return "This is probably a bug.";

    case DI_LACKSLEEP: return _("You haven't slept in a while, and it shows. \n\
    You can't move as quickly and your stats just aren't where they should be.");
    case DI_GRABBED: return _("You have been grabbed by an attacker. \n\
    You cannot dodge and blocking is very difficult.");
    default: break;
    }
    return "Who knows?  This is probably a bug. (disease.cpp:dis_description)";
}

void manage_fungal_infection(player& p, disease& dis)
{
    int bonus = p.health + (p.has_trait("POISRESIST") ? 100 : 0);
    p.moves -= 10;
    p.mod_str_bonus(-1);
    p.mod_dex_bonus(-1);
    if (!dis.permanent) {
        if (dis.duration > 3001) { // First hour symptoms
            if (one_in(160 + bonus)) {
                handle_cough(p, 5, true);
            }
            if (one_in(100 + bonus)) {
                p.add_msg_if_player(m_warning, _("You feel nauseous."));
            }
            if (one_in(100 + bonus)) {
                p.add_msg_if_player(m_warning, _("You smell and taste mushrooms."));
            }
        } else if (dis.duration > 1) { // Five hours of worse symptoms
            if (one_in(600 + bonus * 3)) {
                p.add_msg_if_player(m_bad,  _("You spasm suddenly!"));
                p.moves -= 100;
                p.hurt(bp_torso, -1, 5);
            }
            if (will_vomit(p, 800 + bonus * 4) || one_in(2000 + bonus * 10)) {
                p.add_msg_player_or_npc(m_bad, _("You vomit a thick, gray goop."),
                                               _("<npcname> vomits a thick, grey goop.") );

                int awfulness = rng(0,70);
                p.moves = -200;
                p.hunger += awfulness;
                p.thirst += awfulness;
                p.hurt(bp_torso, -1, awfulness / std::max(p.str_cur, 1)); // can't be healthy
            }
        } else {
            p.add_disease("fungus", 1, true, 1, 1, 0, -1);
        }
    } else if (one_in(1000 + bonus * 8)) {
        p.add_msg_player_or_npc(m_bad,  _("You vomit thousands of live spores!"),
                                        _("<npcname> vomits thousands of live spores!") );

        p.moves = -500;
        int sporex, sporey;
        monster spore(GetMType("mon_spore"));
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) {
                    continue;
                }
                sporex = p.posx + i;
                sporey = p.posy + j;
                if (g->m.move_cost(sporex, sporey) > 0) {
                    const int zid = g->mon_at(sporex, sporey);
                    if (zid >= 0) {  // Spores hit a monster
                        if (g->u_see(sporex, sporey) &&
                              !g->zombie(zid).type->in_species("FUNGUS")) {
                            add_msg(_("The %s is covered in tiny spores!"),
                                       g->zombie(zid).name().c_str());
                        }
                        monster &critter = g->zombie( zid );
                        if( !critter.make_fungus() ) {
                            critter.die( &p ); // counts as kill by player
                        }
                    } else if (one_in(4) && g->num_zombies() <= 1000){
                        spore.spawn(sporex, sporey);
                        g->add_zombie(spore);
                    }
                }
            }
        }
    // we're fucked
    } else if (one_in(6000 + bonus * 20)) {
        if(p.hp_cur[hp_arm_l] <= 0 || p.hp_cur[hp_arm_r] <= 0) {
            if(p.hp_cur[hp_arm_l] <= 0 && p.hp_cur[hp_arm_r] <= 0) {
                p.add_msg_player_or_npc(m_bad, _("The flesh on your broken arms bulges. Fungus stalks burst through!"),
                _("<npcname>'s broken arms bulge. Fungus stalks burst out of the bulges!"));
            } else {
                p.add_msg_player_or_npc(m_bad, _("The flesh on your broken arm bulges, your unbroken arm also bulges. Fungus stalks burst through!"),
                _("<npcname>'s arms bulge. Fungus stalks burst out of the bulges!"));
            }
        } else {
            p.add_msg_player_or_npc(m_bad, _("Your hands bulge. Fungus stalks burst through the bulge!"),
                _("<npcname>'s hands bulge. Fungus stalks burst through the bulge!"));
        }
        p.hurt(bp_arms, 0, 999);
        p.hurt(bp_arms, 1, 999);
    }
}

void manage_sleep(player& p, disease& dis)
{
    p.moves = 0;
    // Hibernating only kicks in whilst Engorged; separate tracking for hunger/thirst here
    // as a safety catch.  One test subject managed to get two Colds during hibernation;
    // since those add fatigue and dry out the character, the subject went for the full 10 days plus
    // a little, and came out of it well into Parched.  Hibernating shouldn't endanger your
    // life like that--but since there's much less fluid reserve than food reserve,
    // simply using the same numbers won't work.
    if((int(calendar::turn) % 350 == 0) && p.has_trait("HIBERNATE") && (p.hunger < -60) && !(p.thirst >= 80)) {
        int recovery_chance;
        // Hibernators' metabolism slows down: you heal and recover Fatigue much more slowly.
        // Accelerated recovery capped to 2x over 2 hours...well, it was ;-P
        // After 16 hours of activity, equal to 7.25 hours of rest
        if (dis.intensity < 24) {
            dis.intensity++;
        } else if (dis.intensity < 1) {
            dis.intensity = 1;
        }
        recovery_chance = 24 - dis.intensity + 1;
        if (p.fatigue > 0) {
            p.fatigue -= 1 + one_in(recovery_chance);
        }
        if ((p.has_trait("FLIMSY") && x_in_y(3 , 4)) || (p.has_trait("FLIMSY2") && one_in(2)) ||
              (p.has_trait("FLIMSY3") && one_in(4)) ||
              (!(p.has_trait("FLIMSY")) && (!(p.has_trait("FLIMSY2"))) &&
               (!(p.has_trait("FLIMSY3"))))) {
            if (p.has_trait("FASTHEALER")) {
                p.healall(1);
            } else if (p.has_trait("FASTHEALER2")) {
                p.healall(1 + one_in(2));
            } else if (p.has_trait("REGEN")) {
                p.healall(2);
            } else if (p.has_trait("SLOWHEALER")) {
                p.healall(one_in(8));
            } else {
                p.healall(one_in(4));
            }
        }

        if (p.fatigue <= 0 && p.fatigue > -20) {
            p.fatigue = -25;
            add_msg(m_good, _("You feel well rested."));
            dis.duration = dice(3, 100);
            p.add_memorial_log(pgettext("memorial_male", "Awoke from hibernation."),
                               pgettext("memorial_female", "Awoke from hibernation."));
        }
    }

    // If you hit Very Thirsty, you kick up into regular Sleep as a safety precaution.
    // See above.  No log note for you. :-/
    if((int(calendar::turn) % 50 == 0) && (!(p.hunger < -60) || (p.thirst >= 80))) {
        int recovery_chance;
        // Accelerated recovery capped to 2x over 2 hours
        // After 16 hours of activity, equal to 7.25 hours of rest
        if (dis.intensity < 24) {
            dis.intensity++;
        } else if (dis.intensity < 1) {
            dis.intensity = 1;
        }
        recovery_chance = 24 - dis.intensity + 1;
        if (p.fatigue > 0) {
            p.fatigue -= 1 + one_in(recovery_chance);
            // You fatigue & recover faster with Sleepy
            // Very Sleepy, you just fatigue faster
            if (p.has_trait("SLEEPY")) {
                p.fatigue -=(1 + one_in(recovery_chance) / 2);
            }
            // Tireless folks recover fatigue really fast
            // as well as gaining it really slowly
            // (Doesn't speed healing any, though...)
            if (p.has_trait("WAKEFUL3")) {
                p.fatigue -=(2 + one_in(recovery_chance) / 2);
            }
        }
        if ((p.has_trait("FLIMSY") && x_in_y(3 , 4)) || (p.has_trait("FLIMSY2") && one_in(2)) ||
              (p.has_trait("FLIMSY3") && one_in(4)) ||
              (!(p.has_trait("FLIMSY")) && (!(p.has_trait("FLIMSY2"))) &&
               (!(p.has_trait("FLIMSY3"))))) {
            if (p.has_trait("FASTHEALER")) {
                p.healall(1);
            } else if (p.has_trait("FASTHEALER2")) {
                p.healall(1 + one_in(2));
            } else if (p.has_trait("REGEN")) {
                p.healall(2);
            } else if (p.has_trait("SLOWHEALER")) {
                p.healall(one_in(8));
            } else {
                p.healall(one_in(4));
            }

            if (p.fatigue <= 0 && p.fatigue > -20) {
                p.fatigue = -25;
                add_msg(m_good, _("You feel well rested."));
                dis.duration = dice(3, 100);
            }
        }
    }

    if (int(calendar::turn) % 100 == 0 && !p.has_bionic("bio_recycler") && !(p.hunger < -60)) {
        // Hunger and thirst advance more slowly while we sleep. This is the standard rate.
        p.hunger--;
        p.thirst--;
    }

    // Hunger and thirst advance *much* more slowly whilst we hibernate.
    // (int (calendar::turn) % 50 would be zero burn.)
    // Very Thirsty catch deliberately NOT applied here, to fend off Dehydration debuffs
    // until the char wakes.  This was time-trial'd quite thoroughly,so kindly don't "rebalance"
    // without a good explanation and taking a night to make sure it works
    // with the extended sleep duration, OK?
    if (int(calendar::turn) % 70 == 0 && !p.has_bionic("bio_recycler") && (p.hunger < -60)) {
        p.hunger--;
        p.thirst--;
    }

    if (int(calendar::turn) % 100 == 0 && p.has_trait("CHLOROMORPH") &&
    g->is_in_sunlight(g->u.posx, g->u.posy) ) {
        // Hunger and thirst fall before your Chloromorphic physiology!
        if (p.hunger >= -30) {
            p.hunger -= 5;
        }
        if (p.thirst >= -30) {
            p.thirst -= 5;
        }
    }

    // Check mutation category strengths to see if we're mutated enough to get a dream
    std::string highcat = p.get_highest_category();
    int highest = p.mutation_category_level[highcat];

    // Determine the strength of effects or dreams based upon category strength
    int strength = 0; // Category too weak for any effect or dream
    if (g->u.crossed_threshold()) {
        strength = 4; // Post-human.
    } else if (highest >= 20 && highest < 35) {
        strength = 1; // Low strength
    } else if (highest >= 35 && highest < 50) {
        strength = 2; // Medium strength
    } else if (highest >= 50) {
        strength = 3; // High strength
    }

    // Get a dream if category strength is high enough.
    if (strength != 0) {
        //Once every 6 / 3 / 2 hours, with a bit of randomness
        if ((int(calendar::turn) % (3600 / strength) == 0) && one_in(3)) {
            // Select a dream
            std::string dream = p.get_category_dream(highcat, strength);
            add_msg("%s",dream.c_str());
        }
    }

    int tirednessVal = rng(5, 200) + rng(0,abs(p.fatigue * 2 * 5));
    if (p.has_trait("HEAVYSLEEPER2") && !p.has_trait("HIBERNATE")) {
        // So you can too sleep through noon
        if ((tirednessVal * 1.25) < g->light_level() && (p.fatigue < 10 || one_in(p.fatigue / 2))) {
        add_msg(_("The light wakes you up."));
        dis.duration = 1;
        }
        return;}
     // Ursine hibernators would likely do so indoors.  Plants, though, might be in the sun.
    if (p.has_trait("HIBERNATE")) {
        if ((tirednessVal * 5) < g->light_level() && (p.fatigue < 10 || one_in(p.fatigue / 2))) {
        add_msg(_("The light wakes you up."));
        dis.duration = 1;
        }
        return;}
    if (tirednessVal < g->light_level() && (p.fatigue < 10 || one_in(p.fatigue / 2))) {
        add_msg(_("The light wakes you up."));
        dis.duration = 1;
        return;
    }

    // Cold or heat may wake you up.
    // Player will sleep through cold or heat if fatigued enough
    for (int i = 0 ; i < num_bp ; i++) {
        if (p.temp_cur[i] < BODYTEMP_VERY_COLD - p.fatigue/2) {
            if (one_in(5000)) {
                add_msg(_("You toss and turn trying to keep warm."));
            }
            if (p.temp_cur[i] < BODYTEMP_FREEZING - p.fatigue/2 ||
                                (one_in(p.temp_cur[i] + 5000))) {
                add_msg(m_bad, _("The cold wakes you up."));
                dis.duration = 1;
                return;
            }
        } else if (p.temp_cur[i] > BODYTEMP_VERY_HOT + p.fatigue/2) {
            if (one_in(5000)) {
                add_msg(_("You toss and turn in the heat."));
            }
            if (p.temp_cur[i] > BODYTEMP_SCORCHING + p.fatigue/2 ||
                                (one_in(15000 - p.temp_cur[i]))) {
                add_msg(m_bad, _("The heat wakes you up."));
                dis.duration = 1;
                return;
            }
        }
    }
}

static void handle_alcohol(player& p, disease& dis)
{
    /*  We get 600 turns, or one hour, of DI_DRUNK for each drink we have (on avg).
        Duration of DI_DRUNK is a good indicator of how much alcohol is in our system.
    */
    p.mod_per_bonus( - int(dis.duration / 1000));
    p.mod_dex_bonus( - int(dis.duration / 1000));
    p.mod_int_bonus( - int(dis.duration /  700));
    p.mod_str_bonus( - int(dis.duration / 1500));
    if (dis.duration <= 600) {
        p.mod_str_bonus(+1);
    }
    if (dis.duration > 2000 + 100 * dice(2, 100) &&
        (will_vomit(p, 1) || one_in(20))) {
        p.vomit();
    }
    bool readyForNap = one_in(500 - int(dis.duration / 80));
    if (!p.has_disease("sleep") && dis.duration >= 4500 && readyForNap) {
        p.add_msg_if_player(m_bad, _("You pass out."));
        p.fall_asleep(dis.duration / 2);
    }
}

static void handle_bite_wound(player& p, disease& dis)
{
    // Recovery chance
    if(int(calendar::turn) % 10 == 1) {
        int recover_factor = 100;
        if (p.has_disease("recover")) {
            recover_factor -= std::min(p.disease_duration("recover") / 720, 100);
        }
        // Infection Resist is exactly that: doesn't make the Deep Bites go away
        // but it does make it much more likely they won't progress
        if (p.has_trait("INFRESIST")) { recover_factor += 1000; }
        recover_factor += p.health; // Health still helps if factor is zero
        recover_factor = std::max(recover_factor, 0); // but can't hurt

        if ((x_in_y(recover_factor, 108000)) || (p.has_trait("INFIMMUNE"))) {
            //~ %s is bodypart name.
            p.add_msg_if_player(m_good, _("Your %s wound begins to feel better."),
                                body_part_name(dis.bp, dis.side).c_str());
             //No recovery time threshold
            if (((3601 - dis.duration) > 2400) && (!(p.has_trait("INFIMMUNE")))) {
                p.add_disease("recover", 2 * (3601 - dis.duration) - 4800);
            }
            p.rem_disease("bite", dis.bp, dis.side);
        }
    }

    // 3600 (6-hour) lifespan + 1 "tick" for conversion
    if (dis.duration > 2401) {
        // No real symptoms for 2 hours
        if ((one_in(300)) && (!(p.has_trait("NOPAIN")))) {
            //~ %s is bodypart name.
            p.add_msg_if_player(m_bad, _("Your %s wound really hurts."),
                                 body_part_name(dis.bp, dis.side).c_str());
        }
    } else if (dis.duration > 1) {
        // Then some pain for 4 hours
        if ((one_in(100)) && (!(p.has_trait("NOPAIN")))) {
            if (p.has_disease("sleep")) {
                p.wake_up();
            }
            //~ %s is bodypart name.
            p.add_msg_if_player(m_bad, _("Your %s wound feels swollen and painful."),
                                 body_part_name(dis.bp, dis.side).c_str());
            if (p.pain < 10) {
                p.mod_pain(1);
            }
        }
        p.mod_dex_bonus(-1);
    } else {
        // Infection starts
         // 1 day of timer + 1 tick
        p.add_disease("infected", 14401, false, 1, 1, 0, 0, dis.bp, dis.side, true);
        p.rem_disease("bite", dis.bp, dis.side);
    }
}

static void handle_infected_wound(player& p, disease& dis)
{
    // Recovery chance
    if(int(calendar::turn) % 10 == 1) {
        if(x_in_y(100 + p.health, 864000)) {
            //~ %s is bodypart name.
            p.add_msg_if_player(m_good, _("Your %s wound begins to feel better."),
                                 body_part_name(dis.bp, dis.side).c_str());
            if (dis.duration > 8401) {
                p.add_disease("recover", 3 * (14401 - dis.duration + 3600) - 4800);
            } else {
                p.add_disease("recover", 4 * (14401 - dis.duration + 3600) - 4800);
            }
            p.rem_disease("infected", dis.bp, dis.side);
        }
    }

    if (dis.duration > 8401) {
        // 10 hours bad pain
        if ((one_in(100)) && (!(p.has_trait("NOPAIN")))) {
            if (p.has_disease("sleep")) {
                p.wake_up();
            }
            //~ %s is bodypart name.
            p.add_msg_if_player(m_bad, _("Your %s wound is incredibly painful."),
                                 body_part_name(dis.bp, dis.side).c_str());
            if(p.pain < 30) {
                p.mod_pain(1);
            }
        }
        p.mod_str_bonus(-1);
        p.mod_dex_bonus(-1);
    } else if (dis.duration > 3601) {
        // 8 hours of vomiting + pain
        if (one_in(100)) {
            if (p.has_disease("sleep")) {
                p.wake_up();
            }
            //~ %s is bodypart name.
            p.add_msg_if_player(m_bad, _("You feel feverish and nauseous, your %s wound has begun to turn green."),
                  body_part_name(dis.bp, dis.side).c_str());
            p.vomit();
            if(p.pain < 50) {
                p.mod_pain(1);
            }
        }
        p.mod_str_bonus(-2);
        p.mod_dex_bonus(-2);
    } else if (dis.duration > 1) {
        // 6 hours extreme symptoms
        if (one_in(100)) {
            if (p.has_disease("sleep")) {
                p.wake_up();
                p.add_msg_if_player(m_warning, _("You feel terribly weak, standing up is nearly impossible."));
            } else {
                p.add_msg_if_player(m_warning, _("You can barely remain standing."));
            }
            p.vomit();
            if(p.pain < 100) {
                p.mod_pain(1);
            }
        }
        p.mod_str_bonus(-3);
        p.mod_dex_bonus(-3);
        if (!p.has_disease("sleep") && one_in(100)) {
            add_msg(m_bad, _("You pass out."));
            p.fall_asleep(60);
        }
    } else {
        // Death. 24 hours after infection. Total time, 30 hours including bite.
        if (p.has_disease("sleep")) {
            p.rem_disease("sleep");
        }
        add_msg(m_bad, _("You succumb to the infection."));
        g->u.add_memorial_log(pgettext("memorial_male", "Succumbed to the infection."),
                              pgettext("memorial_female", "Succumbed to the infection."));
        p.hurtall(500);
    }
}

static void handle_recovery(player& p, disease& dis)
{
    if (dis.duration > 52800) {
        if (one_in(100)) {
            if (p.has_disease("sleep")) {
                p.wake_up();
                p.add_msg_if_player(m_warning, _("You feel terribly weak, standing up is nearly impossible."));
            } else {
                p.add_msg_if_player(m_warning, _("You can barely remain standing."));
            }
            p.vomit();
            if(p.pain < 80) {
                p.mod_pain(1);
            }
        }
        p.mod_str_bonus(-3);
        p.mod_dex_bonus(-3);
        if (!p.has_disease("sleep") && one_in(100)) {
            add_msg(m_bad, _("You pass out."));
            p.fall_asleep(60);
        }
    } else if (dis.duration > 33600) {
        if (one_in(100)) {
            if (p.has_disease("sleep")) {
                p.wake_up();
            }
            p.add_msg_if_player(m_bad, _("You feel feverish and nauseous."));
            p.vomit();
            if(p.pain < 40) {
                p.mod_pain(1);
            }
        }
        p.mod_str_bonus(-2);
        p.mod_dex_bonus(-2);
    } else if (dis.duration > 9600) {
        if ((one_in(100)) && (!(p.has_trait("NOPAIN")))) {
            if (p.has_disease("sleep")) {
                p.wake_up();
            }
            p.add_msg_if_player(m_bad, _("Your healing wound is incredibly painful."));
            if(p.pain < 24) {
                p.mod_pain(1);
            }
        }
        p.mod_str_bonus(-1);
        p.mod_dex_bonus(-1);
    } else {
        if ((one_in(100)) && (!(p.has_trait("NOPAIN")))) {
            if (p.has_disease("sleep")) {
                p.wake_up();
            }
            p.add_msg_if_player(m_bad, _("Your healing wound feels swollen and painful."));
            if(p.pain < 8) {
                p.mod_pain(1);
            }
        }
        p.mod_dex_bonus(-1);
    }
}

static void handle_cough(player &p, int, int loudness, bool harmful)
{
    if (!p.is_npc()) {
        add_msg(m_bad, _("You cough heavily."));
        g->sound(p.posx, p.posy, loudness, "");
    } else {
        g->sound(p.posx, p.posy, loudness, _("a hacking cough."));
    }
    p.moves -= 80;
    if (harmful && !one_in(4)) {
        p.hurt(bp_torso, -1, 1);
    }
    if (p.has_disease("sleep") && ((harmful && one_in(3)) || one_in(10)) ) {
        p.wake_up(_("You wake up coughing."));
    }
}

static void handle_deliriant(player& p, disease& dis)
{
    // To be redone.
    // Time intervals are drawn from the old ones based on 3600 (6-hour) duration.
    static bool puked = false;
    int maxDuration = 3600;
    int comeupTime = int(maxDuration*0.9);
    int noticeTime = int(comeupTime + (maxDuration-comeupTime)/2);
    int peakTime = int(maxDuration*0.8);
    int comedownTime = int(maxDuration*0.3);
    // Baseline
    if (dis.duration == noticeTime) {
        p.add_msg_if_player(m_warning, _("You feel a little strange."));
    } else if (dis.duration == comeupTime) {
        // Coming up
        if (one_in(2)) {
            p.add_msg_if_player(m_warning, _("The world takes on a dreamlike quality."));
        } else if (one_in(3)) {
            p.add_msg_if_player(m_warning, _("You have a sudden nostalgic feeling."));
        } else if (one_in(5)) {
            p.add_msg_if_player(m_warning, _("Everything around you is starting to breathe."));
        } else {
            p.add_msg_if_player(m_warning, _("Something feels very, very wrong."));
        }
    } else if (dis.duration > peakTime && dis.duration < comeupTime) {
        if ((one_in(200) || will_vomit(p, 50)) && !puked) {
            p.add_msg_if_player(m_bad, _("You feel sick to your stomach."));
            p.hunger -= 2;
            if (one_in(6)) {
                p.vomit();
                if (one_in(2)) {
                    // we've vomited enough for now
                    puked = true;
                }
            }
        }
        if (p.is_npc() && one_in(200)) {
            std::string npcText;
            switch(rng(1,4)) {
                case 1:
                    npcText = "\"I think it's starting to kick in.\"";
                    break;
                case 2:
                    npcText = "\"Oh God, what's happening?\"";
                    break;
                case 3:
                    npcText = "\"Of course... it's all fractals!\"";
                    break;
                default:
                    npcText = "\"Huh?  What was that?\"";
                    break;

            }
            int loudness = 20 + p.str_cur - p.int_cur;
            loudness = (loudness > 5 ? loudness : 5);
            loudness = (loudness < 30 ? loudness : 30);
            g->sound(p.posx, p.posy, loudness, _(npcText.c_str()));
        }
    } else if (dis.duration == peakTime) {
        // Visuals start
        p.add_msg_if_player(m_bad, _("Fractal patterns dance across your vision."));
        p.add_disease("visuals", peakTime - comedownTime);
    } else if (dis.duration > comedownTime && dis.duration < peakTime) {
        // Full symptoms
        p.mod_per_bonus(-2);
        p.mod_int_bonus(-1);
        p.mod_dex_bonus(-2);
        p.mod_str_bonus(-1);
        if (one_in(50)) {
            g->spawn_hallucination();
        }
    } else if (dis.duration == comedownTime) {
        if (one_in(42)) {
            p.add_msg_if_player(_("Everything looks SO boring now."));
        } else {
            p.add_msg_if_player(_("Things are returning to normal."));
        }
        puked = false;
    }
}

static void handle_evil(player& p, disease& dis)
{
    bool lesserEvil = false;  // Worn or wielded; diminished effects
    if (p.weapon.is_artifact() && p.weapon.is_tool()) {
        it_artifact_tool *tool = dynamic_cast<it_artifact_tool*>(p.weapon.type);
        for (std::vector<art_effect_passive>::iterator it =
                 tool->effects_carried.begin();
             it != tool->effects_carried.end(); ++it) {
            if (*it == AEP_EVIL) {
                lesserEvil = true;
            }
        }
        for (std::vector<art_effect_passive>::iterator it =
                 tool->effects_wielded.begin();
             it != tool->effects_wielded.end(); ++it) {
            if (*it == AEP_EVIL) {
                lesserEvil = true;
            }
        }
    }
    for (std::vector<item>::iterator it = p.worn.begin();
         !lesserEvil && it != p.worn.end(); ++it) {
        if (it->is_artifact()) {
            it_artifact_armor *armor = dynamic_cast<it_artifact_armor*>(it->type);
            for (std::vector<art_effect_passive>::iterator effect =
                     armor->effects_worn.begin();
                 effect != armor->effects_worn.end(); ++effect) {
                if (*effect == AEP_EVIL) {
                    lesserEvil = true;
                }
            }
        }
    }
    if (lesserEvil) {
        // Only minor effects, some even good!
        p.mod_str_bonus(dis.duration > 4500 ? 10 : int(dis.duration / 450));
        if (dis.duration < 600) {
            p.mod_dex_bonus(1);
        } else {
            p.mod_dex_bonus(-(dis.duration > 3600 ? 10 : int((dis.duration - 600) / 300)));
        }
        p.mod_int_bonus(-(dis.duration > 3000 ? 10 : int((dis.duration - 500) / 250)));
        p.mod_per_bonus(-(dis.duration > 4800 ? 10 : int((dis.duration - 800) / 400)));
    } else {
        // Major effects, all bad.
        p.mod_str_bonus(-(dis.duration > 5000 ? 10 : int(dis.duration / 500)));
        p.mod_dex_bonus(-(dis.duration > 6000 ? 10 : int(dis.duration / 600)));
        p.mod_int_bonus(-(dis.duration > 4500 ? 10 : int(dis.duration / 450)));
        p.mod_per_bonus(-(dis.duration > 4000 ? 10 : int(dis.duration / 400)));
    }
}

static void handle_insect_parasites(player& p, disease& dis)
{
    int formication_chance = 600;
    if (dis.duration > 12001) {
        formication_chance += 2400 - (14401 - dis.duration);
    }
    if (one_in(formication_chance)) {
        p.add_disease("formication", 600, false, 1, 3, 0, 1, dis.bp, dis.side, true);
    }
    if (dis.duration > 1 && one_in(2400)) {
        p.vomit();
    }
    if (dis.duration == 1) {
        // Spawn some larvae!
        // Choose how many insects; more for large characters
        int num_insects = rng(1, std::min(3, p.str_max / 3));
        p.hurt(dis.bp, dis.side, rng(2, 4) * num_insects);
        // Figure out where they may be placed
        p.add_msg_player_or_npc( m_bad, _("Your flesh crawls; insects tear through the flesh and begin to emerge!"),
            _("Insects begin to emerge from <npcname>'s skin!") );
        monster grub(GetMType("mon_dermatik_larva"));
        for (int i = p.posx - 1; i <= p.posx + 1; i++) {
            for (int j = p.posy - 1; j <= p.posy + 1; j++) {
                if (num_insects == 0) {
                    break;
                } else if (i == 0 && j == 0) {
                    continue;
                }
                if (g->mon_at(i, j) == -1) {
                    grub.spawn(i, j);
                    if (one_in(3)) {
                        grub.friendly = -1;
                    } else {
                        grub.friendly = 0;
                    }
                    g->add_zombie(grub);
                    num_insects--;
                }
            }
            if (num_insects == 0) {
                break;
            }
        }
        p.add_memorial_log(pgettext("memorial_male", "Dermatik eggs hatched."),
                           pgettext("memorial_female", "Dermatik eggs hatched."));
        p.rem_disease("formication", dis.bp, dis.side);
        p.moves -= 600;
    }
}

bool will_vomit(player& p, int chance)
{
    bool drunk = p.has_disease("drunk");
    bool antiEmetics = p.has_disease("weed_high");
    bool hasNausea = p.has_trait("NAUSEA") && one_in(chance*2);
    bool stomachUpset = p.has_trait("WEAKSTOMACH") && one_in(chance*3);
    bool suppressed = (p.has_trait("STRONGSTOMACH") && one_in(2)) ||
        (antiEmetics && !drunk && !one_in(chance));
    return ((stomachUpset || hasNausea) && !suppressed);
}
