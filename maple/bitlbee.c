/*-------------------------------------------------------*/
/* bitlbee.c   ( NTHU CS MapleBBS Ver 3.10 )             */
/*-------------------------------------------------------*/
/* author : ono.bbs@lexel.twbbs.org                      */
/* modify : smiler.bbs@bbs.cs.nthu.edu.tw                */
/* target : MSN on MapleBBS                              */
/* create : 05/06/08                                     */
/* update : 08/02/18                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

extern XZ xz[];

static int sock = 0;
static FILE *fw, *fr;
static char buf[512];
static XO bit_xo;

/* �w�]�W�u�̦h 250 �� */
static BITUSR bit_pool[250];

static
bit_fgets ()
{
  char *tmp;

  fgets (buf, sizeof (buf), fr);
  tmp = strstr (buf, "Logged out");

  if (tmp)
    {
      bit_abort ();
      vmsg ("�z�b��L�a��n�J msn�I");
    }
}

static void
bit_item(num, pp)
  int num;
  BITUSR *pp;
{

	  (strstr (pp->status, "Online")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;36m�u�W\033[m ",
	    num, pp->nick, pp->addr) : 
	  (strstr (pp->status,"Away")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;33m���}\033[m ",
	    num, pp->nick, pp->addr) : 
	  (strstr (pp->status,"Busy")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;31m���L\033[m ",
	    num, pp->nick, pp->addr) : 
	  (strstr (pp->status,"Idle")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;34m���m\033[m ",
	    num, pp->nick, pp->addr) : 
	  (strstr (pp->status,"Right")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;35m���W�^��\033[m ",
	    num, pp->nick, pp->addr) : 
	  (strstr (pp->status,"Phone")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;32m�q�ܤ�\033[m ",
	    num, pp->nick, pp->addr) : 
	  (strstr (pp->status,"Lunch")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m �~�X�Y�� ",
	    num, pp->nick, pp->addr) :
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m %-15.14s ",
	    num, pp->nick, pp->addr, pp->status);
		prints("\n");

}

#ifdef HAVE_LIGHTBAR
static int
bit_item_bar(xo, mode)
  XO *xo;
  int mode;     /* 1:�W����  0:�h���� */
{
	BITUSR *pp;
                                                                                
    //pp = bit_pool + xo->pos - xo->top;
    pp = bit_pool + xo->pos;

    prints("%s%5d   \033[1;37m%-18.17s\033[m%s  \033[30;1m%-34.33s\033[m%s ",
	    mode ? COLORBAR_PAL : "",
		xo->pos + 1,
		pp->nick,
		mode ? COLORBAR_PAL : "",
		pp->addr,
		mode ? COLORBAR_PAL : ""
		);
                                                                                
      (strstr (pp->status, "Online")) ?
	    prints("\033[1;36m�u�W           \033[m") : 
	  (strstr (pp->status,"Away")) ?
	    prints("\033[1;33m���}           \033[m") : 
	  (strstr (pp->status,"Busy")) ?
	    prints("\033[1;31m���L           \033[m") : 
	  (strstr (pp->status,"Idle")) ?
	    prints("\033[1;34m���m           \033[m") : 
	  (strstr (pp->status,"Right")) ?
	    prints("\033[1;35m���W�^��       \033[m") : 
	  (strstr (pp->status,"Phone")) ?
	    prints("\033[1;32m�q�ܤ�         \033[m") : 
	  (strstr (pp->status,"Lunch")) ?
	              prints("�~�X�Y��       \033[m") :
	              prints("               \033[m") ;


    return XO_NONE;
}
#endif


static int
bit_body (xo)
     XO *xo;
{
  BITUSR *pp;
  int num, max, tail;
  pp = &bit_pool[xo->top]; //
  max = xo->max;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;
  
  move(3, 0);
  do
  {
    bit_item(++num, pp++);
  } while (num < max);
  clrtobot();
  return XO_FOOT;
}

static int
bit_set (xo)
     XO *xo;
{
  char *tmp, max = 0;
  char seps[] = " \t";

  fprintf (fw, "PRIVMSG root :blist\r\n");
  fflush (fw);

  /* NICK ��n�b�� 48 �Ӧr, �e�����T�����L */
  do
    {
      bit_fgets ();
      if (sock<=0)
	return XO_QUIT;
      tmp = strstr (buf, "Nick");
    }
  while (!tmp);

  for (;;)
    {
      bit_fgets ();
      if (sock <=0)
	return XO_QUIT;

      tmp = strstr (buf, "MSN");

      if (!tmp)
	break;
      else
	{
	  tmp = strtok (buf, ":");
	  tmp = strtok (NULL, seps);
	  sprintf (bit_pool[max].nick, "%s", tmp);
	  tmp = strtok (NULL, seps);
	  sprintf (bit_pool[max].addr, "%s", tmp);
	  tmp = strtok (NULL, seps);
	  tmp = strtok (NULL, "\n");

	  do
	    {
	      *tmp++;
	    }
	  while (*tmp == ' ');

	  sprintf (bit_pool[max].status, "%s", tmp);
	  max++;
	}

    };

  xo->max = max;
  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  return bit_head (xo);
}

/* static */ int
bit_head (xo)
     XO *xo;
{
  clear ();
  vs_head ("MSN �C��", str_site);
  move (1, 0);
  prints
	(" [w]�ǰT [c]��ʺ� [^k]�_�u [a]�W�R�p���H [d]�R���p���H [l]msn���� [h]����   \n"
    "\033[30;47m �s��   �N   ��             �H          �c                     ��  �A         \033[m");

  return bit_body (xo);
}

static int
bit_init (xo)
     XO *xo;
{
  return bit_set (xo);

}

static int
bit_load (xo)
     XO *xo;
{
  return bit_head (xo);
}


static int
bit_help (xo)
  XO *xo;
{
  xo_help("msn");
  return XO_HEAD;
}

static int
bit_write (xo)
     XO *xo;
{
  int pos;
  char hint[30], *nick, str[65], file[128];
  screenline sl[b_lines + 1];
  FILE *fp;

  pos = xo->pos;
  nick = bit_pool[pos].nick;
  sprintf (hint, "��[%s] ", nick);

  vs_save (sl);

  if (vget (b_lines - 1, 0, hint, str, 60, DOECHO) &&
    vans ("�T�w�n��X msn (Y/N)�H[Y] ") != 'n')
    {
      usr_fpath (file, cuser.userid, FN_MSN);
      fp = fopen (file, "a");
      fprintf (fp, "To %s (@msn)�G%s\n", nick, str);
      fclose (fp);

      fprintf (fw, "PRIVMSG %s :%s\r\n", nick, str);
      fflush (fw);
    }

  vs_restore (sl);
  return XO_INIT;
}


/* smiler.080319: �^�Ф��y */
void
bit_reply (nick, msg)
     char *nick;
     char *msg;
{
  char hint[30]; 
  char str[65], file[128];
  FILE *fp;

  sprintf (hint, "��<%s> ", nick);

  usr_fpath (file, cuser.userid, FN_MSN);
  fp = fopen (file, "a");
  fprintf (fp, "To %s (@msn)�G%s\n", nick, msg);
  fclose (fp);

  fprintf (fw, "PRIVMSG %s :%s\r\n", nick, msg);
  fflush (fw);

}

static int
bit_allow (xo)
     XO *xo;
{

  if (vans ("�T�w�n�Ѱ�����L (Y/N)�H[N]") == 'y')
    {
      int pos;
      char *nick;

      pos = xo->pos;
      nick = bit_pool[pos].nick;
      fprintf (fw, "PRIVMSG root :allow %s\r\n", nick);
      fflush (fw);

      return XO_FOOT;
    }
  return XO_FOOT;
}

static int
bit_block (xo)
     XO *xo;
{

  if (vans ("�T�w�n����L (Y/N)�H[N]") == 'y')
    {
      int pos;
      char *nick;

      pos = xo->pos;
      nick = bit_pool[pos].nick;
      fprintf (fw, "PRIVMSG root :block %s\r\n", nick);
      fflush (fw);

      return XO_FOOT;
    }
  return XO_FOOT;
}

#if 0
static int
bit_save (xo)
     XO *xo;
{

  vmsg ("�]�w�n�O�H�ʺ٫�s�_�ӡA���s�W���ᤣ�ΦA�]�w�@�� :p");
  vmsg ("���L���٨S�g�n�� ^^;;");
  return XO_FOOT;
}
#endif

static int
bit_remv (xo)
     XO *xo;
{
  int pos;
  char *nick;

  pos = xo->pos;
  nick = bit_pool[pos].nick;

  if (vans ("�T�w�n�R���n�� (Y/N)�H[N]") == 'y')
    {
      fprintf (fw, "PRIVMSG root :remove %s\r\n", nick);
      fflush (fw);
    }
  return XO_INIT;

}
static int
bit_onick (xo)
     XO *xo;
{
  int pos;
  char *nick, str[10];

  pos = xo->pos;
  nick = bit_pool[pos].nick;

  vmsg ("�Ȯɥu����^��");
  move (b_lines - 1, 0);
  clrtoeol ();

  if (!vget (b_lines, 0, "���L���ӷs�W�r�a�G", str, 10, DOECHO))
    return XO_FOOT;
  if (strchr (str, ';') || strchr (str, ','))
    return XO_FOOT;

  fprintf (fw, "PRIVMSG root :rename %s %s\r\n", nick, str);
  fflush (fw);

  bit_fgets ();
  if (sock <=0)
    return XO_QUIT;

  while (!strstr ((buf), "Nick"))
    {
      bit_fgets ();
      if (sock <=0)
	return XO_QUIT;

    };

  sleep (1);

  return XO_INIT;
}

static int
bit_mynick ()
{
  char nick[40];

  if (!vget (b_lines, 0, "�ڪ��s�ʺ١G", nick, 38, DOECHO))
    return XO_FOOT;
  if (strchr (nick, ';') || strchr (nick, ','))
    return XO_FOOT;

  fprintf (fw, "PRIVMSG root :nick 0 %s\r\n", nick);
  fflush (fw);

  return XO_FOOT;
}

static int
bit_add ()
{

  char addr[40];

  if (!vget (b_lines, 0, "��J�s�W�n�ͫH�c�G", addr, 38, DOECHO))
    return XO_FOOT;

  fprintf (fw, "PRIVMSG root :add 0 %s\r\n", addr);
  fflush (fw);

  return XO_INIT;
}

/* static */ int
bit_recall ()
{
  char fpath[80];

  usr_fpath (fpath, cuser.userid, FN_MSN);
  more (fpath, NULL);
  return XO_INIT;
}

void
bit_abort ()
{
  if (sock >0)
    {
      fprintf (fw, "QUIT :bye\r\n");
      fflush (fw);
      fclose (fw);
      fclose (fr);
      sock = 0;
    }
}

static int
bit_close ()
{

  bit_abort ();

  zmsg ("�еy�� ..... ");
  sleep (1);
  vmsg ("�s�u���_�I");

  return XO_QUIT;
}

static int
bit_test ()
{

  char smiler_buf[32];
  sprintf(smiler_buf,"%d",cutmp->pid);
  vmsg (smiler_buf);
  return XO_INIT;
}

static KeyFunc bit_cb[] = {
#ifdef  HAVE_LIGHTBAR
  XO_ITEM, bit_item_bar,
#endif
  XO_INIT, bit_init,
  XO_LOAD, bit_load,
  XO_HEAD, bit_head,
  XO_BODY, bit_body,

  'b', bit_allow,
  'B', bit_block,
  'a', bit_add,
  'd', bit_remv,
  'l', bit_recall,
//  's', bit_save,
//  'n', bit_onick,
  'c', bit_mynick,
  'w', bit_write,
  Ctrl ('K'), bit_close,
  'h', bit_help
};

void
bit_rqst ()
{
  char *nick, *msg, send[600], file[128];
  FILE *fp;

  while (fgets (buf, sizeof (buf), fr))
    {
      if (msg = strstr (buf, "PRIVMSG"))
	{
	  msg = strstr (msg, ":");
	  *msg++;
	  nick = strtok (buf, "!");
	  *nick++;
	  sprintf (send, "\033[1;33;46m��%s (@msn) \033[37;45m %s \033[m", nick, msg);

	  usr_fpath (file, cuser.userid, FN_MSN);
	  fp = fopen (file, "a");
	  fprintf (fp, "\033[1;33;46m��%s (@msn) \033[m�G%s", nick, msg);
	  fclose (fp);
	  cursor_save ();

	  /***  smiler.080319:�e��bmw����  ***/

      UTMP *up;
      BMW bmw;
      char buf[20];
	  char bmw_msg[49];


      up = utmp_find(cuser.userno);
      sprintf(buf, "��<%s>", up->userid);
      if(strlen(msg) < 49)
		  strcpy(bmw_msg,msg);
	  else
		  strncpy(bmw_msg,msg,48);

	  bmw_msg[strlen(bmw_msg)-1]='\0';  /* smiler.080319:�B�zbmw_msg������ '\n' */
	  strcpy(bmw.nick,nick);       /* smiler.080319: �Ω� bmw���� reply msn*/
      strcpy(bmw.msg,bmw_msg);
      bit_bmw_edit(up,buf,&bmw);
      /***********************************/
	  cursor_restore ();
	  refresh ();
	  bell ();
	  if(strlen(msg) >= 49)   /* ���׶W�L���y�e�\�d��,�~�L�X */
		  vmsg("MSN�T���L���A�Цܡi �^�U msn �T�� �j�[�ݧ���T�� !!");
	  break;
	}
    }
}


int
bit_main ()
{

  int i = 0;
  char *tmp, account[50], pass[30];

  if (sock <= 0)
    {

      if (!vget (b_lines - 1, 0, "��J msn �b���G", account, 45, DOECHO))
	return 0;

      if (!vget (b_lines - 1, 0, "��J�K�X�G", pass, 25, NOECHO))
	return 0;

      move (b_lines - 1, 0);
      sleep (1);
      clrtoeol ();
      sock = dns_open ("127.0.0.1", 6667);
      zmsg ("�s���� :p");
      sleep (1);

      if (sock > 0)
	{
	  zmsg
	    ("�n�J���A�֤F�O��A���ӵn�J�N�n���@�U�� :p (�Q���p��H�b�� ^^O)");

	  fr = fdopen (sock, "r");
	  fw = fdopen (sock, "w");

	  fprintf (fw, "NICK %d\r\n", cutmp->pid);
	  fflush (fw);
	  fprintf (fw, "USER bitlbee ono ccy :bitlbee run\r\n");
	  fflush (fw);
	  fprintf (fw, "JOIN #bitlbee\r\n");
	  fflush (fw);

	  fprintf (fw, "PRIVMSG root :account add msn %s %s\r\n", account,
	    pass);
	  fflush (fw);
	  fprintf (fw, "PRIVMSG root :account on\r\n");
	  fflush (fw);
	  fprintf (fw, "PRIVMSG root :set charset BIG-5\r\n");
	  fflush (fw);


	  sleep (10);

	  /* �e�� login �� 9 �� configure, ���L */
	  while (i < 13)
	    {
	      bit_fgets ();
	      if (sock <=0)
		return 0;

	      i++;
	    };

	  /* root �A�]�w mode, ���L */
	  while (bit_fgets ())
	    {
	      if (sock <=0)
		return XO_QUIT;

	      tmp = strstr (buf, "Error");

	      if (tmp)
		{
		  sock = 0;
		  vmsg ("�b���αK�X��J���~�� :p");
		  goto login_error;
		}
	    };
	}
    }

  if (sock > 0)
    {
      xz[XZ_BITLBEE - XO_ZONE].xo = &bit_xo;
      xz[XZ_BITLBEE - XO_ZONE].cb = bit_cb;
      if (i)
	bell ();
      xover (XZ_BITLBEE);
    }
  else
    vmsg ("�L�k�}�ҳs�u�A�Ц� sysop ���^��");

login_error:
  return 0;
}