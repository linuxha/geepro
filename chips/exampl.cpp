/* $Revision: 1.1.1.1 $ */
/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See the file COPYING. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "modules.h"
#define SIZE_DUMMY 16

/* makro for defining internal variables etc */
MODULE_IMPLEMENTATION

/* function to handle action */
void read_exec( /* additional arguments given by REGISTER_FUNCTION */ )
{
    int count = 100;
    int iterator_var; // set to 0 by makro progress_loop()

    progress_loop( iterator_var, count, "Displayed text")
    {
//	break_if( condition ); -> break loop, and sets ERROR_VAL = condition, if condition differs 0
        hw_delay(1000); // delay 1 ms 
        printf("%i\n", iterator_var);
    }
    finish_action();
}

/* Register function section */
REGISTER_FUNCTION( read, dummy, exec /* some arguments separated by comma You can add here */ );

/* Register chip section */
REGISTER_MODULE_BEGIN( Example dummy )

    /* register_chip_begin() - is a makro to provide basic information for chip, and 'instruction' for chip description
	- "/Example/path" is a path in chip menu
	- "dummy chip" is a chip name
	- "family" is an identifier for driver, defined in a driver xml file
	- SIZE_DUMMY is a size in bytes in buffer creation
    */
    register_chip_begin("/Example/path", "dummy chip", "family", SIZE_DUMMY); 
	/*
	    add_action() - is a makro that adds action like read, write, verify etc
	    - MODULE_READ_ACTION is an icon action identifier
	    - read_dummy is a callback to function created by REGISTER_FUNCTION makro, the name has format: action_chipname
	*/
	add_action( MODULE_READ_ACTION, read_dummy ); 
    /*
	ends definition for chip
    */
    register_chip_end;    

REGISTER_MODULE_END
