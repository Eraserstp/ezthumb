/*  ezwinfont.c

    Copyright (C) 2017  "Andy Xuming" <xuming@users.sourceforge.net>

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
/*
 * http://stackoverflow.com/questions/4577784/get-a-font-filename-based-on-
 * font-name-and-style-bold-italic 
 */
#ifdef  HAVE_CONFIG_H
#include <config.h>
#else
#error "Run configure first"
#endif

#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <windows.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H 
#include FT_TRUETYPE_IDS_H

#include "iconv.h"

#include "libcsoup.h"
#include "ezthumb.h"

/* re-use the debug convention in libcsoup */
//#define CSOUP_DEBUG_LOCAL	SLOG_CWORD(EZTHUMB_MOD_CORE, SLOG_LVL_WARNING)
#define CSOUP_DEBUG_LOCAL	SLOG_CWORD(EZTHUMB_MOD_CORE, SLOG_LVL_DEBUG)
#include "libcsoup_debug.h"


static	TCHAR	*font_subkey[2] = {
	TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"),
	TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts")
};

static	struct	{
	int	platform;
	int	encoding;
	char	*codename;
} iconv_table[] = {
	{ TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_DEFAULT,  "UTF-16" },
	{ TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_UNICODE_1_1, "UTF-16" },
	{ TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_ISO_10646, "ISO-10646" },
	{ TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_UNICODE_2_0, "UTF-16" },
	{ TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_UNICODE_32, "UTF-32" },
	{ TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_VARIANT_SELECTOR, NULL },

	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_ROMAN, "MACINTOSH" },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_JAPANESE, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_TRADITIONAL_CHINESE, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_KOREAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_ARABIC, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_HEBREW, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_GREEK, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_RUSSIAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_RSYMBOL, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_DEVANAGARI, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_GURMUKHI, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_GUJARATI, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_ORIYA, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_BENGALI, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_TAMIL, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_TELUGU, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_KANNADA, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_MALAYALAM, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_SINHALESE, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_BURMESE, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_KHMER, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_THAI, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_LAOTIAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_GEORGIAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_ARMENIAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_MALDIVIAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_SIMPLIFIED_CHINESE, "gb2312" },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_TIBETAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_MONGOLIAN, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_GEEZ, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_SLAVIC, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_VIETNAMESE, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_SINDHI, NULL },
	{ TT_PLATFORM_MACINTOSH, TT_MAC_ID_UNINTERP, NULL },

	{ TT_PLATFORM_ISO, TT_ISO_ID_7BIT_ASCII, NULL },
	{ TT_PLATFORM_ISO, TT_ISO_ID_10646, NULL },
	{ TT_PLATFORM_ISO, TT_ISO_ID_8859_1, NULL },
	
	{ TT_PLATFORM_MICROSOFT, TT_MS_ID_SYMBOL_CS, "ucs-2be" },
	{ TT_PLATFORM_MICROSOFT, TT_MS_ID_UNICODE_CS, "ucs-2be" },
	{ TT_PLATFORM_MICROSOFT, TT_MS_ID_SJIS, NULL },
	{ TT_PLATFORM_MICROSOFT, TT_MS_ID_BIG_5, NULL },
	{ TT_PLATFORM_MICROSOFT, TT_MS_ID_WANSUNG, NULL },
	{ TT_PLATFORM_MICROSOFT, TT_MS_ID_JOHAB, NULL },
	{ TT_PLATFORM_MICROSOFT, TT_MS_ID_UCS_4, "UTF-32" },

	{ -1, -1, NULL }		
};

#define	MAX_FONT_LOOKUP		2048

#define EZFONT_STYLE_NONE	0
#define EZFONT_STYLE_BOLD	1
#define EZFONT_STYLE_ITALIC	2
#define EZFONT_STYLE_ALL	3

static	struct	{
	char	*font_path;
	char	*font_face;
	int	style;		/* 1=bold 2=italic 3=bold&italic */
} winfont_list[MAX_FONT_LOOKUP];

static	int	winfont_idx = 0;


static int ezwinfont_add_fontface(char *ftpath, int style);
static char *ezwinfont_face_alloc(FT_SfntName *aname);
static int ezwinfont_style_from_face(TCHAR *ftface);
static int ezwinfont_style_value(int expected, int received);
static char *ez_convert(char *fromcode, char *mem, size_t mlen);
static void ez_dump(char *prompt, void *mem, int mlen);


int ezwinfont_open(void)
{
	OSVERSIONINFO	osinfo;
	HKEY		hkey;
	TCHAR		ftfile[MAX_PATH], ftface[MAX_PATH], wpbuf[MAX_PATH];
	DWORD		fflen, fclen;
	LONG		rc;
	char		*ftpath;
	int		i, ftstyle;

	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&osinfo)) {
		return -1;
	}
	if (!GetWindowsDirectory(wpbuf, MAX_PATH)) {
		return -2;
	}
	if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, font_subkey[1],
				0, KEY_ALL_ACCESS, &hkey);
	} else {
		rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, font_subkey[0],
				0, KEY_ALL_ACCESS, &hkey);
	}
	if (rc != ERROR_SUCCESS) {
		return -2;
	}

	wcsncat(wpbuf, TEXT("\\Fonts\\"), MAX_PATH);
	for (i = 0; ; i++) {
		fclen = sizeof(ftface) - 1;
		fflen = sizeof(ftfile) - 1;
		if (RegEnumValue(hkey, i, ftface, &fclen, NULL, NULL, 
				(LPBYTE) ftfile, &fflen) != ERROR_SUCCESS) {
			break;
		}

		ftstyle = ezwinfont_style_from_face(ftface);

		if (wcschr(ftfile, L'\\')) {
			/* it's already the full path */
			ftpath = smm_wcstombs_alloc(ftfile);
		} else {
			wcsncpy(ftface, wpbuf, MAX_PATH);
			wcsncat(ftface, ftfile, MAX_PATH);
			ftpath = smm_wcstombs_alloc(ftface);
		}
		//CDB_DEBUG(("Read: %s\n", ftpath));

		/* only support the TrueType */		
		if (csc_cmp_file_extname(ftpath,"ttf") &&
				csc_cmp_file_extname(ftpath,"ttc")) {
			smm_free(ftpath);
			continue;
		}

		/* the ftpath was dynamically allocated so if it's
		 * successfully added to the font lookup table, it shall
		 * be freed only in ezwinfont_close() */
		if (ezwinfont_add_fontface(ftpath, ftstyle) < 0) {
			/* free the memory if it was failed to add up */
			smm_free(ftpath);
		}
	}

	RegCloseKey(hkey);
	return winfont_idx;
}

int ezwinfont_close(void)
{
	int	i;

	CDB_DEBUG(("ezwinfont_close\n"));
	for (i = 0; i < winfont_idx; i++) {
		if (winfont_list[i].font_face) {
			smm_free(winfont_list[i].font_face);
		} else {
			smm_free(winfont_list[i].font_path);
		}
		winfont_list[i].font_path = NULL;
		winfont_list[i].font_face = NULL;
	}
	winfont_idx = 0;
	return 0;
}

char *ezwinfont_faceoff(char *fontface)
{
	char	buf[256];
	int	i, value, style, can_val, can_idx = -1;
	
	//CDB_DEBUG(("FACEOFF: %s\n", fontface));
	
	style = EZFONT_STYLE_NONE;
	if (!strstr(fontface, ":bold")) {
		style |= EZFONT_STYLE_BOLD;
	}
	if (!strstr(fontface, ":italic")) {
		style |= EZFONT_STYLE_ITALIC;
	}

	if (*fontface == '@') {	/* skip the vertical flag */
		fontface++;
	}
	csc_strlcpy(buf, fontface, sizeof(buf));
	if ((fontface = strchr(buf, ':')) != NULL) {       
		*fontface = 0;
	}

	//CDB_DEBUG(("FACEOFF: %s %d\n", buf, winfont_idx));
	for (i = 0; i < winfont_idx; i++) {
		if (winfont_list[i].font_face == NULL) {
			continue;
		}
		/*CDB_DEBUG(("FACEOFF: Searching %s %d -> %s %d\n",
 				winfont_list[i].font_face,
				winfont_list[i].style,
				buf, style));*/
		if (strstr(winfont_list[i].font_face, buf) == NULL) {
			continue;
		}

		value = ezwinfont_style_value(winfont_list[i].style, style);
		if (can_idx == -1) {
			can_idx = i;	/* least matched */
			can_val = value;
		} else if (value < can_val) {
			can_idx = i;
			can_val = value;
		}
	}
	if (can_idx == -1) {
		return NULL;
	}
	return winfont_list[can_idx].font_path;
}

static int ezwinfont_add_fontface(char *ftpath, int ftstyle)
{
	FT_Library	library;
	FT_SfntName	aname;
	FT_Face		face;
	char		*tmp;
	int		i, namecnt;
	
	if (FT_Init_FreeType(&library) != 0) {
		return -1;
	}
	if (FT_New_Face(library, ftpath, 0, &face) != 0) {
		FT_Done_FreeType(library);
		return -2;
	}
	if (winfont_idx >= MAX_FONT_LOOKUP) {
		return -3;
	}

	CDB_DEBUG(("FTPATH: %s [%d]\n", ftpath, ftstyle));

	/* store the allocated string of the font path */
	winfont_list[winfont_idx].font_path = ftpath;
	winfont_list[winfont_idx].font_face = NULL;
	winfont_list[winfont_idx].style = ftstyle;

	namecnt = FT_Get_Sfnt_Name_Count(face);
	for (i = 0, winfont_idx++; i < namecnt; i++) {
		if (FT_Get_Sfnt_Name(face, i, &aname)) {
			continue;
		}
		
#if 0
		if ((tmp = ezwinfont_face_alloc(&aname)) != NULL) {
			CDB_DEBUG(("FTNAME::%3d %3d %3d %5d %4d %s\n", 
					aname.platform_id,
					aname.encoding_id, 
					aname.name_id,
					aname.language_id,
					aname.string_len,
					tmp));
			smm_free(tmp);
		}
#endif

		if (aname.name_id != TT_NAME_ID_FONT_FAMILY) {
			continue;
		}
		if (winfont_idx >= MAX_FONT_LOOKUP) {
			break;
		}

		if ((tmp = ezwinfont_face_alloc(&aname)) != NULL) {
			/* it's just a link to the font path */
			winfont_list[winfont_idx].font_path = ftpath; 
			winfont_list[winfont_idx].font_face = tmp;
			winfont_list[winfont_idx].style = ftstyle;
			winfont_idx++;
		}
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	return i;
}

static char *ezwinfont_face_alloc(FT_SfntName *aname)
{
	char	*mp;
	int	i;

#define	TRY_CHARSET(s)	CDB_DEBUG(("Trying %s: %s\n", (s), \
	ez_convert((s), (char*)aname->string, aname->string_len)));

	for (i = 0; iconv_table[i].platform != -1; i++) {
		if (aname->platform_id != iconv_table[i].platform) {
			continue;
		}
		if (aname->encoding_id != iconv_table[i].encoding) {
			continue;
		}
		if (iconv_table[i].codename == NULL) {
			break;
		}

		mp = ez_convert(iconv_table[i].codename, 
				(char*) aname->string, aname->string_len);

		CDB_DEBUG(("FTFACE:: %2d %2d %2d %s\n", strlen(mp),
				aname->platform_id, aname->encoding_id, mp));

		return csc_strcpy_alloc(mp, 0);
	}

	/* doesn't find the encoding, or doesn't need to convert */
	/* issue a warning and do a quick detection */
	CDB_DEBUG(("Font Face Charset Unknown: %d %d %d %d %d\n", 
			aname->platform_id, aname->encoding_id, aname->name_id,
			aname->language_id, aname->string_len));

	/* dump the font face string */
	ez_dump("Font Face String", aname->string, aname->string_len);
	TRY_CHARSET("UTF-16");
	TRY_CHARSET("UTF-16BE");
	TRY_CHARSET("UTF-16LE");


	/* simply allocate and copy out the string field */
	if ((mp = malloc(aname->string_len + 4)) != NULL) {
		memcpy(mp, aname->string, aname->string_len);
		mp[aname->string_len] = 0;
	}
	return mp;
}

static int ezwinfont_style_from_face(TCHAR *ftface)
{
	char	*s = smm_wcstombs_alloc(ftface);
	int	style = EZFONT_STYLE_NONE;

	if (s) {
		if (!strstr(s, "Bold")) {
			style |= EZFONT_STYLE_BOLD;
		}
		if (!strstr(s, "Italic")) {
			style |= EZFONT_STYLE_ITALIC;
		}
		smm_free(s);
	}
	return style;
}


static	int	style_eval[16][3] = {
	/*  EXPECT              RECEIVED                VALUE  */
	{ EZFONT_STYLE_NONE,	EZFONT_STYLE_NONE,	0 },
	{ EZFONT_STYLE_NONE,	EZFONT_STYLE_BOLD,	1 },
	{ EZFONT_STYLE_NONE,	EZFONT_STYLE_ITALIC,	1 },
	{ EZFONT_STYLE_NONE,	EZFONT_STYLE_ALL,	1 },
	{ EZFONT_STYLE_BOLD,	EZFONT_STYLE_NONE,	2 },
	{ EZFONT_STYLE_BOLD,	EZFONT_STYLE_BOLD,	0 },
	{ EZFONT_STYLE_BOLD,	EZFONT_STYLE_ITALIC,	3 },
	{ EZFONT_STYLE_BOLD,	EZFONT_STYLE_ALL,	1 },
	{ EZFONT_STYLE_ITALIC,	EZFONT_STYLE_NONE,	2 },
	{ EZFONT_STYLE_ITALIC,	EZFONT_STYLE_BOLD,	3 },
	{ EZFONT_STYLE_ITALIC,	EZFONT_STYLE_ITALIC,	0 },
	{ EZFONT_STYLE_ITALIC,	EZFONT_STYLE_ALL,	1 },
	{ EZFONT_STYLE_ALL,	EZFONT_STYLE_NONE,	2 },
	{ EZFONT_STYLE_ALL,	EZFONT_STYLE_BOLD,	1 },
	{ EZFONT_STYLE_ALL,	EZFONT_STYLE_ITALIC,	1 },
	{ EZFONT_STYLE_ALL,	EZFONT_STYLE_ALL,	0 }
};

static int ezwinfont_style_value(int expected, int received)
{
	int	i;

	for (i = 0; i < 16; i++) {
		if ((style_eval[i][0] == expected) && 
				(style_eval[i][1] == received)) {
			return style_eval[i][2];
		}
	}
	return 5;	/* lowest */
}

static char *ez_convert(char *fromcode, char *mem, size_t mlen)
{
	static	char	firmbuf[2048];
	iconv_t	cd;
	char	*buf = firmbuf;
	size_t	len = sizeof(firmbuf);

	if ((cd = iconv_open("utf-8", fromcode)) == (iconv_t) -1) {
		return NULL;
	}

	iconv(cd, (const char **) &mem, &mlen, &buf, &len);
	iconv_close(cd);
	*buf = 0; 	/* iconv doesn't end strings */
	return firmbuf;
}

static void ez_dump(char *prompt, void *mem, int mlen)
{
	char	*mp, buf[256];
	int	len, goc;

#define	EZDUMP_FLAG	CSC_MEMDUMP_NO_ADDR

	for (mp = mem, len = mlen; len > 0; mp += goc, len -= goc) {
		goc = len > 16 ? 16 : len;
		csc_memdump_line(mp, goc, EZDUMP_FLAG, buf, sizeof(buf));
		CDB_DEBUG(("%s: %s\n", prompt, buf));
	}
}



static int CALLBACK ezwinfont_callback(const LOGFONT *pk_Font, 
	const TEXTMETRIC* pk_Metric, DWORD e_FontType, LPARAM lParam)
{
	//ENUMLOGFONTEX	*ftext = (ENUMLOGFONTEX *) pk_Font;
	TCHAR	facebuf[LF_FACESIZE*2];
	char	*ftface, *ftname;

	(void) pk_Metric; (void) lParam;

	if (e_FontType != TRUETYPE_FONTTYPE) {
		return 1;
	}

	wcscpy(facebuf, pk_Font->lfFaceName);
	if (pk_Font->lfWeight >= 700) {
		wcscat(facebuf, TEXT(":bold"));
	}
	if (pk_Font->lfItalic) {
		wcscat(facebuf, TEXT(":italic"));
	}

	//ftname = smm_wcstombs_alloc(ftext->elfFullName);
	//CDB_DEBUG(("FONT: %s # %s\n", ftface, ftname));
	if ((ftface = smm_wcstombs_alloc(facebuf)) != NULL) {
		ftname = ezwinfont_faceoff(ftface);
		CDB_DEBUG(("%s: %s\n", ftface, ftname));
		smm_free(ftface);
	}
	return 1;
}

int ezwinfont_testing(char *fontface)
{
	LOGFONT	lf;
	HDC	hdc;
	TCHAR	*s;

	memset(&lf, 0, sizeof(lf));
	lf.lfCharSet = DEFAULT_CHARSET;

	if (fontface) {
		s = smm_mbstowcs_alloc(fontface);
		//s = iupwinStrToSystem(fontface);
		wcsncpy(lf.lfFaceName, s, LF_FACESIZE);
		free(s);
	}

	hdc = CreateCompatibleDC(NULL);
	EnumFontFamiliesEx(hdc, &lf, ezwinfont_callback, 0, 0);
	DeleteDC(hdc);
	return 0;
}

