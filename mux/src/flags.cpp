// flags.cpp -- Flag manipulation routines.
//
// $Id: flags.cpp,v 1.5 2003/02/03 20:24:45 sdennis Exp $
//

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "externs.h"

#include "command.h"
#include "powers.h"

/* ---------------------------------------------------------------------------
 * fh_any: set or clear indicated bit, no security checking
 */

BOOL fh_any(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    // Never let God drop his/her own wizbit.
    //
    if (  God(target)
       && reset
       && flag == WIZARD
       && fflags == FLAG_WORD1)
    {
        notify(player, "You cannot make God mortal.");
        return FALSE;
    }

    // Otherwise we can go do it.
    //
    if (reset)
    {
        db[target].fs.word[fflags] &= ~flag;
    }
    else
    {
        db[target].fs.word[fflags] |= flag;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------------
 * fh_god: only GOD may set or clear the bit
 */

BOOL fh_god(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    if (!God(player))
    {
        return FALSE;
    }
    return (fh_any(target, player, flag, fflags, reset));
}

/*
 * ---------------------------------------------------------------------------
 * * fh_wiz: only WIZARDS (or GOD) may set or clear the bit
 */

BOOL fh_wiz(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    if (!Wizard(player))
    {
        return FALSE;
    }
    return (fh_any(target, player, flag, fflags, reset));
}

/*
 * ---------------------------------------------------------------------------
 * * fh_wizroy: only WIZARDS, ROYALTY, (or GOD) may set or clear the bit
 */

BOOL fh_wizroy(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    if (!WizRoy(player))
    {
        return FALSE;
    }
    return (fh_any(target, player, flag, fflags, reset));
}

/*
 * ---------------------------------------------------------------------------
 * * fh_restrict_player (renamed from fh_fixed): Only Wizards can set
 * * this on players, but ordinary players can set it on other types
 * * of objects.
 */
BOOL fh_restrict_player
(
    dbref target,
    dbref player,
    FLAG flag,
    int fflags,
    BOOL reset
)
{
    if (  isPlayer(target)
       && !Wizard(player))
    {
        return FALSE;
    }
    return fh_any(target, player, flag, fflags, reset);
}

/* ---------------------------------------------------------------------------
 * fh_privileged: You can set this flag on a non-player object, if you
 * yourself have this flag and are a player who owns themselves (i.e.,
 * no robots). Only God can set this on a player.
 */
BOOL fh_privileged
(
    dbref target,
    dbref player,
    FLAG flag,
    int fflags,
    BOOL reset
)
{
    if (!God(player))
    {
        if (  !isPlayer(player)
           || player != Owner(player)
           || isPlayer(target)
           || (db[player].fs.word[fflags] & flag) == 0)
        {
            return FALSE;
        }
    }
    return fh_any(target, player, flag, fflags, reset);
}

/*
 * ---------------------------------------------------------------------------
 * * fh_inherit: only players may set or clear this bit.
 */

BOOL fh_inherit(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    if (!Inherits(player))
    {
        return FALSE;
    }
    return (fh_any(target, player, flag, fflags, reset));
}

/*
 * ---------------------------------------------------------------------------
 * * fh_dark_bit: manipulate the dark bit. Nonwizards may not set on players.
 */

BOOL fh_dark_bit(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    if (  !reset
       && isPlayer(target)
       && !(  (target == player)
           && Can_Hide(player))
       && !Wizard(player))
    {
        return FALSE;
    }
    return fh_any(target, player, flag, fflags, reset);
}

/*
 * ---------------------------------------------------------------------------
 * * fh_going_bit: manipulate the going bit.  Non-gods may only clear.
 */

BOOL fh_going_bit(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    if (  Going(target)
       && reset
       && (Typeof(target) != TYPE_GARBAGE))
    {
        notify(player, "Your object has been spared from destruction.");
        return fh_any(target, player, flag, fflags, reset);
    }
    if (!God(player))
    {
        return FALSE;
    }

    // Even God should not be allowed set protected dbrefs GOING.
    //
    if (  !reset
       && (  target == 0
          || God(target)
          || target == mudconf.start_home
          || target == mudconf.start_room
          || target == mudconf.default_home
          || target == mudconf.master_room))
    {
        return FALSE;
    }
    return fh_any(target, player, flag, fflags, reset);
}

/*
 * ---------------------------------------------------------------------------
 * * fh_hear_bit: set or clear bits that affect hearing.
 */

BOOL fh_hear_bit(dbref target, dbref player, FLAG flag, int fflags, BOOL reset)
{
    if (isPlayer(target) && (flag & MONITOR))
    {
        if (Can_Monitor(player))
        {
            fh_any(target, player, flag, fflags, reset);
        }
        else
        {
            return FALSE;
        }
    }

    BOOL could_hear = Hearer(target);
    fh_any(target, player, flag, fflags, reset);
    handle_ears(target, could_hear, Hearer(target));
    return TRUE;
}

/* ---------------------------------------------------------------------------
 * fh_player_bit: Can set and reset this on everything but players.
 */
BOOL fh_player_bit
(
    dbref target,
    dbref player,
    FLAG flag,
    int fflags,
    BOOL reset
)
{
    if (isPlayer(target))
    {
        return FALSE;
    }
    return fh_any(target, player, flag, fflags, reset);
}

/* ---------------------------------------------------------------------------
 * fh_staff: only STAFF, WIZARDS, ROYALTY, (or GOD) may set or clear
 * the bit.
 */
BOOL fh_staff
(
    dbref target,
    dbref player,
    FLAG flag,
    int fflags,
    BOOL reset
)
{
    if (!Staff(player) && !God(player))
    {
        return FALSE;
    }
    return (fh_any(target, player, flag, fflags, reset));
}

static FLAGBITENT fbeAbode          = { ABODE,        'A',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeAnsi           = { ANSI,         'X',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeAudible        = { HEARTHRU,     'a',    FLAG_WORD1, 0,                    fh_hear_bit};
static FLAGBITENT fbeAuditorium     = { AUDITORIUM,   'b',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeBlind          = { BLIND,        'B',    FLAG_WORD2, 0,                    fh_wiz};
static FLAGBITENT fbeChownOk        = { CHOWN_OK,     'C',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeConnected      = { CONNECTED,    'c',    FLAG_WORD2, CA_NO_DECOMP,         fh_god};
static FLAGBITENT fbeDark           = { DARK,         'D',    FLAG_WORD1, 0,                    fh_dark_bit};
static FLAGBITENT fbeDestroyOk      = { DESTROY_OK,   'd',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeEnterOk        = { ENTER_OK,     'e',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeFixed          = { FIXED,        'f',    FLAG_WORD2, 0,                    fh_restrict_player};
static FLAGBITENT fbeFloating       = { FLOATING,     'F',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeGagged         = { GAGGED,       'j',    FLAG_WORD2, 0,                    fh_wiz};
static FLAGBITENT fbeGoing          = { GOING,        'G',    FLAG_WORD1, CA_NO_DECOMP,         fh_going_bit};
static FLAGBITENT fbeHalted         = { HALT,         'h',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeHasDaily       = { HAS_DAILY,    '*',    FLAG_WORD2, CA_GOD|CA_NO_DECOMP,  fh_god};
static FLAGBITENT fbeHasForwardList = { HAS_FWDLIST,  '&',    FLAG_WORD2, CA_GOD|CA_NO_DECOMP,  fh_god};
static FLAGBITENT fbeHasListen      = { HAS_LISTEN,   '@',    FLAG_WORD2, CA_GOD|CA_NO_DECOMP,  fh_god};
static FLAGBITENT fbeHasStartup     = { HAS_STARTUP,  '+',    FLAG_WORD1, CA_GOD|CA_NO_DECOMP,  fh_god};
static FLAGBITENT fbeHaven          = { HAVEN,        'H',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeHead           = { HEAD_FLAG,    '?',    FLAG_WORD2, 0,                    fh_wiz};
static FLAGBITENT fbeHtml           = { HTML,         '(',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeImmortal       = { IMMORTAL,     'i',    FLAG_WORD1, 0,                    fh_wiz};
static FLAGBITENT fbeInherit        = { INHERIT,      'I',    FLAG_WORD1, 0,                    fh_inherit};
static FLAGBITENT fbeJumpOk         = { JUMP_OK,      'J',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeKeepAlive      = { CKEEPALIVE,   'k',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeKey            = { KEY,          'K',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeLight          = { LIGHT,        'l',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeLinkOk         = { LINK_OK,      'L',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeMonitor        = { MONITOR,      'M',    FLAG_WORD1, 0,                    fh_hear_bit};
static FLAGBITENT fbeMyopic         = { MYOPIC,       'm',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeNoCommand      = { NO_COMMAND,   'n',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeNoAccents      = { NOACCENTS,    '~',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeNoBleed        = { NOBLEED,      '-',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeNoSpoof        = { NOSPOOF,      'N',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeOpaque         = { TM_OPAQUE,    'O',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeOpenOk         = { OPEN_OK,      'z',    FLAG_WORD2, 0,                    fh_wiz};
static FLAGBITENT fbeParentOk       = { PARENT_OK,    'Y',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbePlayerMails    = { PLAYER_MAILS, ' ',    FLAG_WORD2, CA_GOD|CA_NO_DECOMP,  fh_god};
static FLAGBITENT fbePuppet         = { PUPPET,       'p',    FLAG_WORD1, 0,                    fh_hear_bit};
static FLAGBITENT fbeQuiet          = { QUIET,        'Q',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeRobot          = { ROBOT,        'r',    FLAG_WORD1, 0,                    fh_player_bit};
static FLAGBITENT fbeRoyalty        = { ROYALTY,      'Z',    FLAG_WORD1, 0,                    fh_wiz};
static FLAGBITENT fbeSafe           = { SAFE,         's',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeSlave          = { SLAVE,        'x',    FLAG_WORD2, CA_WIZARD,            fh_wiz};
static FLAGBITENT fbeStaff          = { STAFF,        'w',    FLAG_WORD2, 0,                    fh_wiz};
static FLAGBITENT fbeSticky         = { STICKY,       'S',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeSuspect        = { SUSPECT,      'u',    FLAG_WORD2, CA_WIZARD,            fh_wiz};
static FLAGBITENT fbeTerse          = { TERSE,        'q',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeTrace          = { TRACE,        'T',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeTransparent    = { SEETHRU,      't',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeUnfindable     = { UNFINDABLE,   'U',    FLAG_WORD2, 0,                    fh_any};
static FLAGBITENT fbeUninspected    = { UNINSPECTED,  'g',    FLAG_WORD2, 0,                    fh_wizroy};
static FLAGBITENT fbeVacation       = { VACATION,     '|',    FLAG_WORD2, 0,                    fh_restrict_player};
static FLAGBITENT fbeVerbose        = { VERBOSE,      'v',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeVisual         = { VISUAL,       'V',    FLAG_WORD1, 0,                    fh_any};
static FLAGBITENT fbeWizard         = { WIZARD,       'W',    FLAG_WORD1, 0,                    fh_god};
static FLAGBITENT fbeSitemon        = { SITEMON,      '$',    FLAG_WORD3, 0,                    fh_wiz};
#ifdef WOD_REALMS
static FLAGBITENT fbeFae            = { FAE,          '0',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeChimera        = { CHIMERA,      '1',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbePeering        = { PEERING,      '2',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeUmbra          = { UMBRA,        '3',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeShroud         = { SHROUD,       '4',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeMatrix         = { MATRIX,       '5',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeObf            = { OBF,          '6',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeHss            = { HSS,          '7',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeMedium         = { MEDIUM,       '8',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeDead           = { DEAD,         '9',    FLAG_WORD3, CA_STAFF,             fh_wizroy};
static FLAGBITENT fbeMarker0        = { MARK_0,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker1        = { MARK_1,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker2        = { MARK_2,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker3        = { MARK_3,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker4        = { MARK_4,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker5        = { MARK_5,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker6        = { MARK_6,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker7        = { MARK_7,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker8        = { MARK_8,       ' ',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker9        = { MARK_9,       ' ',    FLAG_WORD3, 0,                    fh_god};
#else // WOD_REALMS
static FLAGBITENT fbeMarker0        = { MARK_0,       '0',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker1        = { MARK_1,       '1',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker2        = { MARK_2,       '2',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker3        = { MARK_3,       '3',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker4        = { MARK_4,       '4',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker5        = { MARK_5,       '5',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker6        = { MARK_6,       '6',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker7        = { MARK_7,       '7',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker8        = { MARK_8,       '8',    FLAG_WORD3, 0,                    fh_god};
static FLAGBITENT fbeMarker9        = { MARK_9,       '9',    FLAG_WORD3, 0,                    fh_god};
#endif // WOD_REALMS

FLAGNAMEENT gen_flag_names[] =
{
    {"ABODE",           TRUE, &fbeAbode          },
    {"ACCENTS",        FALSE, &fbeNoAccents      },
    {"ANSI",            TRUE, &fbeAnsi           },
    {"AUDIBLE",         TRUE, &fbeAudible        },
    {"AUDITORIUM",      TRUE, &fbeAuditorium     },
    {"BLEED",          FALSE, &fbeNoBleed        },
    {"BLIND",           TRUE, &fbeBlind          },
    {"COMMANDS",       FALSE, &fbeNoCommand      },
    {"CHOWN_OK",        TRUE, &fbeChownOk        },
    {"CONNECTED",       TRUE, &fbeConnected      },
    {"DARK",            TRUE, &fbeDark           },
    {"DESTROY_OK",      TRUE, &fbeDestroyOk      },
    {"ENTER_OK",        TRUE, &fbeEnterOk        },
    {"FIXED",           TRUE, &fbeFixed          },
    {"FLOATING",        TRUE, &fbeFloating       },
    {"GAGGED",          TRUE, &fbeGagged         },
    {"GOING",           TRUE, &fbeGoing          },
    {"HALTED",          TRUE, &fbeHalted         },
    {"HAS_DAILY",       TRUE, &fbeHasDaily       },
    {"HAS_FORWARDLIST", TRUE, &fbeHasForwardList },
    {"HAS_LISTEN",      TRUE, &fbeHasListen      },
    {"HAS_STARTUP",     TRUE, &fbeHasStartup     },
    {"HAVEN",           TRUE, &fbeHaven          },
    {"HEAD",            TRUE, &fbeHead           },
    {"HTML",            TRUE, &fbeHtml           },
    {"IMMORTAL",        TRUE, &fbeImmortal       },
    {"INHERIT",         TRUE, &fbeInherit        },
    {"JUMP_OK",         TRUE, &fbeJumpOk         },
    {"KEEPALIVE",       TRUE, &fbeKeepAlive      },
    {"KEY",             TRUE, &fbeKey            },
    {"LIGHT",           TRUE, &fbeLight          },
    {"LINK_OK",         TRUE, &fbeLinkOk         },
    {"MARKER0",         TRUE, &fbeMarker0        },
    {"MARKER1",         TRUE, &fbeMarker1        },
    {"MARKER2",         TRUE, &fbeMarker2        },
    {"MARKER3",         TRUE, &fbeMarker3        },
    {"MARKER4",         TRUE, &fbeMarker4        },
    {"MARKER5",         TRUE, &fbeMarker5        },
    {"MARKER6",         TRUE, &fbeMarker6        },
    {"MARKER7",         TRUE, &fbeMarker7        },
    {"MARKER8",         TRUE, &fbeMarker8        },
    {"MARKER9",         TRUE, &fbeMarker9        },
    {"MONITOR",         TRUE, &fbeMonitor        },
    {"MYOPIC",          TRUE, &fbeMyopic         },
    {"NO_COMMAND",      TRUE, &fbeNoCommand      },
    {"NOACCENTS",       TRUE, &fbeNoAccents      },
    {"NOBLEED",         TRUE, &fbeNoBleed        },
    {"NOSPOOF",         TRUE, &fbeNoSpoof        },
    {"OPAQUE",          TRUE, &fbeOpaque         },
    {"OPEN_OK",         TRUE, &fbeOpenOk         },
    {"PARENT_OK",       TRUE, &fbeParentOk       },
    {"PLAYER_MAILS",    TRUE, &fbePlayerMails    },
    {"PUPPET",          TRUE, &fbePuppet         },
    {"QUIET",           TRUE, &fbeQuiet          },
    {"ROBOT",           TRUE, &fbeRobot          },
    {"ROYALTY",         TRUE, &fbeRoyalty        },
    {"SAFE",            TRUE, &fbeSafe           },
    {"SITEMON",         TRUE, &fbeSitemon        },
    {"SLAVE",           TRUE, &fbeSlave          },
    {"SPOOF",          FALSE, &fbeNoSpoof        },
    {"STAFF",           TRUE, &fbeStaff          },
    {"STICKY",          TRUE, &fbeSticky         },
    {"SUSPECT",         TRUE, &fbeSuspect        },
    {"TERSE",           TRUE, &fbeTerse          },
    {"TRACE",           TRUE, &fbeTrace          },
    {"TRANSPARENT",     TRUE, &fbeTransparent    },
    {"UNFINDABLE",      TRUE, &fbeUnfindable     },
    {"UNINSPECTED",     TRUE, &fbeUninspected    },
    {"VACATION",        TRUE, &fbeVacation       },
    {"VERBOSE",         TRUE, &fbeVerbose        },
    {"VISUAL",          TRUE, &fbeVisual         },
    {"WIZARD",          TRUE, &fbeWizard         },
#ifdef WOD_REALMS
    {"FAE",             TRUE, &fbeFae            },
    {"CHIMERA",         TRUE, &fbeChimera        },
    {"PEERING",         TRUE, &fbePeering        },
    {"UMBRA",           TRUE, &fbeUmbra          },
    {"SHROUD",          TRUE, &fbeShroud         },
    {"MATRIX",          TRUE, &fbeMatrix         },
    {"OBF",             TRUE, &fbeObf            },
    {"HSS",             TRUE, &fbeHss            },
    {"MEDIUM",          TRUE, &fbeMedium         },
    {"DEAD",            TRUE, &fbeDead           },
#endif // WOD_REALMS
    {NULL,             FALSE, NULL}
};

OBJENT object_types[8] =
{
    {"ROOM",    'R', CA_PUBLIC, OF_CONTENTS|OF_EXITS|OF_DROPTO|OF_HOME},
    {"THING",   ' ', CA_PUBLIC, OF_CONTENTS|OF_LOCATION|OF_EXITS|OF_HOME|OF_SIBLINGS},
    {"EXIT",    'E', CA_PUBLIC, OF_SIBLINGS},
    {"PLAYER",  'P', CA_PUBLIC, OF_CONTENTS|OF_LOCATION|OF_EXITS|OF_HOME|OF_OWNER|OF_SIBLINGS},
    {"TYPE5",   '+', CA_GOD,    0},
    {"GARBAGE", '-', CA_PUBLIC, OF_CONTENTS|OF_LOCATION|OF_EXITS|OF_HOME|OF_SIBLINGS},
    {"GARBAGE", '#', CA_GOD,    0}
};

/* ---------------------------------------------------------------------------
 * init_flagtab: initialize flag hash tables.
 */

void init_flagtab(void)
{
    char *nbuf = alloc_sbuf("init_flagtab");
    for (FLAGNAMEENT *fp = gen_flag_names; fp->pOrigName; fp++)
    {
        fp->flagname = fp->pOrigName;
        strncpy(nbuf, fp->pOrigName, SBUF_SIZE);
        nbuf[SBUF_SIZE-1] = '\0';
        mux_strlwr(nbuf);
        hashaddLEN(nbuf, strlen(nbuf), (int *)fp, &mudstate.flags_htab);
    }
    free_sbuf(nbuf);
}

/* ---------------------------------------------------------------------------
 * display_flags: display available flags.
 */

void display_flagtab(dbref player)
{
    char *buf, *bp;
    FLAGNAMEENT *fp;

    bp = buf = alloc_lbuf("display_flagtab");
    safe_str("Flags:", buf, &bp);
    for (fp = gen_flag_names; fp->flagname; fp++)
    {
        FLAGBITENT *fbe = fp->fbe;
        if (  (  (fbe->listperm & CA_WIZARD)
              && !Wizard(player))
           || (  (fbe->listperm & CA_GOD)
              && !God(player)))
        {
            continue;
        }
        safe_chr(' ', buf, &bp);
        safe_str(fp->flagname, buf, &bp);
        if (fbe->flaglett != ' ')
        {
            safe_chr('(', buf, &bp);
            if (!fp->bPositive)
            {
                safe_chr('!', buf, &bp);
            }
            safe_chr(fbe->flaglett, buf, &bp);
            safe_chr(')', buf, &bp);
        }
    }
    *bp = '\0';
    notify(player, buf);
    free_lbuf(buf);
}

char *MakeCanonicalFlagName
(
    const char *pName,
    int *pnName,
    BOOL *pbValid
)
{
    static char buff[SBUF_SIZE];
    char *p = buff;
    int nName = 0;

    while (*pName && nName < SBUF_SIZE)
    {
        *p = Tiny_ToLower[(unsigned char)*pName];
        p++;
        pName++;
        nName++;
    }
    *p = '\0';
    if (nName < SBUF_SIZE)
    {
        *pnName = nName;
        *pbValid = TRUE;
        return buff;
    }
    else
    {
        *pnName = 0;
        *pbValid = FALSE;
        return NULL;
    }
}

FLAGNAMEENT *find_flag(char *flagname)
{
    // Convert flagname to canonical lowercase format.
    //
    int nName;
    BOOL bValid;
    char *pName = MakeCanonicalFlagName(flagname, &nName, &bValid);
    FLAGNAMEENT *fe = NULL;
    if (bValid)
    {
        fe = (FLAGNAMEENT *)hashfindLEN(pName, nName, &mudstate.flags_htab);
    }
    return fe;
}

// ---------------------------------------------------------------------------
// flag_set: Set or clear a specified flag on an object.
//
void flag_set(dbref target, dbref player, char *flag, int key)
{
    BOOL bDone = FALSE;

    do
    {
        // Trim spaces, and handle the negation character.
        //
        while (Tiny_IsSpace[(unsigned char)*flag])
        {
            flag++;
        }

        BOOL bNegate = FALSE;
        if (*flag == '!')
        {
            bNegate = TRUE;
            do
            {
                flag++;
            } while (Tiny_IsSpace[(unsigned char)*flag]);
        }

        // Beginning of flag name is now 'flag'.
        //
        char *nflag = flag;
        while (  *nflag != '\0'
              && !Tiny_IsSpace[(unsigned char)*nflag])
        {
            nflag++;
        }
        if (*nflag == '\0')
        {
            bDone = TRUE;
        }
        else
        {
            *nflag = '\0';
        }

        // Make sure a flag name was specified.
        //
        if (*flag == '\0')
        {
            if (bNegate)
            {
                notify(player, "You must specify a flag to clear.");
            }
            else
            {
                notify(player, "You must specify a flag to set.");
            }
        }
        else
        {
            FLAGNAMEENT *fp = find_flag(flag);
            if (!fp)
            {
                notify(player, "I do not understand that flag.");
            }
            else
            {
                FLAGBITENT *fbe = fp->fbe;

                BOOL bClearSet = bNegate;
                if (!fp->bPositive)
                {
                    bNegate = !bNegate;
                }

                // Invoke the flag handler, and print feedback.
                //
                if (!fbe->handler(target, player, fbe->flagvalue, fbe->flagflag, bNegate))
                {
                    notify(player, NOPERM_MESSAGE);
                }
                else if (!(key & SET_QUIET) && !Quiet(player))
                {
                    notify(player, (bClearSet ? "Cleared." : "Set."));
                }
            }
        }
        flag = nflag + 1;

    } while (!bDone);
}

/*
 * ---------------------------------------------------------------------------
 * * decode_flags: converts a flags word into corresponding letters.
 */

char *decode_flags(dbref player, FLAGSET *fs)
{
    char *buf, *bp;
    buf = bp = alloc_sbuf("decode_flags");
    *bp = '\0';

    if (!Good_obj(player))
    {
        strcpy(buf, "#-2 ERROR");
        return buf;
    }
    int flagtype = fs->word[FLAG_WORD1] & TYPE_MASK;
    BOOL bNeedColon = TRUE;
    if (object_types[flagtype].lett != ' ')
    {
        safe_sb_chr(object_types[flagtype].lett, buf, &bp);
        bNeedColon = FALSE;
    }

    FLAGNAMEENT *fp;
    for (fp = gen_flag_names; fp->flagname; fp++)
    {
        FLAGBITENT *fbe = fp->fbe;
        if (  !fp->bPositive
           || fbe->flaglett == ' ')
        {
            // Only look at positive-sense entries that have non-space flag
            // letters.
            //
            continue;
        }
        if (fs->word[fbe->flagflag] & fbe->flagvalue)
        {
            if (  (  (fbe->listperm & CA_STAFF)
                  && !Staff(player))
               || (  (fbe->listperm & CA_ADMIN)
                  && !WizRoy(player))
               || (  (fbe->listperm & CA_WIZARD)
                  && !Wizard(player))
               || (  (fbe->listperm & CA_GOD)
                  && !God(player)))
            {
                continue;
            }

            // Don't show CONNECT on dark wizards to mortals
            //
            if (  flagtype == TYPE_PLAYER
               && fbe->flagflag == FLAG_WORD2
               && fbe->flagvalue == CONNECTED
               && (fs->word[FLAG_WORD1] & (WIZARD | DARK)) == (WIZARD | DARK)
               && !See_Hidden(player))
            {
                continue;
            }

            if (  bNeedColon
               && Tiny_IsDigit[(unsigned char)fbe->flaglett])
            {
                // We can't allow numerical digits at the beginning.
                //
                safe_sb_chr(':', buf, &bp);
            }
            safe_sb_chr(fbe->flaglett, buf, &bp);
            bNeedColon = FALSE;
        }
    }
    *bp = '\0';
    return buf;
}

/*
 * ---------------------------------------------------------------------------
 * * has_flag: does object have flag visible to player?
 */

BOOL has_flag(dbref player, dbref it, char *flagname)
{
    FLAGNAMEENT *fp = find_flag(flagname);
    if (!fp)
    {
        return FALSE;
    }
    FLAGBITENT *fbe = fp->fbe;

    if (  (  fp->bPositive
          && (db[it].fs.word[fbe->flagflag] & fbe->flagvalue))
       || (  !fp->bPositive
          && (db[it].fs.word[fbe->flagflag] & fbe->flagvalue) == 0))
    {
        if (  (  (fbe->listperm & CA_STAFF)
              && !Staff(player))
           || (  (fbe->listperm & CA_ADMIN)
              && !WizRoy(player))
           || (  (fbe->listperm & CA_WIZARD)
              && !Wizard(player))
           || (  (fbe->listperm & CA_GOD)
              && !God(player)))
        {
            return FALSE;
        }

        // Don't show CONNECT on dark wizards to mortals
        //
        if (  isPlayer(it)
           && (fbe->flagvalue == CONNECTED)
           && (fbe->flagflag == FLAG_WORD2)
           && Hidden(it)
           && !See_Hidden(player))
        {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

/*
 * ---------------------------------------------------------------------------
 * * flag_description: Return an mbuf containing the type and flags on thing.
 */

char *flag_description(dbref player, dbref target)
{
    // Allocate the return buffer.
    //
    int otype = Typeof(target);
    char *buff = alloc_mbuf("flag_description");
    char *bp = buff;

    // Store the header strings and object type.
    //
    safe_mb_str("Type: ", buff, &bp);
    safe_mb_str(object_types[otype].name, buff, &bp);
    safe_mb_str(" Flags:", buff, &bp);
    if (object_types[otype].perm != CA_PUBLIC)
    {
        *bp = '\0';
        return buff;
    }

    // Store the type-invariant flags.
    //
    FLAGNAMEENT *fp;
    for (fp = gen_flag_names; fp->flagname; fp++)
    {
        if (!fp->bPositive)
        {
            continue;
        }
        FLAGBITENT *fbe = fp->fbe;
        if (db[target].fs.word[fbe->flagflag] & fbe->flagvalue)
        {
            if (  (  (fbe->listperm & CA_STAFF)
                  && !Staff(player))
               || (  (fbe->listperm & CA_ADMIN)
                  && !WizRoy(player))
               || (  (fbe->listperm & CA_WIZARD)
                  && !Wizard(player))
               || (  (fbe->listperm & CA_GOD)
                  && !God(player)))
            {
                continue;
            }

            // Don't show CONNECT on dark wizards to mortals.
            //
            if (  isPlayer(target)
               && (fbe->flagvalue == CONNECTED)
               && (fbe->flagflag == FLAG_WORD2)
               && Hidden(target)
               && !See_Hidden(player))
            {
                continue;
            }
            safe_mb_chr(' ', buff, &bp);
            safe_mb_str(fp->flagname, buff, &bp);
        }
    }

    // Terminate the string, and return the buffer to the caller.
    //
    *bp = '\0';
    return buff;
}

/*
 * ---------------------------------------------------------------------------
 * * Return an lbuf containing the name and number of an object
 */

char *unparse_object_numonly(dbref target)
{
    char *buf = alloc_lbuf("unparse_object_numonly");
    if (target < 0)
    {
        strcpy(buf, aszSpecialDBRefNames[-target]);
    }
    else if (!Good_obj(target))
    {
        sprintf(buf, "*ILLEGAL*(#%d)", target);
    }
    else
    {
        sprintf(buf, "%.200s(#%d)", Name(target), target);
    }
    return buf;
}

/*
 * ---------------------------------------------------------------------------
 * * Return an lbuf pointing to the object name and possibly the db# and flags
 */
char *unparse_object(dbref player, dbref target, BOOL obey_myopic)
{
    char *buf = alloc_lbuf("unparse_object");
    if (NOPERM <= target && target < 0)
    {
        strcpy(buf, aszSpecialDBRefNames[-target]);
    }
    else if (!Good_obj(target))
    {
        sprintf(buf, "*ILLEGAL*(#%d)", target);
    }
    else
    {
        BOOL exam;
        if (obey_myopic)
        {
            exam = MyopicExam(player, target);
        }
        else
        {
            exam = Examinable(player, target);
        }
        if (  exam
           || (Flags(target) & (CHOWN_OK | JUMP_OK | LINK_OK | DESTROY_OK))
           || (Flags2(target) & ABODE))
        {
            // show everything
            //
            char *fp = decode_flags(player, &(db[target].fs));
            sprintf(buf, "%.200s(#%d%s)", Moniker(target), target, fp);
            free_sbuf(fp);
        }
        else
        {
            // show only the name.
            //
            strcpy(buf, Moniker(target));
        }
    }
    return buf;
}

/* ---------------------------------------------------------------------------
 * cf_flag_access: Modify who can set a flag.
 */

CF_HAND(cf_flag_access)
{
    TINY_STRTOK_STATE tts;
    mux_strtok_src(&tts, str);
    Tiny_StrTokControl(&tts, " \t=,");
    char *fstr = mux_strtok_parse(&tts);
    char *permstr = mux_strtok_parse(&tts);

    if (!fstr || !*fstr)
    {
        return -1;
    }

    FLAGNAMEENT *fp;
    if ((fp = find_flag(fstr)) == NULL)
    {
        cf_log_notfound(player, cmd, "No such flag", fstr);
        return -1;
    }
    FLAGBITENT *fbe = fp->fbe;

    // Don't change the handlers on special things.
    //
    if (  (fbe->handler != fh_any)
       && (fbe->handler != fh_wizroy)
       && (fbe->handler != fh_wiz)
       && (fbe->handler != fh_god)
       && (fbe->handler != fh_restrict_player)
       && (fbe->handler != fh_privileged))
    {
        STARTLOG(LOG_CONFIGMODS, "CFG", "PERM");
        log_text("Cannot change access for flag: ");
        log_text(fp->flagname);
        ENDLOG;
        return -1;
    }

    if (!strcmp(permstr, "any"))
    {
        fbe->handler = fh_any;
    }
    else if (!strcmp(permstr, "royalty"))
    {
        fbe->handler = fh_wizroy;
    }
    else if (!strcmp(permstr, "wizard"))
    {
        fbe->handler = fh_wiz;
    }
    else if (!strcmp(permstr, "god"))
    {
        fbe->handler = fh_god;
    }
    else if (!strcmp(permstr, "restrict_player"))
    {
        fbe->handler = fh_restrict_player;
    }
    else if (!strcmp(permstr, "privileged"))
    {
        fbe->handler = fh_privileged;
    }
    else if (!strcmp(permstr, "staff"))
    {
        fbe->handler = fh_staff;
    }
    else
    {
        cf_log_notfound(player, cmd, "Flag access", permstr);
        return -1;
    }
    return 0;
}

/*
 * ---------------------------------------------------------------------------
 * * convert_flags: convert a list of flag letters into its bit pattern.
 * * Also set the type qualifier if specified and not already set.
 */

BOOL convert_flags(dbref player, char *flaglist, FLAGSET *fset, FLAG *p_type)
{
    FLAG type = NOTYPE;
    FLAGSET flagmask;
    int i;
    for (i = FLAG_WORD1; i <= FLAG_WORD3; i++)
    {
        flagmask.word[i] = 0;
    }

    char *s;
    BOOL handled;
    for (s = flaglist; *s; s++)
    {
        handled = FALSE;

        // Check for object type.
        //
        for (i = 0; i <= 7 && !handled; i++)
        {
            if (  object_types[i].lett == *s
               && !(  (  (object_types[i].perm & CA_STAFF)
                      && !Staff(player))
                   || (  (object_types[i].perm & CA_ADMIN)
                      && !WizRoy(player))
                   || (  (object_types[i].perm & CA_WIZARD)
                      && !Wizard(player))
                   || (  (object_types[i].perm & CA_GOD)
                      && !God(player))))
            {
                if (  type != NOTYPE
                   && type != i)
                {
                    char *p = tprintf("%c: Conflicting type specifications.",
                        *s);
                    notify(player, p);
                    return FALSE;
                }
                type = i;
                handled = TRUE;
            }
        }

        // Check generic flags.
        //
        if (handled)
        {
            continue;
        }
        FLAGNAMEENT *fp;
        for (fp = gen_flag_names; fp->flagname && !handled; fp++)
        {
            FLAGBITENT *fbe = fp->fbe;
            if (  !fp->bPositive
               || fbe->flaglett == ' ')
            {
                continue;
            }
            if (  fbe->flaglett == *s
               && !(  (  (fbe->listperm & CA_STAFF)
                      && !Staff(player))
                   || (  (fbe->listperm & CA_ADMIN)
                      && !WizRoy(player))
                   || (  (fbe->listperm & CA_WIZARD)
                      && !Wizard(player))
                   || (  (fbe->listperm & CA_GOD)
                      && !God(player))))
            {
                flagmask.word[fbe->flagflag] |= fbe->flagvalue;
                handled = TRUE;
            }
        }

        if (!handled)
        {
            notify(player,
                   tprintf("%c: Flag unknown or not valid for specified object type",
                       *s));
            return FALSE;
        }
    }

    // Return flags to search for and type.
    //
    *fset = flagmask;
    *p_type = type;
    return TRUE;
}

/*
 * ---------------------------------------------------------------------------
 * * decompile_flags: Produce commands to set flags on target.
 */

void decompile_flags(dbref player, dbref thing, char *thingname)
{
    // Report generic flags.
    //
    FLAGNAMEENT *fp;
    for (fp = gen_flag_names; fp->flagname; fp++)
    {
        FLAGBITENT *fbe = fp->fbe;

        // Only handle positive-sense entries.
        // Skip if we shouldn't decompile this flag.
        // Skip if this flag isn't set.
        // Skip if we can't see this flag.
        //
        if (  !fp->bPositive
           || (fbe->listperm & CA_NO_DECOMP)
           || (db[thing].fs.word[fbe->flagflag] & fbe->flagvalue) == 0
           || !check_access(player, fbe->listperm))
        {
            continue;
        }

        // Report this flag.
        //
        notify(player, tprintf("@set %s=%s", strip_ansi(thingname),
            fp->flagname));
    }
}

// do_flag: Rename flags or remove flag aliases.
// Based on RhostMUSH code.
//
BOOL flag_rename(char *alias, char *newname)
{
    int nAlias;
    BOOL bValidAlias;
    char *pCheckedAlias = MakeCanonicalFlagName(alias, &nAlias, &bValidAlias);
    if (!bValidAlias)
    {
        return FALSE;
    }

    int nNewName;
    BOOL bValidNewName;
    char *pCheckedNewName = MakeCanonicalFlagName(newname, &nNewName, &bValidNewName);
    if (!bValidNewName)
    {
        return FALSE;
    }

    char *pAlias = alloc_sbuf("flag_rename.old");
    memcpy(pAlias, pCheckedAlias, nAlias+1);
    char *pNewName = alloc_sbuf("flag_rename.new");
    memcpy(pNewName, pCheckedNewName, nNewName+1);

    FLAGNAMEENT *flag1;
    flag1 = (FLAGNAMEENT *)hashfindLEN(pAlias, nAlias, &mudstate.flags_htab);
    if (flag1 != NULL)
    {
        FLAGNAMEENT *flag2;
        flag2 = (FLAGNAMEENT *)hashfindLEN(pNewName, nNewName, &mudstate.flags_htab);
        if (flag2 == NULL)
        {
            hashaddLEN(pNewName, nNewName, (int *)flag1, &mudstate.flags_htab);

            if (flag1->flagname != flag1->pOrigName)
            {
                MEMFREE(flag1->flagname);
            }
            flag1->flagname = StringCloneLen(pNewName, nNewName);
            mux_strupr(flag1->flagname);

            free_sbuf(pAlias);
            free_sbuf(pNewName);
            return TRUE;
        }
    }
    free_sbuf(pAlias);
    free_sbuf(pNewName);
    return FALSE;
}

void do_flag(dbref executor, dbref caller, dbref enactor, int key, int nargs,
             char *flag1, char *flag2)
{
    if (key & FLAG_REMOVE)
    {
        if (nargs == 2)
        {
            notify(executor, "Extra argument ignored.");
        }
        int nAlias;
        BOOL bValidAlias;
        char *pCheckedAlias = MakeCanonicalFlagName(flag1, &nAlias, &bValidAlias);
        if (bValidAlias)
        {
            FLAGNAMEENT *lookup;
            lookup = (FLAGNAMEENT *)hashfindLEN(pCheckedAlias, nAlias, &mudstate.flags_htab);
            if (lookup)
            {
                if (  lookup->flagname != lookup->pOrigName
                   && mux_stricmp(lookup->flagname, pCheckedAlias) == 0)
                {
                    MEMFREE(lookup->flagname);
                    lookup->flagname = lookup->pOrigName;
                    hashdeleteLEN(pCheckedAlias, nAlias, &mudstate.flags_htab);
                    notify(executor, tprintf("Flag name '%s' removed from the hash table.", pCheckedAlias));
                }
                else
                {
                    notify(executor, "Error: You can't remove the present flag name from the hash table.");
                }
            }
        }
    }
    else
    {
        if (nargs < 2)
        {
            notify(executor, "You must specify a flag and a name.");
            return;
        }
        if (flag_rename(flag1, flag2))
        {
            notify(executor, "Flag name changed.");
        }
        else
        {
            notify(executor, "Error: Bad flagname given or flag not found.");
        }
    }
}

/* ---------------------------------------------------------------------------
 * cf_flag_name: Rename a flag. Counterpart to @flag.
 */

CF_HAND(cf_flag_name)
{
    TINY_STRTOK_STATE tts;
    mux_strtok_src(&tts, str);
    Tiny_StrTokControl(&tts, " \t=,");
    char *flagstr = mux_strtok_parse(&tts);
    char *namestr = mux_strtok_parse(&tts);

    if (  !flagstr 
       || !*flagstr
       || !namestr
       || !*namestr)
    {
        return -1;
    }

    if (flag_rename(flagstr, namestr))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
