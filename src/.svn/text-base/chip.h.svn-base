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

#ifndef __chip_h__
#define __chip_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_MODE
    #define MSG_2DEBUG(a,b)		printf(a,b)
    #define MSG_1DEBUG(a)		printf(a)
#else
    #define MSG_2DEBUG(a,b)		
    #define MSG_1DEBUG(a)		
#endif

#define CHIP_ERROR		-1

typedef struct chip_plugins chip_plugins;

typedef struct{
//    char *mod_name;
    void *handler;
    void *next;
    int (*init_module)(chip_plugins *plg);
} mod_list;

typedef struct{
    mod_list *modl;
    mod_list *first_modl;    
} modules;

typedef struct 
{
    char *name;
    void *wg;
    void *next;
} chip_menu_qe;

typedef int (*chip_act_func)(void *);
typedef struct _chip_action chip_action;
typedef struct _chip_desc chip_desc;

struct chip_plugins
{
    chip_menu_qe *menu_qe;  /* wejście do kolejki menu */
    chip_desc *menu_sel;  /* uzywane przez gui, funkcja: device_sel() */
    chip_desc *chip_qe;
    chip_desc *chip_sel;
    modules *mdl;
};

struct _chip_action
{
    char *name;		/* nazwa ikony */
    char *tip;		/* pomoc */
    chip_act_func action;
    chip_action *next;
};

struct _chip_desc
{
    int  chip_id;
    char *chip_path;	/* chip path format (eg "/EPROMs"), visible later in menu */
    char *chip_name;	/* chip name eg 2716 */
    char *chip_family;	/* chip family name eg 27xx */

    unsigned int  dev_size;	/* allocation size for buffer */
    long checksum;
    char *buffer;		/* buffer data */

    chip_action *actions;	/* action functions */
    chip_act_func autostart;	/* autostart funkcji zdefiniowanej w pluginie dla danego ukladu po jego wyborze */
    
    chip_desc  *next;		/* pointer to next chip structure */
};

/* inicjowanie i usuwanie kolejek */
extern void chip_init_qe(chip_plugins *plg);
extern void chip_rmv_qe(chip_plugins *plg);

/* rejestrowanie/ wyrejestrowywanie ukladów */
extern int chip_register_chip(chip_plugins *plg, chip_desc *new_chip);
extern int chip_unregister_chip(chip_plugins *plg, char *name);
extern chip_desc *chip_lookup_chip(chip_plugins *plg, const char *name);
extern void chip_destroy(chip_plugins *plg);

/* pobieranie i ustawianie bieżącego układu */
extern int chip_invoke_action(chip_plugins *plg, int action);
extern chip_desc *chip_get_chip(chip_plugins *plg);

/* struktura menu układów */
extern void chip_rm_path(chip_plugins *plg);
extern int chip_add_path(chip_plugins *plg, char *path, void *wg);
extern void *chip_find_path(chip_plugins *plg, char *path);

/* operacje tekstowe */
extern char chip_cmp(char *name1, char *name2);
extern char *chip_last_pth(char *pth);

extern void chip_menu_create(chip_plugins *plg, void *wg, void *(*submenu)(void *, char *, void *), 
								    void (*item)(chip_plugins *, void *, void*), void*);
/* Akcje */
extern void chip_add_action(chip_desc *chip, const char *bt_name, const char *bt_tip, chip_act_func action);
extern int  chip_list_action(chip_desc *chip, int (*cb)(chip_desc *, chip_action *, void *ptr), void *ptr);

/* Info */
extern const char *take_signature_name(int signature); // obsolete
extern void loockup_jedec_signature( const char *root_name, unsigned int manufacturer_id, unsigned int chip_id, char *manufacturer_name, char *chip_name );

#ifdef __cplusplus
    } // extern "C"
#endif

#endif

