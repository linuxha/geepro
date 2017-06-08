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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
extern "C" {
#include "chip.h"
}
#include "../intl/lang.h"

static int __plugins__ = 0;    /* zmienna gloabalna pilnująca zainicjowania kolejek */

/**************************************************************************************************************************/
/* rejestracja akcji na ukladzie */

void chip_add_action(chip_desc *chip, const char *bt_name, const char *bt_tip, chip_act_func action)
{
    chip_action *new_tie, *tmp; 
    
    if(!(new_tie = (chip_action *)malloc(sizeof(chip_action)))){
	printf("{chip.c} chip_add_action() ---> out of memory.\n");
	return;
    }
    
    new_tie->name = (char *)bt_name;
    new_tie->tip  = (char *)bt_tip;
    new_tie->action  = action;
    new_tie->next = NULL;
    
    if(!chip->actions){
	chip->actions = new_tie;
	return;
    }
    
    for(tmp = chip->actions; tmp->next; tmp = tmp->next);
    tmp->next = new_tie;

}

static void chip_rem_actions(chip_desc *chip)
{
    chip_action *tmp, *x; 
    
    tmp = chip->actions;
    while( tmp ){
	x = tmp->next;
	free(tmp);
	tmp = x;
    }
    chip->actions = NULL;
}

int chip_list_action(chip_desc *chip, int (*cb)(chip_desc *, chip_action *, void *), void *ptr)
{
    int x;
    chip_action *tmp;
    
    for(tmp = chip->actions; tmp; tmp = tmp->next)
	if((x = cb(chip, tmp, ptr))) return x;

    return 0;
}

/********************************************************************************************************************/
int chip_register_chip(chip_plugins *plg, chip_desc *new_chip)
{
    chip_desc *new_tie, *tmp;

    if(!__plugins__){
	printf("chip_register_chip() called without initialisation (see chip_init_qe()) \n");
	return CHIP_ERROR;
    }

    if(!new_chip) return CHIP_ERROR;
    
    if(!(new_chip->chip_name && new_chip->chip_path )){
	printf(BAD_CHIP_NAME);
	return CHIP_ERROR;
    }

    MSG_1DEBUG("{chip.c} CHIP: Dodawanie ukladu\n");

    if(!(new_tie = (chip_desc *)malloc(sizeof(chip_desc)))){
	printf("[!!] chip_register(plugin, path, widget) -> memory allocation error (1)\n"); 
	return CHIP_ERROR;
    }

    memcpy(new_tie, new_chip, sizeof(chip_desc)); /* skopiowanie struktury definicji układu do nowej pozycji w kolejce */

    if(!(new_tie->chip_path = (char *)malloc(sizeof(char) * (strlen(new_chip->chip_path) +1)))){
	printf("[!!] chip_register(plugin, path, widget) -> memory allocation error (2)\n"); 
	free(new_tie);
	return CHIP_ERROR;
    }
    strcpy(new_tie->chip_path, new_chip->chip_path); /* skopiowanie ścieżki menu */
    if(!(new_tie->chip_name = (char *)malloc(sizeof(char) * (strlen(new_chip->chip_name) +1)))){
	printf("[!!] chip_register(plugin, path, widget) -> memory allocation error (3)\n"); 
	free(new_tie->chip_path);
	free(new_tie);
	return CHIP_ERROR;
    }
    strcpy(new_tie->chip_name, new_chip->chip_name); /* skopiowanie nazwy ukladu */

    new_tie->next = NULL;

    if(!plg->chip_qe){
	plg->chip_qe = new_tie;
	return 0;
    }

    for(tmp = plg->chip_qe; tmp->next; tmp = (chip_desc*)tmp->next);
    tmp->next = new_tie;
    
    return 0;
}

chip_desc *chip_lookup_chip(chip_plugins *plg, const char *name)
{
    if(!name){
	printf("[ERR] chip_lookup_chip() ---> name == NULL !\n");
	return NULL;
    }
    if(!plg){
	printf("[ERR] chip_lookup_chip() ---> plg == NULL !\n");
	return NULL;
    }
    if(!__plugins__){
	printf("chip_lookup_chip() called without initialisation (see chip_init_qe()) \n");
	return NULL;
    }
    for(plg->chip_sel = plg->chip_qe; plg->chip_sel; plg->chip_sel = plg->chip_sel->next )
	    if(!strcmp(plg->chip_sel->chip_name, name)) return plg->chip_sel;
    return NULL;
}

int chip_unregister_chip(chip_plugins *plg, char *name)
{
    char t = 1;
    chip_desc *curr, *prev;

    MSG_2DEBUG("{chip.c} CHIP: Usuwanie układu %s\n", name);
    if(!__plugins__){
	printf("chip_del_chip() called without initialisation (see chip_init_qe()) \n");
	return CHIP_ERROR;
    }

    for(
	curr = prev = plg->chip_qe; 
	curr->next && (t = strcmp(curr->chip_name, name)); 
	curr = (chip_desc *)curr->next
    ) prev = curr;

    if(t) return 0; /* nie ma ogniwa o takiej nazwie */

    prev->next = curr->next; /* przepisanie wskaźników z poprzedniego do kolejnego */
    
    if(curr == plg->chip_qe) plg->chip_qe = curr->next; /* jesli kasujemy pierwszy element kolejki */
    
    if(plg->chip_sel == curr) plg->chip_sel = NULL;

    free(curr->chip_path);
    free(curr->chip_name);
    chip_rem_actions(curr);
    free(curr);
    
    return 1;
}

void chip_destroy(chip_plugins *plg)
{
    chip_desc *tmp, *tmp1;
    
    if(!__plugins__){
	printf("chip_rm_chip() called without initialisation (see chip_init_qe()) \n");
	return;
    }

    tmp = plg->chip_qe;

    while( tmp ){
	tmp1 = tmp->next;
	free(tmp->chip_path);
	free(tmp->chip_name);
	chip_rem_actions(tmp);
	free(tmp);
	tmp = tmp1;
    }
    plg->chip_qe = NULL;
    plg->chip_sel = 0;
}

/*************************************************************************************************************************/
/* wybór układu */

int chip_invoke_action(chip_plugins *plg, int action)
{

    if(!__plugins__){
	printf("chip_invoke_action() called without initialisation (see chip_init_qe()) \n");
	return 0;
    }

    if(!plg->chip_sel) return 0;
//    switch(action){
//	case ACTION_READ	: plg->chip_sel->read_chip(NULL,plg); break;
//	case ACTION_READ_SIG	: plg->chip_sel->read_sig_chip(NULL,plg); break;
//	case ACTION_WRITE	: plg->chip_sel->write_chip(NULL,plg); break;
//	case ACTION_ERASE	: plg->chip_sel->erase_chip(NULL,plg); break;
//	case ACTION_LOCK	: plg->chip_sel->lock_chip(NULL,plg); break;
//	case ACTION_UNLOCK	: plg->chip_sel->unlock_chip(NULL,plg); break;
//	case ACTION_VERIFY	: plg->chip_sel->verify_chip(NULL,plg); break;
//	case ACTION_TEST	: plg->chip_sel->test_chip(NULL,plg); break;
//    }
    return 1;
}

chip_desc *chip_get_chip(chip_plugins *plg)
{ 
    if(!__plugins__){
	printf("chip_get_chip() called without initialisation (see chip_init_qe()) \n");
	return NULL;
    }

    return plg->chip_sel;
}

/**************************************************************************************************************************/
/* kolejka ściezek */

void chip_rm_path(chip_plugins *plg)
{
    chip_menu_qe *tmp,*qe = plg->menu_qe;

    if(!__plugins__){
	printf("chip_rm_path() called without initialisation (see chip_init_qe()) \n");
	return;
    }
    
    MSG_1DEBUG("{chip.c} MENU: Kasowanie kolejki...\n");
    while(qe){
	tmp = (chip_menu_qe*)qe->next;	free(qe->name);	free(qe); qe = (chip_menu_qe*)tmp;
    }
    MSG_1DEBUG("{chip.c} MENU: OK\n");
    plg->menu_qe = NULL;
}

int chip_add_path(chip_plugins *plg, char *path, void *wg)
{
    chip_menu_qe *new_tie, *tmp;

    if(!__plugins__){
	printf("chip_add_path() called without initialisation (see chip_init_qe()) \n");
	return CHIP_ERROR;
    }
    
    if(!(path && wg)){ 
	printf("[!!] chip_add_path(plugin, path, widget) -> (path || widget) == NULL !!!\n"); 
	return CHIP_ERROR; 
    }

    MSG_2DEBUG("{chip.c} MENU: add queue -> %s\n", path);
    if(!(new_tie = (chip_menu_qe *)malloc(sizeof(chip_menu_qe)))){
	printf("[!!] chip_add_path(plugin, path, widget) -> memory allocation error (1)\n"); 
	return CHIP_ERROR;
    }

    if(!(new_tie->name = (char *)malloc(sizeof(char) * strlen(path) + 1 ))){
	printf("[!!] chip_add_path(plugin, path, widget) -> memory allocation error (2)\n"); 
	free(new_tie);
	return CHIP_ERROR;
    }

    strcpy(new_tie->name, path);
    new_tie->wg = wg;
    new_tie->next = NULL;

    if(!plg->menu_qe){
	plg->menu_qe = new_tie;
	return 0;
    }

    for(tmp = plg->menu_qe; tmp->next; tmp = (chip_menu_qe *)tmp->next);
    tmp->next = new_tie;

    return 0;
}

/* zwraca wskaznik dla sciezki, lub NULL jesli sciezki nie ma */
void *chip_find_path(chip_plugins *plg, char *path)
{
    chip_menu_qe *tmp;

    if(!__plugins__){
	printf("chip_find_path() called without initialisation (see chip_init_qe()) \n");
	return NULL;
    }

    MSG_2DEBUG("{chip.c} MENU: xxx-> looking for path=%s\n", path);
    for(tmp = plg->menu_qe; tmp; tmp = (chip_menu_qe *)tmp->next){
	MSG_2DEBUG("--->%s\n", tmp->name);
	if(!strcmp(path, tmp->name)) {
	    if(!tmp->wg){
		printf("ERROR: chip_find_path() --> widget == NULL !!!\n");
		exit(-1);
	    }
	    MSG_1DEBUG("{chip.c} MENU: ------> seq present\n <-------");	
	    return tmp->wg; 
	    }
    }
    MSG_1DEBUG("{chip.c} MENU: ------> seq absent <------\n");
    return NULL;    
}

/*************************************************************************************************************************/
/* inicjowanie i niszczenie kolejki */
void chip_init_qe(chip_plugins *plg)
{
    plg->menu_qe = NULL;
    plg->chip_qe  = NULL;
    plg->chip_sel = NULL;
    plg->mdl = (modules *)malloc(sizeof(modules));
    __plugins__=1;
}

void chip_rmv_qe(chip_plugins *plg)
{
    chip_rm_path(plg);
    chip_destroy(plg);
    if(plg->mdl) free(plg->mdl);
    __plugins__ = 0;    
}

/*************************************************************************************************************************/
/* operacje na lancuchach */

char chip_cmp(char *name1, char *name2){
    char *tmp1,*tmp2;
    int  a,b;
    
    a = strlen(name1);
    b = strlen(name2);
    tmp1 = a > b ? name2 : name1;
    tmp2 = a > b ? name1 : name2;    
    a = a > b ? b : a;

    for(;a > 0; a--){
	if(*(tmp1 + a) > *(tmp2 + a)) return 1;
    }

    return 0;
}

char *chip_last_pth(char *pth)
{
    for(pth += strlen(pth); *pth!='/' ; pth--);
    return pth + 1;
}

/*************************************************************************************************************************/
/* tworzenie menu */

void chip_menu_create(chip_plugins *plg, void *wg, void *(*submenu)(void *, char *, void *), void (*item)(chip_plugins *, void *, void *), void *ptr)
{
    chip_desc *chp;
    char *tmp, t;
    void *p,*op;

    chip_add_path(plg, (char *)"/", wg);
    for(chp = plg->chip_qe; chp; chp = chp->next ){
	tmp = chp->chip_path + 1;
	op = chip_find_path(plg, (char *)"/");
	for(;*tmp; tmp++){
	    for(; *tmp!='/' && *tmp; tmp++);
	    t = *tmp;
	    *tmp = 0;
	    if(!(p = chip_find_path(plg, chp->chip_path))){
		p = submenu(op, chip_last_pth(chp->chip_path), ptr);
		chip_add_path(plg, chp->chip_path, p);
	    }
	    op = p;
	    *tmp = t;
	}
	if((p = chip_find_path(plg, chp->chip_path))){
	    plg->menu_sel = chp;
	    item(plg, p, ptr);
	}
    }
}

/**********************************************************************/
/* Identyfikacja sygnatur - do poprawy */

const char *take_signature_name(int data)
{
//    char b0 = (data >>  0) & 0xff;
    char b1 = (data >>  8) & 0xff;
    char b2 = (data >> 16) & 0xff;
    const char *text = "Unknown signature";
    switch(data & 0xff){
	case 0x1e : switch(b1){
                      case 0x51: text = (b2 == 5) ? "Vendor: ATMEL\nCHIP: AT89C51\nVPP = 5V" : "Vendor: ATMEL\nCHIP: AT89C51\nVPP=12.5V"; break;
                      case 0x52: text = (b2 == 5) ? "Vendor: ATMEL\nCHIP: AT89C52\nVPP = 5V" : "Vendor: ATMEL\nCHIP: AT89C52\nVPP=12.5V"; break;
                      case 0x21: text = (b2 == 5) ? "Vendor: ATMEL\nCHIP: AT89C2051\nVPP = 5V" : "Vendor: ATMEL\nCHIP: AT89C2051\nVPP=12.5V"; break;
                      default:   text = "Vendor: ATMEL"; break;
    		    }
		    break;
    }
    switch(data){
	case 0x01901e: text= "Vendor: ATMEL\nCHIP: AT90S1200\nFLASH: 1kB";    
	case 0x01911e: text= "Vendor: ATMEL\nCHIP: AT90S2313\nFLASH: 2kB";    
    }
    return text;
}

char chip_id_match(char *m, unsigned int id)
{
    unsigned int x;
    char *t;
    
    // skip white characters
    for( ;*m && *m < 33; m++); 
    for(t = m; *t && *t > ' '; t++);
    *t = 0;        

    // check m is a number
    for(t = m; *t; t++)
	if( 
	    (*t < '0' || *t > '9') &&
	    (*t < 'a' || *t > 'f') &&
	    (*t < 'A' || *t > 'F') 
	){
	    printf("signature database incorrect number: %s\n", m);
	    return 0;
	}
    // comparation to id
    x = strlen( m ) * 4; // bits
    x = ~(~0 << x); // create mask
    id &= x;
    x = strtol(m, NULL, 16) & x;
    if( x == id ) return 1;    
    return 0;
}

void chip_signature(char *buffer, const char *root, unsigned int man, unsigned int id, char *vendor, char *chip )
{
    char tmp[256], *n, *m, found;

    if(root == NULL) return;
    if( strlen(root) > 255 ) return;
    sprintf( tmp, "\n%s:", root); // token to search

    found = 0;
    for(n = buffer; *n;){
	// looking for root name
	if((n = strstr(n, tmp)) == NULL) return;
	n = strchr(n, ':');
	n++;
	// take manufacturer id    
	m = n;
	if((n = strchr(n, ':')) == NULL) return;        
	n[0] = 0;
	n++;
	if(!chip_id_match(m, man)) continue;    
	// take chip id    
	m = n;
	if((n = strchr(n, ':')) == NULL) return;        
	found = 1;
	n[0] = 0;
	n++;
	if(!chip_id_match(m, id)) continue;    
	found = 2;
	break;        
    }    
    if( found == 0 ) return;
    m = n;
    if((n = strchr(n, ':')) == NULL) return;        
    n[0] = 0;
    n++;
    strcpy(vendor, m);
    if( found < 2) return;
    m = n;
    if((n = strchr(n, ':')) == NULL) return;        
    n[0] = 0;
    n++;
    strcpy(chip, m);
// add take comment
}

void loockup_jedec_signature(const char *root, unsigned int man, unsigned int id, char *vendor, char *chip)
{
    const char *path = "./chips/signatures.sig";  // it should be automatically determined in the future 
    FILE *f;
    char *buffer;
    unsigned int bfsize;

    // initial return
    strcpy(vendor, "Unknown");    
    strcpy(chip, "Unknown");    
    
    if(!(f = fopen( path, "r" ))){
	printf("{chip.c} loockup_jedec_signature() --> can not open signature database file://%s\n", path );
	return;
    }

    fseek(f, 0L, SEEK_END);
    bfsize = ftell( f );
    fseek(f, 0L, SEEK_SET);
    if((buffer = (char *)malloc( bfsize + 1)) == NULL){
	printf("{chip.c} loockup_jedec_signature() --> memory allocation error.\n" );
	fclose(f);    	
	return;
    }

    if(fread(buffer, 1, bfsize, f) != bfsize){
	printf("{chip.c} loockup_jedec_signature() --> database file read error file://%s\n", path );
	fclose(f);
	return;
    }

    buffer[ bfsize ] = 0;
    
    fclose(f);    

    chip_signature( buffer, root, man, id, vendor, chip );
    free( buffer );    
}

