#pragma once

/**********************************************/
// Game setup
/**********************************************/
#define DO_BUY_BONUS		false
#define DO_BUY_BONUS_OPTION	1		
#define DO_FREE_SPINS		true
#define DO_FLARES			true

/*******************************/
// Special modes
/*******************************/
#define PLAY_MODE		false		// true = play, false = sim
#define RN_STREAM_TEST	false		// true = get all rng results from RN_STREAM_FILE  
#define FIXED_SEED		false		// true = set rng to a repeatable sequence
#define SESSION_LIMIT 0				// reset gamePoints and zones after SESSION_LIMIT plays

/*******************************/
// RN_STREAM_TEST options
/*******************************/
// RN_STREAM_FILE : contains all rng results from the engine
#define RN_STREAM_FILE	"PhantomTides_901_BB_1_50K_RNS.csv"
#define RN_TEST_GAME -1				// show play output for this game 

/*******************************/
// Sim Reporting
/*******************************/
#define SHOW_RTP_REPORT_HISTORY false

/*******************************/
// Force Options
/*******************************/
#define FORCE_NONE -1

/*******************************/
// Force Config
/*******************************/

/**********************************************/
// Meters setup
/**********************************************/
#define PAYTABLE_METERS	false				

/**********************************************/
// Data Output
/**********************************************/
#define OUTPUT_REELS false

