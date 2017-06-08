/* $Revision: 1.2 $ */
/* parport - user space wrapper for LPT port using ppdev  v 0.0.3
 * Copyright (C) 2007 Krzysztof Komarnicki
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

#ifndef __parport_h__
#define __parport_h__

/*#define __PARPORT_CPP_CLASS__  // - kompiluje do klasy parport */

#define PA	0	/* zapis */
#define PB	1	/* tylko odczyt */
#define PC	2	/* zapis, mo¿liwo¶æ odczytania zapisanej wartosci */
#define PP_ERROR	-1

/*************************************************************************************************************************/
/* Maski bitowe dla pinÃ³w portu */

/* dla portu PC */
#define PP_STB		1
#define PP_ALF		2
#define PP_INI		4
#define PP_DSL		8
#define PP_IRQ		16

/* dla portu PB */
#define PP_ERR		8
#define PP_ONOF		16
#define PP_PAP		32
#define PP_ACK		64
#define PP_BUSY		128

/* dla portu PA */
#define PP_D0		1
#define PP_D1		2
#define PP_D2		4
#define PP_D3		8
#define PP_D4		16
#define PP_D5		32
#define PP_D6		64
#define PP_D7		128

/* definicje wg pinów portu */
#define PP_01		PP_STB	/* PC */
#define PP_02		PP_D0	/* PA */
#define PP_03		PP_D1	/* PA */
#define PP_04		PP_D2	/* PA */
#define PP_05		PP_D3	/* PA */
#define PP_06		PP_D4	/* PA */
#define PP_07		PP_D5	/* PA */
#define PP_08		PP_D6	/* PA */
#define PP_09		PP_D7	/* PA */
#define PP_10		PP_ACK	/* PB */
#define PP_11		PP_BUSY	/* PB */
#define PP_12		PP_PAP	/* PB */
#define PP_13		PP_ONOF	/* PB */
#define PP_14		PP_ALF	/* PC */
#define PP_15		PP_ERR	/* PB */
#define PP_16		PP_INI	/* PC */
#define PP_17		PP_DSL	/* PC */

/* zmiana pojedynczych bitów dla funkcji parport_set_bit() i parport_clr_bit()*/
#define SPP_01		PC,PP_STB
#define SPP_02		PA,PP_D0
#define SPP_03		PA,PP_D1
#define SPP_04		PA,PP_D2
#define SPP_05		PA,PP_D3
#define SPP_06		PA,PP_D4
#define SPP_07		PA,PP_D5
#define SPP_08		PA,PP_D6
#define SPP_09		PA,PP_D7
#define SPP_10		PB,PP_ACK
#define SPP_11		PB,PP_BUSY
#define SPP_12		PB,PP_PAP
#define SPP_13		PB,PP_ONOF
#define SPP_14		PC,PP_ALF
#define SPP_15		PB,PP_ERR
#define SPP_16		PC,PP_INI
#define SPP_17		PC,PP_DSL

#ifndef __PARPORT_CPP_CLASS__
#define PARPORT(x)	parport_##x
#define PARPORT_M(x)	parport_##x
#define PARPORT_EM	parport_msgh_ptr

typedef void (*message_type)(int lvl, void *ptr, const char *fmt, ...);

#ifdef __cplusplus
extern "C" {
#endif

/* uchwyt wlasnej funkcji bledu */
extern message_type parport_message_handler;
extern void *parport_msgh_ptr; /* wskaznik przekazywany jako parametr ptr funkcji bledu */

/* funkcje inicjujace */
extern int parport_init(const char *path, int dev_flags);
extern int parport_cleanup(void);

/* ustawienia/odczyt portow, idx = PA,PB,PC */
extern int parport_set(unsigned char port_idx, unsigned char data);
extern int parport_get(unsigned char port_idx);
extern int parport_reset(void);

/* 
    ustawienia/odczyt bitÃ³w portu, idx = PA,PB,PC 
    w przypadku bledu zwraca PP_ERROR
*/
extern int parport_set_bit(unsigned char idx, unsigned char mask);
extern int parport_clr_bit(unsigned char idx, unsigned char mask);
/*
    wartosc zwracana 0,1 w zaleznosci od stanu bitu lub PP_ERROR
*/
extern int parport_get_bit(unsigned char idx, unsigned char mask);

#ifdef __cplusplus
}
#endif

#else
#define PARPORT(x)	parport::x
#define PARPORT_M(x)	x
#define PARPORT_EM	(void*)this
class parport
{
    private:
	unsigned char mirror[3];
	int ppdev_fd;
	int init_lvl;
    protected:
	int w_data(unsigned char);
	int w_ctrl(unsigned char);
	int r_stat(void);
    public:
/* funkcje inicjujace */
	int init(const char *path, int dev_flags);
	int cleanup(void);
/* ustawienia/odczyt portow, idx = PA,PB,PC */
	int set(unsigned char port_idx, unsigned char data);
	int get(unsigned char port_idx);
	int reset(void);
/* 
    ustawienia/odczyt bitÃ³w portu, idx = PA,PB,PC 
    w przypadku bledu zwraca PP_ERROR
*/
	int set_bit(unsigned char idx, unsigned char mask);
	int clr_bit(unsigned char idx, unsigned char mask);
/*
    wartosc zwracana 0,1 w zaleznosci od stanu bitu lub PP_ERROR
*/
	int get_bit(unsigned char idx, unsigned char mask);

	virtual void message(int lvl, void *, const char *fmt, ...);

	/* w przypadku bÅ‚Ä™du generujÄ… wyjatek const char* */
	parport(const char *dev_path, int flags);
	virtual ~parport();
};

#endif

#endif

