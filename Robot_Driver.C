/* MD2QC1E1.C  Level 1 Quick-C example program # 1 */

/*  Include files  */
#include <conio.h>              /*  For outp, inp, kbhit functions  */
#include <stdio.h>              /*  For example section  */
#include <math.h>               /*  For labs function  */

/*  Function prototypes needed by programmer  */
void    main(void);             /*  Example program main  */
void    md2setup(void);         /*  Initialize variables  */
void    md2on(void);            /*  Turn MD-2 on  */
void    md2off(void);           /*  Turn MD-2 off  */
void    md2home(void);          /*  Move motor home  */
void    md2move(void);          /*  Move motor  */

/*  Global motor parameters needed by programmer */
char        md2status;          /*  Completion status  */
int         md2hold;            /*  Hold motor when done  */
long        md2position[7];     /*  Current motor positions  */
int         md2motor;           /*  Selected motor  */
int         md2speed[7];        /*  Motor speeds  */
long        md2target[7];       /*  Target distance and direction  */

/*  Global motor parameters NOT needed by programmer */
unsigned    md2mtradr12;        /*  Port for motors 1 & 2  */
unsigned    md2mtradr34;        /*  Port for motors 3 & 4  */
unsigned    md2mtradr56;        /*  Port for motors 5 & 6  */
int         md2stppat[8];       /*  Step pattern array  */
int         md2patptr[7];       /*  Step pattern pointers  */


/*  main is just an example  */

void main(void)
{
	printf("\n\n\nmoving\n");

	/*  Setup  */
	md2setup();

	/*  Turn on MD-2  */
	md2motor = 5;
	md2on();

	/*  Home motors  */
	md2motor=5;
	md2home();
	md2motor=6;
	md2home();

	/*  Move motors  */
	md2motor=5;
	md2target[5]=400;
	md2move();

	md2motor=6;
	md2target[6]=100;
	md2move();

	md2motor=5;
	md2target[5]=-400;
	md2move();

	md2motor=6;
	md2target[6]=-100;
	md2move();

	/*  Turn off MD-2  */
	md2motor=5;
	md2off();

	/*  Print return status  */
	printf("return status = %c\n",md2status);
}


/*************************************************************************
Name:       MD2QC1S.C
Desc:       MD-2 level 1 motion control subroutines for the
			C language.  Written and tested in Microsoft
			Quick-C.  Should function with most compiliers
			with little or no modification.
By:         Copyright (c) 1993 Arrick Robotics, Roger Arrick
Date:       7/18/94
Edit:       9


SUBROUTINES **************************************************************

	NAME           DESCRIPTION
	----           -----------
	md2setup       Used at the beginning of a program to initialize
				   motor parameters to their default values.
				   Use this subroutine before any other.
	md2on          Turns on an MD-2 system.
	md2off         Turns off an MD-2 system.
	md2home        Moves a motor to the Home position by watching the
				   home switch.  Current position is set to zero.
	md2move        Moves a motor the number of steps in md2target(M)
				   parameter.

MOTOR PARAMETERS AND VARIABLES *******************************************

	NAME            TYPE    DESCRIPTION
	----            ----    -----------
	md2hold         integer -1=Leaves the motor energized after a move
							to cause the motor to hold its position.
							0 causes the motor to turn off after a move
							which conserves power and reduces heat.
	md2motor        integer The selected motor to act upon. 1,2,3,4,5 or 6.
	md2speed[M]     integer Delay count between steps. Determines the
							motor's.  0=fast, 32766=slow.
							Actual motor speed depends on the computer.
							M is the motor number 1,2,3,4,5 or 6.
	md2position[M]  long    Current motor position for each motor in steps
							relative to home. Negative = reverse from home
							and positive = forward from home.
							M is the motor number 1,2,3,4,5 or 6.
	md2status       char    Completion status. O=motion completed OK,
							K=a keypress stopped the motion, B=Bad parameter.
	md2target[M]    long    The number of steps to move. Positive #'s are
							forward, negative are reverse.
							M is the motor number 1,2,3,4,5 or 6.

VARIABLES NEEDED BY SUBROUTINES BUT NOT BY PROGRAMMER ********************

	NAME            TYPE    DESCRIPTION
	----            ----    -----------
	md2delay        integer Used for delay counter.
	md2steps        long    Used for step counter.
	md2dir          integer Used as direction storage.
	md2pmask        integer Bit mask for selected motor.
	md2omask        integer Bit mask for other motor.
	md2smask        integer Bit mask for switch.
	md2mtradr       integer Selected Port address.
	md2mtradr12     integer Port address for motors 1 & 2.
	md2mtradr34     integer Port address for motors 3 & 4.
	md2mtradr56     integer Port address for motors 5 & 6.
	md2stppat[7]    integer Step pattern array.
	md2patptr[6]    integer Pattern pointer for each motor.
	md2pattern      integer Selected step pattern.

*************************************************************************/


void md2setup()
{
	/********************************************************************
	Name:       md2setup
	Desc:       The md2setup procedure sets default MD-2 system
				parameters such as motor speed, current position, etc.
	Usage:      Use at the beginning of a motion control program.
	Inputs:     None.
	Outputs:    Default motor parameters.
	*********************************************************************/

	/* Setup port addresses. */
	md2mtradr12 = 0x3bc;
	md2mtradr34 = 0x378;
	md2mtradr56 = 0x278;

	/* Setup motor parameter defaults. */
	for (md2motor = 1; md2motor < 7; md2motor++ ) {
		md2position[md2motor] = 0;
		md2patptr[md2motor] = 0;
		md2target[md2motor] = 10;
		md2speed[md2motor] = 10000;
	}
	md2motor = 1;
	md2status = 'O';
	md2hold = 0;

	/*  Set half-step phase patterns.  */
	md2stppat[0] = 0x66;
	md2stppat[1] = 0x77;
	md2stppat[2] = 0x33;
	md2stppat[3] = 0xBB;
	md2stppat[4] = 0x99;
	md2stppat[5] = 0xDD;
	md2stppat[6] = 0xCC;
	md2stppat[7] = 0xEE;
}


void md2on()
{
	/********************************************************************
	Name:       md2on
	Desc:       The md2on procedure initializes a parallel printer port
				and turns on an MD-2.
	Usage:      Use at the beginning of a motion control program but
				after the md2setup subroutine.
	Inputs:     Motor # determines MD-2 port.
	Outputs:    None.
	*********************************************************************/

	if (md2motor == 1 || md2motor == 2) {
		outp(md2mtradr12, 0xff);
		outp(md2mtradr12 + 2, 0x05);
	}

	if (md2motor == 3 || md2motor == 4) {
		outp(md2mtradr34, 0xff);
		outp(md2mtradr34 + 2, 0x05);
	}

	if (md2motor == 5 || md2motor == 6) {
		outp(md2mtradr56, 0xff);
		outp(md2mtradr56 + 2, 0x05);
	}
}


void md2off()
{
	/********************************************************************
	Name:       md2off
	Desc:       The md2off procedure returns a parallel printer port
				referenced by the motor # to its previous state ready
				for use with a printer and disables the MD-2.
	Usage:      Use at the end of a motion control program.
	Inputs:     Motor # determines MD-2 port.
	Outputs:    None.
	*********************************************************************/

	/*  Local variables  */
	int i;

	/*  -strobe pin high, -alf high, -init low, -selin low, irq off. */
	if (md2motor == 1 || md2motor == 2) outp(md2mtradr12 + 2, 0x04);
	if (md2motor == 3 || md2motor == 4) outp(md2mtradr34 + 2, 0x04);
	if (md2motor == 5 || md2motor == 6) outp(md2mtradr56 + 2, 0x04);

	/*  Delay.  */
	for (i=1; i<2000; i++) ;

	/* Turn -init pin high */
	if (md2motor == 1 || md2motor == 2) outp(md2mtradr12 + 2, 0x0c);
	if (md2motor == 3 || md2motor == 4) outp(md2mtradr34 + 2, 0x0c);
	if (md2motor == 5 || md2motor == 6) outp(md2mtradr56 + 2, 0x0c);
}


void md2home()
{
	/********************************************************************
	Name:       md2home
	Desc:       The md2home procedure is used to move the stepper motor
				to a know home position.  All other moves are relative
				to this home (zero) position.  The selected motor is
				moved reverse until the switch is activated, then forward
				until deactivated.  The current position is then set to
				zero - this is the home position.
	Usage:      Set the desired motor # and parameters then call.
	Inputs:     Motor # and parameters.
	Outputs:    Current position set to zero, md2status.
	*********************************************************************/

	/*  Local variables  */
	int         md2delay;           /*  Delay loop counter  */
	int         md2pmask;           /*  Selected motor pattern bit mask  */
	int         md2omask;           /*  Other motor pattern bit mask  */
	int         md2smask;           /*  Switch bit mask  */
	unsigned    md2mtradr;          /*  Selected port address  */
	int         md2pattern;         /*  Selected step pattern  */

	/*  Set default status.  */
	md2status = 'O';

	/*  If bad motor # then bail out.  */
	if (md2motor < 1 || md2motor > 6) {
		md2status = 'B';
		return;
	}

	/*  Set up port address.  */
	if (md2motor == 1 || md2motor == 2) md2mtradr = md2mtradr12;
	if (md2motor == 3 || md2motor == 4) md2mtradr = md2mtradr34;
	if (md2motor == 5 || md2motor == 6) md2mtradr = md2mtradr56;

	/*  Set up pattern mask, other motor's mask and switch mask.  */
	if (md2motor == 1 || md2motor == 3 || md2motor == 5) {
		md2pmask = 0x0f;
		md2omask = 0xf0;
		md2smask = 0x20;
	}
	else {
		md2pmask = 0xf0;
		md2omask = 0x0f;
		md2smask = 0x10;
	}

	/******************************************
	 Move motor reverse until switch activated.
	 ******************************************/

	while ((inp(md2mtradr + 1) & md2smask) != 0) {

		/* Quit moving if key pressed */
		if (kbhit()) {
			md2status = 'K';
			break;
		}

		/* Point to the next step pattern */
		md2patptr[md2motor] = (md2patptr[md2motor] - 1) & 0x07;

		/* Get step pattern and mask off unneeded bit */
		md2pattern = md2stppat[md2patptr[md2motor]] & md2pmask;

		/* Don't disturb other motor's bits */
		md2pattern = md2pattern | (inp(md2mtradr) & md2omask);

		/* Output the step pattern to move the motor */
		outp(md2mtradr, md2pattern);

		/* Delay between steps for speed control */
		for (md2delay = md2speed[md2motor]; md2delay != 0; md2delay--) ;
	}


	/********************************************
	 Move motor forward until switch deactivated.
	 ********************************************/

	while ((inp(md2mtradr + 1) & md2smask) == 0) {

		/* Quit moving if key pressed */
		if (kbhit()) {
			md2status = 'K';
			break;
		}

		/* Point to the next step pattern */
		md2patptr[md2motor] = (md2patptr[md2motor] + 1) & 0x07;

		/* Get step pattern and mask off unneeded bit */
		md2pattern = md2stppat[md2patptr[md2motor]] & md2pmask;

		/* Don't disturb other motor's bits */
		md2pattern = md2pattern | (inp(md2mtradr) & md2omask);

		/* Output the step pattern to move the motor */
		outp(md2mtradr, md2pattern);

		/* Delay between steps for speed control */
		for (md2delay = md2speed[md2motor]; md2delay != 0; md2delay--);
	}

	/* Set position to 0 */
	md2position[md2motor] = 0;

	/* Power off motor if desired */
	if (md2hold == 0) outp(md2mtradr, 0xff);
}


void md2move()
{
	/********************************************************************
	Name:       md2move
	Desc:       The md2move procedure is used to move a stepper motor.
				The home switch is ignored.  A keypress will stop motion.
	Usage:      Set the desired motor # and parameters then call.
	Inputs:     Motor # and parameters.
	Outputs:    Current position set to zero, md2status.
	*********************************************************************/

	/*  Local variables  */
	long        md2steps;           /*  Step count  */
	int         md2dir;             /*  Direction  */
	int         md2delay;           /*  Delay loop counter  */
	int         md2pmask;           /*  Selected motor pattern bit mask  */
	int         md2omask;           /*  Other motor pattern bit mask  */
	unsigned    md2mtradr;          /*  Selected port address  */
	int         md2pattern;         /*  Selected step pattern  */

	/*  Set default status.  */
	md2status = 'O';

	/*  If bad motor # then bail out.  */
	if (md2motor < 1 || md2motor > 6) {
		md2status = 'B';
		return;
	}

	/*  Set up port address.  */
	if (md2motor == 1 || md2motor == 2) md2mtradr = md2mtradr12;
	if (md2motor == 3 || md2motor == 4) md2mtradr = md2mtradr34;
	if (md2motor == 5 || md2motor == 6) md2mtradr = md2mtradr56;

	/*  Set up pattern mask and other motor's mask  */
	if (md2motor == 1 || md2motor == 3 || md2motor == 5) {
		md2pmask = 0x0f;
		md2omask = 0xf0;
	}
	else {
		md2pmask = 0xf0;
		md2omask = 0x0f;
	}

	/* Set the # of steps */
	md2steps = labs(md2target[md2motor]);

	/* Set the direction */
	if (md2target[md2motor] < 0) md2dir = -1; else md2dir = 1;

	/* Set the final motor position */
	md2position[md2motor] = md2position[md2motor] + md2target[md2motor];

	/* Move Loop */
	while (md2steps != 0) {

		/* Quit moving if key pressed */
		if (kbhit()) {
			md2status = 'K';
			break;
		}

		/* Point to the next step pattern */
		md2patptr[md2motor] = (md2patptr[md2motor] + md2dir) & 0x07;

		/* Get step pattern and mask off unneeded bit */
		md2pattern = md2stppat[md2patptr[md2motor]] & md2pmask;

		/* Don't disturb other motor's bits */
		md2pattern = md2pattern | (inp(md2mtradr) & md2omask);

		/* Output the step pattern to move the motor */
		outp(md2mtradr, md2pattern);

		/* Delay between steps for speed control */
		for (md2delay = md2speed[md2motor]; md2delay != 0; md2delay--);

		/* Decrement step count */
		md2steps--;
	}

	/* Update position */
	md2position[md2motor] = md2position[md2motor] - (md2steps * md2dir);

	/* Power off motor if desired */
	if (md2hold == 0) outp(md2mtradr, 0xff);
}
