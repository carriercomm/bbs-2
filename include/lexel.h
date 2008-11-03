/*-------------------------------------------------------*/
/* theme.h	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : custom theme				 */
/* create : 02/08/17				 	 */
/* update :   /  /  				 	 */
/*-------------------------------------------------------*/


#ifdef SITE_LEXEL

#undef BBSNAME
#undef BBSNAME2
#undef BBSNAME3
#undef SYSOPNICK
#undef MYIPADDR
#undef MYHOSTNAME
#undef HOST_ALIASES
#undef BBSHOME
#undef BAKPATH
#undef BBSUID
#undef BBSGID
#undef COLOR1
#undef COLOR2
#undef COLOR3
#undef COLOR4
#undef COLOR5
#undef COLOR6
#undef COLOR7
#undef COLOR8
#undef COLOR9
#undef COLOR10
#undef COLOR11
#undef COLOR_SITE
#undef COLORBAR_MENU
#undef COLORBAR_BRD
#undef EDIT_BANNER
#undef ICON_GAMBLED_BRD
#undef ICON_VOTED_BRD
#undef ICON_NOTRAN_BRD
#undef ICON_TRAN_BRD
#undef GOODBYE_MSG


/* ----------------------------------------------------- */
/* �w�q BBS ���W��}					 */
/* ------------------------------------------------------*/

#define BBSNAME		"�ӫȩ�Beta2"		/* ���寸�W */
#define BBSNAME2	"LexelBBS-bsd1"		/* �^�寸�W */
#define BBSNAME3	"�ӫȩ�"		/* �u���W */
#define SYSOPNICK	"sysop"			/* sysop ���ʺ� */

#define MYIPADDR	"140.114.87.41"		/* IP address */
#define MYHOSTNAME	"bsd1.cs.nthu.edu.tw"	/* �����a�} FQDN */
#define HOST_ALIASES	{MYHOSTNAME, MYIPADDR}

#define BBSHOME		"/home/bbs"		/* BBS ���a */
#define BAKPATH		"/home/bbs/bak"		/* �ƥ��ɪ����| */

#define BBSUID		1004
#define BBSGID		1004			/* Linux �г]�� 999 */


  /* ------------------------------------------------- */
  /* �պA�W�������U�{��                                */
  /* ------------------------------------------------- */

#undef POST_PREFIX	/* itoc.020113: �o���峹�ɼ��D�i��ܺ��� */


  /* ------------------------------------------------- */
  /* �պA�W�����ݪO�H�c                                */
  /* ------------------------------------------------- */

#ifdef HAVE_POPUPMENU
#undef HAVE_POPUPMENU		/* �ۥX����� */
#  undef POPUP_ANSWER		/* �ۥX����� -- �߰ݿﶵ */
#  undef POPUP_MESSAGE		/* �ۥX����� -- �T������ */
#endif

#undef HAVE_MULTI_SIGN		/* �h�د�ñ�ѨϥΪ̿�� */

#define MENU_FEAST		/* ��檬�A�C�B�~��ܸ`�� */


 /* ------------------------------------------------- */
  /* �պA�W�����~���{��                                */
  /* ------------------------------------------------- */

#ifdef	HAVE_EXTERNAL
#  undef HAVE_GAME		/* itoc.010208: ���ѹC�� */
#endif


/* ----------------------------------------------------- */
/* ���C��w�q�A�H�Q�����ק�				 */
/* ----------------------------------------------------- */

#define COLOR1		"\033[m\033[1;33;42m"	/* footer ���e�q�C�� */
#define COLOR2		"\033[m\033[30;47m"	/* footer ����q�C�� */
#define COLOR3		"\033[30;47m"		/* unused */
#define COLOR4		"\033[1;42m"		/* ���� ���C�� */
#define COLOR5		"\033[1;37;42m"		/* more ���Y�����D�C�� */
#define COLOR6		"\033[0;34;47m"		/* more ���Y�����e�C�� */
#define COLOR7		"\033[1m"		/* �@�̦b�u�W���C�� */
#define COLOR8		"\033[m\033[32;47m"	/* footer �[�j�е�(��/��) */
#define COLOR9		"\033[m\033[31;47m"	/* footer �[�j�е�(��/��) */
#define COLOR10		"\033[30;47m"		/* neck ���C�� */
#define COLOR11		"\033[31;47m"		/* menu footer ���C�� */
#define COLOR_SITE	"\033[1;37;42m"		/* ryanlei.081017: ���x�D�� */


/* ----------------------------------------------------- */
/* �U�������C��						 */
/* ----------------------------------------------------- */

#define COLORBAR_MENU	"\033[1;37;40m"		/* menu.c ������ */
#define COLORBAR_BRD	"\033[1;42m"		/* board.c, favor.c ������ */


/* ----------------------------------------------------- */
/* ���x�ӷ�ñ�W						 */
/* ----------------------------------------------------- */

#define EDIT_BANNER "\n--\n" \
			"\033[;32m�� %s�q (\033[1;30m%s\033[0;32m) �W�F \033[1;37m" \
			"�ӫȩ� \033[0;32m<lexel.twbbs.org>\033[m\n"


/* ----------------------------------------------------- */
/* ��L�T���r��						 */
/* ----------------------------------------------------- */

#define ICON_GAMBLED_BRD	"�C"	/* �|���L�����ݪO */
#define ICON_VOTED_BRD		"�I"	/* �|��벼�����ݪO */
#define ICON_NOTRAN_BRD		"��"			/* ����H�O */
#define ICON_TRAN_BRD		"��"			/* ��H�O */
#define GOODBYE_MSG		"�T�w���W�ӫȩҤF�� >///< �H[N] "


#endif	/* SITE_LEXEL */