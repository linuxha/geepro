/* $Revision: 1.4 $ */
/* 
 * Copyright (C) Krzysztof Komarnicki
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

extern "C" {
    #include "iface.h"
    #include "../drivers/hwdriver.h"
    #include "chip.h"
    #include "main.h"
    #include "files.h"
}

#include "../intl/lang.h"

iface *iface_init()
{
    iface *ifc;
    if(!(ifc = (iface *)malloc(sizeof(iface)))) return NULL;
    ifc->qe = NULL;
    ifc->prg = NULL;
    ifc->plg = NULL;
    if(!(ifc->plugins = (chip_plugins *)malloc(sizeof(chip_plugins)))) return NULL;
    chip_init_qe(ifc->plugins);
    return ifc;
}

void iface_rmv_ifc(iface *ifc)
{
    iface_qe *tmp;
    
    while(ifc->qe){
	tmp = ifc->qe->next;
	free(ifc->qe->name);
	free(ifc->qe->dev);
	free(ifc->qe);
	ifc->qe = tmp;
    }
}

void iface_rmv_prg(iface *ifc)
{
    iface_prg *ti;

    while(ifc->prg){
	ti = ifc->prg->next;
	free(ifc->prg);
	ifc->prg =ti;
    }
}

int iface_add(iface *ifc, int cl, char *name, char *dev)
{
    iface_qe *new_tie, *tmp;
    
    if(!(new_tie = (iface_qe*) malloc(sizeof(iface_qe)))){
	printf("{iface.h} iface_add() -> memory allocation error (1) \n");
	return -1;
    }

    new_tie->next = NULL;
    
    if(!(new_tie->name = (char *)malloc(strlen(name) + 1))){
	printf("{iface.h} iface_add() -> memory allocation error (2) \n");
	free(new_tie);
	return -1;
    }
    strcpy(new_tie->name, name);    

    if(!(new_tie->dev = (char *)malloc(strlen(dev) + 1))){
	printf("{iface.h} iface_add() -> memory allocation error (3) \n");
	free(new_tie->name);
	free(new_tie);
	return -1;
    }
    strcpy(new_tie->dev, dev);

    new_tie->cl = cl;
    
    if(!ifc->qe){
	ifc->qe = new_tie;
	return 0;
    }
    
    for(tmp = ifc->qe; tmp->next; tmp = tmp->next);
    tmp->next = new_tie;
    
    return 0;
}

void iface_search(iface *ifc, int cl, iface_cb cb, void *ptr)
{
    iface_qe *tmp;
    
    for(tmp = ifc->qe; tmp; tmp = tmp->next) 
	    if(cb && tmp->cl == cl) cb(ifc, tmp->cl, tmp->name, tmp->dev, ptr);

}

char *iface_get_dev(iface *ifc, char *name)
{
    iface_qe *tmp;
    for(tmp = ifc->qe; tmp; tmp = tmp->next) 
	    if(!strcmp(tmp->name, name)) return tmp->dev;

    return (char *)"/dev/null";
}

int iface_select_iface(iface *ifc, char *name)
{
    char *dev;
    iface *ifct = ifc;    
    dev = iface_get_dev(ifc, name);
    printf("Opening interface '%s' (device: '%s')\n", name, dev);
    if(hw_open(dev, 0) == HW_ERROR){
	ifc = ifct;
	return -1;
    }
    return 0;
}

/* dopisać detekcję nazw, aby sie nie powtarzały */
int iface_prg_add(iface *ifc, iface_prg_api api, char on)
{
    iface_prg *new_tie, *tmp;
    char *d_name = NULL;
    if(!api) return -1;

    api(HW_NAME, 0, &d_name);
    if(!d_name) {
	printf("{iface.h} iface_add() -> driver rejected due to missing name !\n");
	return -1; /* brak nazwy */
    }

    if(!(new_tie = (iface_prg*) malloc(sizeof(iface_prg)))){
	printf("{iface.h} iface_add() -> memory allocation error (1) \n");
	return -1;
    }

    new_tie->next = NULL;
    api(HW_NAME, 0, &new_tie->name);
    new_tie->api = api;
    new_tie->on = on;

    printf("   Adding driver: %s\n", new_tie->name);

    if(!ifc->prg){
	ifc->prg = new_tie;
	return 0;
    }

    for(tmp = ifc->prg; tmp->next; tmp = tmp->next);
    tmp->next = new_tie;

    return 0;
}

void iface_list_prg(iface *ifc, iface_prg_func fc, void *ptr)
{
    iface_prg *tmp;
    
    for(tmp = ifc->prg; tmp; tmp = tmp->next) 
		    if(fc) fc(ifc, tmp->name, ptr);

}

iface_prg_api iface_get_func(iface *ifc, char *name)
{
    iface_prg *tmp;
    
    for(tmp = ifc->prg; tmp; tmp = tmp->next) 
		    if(!strcmp(tmp->name, name)) return tmp->api;
    
    return NULL;
}

/*****************************************************************************************/

int iface_add_driver(iface *ifc, void *phandler, iface_regf fc)
{
    iface_driver *new_tie, *tmp;

    if(!(phandler && fc)) return -1;

    if(!(new_tie = (iface_driver*) malloc(sizeof(iface_driver)))){
	printf("{iface.h} iface_add_driver() -> memory allocation error (1) \n");
	return -1;
    }

    new_tie->next = NULL;
    new_tie->phandler = phandler;
    new_tie->fc = fc;

    if(!ifc->plg){
	ifc->plg = new_tie;
	return 0;
    }

    for(tmp = ifc->plg; tmp->next; tmp = tmp->next);
    tmp->next = new_tie;

    return 0;
}

void iface_rmv_driver(iface *ifc)
{
    iface_driver *tmp;

    while(ifc->plg){
	tmp = ifc->plg->next;
	dlclose(ifc->plg->phandler);
	free(ifc->plg);
	ifc->plg =tmp;
    }
}

int iface_dir_fltr(iface *ifc, const char *lst, const char *path, const char *ext, iface_cb_fltr cb)
{
    char *old_path, *tmp, *tx, *n;
    DIR *dir;
    struct dirent *dr;
    char ex;
    int i;

    for(i = 1, old_path = (char *)malloc(16 * sizeof(char)); !(getcwd(old_path, i * 16)); ){
	if(errno != ERANGE){
	    printf("{iface.h} iface_dir_fltr() -> error getting cwd, errno=%i\n", errno);
	    free(old_path);
	    return -1;
	}
	i++;
	old_path = (char *)realloc(old_path, i * 16 * sizeof(char));
    }


    if(chdir(path) != 0){
	printf("{iface.h} iface_dir_fltr() -> chdir() error 1 \n");
	return -1;
    }        
    if(!(dir = opendir("./"))){
	printf("{iface.h} iface_dir_fltr() -> error opening current directory \n");
	return -1;
    }
    
    while((dr = readdir(dir))){
	for(tmp = dr->d_name + strlen(dr->d_name); tmp != dr->d_name && *tmp != '.'; tmp--);
	if( strcmp(tmp, ext) ) continue;


	for(n = (char *)lst; *n; ){
	
	    for(ex = 0, tx = dr->d_name; *tx && (tx != tmp); tx++, n++ ) 
						    if(*tx != *n){ ex = 1; break; };

	    if(!ex)
	    	if(cb(ifc, path, dr->d_name, old_path) == -1) return -1;
	    
	    for(; *n && (*n != ':'); n++); /* dojechanie do końca nazwy, która nie jest zgodna */
	    if(*n == ':') n++;
	}		
    }

    closedir(dir);
    if(chdir(old_path) != 0){
	printf("{iface.h} iface_dir_fltr() -> chdir() error 2 \n");
	return -1;
    }        

    free(old_path);
    return 0;
} 

static int iface_add_plug_file(iface *ifc, const char *pth, const char *name, const char *cwd)
{
    void *pf;
    iface_regf init;
    char tx[256];
    char *tmp = tx, *path = (char*)pth;
    int len, z, n=0;
    if(*path != '/' ){ /* nie jest ścieżką absolutną */
	path = (char*)malloc(sizeof(char) * (strlen(pth) + strlen(cwd) + 2));
	sprintf(path, "%s/%s", cwd, pth);
	n = 1;
    }

    len = strlen(path) + strlen(name) + 2;
    if( len > 256) 
	tmp = (char *) malloc(len * sizeof(char));
    else
	len = 0;    

    /* jesli brakuje '/' na końcu ścieżki to dodaj '/' */
    strcpy(tmp, path);
    z = strlen(path) - 1;
    if(z < 0) z = 0;
    if( *(tmp + z ) != '/') strcat(tmp, "/");
    strcat(tmp, name);
    printf("Adding driver file '%s' ... ", tmp);
    /* czy ścieżka absoltna, jeśli nie to dodaj cwd */
    if(n) free(path);
    /* otwarcie pluginu */
    dlerror(); /* wyzerowanie błędów */
    if(!(pf = dlopen(tmp, RTLD_LAZY))){
	printf("Error: dlopen() --> %s\n", dlerror());
	if(len) free(tmp);
	return -2;
    }
    if(len) free(tmp); /* ściezka do pliku jest już niepotrzebna */

    if(!(init = (iface_regf)dlsym(pf, IFACE_DRIVER_INIT_FUNC_NAME))){
	printf("Error: dlsym() --> %s\n", dlerror());
	dlclose(pf);
	return -2;
    }
    printf("OK\nInit drivers:\n");

    /* wywołanie funkcji rejestrującej plugin */
    if(init(ifc)){
	printf("Error: " IFACE_DRIVER_INIT_FUNC_NAME "()\n");
	dlclose(pf);
	return -2;
    }

    /* dodanie pluginu do kolejki */
    if(iface_add_driver(ifc, pf, init)){
	printf("Error: " IFACE_DRIVER_INIT_FUNC_NAME "()\n");
	dlclose(pf);
	return -2;
    };

    return 0;
}

int iface_make_driver_list(iface *ifc, const char *path, const char *ext)
{
    return iface_dir_fltr(ifc, ifc->plg_list, path, ext, iface_add_plug_file);
}

void iface_rmv_modules(iface *ifc)
{
    modules *m = ifc->plugins->mdl;
    while(m->first_modl){
	m->modl = (mod_list *)m->first_modl->next;
	dlclose(m->first_modl->handler);	
	free(m->first_modl);
	m->first_modl = m->modl;
    }
    m->modl = NULL;
}

void iface_destroy(iface *ifc)
{
    if(!ifc) return;
    hw_close();
    iface_rmv_ifc(ifc);
    iface_rmv_prg(ifc);
    iface_rmv_driver(ifc);
    chip_rmv_qe(ifc->plugins);
    if(ifc->plugins->mdl) iface_rmv_modules(ifc);
    if(ifc->plugins) free(ifc->plugins);
    free(ifc);
}

void iface_driver_allow(iface *ifc, const char *list)
{
    ifc->plg_list = (char*)list;
}

static const char *iface_get_iface_wildcard( int type)
{
    switch (type) {
	case IFACE_LPT: return "^parport[[:digit:]]*$";
	case IFACE_USB: return "^usb[[:digit:]]*$";
	case IFACE_RS232: return "^ttyS[[:digit:]]*$";
    }
    
    return NULL;
}

typedef struct 
{
    iface *ifc;
    int   type;    
} iface_tmp_arg;

static boolean iface_callback_list(const char *fname, const char *error, void *arg)
{
    char tmp[4096];
    
    if( (strlen(fname) + strlen(SYSTEM_DEVICE_PATH)) > 4094) return false;
    
    iface_tmp_arg *targ = (iface_tmp_arg *)arg;
    
    sprintf(tmp, "%s%s", SYSTEM_DEVICE_PATH, fname);
    
    iface_add(targ->ifc, targ->type, (char *)fname, tmp);
    return true;
}

static void iface_add_list(iface *ifc, int iface_type)
{
    char error[256];
    const char *regex = iface_get_iface_wildcard( iface_type );
    iface_tmp_arg tmp;
    
    tmp.ifc = ifc;
    tmp.type = iface_type;
    error[0] = 0;
        
    if(!file_ls(SYSTEM_DEVICE_PATH, regex, error, iface_callback_list, &tmp)){
	fprintf(stderr, "[ERROR]%s\n",error);
    }

}

int iface_load_config(iface *ifc, void *cfg)
{
    char *tmp;
    ifc->ifc_sel = 0;
    ifc->cl = IFACE_LPT;

    ifc->prog_sel = 0;    
    tmp = NULL;
    if(!store_get(&store, "LAST_SELECTED_PROGRAMMER", &tmp)){
	if( tmp ){
	    ifc->prog_sel = strtol(tmp, NULL, 0);
	    free(tmp);
	}
    }
    tmp = NULL;
    if(!store_get(&store, "LAST_SELECTED_IFACE", &tmp)){
	if( tmp ){
	    ifc->ifc_sel = strtol(tmp, NULL, 0);
	    free(tmp);
	}
    }

// interface to choose
    iface_add_list( ifc, IFACE_LPT);
    iface_add_list( ifc, IFACE_USB);
    iface_add_list( ifc, IFACE_RS232);
//exit(0);
    return 0;
}


/******************************************************************************************************************/

void iface_module_allow(iface *ifc, const char *list)
{
    ifc->mod_list = (char*)list;
}

/* kod poniżej do poprawy */
static int iface_add_module(iface *ifc, void *pf, void *init)
{
    static char first_run=1;
    mod_list *tmp;

    if(!(tmp = (mod_list *) malloc(sizeof(mod_list)))){
	printf("{iface.c} iface_add_module() --> memory allocation error !\n");
	return -2;
    }
    tmp->handler = pf;
    tmp->init_module = (int (*)(chip_plugins*))init;
    tmp->next = NULL;
    if(first_run){
	first_run = 0;
	ifc->plugins->mdl->modl = ifc->plugins->mdl->first_modl = tmp;
	return 0;
    }
    
    for(ifc->plugins->mdl->modl = ifc->plugins->mdl->first_modl; 
	ifc->plugins->mdl->modl->next; ifc->plugins->mdl->modl = (mod_list *)ifc->plugins->mdl->modl->next);

    ifc->plugins->mdl->modl->next = tmp;
    return 0;
}

static int iface_add_mod_file(iface *ifc, const char *pth, const char *name, const char *cwd)
{
    void *pf;
    iface_regf init;
    char tx[256];
    char *tmp = tx, *path = (char*)pth;
    int len, z, n=0;

    if(*path != '/' ){ /* nie jest ścieżką absolutną */
	path = (char*)malloc(sizeof(char) * (strlen(pth) + strlen(cwd) + 2));
	sprintf(path, "%s/%s", cwd, pth);
	n = 1;
    }

    len = strlen(path) + strlen(name) + 2;
    if( len > 256) 
	tmp = (char *) malloc(len * sizeof(char));
    else
	len = 0;    

    /* jesli brakuje '/' na końcu ścieżki to dodaj '/' */
    strcpy(tmp, path);
    z = strlen(path) - 1;
    if(z < 0) z = 0;
    if( *(tmp + z ) != '/') strcat(tmp, "/");
    strcat(tmp, name);
    printf("Adding module file %s --> ", name);

    /* czy ścieżka absoltna, jeśli nie to dodaj cwd */
    if(n) free(path);

    /* otwarcie pluginu */
    dlerror(); /* wyzerowanie błędów */
    if(!(pf = dlopen(tmp, RTLD_LAZY))){
	printf("Error: dlopen() --> %s\n", dlerror());
	if(len) free(tmp);
	return -2;
    }
    if(len) free(tmp); /* ściezka do pliku jest już niepotrzebna */

    if(!(init = (iface_regf)dlsym(pf, IFACE_MODULE_INIT_FUNC_NAME))){
	printf("Error: dlsym() --> %s\n", dlerror());
	dlclose(pf);
	return -2;
    }


    /* wywołanie funkcji rejestrującej plugin */
    if(init(ifc->gep)){
	printf("Error: " IFACE_MODULE_INIT_FUNC_NAME "()\n");
	dlclose(pf);
	return -2;
    }

    /* dodanie pluginu do kolejki */
    if(iface_add_module(ifc, pf, (void *)init)){
	printf("Error: " IFACE_MODULE_INIT_FUNC_NAME "()\n");
	dlclose(pf);
	return -2;
    };

    return 0;
}

void iface_make_modules_list(iface *ifc, const char *path, const char *ext)
{
    iface_dir_fltr(ifc, ifc->mod_list, path, ext, iface_add_mod_file);
}

