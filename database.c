#include "misc.h"
#include "sio.h"
#include "Gamecube/wiiSXconfig.h"

/* It's duplicated from emu_if.c */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/* Corresponds to LIGHTREC_OPT_INV_DMA_ONLY of lightrec.h */
#define LIGHTREC_HACK_INV_DMA_ONLY (1 << 0)

u32 lightrec_hacks;

static const char * const MemorycardHack_db[] =
{
    /* Lifeforce Tenka, also known as Codename Tenka */
    "SLES00613", "SLED00690", "SLES00614", "SLES00615",
    "SLES00616", "SLES00617", "SCUS94409"
};

static const char * const cdr_read_hack_db[] =
{
    /* T'ai Fu - Wrath of the Tiger */
    "SLUS00787",
};

static const char * const gpu_slow_llist_db[] =
{
    /* Bomberman Fantasy Race */
    "SLES01712", "SLPS01525", "SLPS91138", "SLPM87102",
    "SLUS00823",
    /* Crash Bash */
    "SCES02834", "SCUS94570", "SCUS94616", "SCUS94654",
    /* Final Fantasy IV */
    "SCES03840", "SLPM86028", "SLUS01360",
    /* Spot Goes to Hollywood */
    "SLES00330", "SLPS00394", "SLUS00014",
    /* Vampire Hunter D */
    "SLES02731", "SLPS02477", "SLPS03198", "SLUS01138",
};

#define HACK_ENTRY(var, list) \
    { #var, &Config.hacks.var, list, ARRAY_SIZE(list) }

static const struct
{
    const char *name;
    bool *var;
    const char * const * id_list;
    size_t id_list_len;
}
hack_db[] =
{
    HACK_ENTRY(cdr_read_timing, cdr_read_hack_db),
    HACK_ENTRY(gpu_slow_list_walking, gpu_slow_llist_db),
};

static const struct
{
    const char * const id;
    int mult;
}
cycle_multiplier_overrides[] =
{
    /* note: values are = (10000 / gui_option) */
    /* Internal Section - fussy about timings */
    { "SLPS01868", 202 },
    /* Super Robot Taisen Alpha - on the edge with 175,
     * changing memcard settings is enough to break/unbreak it */
    { "SLPS02528", 190 },
    { "SLPS02636", 190 },
    /* Brave Fencer Musashi - cd sectors arrive too fast */
    { "SLUS00726", 170 },
    { "SLPS01490", 170 },
    /* new_dynarec has a hack for this game */
    /* Parasite Eve II - internal timer checks */
//    { "SLUS01042", 125 },
//    { "SLUS01055", 125 },
//    { "SLES02558", 125 },
//    { "SLES12558", 125 },
    { "SLUS00447", 125 }, // VANDAL HEARTS
    { "SLUS00940", 125 }, // VANDAL HEARTS II

    { "SLUS01042", 125 }, // PARASITE EVE 2(disk1)
    { "SLUS01055", 125 }, // PARASITE EVE 2(disk2 ?)

    { "SLES00204", 125 }, // VANDAL HEARTS
    { "SLES02469", 125 }, // VANDAL HEARTS II
    { "SLES02496", 125 }, // VANDAL HEARTS II
    { "SLES02497", 125 }, // VANDAL HEARTS II

    { "SLES02558", 125 }, // PARASITE EVE 2(disk1)
    { "SLES12558", 125 }, // PARASITE EVE 2(disk2 ?)
    { "SLES02559", 125 }, // PARASITE EVE 2(disk1)
    { "SLES12559", 125 }, // PARASITE EVE 2(disk2 ?)
    { "SLES02560", 125 }, // PARASITE EVE 2(disk1)
    { "SLES12560", 125 }, // PARASITE EVE 2(disk2 ?)
    { "SLES02561", 125 }, // PARASITE EVE 2(disk1)
    { "SLES12561", 125 }, // PARASITE EVE 2(disk2 ?)
    { "SLES02562", 125 }, // PARASITE EVE 2(disk1)
    { "SLES12562", 125 }, // PARASITE EVE 2(disk2 ?)

    { "SCPS45183", 125 }, // VANDAL HEARTS - USHINAWARETA KODAI BUNMEI
    { "SLPM86007", 125 }, // VANDAL HEARTS - USHINAWARETA KODAI BUNMEI
    { "SLPM86067", 125 }, // VANDAL HEARTS - USHINAWARETA KODAI BUNMEI [PLAYSTATION THE BEST]
    { "SLPM87278", 125 }, // VANDAL HEARTS - USHINAWARETA KODAI BUNMEI [PSONE BOOKS]
    { "SCPS45415", 125 }, // VANDAL HEARTS II - TENJOU NO MON
    { "SLPM86251", 125 }, // VANDAL HEARTS II - TENJOU NO MON
    { "SLPM86504", 125 }, // VANDAL HEARTS II - TENJOU NO MON [KONAMI THE BEST]
    { "SLPM87279", 125 }, // VANDAL HEARTS II - TENJOU NO MON [PSONE BOOKS]

    { "SCPS45467", 125 }, // PARASITE EVE II [ 2 DISCS ]
    { "SCPS45468", 125 }, // PARASITE EVE II [ 2 DISCS ]
    { "SLPS02480", 125 }, // PARASITE EVE II [ 2 DISCS ]
    { "SLPS02481", 125 }, // PARASITE EVE II [ 2 DISCS ]
    { "SLPS91479", 125 }, // PARASITE EVE II [PSONE BOOKS] [ 2 DISCS ]
    { "SLPS91480", 125 }, // PARASITE EVE II [PSONE BOOKS] [ 2 DISCS ]
    { "SLPS02779", 125 }, // PARASITE EVE II [SQUARESOFT MILLENNIUM COLLECTION]  -  [ 2 DISCS ]
    { "SLPS02780", 125 }, // PARASITE EVE II [SQUARESOFT MILLENNIUM COLLECTION]  -  [ 2 DISCS ]

    /* Discworld Noir - audio skips if CPU runs too fast */
    { "SLES01549", 222 },
    { "SLES02063", 222 },
    { "SLES02064", 222 },
    /* Judge Dredd - could also be poor MDEC timing */
    { "SLUS00630", 128 },
    { "SLES00755", 128 },
    /* Digimon World */
    { "SLUS01032", 153 },
    { "SLES02914", 153 },
};

static const struct
{
    const char * const id;
    u32 hacks;
}
lightrec_hacks_db[] =
{
    /* Formula One Arcade */
    { "SCES03886", LIGHTREC_HACK_INV_DMA_ONLY },

    /* Formula One '99 */
    { "SLUS00870", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCPS10101", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCES01979", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SLES01979", LIGHTREC_HACK_INV_DMA_ONLY },

    /* Formula One 2000 */
    { "SLUS01134", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCES02777", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCES02778", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCES02779", LIGHTREC_HACK_INV_DMA_ONLY },

    /* Formula One 2001 */
    { "SCES03404", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCES03423", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCES03424", LIGHTREC_HACK_INV_DMA_ONLY },
    { "SCES03524", LIGHTREC_HACK_INV_DMA_ONLY },
};

/* Function for automatic patching according to GameID. */
void Apply_Hacks_Cdrom()
{
    size_t i, j;

    memset(&Config.hacks, 0, sizeof(Config.hacks));

    for (i = 0; i < ARRAY_SIZE(hack_db); i++)
    {
        for (j = 0; j < hack_db[i].id_list_len; j++)
        {
            if (strncmp(CdromId, hack_db[i].id_list[j], 9))
                continue;
            *hack_db[i].var = 1;
            SysPrintf("using hack: %s\n", hack_db[i].name);
            break;
        }
    }

    /* Apply Memory card hack for Codename Tenka. (The game needs one of the memory card slots to be empty) */
    for (i = 0; i < ARRAY_SIZE(MemorycardHack_db); i++)
    {
        if (strncmp(CdromId, MemorycardHack_db[i], 9) == 0)
        {
            /* Disable the second memory card slot for the game */
            Config.Mcd2[0] = 0;
            /* This also needs to be done because in sio.c, they don't use Config.Mcd2 for that purpose */
            McdDisable[1] = 1;
            break;
        }
    }

    /* Dynarec game-specific hacks */
    //new_dynarec_hacks_pergame = 0;
    Config.cycle_multiplier_override = 0;

    for (i = 0; i < ARRAY_SIZE(cycle_multiplier_overrides); i++)
    {
        if (strcmp(CdromId, cycle_multiplier_overrides[i].id) == 0)
        {
            Config.cycle_multiplier_override = cycle_multiplier_overrides[i].mult;
            //new_dynarec_hacks_pergame |= NDHACK_OVERRIDE_CYCLE_M;
            SysPrintf("using cycle_multiplier_override: %d\n",
                Config.cycle_multiplier_override);
            break;
        }
    }

    lightrec_hacks = 0;

    for (i = 0; (Config.Cpu == DYNACORE_DYNAREC) && i < ARRAY_SIZE(lightrec_hacks_db); i++) {
        if (strcmp(CdromId, lightrec_hacks_db[i].id) == 0)
        {
            lightrec_hacks = lightrec_hacks_db[i].hacks;
            SysPrintf("using lightrec_hacks: 0x%x\n", lightrec_hacks);
            break;
        }
    }
}
