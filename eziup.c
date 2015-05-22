
/*  eziup.c - the graphic user interface based on IUP

    Copyright (C) 2011  "Andy Xuming" <xuming@users.sourceforge.net>

    This file is part of EZTHUMB, a utility to generate thumbnails

    EZTHUMB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EZTHUMB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//#define CSOUP_DEBUG_LOCAL	SLOG_CWORD(EZTHUMB_MOD_GUI, SLOG_LVL_WARNING)
//#define CSOUP_DEBUG_LOCAL	SLOG_CWORD(EZTHUMB_MOD_GUI, SLOG_LVL_MODULE)
#define CSOUP_DEBUG_LOCAL	SLOG_CWORD(EZTHUMB_MOD_GUI, SLOG_LVL_DEBUG)

#include "iup.h"
#include "libcsoup.h"
#include "ezthumb.h"
#include "ezgui.h"
#include "ezicon.h"
#include "id_lookup.h"

#define EZGUI_INST	"GUIEXT"
#define EZGUI_MAINKEY	"GUI"

#define EGPS_SETUP_DESCR	"120"
#define EGPS_SETUP_DROPDOWN	"200x14"


static	struct	idtbl	uir_grid[] = {
	{ 0,	CFG_PIC_AUTO }, 
	{ 0,	CFG_PIC_GRID_DIM }, 
	{ 0,	CFG_PIC_GRID_STEP },
	{ 0,	CFG_PIC_DIS_NUM }, 
	{ 0,	CFG_PIC_DIS_STEP }, 
	{ 0,	CFG_PIC_DIS_KEY }, 
	{ 0,	NULL }
};

static	struct	idtbl	uir_zoom[] = {
	{ 0,	CFG_PIC_AUTO }, 
	{ 0,	CFG_PIC_ZOOM_RATIO }, 
	{ 0,	CFG_PIC_ZOOM_DEFINE },
	{ 0,	CFG_PIC_ZOOM_SCREEN }, 
	{ 0,	NULL }
};

static	struct	idtbl	uir_format[] = {
	{ 0,	CFG_PIC_FMT_JPEG },
	{ 0,	CFG_PIC_FMT_PNG }, 
	{ 0,	CFG_PIC_FMT_GIFA },
	{ 0,	CFG_PIC_FMT_GIF }, 
	{ 0, 	NULL }
};

static	struct	idtbl	uir_outdir[] = {
	{ 0,	CFG_PIC_ODIR_CURR },
	{ 0,	CFG_PIC_ODIR_PATH },
	{ 0,	NULL }
};

static	struct	idtbl	uir_choose_font[] = {
	{ 0,	CFG_PIC_FONT_SYSTEM },
	{ 0,	CFG_PIC_FONT_BROWSE },
	{ 0,	NULL }
};

static int ezgui_create_window(EZGUI *gui);
static EZGUI *ezgui_get_global(Ihandle *any);
static int ezgui_event_window_resize(Ihandle *ih, int width, int height);
static int ezgui_event_window_show(Ihandle *ih, int state);
static int ezgui_event_window_close(Ihandle *ih);

static Ihandle *ezgui_page_main(EZGUI *gui);
static int ezgui_page_main_reset(EZGUI *gui);
static Ihandle *ezgui_page_main_workarea(EZGUI *gui);
static Ihandle *ezgui_page_main_button(EZGUI *gui);
static void *ezgui_page_main_file_append(EZGUI *gui, char *fname);
static int ezgui_list_entry(EZGUI *gui, int usize, int n, EZMEDIA *minfo);
static int ezgui_list_temporary(EZGUI *gui, int usize, int n, char *fname);
//static int ezgui_list_refresh(EZGUI *gui, int usize);
static int ezgui_event_main_workarea(Ihandle *ih, int item, char *text);
static int ezgui_event_main_dropfiles(Ihandle *ih, 
		const char* filename, int num, int x, int y);
static int ezgui_event_main_multi_select(Ihandle *ih, char *value);
static int ezgui_event_main_moused(Ihandle *ih, 
		int button, int pressed, int x, int y, char *status);
static int ezgui_event_main_add(Ihandle *ih);
static int ezgui_event_main_remove(Ihandle *ih);
static int ezgui_event_main_run(Ihandle *ih);
static char *ezgui_make_filters(char *slist);
static int ezgui_remove_item(EZGUI *gui, int idx);
static int ezgui_show_progress(EZGUI *gui, int cur, int range);
static int ezgui_notificate(void *v, int eid, long param, long opt, void *b);

static Ihandle *ezgui_page_setup(EZGUI *gui);
static int ezgui_page_setup_reset(EZGUI *gui);
static Ihandle *ezgui_page_setup_profile(EZGUI *gui);
static Ihandle *ezgui_page_setup_grid_zbox(EZGUI *gui);
static Ihandle *ezgui_page_setup_zoom_zbox(EZGUI *gui);
static Ihandle *ezgui_page_setup_media(EZGUI *gui);
static Ihandle *ezgui_page_setup_directory(EZGUI *gui);
static Ihandle *ezgui_page_setup_font(EZGUI *gui);
static Ihandle *ezgui_page_setup_output(EZGUI *gui);
static Ihandle *ezgui_page_setup_button(EZGUI *gui);
static int ezgui_event_setup_format(Ihandle *ih, char *text, int item, int);
static int ezgui_event_setup_grid(Ihandle *ih, char *text, int i, int s);
static int ezgui_event_setup_zoom(Ihandle *ih, char *text, int i, int s);
static int ezgui_event_setup_outputdir(Ihandle *ih, char *text, int i, int s);
static int ezgui_event_setup_setfont(Ihandle *ih, char *text, int i, int s);
static int ezgui_event_setup_ok(Ihandle *ih);
static int ezgui_event_setup_cancel(Ihandle *ih);

static Ihandle *ezgui_page_about(EZGUI *gui);

static Ihandle *ezgui_sview_create(EZGUI *gui, int dblck);
static int ezgui_sview_progress(Ihandle *ih, int percent);
static int ezgui_sview_active_add(Ihandle *ih, int type, Ihandle *ctrl);
static int ezgui_sview_active_remove(Ihandle *ih, int type, Ihandle *ctrl);
static int ezgui_sview_event_run(Ihandle *ih, int item, char *text);
static int ezgui_sview_event_dropfiles(Ihandle *, const char *,int, int, int);
static int ezgui_sview_event_multi_select(Ihandle *ih, char *value);
static int ezgui_sview_event_moused(Ihandle *ih, 
		int button, int pressed, int x, int y, char *status);
static int ezgui_sview_event_motion(Ihandle *ih, int x, int y, char *status);
static int ezgui_sview_add(SView *sview);
static int ezgui_sview_remove(SView *sview);
static int ezgui_sview_run(SView *sview);
static int ezgui_sview_file_append(SView *sview, char *fname);
static int ezgui_sview_file_remove(SView *sview, int idx);
static int ezgui_sview_active_update(SView *sview, int type, int num);
static int ezgui_sview_notificate(void *v, int eid, long param, long opt, void *b);


static Ihandle *xui_label(char *label, char *size, char *font);
static Ihandle *xui_text(Ihandle **xlst, char *label);
static Ihandle *xui_list_setting(Ihandle **xlst, char *label);
static int xui_list_get_idx(Ihandle *ih);
static int xui_text_get_number(Ihandle *ih);
static int xui_get_size(Ihandle *ih, char *attr, int *height);
static Ihandle *xui_text_setting(Ihandle **xtxt, char *label, char *ext);
static Ihandle *xui_text_grid(char *label, 
		Ihandle **xcol, Ihandle **xrow, char *ext);
static Ihandle *xui_button(char *prompt, Icallback ntf);
static int config_dump(void *config, char *prompt);


EZGUI *ezgui_init(EZOPT *ezopt, int *argcs, char ***argvs)
{
	EZGUI	*gui;
	char	*s;

	IupOpen(argcs, argvs);

	IupSetGlobal("SINGLEINSTANCE", "ezthumb");
	if (!IupGetGlobal("SINGLEINSTANCE")) {
		IupClose();
		return NULL;
	}

	if ((gui = smm_alloc(sizeof(EZGUI))) == NULL) {
		return NULL;
	}

	/* initialize GUI structure with parameters from command line */
	gui->magic  = EZGUI_MAGIC;
	gui->sysopt = ezopt;
	sprintf(gui->inst_id, "EZTHUMB_%p", gui);

	/* load configure from file, or create the file */
	gui->config = csc_cfg_open(SMM_CFGROOT_DESKTOP,
			"ezthumb", "ezthumb.conf", CSC_CFG_RWC);
	if (gui->config) {
		ezopt_load_config(ezopt, gui->config);
		config_dump(gui->config, "Read");
	}

	/* bind the notification function to GUI mode */
	gui->sysopt->notify = ezgui_notificate;

	/* find the index of drop down lists: the grid drop down */
	s = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_GRID);
	if (s) {
		gui->grid_idx = lookup_index_string(uir_grid, 0, s);
	} else if (ezopt->pro_grid == NULL) {
		gui->grid_idx = lookup_index_string(uir_grid, 0,
				CFG_PIC_AUTO);
	} else if (ezopt->grid_col && ezopt->grid_row) {
		gui->grid_idx = lookup_index_string(uir_grid, 0, 
				CFG_PIC_GRID_DIM);
	} else if (ezopt->grid_col && ezopt->tm_step) {
		gui->grid_idx = lookup_index_string(uir_grid, 0,
				CFG_PIC_GRID_STEP);
	} else if (!ezopt->grid_col && ezopt->grid_row) {
		gui->grid_idx = lookup_index_string(uir_grid, 0,
				CFG_PIC_DIS_NUM);
	} else if (!ezopt->grid_col && !ezopt->grid_row && ezopt->tm_step) {
		gui->grid_idx = lookup_index_string(uir_grid, 0,
				CFG_PIC_DIS_STEP);
	} else if (!ezopt->grid_col && !ezopt->grid_row && !ezopt->tm_step) {
		gui->grid_idx = lookup_index_string(uir_grid, 0,
				CFG_PIC_DIS_KEY);
	} else {
		gui->grid_idx = lookup_index_string(uir_grid, 0,
				CFG_PIC_AUTO);
	}

	/* find the index of drop down lists: the zoom drop down */
	s = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_ZOOM);
	if (s) {
		gui->zoom_idx = lookup_index_string(uir_zoom, 0, s);
	} else if (ezopt->pro_size == NULL) {
		gui->zoom_idx = lookup_index_string(uir_zoom, 0, 
				CFG_PIC_AUTO);
	} else if (ezopt->tn_facto) {
		gui->zoom_idx = lookup_index_string(uir_zoom, 0,
				CFG_PIC_ZOOM_RATIO);
	} else if (ezopt->tn_width && ezopt->tn_height) {
		gui->zoom_idx = lookup_index_string(uir_zoom, 0,
				CFG_PIC_ZOOM_DEFINE);
	} else if (ezopt->canvas_width) {
		gui->zoom_idx = lookup_index_string(uir_zoom, 0,
				CFG_PIC_ZOOM_SCREEN);
	} else {
		gui->zoom_idx = lookup_index_string(uir_zoom, 0,
				CFG_PIC_AUTO);
	}

	/* find the index of drop down lists: the duration drop down */
	gui->dfm_idx = lookup_index_idnum(id_duration_long, 0, 
			GETDURMOD(ezopt->flags));

	/* find the index of drop down lists: the file format drop down */
	if (!strcmp(ezopt->img_format, "png")) {
		gui->fmt_idx = lookup_index_string(uir_format, 0, 
				CFG_PIC_FMT_PNG);
	} else if (strcmp(ezopt->img_format, "gif")) {
		gui->fmt_idx = lookup_index_string(uir_format, 0, 
				CFG_PIC_FMT_JPEG);
	} else if (ezopt->img_quality) {
		gui->fmt_idx = lookup_index_string(uir_format, 0, 
				CFG_PIC_FMT_GIFA);
	} else {
		gui->fmt_idx = lookup_index_string(uir_format, 0, 
				CFG_PIC_FMT_GIF);
	}

	/* find the media process method */
	gui->mpm_idx = lookup_index_idnum(id_mprocess, 0, 
			EZOP_PROC(ezopt->flags));

	/* find the process to existed thumbnails */
	gui->exist_idx = lookup_index_idnum(id_existed, 0, 
			EZOP_THUMB_GET(ezopt->flags));

	/* find the thumbnail output method and path */
	gui->dir_idx = 0;
	s = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_OUTPUT_METHOD);
	if (s) {
		gui->dir_idx = lookup_index_string(uir_outdir, 0, s);
		if (!strcmp(s, CFG_PIC_ODIR_PATH)) {
			ezopt->pathout = csc_cfg_read(gui->config, 
					EZGUI_MAINKEY, CFG_KEY_OUTPUT_PATH);
		}
	}

	/* find the font choosing method */
	gui->font_idx = 0;
	s = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_FONT_METHOD);
	if (s) {
		gui->font_idx = lookup_index_string(uir_choose_font, 0, s);
	}

	/* find the extension name filter of files */
	/* the CFG_KEY_SUFFIX_FILTER has been opened in ezthumb.c before
	 * so make it readonly here. */
	s = csc_cfg_read(gui->config, NULL, CFG_KEY_SUFFIX_FILTER);
	if (s) {
		gui->filefilter = ezgui_make_filters(s);
	} else {
		gui->filefilter = ezgui_make_filters(EZ_DEF_FILTER);
	}

	/* seperate the image quality and frame rate */
	gui->tmp_jpg_qf  = 85;
	if (!csc_strcmp_list(gui->sysopt->img_format, "jpg", "jpeg", NULL)) {
		gui->tmp_jpg_qf  = gui->sysopt->img_quality;
	}
	csc_cfg_read_int(gui->config, EZGUI_MAINKEY, 
			CFG_KEY_JPG_QUALITY, &gui->tmp_jpg_qf);

	gui->tmp_gifa_fr = 1000;
	if (!strcmp(gui->sysopt->img_format, "gif") && 
			gui->sysopt->img_quality) {
		gui->tmp_gifa_fr = gui->sysopt->img_quality;
	}
	csc_cfg_read_int(gui->config, EZGUI_MAINKEY, 
			CFG_KEY_GIF_FRATE, &gui->tmp_gifa_fr);

	/* reset the chain list of list control */
	gui->list_cache = NULL;
	csc_cfg_save(gui->config);
	config_dump(gui->config, "Write");
	return gui;
}

int ezgui_run(EZGUI *gui, char *flist[], int fnum)
{
	int	i;

	ezgui_create_window(gui);

	/* filling the work area with file names from command line */
	if (fnum) {
		for (i = 0; i < fnum; i++) {
			ezgui_page_main_file_append(gui, flist[i]);
			ezgui_show_progress(gui, i, fnum);
		}
		ezgui_show_progress(gui, i, fnum);
	}

	IupMainLoop();
	return 0;
}

int ezgui_close(EZGUI *gui)
{
	if (gui) {
		IupClose();
		if (gui->filefilter) {
			smm_free(gui->filefilter);
		}
		csc_cdl_list_destroy(&gui->list_cache);
		if (gui->config) {
			csc_cfg_flush(gui->config);
			config_dump(gui->config, "Finalize");
			csc_cfg_close(gui->config);
		}
		smm_free(gui);
	}
	return 0;
}

void ezgui_version(void)
{
}


static int ezgui_create_window(EZGUI *gui)
{
	Ihandle		*tabs;
	char		*s;

	/* create the Open-File dialog in the initialize stage.
	 * so in the event, it can be popup and hide without a real destory */
	gui->dlg_open = IupFileDlg();
	IupSetAttribute(gui->dlg_open, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_open, "TITLE", "Open");
	IupSetAttribute(gui->dlg_open, "MULTIPLEFILES", "YES");
	IupSetAttribute(gui->dlg_open, "EXTFILTER", gui->filefilter);

	/* the Open-File dialog for picking an output directory */
	gui->dlg_odir = IupFileDlg();
	IupSetAttribute(gui->dlg_odir, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_odir, "TITLE", "Save to");
	IupSetAttribute(gui->dlg_odir, "DIALOGTYPE", "DIR");

	gui->dlg_font = IupFontDlg();
	IupSetAttribute(gui->dlg_font, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_font, "TITLE", "Setup Font");

	tabs = IupTabs(ezgui_page_main(gui), 
			ezgui_page_setup(gui), 
			IupVbox(IupFill(), NULL), 
			ezgui_page_about(gui), 
			NULL);
	IupSetAttribute(tabs, "TABTITLE0", "Generate");
	IupSetAttribute(tabs, "TABTITLE1", " Setup  ");
	IupSetAttribute(tabs, "TABTITLE2", "Advanced");
	IupSetAttribute(tabs, "TABTITLE3", " About  ");
	IupSetAttribute(tabs, "PADDING", "6x2");

	IupSetHandle("DLG_ICON", IupImageRGBA(320, 320, ezicon_pixbuf));
	gui->dlg_main = IupDialog(tabs);
	IupSetAttribute(gui->dlg_main, "TITLE", "Ezthumb");
	
	/* retrieve the previous window size */
	gui->win_width = gui->win_height = 0;
	csc_cfg_read_int(gui->config, EZGUI_MAINKEY, 
			CFG_KEY_WIN_WIDTH, &gui->win_width);
	csc_cfg_read_int(gui->config, EZGUI_MAINKEY,
			CFG_KEY_WIN_HEIGHT, &gui->win_height);
	if ((gui->win_width == 0) || (gui->win_height == 0)) {
		gui->win_width = 800;
		gui->win_height = 540;
	}
	sprintf(gui->win_size, "%dx%d", gui->win_width, gui->win_height);
	IupSetAttribute(gui->dlg_main, "RASTERSIZE", gui->win_size);

	/* recover the minimized placement of the window */
	s = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_WINDOWSTATE);
	if (s) {
		IupSetAttribute(gui->dlg_main, "PLACEMENT", s);
	}
	
	IupSetAttribute(gui->dlg_main, "ICON", "DLG_ICON");
	IupSetHandle(gui->inst_id, gui->dlg_main);

	/* bind the GUI structure into the current dialog so it can be accessed
	 * in its sub-controls */
	IupSetAttribute(gui->dlg_main, EZGUI_INST, (char*) gui);
	IupSetCallback(gui->dlg_main, "RESIZE_CB",
			(Icallback) ezgui_event_window_resize);
	IupSetCallback(gui->dlg_main, "SHOW_CB",
			(Icallback) ezgui_event_window_show);
	IupSetCallback(gui->dlg_main, "CLOSE_CB",
			(Icallback) ezgui_event_window_close);

	/* show the dialog window at the last location */
	s = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_WIN_POS);
	if (s == NULL) {
		IupShow(gui->dlg_main);
	} else {
		int	y = 0, x = (int) strtol(s, NULL, 0);

		if ((s = strchr(s, ',')) != NULL) {
			y = (int) strtol(s+1, NULL, 0);
		}
		IupShowXY(gui->dlg_main, x, y);
	}

	ezgui_page_setup_reset(gui);
	ezgui_page_main_reset(gui);
	return 0;
}

/* retrieve the GUI structure from the top leve dialog wedget */
static EZGUI *ezgui_get_global(Ihandle *any)
{
	Ihandle	*dlg = IupGetDialog(any);

	if (dlg) {
		return (EZGUI*) IupGetAttribute(dlg, EZGUI_INST);
	}
	return NULL;
}

static int ezgui_event_window_resize(Ihandle *ih, int width, int height)
{
	EZGUI	*gui = ezgui_get_global(ih);
	//int	csize;

	(void)height;

	if ((gui->win_dec_x == 0) && (gui->win_dec_y == 0)) {
		gui->win_dec_x = gui->win_width - width;
		gui->win_dec_y = gui->win_height - height;
	} else {
		gui->win_width = width + gui->win_dec_x;
		gui->win_height = height + gui->win_dec_y;
	}

	EDB_MODL(("EVT_RESIZE: C:%dx%d W:%dx%d D:%dx%d P:%s\n", width, height,
			gui->win_width, gui->win_height, 
			gui->win_dec_x, gui->win_dec_y, 
			IupGetAttribute(gui->dlg_main, "SCREENPOSITION")));

	if (gui->win_state != IUP_MAXIMIZE) {
		csc_cfg_write_int(gui->config, EZGUI_MAINKEY,
				CFG_KEY_WIN_WIDTH, gui->win_width);
		csc_cfg_write_int(gui->config, EZGUI_MAINKEY,
				CFG_KEY_WIN_HEIGHT, gui->win_height);
	}

	/* 20150514 find an easy way to shrink the window by setting 
	 * RASTERSIZE of the main dialog. The IUP will do all other things */
#if 1
	sprintf(gui->win_size, "%dx%d", width+gui->win_dec_x, height+gui->win_dec_y);
	IupSetAttribute(gui->dlg_main, "RASTERSIZE", gui->win_size);
#else
	/*xui_get_size(ezgui_get_global(ih)->list_fname, "RASTERSIZE", NULL);
	xui_get_size(ezgui_get_global(ih)->list_fname, "CHARSIZE", NULL);
	xui_get_size(ezgui_get_global(ih)->list_fname, "SIZE", NULL);*/

	/* find the client size of the list control of file names */
	width -= 360;	/* this magic number is the sum-up lenght of 
			   filesize/videolength/resolution/progress */
	/* convert it to the user size */
	csize = xui_get_size(gui->list_fname, "CHARSIZE", NULL);
	xui_get_size(gui->list_fname, "SIZE", NULL);
	width = width / csize * 4;
	
	/* this magic number was defined by the default size of the 
	 * list control of file names */
	if (width < 159) {
		width = 159;
	}
	EDB_MODL(("EVT_RESIZE: list control %dx%d\n", width, height));
	ezgui_list_refresh(gui, width);
#endif
	return 0;
}

static int ezgui_event_window_show(Ihandle *ih, int state)
{
	EZGUI	*gui = ezgui_get_global(ih);

#ifdef	DEBUG
	switch (state) {
	case IUP_HIDE:
		EDB_MODL(("EVT_SHOW(%d): IUP_HIDE\n", state));
		break;
	case IUP_SHOW:
		EDB_MODL(("EVT_SHOW(%d): IUP_SHOW\n", state));
		break;
	case IUP_RESTORE:
		EDB_MODL(("EVT_SHOW(%d): IUP_RESTORE\n", state));
		break;
	case IUP_MINIMIZE:
		EDB_MODL(("EVT_SHOW(%d): IUP_MINIMIZE\n", state));
		break;
	case IUP_MAXIMIZE:
		EDB_MODL(("EVT_SHOW(%d): IUP_MAXIMIZE\n", state));
		break;
	case IUP_CLOSE:
		EDB_MODL(("EVT_SHOW(%d): IUP_CLOSE\n", state));
		break;
	default:
		EDB_MODL(("EVT_SHOW(%d): unknown\n", state));
		break;
	}
#endif
	/* we don't save the MAXIMIZED status because when IUP set the 
	 * PLACEMENT of MAXIMIZED, IUP won't generate the resize event */
	gui->win_state = state;

	switch (state) {
	case IUP_MINIMIZE:
		csc_cfg_write(gui->config, EZGUI_MAINKEY,
				CFG_KEY_WINDOWSTATE, "MINIMIZED");
		break;
	case IUP_HIDE:
		break;
	/* case IUP_MAXIMIZE:
		csc_cfg_write(gui->config, EZGUI_MAINKEY,
				CFG_KEY_WINDOWSTATE, "MAXIMIZED");
		break;*/
	default:
		csc_cfg_delete_key(gui->config, EZGUI_MAINKEY, 
				CFG_KEY_WINDOWSTATE);
		break;
	}
	return 0;
}

static int ezgui_event_window_close(Ihandle *ih)
{
	EZGUI	*gui = ezgui_get_global(ih);
	char	*s = IupGetAttribute(gui->dlg_main, "SCREENPOSITION");
	
	EDB_MODL(("EVT_CLOSE: SCREENPOSITION = %s\n", s));
	csc_cfg_write(gui->config, EZGUI_MAINKEY, CFG_KEY_WIN_POS, s);
	return 0;
}


/****************************************************************************
 * Page Main 
 ****************************************************************************/
static Ihandle *ezgui_page_main(EZGUI *gui)
{
	Ihandle	*vbox, *hbox, *sbox;

	/* the progress bar of the current processing file */
	gui->prog_bar = IupProgressBar();
	IupSetAttribute(gui->prog_bar, "EXPAND", "HORIZONTAL");
	IupSetAttribute(gui->prog_bar, "DASHED", "YES");
	IupSetAttribute(gui->prog_bar, "SIZE", "x10");

	/* the progress bar of the task list */
	gui->prog_wait = IupProgressBar();
	IupSetAttribute(gui->prog_wait, "EXPAND", "HORIZONTAL");
	IupSetAttribute(gui->prog_wait, "DASHED", "YES");
	IupSetAttribute(gui->prog_wait, "SIZE", "x10");
	IupSetAttribute(gui->prog_wait, "MARQUEE", "YES");

	/* the status bar */
	gui->stat_bar = IupLabel("");
	IupSetAttribute(gui->stat_bar, "EXPAND", "HORIZONTAL");

	/* status bar and progress bars share the same conner. normally it
	 * display the status until in the running mode */
	gui->ps_zbox = IupZbox(gui->stat_bar, gui->prog_bar, 
			gui->prog_wait, NULL);
	IupSetAttribute(gui->ps_zbox, "ALIGNMENT", "ACENTER");

	/* progres bar and buttons are in the same bottom line */
	hbox = IupHbox(gui->ps_zbox, ezgui_page_main_button(gui), NULL);
	IupSetAttribute(hbox, "NGAP", "10");
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");

	/* grouping with the work area, a group of lists inside a scroll box */
	//sbox = IupScrollBox(ezgui_page_main_workarea(gui));
	gui->list_view = ezgui_sview_create(gui, 1);
	ezgui_sview_active_add(gui->list_view, 
			EZGUI_SVIEW_ACTIVE_CONTENT, gui->button_run);
	ezgui_sview_active_add(gui->list_view, 
			EZGUI_SVIEW_ACTIVE_SELECT, gui->button_del);
	ezgui_sview_active_add(gui->list_view, 
			EZGUI_SVIEW_ACTIVE_PROGRESS, gui->prog_bar);
	
	ezgui_sview_active_add(gui->list_view, 
			EZGUI_SVIEW_ACTIVE_CONTENT, gui->button_add);
	ezgui_sview_active_remove(gui->list_view,
			EZGUI_SVIEW_ACTIVE_CONTENT, gui->button_add);
	
	sbox = IupScrollBox(gui->list_view);
	vbox = IupVbox(sbox, hbox, NULL);
	IupSetAttribute(vbox, "NGAP", "4");
	IupSetAttribute(vbox, "NMARGIN", "4x4");
	return vbox;
}

static int ezgui_page_main_reset(EZGUI *gui)
{
	IupSetInt(gui->ps_zbox, "VALUEPOS", 0);
	IupSetAttribute(gui->button_del, "ACTIVE", "NO");
	if (gui->list_count == 0) {
		IupSetAttribute(gui->button_run, "ACTIVE", "NO");
	}
	return 0;
}

static Ihandle *ezgui_page_main_workarea(EZGUI *gui)
{
	Ihandle	*vb_main, *vb_size, *vb_len, *vb_res, *vb_prog, *hbox;

	gui->list_fname = IupList(NULL);
	IupSetAttribute(gui->list_fname, "EXPAND", "YES");
	IupSetAttribute(gui->list_fname, "MULTIPLE", "YES");
	IupSetAttribute(gui->list_fname, "SCROLLBAR", "NO");
	IupSetAttribute(gui->list_fname, "DROPFILESTARGET", "YES");
	vb_main = IupVbox(xui_label("Files", NULL, NULL), 
			gui->list_fname, NULL);
	IupSetCallback(gui->list_fname, "DBLCLICK_CB", 
			(Icallback) ezgui_event_main_workarea);
	IupSetCallback(gui->list_fname, "DROPFILES_CB",
			(Icallback) ezgui_event_main_dropfiles);
	IupSetCallback(gui->list_fname, "MULTISELECT_CB",
			(Icallback) ezgui_event_main_multi_select);
	IupSetCallback(gui->list_fname, "BUTTON_CB",
			(Icallback) ezgui_event_main_moused);

	gui->list_size = IupList(NULL);
	IupSetAttribute(gui->list_size, "SIZE", "50");
	IupSetAttribute(gui->list_size, "EXPAND", "VERTICAL");
	IupSetAttribute(gui->list_size, "SCROLLBAR", "NO");
	IupSetAttribute(gui->list_size, "ACTIVE", "NO");
	vb_size = IupVbox(xui_label("Size", "50", NULL), 
			gui->list_size, NULL);

	gui->list_length = IupList(NULL);
	IupSetAttribute(gui->list_length, "SIZE", "48");
	IupSetAttribute(gui->list_length, "EXPAND", "VERTICAL");
	IupSetAttribute(gui->list_length, "SCROLLBAR", "NO");
	IupSetAttribute(gui->list_length, "ACTIVE", "NO");
	vb_len = IupVbox(xui_label("Length", "48", NULL), 
			gui->list_length, NULL);

	gui->list_resolv = IupList(NULL);
	IupSetAttribute(gui->list_resolv, "SIZE", "50");
	IupSetAttribute(gui->list_resolv, "EXPAND", "VERTICAL");
	IupSetAttribute(gui->list_resolv, "SCROLLBAR", "NO");
	IupSetAttribute(gui->list_resolv, "ACTIVE", "NO");
	vb_res = IupVbox(xui_label("Resolution", "50", NULL), 
			gui->list_resolv, NULL);
	
	gui->list_prog = IupList(NULL);
	IupSetAttribute(gui->list_prog, "SIZE", "40");
	IupSetAttribute(gui->list_prog, "SCROLLBAR", "NO");
	IupSetAttribute(gui->list_prog, "EXPAND", "VERTICAL");
	IupSetAttribute(gui->list_prog, "ACTIVE", "NO");
	vb_prog = IupVbox(xui_label("Progress", "40", NULL), 
			gui->list_prog, NULL);

	hbox = IupHbox(vb_main, vb_size, vb_len, vb_res, vb_prog, NULL);
	return hbox;
}

static Ihandle *ezgui_page_main_button(EZGUI *gui)
{
	gui->button_add = xui_button("Add", NULL);
	IupSetCallback(gui->button_add, "ACTION",
			(Icallback) ezgui_event_main_add);
	gui->button_del = xui_button("Remove", NULL);
	IupSetCallback(gui->button_del, "ACTION",
			(Icallback) ezgui_event_main_remove);
	gui->button_run = xui_button("Run", NULL);
	IupSetCallback(gui->button_run, "ACTION",
			(Icallback) ezgui_event_main_run);
	return IupHbox(gui->button_add, gui->button_del, gui->button_run,NULL);
}

static void *ezgui_page_main_file_append(EZGUI *gui, char *fname)
{
	EZVID	vobj;
	EZMEDIA	*minfo;
	CSCLNK	*node;
	int	len, usize, lnext;

	/* preset the filename to make it look better */
	lnext = gui->list_count + 1;
	usize = xui_get_size(gui->list_fname, "SIZE", NULL);
	ezgui_list_temporary(gui, usize, lnext, fname);

	/* 20120903 Bugfix: set the codepage to utf-8 before calling
	 * ezthumb core. In Win32 version, the ezthumb core uses the 
	 * default codepage to process file name. However the GTK 
	 * converted the file name to UTF-8 so the Windows version 
	 * could not find the file. 
	 * There's no such problem in linux.*/
	smm_codepage_set(65001);
	if (ezinfo(fname, gui->sysopt, &vobj) != EZ_ERR_NONE) {
		smm_codepage_reset();
		/* FIXME: disaster control */
		IupSetAttributeId(gui->list_fname,  "", lnext, "");
		return NULL;
	}
	smm_codepage_reset();

	len = strlen(fname) * 2 + sizeof(EZMEDIA) + 8;
	if ((node = csc_cdl_list_alloc_tail(&gui->list_cache, len)) == NULL) {
		IupSetAttributeId(gui->list_fname,  "", lnext, "");
		return NULL;
	}
	minfo = (EZMEDIA *) csc_cdl_payload(node);

	/* highlight the RUN button when the list is not empty */
	IupSetAttribute(gui->button_run, "ACTIVE", "YES");

	strcpy(minfo->fname, fname);
	minfo->showing = minfo->fname + strlen(minfo->fname) + 1;
	minfo->duration = vobj.duration;
	minfo->seekable = vobj.seekable;
	minfo->bitrates = vobj.bitrates;

	meta_timestamp(vobj.duration, 0, minfo->vidlen);
	meta_filesize(vobj.filesize, minfo->fsize);
	sprintf(minfo->resolv, "%dx%d", vobj.width, vobj.height);
	sprintf(minfo->progr, "0%%");

	//IupSetAttributeId(gui->list_fname,  "", lnext, minfo->fname);
	ezgui_list_entry(gui, usize, lnext, minfo);
	IupSetAttributeId(gui->list_size,   "", lnext, minfo->fsize);
	IupSetAttributeId(gui->list_length, "", lnext, minfo->vidlen);
	IupSetAttributeId(gui->list_resolv, "", lnext, minfo->resolv);
	IupSetAttributeId(gui->list_prog,   "", lnext, minfo->progr);

	/* increase the list index first because the IUP list control
	 * starts from 1 */
	gui->list_count++;

	IupFlush();
	/*if (gui->dlg_main) {
		IupRedraw(gui->dlg_main, 1);
	}*/

	return minfo;
}

static int ezgui_list_entry(EZGUI *gui, int usize, int n, EZMEDIA *minfo)
{
	int	len;

	usize = usize / 4 - 3;
	len = strlen(minfo->fname);

	if (len < usize) {
		strcpy(minfo->showing, minfo->fname);
	} else {
		strcpy(minfo->showing, "... ");
		strcat(minfo->showing, &minfo->fname[len - usize + 4]);
	}

	IupSetAttributeId(gui->list_fname, "",  n, minfo->showing);
	return 0;
}

static int ezgui_list_temporary(EZGUI *gui, int usize, int n, char *fname)
{
	int	len;

	usize = usize / 4 - 3;
	len = strlen(fname);
	if (len > usize) {
		fname += len - usize + 3;
	}
	IupSetAttributeId(gui->list_fname, "",  n, fname);
	return 0;
}

/*static int ezgui_list_refresh(EZGUI *gui, int usize)
{
	CSCLNK	*node;
	int	i;

	for (i = 1, node = gui->list_cache; node; 
			i++, node = csc_cdl_next(gui->list_cache, node)) {
		ezgui_list_entry(gui, usize, i, (EZMEDIA*) csc_cdl_payload(node));
	}
	return 0;
}*/

static int ezgui_event_main_workarea(Ihandle *ih, int item, char *text)
{
	EZGUI	*gui = (EZGUI *) ih;
	EZMEDIA	*minfo = NULL;
	CSCLNK	*node;

	(void)text;
	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	EDB_DEBUG(("EVT_Action %d: %p %s\n", item, ih, text));
	gui->list_idx = item;	/* store the current index */

	node = csc_cdl_goto(gui->list_cache, item - 1);
	if (node) {
		minfo = (EZMEDIA*) csc_cdl_payload(node);
		gui->sysopt->pre_dura = minfo->duration;
		gui->sysopt->pre_seek = minfo->seekable;
		gui->sysopt->pre_br = minfo->bitrates;
	
		smm_codepage_set(65001);
		ezthumb(minfo->fname, gui->sysopt);
		smm_codepage_reset();

		gui->sysopt->pre_dura = 0;
		gui->sysopt->pre_seek = 0;
		gui->sysopt->pre_br = 0;
	}
	return IUP_DEFAULT;
}

static int ezgui_event_main_dropfiles(Ihandle *ih, 
		const char* filename, int num, int x, int y)
{
	EZGUI	*gui = (EZGUI *) ih;

	(void)num; (void)x; (void)y;

	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	EDB_DEBUG(("EVT_Dropfiles: fname=%s number=%d %dx%d\n",
				filename, num, x, y));
	ezgui_page_main_file_append(gui, (char*) filename);
	return IUP_DEFAULT;
}

static int ezgui_event_main_multi_select(Ihandle *ih, char *value)
{
	EZGUI	*gui = (EZGUI *) ih;
	int	i;

	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	EDB_DEBUG(("EVT_Multi_select: %s\n", value));
	EDB_DEBUG(("List value=%s\n", 
				IupGetAttribute(gui->list_fname, "VALUE")));
	
	/* the parameter 'value' is useless. grab list myself */
	value = IupGetAttribute(gui->list_fname, "VALUE");
	for (i = 0; value[i]; i++) {
		if (value[i] == '+') {
			IupSetAttribute(gui->button_del, "ACTIVE", "YES");
			return IUP_DEFAULT;
		}
	}
	IupSetAttribute(gui->button_del, "ACTIVE", "NO");
	return IUP_DEFAULT;
}

static int ezgui_event_main_moused(Ihandle *ih, 
		int button, int pressed, int x, int y, char *status)
{
	EZGUI	*gui = (EZGUI *) ih;

	(void)x; (void)y; (void)status;	/* stop compiler complains */

	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	EDB_MODL(("EVT_Mouse: %d %d %dx%d %s\n", 
				button, pressed, x, y, status));
	if (pressed) {	/* only act when button is released */
		return IUP_DEFAULT;
	}
	//line = IupConvertXYToPos(ih, x, y);
	/* deselect every thing if the right button was released */
	if (button == IUP_BUTTON3) {
		IupSetAttribute(gui->list_fname, "VALUE", "");
		IupSetAttribute(gui->button_del, "ACTIVE", "NO");
	}
	return IUP_DEFAULT;
}

static int ezgui_event_main_add(Ihandle *ih)
{
	SView	*sview;

	if ((sview = (SView*)IupGetAttribute(ih, "SVIEWOBJ")) != NULL) {
		ezgui_sview_add(sview);
	}
	return IUP_DEFAULT;
#if 0
	EZGUI	*gui = (EZGUI *) ih;
	char	*flist, *fname, *path, *sdir;
	int	i, amnt;

	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}
	
	/* Store the recent visited diretory so it can be used next time.
	 * The file open dialog can not go to the last directory in gtk */
	path = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_DIRECTORY);
	if (path) {
		IupSetAttribute(gui->dlg_open, "DIRECTORY", path);
	}
	IupPopup(gui->dlg_open, IUP_CENTERPARENT, IUP_CENTERPARENT);

	if (IupGetInt(gui->dlg_open, "STATUS") < 0) {
		return IUP_DEFAULT;	/* cancelled */
	}

	/* IUP generate different file list between one file and more files:
	 * Open File VALUE: /home/xum1/dwhelper/lan_ke_er.flv
	 * Last  DIRECTORY: /home/xum1/dwhelper/
	 * Open File VALUE: /home/xum1/dwhelper|file-602303262.flv|
	 * 			lan_ke_er.flv|Powered_by_Discuz.flv|
	 * Last  DIRECTORY: /home/xum1/dwhelper0 */
	EDB_DEBUG(("Open File VALUE: %s\n", 
			IupGetAttribute(gui->dlg_open, "VALUE")));
	EDB_DEBUG(("Last  DIRECTORY: %s\n", 
			IupGetAttribute(gui->dlg_open, "DIRECTORY")));
	/* duplicate the path and filename list */
	flist = csc_strcpy_alloc(IupGetAttribute(gui->dlg_open, "VALUE"), 16);
	/* find out how many files is in */
	for (i = amnt = 0; flist[i]; i++) {
		amnt += (flist[i] == '|') ? 1 : 0;
	}
	if (amnt == 0) {
		amnt++;
	} else {
		amnt--;
	}
	ezgui_show_progress(gui, 0, amnt);

	/* store the current directory. note that the tailing '/' or '0' 
	 * need to be cut out first.
	 * Is the tailing '0' a bug of IUP? */
	path = csc_strcpy_alloc(
			IupGetAttribute(gui->dlg_open, "DIRECTORY"), 4);
	i = strlen(path) - 1;
	if ((path[i] == '/') || (path[i] == '\\') || (path[i] == '0')) {
		path[i] = 0;
	}
	csc_cfg_write(gui->config, EZGUI_MAINKEY, CFG_KEY_DIRECTORY, path);
	smm_free(path);

	/* process the single file list */
	if (amnt == 1) {
		ezgui_page_main_file_append(gui, flist);
		ezgui_show_progress(gui, amnt, amnt);
		smm_free(flist);
		return IUP_DEFAULT;
	}

	/* cut out the path first */
	flist = csc_cuttoken(flist, &sdir, "|");
	/* extract the file names */
	i = 0;
	while ((flist = csc_cuttoken(flist, &fname, "|")) != NULL) {
		if (fname[0] == 0) {
			break;	/* end of list */
		}
		path = csc_strcpy_alloc(sdir, strlen(fname)+8);
		strcat(path, "/");
		strcat(path, fname);
		//printf("%s\n", path);
		ezgui_page_main_file_append(gui, path);
		ezgui_show_progress(gui, ++i, amnt);
		smm_free(path);
	}
	if (i < amnt) {
		ezgui_show_progress(gui, amnt, amnt);
	}
	smm_free(flist);
	return IUP_DEFAULT;
#endif
}

static int ezgui_event_main_remove(Ihandle *ih)
{
	SView	*sview;

	if ((sview = (SView*)IupGetAttribute(ih, "SVIEWOBJ")) != NULL) {
		ezgui_sview_remove(sview);
	}
	return IUP_DEFAULT;
#if 0
	EZGUI	*gui = (EZGUI *) ih;
	char	*value;
	int	i;

	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	while (1) {
		value = IupGetAttribute(gui->list_fname, "VALUE");
		EDB_PROG(("EVT_Remove: %s\n", value));
		for (i = 0; value[i]; i++) {
			if (value[i] == '+') {
				ezgui_remove_item(gui, i+1);
				break;
			}
		}
		if (!value[i]) {
			break;
		}
	}
	IupSetAttribute(gui->button_del, "ACTIVE", "NO");
	if (gui->list_count == 0) {
		IupSetAttribute(gui->button_run, "ACTIVE", "NO");
	}
	return IUP_DEFAULT;
#endif
}

static int ezgui_event_main_run(Ihandle *ih)
{
	SView	*sview;

	if ((sview = (SView*)IupGetAttribute(ih, "SVIEWOBJ")) != NULL) {
		ezgui_sview_run(sview);
	}
	return IUP_DEFAULT;
#if 0
	EZGUI	*gui = (EZGUI *) ih;
	char	*value, *fname;
	int	i, n;

	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	value = IupGetAttribute(gui->list_fname, "VALUE");
	for (i = n = 0; value[i]; i++) {
		if (value[i] == '+') {
			fname = IupGetAttributeId(gui->list_fname, "",  i+1);
			ezgui_event_main_workarea((Ihandle*)gui, i+1, fname);
			n++;
		}
	}
	if (n == 0) {
		for (i = 0; i < gui->list_count; i++) {
			fname = IupGetAttributeId(gui->list_fname, "",  i+1);
			ezgui_event_main_workarea((Ihandle*)gui, i+1, fname);
		}
	}
	return IUP_DEFAULT;
#endif
}

/* expand the short form extension list to the full length */
static char *ezgui_make_filters(char *slist)
{
	char	*flt, token[32];
	int	n;

	/* Assuming the worst case of the short list is like "a;b;c;d",
	 * it will then be expanded to "*.a;*.b;*.c;*.d". The biggest length 
	 * is N + (N/2+1) * 2 */
	if ((flt = smm_alloc(strlen(slist) * 2 + 64)) == NULL) {
		return NULL;
	}

	strcpy(flt, "Video Files|");
	n = strlen(flt);
	while ((slist = csc_gettoken(slist, token, ",;:")) != NULL) {
		n += sprintf(flt + n, "*.%s;", token);
	}
	flt[strlen(flt)-1] = 0;		/* cut out the tailing ';' */
	strcat(flt, "|All Files|*.*|");
	return flt;
}

static int ezgui_remove_item(EZGUI *gui, int idx)
{
	CSCLNK	*node;
	char	*fname;

	fname = IupGetAttributeId(gui->list_fname, "",  idx);
	EDB_INFO(("Removing: %s\n", fname));

	IupSetInt(gui->list_fname, "REMOVEITEM", idx);
	IupSetInt(gui->list_size, "REMOVEITEM", idx);
	IupSetInt(gui->list_length, "REMOVEITEM", idx);
	IupSetInt(gui->list_resolv, "REMOVEITEM", idx);
	IupSetInt(gui->list_prog, "REMOVEITEM", idx);
	gui->list_count--;

	if ((node = csc_cdl_goto(gui->list_cache, idx - 1)) != NULL) {
		csc_cdl_list_free(&gui->list_cache, node);
	}
	return 0;
}

static int ezgui_show_progress(EZGUI *gui, int cur, int range)
{
	static	int	zpos;

	if (cur == 0) {		/* begin to display progress */
		zpos = IupGetInt(gui->ps_zbox, "VALUEPOS");
		IupSetInt(gui->prog_bar, "MIN", 0);
		//IupSetInt(gui->prog_bar, "MAX", range);
		IupSetInt(gui->prog_bar, "VALUE", 0);
		IupSetInt(gui->ps_zbox, "VALUEPOS", 1);	/* show progress */
	} else if (cur == range) {	/* end of display */
		IupSetInt(gui->prog_bar, "VALUE", range);
		IupFlush();
		//smm_sleep(0, 500000);
		IupSetInt(gui->ps_zbox, "VALUEPOS", zpos);
	} else if (cur < range) {
		IupSetInt(gui->prog_bar, "MAX", range);
		IupSetInt(gui->prog_bar, "VALUE", cur);
	}
	IupFlush();
	return 0;
}

static int ezgui_show_duration(EZGUI *gui, int state)
{
	static	SMM_TIME	last;
	static	int		zpos;

	if (state == EN_OPEN_BEGIN) {
		zpos = IupGetInt(gui->ps_zbox, "VALUEPOS");
		smm_time_get_epoch(&last);
		IupSetInt(gui->prog_wait, "VALUE", 0);
		IupSetInt(gui->ps_zbox, "VALUEPOS", 2);	/* show progress */
	} else if (state == EN_OPEN_END) {
		IupSetInt(gui->ps_zbox, "VALUEPOS", zpos);
	} else if (smm_time_diff(&last) > 50) {
		/* update the progress bar per 50ms */
		smm_time_get_epoch(&last);
		IupSetInt(gui->prog_wait, "VALUE", 1);
	}
	IupFlush();
	return 0;
}

static int ezgui_notificate(void *v, int eid, long param, long opt, void *b)
{
	EZGUI	*gui = ((EZOPT*) v)->gui;
	//EZVID	*vidx = ((EZOPT*) v)->vidobj;
	EZMEDIA	*minfo = NULL;
	CSCLNK	*node;

	(void)b;
	switch (eid) {
	case EN_PROC_BEGIN:
		ezgui_show_progress(gui, 0, 0);	/* show/reset progress bar */
		break;
	case EN_PROC_CURRENT:
		node = csc_cdl_goto(gui->list_cache, gui->list_idx - 1);
		if (node) {
			minfo = (EZMEDIA*) csc_cdl_payload(node);
			sprintf(minfo->progr, "%d%%", (int)(opt*100/param));
			IupSetAttributeId(gui->list_prog, "", 
					gui->list_idx, minfo->progr);
		}
		ezgui_show_progress(gui, opt, param);
		break;
	case EN_PROC_END:
		node = csc_cdl_goto(gui->list_cache, gui->list_idx - 1);
		if (node) {
			minfo = (EZMEDIA*) csc_cdl_payload(node);
			strcpy(minfo->progr, "100%");
			IupSetAttributeId(gui->list_prog, "", 
					gui->list_idx, minfo->progr);
		}
		ezgui_show_progress(gui, param, param);
		break;
	case EN_OPEN_BEGIN:
	case EN_OPEN_GOING:
	case EN_OPEN_END:
		ezgui_show_duration(gui, eid);
		break;
	default:
		return EN_EVENT_PASSTHROUGH;
	}
	return eid;
}


/****************************************************************************
 * Page Setup 
 ****************************************************************************/
static Ihandle *ezgui_page_setup(EZGUI *gui)
{
	Ihandle	*vbox, *sbox;

	sbox = IupVbox(xui_label("Profile Selection", NULL, "Bold"), 
			ezgui_page_setup_profile(gui), 
			xui_label("Media Processing", NULL, "Bold"), 
			ezgui_page_setup_media(gui), 
			xui_label("Font", NULL, "Bold"),
			ezgui_page_setup_font(gui),
			xui_label("Output Directory", NULL, "Bold"),
			ezgui_page_setup_directory(gui),
			xui_label("Output Thumbnails", NULL, "Bold"), 
			ezgui_page_setup_output(gui),
			NULL);
	IupSetAttribute(sbox, "NGAP", "8");
	IupSetAttribute(sbox, "NMARGIN", "16x16");
	
	sbox = IupScrollBox(sbox);
	IupSetAttribute(sbox, "SCROLLBAR", "VERTICAL");

	vbox = IupVbox(sbox, ezgui_page_setup_button(gui), NULL);
	IupSetAttribute(vbox, "NGAP", "4");
	IupSetAttribute(vbox, "NMARGIN", "4x4");
	return vbox;
}

static int ezgui_page_setup_reset(EZGUI *gui)
{
	char	*s;

	IupSetInt(gui->prof_grid, "VALUE", gui->grid_idx + 1);
	IupSetInt(gui->prof_zoom, "VALUE", gui->zoom_idx + 1);
	IupSetInt(gui->dfm_list, "VALUE", gui->dfm_idx + 1);
	IupSetInt(gui->mpm_list, "VALUE", gui->mpm_idx + 1);
	IupSetInt(gui->fmt_list, "VALUE", gui->fmt_idx + 1);
	IupSetInt(gui->fmt_exist, "VALUE", gui->exist_idx + 1);

	IupSetInt(gui->dir_list, "VALUE", gui->dir_idx + 1);
	if (gui->sysopt->pathout) {
		IupSetAttribute(gui->dir_path, "VALUE", gui->sysopt->pathout);
	}
	if (!strcmp(uir_outdir[gui->dir_idx].s, CFG_PIC_ODIR_PATH)) {
		IupSetAttribute(gui->dir_path, "VISIBLE", "YES");
	} else {
		IupSetAttribute(gui->dir_path, "VISIBLE", "NO");
	}

	IupSetInt(gui->font_list, "VALUE", gui->font_idx + 1);
	s = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_FONT_FACE);
	if (s) {
		IupSetAttribute(gui->font_face, "VALUE", s);
	}
	if (!strcmp(uir_choose_font[gui->font_idx].s, CFG_PIC_FONT_BROWSE)) {
		IupSetAttribute(gui->font_face, "VISIBLE", "YES");
	} else {
		IupSetAttribute(gui->font_face, "VISIBLE", "NO");
	}

	IupSetInt(gui->fmt_gif_fr, "VALUE", gui->tmp_gifa_fr);
	IupSetInt(gui->fmt_jpg_qf, "VALUE", gui->tmp_jpg_qf);
	IupSetAttribute(gui->fmt_suffix, "VALUE", gui->sysopt->suffix);

	if (gui->sysopt->flags & EZOP_TRANSPARENT) {
		IupSetAttribute(gui->fmt_transp, "VALUE", "ON");
	} else {
		IupSetAttribute(gui->fmt_transp, "VALUE", "OFF");
	}

	IupSetInt(gui->entry_col_grid, "VALUE", gui->sysopt->grid_col);
	IupSetInt(gui->entry_col_step, "VALUE", gui->sysopt->grid_col);
	IupSetInt(gui->entry_row, "VALUE", gui->sysopt->grid_row);
	IupSetInt(gui->entry_step, "VALUE", gui->sysopt->tm_step / 1000);
	IupSetInt(gui->entry_dss_amnt, "VALUE", gui->sysopt->grid_row);
	IupSetInt(gui->entry_dss_step, "VALUE", gui->sysopt->tm_step / 1000);
	IupSetInt(gui->entry_zoom_ratio, "VALUE", gui->sysopt->tn_facto);
	IupSetInt(gui->entry_zoom_wid, "VALUE", gui->sysopt->tn_width);
	IupSetInt(gui->entry_zoom_hei, "VALUE", gui->sysopt->tn_height);
	IupSetInt(gui->entry_width, "VALUE", gui->sysopt->canvas_width);

	ezgui_event_setup_format((Ihandle*) gui, 
			uir_format[gui->fmt_idx].s, gui->fmt_idx + 1, 1);

	ezgui_event_setup_ok((Ihandle*) gui);
	return 0;
}

static Ihandle *ezgui_page_setup_profile(EZGUI *gui)
{
	Ihandle	*hbox1, *hbox2, *vbox;
	int	i;

	/* First line: Grid Setting */
	hbox1 = xui_list_setting(&gui->prof_grid, "Grid Setting");
	for (i = 0; uir_grid[i].s; i++) {
		IupSetAttributeId(gui->prof_grid, "", i + 1, 
				uir_grid[i].s);
	}
	IupSetCallback(gui->prof_grid, "ACTION",
			(Icallback) ezgui_event_setup_grid);
	gui->entry_zbox_grid = ezgui_page_setup_grid_zbox(gui);
	IupAppend(hbox1, gui->entry_zbox_grid);

	/* Second line: Zoom Setting */
	hbox2 = xui_list_setting(&gui->prof_zoom, "Zoom Setting");
	for (i = 0; uir_zoom[i].s; i++) {
		IupSetAttributeId(gui->prof_zoom, "", i + 1, 
				uir_zoom[i].s);
	}
	IupSetCallback(gui->prof_zoom, "ACTION",
			(Icallback) ezgui_event_setup_zoom);
	gui->entry_zbox_zoom = ezgui_page_setup_zoom_zbox(gui);
	IupAppend(hbox2, gui->entry_zbox_zoom);

	/* assemble the Profile Setting area */
	vbox = IupVbox(hbox1, hbox2, NULL);
	IupSetAttribute(vbox, "NMARGIN", "16x4");
	return vbox;
}

static Ihandle *ezgui_page_setup_grid_zbox(EZGUI *gui)
{
	Ihandle	*hbox, *zbox;

	zbox = IupZbox(IupFill(), NULL);

	hbox = xui_text_grid("Grid Of", 
			&gui->entry_col_grid, &gui->entry_row, NULL);
	IupAppend(zbox, hbox);
	
	hbox = xui_text_grid("Column", 
			&gui->entry_col_step, &gui->entry_step, "(s)");
	IupAppend(zbox, hbox);

	hbox = xui_text_grid("Total", &gui->entry_dss_amnt, NULL, NULL);
	IupAppend(zbox, hbox);

	hbox = xui_text_grid("Every", &gui->entry_dss_step, NULL, "(s) ");
	IupAppend(zbox, hbox);

	IupAppend(zbox, IupFill());
	return zbox;
}

static Ihandle *ezgui_page_setup_zoom_zbox(EZGUI *gui)
{
	Ihandle	*hbox, *zbox;

	zbox = IupZbox(IupFill(), NULL);

	hbox = xui_text_grid("Ratio", &gui->entry_zoom_ratio, NULL, "%");
	IupSetAttribute(gui->entry_zoom_ratio, "SIZE", "24x11");
	IupSetAttribute(gui->entry_zoom_ratio, "SPIN", "YES");
	IupSetAttribute(gui->entry_zoom_ratio, "SPINMIN", "5");
	IupSetAttribute(gui->entry_zoom_ratio, "SPINMAX", "200");
	IupSetAttribute(gui->entry_zoom_ratio, "SPININC", "5");
	IupSetAttribute(gui->entry_zoom_ratio, "SPINALIGN", "LEFT");
	IupSetAttribute(gui->entry_zoom_ratio, "SPINVALUE", "50");
	IupAppend(zbox, hbox);

	hbox = xui_text_grid("Resolu", 
			&gui->entry_zoom_wid, &gui->entry_zoom_hei, NULL);
	IupAppend(zbox, hbox);
	
	hbox = xui_text_grid("Width", &gui->entry_width, NULL, NULL);
	IupAppend(zbox, hbox);
	return zbox;
}


static Ihandle *ezgui_page_setup_media(EZGUI *gui)
{
	Ihandle	*hbox1, *hbox2, *vbox;
	int	i;

	hbox1 = xui_list_setting(&gui->dfm_list, "Find Media Duration By");
	for (i = 0; id_duration_long[i].s; i++) {
		IupSetAttributeId(gui->dfm_list, "", i + 1, 
				id_duration_long[i].s);
	}
	//IupSetAttribute(hbox, "NMARGIN", "16x4");
	
	hbox2 = xui_list_setting(&gui->mpm_list, "Media Process Method");
	for (i = 0; id_mprocess[i].s; i++) {
		IupSetAttributeId(gui->mpm_list, "", i + 1, id_mprocess[i].s);
	}

	vbox = IupVbox(hbox1, hbox2, NULL);
	IupSetAttribute(vbox, "NMARGIN", "16x4");
	IupSetAttribute(vbox, "NGAP", "4");
	return vbox;
}

static Ihandle *ezgui_page_setup_directory(EZGUI *gui)
{
	Ihandle	*hbox1, *hbox2, *vbox;
	int	i;

	hbox1 = xui_list_setting(&gui->dir_list, "Save Thumbnails ");
	for (i = 0; uir_outdir[i].s; i++) {
		IupSetAttributeId(gui->dir_list, "", i + 1, uir_outdir[i].s);
	}
	IupSetCallback(gui->dir_list, "ACTION",
			(Icallback) ezgui_event_setup_outputdir);

	hbox2 = xui_text(&gui->dir_path, "");
	IupSetAttribute(gui->dir_path, "VISIBLE", "NO");

	vbox = IupVbox(hbox1,  hbox2, NULL);
	IupSetAttribute(vbox, "NMARGIN", "16x4");
	IupSetAttribute(vbox, "NGAP", "4");
	return vbox;
}

static Ihandle *ezgui_page_setup_font(EZGUI *gui)
{
	Ihandle	*hbox1, *hbox2, *vbox;
	int	i;

	hbox1 = xui_list_setting(&gui->font_list, "Choose Font");
	for (i = 0; uir_choose_font[i].s; i++) {
		IupSetAttributeId(gui->font_list, "", i + 1, 
				uir_choose_font[i].s);
	}
	IupSetCallback(gui->font_list, "ACTION",
			(Icallback) ezgui_event_setup_setfont);

	hbox2 = xui_text(&gui->font_face, "");
	IupSetAttribute(gui->font_face, "READONLY", "YES");

	vbox = IupVbox(hbox1,  hbox2, NULL);
	IupSetAttribute(vbox, "NMARGIN", "16x4");
	IupSetAttribute(vbox, "NGAP", "4");
	return vbox;
}

static Ihandle *ezgui_page_setup_output(EZGUI *gui)
{
	Ihandle	*hbox1, *hbox2, *hbox3, *hbox4, *vbox;
	int	i;

	/* first line: define the suffix of file name */
	hbox1 = xui_text(&gui->fmt_suffix, "Suffix of Thumbnails");
	IupSetAttribute(gui->fmt_suffix, "VALUE", gui->sysopt->suffix);

	/* second line: process of existed thumbnails */
	hbox2 = xui_list_setting(&gui->fmt_exist, "Existed Thumbnails");
	for (i = 0; id_existed[i].s; i++) {
		IupSetAttributeId(gui->fmt_exist, "", i+1, id_existed[i].s);
	}

	/* third line: file format of thumbnails */
	hbox3 = xui_list_setting(&gui->fmt_list, "Save Picture As");
	for (i = 0; uir_format[i].s; i++) {
		IupSetAttributeId(gui->fmt_list, "", i+1, 
				uir_format[i].s);
	}
	IupSetCallback(gui->fmt_list, "ACTION",
			(Icallback) ezgui_event_setup_format);
	
	/* append the transparent control for png/gif */
	gui->fmt_transp = IupToggle("Transparent", NULL);
	IupAppend(hbox3, gui->fmt_transp);

	/* optional line: attribute of the file format */
	gui->fmt_zbox = IupZbox(IupFill(), 
			xui_text_setting(&gui->fmt_gif_fr, "FRate:", "(ms)"),
			xui_text_setting(&gui->fmt_jpg_qf, "Quality:", NULL),
			NULL);
	hbox4 = IupHbox(xui_label("", EGPS_SETUP_DESCR, NULL), 
			gui->fmt_zbox, NULL);
	IupSetAttribute(hbox4, "NGAP", "4");


	/* assemble the Picture Format area */
	vbox = IupVbox(hbox1, hbox2, hbox3, hbox4, NULL);
	IupSetAttribute(vbox, "NMARGIN", "16x4");
	IupSetAttribute(vbox, "NGAP", "4");
	return vbox;
}

static Ihandle *ezgui_page_setup_button(EZGUI *gui)
{
	Ihandle *hbox;
	
	gui->butt_setup_apply = 
		xui_button("OK", (Icallback) ezgui_event_setup_ok);

	gui->butt_setup_cancel = 
		xui_button("Cancel", (Icallback) ezgui_event_setup_cancel);

	hbox = IupHbox(gui->butt_setup_cancel, gui->butt_setup_apply, NULL);
	return IupHbox(xui_label("", "320", NULL), hbox, NULL);
}

static int ezgui_event_setup_format(Ihandle *ih, char *text, int i, int s)
{
	EZGUI	*gui;

	(void) i;

	if (s == 0) {
		return IUP_DEFAULT;	/* ignore the leaving item */
	}
	
	/* the EZGUI structure can be an impostor of the Ihandle when 
	 * initializing the widgets, where the dialog hasn't been linked 
	 * with the GUI object. A magic word is used to tell them */
	gui = (EZGUI *) ih;
	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	/* hide the transparent toggle and quality editboxes */
	IupSetAttribute(gui->fmt_transp, "VISIBLE", "NO");
	IupSetInt(gui->fmt_zbox, "VALUEPOS", 0);

	if (!strcmp(text, CFG_PIC_FMT_JPEG)) {
		IupSetInt(gui->fmt_zbox, "VALUEPOS", 2);
	} else {
		IupSetAttribute(gui->fmt_transp, "VISIBLE", "YES");
		if (!strcmp(text, CFG_PIC_FMT_GIFA)) {
			IupSetInt(gui->fmt_zbox, "VALUEPOS", 1);
		}
	}	
	return IUP_DEFAULT;
}

static int ezgui_event_setup_grid(Ihandle *ih, char *text, int i, int s)
{
	EZGUI	*gui = ezgui_get_global(ih);
	
	(void) text;	/* stop the gcc complaining */
	//printf("ezgui_event_setup_grid: %s %d %d\n", text, i, s);
	
	if (s) {
		IupSetInt(gui->entry_zbox_grid, "VALUEPOS", i - 1);
	}
	return IUP_DEFAULT;
}

static int ezgui_event_setup_zoom(Ihandle *ih, char *text, int i, int s)
{
	EZGUI	*gui = ezgui_get_global(ih);

	(void) text;	/* stop the gcc complaining */

	if (s) {
		IupSetInt(gui->entry_zbox_zoom, "VALUEPOS", i - 1);
	}
	return IUP_DEFAULT;
}

static int ezgui_event_setup_outputdir(Ihandle *ih, char *text, int i, int s)
{
	EZGUI	*gui;
	char	*val;

	(void) i;

	if (s == 0) {
		return IUP_DEFAULT;	/* ignore the leaving item */
	}
	
	/* the EZGUI structure can be an impostor of the Ihandle when 
	 * initializing the widgets, where the dialog hasn't been linked 
	 * with the GUI object. A magic word is used to tell them */
	gui = (EZGUI *) ih;
	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}
	
	if (strcmp(text, CFG_PIC_ODIR_PATH)) {
		IupSetAttribute(gui->dir_path, "VISIBLE", "NO");
		return IUP_DEFAULT;
	}

	IupSetAttribute(gui->dir_path, "VISIBLE", "YES");
	val = IupGetAttribute(gui->dir_path, "VALUE");
	if (val) {
		IupSetAttribute(gui->dlg_odir, "DIRECTORY", val);
	}
	IupPopup(gui->dlg_odir, IUP_CENTERPARENT, IUP_CENTERPARENT);
	if (IupGetInt(gui->dlg_odir, "STATUS") < 0) {
		return IUP_DEFAULT;	/* cancelled */
	}
	
	val = IupGetAttribute(gui->dlg_odir, "VALUE");
	EDB_DEBUG(("Open File VALUE: %s\n", val));
	EDB_DEBUG(("Last  DIRECTORY: %s\n", 
			IupGetAttribute(gui->dlg_odir, "DIRECTORY")));
	if (val) {
		IupSetAttribute(gui->dir_path, "VALUE", val);
	}
	return IUP_DEFAULT;
}

static int ezgui_event_setup_setfont(Ihandle *ih, char *text, int i, int s)
{
	EZGUI	*gui;
	char	*val;

	(void) i;

	if (s == 0) {
		return IUP_DEFAULT;	/* ignore the leaving item */
	}

	/* the EZGUI structure can be an impostor of the Ihandle when 
	 * initializing the widgets, where the dialog hasn't been linked 
	 * with the GUI object. A magic word is used to tell them */
	gui = (EZGUI *) ih;
	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}

	if (strcmp(text, CFG_PIC_FONT_BROWSE)) {
		IupSetAttribute(gui->font_face, "VISIBLE", "NO");
		return IUP_DEFAULT;
	}
	
	IupSetAttribute(gui->font_face, "VISIBLE", "YES");
	val = IupGetAttribute(gui->font_face, "VALUE");
	if (val) {
		IupSetAttribute(gui->dlg_font, "VALUE", val);
	}
	IupSetAttribute(gui->dlg_font, "COLOR", "128 0 255");
	IupPopup(gui->dlg_font, IUP_CENTERPARENT, IUP_CENTERPARENT);

	if (IupGetAttribute(gui->dlg_font, "STATUS") == NULL) {
		return IUP_DEFAULT;	/* cancelled */
	}

	val = IupGetAttribute(gui->dlg_font, "VALUE");
	EDB_DEBUG(("Font Face: %s\n", val));
	EDB_DEBUG(("Font Color: %s\n",
				IupGetAttribute(gui->dlg_font, "COLOR")));
	if (val) {
		IupSetAttribute(gui->font_face, "VALUE", val);
	}
	return IUP_DEFAULT;
}

static int ezgui_event_setup_ok(Ihandle *ih)
{
	EZGUI	*gui;
	EZOPT	*opt;
	char	*val, tmp[128];

	/* the EZGUI structure can be an impostor of the Ihandle when 
	 * initializing the widgets, where the dialog hasn't been linked 
	 * with the GUI object. A magic word is used to tell them */
	gui = (EZGUI *) ih;
	if (gui->magic != EZGUI_MAGIC) {
		gui = ezgui_get_global(ih);
	}
	opt = gui->sysopt;

	gui->grid_idx = xui_list_get_idx(gui->prof_grid);
	csc_cfg_write(gui->config, EZGUI_MAINKEY, 
			CFG_KEY_GRID, uir_grid[gui->grid_idx].s);
	gui->zoom_idx = xui_list_get_idx(gui->prof_zoom);
	csc_cfg_write(gui->config, EZGUI_MAINKEY,
			CFG_KEY_ZOOM, uir_zoom[gui->zoom_idx].s);
	gui->dfm_idx  = xui_list_get_idx(gui->dfm_list);
	csc_cfg_write(gui->config, EZGUI_MAINKEY,
			CFG_KEY_DURATION, id_duration_long[gui->dfm_idx].s);
	gui->fmt_idx  = xui_list_get_idx(gui->fmt_list);
	csc_cfg_write(gui->config, EZGUI_MAINKEY,
			CFG_KEY_FILE_FORMAT, uir_format[gui->fmt_idx].s);

	gui->mpm_idx  = xui_list_get_idx(gui->mpm_list);
	EZOP_PROC_MAKE(gui->sysopt->flags, id_mprocess[gui->mpm_idx].id);
	csc_cfg_write(gui->config, NULL, CFG_KEY_MEDIA_PROC,
			id_mprocess[gui->mpm_idx].s);
	
	gui->exist_idx = xui_list_get_idx(gui->fmt_exist);
	EZOP_THUMB_SET(gui->sysopt->flags, id_existed[gui->exist_idx].id);
	csc_cfg_write(gui->config, NULL, CFG_KEY_FILE_EXISTED, 
			id_existed[gui->exist_idx].s);

	gui->dir_idx  = xui_list_get_idx(gui->dir_list);
	csc_cfg_write(gui->config, EZGUI_MAINKEY,
			CFG_KEY_OUTPUT_METHOD, uir_outdir[gui->dir_idx].s);
	if (!strcmp(uir_outdir[gui->dir_idx].s, CFG_PIC_ODIR_CURR)) {
		csc_cfg_delete_key(gui->config, EZGUI_MAINKEY, 
				CFG_KEY_OUTPUT_PATH);
	} else {
		gui->sysopt->pathout = IupGetAttribute(gui->dir_path,"VALUE");
		csc_cfg_write(gui->config, EZGUI_MAINKEY, 
				CFG_KEY_OUTPUT_PATH, gui->sysopt->pathout);
	}

	gui->tmp_jpg_qf = (int) strtol(
			IupGetAttribute(gui->fmt_jpg_qf, "VALUE"), NULL, 10);
	csc_cfg_write_int(gui->config, EZGUI_MAINKEY,
			CFG_KEY_JPG_QUALITY, gui->tmp_jpg_qf);
	gui->tmp_gifa_fr = (int) strtol(
			IupGetAttribute(gui->fmt_gif_fr, "VALUE"), NULL, 10);
	csc_cfg_write_int(gui->config, EZGUI_MAINKEY,
			CFG_KEY_GIF_FRATE, gui->tmp_gifa_fr);

	val = IupGetAttribute(gui->fmt_transp, "VALUE");
	if (!strcmp(val, "ON")) {
		gui->sysopt->flags |= EZOP_TRANSPARENT;
	} else {
		gui->sysopt->flags &= ~EZOP_TRANSPARENT;
	}
	csc_cfg_write(gui->config, EZGUI_MAINKEY, CFG_KEY_TRANSPARENCY, val);
	
	EDB_DEBUG(("EVT_SETUP: Grid=%d Zoom=%d Dur=%d Fmt=%d JPG=%d "
				"GIFA=%d Tra=%s\n",
			gui->grid_idx, gui->zoom_idx, gui->dfm_idx, 
			gui->fmt_idx, gui->tmp_jpg_qf, gui->tmp_gifa_fr, val));

	/* FIXME: not quite readible */
	switch (gui->grid_idx) {
	case 0:
		strcpy(gui->status, "Auto Grid ");
		break;
	case 1:
		opt->grid_col = xui_text_get_number(gui->entry_col_grid);
		opt->grid_row = xui_text_get_number(gui->entry_row);
		sprintf(gui->status, "Grid:%dx%d ", 
				opt->grid_col, opt->grid_row);
		ezopt_profile_disable(opt, EZ_PROF_LENGTH);
		break;
	case 2:
		opt->grid_col = xui_text_get_number(gui->entry_col_step);
		opt->tm_step  = xui_text_get_number(gui->entry_step);
		sprintf(gui->status, "Column:%d Step:%d(s) ", 
				opt->grid_col, opt->tm_step);
		opt->tm_step *= 1000;
		ezopt_profile_disable(opt, EZ_PROF_LENGTH);
		break;
	case 3:
		opt->grid_col = 0;
		opt->grid_row = xui_text_get_number(gui->entry_dss_amnt);
		sprintf(gui->status, "Total %d snaps ", opt->grid_row);
		ezopt_profile_disable(opt, EZ_PROF_LENGTH);
		break;
	case 4:
		opt->grid_col = 0;
		opt->grid_row = 0;
		opt->tm_step  = xui_text_get_number(gui->entry_dss_step);
		sprintf(gui->status, "Snap every %d(s) ", 
				opt->tm_step);
		gui->sysopt->tm_step *= 1000;
		ezopt_profile_disable(opt, EZ_PROF_LENGTH);
		break;
	case 5:
		opt->grid_col = 0;
		opt->grid_row = 0;
		opt->tm_step  = 0;
		strcpy(gui->status, "Separate I-Frames ");
		ezopt_profile_disable(opt, EZ_PROF_LENGTH);
		break;
	default:
		strcpy(gui->status, "Oops; ");
		break;
	}
	switch (gui->zoom_idx) {
	case 0:
		strcpy(tmp, "Auto Zoom ");
		break;
	case 1:
		opt->tn_facto  = xui_text_get_number(gui->entry_zoom_ratio);
		sprintf(tmp, "Zoom to %d%% ", opt->tn_facto);
		ezopt_profile_disable(opt, EZ_PROF_WIDTH);
		break;
	case 2:
		opt->tn_width  = xui_text_get_number(gui->entry_zoom_wid);
		opt->tn_height = xui_text_get_number(gui->entry_zoom_hei);
		sprintf(tmp, "Zoom to %dx%d ", 
				opt->tn_width, opt->tn_height);
		ezopt_profile_disable(opt, EZ_PROF_WIDTH);
		break;
	case 3:
		opt->canvas_width = xui_text_get_number(gui->entry_width);
		sprintf(tmp, "Canvas Width %d ", opt->canvas_width);
		ezopt_profile_disable(opt, EZ_PROF_WIDTH);
		break;
	default:
		strcpy(tmp, "Oops; ");
		break;
	}
	strcat(gui->status, tmp);
	switch (gui->dfm_idx) {
	case 0:
		SETDURMOD(opt->flags, EZOP_DUR_AUTO);
		strcpy(tmp, "Auto detect");
		break;
	case 1:
		SETDURMOD(opt->flags, EZOP_DUR_HEAD);
		strcpy(tmp, "Detect by Head");
		break;
	case 2:
		SETDURMOD(opt->flags, EZOP_DUR_FSCAN);
		strcpy(tmp, "Detect by Full Scan");
		break;
	case 3:
		SETDURMOD(opt->flags, EZOP_DUR_QSCAN);
		strcpy(tmp, "Detect by Partial Scan");
		break;
	default:
		strcpy(tmp, "Oops; ");
		break;
	}
	strcat(gui->status, tmp);
	IupSetAttribute(gui->stat_bar, "TITLE", gui->status);
	//printf("%s\n", gui->status);
	      	
	IupSetInt(gui->entry_zbox_grid, "VALUEPOS", gui->grid_idx);
	IupSetInt(gui->entry_zbox_zoom, "VALUEPOS", gui->zoom_idx);

	/* save all related parameters into the configure file */
	csc_cfg_write_int(gui->config, NULL, CFG_KEY_GRID_COLUMN, 
			opt->grid_col);
	csc_cfg_write_int(gui->config, NULL, CFG_KEY_GRID_ROW, 
			opt->grid_row);
	csc_cfg_write_int(gui->config, NULL, CFG_KEY_TIME_STEP, 
			opt->tm_step);
	csc_cfg_write_int(gui->config, NULL, CFG_KEY_ZOOM_RATIO, 
			opt->tn_facto);
	csc_cfg_write_int(gui->config, NULL, CFG_KEY_ZOOM_WIDTH, 
			opt->tn_width);
	csc_cfg_write_int(gui->config, NULL, CFG_KEY_ZOOM_HEIGHT, 
			opt->tn_height);
	csc_cfg_write_int(gui->config, NULL, CFG_KEY_CANVAS_WIDTH, 
			opt->canvas_width);
	csc_cfg_write(gui->config, NULL, CFG_KEY_DURATION, 
			lookup_string_idnum(id_duration_long, -1, 
				GETDURMOD(opt->flags)));
	return IUP_DEFAULT;
}

static int ezgui_event_setup_cancel(Ihandle *ih)
{
	puts("Why");
	ezgui_page_setup_reset(ezgui_get_global(ih));
	return IUP_DEFAULT;
}


/****************************************************************************
 * Page About
 ****************************************************************************/
static	const	char	*description = "\
A video thumbnail generator based on FFMPEG library.\n\
\n\
Copyright (C) 2011-2015 \"Andy Xuming\" <xuming@users.sourceforge.net>\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";

static	const	char	*credits = "\
FFmpeg Win32 shared build by Kyle Schwarz from Zeranoe's:\n\
http://ffmpeg.zeranoe.com/builds\n\
You can find source codes and copyrights of FFMPEG at\n\
https://www.ffmpeg.org\n\
\n\
Following Libraries were grabbed from GnuWin:\n\
http://sourceforge.net/projects/gnuwin32/files\n\
\n\
gd-2.0.33-1\n\
jpeg-6b-4\n\
libiconv-1.9.2-1\n\
libpng-1.2.37\n\
zlib-1.2.3\n\
freetype-2.3.5-1\n\
\n\
The icon is a public domain under GNU Free Documentation License:\n\
http://commons.wikimedia.org/wiki/File:SMirC-thumbsup.svg\n\
\n\
The GUI frontend is based on IUP, a multi-platform toolkit for building\n\
graphical user interfaces.\n\
http://webserver2.tecgraf.puc-rio.br/iup\n\
\n\
This program was inspired by movie thumbnailer (mtn):\n\
http://sourceforge.net/projects/moviethumbnail\n";

static Ihandle *ezgui_page_about(EZGUI *gui)
{
	Ihandle	*icon, *name, *descr, *thanks;
	Ihandle	*vbox, *sbox;

	(void) gui;

	/* show the icon */
	icon = IupLabel(NULL);
	IupSetAttributeHandle(icon, "IMAGE", 
			IupImageRGBA(64, 64, ezicon_about));

	/* show name and the version */
	name = IupLabel("Ezthumb " EZTHUMB_VERSION);
	IupSetAttribute(name, "FONTSIZE", "20");
	IupSetAttribute(name, "FONTSTYLE", "Bold");
	
	/* show the simple description */
	descr = IupLabel(description);
	IupSetAttribute(descr, "ALIGNMENT", "ACENTER:ACENTER");

	/* show the credits */
	thanks = IupLabel(credits);
	IupSetAttribute(thanks, "ALIGNMENT", "ACENTER:ACENTER");
	
	/* group these elements inside a vertical box */
	vbox = IupVbox(icon, name, descr, thanks, NULL);
	IupSetAttribute(vbox, "NGAP", "8");
	IupSetAttribute(vbox, "NMARGIN", "16x16");
	IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");

	/* fill the right side of the veritcal box with blank and pack
	 * into a scrollbox */
	sbox = IupScrollBox(IupHbox(vbox, IupHbox(IupFill(), NULL), NULL));
	IupSetAttribute(sbox, "SCROLLBAR", "VERTICAL");
	return sbox;
}


/****************************************************************************
 * New control for workarea
 ****************************************************************************/

static Ihandle *ezgui_sview_create(EZGUI *gui, int dblck)
{
	Ihandle	*vb_main, *vb_size, *vb_len, *vb_res, *vb_prog, *hbox;
	SView	*sview;

	if ((sview = smm_alloc(sizeof(SView))) == NULL) {
		return NULL;
	}
	sview->gui = gui;

	sview->filename = IupList(NULL);
	IupSetAttribute(sview->filename, "EXPAND", "YES");
	IupSetAttribute(sview->filename, "MULTIPLE", "YES");
	IupSetAttribute(sview->filename, "SCROLLBAR", "NO");
	IupSetAttribute(sview->filename, "DROPFILESTARGET", "YES");
	IupSetAttribute(sview->filename, "ALIGNMENT", "ARIGHT");
	IupSetAttribute(sview->filename, "SVIEWOBJ", (char*) sview);
	vb_main = IupVbox(xui_label("Files", NULL, NULL), 
			sview->filename, NULL);

	IupSetCallback(sview->filename, "DROPFILES_CB",
			(Icallback) ezgui_sview_event_dropfiles);
	IupSetCallback(sview->filename, "MULTISELECT_CB",
			(Icallback) ezgui_sview_event_multi_select);
	IupSetCallback(sview->filename, "BUTTON_CB",
			(Icallback) ezgui_sview_event_moused);
	IupSetCallback(sview->filename, "MOTION_CB",
			(Icallback) ezgui_sview_event_motion);
	if (dblck) {
		IupSetCallback(sview->filename, "DBLCLICK_CB", 
				(Icallback) ezgui_sview_event_run);
	}

	sview->filesize = IupList(NULL);
	IupSetAttribute(sview->filesize, "SIZE", "50");
	IupSetAttribute(sview->filesize, "EXPAND", "VERTICAL");
	IupSetAttribute(sview->filesize, "SCROLLBAR", "NO");
	IupSetAttribute(sview->filesize, "ACTIVE", "NO");
	vb_size = IupVbox(xui_label("Size", "50", NULL), 
			sview->filesize, NULL);

	sview->medialen = IupList(NULL);
	IupSetAttribute(sview->medialen, "SIZE", "48");
	IupSetAttribute(sview->medialen, "EXPAND", "VERTICAL");
	IupSetAttribute(sview->medialen, "SCROLLBAR", "NO");
	IupSetAttribute(sview->medialen, "ACTIVE", "NO");
	vb_len = IupVbox(xui_label("Length", "48", NULL), 
			sview->medialen, NULL);

	sview->resolution = IupList(NULL);
	IupSetAttribute(sview->resolution, "SIZE", "50");
	IupSetAttribute(sview->resolution, "EXPAND", "VERTICAL");
	IupSetAttribute(sview->resolution, "SCROLLBAR", "NO");
	IupSetAttribute(sview->resolution, "ACTIVE", "NO");
	vb_res = IupVbox(xui_label("Resolution", "50", NULL), 
			sview->resolution, NULL);
	
	sview->progress = IupList(NULL);
	IupSetAttribute(sview->progress, "SIZE", "40");
	IupSetAttribute(sview->progress, "SCROLLBAR", "NO");
	IupSetAttribute(sview->progress, "EXPAND", "VERTICAL");
	IupSetAttribute(sview->progress, "ACTIVE", "NO");
	vb_prog = IupVbox(xui_label("Progress", "40", NULL), 
			sview->progress, NULL);

	sview->attrib = IupList(NULL);
	IupSetAttribute(sview->attrib, "SIZE", "50");
	IupSetAttribute(sview->attrib, "EXPAND", "VERTICAL");
	IupSetAttribute(sview->attrib, "SCROLLBAR", "NO");
	IupSetAttribute(sview->attrib, "ACTIVE", "NO");
	IupVbox(xui_label("Attribution", "50", NULL), 
			sview->attrib, NULL);

	hbox = IupHbox(vb_main, vb_size, vb_len, vb_res, vb_prog, NULL);
	IupSetAttribute(hbox, "SVIEWOBJ", (char*) sview);
	return hbox;
}

static int ezgui_sview_progress(Ihandle *ih, int percent)
{
	SView	*sview;
	char	*tmp;

	if ((sview = (SView *) IupGetAttribute(ih, "SVIEWOBJ")) == NULL) {
		return EZ_ERR_PARAM;
	}
	tmp = IupGetAttributeId(sview->progress, "", sview->svidx);
	if (tmp) {
		sprintf(tmp, "%d%%",  percent);
		IupSetAttributeId(sview->progress, "", sview->svidx, tmp);
	}
	return EZ_ERR_NONE;
}

static int ezgui_sview_active_add(Ihandle *ih, int type, Ihandle *ctrl)
{
	SView	*sview;
	Ihandle	**clist;	/* control list */
	int	i;

	if ((sview = (SView*)IupGetAttribute(ih, "SVIEWOBJ")) == NULL) {
		return EZ_ERR_PARAM;
	}

	switch (type) {
	case EZGUI_SVIEW_ACTIVE_CONTENT:
		clist = sview->act_content;
		break;
	case EZGUI_SVIEW_ACTIVE_SELECT:
		clist = sview->act_select;
		break;
	case EZGUI_SVIEW_ACTIVE_PROGRESS:
		clist = sview->act_progress;
		break;
	default:
		return EZ_ERR_PARAM;	/* wrong type */
	}

	for (i = 0; i < EZGUI_SVIEW_ACTIVE_MAX; i++) {
		if (clist[i]) {
			continue;
		}
		clist[i] = ctrl;
		IupSetAttribute(clist[i], "SVIEWOBJ", (char*) sview);
		return EZ_ERR_NONE;	/* successful */
	}
	return EZ_ERR_LOWMEM;	/* full */ 
}

static int ezgui_sview_active_remove(Ihandle *ih, int type, Ihandle *ctrl)
{
	SView	*sview;
	Ihandle	**clist;	/* control list */
	int	i;

	if ((sview = (SView*)IupGetAttribute(ih, "SVIEWOBJ")) == NULL) {
		return EZ_ERR_PARAM;
	}

	switch (type) {
	case EZGUI_SVIEW_ACTIVE_CONTENT:
		clist = sview->act_content;
		break;
	case EZGUI_SVIEW_ACTIVE_SELECT:
		clist = sview->act_select;
		break;
	case EZGUI_SVIEW_ACTIVE_PROGRESS:
		clist = sview->act_progress;
		break;
	default:
		return EZ_ERR_PARAM;	/* wrong type */
	}

	for (i = 0; i < EZGUI_SVIEW_ACTIVE_MAX && clist[i]; i++) {
		if (ctrl && (ctrl != clist[i])) {
			continue;
		}
		/* dirty trick: do not remove the "SVIEWOBJ" attribution
		 * so it can be used to setup the "SVIEWOBJ" only */
		//IupSetAttribute(clist[i], "SVIEWOBJ", NULL);
		clist[i] = NULL;
	}
	return EZ_ERR_NONE;	/* successful */
}


static int ezgui_sview_event_run(Ihandle *ih, int item, char *text)
{
	EZGUI	*gui;
	SView	*sview;
	char	*attr;

	(void)text;

	if ((sview = (SView *) IupGetAttribute(ih, "SVIEWOBJ")) == NULL) {
		return IUP_DEFAULT;
	}

	sview->svidx = item;	/* store the current index */
	gui = sview->gui;

	if ((attr = IupGetAttributeId(sview->attrib, "", item)) == NULL) {
		return IUP_DEFAULT;
	}

	EDB_DEBUG(("EVT_Action %d: %s %s\n", item, attr, text));

	gui->sysopt->pre_seek = (int) strtol(attr, NULL, 0);
	attr = strchr(attr, ':');
	gui->sysopt->pre_br = (int) strtol(++attr, NULL, 0);
	attr = strchr(attr, ':');
	gui->sysopt->pre_dura = (EZTIME) strtoll(++attr, NULL, 0);

	/* bind the SVIEW structure to the progress bar */
	//IupSetAttribute(gui->prog_bar, "SVIEWOBJ", (char*) sview);
	/* bind the notification */
	gui->sysopt->notify = ezgui_sview_notificate;

	smm_codepage_set(65001);
	ezthumb(text, gui->sysopt);
	smm_codepage_reset();

	gui->sysopt->notify = NULL;
	gui->sysopt->pre_seek = 0;
	gui->sysopt->pre_br   = 0;
	gui->sysopt->pre_dura = 0;
	return IUP_DEFAULT;
}

static int ezgui_sview_event_dropfiles(Ihandle *ih, 
		const char* filename, int num, int x, int y)
{
	SView	*sview;

	(void)num; (void)x; (void)y;

	if ((sview = (SView *) IupGetAttribute(ih, "SVIEWOBJ")) == NULL) {
		return IUP_DEFAULT;
	}

	EDB_DEBUG(("EVT_Dropfiles: fname=%s number=%d %dx%d\n",
				filename, num, x, y));
	ezgui_sview_file_append(sview, (char*) filename);
	/* highlight the RUN button when the list is not empty */
	ezgui_sview_active_update(sview, 
			EZGUI_SVIEW_ACTIVE_CONTENT, sview->svnum);
	return IUP_DEFAULT;
}

static int ezgui_sview_event_multi_select(Ihandle *ih, char *value)
{
	SView	*sview;
	int	i, n;

	if ((sview = (SView *) IupGetAttribute(ih, "SVIEWOBJ")) == NULL) {
		return IUP_DEFAULT;
	}

	EDB_DEBUG(("EVT_Multi_select: %s\n", value));
	EDB_DEBUG(("List value=%s\n", 
				IupGetAttribute(sview->filename, "VALUE")));
	
	/* the parameter 'value' is useless. grab list myself */
	/*value = IupGetAttribute(sview->list_fname, "VALUE");
	for (i = 0; value[i]; i++) {
		if (value[i] == '+') {
			IupSetAttribute(gui->button_del, "ACTIVE", "YES");
			return IUP_DEFAULT;
		}
	}
	IupSetAttribute(gui->button_del, "ACTIVE", "NO");*/
	value = IupGetAttribute(sview->filename, "VALUE");
	for (i = n = 0; value[i]; i++) {
		if (value[i] == '+') {
			n++;
		}
	}
	ezgui_sview_active_update(sview, EZGUI_SVIEW_ACTIVE_SELECT, n);
	return IUP_DEFAULT;
}

static int ezgui_sview_event_moused(Ihandle *ih, 
		int button, int pressed, int x, int y, char *status)
{
	SView	*sview;

	(void)x; (void)y; (void)status;	/* stop compiler complains */

	if ((sview = (SView *) IupGetAttribute(ih, "SVIEWOBJ")) == NULL) {
		return IUP_DEFAULT;
	}

	EDB_MODL(("EVT_Mouse: %d %d %dx%d %s\n", 
				button, pressed, x, y, status));
	if (pressed) {	/* only act when button is released */
		return IUP_DEFAULT;
	}
	/* deselect every thing if the right button was released */
	if (button == IUP_BUTTON3) {
		IupSetAttribute(sview->filename, "VALUE", "");
		ezgui_sview_active_update(sview, 
				EZGUI_SVIEW_ACTIVE_SELECT, 0);
		//IupSetAttribute(gui->button_del, "ACTIVE", "NO");
	}
	return IUP_DEFAULT;
}

static int ezgui_sview_event_motion(Ihandle *ih, int x, int y, char *status)
{
	SView	*sview;
	char	*fname;
	int	line;

	(void)status;	/* stop compiler complains */

	line  = IupConvertXYToPos(ih, x, y);
	sview = (SView *) IupGetAttribute(ih, "SVIEWOBJ");

	EDB_MODL(("EVT_Motion: %d %d %d\n", x, y, line));
	
	if ((line < 1) || (sview == NULL)) {
		IupSetAttribute(ih, "TIP", "");
		return IUP_DEFAULT;
	}

	if (line != sview->moused) {
		sview->moused = line;
		if ((fname = IupGetAttributeId(ih, "",  line)) != NULL) {
			IupSetAttribute(ih, "TIP", fname);
		}
	}
	return IUP_DEFAULT;
}

static int ezgui_sview_add(SView *sview)
{
	EZGUI	*gui = (EZGUI *) sview->gui;
	char	*flist, *fname, *path, *sdir;
	int	i, amnt;
	
	/* Store the recent visited diretory so it can be used next time.
	 * The file open dialog can not go to the last directory in gtk */
	path = csc_cfg_read(gui->config, EZGUI_MAINKEY, CFG_KEY_DIRECTORY);
	if (path) {
		IupSetAttribute(gui->dlg_open, "DIRECTORY", path);
	}
	IupPopup(gui->dlg_open, IUP_CENTERPARENT, IUP_CENTERPARENT);

	if (IupGetInt(gui->dlg_open, "STATUS") < 0) {
		return IUP_DEFAULT;	/* cancelled */
	}

	/* IUP generate different file list between one file and more files:
	 * Open File VALUE: /home/xum1/dwhelper/lan_ke_er.flv
	 * Last  DIRECTORY: /home/xum1/dwhelper/
	 * Open File VALUE: /home/xum1/dwhelper|file-602303262.flv|
	 * 			lan_ke_er.flv|Powered_by_Discuz.flv|
	 * Last  DIRECTORY: /home/xum1/dwhelper0 */
	EDB_DEBUG(("Open File VALUE: %s\n", 
			IupGetAttribute(gui->dlg_open, "VALUE")));
	EDB_DEBUG(("Last  DIRECTORY: %s\n", 
			IupGetAttribute(gui->dlg_open, "DIRECTORY")));
	/* duplicate the path and filename list */
	flist = csc_strcpy_alloc(IupGetAttribute(gui->dlg_open, "VALUE"), 16);
	/* find out how many files is in */
	for (i = amnt = 0; flist[i]; i++) {
		amnt += (flist[i] == '|') ? 1 : 0;
	}
	if (amnt == 0) {
		amnt++;
	} else {
		amnt--;
	}

	/* store the current directory. note that the tailing '/' or '0' 
	 * need to be cut out first.
	 * Is the tailing '0' a bug of IUP? */
	path = csc_strcpy_alloc(
			IupGetAttribute(gui->dlg_open, "DIRECTORY"), 4);
	i = strlen(path) - 1;
	if ((path[i] == '/') || (path[i] == '\\') || (path[i] == '0')) {
		path[i] = 0;
	}
	csc_cfg_write(gui->config, EZGUI_MAINKEY, CFG_KEY_DIRECTORY, path);
	smm_free(path);

	/* process the single file list */
	if (amnt == 1) {
		ezgui_sview_file_append(sview, flist);
		/* highlight the RUN button when the list is not empty */
		ezgui_sview_active_update(sview, 
				EZGUI_SVIEW_ACTIVE_CONTENT, sview->svnum);
		smm_free(flist);
		return IUP_DEFAULT;
	}

	/* cut out the path first */
	flist = csc_cuttoken(flist, &sdir, "|");
	/* extract the file names */
	while ((flist = csc_cuttoken(flist, &fname, "|")) != NULL) {
		if (fname[0] == 0) {
			break;	/* end of list */
		}
		path = csc_strcpy_alloc(sdir, strlen(fname)+8);
		strcat(path, "/");
		strcat(path, fname);
		//printf("%s\n", path);
		ezgui_sview_file_append(sview, path);
		/* highlight the RUN button when the list is not empty */
		ezgui_sview_active_update(sview, 
				EZGUI_SVIEW_ACTIVE_CONTENT, sview->svnum);
		smm_free(path);
	}
	smm_free(flist);
	return IUP_DEFAULT;
}

static int ezgui_sview_remove(SView *sview)
{
	char	*value;
	int	i;

	while (1) {
		value = IupGetAttribute(sview->filename, "VALUE");
		EDB_PROG(("EVT_Remove: %s\n", value));
		for (i = 0; value[i]; i++) {
			if (value[i] == '+') {
				ezgui_sview_file_remove(sview, i+1);
				break;
			}
		}
		if (!value[i]) {
			break;
		}
	}
	ezgui_sview_active_update(sview, 
			EZGUI_SVIEW_ACTIVE_SELECT, 0);
	ezgui_sview_active_update(sview, 
			EZGUI_SVIEW_ACTIVE_CONTENT, sview->svnum);
	return IUP_DEFAULT;
}

static int ezgui_sview_run(SView *sview)
{
	char	*value, *fname;
	int	i, n;

	value = IupGetAttribute(sview->filename, "VALUE");
	for (i = n = 0; value[i]; i++) {
		if (value[i] == '+') {
			fname = IupGetAttributeId(sview->filename, "",  i+1);
			ezgui_sview_event_run(sview->filename, i+1, fname);
			n++;
		}
	}
	if (n == 0) {
		for (i = 0; i < sview->svnum; i++) {
			fname = IupGetAttributeId(sview->filename, "",  i+1);
			ezgui_sview_event_run(sview->filename, i+1, fname);
		}
	}
	return IUP_DEFAULT;
}


static int ezgui_sview_file_append(SView *sview, char *fname)
{
	EZVID	vobj;
	char	buf[64];
	int	lnext;

	/* preset the filename to make it look better */
	lnext = sview->svnum + 1;
	IupSetStrAttributeId(sview->filename, "",  lnext, fname);

	/* 20120903 Bugfix: set the codepage to utf-8 before calling
	 * ezthumb core. In Win32 version, the ezthumb core uses the 
	 * default codepage to process file name. However the GTK 
	 * converted the file name to UTF-8 so the Windows version 
	 * could not find the file. 
	 * There's no such problem in linux.*/
	smm_codepage_set(65001);
	if (ezinfo(fname, sview->gui->sysopt, &vobj) != EZ_ERR_NONE) {
		smm_codepage_reset();
		/* FIXME: disaster control */
		IupSetInt(sview->filename, "REMOVEITEM", lnext);
		return EZ_ERR_FORMAT;
	}
	smm_codepage_reset();

	meta_filesize(vobj.filesize, buf);
	IupSetStrAttributeId(sview->filesize,   "", lnext, buf);
	meta_timestamp(vobj.duration, 0, buf);
	IupSetStrAttributeId(sview->medialen,   "", lnext, buf);
	sprintf(buf, "%dx%d", vobj.width, vobj.height);
	IupSetStrAttributeId(sview->resolution, "", lnext, buf);
	sprintf(buf, "%d:%d:%lld", vobj.seekable, vobj.bitrates, 
			(long long) vobj.duration);
	IupSetStrAttributeId(sview->attrib, "", lnext, buf);
	/* hope the internal allocated memory is reusable */
	IupSetStrAttributeId(sview->progress,   "", lnext, "0%     ");

	/* increase the list index first because the IUP list control
	 * starts from 1 */
	sview->svnum++;

	IupFlush();
	return sview->svnum;
}

static int ezgui_sview_file_remove(SView *sview, int idx)
{
	EDB_INFO(("Removing: %s\n", 
			IupGetAttributeId(sview->filename, "",  idx)));

	IupSetInt(sview->filename, "REMOVEITEM", idx);
	IupSetInt(sview->filesize, "REMOVEITEM", idx);
	IupSetInt(sview->medialen, "REMOVEITEM", idx);
	IupSetInt(sview->resolution, "REMOVEITEM", idx);
	IupSetInt(sview->progress, "REMOVEITEM", idx);
	IupSetInt(sview->attrib, "REMOVEITEM", idx);
	sview->svnum--;
	return sview->svnum;
}

static int ezgui_sview_active_update(SView *sview, int type, int num)
{
	Ihandle	**clist;	/* control list */
	int	i;

	switch (type) {
	case EZGUI_SVIEW_ACTIVE_CONTENT:
		clist = sview->act_content;
		break;
	case EZGUI_SVIEW_ACTIVE_SELECT:
		clist = sview->act_select;
		break;
	case EZGUI_SVIEW_ACTIVE_PROGRESS:
		clist = sview->act_progress;
		break;
	default:
		return EZ_ERR_PARAM;
	}

	for (i = 0; i < EZGUI_SVIEW_ACTIVE_MAX; i++) {
		if (clist[i] == NULL) {
			continue;
		}
		if (num) {
			IupSetAttribute(clist[i], "ACTIVE", "YES");
		} else {
			IupSetAttribute(clist[i], "ACTIVE", "NO");
		}
	}
	return EZ_ERR_NONE;
}

static int ezgui_sview_notificate(void *v, int eid, long param, long opt, void *b)
{
	EZGUI	*gui = ((EZOPT*) v)->gui;

	(void)b;

	switch (eid) {
	case EN_PROC_BEGIN:
		ezgui_show_progress(gui, 0, 0);	/* show/reset progress bar */
		break;
	case EN_PROC_CURRENT:
		ezgui_sview_progress(gui->prog_bar, (int)(opt * 100 / param));
		ezgui_show_progress(gui, opt, param);
		break;
	case EN_PROC_END:
		ezgui_sview_progress(gui->prog_bar, 100);
		ezgui_show_progress(gui, param, param);
		break;
	/*case EN_OPEN_BEGIN:
	case EN_OPEN_GOING:
	case EN_OPEN_END:
		ezgui_show_duration(gui, eid);
		break;*/
	default:
		return EN_EVENT_PASSTHROUGH;
	}
	return eid;
}


/****************************************************************************
 * Support Functions 
 ****************************************************************************/
static Ihandle *xui_label(char *label, char *size, char *font)
{
	Ihandle	*ih;

	ih = IupLabel(label);
	if (size) {
		IupSetAttribute(ih, "SIZE", size);
	}
	if (font) {
		IupSetAttribute(ih, "FONTSTYLE", font);
	}
	return ih;
}

static Ihandle *xui_text(Ihandle **xlst, char *label)
{
	Ihandle	*text, *hbox;

	text = IupText(NULL);
	IupSetAttribute(text, "SIZE", EGPS_SETUP_DROPDOWN);

	hbox = IupHbox(xui_label(label, EGPS_SETUP_DESCR, NULL), text, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(hbox, "NGAP", "4");

	if (xlst) {
		*xlst = text;
	}
	return hbox;
}

/* define the standart dropdown list for the setup page */
static Ihandle *xui_list_setting(Ihandle **xlst, char *label)
{
	Ihandle	*list, *hbox;

	list = IupList(NULL);
	IupSetAttribute(list, "SIZE", EGPS_SETUP_DROPDOWN);
	IupSetAttribute(list, "DROPDOWN", "YES");

	hbox = IupHbox(xui_label(label, EGPS_SETUP_DESCR, NULL), list, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(hbox, "NGAP", "4");

	if (xlst) {
		*xlst = list;
	}
	return hbox;
}

static int xui_list_get_idx(Ihandle *ih)
{
	int	val;

	val = (int) strtol(IupGetAttribute(ih, "VALUE"), NULL, 10);
	return val - 1;
}

static int xui_text_get_number(Ihandle *ih)
{
	return (int) strtol(IupGetAttribute(ih, "VALUE"), NULL, 0);
}

static int xui_get_size(Ihandle *ih, char *attr, int *height)
{
	char	*ssize;
	int	width;

	if ((ssize = IupGetAttribute(ih, attr)) == NULL) {
		return -1;
	}
	//printf("xui_get_size: %s = %s\n", attr, ssize);

	width = (int) strtol(ssize, NULL, 10);

	if ((ssize = strchr(ssize, 'x')) != NULL) {
		if (height) {
			*height = (int) strtol(++ssize, NULL, 10);
		}
	}
	return width;
}

static Ihandle *xui_text_setting(Ihandle **xtxt, char *label, char *ext)
{
	Ihandle	*hbox, *text;

	text = IupText(NULL);
	IupSetAttribute(text, "SIZE", "40x10");

	hbox = IupHbox(IupLabel(label), text,
			ext ? IupLabel(ext) : NULL, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(hbox, "NGAP", "4");

	if (xtxt) {
		*xtxt = text;
	}
	return hbox;
}

static Ihandle *xui_text_grid(char *label, 
		Ihandle **xcol, Ihandle **xrow, char *ext)
{
	Ihandle	*hbox, *text1, *text2;

	if ((xcol == NULL) && (xrow == NULL)) {
		return NULL;
	}

	if ((xcol == NULL) || (xrow == NULL)) {
		text1 = IupText(NULL);
		IupSetAttribute(text1, "SIZE", "24x10");
		if (xcol) {
			*xcol = text1;
		} else {
			*xrow = text1;
		}
		hbox = IupHbox(xui_label(label, "28", NULL), text1,
				ext ? IupLabel(ext) : NULL, NULL);
		IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
		IupSetAttribute(hbox, "NGAP", "4");
		return hbox;
	}

	text1 = IupText(NULL);
	*xcol = text1;
	IupSetAttribute(text1, "SIZE", "18x10");
	text2 = IupText(NULL);
	*xrow = text2;
	IupSetAttribute(text2, "SIZE", "18x10");
	hbox = IupHbox(xui_label(label, "28", NULL), text1, IupLabel("x"), 
			text2, ext ? IupLabel(ext) : NULL, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(hbox, "NGAP", "4");
	return hbox;
}


static Ihandle *xui_button(char *prompt, Icallback ntf)
{
	Ihandle	*button;

	button = IupButton(prompt, NULL);
	IupSetAttribute(button, "SIZE", "42");
	if (ntf) {
		IupSetCallback(button, "ACTION", ntf);
	}
	return button;
}


static int config_dump(void *config, char *prompt)
{
	char	*path;
	int	item;

	path = csc_cfg_status(config, &item);
	EDB_SHOW(("%s: %d items in %s\n", prompt, item, path));
	return 0;
}


