/* $Revision: 1.5 $ */
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

#include <iostream>
#include <string>
#include <list>

extern "C" {
#include "src/config.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <signal.h>

#include "buffer.h"
#include "../gui-gtk/gui.h"
#include "main.h"
#include "chip.h"
#include "../intl/lang.h"
#include "../drivers/hwdriver.h"
#include "dummy.h"
#include "geepro.h"
#include "storings.h"


/* uchwyt api do wybranego sterownika, global na cały program */
hw_driver_type ___hardware_driver___ = dummy_hardware_driver; 

/*global do zmiennych przechowywanych w pliku */
store_str store;

// Ugly global variable to inform drivers of the location
// of the xml gui file.
const char *shared_drivers_xml_file;
const char *shared_geepro_dir;
}

//#include <string>

/* globalna zmienna zawierająca uzytkownika */
int ___uid___= -1;

int test_uid(geepro *gep)
{
    if((___uid___ = getuid())) 
	gui_dialog_box(gep, "[WN][TEXT]\n   For lower latency time \n please run program using \n                 sudo[/TEXT][BR]  OK  ");
    return ___uid___;
}

char test_hw(void *wg, geepro *gep)
{
    for(;;)
	if(hw_test_conn()){
        	gui_dialog_box(gep,"[IF][TEXT]\n     Hardware present [/TEXT][BR]  OK  ");
	    break;
	}else{
		if(gui_dialog_box(gep,"[ER][TEXT]\n hardware error :  Check Power and\n connections[/TEXT][BR]Try again[BR]Cancel") == 2) return 0;
	}
    return 1;
}

bool file_exists(const std::string fn)
{
    struct stat stfi;
    int ret;
    ret=stat(fn.c_str(),&stfi);
    if (ret == 0) return true; else return false;
}

bool find_directory_of_file(std::string &dir,const std::string file,const char *lookthere[],int nb)
{
    static const std::string sep="/";
    for (int i=0;i<nb;i++) {
        std::string myfile=lookthere[i]+sep+file;
        if (file_exists(myfile)) {
            //std::cout << "OK for: "+myfile <<std::endl;
            dir=lookthere[i];
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    geepro geep;
    gui g;

    geep.gui = &g;
    geep.ifc = iface_init();
    geep.ifc->gep = &geep;
    geep.argc = argc;
    geep.argv = argv;
    geep.chp = NULL;

    // Looking for prefix location of data files
    std::string drivers_path;
    static const char* drivers_pathlist[]={"./drivers",DEFAULT_DRIVERS_PATH,"/usr/lib/geepro/drivers"};
    find_directory_of_file(drivers_path,"willem.driver",drivers_pathlist,3);

    std::string chips_path;
    static const char* chips_pathlist[]={"./chips",DEFAULT_CHIPS_PATH,"/usr/lib/geepro/chips"};
    find_directory_of_file(chips_path,"24Cxx.chip",chips_pathlist,3);

    std::string share_path;
    static const char* share_pathlist[]={".",DEFAULT_SHARE_PATH,"/usr/share/geepro"};
    find_directory_of_file(share_path,"drivers/willem.xml",share_pathlist,3);
    std::string shared_drivers_xml_file_s="file://"+share_path+"/drivers/willem.xml";
    shared_drivers_xml_file=shared_drivers_xml_file_s.c_str();
    std::string shared_geepro_dir_s=share_path+"/";
    shared_geepro_dir=shared_geepro_dir_s.c_str();
    
    store_constr(&store, "~/.geepro","geepro.st");
// do poprawki jak będzie config - te wszystkie stałe mają być pobierane z pliku configuracyjnego 
    iface_driver_allow(geep.ifc, "willem:stk200:xilinx_cable_iii:altera_byteblaster:jtag_noname:galblast:funprog");
    iface_module_allow(geep.ifc, "prom:mcs51:mcs48:exampl:93Cxx:27xx:24Cxx:28xx:hub_lpc:29xx:25xx");
    iface_load_config(geep.ifc, NULL);
    iface_make_driver_list(geep.ifc, drivers_path.c_str(), ".driver");
    gui_menu_setup(&geep);
/* moduły chipów inicjują menu gui, dlatego gui musi być zainicjowane */
/* parametry z configa w przyszłości */
    iface_make_modules_list( geep.ifc, chips_path.c_str(), ".chip"); 

//    signal(SIGINT, kill_me);
    
    gui_run(&geep);
    iface_destroy(geep.ifc);
    store_destr(&store);
    return 0;
}

